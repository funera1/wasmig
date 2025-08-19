#include "wasmig/stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>

// ========================================
// Basic Stack Implementation
// ========================================

// Internal stack node structure for persistent stack
struct stack_node {
    uint64_t value;
    struct stack_node* next;
    int ref_count;  // Reference counting for memory management
    bool is_terminal; // true if this node represents an empty terminal (sentinel/root)
};

// Global empty stack singleton
static struct stack_node empty_stack_singleton = { 0, NULL, 1, true };
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

// ========================================
// Stack State Map Implementation
// ========================================

// Stack state map structure using C++ unordered_map for efficiency
struct stack_state_map {
    // Store a pair of stacks per key: address stack and type stack
    struct stack_pair {
        Stack address_stack;
        Stack type_stack;
    };

    std::unordered_map<uint32_t, stack_pair>* map;

    stack_state_map() : map(new std::unordered_map<uint32_t, stack_pair>()) {}
    ~stack_state_map() { delete map; }
};

// ========================================
// Global registry for StackStateMap
// ========================================
static std::unordered_map<uint32_t, StackStateMap> g_state_map_registry;

extern "C" {
    // ========================================
    // Basic Stack Operations
    // ========================================
    
    Stack wasmig_stack_create() {
        struct stack_node* new_node = (struct stack_node*)malloc(sizeof(struct stack_node));
        if (!new_node) return NULL;
        new_node->value = 0;
        new_node->next = NULL;
        new_node->ref_count = 1;
        new_node->is_terminal = true; // allocated empty root
        return new_node;
    }
    
    Stack wasmig_stack_empty() {
        return empty_stack;
    }

    Stack wasmig_stack_push(Stack stack, uint64_t value) {
        struct stack_node* new_node = (struct stack_node*)malloc(sizeof(struct stack_node));
        if (!new_node) {
            return NULL;  // Memory allocation failed
        }
        
        new_node->value = value;
        new_node->next = stack;
        new_node->ref_count = 1;
        new_node->is_terminal = false; // element node
        
        // Increment reference count of the previous stack
        stack_retain(stack);
        
        return new_node;
    }

    Stack wasmig_stack_pop(Stack stack, uint64_t *value) {
        if (!stack || stack->is_terminal) {
            if (value) *value = 0;
            return stack ? stack : empty_stack;
        }
        
        if (value) {
            *value = stack->value;
        }
        
        Stack next = stack->next;
        stack_retain(next);  // Retain the next stack before releasing current
        
        return next ? next : empty_stack;
    }

    bool wasmig_stack_is_empty(Stack stack) {
        return stack == NULL || stack->is_terminal;
    }

    uint64_t wasmig_stack_top(Stack stack) {
    if (!stack || stack->is_terminal) {
            return 0;  // Default value for empty stack
        }
        return stack->value;
    }

    size_t wasmig_stack_size(Stack stack) {
        size_t count = 0;
        while (stack && !stack->is_terminal) {
            count++;
            stack = stack->next;
        }
        return count;
    }

    void wasmig_stack_destroy(Stack stack) {
        stack_release(stack);
    }

    // Print the stack from top towards the bottom
    void wasmig_stack_print(Stack stack) {
        printf("Stack: ");
        if (wasmig_stack_is_empty(stack)) {
            printf("(empty)\n");
            return;
        }
        
        printf("[ ");
        Stack current = stack;
        while (current && !current->is_terminal) {
            printf("%lu ", current->value);
            current = current->next;
        }
        printf("]\n");
    }

    // Stack state management functions
    StackStateMap wasmig_stack_state_map_create() {
        try {
            return new stack_state_map();
        } catch (...) {
            return nullptr;
        }
    }

    // =====================
    // Stack State Management Functions
    // =====================
    
    // Save a pair of stacks (address and type) under a key
    bool wasmig_stack_state_save_pair(StackStateMap map, uint32_t key, Stack address_stack, Stack type_stack) {
        if (!map) return false;

        try {
            auto it = map->map->find(key);
            if (it != map->map->end()) {
                // release existing stacks
                stack_release(it->second.address_stack);
                stack_release(it->second.type_stack);
            }

            // retain incoming stacks
            if (address_stack) stack_retain(address_stack);
            if (type_stack) stack_retain(type_stack);

            (*map->map)[key] = { address_stack, type_stack };
            return true;
        } catch (...) {
            return false;
        }
    }

    // Load pair of stacks; outputs retained stacks via out pointers
    bool wasmig_stack_state_load_pair(StackStateMap map, uint32_t key, Stack *out_address_stack, Stack *out_type_stack) {
        if (!map || !out_address_stack || !out_type_stack) return false;

        try {
            auto it = map->map->find(key);
            if (it != map->map->end()) {
                *out_address_stack = it->second.address_stack ? it->second.address_stack : empty_stack;
                *out_type_stack = it->second.type_stack ? it->second.type_stack : empty_stack;

                // retain both so caller owns them
                if (*out_address_stack) stack_retain(*out_address_stack);
                if (*out_type_stack) stack_retain(*out_type_stack);
                return true;
            }
        } catch (...) {
            // fallthrough
        }

        return false;
    }


    bool wasmig_stack_state_exists(StackStateMap map, uint32_t key) {
        if (!map) return false;
        
        try {
            return map->map->find(key) != map->map->end();
        } catch (...) {
            return false;
        }
    }

    bool wasmig_stack_state_remove(StackStateMap map, uint32_t key) {
        if (!map) return false;
        
        try {
            auto it = map->map->find(key);
            if (it != map->map->end()) {
                // release both stacks if present
                stack_release(it->second.address_stack);
                stack_release(it->second.type_stack);
                map->map->erase(it);
                return true;
            }
        } catch (...) {
            return false;
        }
        
        return false;
    }

    void wasmig_stack_state_map_destroy(StackStateMap map) {
        if (!map) return;
        
        try {
            // レジストリから該当マップを取り除く
            for (auto it = g_state_map_registry.begin(); it != g_state_map_registry.end(); ) {
                if (it->second == map) {
                    it = g_state_map_registry.erase(it);
                } else {
                    ++it;
                }
            }
            // すべての保存されたスタックを解放
            for (auto& kv : *map->map) {
                stack_release(kv.second.address_stack);
                stack_release(kv.second.type_stack);
            }
            delete map;
        } catch (...) {
            // エラーが発生してもメモリリークを防ぐため削除を試行
            delete map;
        }
    }

    // Debug print for map contents
    void wasmig_stack_state_map_debug_print(StackStateMap map) {
        if (!map) {
            printf("StackStateMap: (null)\n");
            return;
        }
        printf("StackStateMap contents (size=%zu):\n", map->map->size());
        for (auto &kv : *map->map) {
            uint32_t k = kv.first;
            size_t asz = wasmig_stack_size(kv.second.address_stack);
            size_t tsz = wasmig_stack_size(kv.second.type_stack);
            printf("  key=%u -> address_stack_size=%zu, type_stack_size=%zu\n", k, asz, tsz);
        }
    }

}
