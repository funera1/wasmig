#include <wasmig/utils.h>
#include <wasmig/migration.h>
#include <spdlog/spdlog.h>

namespace {

bool get_file_size(FILE* fp, size_t* out_size) {
    if (fseek(fp, 0, SEEK_END) != 0) {
        return false;
    }
    long size = ftell(fp);
    if (size < 0) {
        return false;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        return false;
    }
    *out_size = static_cast<size_t>(size);
    return true;
}

bool restore_sparse_memory(uint8_t* memory, uint32_t page_count, FILE* memory_fp) {
    while (true) {
        uint32_t page_idx = 0;
        size_t index_read = fread(&page_idx, sizeof(uint32_t), 1, memory_fp);
        if (index_read == 0) {
            return feof(memory_fp) != 0;
        }
        if (index_read != 1) {
            return false;
        }
        if (page_idx >= page_count) {
            spdlog::error("memory image contains out-of-range page index {}", page_idx);
            return false;
        }

        uint8_t* page = memory + (static_cast<size_t>(page_idx) * WASM_PAGE_SIZE);
        if (fread(page, sizeof(uint8_t), WASM_PAGE_SIZE, memory_fp) != WASM_PAGE_SIZE) {
            return false;
        }
    }
}

}

void restore_dirty_memory(uint8_t *memory, FILE* memory_fp) {
    const int PAGE_SIZE = 4096;
    while (!feof(memory_fp)) {
        if (feof(memory_fp)) break;
        uint32_t offset;
        uint32_t len;
        len = fread(&offset, sizeof(uint32_t), 1, memory_fp);
        if (len == 0) continue;
        len = fread(memory, PAGE_SIZE, 1, memory_fp);
    }
}

Array8 wasmig_restore_memory() {
    // FILE *mem_fp = open_image("memory.img", "wb");
    FILE* memory_fp = open_image("memory.img", "rb");
    FILE* mem_size_fp = open_image("mem_page_count.img", "rb");

    // check file pointers
    if (!memory_fp || !mem_size_fp) {
        spdlog::error("failed to open memory file");
        if (memory_fp) fclose(memory_fp);
        if (mem_size_fp) fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    // restore page_count
    uint32_t page_count;
    if (fread(&page_count, sizeof(uint32_t), 1, mem_size_fp) != 1) {
        spdlog::error("failed to read page_count");
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    // overflow check: WASM_PAGE_SIZE * page_count
    size_t total_size;
    if (__builtin_mul_overflow((size_t)WASM_PAGE_SIZE, (size_t)page_count, &total_size)) {
        spdlog::error("memory size overflow: page_count={}", page_count);
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    // allocate memory
    uint8_t* memory = (uint8_t*)calloc(1, total_size);
    if (!memory) {
        spdlog::error("calloc failed for {} bytes", total_size);
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    size_t file_size = 0;
    if (!get_file_size(memory_fp, &file_size)) {
        spdlog::error("failed to inspect memory image size");
        free(memory);
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    bool restored = false;
    if (file_size == total_size) {
        size_t read_count = fread(memory, sizeof(uint8_t), total_size, memory_fp);
        restored = (read_count == total_size);
        if (!restored) {
            spdlog::error("failed to read dense memory image: expected {} bytes, got {}", total_size, read_count);
        }
    } else {
        restored = restore_sparse_memory(memory, page_count, memory_fp);
        if (!restored) {
            spdlog::error("failed to read sparse memory image");
        }
    }

    if (!restored) {
        free(memory);
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    fclose(memory_fp);
    fclose(mem_size_fp);

    return (Array8){.size = total_size, .contents = memory};
}

CodePos wasmig_restore_pc() {
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

Array64 wasmig_restore_global(Array8 types) {
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

TypedArray wasmig_restore_global_v2() {
    FILE *fp = open_image("global.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open global img file");
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
    TypedArray globals = deserialize_typed_array(&array8);
    free(buf);
    return globals;
}

CallStack wasmig_restore_stack() {
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
