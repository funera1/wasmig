#include <wasmig/utils.h>
#include <wasmig/migration.h>
#include <spdlog/spdlog.h>

void restore_dirty_memory(uint8_t *memory, FILE* memory_fp) {
    const int PAGE_SIZE = 4096;
    while (!feof(memory_fp)) {
        if (feof(memory_fp)) break;
        uint32_t offset;
        uint32_t len;
        len = fread(&offset, sizeof(uint32_t), 1, memory_fp);
        if (len == 0) break;
        len = fread(memory, PAGE_SIZE, 1, memory_fp);
    }
}

Array8 restore_memory() {
    // FILE *mem_fp = open_image("memory.img", "wb");
    FILE* memory_fp = open_image("memory.img", "rb");
    FILE* mem_size_fp = open_image("mem_page_count.img", "rb");
    if (mem_size_fp == NULL || memory_fp == NULL) {
        spdlog::error("failed to open memory file");
        return {0, NULL};
    }
    // restore page_count
    uint32_t page_count;
    fread(&page_count, sizeof(uint32_t), 1, mem_size_fp);
    
    // restore memory
    uint8_t* memory = (uint8_t*)malloc(WASM_PAGE_SIZE * page_count);
    restore_dirty_memory(memory, memory_fp);
    
    return Array8{.size = WASM_PAGE_SIZE*page_count, .contents = memory};
}

CodePos restore_pc() {
    FILE *fp = open_image("program_counter.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open program counter file");
        return {0, 0};
    }
    CodePos pc;
    fread(&pc.fidx, sizeof(uint32_t), 1, fp);
    fread(&pc.offset, sizeof(uint32_t), 1, fp);
    fclose(fp);
    return pc;
}

Array64 restore_global(Array8 types) {
    FILE *fp = open_image("global.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open global file");
        return {0, NULL};
    }
    Array64 globals;
    globals.size = types.size;
    globals.contents = (uint64_t*)malloc(sizeof(uint64_t) * globals.size);
    
    for (int i = 0; i < globals.size; ++i) {
        fread(&globals.contents[i], types.contents[i], 1, fp);
    }
    fclose(fp);
    return globals;
}

CallStack restore_stack() {
    FILE *fp = open_image("call_stack.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open call stack file");
        return {0, NULL};
    }
    // fpからデータを読み込む
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc(file_size);
    fread(buf, file_size, 1, fp);
    fclose(fp);
    // bufをデシリアライズする
    Array8 array8;
    array8.size = file_size;
    array8.contents = buf;
    CallStack call_stack = deserialize_call_stack(&array8);
    free(buf);
    return call_stack;
}