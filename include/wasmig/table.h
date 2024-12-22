#ifndef WASM_TABLE_H
#define WASM_TABLE_H

#include "migration.h"

// extern "C" int tab_alloc(uint32_t func_idx);
// extern "C" int tab_destroy();
int tab_set(uint32_t func_idx, uint64_t offset, uintptr_t address);
CodePos tab_get(uintptr_t address);

#endif