#include "ddprof.hh"

extern "C" {
#include <sys/types.h>
#include <unistd.h>

#include "sampler.h"
}

namespace ddtrace {

// todo: move to periodic collector
class stack_collector : public ddprof::collector {
    // todo: this should only be a reference
    class ddprof::recorder recorder {};

   public:
    // todo: switch to collector API

    void push(std::size_t num_entries, ddtrace_sample_entry entries[]) {
        ddprof::string_table &string_table = recorder.get_string_table();

        ddprof::stack_event event{};
        event.sampled.basic.name = 0;                                 // todo
        event.sampled.basic.timestamp = ddprof::system_clock::now();  // todo
        event.sampled.sampling_period = std::chrono::nanoseconds(0);  // todo
        event.thread_id = getpid();                                   // todo: get sampled thread, not collector thread
        event.thread_name = 0;                                        // todo

        for (std::size_t i = 0; i < num_entries; ++i) {
            ddtrace_sample_entry *entry = entries + i;

            size_t function = 0;
            if (entry->function) {
                auto &interned = string_table.intern(std::string_view{&entry->function->val[0], entry->function->len});
                function = interned.offset;
            }

            size_t filename = 0;
            if (entry->filename) {
                auto &interned = string_table.intern(std::string_view{&entry->filename->val[0], entry->filename->len});
                filename = interned.offset;
            }

            int64_t lineno = entry->lineno;
            ddprof::frame frame{function, filename, lineno};
            event.frames.push_back(frame);
        }
    }

    void start() override {}

    void stop() override {}

    void join() override {}
};

}  // namespace ddtrace

void ddtrace_record_stack_samples(void *obj, size_t size, ddtrace_sample_entry *entries) {
    auto collector = reinterpret_cast<ddtrace::stack_collector *>(obj);
    collector->push(size, entries);
}

void *ddprof_make_stack_sampler() { return (void *)new ddtrace::stack_collector(); }

void ddprof_destroy_stack_sampler(void *obj) {
    auto collector = reinterpret_cast<ddtrace::stack_collector *>(obj);
    delete collector;
}