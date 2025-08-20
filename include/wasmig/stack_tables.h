#ifndef WASM_STACK_TABLES_H
#define WASM_STACK_TABLES_H

#include <stddef.h>
#include <stdint.h>
#include <wasmig/state.h>

typedef enum {
    LOCAL_GET,
    I32_CONST,
    I64_CONST,
    F32_CONST,
    F64_CONST,
    WASMIG_Call,
    Other,
} Opcode;
typedef struct stack_table_entry {
    Opcode opcode;
    uint8_t ty;
    union Operand {
        uint32_t local_idx;
        uint32_t i32;
        uint64_t i64;
        float f32;
        double f64;
        uint32_t call_result_type;
    } operand;
} StackTableEntry;

typedef struct stack_table {
    size_t size;
    StackTableEntry* data;
} StackTable;

#ifdef __cplusplus
extern "C" {
#endif

Array8 get_local_types(uint32_t fidx);
StackTable get_stack_table(uint32_t fidx, uint64_t offset);
// NOTE: stackの値の個数ではなく、32bit slotの個数を返す ([i32, i64, i32]なら1+2+1=4を返す)
uint32_t get_stack_size(StackTable table);
// Callのresultのサイズを返す. 
uint32_t get_result_size(StackTable table);

Array8 convert_type_stack_from_stack_table(StackTable *table);

#ifdef __cplusplus
}
#endif

#endif