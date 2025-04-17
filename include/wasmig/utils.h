#ifndef WASMIG_UTILS_H
#define WASMIG_UTILS_H

#include <stdint.h>

typedef struct codepos {
    uint32_t fidx;
    uint64_t offset;
} CodePos;

typedef struct array8 {
    uint32_t size;
    uint8_t *contents;
} Array8;

typedef struct array32 {
    uint32_t size;
    uint32_t *contents;
} Array32;

typedef struct labels {
    uint32_t size;
    uint32_t *begins;
    uint32_t *targets;
    uint32_t *stack_pointers;
    uint32_t *cell_nums;
} LabelStack;

typedef struct callstack_entry {
    CodePos pc;
    Array32 locals;
    Array32 value_stack;
    LabelStack label_stack;
} CallStackEntry;

#endif