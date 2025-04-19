// src/example.c
#include "wasmig/migration.h"
#include "wasmig/stack_tables.h"
#include "wasmig/utils.h"
#include "wasmig/state.h"
#include "wasmig/internal/debug.hpp"
#include "proto/state.pb-c.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <wcrn.h>

extern "C" {

// TODO: バグってるので修正
int write_dirty_memory(uint8_t* memory, uint32_t cur_page) {
    const int PAGEMAP_LENGTH = 8;
    const int OS_PAGE_SIZE = 4096;
    FILE *memory_fp = open_image("memory.img", "wb");
    if (memory_fp == NULL) {
        return -1;
    }

    int fd;
    uint64_t pagemap_entry;
    // プロセスのpagemapを開く
    fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd == -1) {
        perror("Error opening pagemap");
        return -1;
    }

    // pfnに対応するpagemapエントリを取得
    unsigned long pfn = (unsigned long)memory / OS_PAGE_SIZE;
    off_t offset = sizeof(uint64_t) * pfn;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("Error seeking to pagemap entry");
        close(fd);
        return -1;
    }

    uint8_t* memory_data = memory;
    uint8_t* memory_data_end = memory + (cur_page * WASM_PAGE_SIZE);
    int i = 0;
    for (uint8_t* addr = memory; addr < memory_data_end; addr += OS_PAGE_SIZE, ++i) {
        unsigned long pfn = (unsigned long)addr / OS_PAGE_SIZE;
        off_t offset = sizeof(uint64_t) * pfn;
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("Error seeking to pagemap entry");
            close(fd);
            return -1;
        }

        if (read(fd, &pagemap_entry, PAGEMAP_LENGTH) != PAGEMAP_LENGTH) {
            perror("Error reading pagemap entry");
            close(fd);
            return -1;
        }

        // dirty pageのみdump
        // if (is_page_dirty(pagemap_entry)) {
        if (is_page_soft_dirty(pagemap_entry)) {
            // printf("[%x, %x]: dirty page\n", i*PAGE_SIZE, (i+1)*PAGE_SIZE);
            uint32_t offset = (uint64_t)addr - (uint64_t)memory_data;
            // printf("i: %d\n", offset);
            fwrite(&offset, sizeof(uint32_t), 1, memory_fp);
            fwrite(addr, OS_PAGE_SIZE, 1, memory_fp);
        }
    }

    close(fd);
    fclose(memory_fp);
    return 0;
}

int checkpoint_memory(uint8_t* memory, uint32_t cur_page) {
    FILE *mem_fp = open_image("memory.img", "wb");
    FILE *mem_size_fp = open_image("mem_page_count.img", "wb");
    if (mem_fp == NULL || mem_size_fp == NULL) {
        spdlog::error("failed to open memory file");
        return -1;
    }

    // write_dirty_memory(memory, cur_page);
    fwrite(memory, sizeof(uint8_t), WASM_PAGE_SIZE * cur_page, mem_fp);
    fwrite(&cur_page, sizeof(uint32_t), 1, mem_size_fp);

    // fclose(mem_fp);
    fclose(mem_size_fp);
    spdlog::info("checkpoint memory");
    
    return 0;
}

int checkpoint_global(uint64_t* values, uint32_t* types, int len) {
    FILE *fp = open_image("global.img", "wb");
    if (fp == NULL) {
        return -1;
    }

    for (int i = 0; i < len; i++) {
        fwrite(&values[i], types[i], 1, fp);
    }

    fclose(fp);
    return 0;
}

int checkpoint_pc(uint32_t func_idx, uint32_t offset) {
    FILE *fp = open_image("program_counter.img", "wb");
    if (fp == NULL) {
        return -1;
    }
    fwrite(&func_idx, sizeof(uint32_t), 1, fp);
    fwrite(&offset, sizeof(uint32_t), 1, fp);
    fclose(fp);

    return 0;
}

uint8_t* get_type_stack(uint32_t fidx, uint32_t offset, uint32_t* type_stack_size, bool is_return_address) {
    FILE *tablemap_func = fopen("tablemap_func", "rb");
    if (!tablemap_func) printf("not found tablemap_func\n");
    FILE *tablemap_offset = fopen("tablemap_offset", "rb");
    if (!tablemap_func) printf("not found tablemap_offset\n");
    FILE *type_table = fopen("type_table", "rb");
    if (!tablemap_func) printf("not found type_table\n");
    
    /// tablemap_func
    fseek(tablemap_func, fidx*sizeof(uint32_t)*3, SEEK_SET);
    uint32_t ffidx;
    uint64_t tablemap_offset_addr;
    fread(&ffidx, sizeof(uint32_t), 1, tablemap_func);
    if (fidx != ffidx) {
        perror("tablemap_funcがおかしい\n");
        exit(1);
    }
    fread(&tablemap_offset_addr, sizeof(uint64_t), 1, tablemap_func);

    /// tablemap_offset
    fseek(tablemap_offset, tablemap_offset_addr, SEEK_SET);
    // 関数fidxのローカルを取得
    uint32_t locals_size;
    fread(&locals_size, sizeof(uint32_t), 1, tablemap_offset);
    uint8_t locals[locals_size];
    fread(locals, sizeof(uint8_t), locals_size, tablemap_offset);
    // 対応するoffsetまで移動
    uint32_t ooffset;
    uint64_t type_table_addr, pre_type_table_addr;
    while(!feof(tablemap_offset)) {
       fread(&ooffset, sizeof(uint32_t), 1, tablemap_offset); 
       fread(&type_table_addr, sizeof(uint64_t), 1, tablemap_offset); 
       if (offset == ooffset) break;
       pre_type_table_addr = type_table_addr;
    }
    if (feof(tablemap_offset)) {
        perror("tablemap_offsetがおかしい\n");
        exit(1);
    }
    // type_table_addr = pre_type_table_addr;

    /// type_table
    fseek(type_table, type_table_addr, SEEK_SET);
    uint32_t stack_size;
    fread(&stack_size, sizeof(uint32_t), 1, type_table);
    uint8_t stack[stack_size];
    fread(stack, sizeof(uint8_t), stack_size, type_table);

    if (is_return_address) {
        fread(&stack_size, sizeof(uint32_t), 1, type_table);
        fread(stack, sizeof(uint8_t), stack_size, type_table);
    }

    // uint8 type_stack[locals_size + stack_size];
    uint8_t* type_stack = (uint8_t *)malloc(locals_size + stack_size);
    for (uint32_t i = 0; i < locals_size; ++i) type_stack[i] = locals[i];
    for (uint32_t i = 0; i < stack_size; ++i) type_stack[locals_size + i] = stack[i];

    // printf("new type stack: [");
    // for (uint32 i = 0; i < locals_size + stack_size; ++i) {
    //     if (i+1 == locals_size + stack_size)printf("%d", type_stack[i]);
    //     else                                printf("%d, ", type_stack[i]);
    // }
    // printf("]\n");

    fclose(tablemap_func);
    fclose(tablemap_offset);
    fclose(type_table);

    *type_stack_size = locals_size + stack_size;
    return type_stack;
}

Array8 get_type_stack_v2(uint32_t fidx, uint32_t offset, bool is_return_address) {
    // ローカル変数の型を取得
    Array8 local_types = wcrn_get_local_types(fidx);

    // スタックテーブルの取得
    StackTable table = wcrn_get_stack_table(fidx, offset);
    uint32_t effective_stack_size = table.size;

    // 戻りアドレスなら、最後の命令の戻り型を削る
    if (is_return_address && table.size > 0) {
        StackTableEntry last_entry = table.data[table.size - 1];
        if (last_entry.opcode == Opcode::WASMIG_Call) {
            effective_stack_size -= last_entry.operand.call_result_type;
        }
    }

    // 結合後の型スタック全体のサイズを計算
    uint32_t total_size = local_types.size + effective_stack_size;

    // 配列を確保
    uint8_t* type_stack = (uint8_t*)malloc(total_size * sizeof(uint8_t));

    // ローカル型をコピー
    memcpy(type_stack, local_types.contents, local_types.size * sizeof(uint8_t));

    // スタック型をコピー
    for (uint32_t i = 0; i < effective_stack_size; ++i) {
        type_stack[local_types.size + i] = table.data[i].ty;
    }

    return (Array8){ .size = total_size, .contents = type_stack };
}

Array8 get_type_stack_v3(uint32_t fidx, uint32_t offset, bool is_return_address) {
    // スタックテーブルの取得
    StackTable table = wcrn_get_stack_table(fidx, offset);
    uint32_t stack_size = table.size;

    // 戻りアドレスなら、最後の命令の戻り型を削る
    if (is_return_address && table.size > 0) {
        StackTableEntry last_entry = table.data[table.size - 1];
        if (last_entry.opcode == Opcode::WASMIG_Call) {
            stack_size -= last_entry.operand.call_result_type;
        }
    }

    // 配列を確保
    uint8_t* type_stack = (uint8_t*)malloc(stack_size * sizeof(uint8_t));

    return (Array8){ .size = stack_size, .contents = type_stack };
}


// TODO: ポインタわたしじゃなく、参照渡しにする
int checkpoint_stack(uint32_t call_stack_id, uint32_t entry_fidx, 
    CodePos *cur_addr, CodePos *ret_addr,Array32 *locals, Array32 *value_stack, LabelStack *label_stack, bool is_top) {
    char file[32];
    // TODO: stack_%d.imgに変更する
    snprintf(file, sizeof(file), "stack%d.img", call_stack_id);

    spdlog::info("entry_fidx: {}", entry_fidx);
    spdlog::info("cur_addr: (fidx={}, offset={})", cur_addr->fidx, cur_addr->offset);
    spdlog::info("ret_addr: (fidx={}, offset={})", ret_addr->fidx, ret_addr->offset);

    FILE *fp = open_image(file, "wb");
    if (fp == NULL) {
        return -1;
    }
    fwrite(&entry_fidx, sizeof(uint32_t), 1, fp);

    fwrite(&ret_addr->fidx, sizeof(uint32_t), 1, fp);
    fwrite(&ret_addr->offset, sizeof(uint32_t), 1, fp);

    // 型スタック
    Array8 type_stack = get_type_stack_v2(cur_addr->fidx, cur_addr->offset, !is_top);
    fwrite(&type_stack.size, sizeof(uint32_t), 1, fp);
    fwrite(type_stack.contents, sizeof(uint8_t), type_stack.size, fp);
    
    // debug print type stack
    print_type_stack(type_stack.contents, type_stack.size);
    print_locals(*cur_addr, &type_stack, locals);
    print_stack(*cur_addr, &type_stack, value_stack);

    // 値スタック
    fwrite(locals->contents, sizeof(uint32_t), locals->size, fp);
    fwrite(value_stack->contents, sizeof(uint32_t), value_stack->size, fp);

    // 制御スタック
    fwrite(&label_stack->size, sizeof(uint32_t), 1, fp);    // CodePos poses[cs_size];
    // Array32 locals[cs_size];
    // Array32 stack[cs_size];
    // LabelStack labels_stack[cs_size];
    for (int i = 0; i < label_stack->size; ++i) {
        // uint8 *begin_addr;
        // label_stack->begins[i] = get_addr_offset(csp->begin_addr, ip_start);
        fwrite(&label_stack->begins[i], sizeof(uint32_t), 1, fp);

        // uint8 *target_addr;
        // targets[i] = get_addr_offset(csp->target_addr, ip_start);
        fwrite(&label_stack->targets[i], sizeof(uint32_t), 1, fp);

        // uint32 *frame_sp;
        // stack_pointers[i] = get_addr_offset(csp->frame_sp, frame->sp_bottom);
        fwrite(&label_stack->stack_pointers[i], sizeof(uint32_t), 1, fp);

        // uint32 cell_num;
        // cell_nums[i] = csp->cell_num;
        fwrite(&label_stack->cell_nums[i], sizeof(uint32_t), 1, fp);
    }

    fclose(fp);

    return 0;
}

int checkpoint_call_stack_size(uint32_t call_stack_size) {
    FILE *fp = open_image("frame.img", "wb");
    if (fp == NULL) {
        return -1;
    }
    fwrite(&call_stack_size, sizeof(uint32_t), 1, fp);
    fclose(fp);
    return 0;
}



int checkpoint_stack_v3(size_t size, BaseCallStackEntry *call_stack) {
    // checkpoint call stack size
    CallStackEntry entry[size];
    for (int i = 0; i < size; ++i) {
        CodePos *cur_pos = &call_stack[i].pc;
        Array32 *locals = &call_stack[i].locals;
        Array32 *value_stack = &call_stack[i].value_stack;
        spdlog::debug("checkpoint stack: (fidx={}, offset={})", cur_pos->fidx, cur_pos->offset);

        // 型スタック
        Array8 locals_types = wcrn_get_local_types(cur_pos->fidx);
        Array8 stack_types = get_type_stack_v3(cur_pos->fidx, cur_pos->offset, (i == size-1));
        
        // 型スタックをもとに、localsとvalues stackのsizeを計算
        uint32_t locals_bytes = 0, stack_bytes = 0;
        for (int i = 0; i < locals_types.size; ++i) locals_bytes += locals_types.contents[i];
        for (int i = 0; i < stack_types.size; ++i) stack_bytes += stack_types.contents[i];
        // typesはu32型が1と表現されるので4倍
        locals->size = locals_bytes * sizeof(uint32_t);
        value_stack->size = stack_bytes * sizeof(uint32_t);
        
        TypedArray locals_typed_array = { .types = locals_types, .values = *locals };
        TypedArray value_stack_typed_array = { .types = stack_types, .values = *value_stack };
        
        // CallStackEntryに変換
        entry[i].pc = call_stack[i].pc;
        entry[i].locals = locals_typed_array;
        entry[i].value_stack = value_stack_typed_array;
        entry[i].label_stack = call_stack[i].label_stack;
    }
    
    // checkpoint call stack
    CallStack cs = { .size = size, .entries = entry };
    print_call_stack(&cs);
    spdlog::info("print call stack");
    Array8 serialized_call_stack = serialize_call_stack(&cs);
    FILE *fp = open_image("call_stack.img", "wb");
    spdlog::info("open call stack file");
    if (fp == NULL) {
        return -1;
    }
    uint32_t len = serialized_call_stack.size;
    uint8_t *buf = serialized_call_stack.contents;
    fwrite(buf, 1, len, fp);
    spdlog::info("write call stack file");
    
    free(buf);
    fclose(fp);
    
            
    return 0;
}

// int checkpoint_stack_v3(CallStack cs) {
//     Array8 serialized_call_stack = serialize_call_stack(&cs);
//     FILE *fp = open_image("call_stack.img", "wb");
//     if (fp == NULL) {
//         return -1;
//     }
//     uint32_t len = serialized_call_stack.size;
//     uint8_t *buf = serialized_call_stack.contents;
//     fwrite(buf, sizeof(uint8_t), len, fp);
//     // TODO: free cs
// }

}