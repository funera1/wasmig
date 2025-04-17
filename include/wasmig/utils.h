#ifndef WASMIG_UTILS_H
#define WASMIG_UTILS_H

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

const uint32_t WASM_PAGE_SIZE = 0x10000;

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

typedef struct array64 {
    uint32_t size;
    uint64_t *contents;
} Array64;

typedef struct typed_array {
    Array8 types;
    Array32 values;
} TypedArray;

typedef struct labels {
    uint32_t size;
    uint32_t *begins;
    uint32_t *targets;
    uint32_t *stack_pointers;
    uint32_t *cell_nums;
} LabelStack;

typedef struct callstack_entry {
    CodePos pc;
    // TypedArray locals;
    // TypedArray value_stack;
    Array32 locals;
    Array32 value_stack;
    LabelStack label_stack;
} CallStackEntry;

typedef struct callstack {
    uint32_t size;
    CallStackEntry *entries;
} CallStack;

FILE* open_image(const char* file, const char* flag);
int is_page_dirty(uint64_t pagemap_entry);
int is_page_soft_dirty(uint64_t pagemap_entry);


#endif