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
    std::unordered_map<uint32_t, Stack>* map;
    
    stack_state_map() : map(new std::unordered_map<uint32_t, Stack>()) {}
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

    struct stack_iterator {
        Stack current; // retained current node
    };

    // Create an iterator. Retains the root node (or empty singleton).
    StackIterator wasmig_stack_iterator_create(Stack stack) {
        StackIterator it = (StackIterator)malloc(sizeof(struct stack_iterator));
        if (!it) return NULL;
        if (!stack) stack = empty_stack;
        // Retain the starting node to keep it alive during iteration
        stack_retain(stack);
        it->current = stack;
        return it;
    }

    bool wasmig_stack_iterator_has_next(StackIterator it) {
        if (!it || !it->current) return false;
        return !it->current->is_terminal;
    }

    uint64_t wasmig_stack_iterator_peek(StackIterator it) {
        if (!it || !it->current || it->current->is_terminal) return 0;
        return it->current->value;
    }

    uint64_t wasmig_stack_iterator_next(StackIterator it) {
        if (!it || !it->current || it->current->is_terminal) return 0;
        uint64_t val = it->current->value;
        Stack next = it->current->next ? it->current->next : empty_stack;
        // retain next before releasing current to ensure safety
        stack_retain(next);
        stack_release(it->current);
        it->current = next;
        return val;
    }

    void wasmig_stack_iterator_destroy(StackIterator it) {
        if (!it) return;
        if (it->current) stack_release(it->current);
        free(it);
    }

    void wasmig_stack_foreach(Stack stack, wasmig_stack_iter_cb cb, void* user) {
        if (!cb) return;
        StackIterator it = wasmig_stack_iterator_create(stack);
        if (!it) return;
        while (wasmig_stack_iterator_has_next(it)) {
            uint64_t v = wasmig_stack_iterator_next(it);
            cb(v, user);
        }
        wasmig_stack_iterator_destroy(it);
    }

    // =====================
    // Stack State Management Functions
    // =====================

    // Stack state management functions
    StackStateMap wasmig_stack_state_map_create() {
        try {
            return new stack_state_map();
        } catch (...) {
            return nullptr;
        }
    }
    
    bool wasmig_stack_state_save(StackStateMap map, uint32_t key, Stack stack) {
        if (!map) return false;
        
        try {
            auto it = map->map->find(key);
            if (it != map->map->end()) {
                // 既存のキーがある場合、古いスタックを解放
                stack_release(it->second);
            }
            
            // 新しいスタックを保存
            stack_retain(stack);
            (*map->map)[key] = stack;
            return true;
        } catch (...) {
            return false;
        }
    }

    Stack wasmig_stack_state_load(StackStateMap map, uint32_t key) {
        if (!map) return empty_stack;
        
        try {
            auto it = map->map->find(key);
            if (it != map->map->end()) {
                // Return a retained reference so caller owns it independently
                stack_retain(it->second);
                return it->second;
            }
        } catch (...) {
            // エラーが発生した場合は空のスタックを返す
        }
        
        return empty_stack;
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
                stack_release(it->second);
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
            for (auto& pair : *map->map) {
                stack_release(pair.second);
            }
            delete map;
        } catch (...) {
            // エラーが発生してもメモリリークを防ぐため削除を試行
            delete map;
        }
    }

}
