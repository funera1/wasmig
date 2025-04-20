// include/example.h
#ifndef WASM_MIGRATION_H
#define WASM_MIGRATION_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <wasmig/utils.h>
#include <wasmig/state.h>
#include <wasmig/codepos.h>

#ifdef __cplusplus
extern "C" {
#endif

int checkpoint_memory(uint8_t* memory, uint32_t cur_page);
int checkpoint_global(uint64_t* values, uint32_t* types, int len);
int checkpoint_global_v2(TypedArray globals);
int checkpoint_pc(uint32_t func_idx, uint32_t offset);
int checkpoint_stack_v2(size_t size, BaseCallStackEntry *call_stack);
int checkpoint_stack_v3(size_t size, BaseCallStackEntry *call_stack);

Array8 restore_memory();
Array64 restore_global(Array8 types);
TypedArray restore_global_v2();
CodePos restore_pc();
CallStack restore_stack();

#ifdef __cplusplus
}
#endif

#endif