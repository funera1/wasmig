// include/example.h
#ifndef WASM_MIGRATION_H
#define WASM_MIGRATION_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

void hello_world();

typedef struct codepos {
    uint32_t fidx;
    uint32_t offset;
} CodePos;

typedef struct array {
    uint32_t size;
    uint8_t *contents;
} Array8;

typedef struct array {
    uint32_t size;
    uint32_t *contents;
} Array32;

typedef struct array {
    uint32_t size;
    uint8_t **begin_addr;
    uint8_t **target_addr;
    uint32_t **frame_sp;
    uint32_t *cell_num;
} LabelStack;

int checkpoint_memory(uint8_t* memory, uint32_t cur_page);
int checkpoint_global(uint64_t* values, uint32_t* types, int len);
int checkpoint_pc(uint32_t func_idx, uint32_t offset);
int checkpoint_stack(CodePos *ret_addr, Array8 *type_stack, Array32 *value_stack, LabelStack *label_stack);

uint8_t* get_type_stack(uint32_t fidx, uint32_t offset, uint32_t* type_stack_size, bool is_return_address);
#endif