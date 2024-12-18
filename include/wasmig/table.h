#ifndef WASM_TABLE_H
#define WASM_TABLE_H

#include "migration.h"
#include <vector>
#include <map>

using table = std::map<uintptr_t, CodePos>;
static std::vector<table> gtable;

extern "C" int t_alloc(uint32_t func_idx);
extern "C" int t_destroy();
extern "C" int t_register(uint32_t func_idx, uint64_t offset, uintptr_t addoress);
extern "C" CodePos t_get(uintptr_t address);

#endif