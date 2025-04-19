#include "wasmig/stack_tables.h"
#include <wcrn.h>

extern "C" {

    
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

}