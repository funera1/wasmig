#ifndef WASM_STACK_TABLES_H
#define WASM_STACK_TABLES_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    LOCAL_GET,
    I32_CONST,
    I64_CONST,
    F32_CONST,
    F64_CONST,
    Other,
} Opcode;
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

#ifdef __cplusplus
extern "C" {
#endif

#include <wcrn.h>
size_t get_stack_size(uint32_t fidx, uint64_t offset);
StackTable get_stack_table(uint32_t fidx, uint64_t offset);

#ifdef __cplusplus
}
#endif

#endif