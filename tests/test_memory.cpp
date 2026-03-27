#include <gtest/gtest.h>
#include <wasmig/migration.h>
#include <wasmig/utils.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

namespace {

class ScopedWorkDir {
public:
    explicit ScopedWorkDir(const fs::path& path) : old_path_(fs::current_path()) {
        fs::create_directories(path);
        fs::current_path(path);
    }

    ~ScopedWorkDir() {
        fs::current_path(old_path_);
    }

private:
    fs::path old_path_;
};

class MemoryCheckpointTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() /
                    ("wasmig-memory-test-" + std::to_string(static_cast<long long>(getpid())) +
                     "-" + std::to_string(counter_++));
        fs::create_directories(test_dir_);
    }

    void TearDown() override {
        fs::remove_all(test_dir_);
    }

    static size_t file_size(const fs::path& path) {
        return static_cast<size_t>(fs::file_size(path));
    }

    fs::path test_dir_;

private:
    inline static int counter_ = 0;
};

TEST_F(MemoryCheckpointTest, CheckpointSkipsZero4KBChunksAndRestoreReconstructsMemory) {
    ScopedWorkDir work_dir(test_dir_);

    const uint32_t page_count = 3;
    std::vector<uint8_t> memory(static_cast<size_t>(page_count) * WASM_PAGE_SIZE, 0);
    memory[17] = 0xAB;
    memory[static_cast<size_t>(WASM_PAGE_SIZE) * 2 + 99] = 0xCD;

    ASSERT_EQ(wasmig_checkpoint_memory(memory.data(), page_count), 0);

    constexpr size_t kChunkSize = 4096;
    EXPECT_EQ(file_size(test_dir_ / "memory.img"), static_cast<size_t>(2) * (sizeof(uint32_t) + kChunkSize));
    EXPECT_EQ(file_size(test_dir_ / "mem_page_count.img"), sizeof(uint32_t) * 2);

    Array8 restored = wasmig_restore_memory();
    ASSERT_NE(restored.contents, nullptr);
    ASSERT_EQ(restored.size, memory.size());
    EXPECT_EQ(std::memcmp(restored.contents, memory.data(), memory.size()), 0);

    free(restored.contents);
}

TEST_F(MemoryCheckpointTest, RestoreSupportsLegacyDenseMemoryImage) {
    ScopedWorkDir work_dir(test_dir_);

    const uint32_t page_count = 2;
    std::vector<uint8_t> memory(static_cast<size_t>(page_count) * WASM_PAGE_SIZE, 0);
    memory[1] = 0x11;
    memory[WASM_PAGE_SIZE + 2] = 0x22;

    {
        std::ofstream mem_size("mem_page_count.img", std::ios::binary);
        ASSERT_TRUE(mem_size.is_open());
        mem_size.write(reinterpret_cast<const char*>(&page_count), sizeof(page_count));
    }
    {
        std::ofstream mem_file("memory.img", std::ios::binary);
        ASSERT_TRUE(mem_file.is_open());
        mem_file.write(reinterpret_cast<const char*>(memory.data()), static_cast<std::streamsize>(memory.size()));
    }

    Array8 restored = wasmig_restore_memory();
    ASSERT_NE(restored.contents, nullptr);
    ASSERT_EQ(restored.size, memory.size());
    EXPECT_EQ(std::memcmp(restored.contents, memory.data(), memory.size()), 0);

    free(restored.contents);
}

}  // namespace
