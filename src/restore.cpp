#include <wasmig/utils.h>
#include <wasmig/migration.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ctime>
#include <vector>

namespace {

constexpr uint32_t kMemoryChunkSize = 4096;
constexpr uint32_t kSparseMemoryFormatV1 = 0x4d534731;
constexpr uint32_t kSparseMemoryFormatV2 = 0x4d534732;

bool get_file_size(FILE* fp, size_t* out_size) {
    if (fseek(fp, 0, SEEK_END) != 0) {
        return false;
    }
    long size = ftell(fp);
    if (size < 0) {
        return false;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        return false;
    }
    *out_size = static_cast<size_t>(size);
    return true;
}

uint64_t monotonic_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull +
           static_cast<uint64_t>(ts.tv_nsec);
}

bool inspect_memory_image(FILE* mem_size_fp, WasmigMemoryImageInfo* out_info) {
    size_t mem_size_file_size = 0;
    if (!get_file_size(mem_size_fp, &mem_size_file_size)) {
        spdlog::error("failed to inspect mem_page_count image size");
        return false;
    }

    WasmigMemoryImageInfo info = {};
    info.chunk_size = kMemoryChunkSize;

    if (mem_size_file_size == sizeof(uint32_t)) {
        if (fread(&info.page_count, sizeof(uint32_t), 1, mem_size_fp) != 1) {
            spdlog::error("failed to read legacy page_count");
            return false;
        }
    } else if (mem_size_file_size == sizeof(uint32_t) * 2) {
        if (fread(&info.format, sizeof(uint32_t), 1, mem_size_fp) != 1 ||
            fread(&info.page_count, sizeof(uint32_t), 1, mem_size_fp) != 1) {
            spdlog::error("failed to read sparse memory metadata");
            return false;
        }
        if (info.format != kSparseMemoryFormatV1 &&
            info.format != kSparseMemoryFormatV2) {
            spdlog::error("unknown memory image format marker: {}", info.format);
            return false;
        }
        info.sparse_format = true;
    } else {
        spdlog::error("unexpected mem_page_count image size: {}", mem_size_file_size);
        return false;
    }

    *out_info = info;
    return true;
}

}

void restore_dirty_memory(uint8_t *memory, FILE* memory_fp) {
    const int PAGE_SIZE = 4096;
    while (!feof(memory_fp)) {
        if (feof(memory_fp)) break;
        uint32_t offset;
        uint32_t len;
        len = fread(&offset, sizeof(uint32_t), 1, memory_fp);
        if (len == 0) continue;
        len = fread(memory, PAGE_SIZE, 1, memory_fp);
    }
}

int wasmig_memory_image_info(WasmigMemoryImageInfo *out_info) {
    if (!out_info) {
        spdlog::error("memory image info output is null");
        return -1;
    }

    FILE* mem_size_fp = open_image("mem_page_count.img", "rb");
    if (!mem_size_fp) {
        spdlog::error("failed to open memory metadata file");
        return -1;
    }

    WasmigMemoryImageInfo info = {};
    const bool ok = inspect_memory_image(mem_size_fp, &info);
    fclose(mem_size_fp);
    if (!ok) {
        return -1;
    }

    *out_info = info;
    return 0;
}

int wasmig_visit_memory_chunks(wasmig_memory_chunk_visitor_t visitor, void *user_data) {
    const uint64_t total_start_ns = monotonic_ns();
    if (!visitor) {
        spdlog::error("memory chunk visitor is null");
        return -1;
    }

    const uint64_t open_start_ns = monotonic_ns();
    FILE* memory_fp = open_image("memory.img", "rb");
    if (!memory_fp) {
        spdlog::error("failed to open memory file");
        return -1;
    }
    const uint64_t open_end_ns = monotonic_ns();

    const uint64_t info_start_ns = monotonic_ns();
    WasmigMemoryImageInfo info = {};
    if (wasmig_memory_image_info(&info) != 0) {
        fclose(memory_fp);
        return -1;
    }
    const uint64_t info_end_ns = monotonic_ns();

    const uint64_t size_start_ns = monotonic_ns();
    size_t total_size = 0;
    if (__builtin_mul_overflow((size_t)WASM_PAGE_SIZE, (size_t)info.page_count, &total_size)) {
        spdlog::error("memory size overflow: page_count={}", info.page_count);
        fclose(memory_fp);
        return -1;
    }

    size_t file_size = 0;
    if (!get_file_size(memory_fp, &file_size)) {
        spdlog::error("failed to inspect memory image size");
        fclose(memory_fp);
        return -1;
    }
    const uint64_t size_end_ns = monotonic_ns();

    uint64_t read_ns = 0;
    uint64_t visitor_ns = 0;
    uint32_t records = 0;

    if (!info.sparse_format) {
        if (file_size != total_size) {
            spdlog::error("legacy dense memory image size mismatch: expected {} bytes, got {}", total_size, file_size);
            fclose(memory_fp);
            return -1;
        }

        std::vector<uint8_t> chunk(info.chunk_size);
        for (size_t offset = 0; offset < total_size; offset += info.chunk_size) {
            const size_t len = std::min<size_t>(info.chunk_size, total_size - offset);
            const uint64_t read_start_ns = monotonic_ns();
            if (fread(chunk.data(), sizeof(uint8_t), len, memory_fp) != len) {
                spdlog::error("failed to read dense memory chunk at offset {}", offset);
                fclose(memory_fp);
                return -1;
            }
            read_ns += monotonic_ns() - read_start_ns;
            const uint64_t visitor_start_ns = monotonic_ns();
            if (visitor(static_cast<uint32_t>(offset), chunk.data(), static_cast<uint32_t>(len), user_data) != 0) {
                fclose(memory_fp);
                return -1;
            }
            visitor_ns += monotonic_ns() - visitor_start_ns;
            records += 1;
        }
        const uint64_t total_end_ns = monotonic_ns();
        fprintf(stderr,
                "restore_memory_visit_open, %lu\n"
                "restore_memory_visit_metadata, %lu\n"
                "restore_memory_visit_size_check, %lu\n"
                "restore_memory_visit_read, %lu\n"
                "restore_memory_visit_callback, %lu\n"
                "restore_memory_visit_records, %u\n"
                "restore_memory_visit_total, %lu\n",
                (unsigned long)(open_end_ns - open_start_ns),
                (unsigned long)(info_end_ns - info_start_ns),
                (unsigned long)(size_end_ns - size_start_ns),
                (unsigned long)read_ns,
                (unsigned long)visitor_ns,
                records,
                (unsigned long)(total_end_ns - total_start_ns));
        fclose(memory_fp);
        return 0;
    }

    const uint64_t chunk_capacity = (static_cast<uint64_t>(info.page_count) * WASM_PAGE_SIZE) / info.chunk_size;
    std::vector<uint8_t> chunk(info.chunk_size);
    while (true) {
        uint32_t chunk_index = 0;
        size_t index_read = fread(&chunk_index, sizeof(uint32_t), 1, memory_fp);
        if (index_read == 0) {
            const int eof = feof(memory_fp);
            const int err = ferror(memory_fp);
            if (!eof) {
                spdlog::error("stopped reading sparse memory chunk index without EOF: ferror={}", err);
            }
            const uint64_t total_end_ns = monotonic_ns();
            fprintf(stderr,
                    "restore_memory_visit_open, %lu\n"
                    "restore_memory_visit_metadata, %lu\n"
                    "restore_memory_visit_size_check, %lu\n"
                    "restore_memory_visit_read, %lu\n"
                    "restore_memory_visit_callback, %lu\n"
                    "restore_memory_visit_records, %u\n"
                    "restore_memory_visit_total, %lu\n",
                    (unsigned long)(open_end_ns - open_start_ns),
                    (unsigned long)(info_end_ns - info_start_ns),
                    (unsigned long)(size_end_ns - size_start_ns),
                    (unsigned long)read_ns,
                    (unsigned long)visitor_ns,
                    records,
                    (unsigned long)(total_end_ns - total_start_ns));
            fclose(memory_fp);
            return eof != 0 ? 0 : -1;
        }
        if (index_read != 1) {
            spdlog::error("failed to read sparse memory chunk index");
            fclose(memory_fp);
            return -1;
        }
        if (info.format == kSparseMemoryFormatV1) {
            if (chunk_index >= chunk_capacity) {
                spdlog::error("memory image contains out-of-range chunk index {}", chunk_index);
                fclose(memory_fp);
                return -1;
            }
            const uint64_t read_start_ns = monotonic_ns();
            if (fread(chunk.data(), sizeof(uint8_t), info.chunk_size, memory_fp) != info.chunk_size) {
                spdlog::error("failed to read sparse memory chunk {}", chunk_index);
                fclose(memory_fp);
                return -1;
            }
            read_ns += monotonic_ns() - read_start_ns;
            const uint64_t visitor_start_ns = monotonic_ns();
            if (visitor(chunk_index * info.chunk_size, chunk.data(), info.chunk_size, user_data) != 0) {
                spdlog::error("memory chunk visitor failed at chunk_index={} offset={} len={}",
                              chunk_index, chunk_index * info.chunk_size, info.chunk_size);
                fclose(memory_fp);
                return -1;
            }
            visitor_ns += monotonic_ns() - visitor_start_ns;
            records += 1;
            continue;
        }

        uint32_t extent_chunk_count = 0;
        const uint64_t read_header_start_ns = monotonic_ns();
        if (fread(&extent_chunk_count, sizeof(uint32_t), 1, memory_fp) != 1) {
            spdlog::error("failed to read sparse memory extent length for start_chunk={}", chunk_index);
            fclose(memory_fp);
            return -1;
        }
        read_ns += monotonic_ns() - read_header_start_ns;
        if (extent_chunk_count == 0 ||
            static_cast<uint64_t>(chunk_index) + extent_chunk_count > chunk_capacity) {
            spdlog::error("memory image contains invalid extent start_chunk={} chunk_count={}",
                          chunk_index, extent_chunk_count);
            fclose(memory_fp);
            return -1;
        }

        size_t extent_size = 0;
        if (__builtin_mul_overflow(static_cast<size_t>(extent_chunk_count),
                                   static_cast<size_t>(info.chunk_size), &extent_size)) {
            spdlog::error("memory extent size overflow start_chunk={} chunk_count={}",
                          chunk_index, extent_chunk_count);
            fclose(memory_fp);
            return -1;
        }
        std::vector<uint8_t> extent(extent_size);
        const uint64_t read_body_start_ns = monotonic_ns();
        if (fread(extent.data(), sizeof(uint8_t), extent_size, memory_fp) != extent_size) {
            spdlog::error("failed to read sparse memory extent start_chunk={} chunk_count={}",
                          chunk_index, extent_chunk_count);
            fclose(memory_fp);
            return -1;
        }
        read_ns += monotonic_ns() - read_body_start_ns;
        const uint64_t visitor_start_ns = monotonic_ns();
        if (visitor(chunk_index * info.chunk_size, extent.data(),
                    static_cast<uint32_t>(extent_size), user_data) != 0) {
            spdlog::error("memory extent visitor failed at start_chunk={} offset={} len={}",
                          chunk_index, chunk_index * info.chunk_size, extent_size);
            fclose(memory_fp);
            return -1;
        }
        visitor_ns += monotonic_ns() - visitor_start_ns;
        records += 1;
    }
}

int wasmig_restore_memory_into(uint8_t *dst, size_t dst_size) {
    const uint64_t restore_start_ns = monotonic_ns();
    if (!dst && dst_size != 0) {
        spdlog::error("memory restore destination is null");
        return -1;
    }

    const uint64_t metadata_start_ns = monotonic_ns();
    WasmigMemoryImageInfo info = {};
    if (wasmig_memory_image_info(&info) != 0) {
        spdlog::error("failed to read memory image metadata before restore");
        return -1;
    }
    const uint64_t metadata_end_ns = monotonic_ns();

    const uint64_t size_check_start_ns = monotonic_ns();
    size_t total_size = 0;
    if (__builtin_mul_overflow((size_t)WASM_PAGE_SIZE, (size_t)info.page_count, &total_size)) {
        spdlog::error("memory size overflow: page_count={}", info.page_count);
        return -1;
    }
    if (dst_size < total_size) {
        spdlog::error("restore destination is too small: need {}, got {}", total_size, dst_size);
        return -1;
    }
    const uint64_t size_check_end_ns = monotonic_ns();

    struct RestoreState {
        uint8_t *dst;
        size_t dst_size;
        uint64_t bytes_copied;
        uint32_t callbacks;
        uint64_t memcpy_ns;
    } state = {dst, dst_size};

    auto visitor = [](uint32_t offset, const uint8_t *data, uint32_t len, void *user_data) -> int {
        RestoreState *state = static_cast<RestoreState *>(user_data);
        const size_t end_offset = static_cast<size_t>(offset) + static_cast<size_t>(len);
        if (end_offset > state->dst_size) {
            spdlog::error("restore chunk exceeds destination: offset={}, len={}, dst_size={}",
                          offset, len, state->dst_size);
            return -1;
        }
        const uint64_t memcpy_start_ns = monotonic_ns();
        memcpy(state->dst + offset, data, len);
        state->memcpy_ns += monotonic_ns() - memcpy_start_ns;
        state->bytes_copied += len;
        state->callbacks += 1;
        return 0;
    };

    const uint64_t visit_start_ns = monotonic_ns();
    const int rc = wasmig_visit_memory_chunks(visitor, &state);
    const uint64_t visit_end_ns = monotonic_ns();

    fprintf(stderr,
            "restore_memory_into_metadata, %lu\n"
            "restore_memory_into_size_check, %lu\n"
            "restore_memory_into_visit_total, %lu\n"
            "restore_memory_into_memcpy_only, %lu\n"
            "restore_memory_into_callbacks, %u\n"
            "restore_memory_into_bytes, %lu\n"
            "restore_memory_into_total, %lu\n",
            (unsigned long)(metadata_end_ns - metadata_start_ns),
            (unsigned long)(size_check_end_ns - size_check_start_ns),
            (unsigned long)(visit_end_ns - visit_start_ns),
            (unsigned long)state.memcpy_ns,
            state.callbacks,
            (unsigned long)state.bytes_copied,
            (unsigned long)(visit_end_ns - restore_start_ns));

    return rc;
}

Array8 wasmig_restore_memory() {
    // FILE *mem_fp = open_image("memory.img", "wb");
    FILE* memory_fp = open_image("memory.img", "rb");
    FILE* mem_size_fp = open_image("mem_page_count.img", "rb");

    // check file pointers
    if (!memory_fp || !mem_size_fp) {
        spdlog::error("failed to open memory file");
        if (memory_fp) fclose(memory_fp);
        if (mem_size_fp) fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    WasmigMemoryImageInfo info = {};
    if (!inspect_memory_image(mem_size_fp, &info)) {
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    // overflow check: WASM_PAGE_SIZE * page_count
    size_t total_size;
    if (__builtin_mul_overflow((size_t)WASM_PAGE_SIZE, (size_t)info.page_count, &total_size)) {
        spdlog::error("memory size overflow: page_count={}", info.page_count);
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    // allocate memory
    uint8_t* memory = (uint8_t*)calloc(1, total_size);
    if (!memory) {
        spdlog::error("calloc failed for {} bytes", total_size);
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    size_t file_size = 0;
    if (!get_file_size(memory_fp, &file_size)) {
        spdlog::error("failed to inspect memory image size");
        free(memory);
        fclose(memory_fp);
        fclose(mem_size_fp);
        return (Array8){0, NULL};
    }

    fclose(memory_fp);
    fclose(mem_size_fp);

    if (wasmig_restore_memory_into(memory, total_size) != 0) {
        free(memory);
        return (Array8){0, NULL};
    }

    return (Array8){.size = total_size, .contents = memory};
}

CodePos wasmig_restore_pc() {
    FILE *fp = open_image("program_counter.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open program counter file");
        return {0, 0};
    }
    CodePos pc;
    fread(&pc.fidx, sizeof(uint32_t), 1, fp);
    fread(&pc.offset, sizeof(uint32_t), 1, fp);
    fclose(fp);
    return pc;
}

Array64 wasmig_restore_global(Array8 types) {
    FILE *fp = open_image("global.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open global file");
        return {0, NULL};
    }
    Array64 globals;
    globals.size = types.size;
    globals.contents = (uint64_t*)malloc(sizeof(uint64_t) * globals.size);
    
    for (int i = 0; i < globals.size; ++i) {
        fread(&globals.contents[i], types.contents[i], 1, fp);
    }
    fclose(fp);
    return globals;
}

TypedArray wasmig_restore_global_v2() {
    FILE *fp = open_image("global.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open global img file");
        return {0, NULL};
    }
    // fpからデータを読み込む
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc(file_size);
    fread(buf, file_size, 1, fp);
    fclose(fp);
    // bufをデシリアライズする
    Array8 array8;
    array8.size = file_size;
    array8.contents = buf;
    TypedArray globals = deserialize_typed_array(&array8);
    free(buf);
    return globals;
}

CallStack wasmig_restore_stack() {
    FILE *fp = open_image("call_stack.img", "rb");
    if (fp == NULL) {
        spdlog::error("failed to open call stack file");
        return {0, NULL};
    }
    // fpからデータを読み込む
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc(file_size);
    fread(buf, file_size, 1, fp);
    fclose(fp);
    // bufをデシリアライズする
    Array8 array8;
    array8.size = file_size;
    array8.contents = buf;
    CallStack call_stack = deserialize_call_stack(&array8);
    free(buf);
    return call_stack;
}
