#include "wasmig/state.h"
#include "wasmig/utils.h"
#include "wasmig/proto/state.pb-c.h"
#include "spdlog/spdlog.h"
#include <unistd.h>
#include <cstdio>

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

int serialize_array32(FILE *fp, Array32 *array) {
    if (fp == NULL) {
        spdlog::error("failed to open array file");
        return -1;
    }
    
    State__Array32 *array_proto = from_array32(array);

    // packed array32 and write to file
    size_t size = state__array32__get_packed_size(array_proto);
    uint8_t *buf = (uint8_t*)malloc(size);
    size_t len = state__array32__pack(array_proto, buf);
    spdlog::info("packed array size: {}", len);
    fwrite(buf, sizeof(uint8_t), len, fp);
    spdlog::info("write array to file");
    
    // free memory
    free(buf);
    spdlog::info("free buf");
    free(array_proto->contents);
    spdlog::info("free array contents");
    free(array_proto);
    spdlog::info("free array proto");
    return 0;
}

Array32 deserialize_array32(FILE *fp) {
    if (!fp) {
        spdlog::error("failed to open file");
        return (Array32){0, NULL};
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t*)malloc(len);
    fread(buf, sizeof(uint8_t), len, fp);

    State__Array32 *array_proto;
    array_proto = state__array32__unpack(NULL, len, buf);
    Array32 array;
    array.size = array_proto->n_contents;
    array.contents = (uint32_t*)malloc(sizeof(uint32_t) * array.size);
    memcpy(array.contents, array_proto->contents, sizeof(uint32_t) * array.size);

    free(buf);
    state__array32__free_unpacked(array_proto, NULL);
    
    return array;
}


int serialize_call_stack(FILE *fp, CallStack *cs) {
/*     if (fp == NULL) {
        spdlog::error("failed to open call stack file");
        return -1;
    }
    
    // State_CallStack 初期化
    State__CallStack *call_stack_proto;
    call_stack_proto = (State__CallStack*)malloc(sizeof(State__CallStack));
    state__call_stack__init(call_stack_proto);
    spdlog::info("init call stack proto");
    call_stack_proto->n_entries = cs->size;
    call_stack_proto->entries = (State__CallStackEntry**)malloc(sizeof(State__CallStackEntry*) * cs->size);
    spdlog::info("init call stack entries");
    for (uint32_t i = 0; i < cs->size; ++i) {
        State__CallStackEntry *entry_proto;
        // entry_protoの初期化
        entry_proto = (State__CallStackEntry*)malloc(sizeof(State__CallStackEntry));
        state__call_stack_entry__init(entry_proto);
        spdlog::info("init call stack entry proto");
        
        // pc_protoの初期化
        State__CodePos *pc_proto;
        pc_proto = (State__CodePos*)malloc(sizeof(State__CodePos));
        state__code_pos__init(pc_proto);
        spdlog::info("init call stack entry pc proto");
        pc_proto->fidx = cs->entries[i].pc.fidx;
        pc_proto->offset = cs->entries[i].pc.offset;
        entry_proto->pc = pc_proto;
        
        // localsの初期化
        State__TypedArray *locals_proto;
        locals_proto = (State__TypedArray*)malloc(sizeof(State__TypedArray));
        state__typed_array__init(locals_proto);
        spdlog::info("init call stack entry locals proto");
        State__Array8 *locals_types_proto;
        locals_types_proto = (State__Array8*)malloc(sizeof(State__Array8));
        state__array8__init(locals_types_proto);
        spdlog::info("init call stack entry locals types proto");
        locals_types_proto->n_contents = cs->entries[i].locals.size;
        locals_types_proto->contents = (uint8_t*)malloc(sizeof(uint8_t) * cs->entries[i].locals.size);
        memcpy(locals_types_proto->contents, cs->entries[i].locals.types.contents, sizeof(uint8_t) * cs->entries[i].locals.size);
        locals_proto->types.n_contents = cs->entries[i].locals.size;
        locals_proto->types.contents = (uint8_t*)malloc(sizeof(uint8_t) * cs->entries[i].locals.size);
        memcpy(locals_proto->types.contents, cs->entries[i].locals.types.contents, sizeof(uint8_t) * cs->entries[i].locals.size);
        entry_proto->locals = locals_proto;
        // entry_proto->locals.n_contents = cs->entries[i].locals.size;
        // entry_proto->locals.contents = (uint32_t*)malloc(sizeof(uint32_t) * cs->entries[i].locals.size);
        // memcpy(entry_proto->locals.contents, cs->entries[i].locals.contents, sizeof(uint32_t) * cs->entries[i].locals.size);
        
        // value_stackの初期化
        entry_proto->value_stack.n_contents = cs->entries[i].value_stack.size;
        entry_proto->value_stack.contents = (uint32_t*)malloc(sizeof(uint32_t) * cs->entries[i].value_stack.size);
        memcpy(entry_proto->value_stack.contents, cs->entries[i].value_stack.contents, sizeof(uint32_t) * cs->entries[i].value_stack.size);

        // label_stackの初期化
        entry_proto->label_stack.n_begins = cs->entries[i].label_stack.size;
        entry_proto->label_stack.begins = (uint32_t*)malloc(sizeof(uint32_t) * cs->entries[i].label_stack.size);
        memcpy(entry_proto->label_stack.begins, cs->entries[i].label_stack.begins, sizeof(uint32_t) * cs->entries[i].label_stack.size);
        
        // entriesに追加
        call_stack_proto->entries[i] = entry_proto;
    }
 */
}

CallStack deserialize_call_stack(FILE *fp) {

}