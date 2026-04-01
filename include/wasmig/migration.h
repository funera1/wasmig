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

int wasmig_checkpoint_memory(uint8_t* memory, uint32_t cur_page);
int wasmig_checkpoint_global(uint64_t* values, uint32_t* types, int len);
int wasmig_checkpoint_global_v2(TypedArray globals);
int wasmig_checkpoint_pc(uint32_t func_idx, uint32_t offset);
int wasmig_checkpoint_stack_v2(size_t size, BaseCallStackEntry *call_stack);
int wasmig_checkpoint_stack_v3(size_t size, BaseCallStackEntry *call_stack);
int wasmig_checkpoint_stack_v4(size_t size, CallStackEntry *call_stack);

typedef struct {
    uint32_t format;
    uint32_t page_count;
    uint32_t chunk_size;
    bool sparse_format;
} WasmigMemoryImageInfo;

typedef int (*wasmig_memory_chunk_visitor_t)(uint32_t offset, const uint8_t *data,
                                             uint32_t len, void *user_data);

int wasmig_memory_image_info(WasmigMemoryImageInfo *out_info);
int wasmig_visit_memory_chunks(wasmig_memory_chunk_visitor_t visitor, void *user_data);
int wasmig_restore_memory_into(uint8_t *dst, size_t dst_size);
Array8 wasmig_restore_memory();
Array64 wasmig_restore_global(Array8 types);
TypedArray wasmig_restore_global_v2();
CodePos wasmig_restore_pc();
CallStack wasmig_restore_stack();

#ifdef __cplusplus
}
#endif

#endif