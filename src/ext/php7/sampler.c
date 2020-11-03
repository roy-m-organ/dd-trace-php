#include "sampler.h"

#include <datadog/arena.h>
#include <php.h>
#include <pthread.h>

// TODO Remove: Just for var_dump()'ing
#include <ext/standard/php_var.h>

// TODO: figure out how to conditionally include this stuff if it works out
#include <sys/uio.h>

#include "../logging.h"

/* Heavily inspired by Nikita Popov's sampling profiler
   https://github.com/nikic/sample_prof */

#define DD_SAMPLE_DEFAULT_INTERVAL 9973  // prime close to 10 millisecond

/* On 64-bit this will give a 16 * 1MB allocation */
// todo: recalculate above math ^
#define DD_SAMPLE_DEFAULT_ALLOC (1 << 20)

ZEND_TLS pthread_t thread_id;
ZEND_TLS ddtrace_sample_entry *entries;
ZEND_TLS size_t entries_num;

typedef enum dd_readv_result {
    DD_READV_FAILURE = -1,
    DD_READV_SUCCESS = 0,
    DD_READV_PARTIAL = 1,
} dd_readv_result;

typedef struct dd_readv_t {
    void *local;
    void *remote;
    size_t size;
} dd_readv_t;

static dd_readv_result dd_process_vm_readv(pid_t pid, dd_readv_t readv) {
    struct iovec local = {.iov_base = readv.local, .iov_len = readv.size};
    struct iovec remote = {.iov_base = readv.remote, .iov_len = readv.size};

    ssize_t bytes_read = process_vm_readv(pid, &local, 1, &remote, 1, 0);

    ssize_t bytes_expected = readv.size;
    if (EXPECTED(bytes_read == bytes_expected)) {
        return DD_READV_SUCCESS;
    }

    return bytes_read < 0 ? DD_READV_FAILURE : DD_READV_PARTIAL;
}

static dd_readv_result dd_process_vm_readv_multiple(pid_t pid, unsigned n, dd_readv_t readvs[]) {
    // we only support up to 4 reads atm
    ZEND_ASSERT(n <= 4);
    struct iovec local[4];
    struct iovec remote[4];

    ssize_t bytes_expected = 0;
    for (unsigned i = 0; i < n; ++i) {
        dd_readv_t readv = readvs[i];
        local[i].iov_len = readv.size;
        local[i].iov_base = readv.local;
        remote[i].iov_len = readv.size;
        remote[i].iov_base = readv.remote;
        bytes_expected += readv.size;
    }

    ssize_t bytes_read = process_vm_readv(pid, local, n, remote, n, 0);

    if (EXPECTED(bytes_read == bytes_expected)) {
        return DD_READV_SUCCESS;
    }

    return bytes_read < 0 ? DD_READV_FAILURE : DD_READV_PARTIAL;
}

static zend_string *dd_readv_string(pid_t pid, datadog_arena *arena, zend_string *remote) {
    if (!remote) {
        return NULL;
    }

    zend_string tmp;
    dd_readv_result result = dd_process_vm_readv(pid, (dd_readv_t){&tmp, remote, sizeof tmp});
    if (UNEXPECTED(result != DD_READV_SUCCESS)) {
        if (result == DD_READV_PARTIAL) {
            ddtrace_log_errf("Continuous profiling: partial read of zend_string");
        } else {
            ddtrace_log_errf("Continuous profiling: failed to read zend_string");
        }
        return NULL;
    }

    // zend_strings are null terminated, hence the +1
    size_t total_size = offsetof(zend_string, val) + tmp.len + 1;
    void *checkpoint = datadog_arena_checkpoint(arena);
    zend_string *local = (zend_string *)datadog_arena_try_alloc(arena, total_size);
    if (!local) {
        ddtrace_log_errf("Continuous profiling: failed to arena allocate a zend_string of length %l", total_size);
        return NULL;
    }

    result = dd_process_vm_readv(pid, (dd_readv_t){local, remote, total_size});
    if (UNEXPECTED(result != DD_READV_SUCCESS)) {
        ddtrace_log_errf("Continuous profiling: failed to read zend_string data");
        datadog_arena_restore(&arena, checkpoint);
        return NULL;
    }

    local->gc.u.type_info = IS_STR_INTERNED;
    GC_SET_REFCOUNT(local, 1);

    return local;
}

ZEND_TLS datadog_arena *profiling_arena = NULL;

static void *dd_sample_handler(void *data) {
    UNUSED(data);
#ifndef ZTS
    volatile zend_executor_globals *eg = &executor_globals;

    /* Big open question: is process_vm_readv _actually_ providing safety?
     * The thought is that if they aren't in the address space then it won't
     * sigsegv, which is good but not the whole story because of the ZMM. If an
     * address exists in the process but has been efree'd then it will have
     * garbage and I don't think we can tell at all. Maybe we can use some
     * heuristics based on its type and context? For instance, a string that
     * represents a function or class name is probably less than 100 characters
     * and its refcount is probably also less than 100.
     *
     * Really should consider investing into PHP 8.1 some way to do this safely.
     */

    pid_t pid = getpid();

    // if we run out of space in a single arena, stop gathering data
    profiling_arena = datadog_arena_create(1048576);  // 1 MiB
    void *checkpoint = datadog_arena_checkpoint(profiling_arena);

    // todo: clean up
    void *collector = ddprof_make_stack_sampler();

    while (true) {
        usleep(DD_SAMPLE_DEFAULT_INTERVAL);

        zend_execute_data local_ex, local_prev_execute_data;
        zend_execute_data *remote_ex = eg->current_execute_data;

        /* We're not executing code right now, try again later */
        if (!remote_ex) continue;

        dd_readv_result readv_result = dd_process_vm_readv(pid, (dd_readv_t){&local_ex, remote_ex, sizeof local_ex});
        if (UNEXPECTED(readv_result != DD_READV_SUCCESS)) {
            if (readv_result == DD_READV_FAILURE) {
                ddtrace_log_errf("Continuous profiling: failed to read root execute_data call frame: %s",
                                 strerror(errno));
            } else {
                ddtrace_log_err("Continuous profiling: partial read on root execute_data call frame");
            }
            continue;
        }

        bool should_continue = true;
        do {
            /* Avoid nullptrs in our reads so that we can distinguish between
             * failed and partial reads. Also minimize the number of remote
             * reads for performance.
             * Conditionally build up an array of things to read to solve this.
             */
            unsigned n = 0;
            dd_readv_t readv[3];

            zend_function local_func;
            zend_op local_opline;

            if (local_ex.prev_execute_data) {
                readv[n++] =
                    (dd_readv_t){&local_prev_execute_data, local_ex.prev_execute_data, sizeof local_prev_execute_data};
            } else {
                should_continue = false;
            }

            bool has_func = local_ex.func;
            if (has_func) {
                readv[n++] = (dd_readv_t){&local_func, local_ex.func, sizeof local_func};
            }

            bool has_opline = local_ex.opline;
            if (has_opline) {
                readv[n++] = (dd_readv_t){&local_opline, (void *)local_ex.opline, sizeof local_opline};
            }

            if (n == 0) break;

            dd_readv_result read_result = dd_process_vm_readv_multiple(pid, n, readv);
            if (UNEXPECTED(read_result != DD_READV_SUCCESS)) {
                if (read_result == DD_READV_FAILURE) {
                    ddtrace_log_errf("Continuous profiling: failed to read sub-objects of execute_data: %s",
                                     strerror(errno));
                } else {
                    ddtrace_log_err("Continuous profiling: partial read");
                }
                /* if we couldn't fetch the prev_execute_data then we're done
                 * with this specific stack trace.
                 */
                break;
            }

            if (local_ex.prev_execute_data) {
                local_ex = local_prev_execute_data;
            }

            if (has_func) {
                if (ZEND_USER_CODE(local_func.type)) {
                    entries[entries_num].function =
                        dd_readv_string(pid, profiling_arena, local_func.op_array.function_name);
                    if (!entries[entries_num].function && local_func.op_array.function_name) {
                        ddtrace_log_err("Continuous profiling: failed to read userland function name");
                    }
                    entries[entries_num].filename = dd_readv_string(pid, profiling_arena, local_func.op_array.filename);
                    entries[entries_num].lineno = has_opline ? local_opline.lineno : 0;

                } else {
                    zend_string *function = local_func.internal_function.function_name;
                    entries[entries_num].function = dd_readv_string(pid, profiling_arena, function);
                    if (function && !entries[entries_num].function) {
                        ddtrace_log_err("Continuous profiling: failed to read internal function name");
                    }
                    entries[entries_num].filename = NULL;
                    entries[entries_num].lineno = 0;
                }

                if (++entries_num == DD_SAMPLE_DEFAULT_ALLOC) {
                    // todo: figure out when to reallocate
                    // todo: save samples first
                    goto retry_never;
                }
            }

            ddtrace_record_stack_samples(collector, entries_num, entries);

            // Once events have been pushed we can reset entries and arena!
            // entries_num = 0;
            // datadog_arena_restore(&profiling_arena, checkpoint);
        } while (should_continue);
    }

retry_never:
    pthread_exit(NULL);
#endif
}

void ddtrace_sampler_rinit(void) {
    entries_num = 0;
    entries = safe_emalloc(DD_SAMPLE_DEFAULT_ALLOC, sizeof(ddtrace_sample_entry), 0);

    /* Register signal handler */
    if (pthread_create(&thread_id, NULL, dd_sample_handler, NULL)) {
        ddtrace_log_debugf("Could not register signal handler");
        return;
    }
}

void ddtrace_serialize_samples(HashTable *serialized) {
    size_t entry_num;

    ZEND_HASH_FILL_PACKED(serialized) {
        for (entry_num = 0; entry_num < entries_num; ++entry_num) {
            ddtrace_sample_entry *entry = &entries[entry_num];
            // create a tuple (function, filename, lineno)
            zval tuple;
            array_init_size(&tuple, 3);
            zend_hash_real_init_packed(Z_ARRVAL(tuple));
            ZEND_HASH_FILL_PACKED(Z_ARRVAL(tuple)) {
                zval tmp = {.u1.type_info = IS_NULL};
                if (entry->function) {
                    ZVAL_STR(&tmp, entry->function);
                }
                ZEND_HASH_FILL_ADD(&tmp);
                zval_dtor(&tmp);

                ZVAL_NULL(&tmp);
                if (entry->filename) {
                    ZVAL_STR(&tmp, entry->filename);
                }
                ZEND_HASH_FILL_ADD(&tmp);
                zval_dtor(&tmp);

                ZVAL_LONG(&tmp, entry->lineno);
                ZEND_HASH_FILL_ADD(&tmp);
            }
            ZEND_HASH_FILL_END();

            ZEND_HASH_FILL_ADD(&tuple);
        }
    }
    ZEND_HASH_FILL_END();
}

void ddtrace_sampler_rshutdown(void) {
    ///*
    zval serialized;
    array_init_size(&serialized, entries_num);
    zend_hash_real_init_packed(Z_ARR(serialized));
    ddtrace_serialize_samples(Z_ARR(serialized));

    // For now we'll just dump them to STDOUT
    php_printf("Took %zu samples:\n", entries_num);
    php_var_export(&serialized, 1);
    php_printf("\n");

    //*/

    pthread_cancel(thread_id);
    pthread_join(thread_id, NULL);

    zend_array_destroy(Z_ARR(serialized));
    datadog_arena_destroy(profiling_arena);
    efree(entries);
}
