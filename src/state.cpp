#include "wasmig/state.h"
#include "wasmig/utils.h"
#include "proto/state.pb-c.h"
#include "spdlog/spdlog.h"
#include <unistd.h>
#include <cstdio>

// TODO: 各構造体にfree関数を用意
State__Array8* from_array8(Array8 *array) {
    State__Array8* array_proto = (State__Array8*)malloc(sizeof(State__Array8));
    state__array8__init(array_proto);
    array_proto->contents.len = array->size;
    array_proto->contents.data = (uint8_t*)malloc(sizeof(uint8_t) * array->size);
    memcpy(array_proto->contents.data, array->contents, sizeof(uint8_t) * array->size);
    return array_proto;
}

Array8 to_array8(State__Array8 *array_proto) {
    Array8 array;
    array.size = array_proto->contents.len;
    array.contents = (uint8_t*)malloc(sizeof(uint8_t) * array.size);
    memcpy(array.contents, array_proto->contents.data, sizeof(uint8_t) * array.size);
    return array;
}

State__Array32* from_array32(Array32 *array) {
    State__Array32* array_proto = (State__Array32*)malloc(sizeof(State__Array32));
    state__array32__init(array_proto);
    array_proto->n_contents = array->size;
    array_proto->contents = (uint32_t*)malloc(sizeof(uint32_t) * array->size);
    memcpy(array_proto->contents, array->contents, sizeof(uint32_t) * array->size);
    return array_proto;
}

Array32 to_array32(State__Array32 *array_proto) {
    Array32 array;
    array.size = array_proto->n_contents;
    array.contents = (uint32_t*)malloc(sizeof(uint32_t) * array.size);
    memcpy(array.contents, array_proto->contents, sizeof(uint32_t) * array.size);
    return array;
}

State__Array64* from_array64(Array64 *array) {
    State__Array64* array_proto = (State__Array64*)malloc(sizeof(State__Array64));
    state__array64__init(array_proto);
    array_proto->n_contents = array->size;
    array_proto->contents = (uint64_t*)malloc(sizeof(uint64_t) * array->size);
    memcpy(array_proto->contents, array->contents, sizeof(uint64_t) * array->size);
    return array_proto;
}
Array64 to_array64(State__Array64 *array_proto) {
    Array64 array;
    array.size = array_proto->n_contents;
    array.contents = (uint64_t*)malloc(sizeof(uint64_t) * array.size);
    memcpy(array.contents, array_proto->contents, sizeof(uint64_t) * array.size);
    return array;
}

State__TypedArray* from_typed_array(TypedArray *typed_array) {
    State__TypedArray* typed_array_proto = (State__TypedArray*)malloc(sizeof(State__TypedArray));
    state__typed_array__init(typed_array_proto);
    typed_array_proto->types = from_array8(&typed_array->types);
    typed_array_proto->values = from_array32(&typed_array->values);
    return typed_array_proto;
}

TypedArray to_typed_array(State__TypedArray *typed_array_proto) {
    TypedArray typed_array;
    typed_array.types = to_array8(typed_array_proto->types);
    typed_array.values = to_array32(typed_array_proto->values);
    return typed_array;
}

State__CodePos* from_code_pos(CodePos *code_pos) {
    State__CodePos* code_pos_proto = (State__CodePos*)malloc(sizeof(State__CodePos));
    state__code_pos__init(code_pos_proto);
    code_pos_proto->fidx = code_pos->fidx;
    code_pos_proto->offset = code_pos->offset;
    return code_pos_proto;
}
CodePos to_code_pos(State__CodePos *code_pos_proto) {
    CodePos code_pos;
    code_pos.fidx = code_pos_proto->fidx;
    code_pos.offset = code_pos_proto->offset;
    return code_pos;
}

State__CallStackEntry* from_call_stack_entry(CallStackEntry *entry) {
    State__CallStackEntry* entry_proto = (State__CallStackEntry*)malloc(sizeof(State__CallStackEntry));
    state__call_stack_entry__init(entry_proto);
    entry_proto->pc = from_code_pos(&entry->pc);
    entry_proto->locals = from_typed_array(&entry->locals);
    entry_proto->value_stack = from_typed_array(&entry->value_stack);
    return entry_proto;
}
CallStackEntry to_call_stack_entry(State__CallStackEntry *entry_proto) {
    CallStackEntry entry;
    entry.pc = to_code_pos(entry_proto->pc);
    entry.locals = to_typed_array(entry_proto->locals);
    entry.value_stack = to_typed_array(entry_proto->value_stack);
    return entry;
}

State__CallStack* from_call_stack(CallStack *call_stack) {
    State__CallStack* call_stack_proto = (State__CallStack*)malloc(sizeof(State__CallStack));
    state__call_stack__init(call_stack_proto);
    call_stack_proto->n_entries = call_stack->size;
    call_stack_proto->entries = (State__CallStackEntry**)malloc(sizeof(State__CallStackEntry*) * call_stack->size);
    for (uint32_t i = 0; i < call_stack->size; ++i) {
        call_stack_proto->entries[i] = from_call_stack_entry(&call_stack->entries[i]);
    }
    return call_stack_proto;
}
CallStack to_call_stack(State__CallStack *call_stack_proto) {
    CallStack call_stack;
    call_stack.size = call_stack_proto->n_entries;
    call_stack.entries = (CallStackEntry*)malloc(sizeof(CallStackEntry) * call_stack.size);
    for (uint32_t i = 0; i < call_stack.size; ++i) {
        call_stack.entries[i] = to_call_stack_entry(call_stack_proto->entries[i]);
    }
    return call_stack;
}

Array8 serialize_array32(Array32 *array) {
    State__Array32 *array_proto = from_array32(array);
    size_t size = state__array32__get_packed_size(array_proto);
    uint8_t *buf = (uint8_t*)malloc(size);
    uint32_t len = state__array32__pack(array_proto, buf);
    return Array8 {
        .size = len,
        .contents = buf
    };
}

Array32 deserialize_array32(Array8 *buf) {
    State__Array32 *array_proto = state__array32__unpack(NULL, buf->size, buf->contents);
    Array32 ret = to_array32(array_proto);
    state__array32__free_unpacked(array_proto, NULL);
    return ret;
}

Array8 serialize_call_stack(CallStack *cs) {
    State__CallStack *call_stack_proto = from_call_stack(cs);
    size_t size = state__call_stack__get_packed_size(call_stack_proto);
    uint8_t *buf = (uint8_t*)malloc(size);
    uint32_t len = state__call_stack__pack(call_stack_proto, buf);
    return Array8 {
        .size = len,
        .contents = buf
    };
}
CallStack deserialize_call_stack(Array8 *buf) {
    State__CallStack *call_stack_proto = state__call_stack__unpack(NULL, buf->size, buf->contents);
    CallStack ret = to_call_stack(call_stack_proto);
    state__call_stack__free_unpacked(call_stack_proto, NULL);
    return ret;
}

void print_call_stack_entry(CallStackEntry *entry) {
    spdlog::debug("CallStackEntry: fidx={}, offset={}", entry->pc.fidx, entry->pc.offset);
    spdlog::debug("locals: ");
    for (int j = 0; j < entry->locals.values.size; ++j) {
        spdlog::debug("  {}", entry->locals.values.contents[j]);
    }
    spdlog::debug("value_stack: ");
    for (int j = 0; j < entry->value_stack.values.size; ++j) {
        spdlog::debug("  {}", entry->value_stack.values.contents[j]);
    }
}
void print_call_stack(CallStack *cs) {
    for (int i = 0; i < cs->size; ++i) {
        CallStackEntry *entry = &cs->entries[i];
        print_call_stack_entry(entry);
    }
}