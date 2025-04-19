#include "wasmig/state.h"
#include "wasmig/stack_tables.h"
#include "wcrn.h"

extern "C" {
    CodePos prev(CodePos pc) {
        Array32 offs = wcrn_offset_list(pc.fidx);
        for (int i = 0; i < offs.size; i++) {
            if (pc.offset == offs[i]) {
                if (i > 0) offset = offs[i-1];
                else offset = offs[i];
                return CodePos {
                    .fidx: fidx
                    .offset: offset
                };
            }
        }
        spdlog::warn("This pc is inccorrect");
        return pc;
    }
    CodePos next(CodePos pc) {
        Array32 offs = wcrn_offset_list(pc.fidx);
        for (int i = 0; i < offs.size; i++) {
            if (pc.offset == offs[i]) {
                if (i > 0) offset = offs[i+1];
                else offset = offs[i];
                return CodePos {
                    .fidx: fidx
                    .offset: offset
                };
            }
        }
        spdlog::warn("This pc is inccorrect");
        return pc;
    }
}