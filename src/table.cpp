#include "table.h"
#include <vector>
#include <map>

static std::map<uintptr_t, CodePos> gtable;

int tab_set(uint32_t func_idx, uint64_t offset, uintptr_t address) {
    gtable[address] = CodePos(func_idx, offset);
    return 0;
}

CodePos tab_get(uintptr_t address) {
    return gtable[address];
}