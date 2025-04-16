#ifndef RUST_C_API_H
#define RUST_C_API_H

#include <stdint.h>
#include <stddef.h>
#include "../../include/wasmig/stack_tables.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for the Rust library
int wcrn_rust_function(); // Example function
size_t wcrn_get_stack_size(uint32_t fidx, uint32_t offset);
StackTable wcrn_get_stack_table(uint32_t fidx, uint32_t offset);


#ifdef __cplusplus
}
#endif

#endif // RUST_C_API_H