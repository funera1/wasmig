#include <gtest/gtest.h>
#include <wasmig/stack.h>

TEST(StackRegistryTest, RegisterGetUnregister) {
    stack_state_map_registry_clear();

    StackStateMap map1 = stack_state_map_create();
    ASSERT_NE(map1, nullptr);
    EXPECT_TRUE(stack_state_map_register(100, map1));
    EXPECT_FALSE(stack_state_map_register(100, map1)); // duplicate id not allowed

    StackStateMap fetched = stack_state_map_get(100);
    EXPECT_EQ(fetched, map1);

    EXPECT_TRUE(stack_state_map_unregister(100));
    EXPECT_EQ(stack_state_map_get(100), nullptr);

    stack_state_map_destroy(map1);
}

TEST(StackRegistryTest, AutoUnregisterOnDestroy) {
    stack_state_map_registry_clear();

    StackStateMap map1 = stack_state_map_create();
    ASSERT_NE(map1, nullptr);
    EXPECT_TRUE(stack_state_map_register(200, map1));

    // Destroy the map; it should unregister itself
    stack_state_map_destroy(map1);

    EXPECT_EQ(stack_state_map_get(200), nullptr);
}

TEST(StackRegistryTest, MultipleMaps) {
    stack_state_map_registry_clear();

    StackStateMap m1 = stack_state_map_create();
    StackStateMap m2 = stack_state_map_create();
    StackStateMap m3 = stack_state_map_create();

    ASSERT_TRUE(stack_state_map_register(1, m1));
    ASSERT_TRUE(stack_state_map_register(2, m2));
    ASSERT_TRUE(stack_state_map_register(3, m3));

    EXPECT_EQ(stack_state_map_get(1), m1);
    EXPECT_EQ(stack_state_map_get(2), m2);
    EXPECT_EQ(stack_state_map_get(3), m3);

    stack_state_map_registry_clear();

    EXPECT_EQ(stack_state_map_get(1), nullptr);
    EXPECT_EQ(stack_state_map_get(2), nullptr);
    EXPECT_EQ(stack_state_map_get(3), nullptr);

    stack_state_map_destroy(m1);
    stack_state_map_destroy(m2);
    stack_state_map_destroy(m3);
}
