#include <gtest/gtest.h>
#include <wasmig/stack_tables.h>
#include <spdlog/spdlog.h>

int sum(int a, int b) {
    return a + b;
}

TEST(TestCase, sum) {
    EXPECT_EQ(2, sum(1, 1));
}

// TEST DATA DIR は CMake で定義される
#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "./test_data"
#endif

// TEST(TestCase, deserialize) {
//     spdlog::set_level(spdlog::level::info);
//     spdlog::info("deserialize test");

//     fs::path path(TEST_DATA_DIR);
//     spdlog::info("path: {}", path.string());
//     StackTable table = deserialize(path);
//     EXPECT_EQ(table.entries.size(), 2);
//     EXPECT_EQ(table.entries[0].offset, 0x1000);
//     EXPECT_EQ(table.entries[1].offset, 0x2000);
// }