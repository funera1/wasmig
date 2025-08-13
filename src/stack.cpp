#include "wasmig/stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Hash table for stack state mapping
#define STACK_STATE_MAP_SIZE 256

typedef struct stack_state_entry {
    char* key;
    Stack stack;
    struct stack_state_entry* next;
} StackStateEntry;

struct stack_state_map {
    StackStateEntry* buckets[STACK_STATE_MAP_SIZE];
};

// Simple hash function for strings
static unsigned int hash_string(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % STACK_STATE_MAP_SIZE;
}

// Internal stack node structure for persistent stack
struct stack_node {
    uint64_t value;
    struct stack_node* next;
    int ref_count;  // Reference counting for memory management
};

// Global empty stack singleton
static struct stack_node empty_stack_singleton = { 0, NULL, 1 };
static Stack empty_stack = &empty_stack_singleton;

// Helper function to increment reference count
static void stack_retain(Stack stack) {
    if (stack && stack != empty_stack) {
        stack->ref_count++;
    }
}

// Helper function to decrement reference count and free if needed
static void stack_release(Stack stack) {
    if (stack && stack != empty_stack) {
        stack->ref_count--;
        if (stack->ref_count <= 0) {
            stack_release(stack->next);
            free(stack);
        }
    }
}

extern "C" {
    Stack stack_empty() {
        return empty_stack;
    }

    Stack stack_push(Stack stack, uint64_t value) {
        struct stack_node* new_node = (struct stack_node*)malloc(sizeof(struct stack_node));
        if (!new_node) {
            return NULL;  // Memory allocation failed
        }
        
        new_node->value = value;
        new_node->next = stack;
        new_node->ref_count = 1;
        
        // Increment reference count of the previous stack
        stack_retain(stack);
        
        return new_node;
    }

    Stack stack_pop(Stack stack, uint64_t *value) {
        if (stack == empty_stack || !stack) {
            if (value) *value = 0;
            return empty_stack;
        }
        
        if (value) {
            *value = stack->value;
        }
        
        Stack next = stack->next;
        stack_retain(next);  // Retain the next stack before releasing current
        
        return next ? next : empty_stack;
    }

    bool stack_is_empty(Stack stack) {
        return stack == empty_stack || stack == NULL;
    }

    uint64_t stack_top(Stack stack) {
        if (stack == empty_stack || !stack) {
            return 0;  // Default value for empty stack
        }
        return stack->value;
    }

    size_t stack_size(Stack stack) {
        size_t count = 0;
        while (stack && stack != empty_stack) {
            count++;
            stack = stack->next;
        }
        return count;
    }

    void stack_destroy(Stack stack) {
        stack_release(stack);
    }

    void stack_print(Stack stack) {
        printf("Stack: ");
        if (stack_is_empty(stack)) {
            printf("(empty)\n");
            return;
        }
        
        printf("[ ");
        Stack current = stack;
        while (current && current != empty_stack) {
            printf("%lu ", current->value);
            current = current->next;
        }
        printf("]\n");
    }

    // Stack state management functions
    StackStateMap stack_state_map_create() {
        StackStateMap map = (StackStateMap)malloc(sizeof(struct stack_state_map));
        if (!map) return NULL;
        
        for (int i = 0; i < STACK_STATE_MAP_SIZE; i++) {
            map->buckets[i] = NULL;
        }
        return map;
    }

    bool stack_state_save(StackStateMap map, const char* key, Stack stack) {
        if (!map || !key) return false;
        
        unsigned int index = hash_string(key);
        
        // Check if key already exists and update
        StackStateEntry* entry = map->buckets[index];
        while (entry) {
            if (strcmp(entry->key, key) == 0) {
                // Release old stack and save new one
                stack_release(entry->stack);
                entry->stack = stack;
                stack_retain(stack);
                return true;
            }
            entry = entry->next;
        }
        
        // Create new entry
        entry = (StackStateEntry*)malloc(sizeof(StackStateEntry));
        if (!entry) return false;
        
        entry->key = (char*)malloc(strlen(key) + 1);
        if (!entry->key) {
            free(entry);
            return false;
        }
        
        strcpy(entry->key, key);
        entry->stack = stack;
        stack_retain(stack);
        entry->next = map->buckets[index];
        map->buckets[index] = entry;
        
        return true;
    }

    Stack stack_state_load(StackStateMap map, const char* key) {
        if (!map || !key) return empty_stack;
        
        unsigned int index = hash_string(key);
        StackStateEntry* entry = map->buckets[index];
        
        while (entry) {
            if (strcmp(entry->key, key) == 0) {
                return entry->stack;
            }
            entry = entry->next;
        }
        
        return empty_stack;
    }

    bool stack_state_exists(StackStateMap map, const char* key) {
        if (!map || !key) return false;
        
        unsigned int index = hash_string(key);
        StackStateEntry* entry = map->buckets[index];
        
        while (entry) {
            if (strcmp(entry->key, key) == 0) {
                return true;
            }
            entry = entry->next;
        }
        
        return false;
    }

    bool stack_state_remove(StackStateMap map, const char* key) {
        if (!map || !key) return false;
        
        unsigned int index = hash_string(key);
        StackStateEntry* entry = map->buckets[index];
        StackStateEntry* prev = NULL;
        
        while (entry) {
            if (strcmp(entry->key, key) == 0) {
                if (prev) {
                    prev->next = entry->next;
                } else {
                    map->buckets[index] = entry->next;
                }
                
                stack_release(entry->stack);
                free(entry->key);
                free(entry);
                return true;
            }
            prev = entry;
            entry = entry->next;
        }
        
        return false;
    }

    void stack_state_map_destroy(StackStateMap map) {
        if (!map) return;
        
        for (int i = 0; i < STACK_STATE_MAP_SIZE; i++) {
            StackStateEntry* entry = map->buckets[i];
            while (entry) {
                StackStateEntry* next = entry->next;
                stack_release(entry->stack);
                free(entry->key);
                free(entry);
                entry = next;
            }
        }
        
        free(map);
    }
}
