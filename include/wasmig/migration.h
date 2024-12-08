// include/example.h
#ifndef WASM_MIGRATION_H
#define WASM_MIGRATION_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void hello_world();

int checkpoint_memory(uint8_t* memory, uint32_t cur_page);
int checkpoint_global(uint64_t* values, uint32_t* types, int len);
int checkpoint_pc(uint32_t func_idx, uint32_t offset);

#endif