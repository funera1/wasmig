#ifndef WASMIG_LOG_H
#define WASMIG_LOG_H

#ifdef __cplusplus
extern "C" {
#endif
    
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} WasmigLogLevel;
    
void wasmig_log_init(WasmigLogLevel log_level);
void wasmig_info(const char* fmt, ...);
void wasmig_debug(const char* fmt, ...);
void wasmig_error(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif