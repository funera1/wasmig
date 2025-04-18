#ifndef WASMIG_STATE_H
#define WASMIG_STATE_H

#include <unistd.h>
#include <cstdio>
#include <stdint.h>
#include <wasmig/proto/state.pb-c.h>

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


Array8 serialize_array32(Array32 *array);
Array32 deserialize_array32(Array8 *buf);
Array8 serialize_call_stack(CallStack *cs);
CallStack deserialize_call_stack(Array8 *buf);

State__Array8* from_array8(Array8 *array);
Array8 to_array8(State__Array8 *array_proto);
State__Array32* from_array32(Array32 *array);
Array32 to_array32(State__Array32 *array_proto);
State__Array64* from_array64(Array64 *array);
Array64 to_array64(State__Array64 *array_proto);
State__TypedArray* from_typed_array(TypedArray *typed_array);
TypedArray to_typed_array(State__TypedArray *typed_array_proto);
State__CodePos* from_code_pos(CodePos *code_pos);
CodePos to_code_pos(State__CodePos *code_pos_proto);
State__CallStackEntry* from_call_stack_entry(CallStackEntry *entry);
CallStackEntry to_call_stack_entry(State__CallStackEntry *entry_proto);
State__CallStack* from_call_stack(CallStack *call_stack);
CallStack to_call_stack(State__CallStack *call_stack_proto);

#endif