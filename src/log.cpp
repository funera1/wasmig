#include "wasmig/log.h"
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static bool log_initialized = false;

static void ensure_log_initialized() {
    if (!log_initialized) {
        // 環境変数WASMIG_LOG_LEVELから設定を読み取る
        const char* env_level = getenv("WASMIG_LOG_LEVEL");
        spdlog::level::level_enum level = spdlog::level::info; // デフォルト
        
        if (env_level) {
            if (strcmp(env_level, "debug") == 0 || strcmp(env_level, "DEBUG") == 0) {
                level = spdlog::level::debug;
            } else if (strcmp(env_level, "info") == 0 || strcmp(env_level, "INFO") == 0) {
                level = spdlog::level::info;
            } else if (strcmp(env_level, "warn") == 0 || strcmp(env_level, "WARN") == 0) {
                level = spdlog::level::warn;
            } else if (strcmp(env_level, "error") == 0 || strcmp(env_level, "ERROR") == 0) {
                level = spdlog::level::err;
            } else if (strcmp(env_level, "off") == 0 || strcmp(env_level, "OFF") == 0) {
                level = spdlog::level::off;
            }
        }
        
        spdlog::set_level(level);
        log_initialized = true;
    }
}

extern "C" {
    void wasmig_log_init(WasmigLogLevel log_level) {
        spdlog::level::level_enum spdlog_level;
        
        switch (log_level) {
            case LOG_LEVEL_DEBUG:
                spdlog_level = spdlog::level::debug;
                break;
            case LOG_LEVEL_INFO:
                spdlog_level = spdlog::level::info;
                break;
            case LOG_LEVEL_ERROR:
                spdlog_level = spdlog::level::err;
                break;
            default:
                spdlog_level = spdlog::level::info;
                break;
        }
        
        spdlog::set_level(spdlog_level);
        log_initialized = true;
    } 

    void wasmig_info(const char* fmt, ...) {
        ensure_log_initialized();
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        spdlog::info("{}", buffer);
    }

    void wasmig_debug(const char* fmt, ...) {
        ensure_log_initialized();
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        spdlog::debug("{}", buffer);
    }

    void wasmig_error(const char* fmt, ...) {
        ensure_log_initialized();
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        spdlog::error("{}", buffer);
    }

}