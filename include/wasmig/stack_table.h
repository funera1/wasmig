#pragma once
#include <msgpack.hpp>
#include <filesystem>

namespace fs = std::filesystem;

enum class OpKind {
    LocalGet,
    I32Const,
    F32Const,
    I64Const,
    F64Const,
    Other
};

struct CompiledOp {
    OpKind kind;
    union {
        uint32_t u32val;
        int32_t i32val;
        int64_t i64val;
        uint64_t u64val;
    } value;

    static CompiledOp from_msgpack(const msgpack::object& obj) {
        CompiledOp out;

        if (obj.type != msgpack::type::ARRAY || obj.via.array.size != 2)
            throw std::runtime_error("Invalid CompiledOp");

        std::string kind = obj.via.array.ptr[0].as<std::string>();
        const msgpack::object& val = obj.via.array.ptr[1];

        if (kind == "LocalGet") {
            out.kind = OpKind::LocalGet;
            out.value.u32val = val.as<uint32_t>();
        } else if (kind == "I32Const") {
            out.kind = OpKind::I32Const;
            out.value.i32val = val.as<int32_t>();
        } else if (kind == "F32Const") {
            out.kind = OpKind::F32Const;
            out.value.u32val = val.as<uint32_t>();
        } else if (kind == "I64Const") {
            out.kind = OpKind::I64Const;
            out.value.i64val = val.as<int64_t>();
        } else if (kind == "F64Const") {
            out.kind = OpKind::F64Const;
            out.value.u64val = val.as<uint64_t>();
        } else {
            out.kind = OpKind::Other;
        }

        return out;
    }
};

struct StEntry {
    uint32_t offset;
    std::vector<std::pair<CompiledOp, std::string>> stack;

    static StEntry from_msgpack(const msgpack::object& obj) {
        StEntry out;

        if (obj.type != msgpack::type::MAP)
            throw std::runtime_error("Expected map for StEntry");

        for (uint32_t i = 0; i < obj.via.map.size; ++i) {
            std::string key = obj.via.map.ptr[i].key.as<std::string>();
            const msgpack::object& val = obj.via.map.ptr[i].val;

            if (key == "offset") {
                out.offset = val.as<uint32_t>();
            } else if (key == "stack") {
                for (uint32_t j = 0; j < val.via.array.size; ++j) {
                    auto& pair = val.via.array.ptr[j];
                    const msgpack::object& op_obj = pair.via.array.ptr[0];
                    const msgpack::object& type_obj = pair.via.array.ptr[1];

                    CompiledOp op = CompiledOp::from_msgpack(op_obj);
                    std::string type_str = type_obj.as<std::string>(); // WasmTypeをstringとして扱う

                    out.stack.emplace_back(op, type_str);
                }
            }
        }

        return out;
    }
};

struct StackTable {
    std::vector<StEntry> entries;

    static StackTable from_msgpack(const msgpack::object& obj) {
        StackTable table;

        for (uint32_t i = 0; i < obj.via.map.size; ++i) {
            std::string key = obj.via.map.ptr[i].key.as<std::string>();
            const msgpack::object& val = obj.via.map.ptr[i].val;

            if (key == "inner") {
                for (uint32_t j = 0; j < val.via.array.size; ++j) {
                    table.entries.push_back(StEntry::from_msgpack(val.via.array.ptr[j]));
                }
            }
        }

        return table;
    }
};

StackTable deserialize(fs::path);