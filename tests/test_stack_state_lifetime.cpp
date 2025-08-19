#include <gtest/gtest.h>
#include <wasmig/stack.h>

// Ensure that stacks loaded from the map can be destroyed by the caller
// without invalidating the map entry (i.e., load retains before returning).
TEST(StackStateLifetime, LoadReturnsRetained) {
    StackStateMap map = stack_state_map_create();
    ASSERT_NE(map, nullptr);

    Stack s = stack_create();
    s = stack_push(s, 1);
    EXPECT_TRUE(stack_state_save_pair(map, 123, s, stack_empty()));

    // Load twice; each load should be independently destroyable
    Stack a = stack_empty(); Stack a_type = stack_empty();
    Stack b = stack_empty(); Stack b_type = stack_empty();
    EXPECT_TRUE(stack_state_load_pair(map, 123, &a, &a_type));
    EXPECT_TRUE(stack_state_load_pair(map, 123, &b, &b_type));

    // Destroy loaded stacks
    stack_destroy(a); stack_destroy(a_type);
    stack_destroy(b); stack_destroy(b_type);

    // Map should still have the stack
    EXPECT_TRUE(stack_state_exists(map, 123));
    Stack c = stack_empty(); Stack c_type = stack_empty();
    EXPECT_TRUE(stack_state_load_pair(map, 123, &c, &c_type));
    EXPECT_EQ(stack_top(c), 1u);
    stack_destroy(c); stack_destroy(c_type);

    // Clean up original
    stack_destroy(s);
    stack_state_map_destroy(map);
}
