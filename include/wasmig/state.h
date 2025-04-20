#ifndef WASMIG_STATE_H
#define WASMIG_STATE_H

#include <unistd.h>
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

typedef struct base_callstack_entry {
    CodePos pc;
    Array32 locals;
    Array32 value_stack;
    LabelStack label_stack;
} BaseCallStackEntry;

typedef struct callstack_entry {
    CodePos pc;
    TypedArray locals;
    TypedArray value_stack;
    LabelStack label_stack;
} CallStackEntry;

typedef struct callstack {
    uint32_t size;
    CallStackEntry *entries;
} CallStack;

#ifdef __cplusplus
extern "C" {
#endif

Array8 serialize_array32(Array32 *array);
Array32 deserialize_array32(Array8 *buf);
Array8 serialize_call_stack(CallStack *cs);
CallStack deserialize_call_stack(Array8 *buf);
Array8 serialize_typed_array(TypedArray *array);
TypedArray deserialize_typed_array(Array8 *buf);

void print_call_stack(CallStack *cs);

#ifdef __cplusplus
}
#endif

#endif