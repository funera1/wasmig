#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <wasmig/stack_tables.h>
#include <wasmig/internal/debug.hpp>
#include <wasmig/state.h>

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

TEST(TestCase, type_stack_to_string) {
    size_t stack_size = 3;
    uint8_t type_stack[] = {1, 2, 4};
    std::string out = type_stack_to_string(type_stack, stack_size);
    ASSERT_EQ(out, "type stack: [S32, S64, S128]");
}

TEST(TestCase, locals_to_string) {
    uint32_t type_stack_size = 3;
    uint8_t type_stack[] = {1, 2, 1};
    
    uint32_t locals_size = 3;
    uint32_t locals[] = {100, 0, 200, 300};
    
    Array8 type_stack_array = {type_stack_size, type_stack};
    Array32 locals_array = {locals_size, locals};
    ArrayStringResult out = locals_to_string(&type_stack_array, &locals_array);
    ASSERT_EQ(out.output, "locals: [100, 200, 300]");
}

TEST(TestCase, protobuf_array32) {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("serialize test");

    // array32: [1, 2, 3]
    Array32 array;
    array.size = 3;
    array.contents = (uint32_t*)malloc(sizeof(uint32_t) * array.size);
    array.contents[0] = 1;
    array.contents[1] = 2;
    array.contents[2] = 3;

    // serialize
    Array8 ret = serialize_array32(&array);
    
    // deserialize
    Array32 array2 = deserialize_array32(&ret);
    ASSERT_EQ(array2.size, 3);
    ASSERT_EQ(array2.contents[0], 1);
    ASSERT_EQ(array2.contents[1], 2);
    ASSERT_EQ(array2.contents[2], 3);

    free(array.contents);
}

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