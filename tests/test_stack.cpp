#include <gtest/gtest.h>
#include <wasmig/stack.h>

class StackTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テスト前の準備
    }

    void TearDown() override {
        // テスト後のクリーンアップ
    }
};

// 基本的なスタック操作のテスト
TEST_F(StackTest, BasicStackOperations) {
    Stack stack = stack_empty();
    
    // 空のスタックのテスト
    EXPECT_TRUE(stack_is_empty(stack));
    EXPECT_EQ(stack_size(stack), 0);
    EXPECT_EQ(stack_top(stack), 0);
    
    // push操作のテスト
    stack = stack_push(stack, 10);
    EXPECT_FALSE(stack_is_empty(stack));
    EXPECT_EQ(stack_size(stack), 1);
    EXPECT_EQ(stack_top(stack), 10);
    
    stack = stack_push(stack, 20);
    EXPECT_EQ(stack_size(stack), 2);
    EXPECT_EQ(stack_top(stack), 20);
    
    stack = stack_push(stack, 30);
    EXPECT_EQ(stack_size(stack), 3);
    EXPECT_EQ(stack_top(stack), 30);
    
    // pop操作のテスト
    uint64_t value;
    stack = stack_pop(stack, &value);
    EXPECT_EQ(value, 30);
    EXPECT_EQ(stack_size(stack), 2);
    EXPECT_EQ(stack_top(stack), 20);
    
    stack = stack_pop(stack, &value);
    EXPECT_EQ(value, 20);
    EXPECT_EQ(stack_size(stack), 1);
    EXPECT_EQ(stack_top(stack), 10);
    
    stack = stack_pop(stack, &value);
    EXPECT_EQ(value, 10);
    EXPECT_EQ(stack_size(stack), 0);
    EXPECT_TRUE(stack_is_empty(stack));
    
    // 空のスタックからのpop
    stack = stack_pop(stack, &value);
    EXPECT_EQ(value, 0);
    EXPECT_TRUE(stack_is_empty(stack));
    
    stack_destroy(stack);
}

// 永続性のテスト
TEST_F(StackTest, PersistentStack) {
    Stack stack1 = stack_empty();
    stack1 = stack_push(stack1, 10);
    stack1 = stack_push(stack1, 20);
    
    // stack1の状態を保存
    Stack stack2 = stack1;
    
    // stack1を更に操作
    stack1 = stack_push(stack1, 30);
    
    // stack2は元の状態を保持している
    EXPECT_EQ(stack_size(stack1), 3);
    EXPECT_EQ(stack_size(stack2), 2);
    EXPECT_EQ(stack_top(stack1), 30);
    EXPECT_EQ(stack_top(stack2), 20);
    
    stack_destroy(stack1);
    stack_destroy(stack2);
}

// スタック状態マップのテスト
TEST_F(StackTest, StackStateMap) {
    StackStateMap map = stack_state_map_create();
    ASSERT_NE(map, nullptr);
    
    // 初期状態のテスト
    EXPECT_FALSE(stack_state_exists(map, "checkpoint1"));
    Stack loaded = stack_state_load(map, "checkpoint1");
    EXPECT_TRUE(stack_is_empty(loaded));
    
    // スタックを作成して保存
    Stack stack = stack_empty();
    stack = stack_push(stack, 100);
    stack = stack_push(stack, 200);
    stack = stack_push(stack, 300);
    
    EXPECT_TRUE(stack_state_save(map, "checkpoint1", stack));
    EXPECT_TRUE(stack_state_exists(map, "checkpoint1"));
    
    // 保存した状態をロード
    Stack loaded_stack = stack_state_load(map, "checkpoint1");
    EXPECT_EQ(stack_size(loaded_stack), 3);
    EXPECT_EQ(stack_top(loaded_stack), 300);
    
    uint64_t value;
    loaded_stack = stack_pop(loaded_stack, &value);
    EXPECT_EQ(value, 300);
    loaded_stack = stack_pop(loaded_stack, &value);
    EXPECT_EQ(value, 200);
    loaded_stack = stack_pop(loaded_stack, &value);
    EXPECT_EQ(value, 100);
    EXPECT_TRUE(stack_is_empty(loaded_stack));
    
    stack_destroy(stack);
    stack_state_map_destroy(map);
}

// 複数のチェックポイントのテスト
TEST_F(StackTest, MultipleCheckpoints) {
    StackStateMap map = stack_state_map_create();
    
    // 異なる状態のスタックを複数保存
    Stack stack1 = stack_empty();
    stack1 = stack_push(stack1, 10);
    stack_state_save(map, "state1", stack1);
    
    Stack stack2 = stack1;
    stack2 = stack_push(stack2, 20);
    stack_state_save(map, "state2", stack2);
    
    Stack stack3 = stack2;
    stack3 = stack_push(stack3, 30);
    stack_state_save(map, "state3", stack3);
    
    // それぞれの状態を確認
    Stack loaded1 = stack_state_load(map, "state1");
    Stack loaded2 = stack_state_load(map, "state2");
    Stack loaded3 = stack_state_load(map, "state3");
    
    EXPECT_EQ(stack_size(loaded1), 1);
    EXPECT_EQ(stack_top(loaded1), 10);
    
    EXPECT_EQ(stack_size(loaded2), 2);
    EXPECT_EQ(stack_top(loaded2), 20);
    
    EXPECT_EQ(stack_size(loaded3), 3);
    EXPECT_EQ(stack_top(loaded3), 30);
    
    // 状態の削除テスト
    EXPECT_TRUE(stack_state_remove(map, "state2"));
    EXPECT_FALSE(stack_state_exists(map, "state2"));
    EXPECT_TRUE(stack_state_exists(map, "state1"));
    EXPECT_TRUE(stack_state_exists(map, "state3"));
    
    // 存在しないキーの削除
    EXPECT_FALSE(stack_state_remove(map, "nonexistent"));
    
    stack_destroy(stack1);
    stack_destroy(stack2);
    stack_destroy(stack3);
    stack_state_map_destroy(map);
}

// キーの上書きテスト
TEST_F(StackTest, OverwriteCheckpoint) {
    StackStateMap map = stack_state_map_create();
    
    // 最初の状態を保存
    Stack stack1 = stack_empty();
    stack1 = stack_push(stack1, 10);
    stack_state_save(map, "checkpoint", stack1);
    
    Stack loaded = stack_state_load(map, "checkpoint");
    EXPECT_EQ(stack_size(loaded), 1);
    EXPECT_EQ(stack_top(loaded), 10);
    
    // 同じキーで別の状態を保存（上書き）
    Stack stack2 = stack_empty();
    stack2 = stack_push(stack2, 20);
    stack2 = stack_push(stack2, 30);
    stack_state_save(map, "checkpoint", stack2);
    
    loaded = stack_state_load(map, "checkpoint");
    EXPECT_EQ(stack_size(loaded), 2);
    EXPECT_EQ(stack_top(loaded), 30);
    
    stack_destroy(stack1);
    stack_destroy(stack2);
    stack_state_map_destroy(map);
}

// エラーハンドリングのテスト
TEST_F(StackTest, ErrorHandling) {
    // NULLポインタのテスト
    StackStateMap map = stack_state_map_create();
    EXPECT_NE(map, nullptr); // 正常に作成される
    
    // 無効な引数でのテスト
    EXPECT_FALSE(stack_state_save(nullptr, "key", stack_empty()));
    EXPECT_FALSE(stack_state_save(map, nullptr, stack_empty()));
    EXPECT_FALSE(stack_state_exists(nullptr, "key"));
    EXPECT_FALSE(stack_state_exists(map, nullptr));
    EXPECT_FALSE(stack_state_remove(nullptr, "key"));
    EXPECT_FALSE(stack_state_remove(map, nullptr));
    
    Stack loaded = stack_state_load(nullptr, "key");
    EXPECT_TRUE(stack_is_empty(loaded));
    
    loaded = stack_state_load(map, nullptr);
    EXPECT_TRUE(stack_is_empty(loaded));
    
    stack_state_map_destroy(map);
    stack_state_map_destroy(nullptr); // 安全に呼び出せる
}

// ハッシュ衝突のテスト
TEST_F(StackTest, HashCollision) {
    StackStateMap map = stack_state_map_create();
    
    // 多数のキーで状態を保存してハッシュ衝突をテスト
    for (int i = 0; i < 1000; i++) {
        Stack stack = stack_empty();
        stack = stack_push(stack, i);
        
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        
        EXPECT_TRUE(stack_state_save(map, key, stack));
        EXPECT_TRUE(stack_state_exists(map, key));
        
        Stack loaded = stack_state_load(map, key);
        EXPECT_EQ(stack_top(loaded), i);
        
        stack_destroy(stack);
    }
    
    // すべてのキーが正しく保存されているか確認
    for (int i = 0; i < 1000; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        
        EXPECT_TRUE(stack_state_exists(map, key));
        Stack loaded = stack_state_load(map, key);
        EXPECT_EQ(stack_top(loaded), i);
    }
    
    stack_state_map_destroy(map);
}
