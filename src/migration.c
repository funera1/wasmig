// src/example.c
#include "wasmig/migration.h"

const PAGE_SIZE = 64 * 1024;

void hello_world() {
    printf("Hello, World!\n");
}

static FILE* open_image(const char* file, const char* flag) {
    FILE *fp = fopen(file, flag);
    if (fp == NULL) {
        fprintf(stderr, "failed to open %s\n", file);
        return NULL;
    }
    return fp;
}

int checkpoint_memory(uint8_t* memory, uint32_t cur_page) {
    FILE *memory_fp = open_image("memory.img", "wb");
    FILE *mem_size_fp = open_image("mem_page_count.img", "wb");

    // WASMMemoryInstance *memory = module->default_memory;
    fwrite(memory, sizeof(uint8_t), PAGE_SIZE * cur_page, memory_fp);

    // printf("page_count: %d\n", memory->cur_page_count);
    fwrite(&cur_page, sizeof(uint32_t), 1, mem_size_fp);

    fclose(memory_fp);
    fclose(mem_size_fp);
}

int checkpoint_global(uint64_t* values, uint32_t* types, int len) {
    FILE *fp = open_image("global.img", "wb");

    for (int i = 0; i < len; i++) {
        if (types[i] == 1) {
            fwrite(&values[i], sizeof(uint32_t), 1, fp);
        }
        else if (types[i] == 2) {
            fwrite(&values[i], sizeof(uint32_t), 2, fp);
        }
        else if (types[i] == 4) {
            fwrite(&values[i], sizeof(uint32_t), 4, fp);
        }
        else {
            printf("Error: invalid type\n");
        }
    }

    fclose(fp);
}