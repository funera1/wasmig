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

std::string array32_to_string(std::ostringstream &oss, Array8 *type_stack, Array32 *array, std::string name) {
    oss << name << ": [";
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
                spdlog::error("Not support S128");
                oss << "(S128)";
                break;
            default:
                spdlog::error("Unknown type {}", type_stack->contents[i]);
                oss << "(unknown)";
                break;
        }
    }
    oss << "]";
    return oss.str();
}

std::string locals_to_string(Array8 *type_stack, Array32 *locals) {
    std::ostringstream oss;
    return array32_to_string(oss, type_stack, locals, "locals");
}

std::string value_stack_to_string(Array8 *type_stack, Array32 *value_stack) {
    std::ostringstream oss;
    return array32_to_string(oss, type_stack, value_stack, "value stack");
}


void print_type_stack(uint8_t* stack, uint32_t stack_size) {
    std::string output = type_stack_to_string(stack, stack_size);
    spdlog::debug("{}", output);  // spdlogで出力
}

void print_locals(Array8 *type_stack, Array32 *locals) {
    std::string output = locals_to_string(type_stack, locals);
    spdlog::debug("{}", output);  // spdlogで出力
}

void print_stack(Array8 *type_stack, Array32 *stack) {
    std::string output = value_stack_to_string(type_stack, stack);
    spdlog::debug("{}", output);  // spdlogで出力
}