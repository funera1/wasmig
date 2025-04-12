#include <msgpack.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <wasmig/stack_table.h>
#include <spdlog/spdlog.h>

using namespace std;


StackTable deserialize(fs::path path) {
    fs::path full_path = path / "stack-table.msgpack";
    spdlog::info("Loading stack table from: {}", full_path.string());
    std::ifstream ifs(full_path, std::ios::binary);
    if (!ifs) {
        spdlog::error("Error opening file: {}", full_path.string());
        throw std::runtime_error("Failed to open file");
    }
    spdlog::info("Open stack table file");

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string data = buffer.str();
    spdlog::info("Read {} bytes", data.size());

    msgpack::object_handle oh = msgpack::unpack(data.data(), data.size());
    spdlog::info("Unpacked msgpack data");

    StackTable table = StackTable::from_msgpack(oh.get());
    spdlog::info("Deserialized stack table with {} entries", table.entries.size());
    return table;
}