#include <gtest/gtest.h>
#include <wasmig/stack.h>

// Ensure that stacks loaded from the map can be destroyed by the caller
// without invalidating the map entry (i.e., load retains before returning).
TEST(StackStateLifetime, LoadReturnsRetained) {
    StackStateMap map = stack_state_map_create();
    ASSERT_NE(map, nullptr);

    Stack s = stack_create();
    s = stack_push(s, 1);
    EXPECT_TRUE(stack_state_save(map, 123, s));

    // Load twice; each load should be independently destroyable
    Stack a = stack_state_load(map, 123);
    Stack b = stack_state_load(map, 123);

    // Destroy loaded stacks
    stack_destroy(a);
    stack_destroy(b);

    // Map should still have the stack
    EXPECT_TRUE(stack_state_exists(map, 123));
    Stack c = stack_state_load(map, 123);
    EXPECT_EQ(stack_top(c), 1u);
    stack_destroy(c);

    // Clean up original
    stack_destroy(s);
    stack_state_map_destroy(map);
}
