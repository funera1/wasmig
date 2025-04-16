#ifndef RUST_C_API_H
#define RUST_C_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum Opcode {
    LOCAL_GET,
    I32_CONST,
    I64_CONST,
    F32_CONST,
    F64_CONST,
    Other,
};

typedef struct stack_table_entry {
    Opcode opcode;
    union Operand {
        uint32_t local_idx;
        uint32_t i32;
        uint64_t i64;
        float f32;
        double f64;
    };
} StackTableEntry;

typedef struct stack_table {
    size_t size;
    StackTableEntry* data;
} StackTable;

// Function prototypes for the Rust library
int wcrn_rust_function(); // Example function
size_t wcrn_get_stack_size(uint32_t fidx, uint32_t offset);
StackTable wcrn_get_stack_table(uint32_t fidx, uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif // RUST_C_API_H