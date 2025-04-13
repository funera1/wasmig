#pragma once
#include <wcrn.h>

size_t get_stack_size(uint32_t fidx, uint64_t offset) {
    return wcrn_get_stack_size(fidx, offset);
}