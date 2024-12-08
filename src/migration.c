// src/example.c
#include "wasmig/migration.h"
#include <fcntl.h>

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

int is_dirty(uint64_t pagemap_entry) {
    return (pagemap_entry>>62&1) | (pagemap_entry>>63&1);
}

int is_soft_dirty(uint64_t pagemap_entry) {
    return (pagemap_entry >> 55 & 1);
}

int write_dirty_memory(uint8_t* memory, uint32_t cur_page) {
    const int PAGEMAP_LENGTH = 8;
    const int PAGE_SIZE = 4096;
    FILE *memory_fp = open_image("memory.img", "wb");
    int fd;
    uint64_t pagemap_entry;
    // プロセスのpagemapを開く
    fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd == -1) {
        perror("Error opening pagemap");
        return -1;
    }

    // pfnに対応するpagemapエントリを取得
    unsigned long pfn = (unsigned long)memory / PAGE_SIZE;
    off_t offset = sizeof(uint64_t) * pfn;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("Error seeking to pagemap entry");
        close(fd);
        return -1;
    }

    uint8_t* memory_data = memory;
    uint8_t* memory_data_end = memory + (cur_page * PAGE_SIZE);
    int i = 0;
    for (uint8_t* addr = memory; addr < memory_data_end; addr += PAGE_SIZE, ++i) {
        unsigned long pfn = (unsigned long)addr / PAGE_SIZE;
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
        // if (is_dirty(pagemap_entry)) {
        if (is_soft_dirty(pagemap_entry)) {
            // printf("[%x, %x]: dirty page\n", i*PAGE_SIZE, (i+1)*PAGE_SIZE);
            uint32_t offset = (uint64_t)addr - (uint64_t)memory_data;
            // printf("i: %d\n", offset);
            fwrite(&offset, sizeof(uint32_t), 1, memory_fp);
            fwrite(addr, PAGE_SIZE, 1, memory_fp);
        }
    }

    close(fd);
    fclose(memory_fp);
    return 0;
}

int checkpoint_memory(uint8_t* memory, uint32_t cur_page) {
    FILE *mem_size_fp = open_image("mem_page_count.img", "wb");

    write_dirty_memory(memory, cur_page);
    // fwrite(memory, sizeof(uint8_t), PAGE_SIZE * cur_page, memory_fp);
    fwrite(&cur_page, sizeof(uint32_t), 1, mem_size_fp);

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

int checkpoint_pc(uint32_t func_idx, uint32_t offset) {
    FILE *fp = open_image("program_counter.img", "wb");
    fwrite(&func_idx, sizeof(uint32_t), 1, fp);
    fwrite(&offset, sizeof(uint32_t), 1, fp);
    fclose(fp);
}