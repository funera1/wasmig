#include "wasmig/state.h"
#include "wasmig/utils.h"
#include "wasmig/proto/state.pb-c.h"
#include "spdlog/spdlog.h"
#include <unistd.h>
#include <cstdio>

int serialize_array32(FILE *fp, Array32 *array) {
    if (fp == NULL) {
        spdlog::error("failed to open array file");
        return -1;
    }

    // State__Array32 初期化
    State__Array32 *array_proto;
    array_proto = (State__Array32*)malloc(sizeof(State__Array32));
    state__array32__init(array_proto);
    spdlog::info("init array proto");
    
    // intialize array_proto
    array_proto->n_contents = array->size;
    array_proto->contents = (uint32_t*)malloc(sizeof(uint32_t) * array->size);
    memcpy(array_proto->contents, array->contents, sizeof(uint32_t) * array->size);

    // packed array32 and write to file
    size_t size = state__array32__get_packed_size(array_proto);
    uint8_t *buf = (uint8_t*)malloc(size);
    size_t len = state__array32__pack(array_proto, buf);
    spdlog::info("packed array size: {}", len);
    fwrite(buf, sizeof(uint8_t), len, fp);
    spdlog::info("write array to file");
    
    // free memory
    free(buf);
    spdlog::info("free buf");
    free(array_proto->contents);
    spdlog::info("free array contents");
    free(array_proto);
    spdlog::info("free array proto");
    return 0;
}

Array32 deserialize_array32(FILE *fp) {
    if (!fp) {
        spdlog::error("failed to open file");
        return (Array32){0, NULL};
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t*)malloc(len);
    fread(buf, sizeof(uint8_t), len, fp);

    State__Array32 *array_proto;
    array_proto = state__array32__unpack(NULL, len, buf);
    Array32 array;
    array.size = array_proto->n_contents;
    array.contents = (uint32_t*)malloc(sizeof(uint32_t) * array.size);
    memcpy(array.contents, array_proto->contents, sizeof(uint32_t) * array.size);

    free(buf);
    state__array32__free_unpacked(array_proto, NULL);
    
    return array;
}

