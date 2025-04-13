#ifndef WASM_STACK_TABLES_H
#define WASM_STACK_TABLES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t get_stack_size(uint32_t fidx, uint64_t offset);

#ifdef __cplusplus
}
#endif

#endif