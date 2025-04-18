#ifndef WASMIG_UTILS_H
#define WASMIG_UTILS_H

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static const uint32_t WASM_PAGE_SIZE = 0x10000;

FILE* open_image(const char* file, const char* flag);
int is_page_dirty(uint64_t pagemap_entry);
int is_page_soft_dirty(uint64_t pagemap_entry);


#endif