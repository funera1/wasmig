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

void hello_world();

typedef struct codepos {
    uint32_t fidx;
    uint32_t offset;
} CodePos;

typedef struct array8 {
    uint32_t size;
    uint8_t *contents;
} Array8;

typedef struct array32 {
    uint32_t size;
    uint32_t *contents;
} Array32;

typedef struct labels {
    uint32_t size;
    uint32_t *begins;
    uint32_t *targets;
    uint32_t *stack_pointers;
    uint32_t *cell_nums;
} LabelStack;

int checkpoint_memory(uint8_t* memory, uint32_t cur_page);
int checkpoint_global(uint64_t* values, uint32_t* types, int len);
int checkpoint_pc(uint32_t func_idx, uint32_t offset);
int checkpoint_stack(uint32_t call_stack_id, uint32_t entry_fidx, 
    CodePos *ret_addr, CodePos *cur_addr, Array32 *locals, Array32 *value_stack, LabelStack *label_stack, bool is_top);
int checkpoint_call_stack_size(uint32_t call_stack_size);

uint8_t* get_type_stack(uint32_t fidx, uint32_t offset, uint32_t* type_stack_size, bool is_return_address);
#endif