// src/example.c
#include "wasmig/migration.h"

const uint32_t WASM_PAGE_SIZE = 0x10000;

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

int is_page_dirty(uint64_t pagemap_entry) {
    return (pagemap_entry>>62&1) | (pagemap_entry>>63&1);
}

int is_page_soft_dirty(uint64_t pagemap_entry) {
    return (pagemap_entry >> 55 & 1);
}

// TODO: バグってるので修正
int write_dirty_memory(uint8_t* memory, uint32_t cur_page) {
    const int PAGEMAP_LENGTH = 8;
    const int OS_PAGE_SIZE = 4096;
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
    // FILE *mem_fp = open_image("memory.img", "wb");
    FILE *mem_size_fp = open_image("mem_page_count.img", "wb");

    write_dirty_memory(memory, cur_page);
    // fwrite(memory, sizeof(uint8_t), WASM_PAGE_SIZE * cur_page, mem_fp);
    fwrite(&cur_page, sizeof(uint32_t), 1, mem_size_fp);

    // fclose(mem_fp);
    fclose(mem_size_fp);
}

int checkpoint_global(uint64_t* values, uint32_t* types, int len) {
    FILE *fp = open_image("global.img", "wb");

    for (int i = 0; i < len; i++) {
        fwrite(&values[i], types[i], 1, fp);
        // if (types[i] == 1) {
        //     fwrite(&values[i], sizeof(uint32_t), 1, fp);
        // }
        // else if (types[i] == 2) {
        //     fwrite(&values[i], sizeof(uint32_t), 2, fp);
        // }
        // else if (types[i] == 4) {
        //     fwrite(&values[i], sizeof(uint32_t), 4, fp);
        // }
        // else {
        //     printf("Error: invalid type\n");
        // }
    }

    fclose(fp);
}

int checkpoint_pc(uint32_t func_idx, uint32_t offset) {
    FILE *fp = open_image("program_counter.img", "wb");
    fwrite(&func_idx, sizeof(uint32_t), 1, fp);
    fwrite(&offset, sizeof(uint32_t), 1, fp);
    fclose(fp);
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
    uint8_t* type_stack = malloc(locals_size + stack_size);
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


int checkpoint_stack(uint32_t call_stack_id, uint32_t entry_fidx, 
    CodePos *ret_addr, CodePos *cur_addr, Array32 *locals, Array32 *value_stack, LabelStack *label_stack, bool is_top) {
    char file[32];
    sprintf(file, "stack%d.img", call_stack_id);

    FILE *fp = open_image(file, "wb");
    fwrite(&entry_fidx, sizeof(uint32_t), 1, fp);

    fwrite(&ret_addr->fidx, sizeof(uint32_t), 1, fp);
    fwrite(&ret_addr->offset, sizeof(uint32_t), 1, fp);

    // 型スタック
    uint32_t type_stack_size;
    uint8_t* type_stack = get_type_stack(cur_addr->fidx, cur_addr->offset, &type_stack_size, !is_top);
    fwrite(&type_stack_size, sizeof(uint32_t), 1, fp);
    fwrite(type_stack, sizeof(uint8_t), type_stack_size, fp);

    // 値スタック
    fwrite(locals->contents, sizeof(uint32_t), locals->size, fp);
    fwrite(value_stack->contents, sizeof(uint32_t), value_stack->size, fp);

    // 制御スタック
    fwrite(&label_stack->size, sizeof(uint32_t), 1, fp);
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
}

int checkpoint_call_stack_size(uint32_t call_stack_size) {
    FILE *fp = open_image("frame.img", "wb");
    fwrite(&call_stack_size, sizeof(uint32_t), 1, fp);
    fclose(fp);
}