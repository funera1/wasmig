#ifndef RUST_C_API_H
#define RUST_C_API_H

#include <stdint.h>
#include <stddef.h>
// TODO: 相対パスをincludeしないようにする
#include "../../include/wasmig/stack_tables.h"
#include "../../include/wasmig/migration.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for the Rust library
int wcrn_rust_function(); // Example function

size_t wcrn_load_stack_tables();
size_t wcrn_get_stack_size(uint32_t fidx, uint32_t offset);
// TODO: deserializeを何回もしないようにする
StackTable wcrn_get_stack_table(uint32_t fidx, uint32_t offset);
Array8 wcrn_get_local_types(uint32_t fidx);
Array32 wcrn_offset_list(uint32_t fidx);

#ifdef __cplusplus
}
#endif

#endif // RUST_C_API_H