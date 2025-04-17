// #include <wasmig/utils.h>
// #include <wasmig/migration.h>
// #include <spdlog/spdlog.h>

// void restore_dirty_memory(uint8_t *memory, FILE* memory_fp) {
//     const int PAGE_SIZE = 4096;
//     while (!feof(memory_fp)) {
//         if (feof(memory_fp)) break;
//         uint32_t offset;
//         uint32_t len;
//         len = fread(&offset, sizeof(uint32_t), 1, memory_fp);
//         if (len == 0) break;
//         len = fread(memory, PAGE_SIZE, 1, memory_fp);
//     }
// }

// Array8 restore_memory() {
//     // FILE *mem_fp = open_image("memory.img", "wb");
//     FILE* memory_fp = open_image("memory.img", "rb");
//     FILE* mem_size_fp = open_image("mem_page_count.img", "rb");
//     if (mem_size_fp == NULL || memory_fp == NULL) {
//         spdlog::error("failed to open memory file");
//         return {0, NULL};
//     }
//     // restore page_count
//     uint32_t page_count;
//     fread(&page_count, sizeof(uint32_t), 1, mem_size_fp);
    
//     // restore memory
//     uint8_t* memory = (uint8_t*)malloc(WASM_PAGE_SIZE * page_count);
//     restore_dirty_memory(memory, memory_fp);
    
//     return Array8{.size = WASM_PAGE_SIZE*page_count, .contents = memory};
// }

// CodePos restore_pc() {
//     FILE *fp = open_image("program_counter.img", "rb");
//     if (fp == NULL) {
//         spdlog::error("failed to open program counter file");
//         return {0, 0};
//     }
//     CodePos pc;
//     fread(&pc.fidx, sizeof(uint32_t), 1, fp);
//     fread(&pc.offset, sizeof(uint32_t), 1, fp);
//     fclose(fp);
//     return pc;
// }

// Array64 restore_global(Array8 types) {
//     FILE *fp = open_image("global.img", "rb");
//     if (fp == NULL) {
//         spdlog::error("failed to open global file");
//         return {0, NULL};
//     }
//     Array64 globals;
//     globals.size = types.size;
//     globals.contents = (uint64_t*)malloc(sizeof(uint64_t) * globals.size);
    
//     for (int i = 0; i < globals.size; ++i) {
//         fread(&globals.contents[i], types.contents[i], 1, fp);
//     }
//     fclose(fp);
//     return globals;
// }

// CallStack restore_stack() {
//     FILE *fp = open_image("frame.img", "rb");
//     if (fp == NULL) {
//         spdlog::error("failed to open frame file");
//         return {0, NULL};
//     }
//     uint32_t call_stack_size;
//     fread(&call_stack_size, sizeof(uint32_t), 1, fp);
//     fclose(fp);
    
//     CallStack call_stack;
//     call_stack.size = call_stack_size;
//     call_stack.entries = (CallStackEntry*)malloc(sizeof(CallStackEntry) * call_stack_size);
    
//     for (int i = 0; i < call_stack_size; ++i) {
//         char file[32];
//         // TODO: stack_%d.imgに変更する
//         snprintf(file, sizeof(file), "stack%d.img", i+1);
//         FILE *fp = open_image(file, "rb");
//         if (fp == NULL) {
//             spdlog::error("failed to open frame file {}", file);
//             return {0, NULL};
//         }

//         // entry_fidx
//         uint32_t entry_fidx;
//         fread(&entry_fidx, sizeof(uint32_t), 1, fp);

//         // return address
//         CodePos ret_pos;
//         fread(&ret_pos.fidx, sizeof(uint32_t), 1, fp);
//         fread(&ret_pos.offset, sizeof(uint32_t), 1, fp);
        
//         // 型スタック
//         Array8 type_stack;
//         fread(&type_stack.size, sizeof(uint32_t), 1, fp);
//         type_stack.contents = (uint8_t*)malloc(sizeof(uint8_t) * type_stack.size);
//         fread(type_stack.contents, sizeof(uint8_t), type_stack.size, fp);

//         // ローカル&値スタック
//         Array32 locals, value_stack;
//         fread(&locals.size, sizeof(uint32_t), 1, fp);
//         locals.contents = (uint32_t*)malloc(sizeof(uint32_t) * locals.size);
//         fread(locals->contents, sizeof(uint32_t), locals->size, fp);

//         fread(&value_stack->size, sizeof(uint32_t), 1, fp);
//         value_stack->contents = (uint32_t*)malloc(sizeof(uint32_t) * value_stack->size);
//         fread(value_stack->contents, sizeof(uint32_t), value_stack->size, fp);

//         fread(&label_stack->size, sizeof(uint32_t), 1, fp);
//         label_stack->begins = (uint32_t*)malloc(sizeof(uint32_t) * label_stack->size);
//         label_stack->targets = (uint32_t*)malloc(sizeof(uint32_t) * label_stack->size);
//         label_stack->stack_pointers = (uint32_t*)malloc(sizeof(uint32_t) * label_stack->size);
//         label_stack->cell_nums = (uint32_t*)malloc(sizeof(uint32_t) * label_stack->size);
//         for (int i = 0; i < label_stack->size; ++i) {
//             fread(label_stack->begins, sizeof(uint32_t), 1, fp);
//             fread(label_stack->targets, sizeof(uint32_t), 1, fp);
//             fread(label_stack->stack_pointers, sizeof(uint32_t), 1, fp);
//             fread(label_stack->cell_nums, sizeof(uint32_t), 1, fp);
//         }
//     }
//     return call_stack; 
// }