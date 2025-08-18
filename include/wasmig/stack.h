#ifndef WASMIG_STACK_H
#define WASMIG_STACK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// Basic Stack API
// ========================================

// Opaque stack handle
typedef struct stack_node* Stack;

// Basic stack operations
Stack wasmig_stack_create();
Stack wasmig_stack_empty();
Stack wasmig_stack_push(Stack stack, uint64_t value);
Stack wasmig_stack_pop(Stack stack, uint64_t *value);
bool wasmig_stack_is_empty(Stack stack);
uint64_t wasmig_stack_top(Stack stack);
size_t wasmig_stack_size(Stack stack);
void wasmig_stack_destroy(Stack stack);
void wasmig_stack_print(Stack stack);

// ========================================
// Stack State Management API
// ========================================

// Opaque stack state map handle
typedef struct stack_state_map* StackStateMap;

// Stack state map operations
StackStateMap wasmig_stack_state_map_create();
void wasmig_stack_state_map_destroy(StackStateMap map);
bool wasmig_stack_state_save(StackStateMap map, uint32_t key, Stack stack);
Stack wasmig_stack_state_load(StackStateMap map, uint32_t key);
bool wasmig_stack_state_exists(StackStateMap map, uint32_t key);
bool wasmig_stack_state_remove(StackStateMap map, uint32_t key);
void wasmig_stack_state_map_debug_print(StackStateMap map);

// Backward-compat macros
#define stack_create wasmig_stack_create
#define stack_empty wasmig_stack_empty
#define stack_push wasmig_stack_push
#define stack_pop wasmig_stack_pop
#define stack_is_empty wasmig_stack_is_empty
#define stack_top wasmig_stack_top
#define stack_size wasmig_stack_size
#define stack_destroy wasmig_stack_destroy
#define stack_print wasmig_stack_print

#define stack_state_map_create wasmig_stack_state_map_create
#define stack_state_map_destroy wasmig_stack_state_map_destroy
#define stack_state_save wasmig_stack_state_save
#define stack_state_load wasmig_stack_state_load
#define stack_state_exists wasmig_stack_state_exists
#define stack_state_remove wasmig_stack_state_remove
#define stack_state_map_debug_print wasmig_stack_state_map_debug_print

#ifdef __cplusplus
}
#endif

#endif // WASMIG_STACK_H
