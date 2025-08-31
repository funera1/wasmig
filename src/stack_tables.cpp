#include "wasmig/stack_tables.h"
#include <wcrn.h>

extern "C" {

size_t load_stack_tables() {
    return wcrn_load_stack_tables();    
}

Array8 get_local_types(uint32_t fidx) {
    Array8 types = wcrn_get_local_types(fidx);
    return types;
}
    
StackTable get_stack_table(uint32_t fidx, uint64_t offset) {
    return wcrn_get_stack_table(fidx, offset);
}

uint32_t get_stack_size(StackTable table) {
    uint32_t stack_size = 0;
    for (int i = 0; i < table.size; i++) {
        StackTableEntry entry = table.data[i];
        stack_size += entry.ty;
    }
    return stack_size;
}

uint32_t get_result_size(StackTable table) {
    uint32_t result_size = 0;
    for (int i = table.size-1; i >= 0; i--) {
        StackTableEntry entry = table.data[i];
        if (entry.opcode == WASMIG_Call) {
            result_size += entry.ty;
        } else {
            break;
        }
    }
    return result_size;
}

Array8 convert_type_stack_from_stack_table(StackTable *table) {
    uint32_t stack_size = table->size;
    
    // 配列を確保
    uint8_t* type_stack = (uint8_t*)malloc(stack_size * sizeof(uint8_t));
    // スタック型をコピー
    for (uint32_t i = 0; i < stack_size; ++i) {
        type_stack[i] = table->data[i].ty;
    }

    return (Array8){ .size = stack_size, .contents = type_stack };
}


}