#include "wasmig/state.h"
#include "wasmig/utils.h"
#include "proto/state.pb-c.h"
#include "spdlog/spdlog.h"
#include <unistd.h>
#include <cstdio>

extern "C" {

// TODO: 各構造体にfree関数を用意
State__Array8* from_array8(Array8 *array) {
    State__Array8* array_proto = (State__Array8*)malloc(sizeof(State__Array8));
    state__array8__init(array_proto);
    array_proto->contents.len = array->size;
    array_proto->contents.data = (uint8_t*)malloc(sizeof(uint8_t) * array->size);
    memcpy(array_proto->contents.data, array->contents, sizeof(uint8_t) * array->size);
    return array_proto;
}

Array8* to_array8(State__Array8 *array_proto) {
    Array8* array = (Array8*)malloc(sizeof(Array8));
    array->size = array_proto->contents.len;
    array->contents = (uint8_t*)malloc(array->size);
    memcpy(array->contents, array_proto->contents.data, array->size);
    return array;
}
void free_array8_proto(State__Array8 *array_proto) {
    free(array_proto->contents.data);
    free(array_proto);
}

State__Array32* from_array32(Array32 *array) {
    State__Array32* array_proto = (State__Array32*)malloc(sizeof(State__Array32));
    state__array32__init(array_proto);
    array_proto->n_contents = array->size;
    array_proto->contents = (uint32_t*)malloc(array->size * sizeof(uint32_t));
    memcpy(array_proto->contents, array->contents, array->size * sizeof(uint32_t));
    return array_proto;
}
Array32* to_array32(State__Array32 *array_proto) {
    Array32* array = (Array32*)malloc(sizeof(Array32));
    array->size = array_proto->n_contents;
    array->contents = (uint32_t*)malloc(array->size * sizeof(uint32_t));
    memcpy(array->contents, array_proto->contents, array->size * sizeof(uint32_t));
    return array;
}
void free_array32_proto(State__Array32 *array_proto) {
    free(array_proto->contents);
    free(array_proto);
}

State__Array64* from_array64(Array64 *array) {
    State__Array64* array_proto = (State__Array64*)malloc(sizeof(State__Array64));
    state__array64__init(array_proto);
    array_proto->n_contents = array->size;
    array_proto->contents = (uint64_t*)malloc(array->size);
    memcpy(array_proto->contents, array->contents, array->size);
    return array_proto;
}
Array64* to_array64(State__Array64 *array_proto) {
    Array64* array = (Array64*)malloc(sizeof(Array64));
    array->size = array_proto->n_contents;
    array->contents = (uint64_t*)malloc(array->size);
    memcpy(array->contents, array_proto->contents, array->size);
    return array;
}
void free_array64_proto(State__Array64 *array_proto) {
    free(array_proto->contents);
    free(array_proto);
}

State__TypedArray* from_typed_array(TypedArray *typed_array) {
    State__TypedArray* typed_array_proto = (State__TypedArray*)malloc(sizeof(State__TypedArray));
    state__typed_array__init(typed_array_proto);
    typed_array_proto->types = from_array8(&typed_array->types);
    typed_array_proto->values = from_array32(&typed_array->values);
    return typed_array_proto;
}
TypedArray* to_typed_array(State__TypedArray *typed_array_proto) {
    TypedArray* typed_array = (TypedArray*)malloc(sizeof(TypedArray));
    typed_array->types = *to_array8(typed_array_proto->types);
    typed_array->values = *to_array32(typed_array_proto->values);
    return typed_array;
}
void free_typed_array_proto(State__TypedArray *typed_array_proto) {
    free_array8_proto(typed_array_proto->types);
    free_array32_proto(typed_array_proto->values);
    free(typed_array_proto);
}

State__CodePos* from_code_pos(CodePos *code_pos) {
    State__CodePos* code_pos_proto = (State__CodePos*)malloc(sizeof(State__CodePos));
    state__code_pos__init(code_pos_proto);
    code_pos_proto->fidx = code_pos->fidx;
    code_pos_proto->offset = code_pos->offset;
    return code_pos_proto;
}
CodePos* to_code_pos(State__CodePos *code_pos_proto) {
    CodePos *code_pos = (CodePos*)malloc(sizeof(CodePos));
    code_pos->fidx = code_pos_proto->fidx;
    code_pos->offset = code_pos_proto->offset;
    return code_pos;
}
void free_code_pos_proto(State__CodePos *code_pos_proto) {
    free(code_pos_proto);
}

State__LabelStack* from_label_stack(LabelStack *label_stack) {
    State__LabelStack* label_stack_proto = (State__LabelStack *)malloc(sizeof(State__LabelStack));
    state__label_stack__init(label_stack_proto);

    size_t size = label_stack->size;
    size_t bytes = sizeof(uint32_t) * size;

    // begins
    label_stack_proto->n_begins = size;
    label_stack_proto->begins = (uint32_t *)malloc(bytes);
    memcpy(label_stack_proto->begins, label_stack->begins, bytes);

    // targets
    label_stack_proto->n_targets = size;
    label_stack_proto->targets = (uint32_t *)malloc(bytes);
    memcpy(label_stack_proto->targets, label_stack->targets, bytes);

    // stack pointers
    label_stack_proto->n_stack_pointers = size;
    label_stack_proto->stack_pointers = (uint32_t *)malloc(bytes);
    memcpy(label_stack_proto->stack_pointers, label_stack->stack_pointers, bytes);

    // cell nums
    label_stack_proto->n_cell_nums = size;
    label_stack_proto->cell_nums = (uint32_t *)malloc(bytes);
    memcpy(label_stack_proto->cell_nums, label_stack->cell_nums, bytes);
    
    return label_stack_proto;
}

LabelStack* to_label_stack(State__LabelStack *label_stack_proto) {
    LabelStack *label_stack = (LabelStack *)malloc(sizeof(LabelStack));
    label_stack->size = label_stack_proto->n_begins;
    size_t bytes = label_stack->size * sizeof(uint32_t);

    // begins
    label_stack->begins = (uint32_t *)malloc(bytes);
    memcpy(label_stack->begins, label_stack_proto->begins, bytes);
    // targets
    label_stack->targets = (uint32_t *)malloc(bytes);
    memcpy(label_stack->targets, label_stack_proto->targets, bytes);
    // stack pointers
    label_stack->stack_pointers = (uint32_t *)malloc(bytes);
    memcpy(label_stack->stack_pointers, label_stack_proto->stack_pointers, bytes);
    // cell nums
    label_stack->cell_nums = (uint32_t *)malloc(bytes);
    memcpy(label_stack->cell_nums, label_stack_proto->cell_nums, bytes);
    
    return label_stack;
}
void free_label_stack_proto(State__LabelStack *label_stack_proto) {
    free(label_stack_proto->begins);
    free(label_stack_proto->targets);
    free(label_stack_proto->stack_pointers);
    free(label_stack_proto->cell_nums);
    free(label_stack_proto);
}

State__CallStackEntry* from_call_stack_entry(CallStackEntry *entry) {
    State__CallStackEntry* entry_proto = (State__CallStackEntry*)malloc(sizeof(State__CallStackEntry));
    state__call_stack_entry__init(entry_proto);
    entry_proto->pc = from_code_pos(&entry->pc);
    entry_proto->locals = from_typed_array(&entry->locals);
    entry_proto->value_stack = from_typed_array(&entry->value_stack);
    entry_proto->label_stack = from_label_stack(&entry->label_stack);
    return entry_proto;
}
CallStackEntry* to_call_stack_entry(State__CallStackEntry *entry_proto) {
    CallStackEntry *entry = (CallStackEntry *)malloc(sizeof(CallStackEntry));
    entry->pc = *to_code_pos(entry_proto->pc);
    entry->locals = *to_typed_array(entry_proto->locals);
    entry->value_stack = *to_typed_array(entry_proto->value_stack);
    entry->label_stack = *to_label_stack(entry_proto->label_stack);
    return entry;
}
void free_call_stack_entry_proto(State__CallStackEntry *entry_proto) {
    free_code_pos_proto(entry_proto->pc);
    free_typed_array_proto(entry_proto->locals);
    free_typed_array_proto(entry_proto->value_stack);
    free_label_stack_proto(entry_proto->label_stack);

    free(entry_proto);
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
    CallStack call_stack;;
    call_stack.size = call_stack_proto->n_entries;
    call_stack.entries = (CallStackEntry*)malloc(sizeof(CallStackEntry) * call_stack.size);
    for (uint32_t i = 0; i < call_stack.size; ++i) {
        call_stack.entries[i] = *to_call_stack_entry(call_stack_proto->entries[i]);
    }
    return call_stack;
}
void free_call_stack_proto(State__CallStack *call_stack_proto) {
    int size = call_stack_proto->n_entries;
    for (uint32_t i = 0; i < size; ++i) {
        free_call_stack_entry_proto(call_stack_proto->entries[i]);
    }
    free(call_stack_proto);
}

Array8 serialize_array32(Array32 *array) {
    State__Array32 *array_proto = from_array32(array);
    uint32_t size = state__array32__get_packed_size(array_proto);
    uint8_t *buf = (uint8_t*)malloc(size);
    state__array32__pack(array_proto, buf);
    return Array8 {
        .size = size,
        .contents = buf
    };
}

Array32 deserialize_array32(Array8 *buf) {
    State__Array32 *array_proto = state__array32__unpack(NULL, buf->size, buf->contents);
    Array32 ret = *to_array32(array_proto);
    state__array32__free_unpacked(array_proto, NULL);
    return ret;
}

Array8 serialize_call_stack(CallStack *cs) {
    State__CallStack *call_stack_proto = from_call_stack(cs);
    uint32_t size = state__call_stack__get_packed_size(call_stack_proto);
    uint8_t *buf = (uint8_t*)malloc(size);
    state__call_stack__pack(call_stack_proto, buf);
    free_call_stack_proto(call_stack_proto);
    return Array8 {
        .size = size,
        .contents = buf
    };
}
CallStack deserialize_call_stack(Array8 *buf) {
    State__CallStack *call_stack_proto = state__call_stack__unpack(NULL, buf->size, buf->contents);
    CallStack ret = to_call_stack(call_stack_proto);
    state__call_stack__free_unpacked(call_stack_proto, NULL);
    return ret;
}

Array8 serialize_typed_array(TypedArray *typed_array) {
    State__TypedArray *typed_array_proto = from_typed_array(typed_array);
    uint32_t size = state__typed_array__get_packed_size(typed_array_proto);
    uint8_t *buf = (uint8_t*)malloc(size);
    state__typed_array__pack(typed_array_proto, buf);
    free_typed_array_proto(typed_array_proto);
    return Array8 {
        .size = size,
        .contents = buf
    };
}

TypedArray deserialize_typed_array(Array8 *buf) {
    State__TypedArray *typed_array_proto = state__typed_array__unpack(NULL, buf->size, buf->contents);
    TypedArray ret = *to_typed_array(typed_array_proto);
    state__typed_array__free_unpacked(typed_array_proto, NULL);
    return ret;
}

void print_codepos(CodePos *pc) {
    std::string str = "pc: (";
    str += std::to_string(pc->fidx);
    str += ", ";
    str += std::to_string(pc->offset);
    str += ")";
    printf("  %s\n", str.c_str());
}

void print_typed_array(TypedArray *typed_array) {
    std::string types_str = "types: ";
    for (int j = 0; j < typed_array->types.size; ++j) {
        types_str += std::to_string(typed_array->types.contents[j]) + " ";
    }
    printf("  %s\n", types_str.c_str());

    std::string values_str = "values: ";
    for (int j = 0; j < typed_array->types.size; ++j) {
        uint8_t ty = typed_array->types.contents[j];
        switch (ty) {
            case 1: {
                uint32_t val32 = typed_array->values.contents[j];
                values_str += std::to_string(val32) + " ";
                break;
            }
            case 2: {
                uint32_t high = typed_array->values.contents[j];
                uint32_t low = typed_array->values.contents[j+1];
                uint64_t val64 = ((uint64_t)high << 32) | low;
                values_str += std::to_string(val64) + " ";
                j += 1;
                break;
            }
        }
    }
    printf("  %s\n", values_str.c_str());
}

void print_label_stack(LabelStack* label_stack) {
    size_t size = label_stack->size;
    printf("  size: %d\n", size);
    
    // begins
    std::string str;
    str = "  begins: ";
    for (int j = 0; j < size; ++j) {
        str += std::to_string(label_stack->begins[j]) + " ";
    }
    printf("  %s\n", str.c_str());

    // targets
    str = "  targets: ";
    for (int j = 0; j < size; ++j) {
        str += std::to_string(label_stack->targets[j]) + " ";
    }
    printf("  %s\n", str.c_str());

    // stack_pointers
    str = "  stack_pointers: ";
    for (int j = 0; j < size; ++j) {
        str += std::to_string(label_stack->stack_pointers[j]) + " ";
    }
    printf("  %s\n", str.c_str());

    // cell nums
    str = "  cell_nums: ";
    for (int j = 0; j < size; ++j) {
        str += std::to_string(label_stack->cell_nums[j]) + " ";
    }
    printf("  %s\n", str.c_str());
}

void print_call_stack_entry(CallStackEntry *entry) {
    printf("CallStackEntry: fidx=%d, offset=%d\n", entry->pc.fidx, entry->pc.offset);
    printf("pc: \n");
    print_codepos(&entry->pc);

    printf("locals: \n");
    print_typed_array(&entry->locals);

    printf("value_stack: \n");
    print_typed_array(&entry->value_stack);

    printf("label stack: \n");
    print_label_stack(&entry->label_stack);
}
void print_call_stack(CallStack *cs) {
    printf("call_stack size: %d\n", cs->size);
    for (int i = 0; i < cs->size; ++i) {
        CallStackEntry *entry = &cs->entries[i];
        print_call_stack_entry(entry);
    }
}

}