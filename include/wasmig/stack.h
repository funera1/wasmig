#ifndef WASMIG_STACK_H
#define WASMIG_STACK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque stack handle
typedef struct stack_node* Stack;

// Stack operations
Stack stack_empty();
Stack stack_push(Stack stack, uint64_t value);
Stack stack_pop(Stack stack, uint64_t *value);
bool stack_is_empty(Stack stack);
uint64_t stack_top(Stack stack);
size_t stack_size(Stack stack);

// Memory management
void stack_destroy(Stack stack);

// Debug utilities
void stack_print(Stack stack);

#ifdef __cplusplus
}
#endif

#endif // WASMIG_STACK_H
