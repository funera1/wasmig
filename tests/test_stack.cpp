#include <gtest/gtest.h>
#include <wasmig/stack.h>
#include <vector>

class StackTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テスト前の準備
    }

    void TearDown() override {
        // テスト後のクリーンアップ
    }
};

// Helper callback for wasmig_stack_foreach tests
static void collect_cb(uint64_t value, void* user) {
    auto* v = static_cast<std::vector<uint64_t>*>(user);
    v->push_back(value);
}

// 基本的なスタック操作のテスト
TEST_F(StackTest, BasicStackOperations) {
    // stack_create()のテスト
    Stack stack = wasmig_stack_create();
    EXPECT_TRUE(wasmig_stack_is_empty(stack));
    EXPECT_EQ(wasmig_stack_size(stack), 0);
    EXPECT_EQ(wasmig_stack_top(stack), 0);
    
    // stack_empty()との同等性テスト: どちらも空であるが、ポインタは異なる
    Stack empty_stack = wasmig_stack_empty();
    EXPECT_TRUE(wasmig_stack_is_empty(empty_stack));
    EXPECT_TRUE(wasmig_stack_is_empty(stack));
    
    // 空のスタックのテスト
    EXPECT_TRUE(wasmig_stack_is_empty(stack));
    EXPECT_EQ(wasmig_stack_size(stack), 0);
    EXPECT_EQ(wasmig_stack_top(stack), 0);
    
    // push操作のテスト
    stack = wasmig_stack_push(stack, 10);
    EXPECT_FALSE(wasmig_stack_is_empty(stack));
    EXPECT_EQ(wasmig_stack_size(stack), 1);
    EXPECT_EQ(wasmig_stack_top(stack), 10);
    
    stack = wasmig_stack_push(stack, 20);
    EXPECT_EQ(wasmig_stack_size(stack), 2);
    EXPECT_EQ(wasmig_stack_top(stack), 20);
    
    stack = wasmig_stack_push(stack, 30);
    EXPECT_EQ(wasmig_stack_size(stack), 3);
    EXPECT_EQ(wasmig_stack_top(stack), 30);
    
    // pop操作のテスト
    uint64_t value;
    stack = wasmig_stack_pop(stack, &value);
    EXPECT_EQ(value, 30);
    EXPECT_EQ(wasmig_stack_size(stack), 2);
    EXPECT_EQ(wasmig_stack_top(stack), 20);
    
    stack = wasmig_stack_pop(stack, &value);
    EXPECT_EQ(value, 20);
    EXPECT_EQ(wasmig_stack_size(stack), 1);
    EXPECT_EQ(wasmig_stack_top(stack), 10);
    
    stack = wasmig_stack_pop(stack, &value);
    EXPECT_EQ(value, 10);
    EXPECT_EQ(wasmig_stack_size(stack), 0);
    EXPECT_TRUE(wasmig_stack_is_empty(stack));
    
    // 空のスタックからのpop
    stack = wasmig_stack_pop(stack, &value);
    EXPECT_EQ(value, 0);
    EXPECT_TRUE(wasmig_stack_is_empty(stack));
    
    wasmig_stack_destroy(stack);
}

// 永続性のテスト
TEST_F(StackTest, PersistentStack) {
    Stack stack1 = wasmig_stack_empty();
    stack1 = wasmig_stack_push(stack1, 10);
    stack1 = wasmig_stack_push(stack1, 20);
    
    // stack1の状態を保存
    Stack stack2 = stack1;
    
    // stack1を更に操作
    stack1 = wasmig_stack_push(stack1, 30);
    
    // stack2は元の状態を保持している
    EXPECT_EQ(wasmig_stack_size(stack1), 3);
    EXPECT_EQ(wasmig_stack_size(stack2), 2);
    EXPECT_EQ(wasmig_stack_top(stack1), 30);
    EXPECT_EQ(wasmig_stack_top(stack2), 20);
    
    wasmig_stack_destroy(stack1);
    wasmig_stack_destroy(stack2);
}

// スタック状態マップのテスト
TEST_F(StackTest, StackStateMap) {
    StackStateMap map = wasmig_stack_state_map_create();
    ASSERT_NE(map, nullptr);
    
    // 初期状態のテスト
    EXPECT_FALSE(wasmig_stack_state_exists(map, 1));
    {
    Stack loaded = wasmig_stack_empty();
    Stack loaded_type = wasmig_stack_empty();
    bool ok = wasmig_stack_state_load_pair(map, 1, &loaded, &loaded_type);
        // missing key -> not ok, loaded remains empty
        EXPECT_TRUE(wasmig_stack_is_empty(loaded));
        if (ok) { wasmig_stack_destroy(loaded); wasmig_stack_destroy(loaded_type); }
    }
    
    // スタックを作成して保存
        Stack stack = wasmig_stack_empty();
    stack = wasmig_stack_push(stack, 100);
    stack = wasmig_stack_push(stack, 200);
    stack = wasmig_stack_push(stack, 300);

    EXPECT_TRUE(wasmig_stack_state_save_pair(map, 1, stack, wasmig_stack_empty()));
    EXPECT_TRUE(wasmig_stack_state_exists(map, 1));
    // 保存した状態をロード
    {
    Stack loaded_stack = wasmig_stack_empty();
    Stack loaded_type = wasmig_stack_empty();
    bool ok = wasmig_stack_state_load_pair(map, 1, &loaded_stack, &loaded_type);
    EXPECT_TRUE(ok);
    EXPECT_EQ(wasmig_stack_size(loaded_stack), 3);
    EXPECT_EQ(wasmig_stack_top(loaded_stack), 300);

        uint64_t value;
    loaded_stack = wasmig_stack_pop(loaded_stack, &value);
    EXPECT_EQ(value, 300);
    loaded_stack = wasmig_stack_pop(loaded_stack, &value);
    EXPECT_EQ(value, 200);
    loaded_stack = wasmig_stack_pop(loaded_stack, &value);
    EXPECT_EQ(value, 100);
    EXPECT_TRUE(wasmig_stack_is_empty(loaded_stack));

    // cleanup loaded pair
    wasmig_stack_destroy(loaded_stack);
    wasmig_stack_destroy(loaded_type);
    }
    
    wasmig_stack_destroy(stack);
    wasmig_stack_state_map_destroy(map);
}

// 複数のチェックポイントのテスト
TEST_F(StackTest, MultipleCheckpoints) {
    StackStateMap map = wasmig_stack_state_map_create();
    
    // 異なる状態のスタックを複数保存
    Stack stack1 = wasmig_stack_empty();
    stack1 = wasmig_stack_push(stack1, 10);
    wasmig_stack_state_save_pair(map, 1, stack1, wasmig_stack_empty());
    
    Stack stack2 = stack1;
    stack2 = wasmig_stack_push(stack2, 20);
    wasmig_stack_state_save_pair(map, 2, stack2, wasmig_stack_empty());
    
    Stack stack3 = stack2;
    stack3 = wasmig_stack_push(stack3, 30);
    wasmig_stack_state_save_pair(map, 3, stack3, wasmig_stack_empty());
    
    // それぞれの状態を確認
    Stack loaded1 = wasmig_stack_empty(); Stack loaded1_type = wasmig_stack_empty();
    Stack loaded2 = wasmig_stack_empty(); Stack loaded2_type = wasmig_stack_empty();
    Stack loaded3 = wasmig_stack_empty(); Stack loaded3_type = wasmig_stack_empty();
    EXPECT_TRUE(wasmig_stack_state_load_pair(map, 1, &loaded1, &loaded1_type));
    EXPECT_TRUE(wasmig_stack_state_load_pair(map, 2, &loaded2, &loaded2_type));
    EXPECT_TRUE(wasmig_stack_state_load_pair(map, 3, &loaded3, &loaded3_type));
    
    EXPECT_EQ(wasmig_stack_size(loaded1), 1);
    EXPECT_EQ(wasmig_stack_top(loaded1), 10);
    
    EXPECT_EQ(wasmig_stack_size(loaded2), 2);
    EXPECT_EQ(wasmig_stack_top(loaded2), 20);
    
    EXPECT_EQ(wasmig_stack_size(loaded3), 3);
    EXPECT_EQ(wasmig_stack_top(loaded3), 30);
    
    // 状態の削除テスト
    EXPECT_TRUE(wasmig_stack_state_remove(map, 2));
    EXPECT_FALSE(wasmig_stack_state_exists(map, 2));
    EXPECT_TRUE(wasmig_stack_state_exists(map, 1));
    EXPECT_TRUE(wasmig_stack_state_exists(map, 3));
    
    // 存在しないキーの削除
    EXPECT_FALSE(wasmig_stack_state_remove(map, 999999u));
    
    wasmig_stack_destroy(stack1);
    wasmig_stack_destroy(stack2);
    wasmig_stack_destroy(stack3);
    // destroy loaded pairs
    wasmig_stack_destroy(loaded1); wasmig_stack_destroy(loaded1_type);
    wasmig_stack_destroy(loaded2); wasmig_stack_destroy(loaded2_type);
    wasmig_stack_destroy(loaded3); wasmig_stack_destroy(loaded3_type);
    wasmig_stack_state_map_destroy(map);
}

// キーの上書きテスト
TEST_F(StackTest, OverwriteCheckpoint) {
    StackStateMap map = wasmig_stack_state_map_create();
    
    // 最初の状態を保存
    Stack stack1 = wasmig_stack_empty();
    stack1 = wasmig_stack_push(stack1, 10);
    wasmig_stack_state_save_pair(map, 42, stack1, wasmig_stack_empty());
    
    {
    Stack loaded = wasmig_stack_empty(); Stack loaded_type = wasmig_stack_empty();
    EXPECT_TRUE(wasmig_stack_state_load_pair(map, 42, &loaded, &loaded_type));
    EXPECT_EQ(wasmig_stack_size(loaded), 1);
    EXPECT_EQ(wasmig_stack_top(loaded), 10);
    wasmig_stack_destroy(loaded); wasmig_stack_destroy(loaded_type);
    }
    
    // 同じキーで別の状態を保存（上書き）
    Stack stack2 = wasmig_stack_empty();
    stack2 = wasmig_stack_push(stack2, 20);
    stack2 = wasmig_stack_push(stack2, 30);
    wasmig_stack_state_save_pair(map, 42, stack2, wasmig_stack_empty());
    {
    Stack loaded = wasmig_stack_empty(); Stack loaded_type = wasmig_stack_empty();
    EXPECT_TRUE(wasmig_stack_state_load_pair(map, 42, &loaded, &loaded_type));
    EXPECT_EQ(wasmig_stack_size(loaded), 2);
    EXPECT_EQ(wasmig_stack_top(loaded), 30);
    wasmig_stack_destroy(loaded); wasmig_stack_destroy(loaded_type);
    }
    
    wasmig_stack_destroy(stack1);
    wasmig_stack_destroy(stack2);
    wasmig_stack_state_map_destroy(map);
}

// エラーハンドリングのテスト
TEST_F(StackTest, ErrorHandling) {
    // NULLポインタのテスト
    StackStateMap map = wasmig_stack_state_map_create();
    EXPECT_NE(map, nullptr); // 正常に作成される
    
    // 無効な引数でのテスト
    EXPECT_FALSE(wasmig_stack_state_save_pair(nullptr, 1, wasmig_stack_empty(), wasmig_stack_empty()));
    EXPECT_FALSE(wasmig_stack_state_exists(nullptr, 1));
    EXPECT_FALSE(wasmig_stack_state_remove(nullptr, 1));
    
    {
    Stack loaded = wasmig_stack_empty(); Stack loaded_type = wasmig_stack_empty();
    EXPECT_FALSE(wasmig_stack_state_load_pair(nullptr, 1, &loaded, &loaded_type));
    }
    {
    Stack loaded = wasmig_stack_empty(); Stack loaded_type = wasmig_stack_empty();
    EXPECT_FALSE(wasmig_stack_state_load_pair(map, 0, &loaded, &loaded_type));
    }
    
    wasmig_stack_state_map_destroy(map);
    wasmig_stack_state_map_destroy(nullptr); // 安全に呼び出せる
}

// ハッシュ衝突のテスト
TEST_F(StackTest, HashCollision) {
    StackStateMap map = wasmig_stack_state_map_create();
    
    // 多数のキーで状態を保存してハッシュ衝突をテスト
    for (int i = 0; i < 1000; i++) {
        Stack stack = wasmig_stack_empty();
        stack = wasmig_stack_push(stack, i);
    EXPECT_TRUE(wasmig_stack_state_save_pair(map, static_cast<uint32_t>(i), stack, wasmig_stack_empty()));
        EXPECT_TRUE(wasmig_stack_state_exists(map, static_cast<uint32_t>(i)));
    Stack loaded = wasmig_stack_empty(); Stack loaded_type = wasmig_stack_empty();
    EXPECT_TRUE(wasmig_stack_state_load_pair(map, static_cast<uint32_t>(i), &loaded, &loaded_type));
    EXPECT_EQ(wasmig_stack_top(loaded), i);
    wasmig_stack_destroy(loaded); wasmig_stack_destroy(loaded_type);
        
        wasmig_stack_destroy(stack);
    }
    
    // すべてのキーが正しく保存されているか確認
    for (int i = 0; i < 1000; i++) {
    EXPECT_TRUE(wasmig_stack_state_exists(map, static_cast<uint32_t>(i)));
    Stack loaded = wasmig_stack_empty(); Stack loaded_type = wasmig_stack_empty();
    EXPECT_TRUE(wasmig_stack_state_load_pair(map, static_cast<uint32_t>(i), &loaded, &loaded_type));
    EXPECT_EQ(wasmig_stack_top(loaded), i);
    wasmig_stack_destroy(loaded); wasmig_stack_destroy(loaded_type);
    }
    
    wasmig_stack_state_map_destroy(map);
}

// stack_create関数の専用テスト
TEST_F(StackTest, StackCreate) {
    // stack_create()の基本テスト
    Stack stack1 = wasmig_stack_create();
    EXPECT_TRUE(wasmig_stack_is_empty(stack1));
    EXPECT_EQ(wasmig_stack_size(stack1), 0);
    
    // 複数回の呼び出しテスト
    Stack stack2 = wasmig_stack_create();
    Stack stack3 = wasmig_stack_create();
    
    // それぞれ独立した空スタック（ポインタは異なる）が返ること
    EXPECT_NE(stack1, stack2);
    EXPECT_NE(stack2, stack3);
    
    // stack_empty()はグローバルな空スタック（ポインタは異なる可能性あり）
    Stack empty_stack = wasmig_stack_empty();
    EXPECT_TRUE(wasmig_stack_is_empty(empty_stack));
    
    // 作成したスタックに対する操作テスト
    Stack new_stack = wasmig_stack_push(stack1, 42);
    EXPECT_FALSE(wasmig_stack_is_empty(new_stack));
    EXPECT_EQ(wasmig_stack_top(new_stack), 42);
    EXPECT_EQ(wasmig_stack_size(new_stack), 1);
    
    // 元のスタックは変更されていないことを確認
    EXPECT_TRUE(wasmig_stack_is_empty(stack1));
    
    wasmig_stack_destroy(new_stack);
}

// 非破壊イテレータのテスト
TEST_F(StackTest, IteratorNonDestructive) {
    Stack stack = wasmig_stack_empty();
    stack = wasmig_stack_push(stack, 10);
    stack = wasmig_stack_push(stack, 20);
    stack = wasmig_stack_push(stack, 30);

    // 元のスタックは変化しないことを確認
    size_t orig_size = wasmig_stack_size(stack);
    uint64_t orig_top = wasmig_stack_top(stack);

    std::vector<uint64_t> seen;
    StackIterator it = wasmig_stack_iterator_create(stack);
    ASSERT_NE(it, nullptr);

    while (wasmig_stack_iterator_has_next(it)) {
        uint64_t p = wasmig_stack_iterator_peek(it);
        uint64_t n = wasmig_stack_iterator_next(it);
        EXPECT_EQ(p, n);
        seen.push_back(n);
    }

    wasmig_stack_iterator_destroy(it);

    // pushで積んだ順にトップから列挙されるはず
    std::vector<uint64_t> expected = {30, 20, 10};
    EXPECT_EQ(seen, expected);

    // 元のスタックはそのまま
    EXPECT_EQ(wasmig_stack_size(stack), orig_size);
    EXPECT_EQ(wasmig_stack_top(stack), orig_top);

    wasmig_stack_destroy(stack);
}

// foreach コールバックのテスト
TEST_F(StackTest, IteratorForeach) {
    Stack stack = wasmig_stack_empty();
    stack = wasmig_stack_push(stack, 1);
    stack = wasmig_stack_push(stack, 2);
    stack = wasmig_stack_push(stack, 3);

    std::vector<uint64_t> collected;
    wasmig_stack_foreach(stack, collect_cb, &collected);

    std::vector<uint64_t> expected = {3,2,1};
    EXPECT_EQ(collected, expected);

    wasmig_stack_destroy(stack);
}
