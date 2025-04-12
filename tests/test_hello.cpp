#include <gtest/gtest.h>
#include <stack_table.h>

int sum(int a, int b) {
    return a + b;
}

TEST(TestCase, sum) {
    EXPECT_EQ(2, sum(1, 1));
}

TEST(TestCase, deserialize) {
    fs::path path = "test_data";
    StackTable table = deserialize(path);
    EXPECT_EQ(table.entries.size(), 2);
    EXPECT_EQ(table.entries[0].offset, 0x1000);
    EXPECT_EQ(table.entries[1].offset, 0x2000);
}