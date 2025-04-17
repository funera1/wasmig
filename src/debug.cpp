#include <wasmig/internal/debug.hpp>
#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>

std::string type_stack_to_string(uint8_t* stack, uint32_t stack_size) {
    std::ostringstream oss;
    oss << "type stack: [";
    for (uint32_t i = 0; i < stack_size; ++i) {
        if (i != 0) oss << ", ";
        switch (stack[i]) {
            case 1: oss << "S32"; break;
            case 2: oss << "S64"; break;
            case 4: oss << "S128"; break;
            default: oss << static_cast<int>(stack[i]); break; // その他の型はそのまま出力
        }
    }
    oss << "]";
    return oss.str();
}

ArrayStringResult array32_to_string(std::ostringstream &oss, Array8 *type_stack, Array32 *array, std::string name) {
    oss << name << ": [";
    std::string error = "";
    uint32_t *sp = array->contents;
    for (int i = 0; i < array->size; ++i) {
        if (i != 0) oss << ", ";
        switch (type_stack->contents[i]) {
            case 1:  // i32
                oss << static_cast<uint32_t>(*sp);
                sp++;
                break;
            case 2: {  // i64
                // エンディアン間違えてるかも
                uint32_t high = sp[0];
                uint32_t low = sp[1];
                uint64_t value = (static_cast<uint64_t>(high) << 32) | low;
                oss << value;
                sp += 2;
                break;
            }
            case 4:
                error = "Not support S128";
                spdlog::error(error);
                oss << "(S128)";
                break;
            default:
                error = "Unknown type " + std::to_string(type_stack->contents[i]);
                spdlog::error(error);
                oss << "(unknown)";
                break;
        }
    }
    oss << "]";
    return ArrayStringResult(oss.str(), error);
}

ArrayStringResult locals_to_string(Array8 *type_stack, Array32 *locals) {
    std::ostringstream oss;
    return array32_to_string(oss, type_stack, locals, "locals");
}

ArrayStringResult value_stack_to_string(Array8 *type_stack, Array32 *value_stack) {
    std::ostringstream oss;
    return array32_to_string(oss, type_stack, value_stack, "value stack");
}

void print_type_stack(uint8_t* stack, uint32_t stack_size) {
    std::string output = type_stack_to_string(stack, stack_size);
    spdlog::debug("{}", output);  // spdlogで出力
}

void print_locals(CodePos &pos, Array8 *type_stack, Array32 *locals) {
    ArrayStringResult result = locals_to_string(type_stack, locals);
    if (result.error.empty()) {
        spdlog::debug("{}", result.output);  // spdlogで出力
    } else {
        spdlog::debug("({}, {}): {}", pos.fidx, pos.offset, result.output);  // spdlogで出力
    }
}

void print_stack(CodePos &pos, Array8 *type_stack, Array32 *stack) {
    ArrayStringResult result = value_stack_to_string(type_stack, stack);
    if (result.error.empty()) {
        spdlog::debug("{}", result.output);  // spdlogで出力
    } else {
        spdlog::debug("({}, {}): {}", pos.fidx, pos.offset, result.output);  // spdlogで出力
    }
}