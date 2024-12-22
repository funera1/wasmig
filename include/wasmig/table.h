#ifndef WASM_TABLE_H
#define WASM_TABLE_H

#include "migration.h"
#include <vector>
#include <map>

// using table = std::map<uintptr_t, CodePos>;
// static std::vector<table> gtable;
static std::map<uintptr_t, CodePos> gtable;

extern "C" int tab_alloc(uint32_t func_idx);
extern "C" int tab_destroy();
extern "C" int tab_set(uint32_t func_idx, uint64_t offset, uintptr_t address);
extern "C" CodePos tab_get(uintptr_t address);

#endif