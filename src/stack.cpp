#include "wasmig/stack.h"
#include <stdlib.h>
#include <stdio.h>

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
}
