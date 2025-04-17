#pragma once
#include <cstdint>
#include <string>
#include <wasmig/migration.h>

// TODO: test時だけinclude可能にする
void print_type_stack(uint8_t* stack, uint32_t stack_size);
void print_locals(Array8 *type_stack, Array32 *locals);
void print_stack(Array8 *type_stack, Array32 *stack);

std::string type_stack_to_string(uint8_t* stack, uint32_t stack_size);
std::string locals_to_string(Array8 *type_stack, Array32 *locals);
std::string value_stack_to_string(Array8 *type_stack, Array32 *value_stack);
