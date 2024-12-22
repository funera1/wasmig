#include "table.h"

// static inline int is_allocated(uint32_t func_idx) {
//     return (func_idx < gtable.size());
// }

// int tab_alloc(uint32_t func_idx) {
//     // gtable[func_idx]がすでに確保されている
//     if (is_allocated(func_idx)) {
//         return 0;
//     }

//     // 不足分を確保する
//     std::vector<table> newmaps(func_idx-gtable.size());
//     gtable.insert(gtable.end(), newmaps.begin(), newmaps.end());
//     return 0;
// }

// int tab_detroy() {
//     // gtableのメモリを開放する
//     std::vector<table>().swap(gtable);
//     return 0;
// }

int tab_set(uint32_t func_idx, uint64_t offset, uintptr_t address) {
    // gtable[func_idx]が確保されているかcheck
    // if (!is_allocated(func_idx)) {
    //     return 1;
    // }

    gtable[address] = CodePos(func_idx, offset);
    return 0;
}

CodePos tab_get(uintptr_t address) {
    return gtable[address];
}