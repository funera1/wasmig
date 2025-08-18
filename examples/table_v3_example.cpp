#include "wasmig/table_v3.h"
#include <stdio.h>

/**
 * Table V3 使用例
 * 
 * この例では、3つのデータ構造の基本的な使用方法を示しています：
 * 1. アドレスマップ: u32キーとu64バリューのマッピング
 * 2. チェックポイント禁止リスト: 特定のアドレスでのチェックポイント禁止
 * 3. 状態管理キュー: 未確定命令の一時保持と確定処理
 */

void example_address_map() {
    printf("\n=== アドレスマップの例 ===\n");
    
    // アドレスマップを作成
    AddressMap map = wasmig_address_map_create(32);

    // キーとバリューを定義
    uint32_t key1 = 100;
    uint32_t key2 = 200;
    uint64_t value1 = 0x1000000000ULL;
    uint64_t value2 = 0x2000000000ULL;
    
    // マッピングを追加
    wasmig_address_map_set(map, key1, value1);
    wasmig_address_map_set(map, key2, value2);
    
    printf("登録されたマッピング数: %zu\n", wasmig_address_map_size(map));
    
    // キーからバリューを取得
    uint64_t retrieved_value;
    if (wasmig_address_map_get(map, key1, &retrieved_value)) {
        printf("キー %u -> バリュー %lu\n", key1, retrieved_value);
    }
    
    if (wasmig_address_map_get(map, key2, &retrieved_value)) {
        printf("キー %u -> バリュー %lu\n", key2, retrieved_value);
    }
    
    // エントリの更新
    uint64_t new_value = 0x3000000000ULL;
    wasmig_address_map_set(map, key1, new_value);
    printf("キー %u を更新\n", key1);
    
    // エントリの削除
    wasmig_address_map_remove(map, key2);
    printf("キー %u を削除\n", key2);
    printf("削除後のマッピング数: %zu\n", wasmig_address_map_size(map));

    // マップの内容を印刷
    wasmig_address_map_print(map);
    
    wasmig_address_map_destroy(map);
}

void example_forbidden_list() {
    printf("\n=== チェックポイント禁止リストの例 ===\n");
    
    // 禁止リストを作成
    CheckpointForbiddenList* list = wasmig_forbidden_list_create(32);
    
    // 禁止する64ビットアドレスを定義
    uint64_t forbidden_addr1 = 50;   // アドレス50
    uint64_t forbidden_addr2 = 150;  // アドレス150
    uint64_t allowed_addr = 75;      // 許可されるアドレス
    
    // 禁止リストに追加
    wasmig_forbidden_list_add(list, forbidden_addr1);
    wasmig_forbidden_list_add(list, forbidden_addr2);
    
    printf("禁止リストのサイズ: %zu\n", wasmig_forbidden_list_size(list));
    
    // チェックポイント許可確認
    printf("アドレス%luでのチェックポイント: %s\n", 
           forbidden_addr1,
           wasmig_forbidden_list_contains(list, forbidden_addr1) ? "禁止" : "許可");
    
    printf("アドレス%luでのチェックポイント: %s\n", 
           allowed_addr,
           wasmig_forbidden_list_contains(list, allowed_addr) ? "禁止" : "許可");
    
    // リストの内容を印刷
    wasmig_forbidden_list_print(list);
    
    // 禁止を解除
    wasmig_forbidden_list_remove(list, forbidden_addr1);
    printf("禁止解除後のサイズ: %zu\n", wasmig_forbidden_list_size(list));

    wasmig_forbidden_list_destroy(list);
}

void example_state_queue() {
    printf("\n=== 状態管理キューの例 ===\n");
    
    // 状態管理キューを作成
    StateManagementQueue* queue = wasmig_state_queue_create();

    // 未確定のオフセットを定義
    uint32_t pending_offset1 = 300;
    uint32_t pending_offset2 = 400;
    
    // キューに追加
    wasmig_state_queue_enqueue(queue, pending_offset1);
    wasmig_state_queue_enqueue(queue, pending_offset2);

    printf("キューサイズ: %zu\n", wasmig_state_queue_size(queue));

    // 未確定命令を確認
    printf("オフセット%uを確認中...\n", pending_offset1);
    wasmig_state_queue_confirm_pending(queue, pending_offset1);
    
    // キューの内容を印刷
    wasmig_state_queue_print(queue);
    
    // キューから取得（FIFO順）
    uint32_t dequeued_offset;

    while (!wasmig_state_queue_is_empty(queue)) {
        if (wasmig_state_queue_dequeue(queue, &dequeued_offset)) {
            printf("デキュー: オフセット%u\n", dequeued_offset);
        }
    }
    
    wasmig_state_queue_destroy(queue);
}

int main() {
    printf("Table V3 使用例\n");
    printf("================\n");
    
    // 各データ構造の使用例を実行
    example_address_map();
    example_forbidden_list();
    example_state_queue();
    
    printf("\n全ての例が完了しました。\n");
    return 0;
}
