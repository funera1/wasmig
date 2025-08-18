#include "wasmig/table_v3.h"
#include <gtest/gtest.h>
#include <stdio.h>

class TableV3Test : public ::testing::Test {
protected:
    void SetUp() override {
        // 禁止リスト用の64ビットアドレス
        forbidden_addr1 = 100;
        forbidden_addr2 = 300;
        forbidden_addr3 = 500;
        
        // 状態キュー用のオフセット
        offset1 = 100;
        offset2 = 200;
        offset3 = 300;
        
        // アドレスマップ用のキー・バリュー
        key1 = 100;
        key2 = 200;
        key3 = 300;
        
        value1 = 0x1000000000ULL;
        value2 = 0x2000000000ULL;
        value3 = 0x3000000000ULL;
    }
    
    uint64_t forbidden_addr1, forbidden_addr2, forbidden_addr3;
    uint32_t offset1, offset2, offset3;
    uint32_t key1, key2, key3;
    uint64_t value1, value2, value3;
};

// アドレスマップのテスト
TEST_F(TableV3Test, AddressMapBasicOperations) {
    AddressMap map = wasmig_address_map_create(16);
    ASSERT_NE(map, nullptr);
    
    // 初期状態
    EXPECT_EQ(wasmig_address_map_size(map), 0);
    
    // エントリの追加
    EXPECT_TRUE(wasmig_address_map_set(map, key1, value1));
    EXPECT_TRUE(wasmig_address_map_set(map, key2, value2));
    EXPECT_EQ(wasmig_address_map_size(map), 2);
    
    // キーからバリューを取得
    uint64_t retrieved_value;
    EXPECT_TRUE(wasmig_address_map_get(map, key1, &retrieved_value));
    EXPECT_EQ(retrieved_value, value1);
    
    EXPECT_TRUE(wasmig_address_map_get(map, key2, &retrieved_value));
    EXPECT_EQ(retrieved_value, value2);
    
    // 存在しないエントリ
    EXPECT_FALSE(wasmig_address_map_get(map, key3, &retrieved_value));
    
    // エントリの更新
    uint64_t new_value = 0x4000000000ULL;
    EXPECT_TRUE(wasmig_address_map_set(map, key1, new_value));
    EXPECT_EQ(wasmig_address_map_size(map), 2); // サイズは変わらない
    EXPECT_TRUE(wasmig_address_map_get(map, key1, &retrieved_value));
    EXPECT_EQ(retrieved_value, new_value);
    
    // エントリの削除
    EXPECT_TRUE(wasmig_address_map_remove(map, key1));
    EXPECT_FALSE(wasmig_address_map_get(map, key1, &retrieved_value));
    EXPECT_EQ(wasmig_address_map_size(map), 1);

    // 存在しないエントリの削除
    EXPECT_FALSE(wasmig_address_map_remove(map, key3));
    
    wasmig_address_map_destroy(map);
}

// 禁止リストのテスト
TEST_F(TableV3Test, ForbiddenListBasicOperations) {
    CheckpointForbiddenList list = wasmig_forbidden_list_create(16);
    ASSERT_NE(list, nullptr);
    
    // 初期状態
    EXPECT_EQ(wasmig_forbidden_list_size(list), 0);
    EXPECT_FALSE(wasmig_forbidden_list_contains(list, forbidden_addr1));
    
    // エントリの追加
    EXPECT_TRUE(wasmig_forbidden_list_add(list, forbidden_addr1));
    EXPECT_TRUE(wasmig_forbidden_list_add(list, forbidden_addr2));
    EXPECT_EQ(wasmig_forbidden_list_size(list), 2);
    
    // 存在チェック
    EXPECT_TRUE(wasmig_forbidden_list_contains(list, forbidden_addr1));
    EXPECT_TRUE(wasmig_forbidden_list_contains(list, forbidden_addr2));
    EXPECT_FALSE(wasmig_forbidden_list_contains(list, forbidden_addr3));
    
    // 重複追加（成功とみなす）
    EXPECT_TRUE(wasmig_forbidden_list_add(list, forbidden_addr1));
    EXPECT_EQ(wasmig_forbidden_list_size(list), 2); // サイズは変わらない

    // エントリの削除
    EXPECT_TRUE(wasmig_forbidden_list_remove(list, forbidden_addr1));
    EXPECT_FALSE(wasmig_forbidden_list_contains(list, forbidden_addr1));
    EXPECT_EQ(wasmig_forbidden_list_size(list), 1);

    // 存在しないエントリの削除
    EXPECT_FALSE(wasmig_forbidden_list_remove(list, forbidden_addr3));

    wasmig_forbidden_list_destroy(list);
}

// 状態管理キューのテスト
TEST_F(TableV3Test, StateQueueBasicOperations) {
    StateManagementQueue queue = wasmig_state_queue_create();
    ASSERT_NE(queue, nullptr);
    
    // 初期状態
    EXPECT_TRUE(wasmig_state_queue_is_empty(queue));
    EXPECT_EQ(wasmig_state_queue_size(queue), 0);
    
    // エントリの追加
    EXPECT_TRUE(wasmig_state_queue_enqueue(queue, offset1));
    EXPECT_TRUE(wasmig_state_queue_enqueue(queue, offset2));
    EXPECT_FALSE(wasmig_state_queue_is_empty(queue));
    EXPECT_EQ(wasmig_state_queue_size(queue), 2);
    
    // 確認
    EXPECT_TRUE(wasmig_state_queue_confirm_pending(queue, offset1));
    EXPECT_FALSE(wasmig_state_queue_confirm_pending(queue, offset3)); // 存在しない
    
    // エントリの取得（FIFO）
    uint32_t retrieved_offset;
    EXPECT_TRUE(wasmig_state_queue_dequeue(queue, &retrieved_offset));
    EXPECT_EQ(retrieved_offset, offset1);
    EXPECT_EQ(wasmig_state_queue_size(queue), 1);
    
    // 残りのエントリを取得
    EXPECT_TRUE(wasmig_state_queue_dequeue(queue, &retrieved_offset));
    EXPECT_EQ(retrieved_offset, offset2);
    EXPECT_TRUE(wasmig_state_queue_is_empty(queue));

    // 空のキューから取得
    EXPECT_FALSE(wasmig_state_queue_dequeue(queue, &retrieved_offset));

    wasmig_state_queue_destroy(queue);
}

// 印刷機能のテスト（出力を確認するため手動実行用）
TEST_F(TableV3Test, PrintFunctions) {
    printf("\n=== Print Functions Test ===\n");
    
    // アドレスマップの印刷
    AddressMap map = wasmig_address_map_create(16);
    wasmig_address_map_set(map, key1, value1);
    wasmig_address_map_set(map, key2, value2);
    wasmig_address_map_print(map);

    // 禁止リストの印刷
    CheckpointForbiddenList list = wasmig_forbidden_list_create(16);
    wasmig_forbidden_list_add(list, forbidden_addr1);
    wasmig_forbidden_list_add(list, forbidden_addr2);
    wasmig_forbidden_list_print(list);
    
    // 状態キューの印刷
    StateManagementQueue queue = wasmig_state_queue_create();
    wasmig_state_queue_enqueue(queue, offset1);
    wasmig_state_queue_enqueue(queue, offset2);
    wasmig_state_queue_confirm_pending(queue, offset1);
    wasmig_state_queue_print(queue);

    // クリーンアップ
    wasmig_address_map_destroy(map);
    wasmig_forbidden_list_destroy(list);
    wasmig_state_queue_destroy(queue);

    printf("=============================\n\n");
}
