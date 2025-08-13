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
Stack stack_create();
Stack stack_empty();
Stack stack_push(Stack stack, uint64_t value);
Stack stack_pop(Stack stack, uint64_t *value);
bool stack_is_empty(Stack stack);
uint64_t stack_top(Stack stack);
size_t stack_size(Stack stack);
void stack_destroy(Stack stack);

// Debug utilities for basic stack
void stack_print(Stack stack);

// ========================================
// Stack State Management API
// ========================================

// Opaque stack state map handle
typedef struct stack_state_map* StackStateMap;

// Stack state map operations
StackStateMap stack_state_map_create();
void stack_state_map_destroy(StackStateMap map);

// State management operations (uint32_t keys)
bool stack_state_save(StackStateMap map, uint32_t key, Stack stack);
Stack stack_state_load(StackStateMap map, uint32_t key);
bool stack_state_exists(StackStateMap map, uint32_t key);
bool stack_state_remove(StackStateMap map, uint32_t key);

// Debug utilities for state map
void stack_state_map_debug_print(StackStateMap map);

// ========================================
// Stack State Map Registry API (global)
// ========================================
// Register a map with an ID so it can be retrieved elsewhere.
// Returns false if id already exists or map is NULL.
bool stack_state_map_register(uint32_t id, StackStateMap map);

// Get a registered map by ID. Returns NULL if not found.
StackStateMap stack_state_map_get(uint32_t id);

// Unregister a map by ID. Does not destroy the map. Returns false if not found.
bool stack_state_map_unregister(uint32_t id);

// Clear all registered maps (does not destroy them).
void stack_state_map_registry_clear();

#ifdef __cplusplus
}
#endif

#endif // WASMIG_STACK_H
