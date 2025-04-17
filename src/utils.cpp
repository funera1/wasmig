#include "wasmig/utils.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <spdlog/spdlog.h>

FILE* open_image(const char* file, const char* flag) {
    FILE *fp = fopen(file, flag);
    if (fp == NULL) {
        spdlog::error("faield to open file: {}", file);
        return NULL;
    }
    return fp;
}

int is_page_dirty(uint64_t pagemap_entry) {
    return (pagemap_entry>>62&1) | (pagemap_entry>>63&1);
}

int is_page_soft_dirty(uint64_t pagemap_entry) {
    return (pagemap_entry >> 55 & 1);
}
