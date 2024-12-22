#ifndef WASM_TABLE_H
#define WASM_TABLE_H

#include "migration.h"

static uint32_t cur_fidx;

// extern "C" int tab_alloc(uint32_t func_idx);
// extern "C" int tab_destroy();
int tab_set(uint32_t func_idx, uint64_t offset, uintptr_t address);
CodePos tab_get(uintptr_t address);

int set_cur_fidx(uint32_t fidx);
uint32_t get_cur_fidx();

#endif