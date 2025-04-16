#include "wasmig/stack_tables.h"
#include <wcrn.h>


extern "C" {
    
size_t get_stack_size(uint32_t fidx, uint64_t offset) {
    return wcrn_get_stack_size(fidx, offset);
}

StackTable get_stack_table(uint32_t fidx, uint64_t offset) {
    return wcrn_get_stack_table(fidx, offset);
}

}