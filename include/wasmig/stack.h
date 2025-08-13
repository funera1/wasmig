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

// Stack state management
typedef struct stack_state_map* StackStateMap;

// Create a new stack state map
StackStateMap stack_state_map_create();

// Save stack state with a key
bool stack_state_save(StackStateMap map, const char* key, Stack stack);

// Load stack state by key
Stack stack_state_load(StackStateMap map, const char* key);

// Check if key exists
bool stack_state_exists(StackStateMap map, const char* key);

// Remove state by key
bool stack_state_remove(StackStateMap map, const char* key);

// Destroy the state map
void stack_state_map_destroy(StackStateMap map);

// Debug utilities
void stack_print(Stack stack);

#ifdef __cplusplus
}
#endif

#endif // WASMIG_STACK_H
