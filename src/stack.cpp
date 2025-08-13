#include "wasmig/stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <string>

// ========================================
// Basic Stack Implementation
// ========================================

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

// ========================================
// Stack State Map Implementation
// ========================================

// Stack state map structure using C++ unordered_map for efficiency
struct stack_state_map {
    std::unordered_map<std::string, Stack>* map;
    
    stack_state_map() : map(new std::unordered_map<std::string, Stack>()) {}
    ~stack_state_map() { delete map; }
};

extern "C" {
    // ========================================
    // Basic Stack Operations
    // ========================================
    
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
        try {
            return new stack_state_map();
        } catch (...) {
            return nullptr;
        }
    }

    // =====================
    // Stack State Management Functions
    // =====================
    
    bool stack_state_save(StackStateMap map, const char* key, Stack stack) {
        if (!map || !key) return false;
        
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

    Stack stack_state_load(StackStateMap map, const char* key) {
        if (!map || !key) return empty_stack;
        
        try {
            auto it = map->map->find(key);
            if (it != map->map->end()) {
                return it->second;
            }
        } catch (...) {
            // エラーが発生した場合は空のスタックを返す
        }
        
        return empty_stack;
    }

    bool stack_state_exists(StackStateMap map, const char* key) {
        if (!map || !key) return false;
        
        try {
            return map->map->find(key) != map->map->end();
        } catch (...) {
            return false;
        }
    }

    bool stack_state_remove(StackStateMap map, const char* key) {
        if (!map || !key) return false;
        
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

    void stack_state_map_destroy(StackStateMap map) {
        if (!map) return;
        
        try {
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
