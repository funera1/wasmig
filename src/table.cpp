#include "wasmig/table.h"
#include <map>

static uint32_t cur_fidx;
static std::map<uintptr_t, CodePos> gtable;

extern "C" int tab_set(uint32_t func_idx, uint64_t offset, uintptr_t address) {
    gtable[address] = CodePos{
        func_idx, 
        offset
    };
    return 0;
}

extern "C" CodePos tab_get(uintptr_t address) {
    return gtable[address];
}

int set_cur_fidx(uint32_t fidx) {
    cur_fidx = fidx;
    return 0;
}

uint32_t get_cur_fidx() {
    return cur_fidx;
}