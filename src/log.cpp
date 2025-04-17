#include "wasmig/log.h"
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdarg.h>

extern "C" {
    void wasmig_log_init(WasmigLogLevel log_level) {
        // TODO: log levelに応じてspdlogのログレベルを設定
        spdlog::set_level(spdlog::level::debug);
    } 

    void wasmig_info(const char* fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        spdlog::info("{}", buffer);
    }

    void wasmig_debug(const char* fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        spdlog::debug("{}", buffer);
    }

    void wasmig_error(const char* fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        spdlog::error("{}", buffer);
    }

}