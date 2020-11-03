#ifndef DD_TRACE_SAMPLER_H
#define DD_TRACE_SAMPLER_H
#include <php.h>

typedef struct _ddtrace_sample_entry {
    zend_string *function;
    zend_string *filename;
    uint32_t lineno;
} ddtrace_sample_entry;

void ddtrace_sampler_rinit(void);
void ddtrace_serialize_samples(HashTable *serialized);
void ddtrace_sampler_rshutdown(void);

void *ddprof_make_stack_sampler(void);
void ddprof_destroy_stack_sampler(void *collector);

void ddtrace_record_stack_samples(void *collector, size_t size, ddtrace_sample_entry *entries);

#endif  // DD_TRACE_SAMPLER_H
