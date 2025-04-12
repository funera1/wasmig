#include <msgpack.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <wasmig/stack_table.h>

using namespace std;


StackTable deserialize(fs::path path) {
    fs::path full_path = path / "stack-table.msgpack";
    std::ifstream ifs(full_path, std::ios::binary);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string data = buffer.str();

    msgpack::object_handle oh = msgpack::unpack(data.data(), data.size());
    StackTable table = StackTable::from_msgpack(oh.get());
    return table;
}