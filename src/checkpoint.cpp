// src/example.c
#include "wasmig/migration.h"
#include "wasmig/stack_tables.h"
#include "wasmig/utils.h"
#include "wasmig/state.h"
#include "wasmig/stack.h"
#include "wasmig/registry.h"
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

int wasmig_checkpoint_memory(uint8_t* memory, uint32_t cur_page) {
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

int wasmig_checkpoint_global_v2(TypedArray globals) {
    FILE *fp = open_image("global.img", "wb");
    if (fp == NULL) {
        return -1;
    }
    
    // serialize globals
    Array8 serialized = serialize_typed_array(&globals);
    spdlog::info("serialize globals: size = {}", serialized.size);
    uint32_t len = serialized.size;
    uint8_t *buf = serialized.contents;
    fwrite(buf, 1, len, fp);
    spdlog::info("write globals");

    fclose(fp);
    return 0;
}

int wasmig_checkpoint_global(uint64_t* values, uint32_t* types, int len) {
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

int wasmig_checkpoint_pc(uint32_t func_idx, uint32_t offset) {
    FILE *fp = open_image("program_counter.img", "wb");
    if (fp == NULL) {
        return -1;
    }
    fwrite(&func_idx, sizeof(uint32_t), 1, fp);
    fwrite(&offset, sizeof(uint32_t), 1, fp);
    fclose(fp);

    return 0;
}

int wasmig_checkpoint_stack_v3(size_t size, BaseCallStackEntry *call_stack) {
    // checkpoint call stack size
    CallStackEntry entry[size];
    for (int i = 0; i < size; ++i) {
        CodePos *cur_pos = &call_stack[i].pc;
        Array32 *locals = &call_stack[i].locals;
        Array32 *value_stack = &call_stack[i].value_stack;
        spdlog::debug("checkpoint stack: (fidx={}, offset={})", cur_pos->fidx, cur_pos->offset);

        // 型スタック
        Array8 locals_types = get_local_types(cur_pos->fidx);
        uint32_t offset = (i == size-1) ? cur_pos->offset : cur_pos->offset - 1;
        StackTable stack_table = get_stack_table(cur_pos->fidx, offset);
        Array8 stack_types = convert_type_stack_from_stack_table(&stack_table);
        
        // 型スタックをもとに、localsとvalues stackのsizeを計算
        uint32_t locals_bytes = 0, stack_bytes = 0;
        for (int i = 0; i < locals_types.size; ++i) locals_bytes += locals_types.contents[i];
        for (int i = 0; i < stack_types.size; ++i) stack_bytes += stack_types.contents[i];
        locals->size = locals_bytes;
        value_stack->size = stack_bytes;
        spdlog::info("frame[{}] (fidx={}, offset={})", i, cur_pos->fidx, cur_pos->offset);
        spdlog::info("checkpoint stack: (locals_size={}, value_stack_size={})", locals_bytes, stack_bytes);

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

int wasmig_checkpoint_stack_v4(size_t size, CallStackEntry *call_stack) {
    // checkpoint call stack
    CallStack cs = { .size = size, .entries = call_stack };
    // print_call_stack(&cs);
    spdlog::debug("print call stack");
    Array8 serialized_call_stack = serialize_call_stack(&cs);
    FILE *fp = open_image("call_stack.img", "wb");
    spdlog::debug("open call stack file");
    if (fp == NULL) {
        return -1;
    }
    uint32_t len = serialized_call_stack.size;
    uint8_t *buf = serialized_call_stack.contents;
    fwrite(buf, 1, len, fp);
    spdlog::debug("write call stack file");
    
    free(buf);
    fclose(fp);
    
            
    return 0;
}

}