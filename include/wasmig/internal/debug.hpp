#pragma once
#include <cstdint>
#include <string>
#include <wasmig/state.h>

// TODO: test時だけinclude可能にする
void print_type_stack(uint8_t* stack, uint32_t stack_size);
void print_locals(CodePos &pos, Array8 *type_stack, Array32 *locals);
void print_stack(CodePos &pos, Array8 *type_stack, Array32 *stack);

struct ArrayStringResult {
    std::string output;
    std::string error;
    
    ArrayStringResult(std::string o, std::string e) : output(o), error(e) {}
    ArrayStringResult(std::string o) : output(o), error("") {}
};

std::string type_stack_to_string(uint8_t* stack, uint32_t stack_size);
ArrayStringResult locals_to_string(Array8 *type_stack, Array32 *locals);
ArrayStringResult value_stack_to_string(Array8 *type_stack, Array32 *value_stack);
