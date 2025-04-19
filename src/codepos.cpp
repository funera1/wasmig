#include "wasmig/state.h"
#include "wasmig/stack_tables.h"
#include "wcrn.h"
#include <spdlog/spdlog.h>

extern "C" {
    CodePos prev_pc(CodePos pc) {
        Array32 offs = wcrn_offset_list(pc.fidx);
        for (int i = 0; i < offs.size; i++) {
            if (pc.offset == offs.contents[i]) {
                uint64_t offset;
                if (i > 0) offset = offs.contents[i-1];
                else offset = offs.contents[i];

                return CodePos {
                    .fidx = pc.fidx,
                    .offset = offset,
                };
            }
        }
        spdlog::warn("This pc is inccorrect");
        return pc;
    }
    CodePos next_pc(CodePos pc) {
        Array32 offs = wcrn_offset_list(pc.fidx);
        for (int i = 0; i < offs.size; i++) {
            if (pc.offset == offs.contents[i]) {
                uint64_t offset;
                if (i > 0) offset = offs.contents[i+1];
                else offset = offs.contents[i];

                return CodePos {
                    .fidx = pc.fidx,
                    .offset = offset,
                };
            }
        }
        spdlog::warn("This pc is inccorrect");
        return pc;
    }
}