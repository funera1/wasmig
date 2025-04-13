#include "wasmig/table.h"
#include <map>

static uint32_t cur_fidx, cur_offset;
static std::map<uintptr_t, CodePos> gtable;

extern "C" {
    int tab_set(uint32_t func_idx, uint64_t offset, uintptr_t address) {
        gtable[address] = CodePos{
            func_idx, 
            offset
        };
        return 0;
    }

    bool tab_get(uintptr_t address, CodePos *out) {
        // check exist
        auto it  = gtable.find(address);
        if (it == gtable.end()) {
            return false;
        }

        *out = it->second;
        return true;
    }

    int set_cur_fidx(uint32_t fidx) {
        cur_fidx = fidx;
        return 0;
    }

    uint32_t get_cur_fidx() {
        return cur_fidx;
    }

    int set_cur_offset(uint32_t offset) {
        cur_offset = offset;
        return 0;
    }

    uint32_t get_cur_offset() {
        return cur_offset;
    }
    
    void print_table() {
        for (auto it = gtable.begin(); it != gtable.end(); ++it) {
            printf("address: %p, fidx: %d, offset: %lu\n", it->first, it->second.fidx, it->second.offset);
        }
    }
}