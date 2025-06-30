#include "wasmig/stack_tables.h"
#include <wcrn.h>

extern "C" {

Array8 get_local_types(uint32_t fidx) {
    Array8 types = wcrn_get_local_types(fidx);
    return types;
}
    
StackTable get_stack_table(uint32_t fidx, uint64_t offset, bool is_stack_top) {
    if (is_stack_top) {
        return wcrn_get_stack_table(fidx, offset);
    } else {
        // スタックのトップ以外（関数実行中のスタック）を取得する場合は、offset+1にする
        return wcrn_get_stack_table(fidx, offset+1);
    }
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