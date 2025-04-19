#include "wasmig/state.h"

#ifdef __cplusplus
extern "C" {
#endif

CodePos prev_pc(CodePos pc);
CodePos next_pc(CodePos pc);

#ifdef __cplusplus
}
#endif