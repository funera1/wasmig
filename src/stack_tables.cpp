#include "wasmig/stack_tables.h"
#include <wcrn.h>


extern "C" {
    
StackTable get_stack_table(uint32_t fidx, uint64_t offset) {
    StackTable stack_table;
    // stack_table.locals = wcrn_get_locals(fidx, offset);
    // stack_table.value_stack = wcrn_get_value_stack(fidx, offset);
    // stack_table.label_stack = wcrn_get_label_stack(fidx, offset);
    return stack_table;
}
size_t get_stack_size(uint32_t fidx, uint64_t offset) {
    return wcrn_get_stack_size(fidx, offset);
}

}