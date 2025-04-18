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

#ifdef __cplusplus
extern "C" {
#endif

int checkpoint_memory(uint8_t* memory, uint32_t cur_page);
int checkpoint_global(uint64_t* values, uint32_t* types, int len);
int checkpoint_pc(uint32_t func_idx, uint32_t offset);
int checkpoint_stack_v2(size_t size, CallStackEntry *call_stack);

Array8 restore_memory();
Array64 restore_global(Array8 types);
CodePos restore_pc();
CallStack restore_stack();

int serialize_array32(Array32 *array);
Array32 deserialize_array32();

#ifdef __cplusplus
}
#endif

#endif