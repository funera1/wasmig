#include "table.h"

int t_alloc(uint32_t func_idx) {
    // gtable[func_idx]がすでに確保されている
    if (func_idx < gtable.size()) {
        return 0;
    }

    // 不足分を確保する
    std::vector<table> newmaps(func_idx-gtable.size());
    gtable.insert(gtable.end(), newmaps.begin(), newmaps.end());
    return 0;
}

int t_detroy() {
    // gtableのメモリを開放する
    std::vector<table>().swap(gtable);
    return 0;
}