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
    AddressMap* map = address_map_create(32);
    
    // キーとバリューを定義
    uint32_t key1 = 100;
    uint32_t key2 = 200;
    uint64_t value1 = 0x1000000000ULL;
    uint64_t value2 = 0x2000000000ULL;
    
    // マッピングを追加
    address_map_set(map, key1, value1);
    address_map_set(map, key2, value2);
    
    printf("登録されたマッピング数: %zu\n", address_map_size(map));
    
    // キーからバリューを取得
    uint64_t retrieved_value;
    if (address_map_get(map, key1, &retrieved_value)) {
        printf("キー %u -> バリュー %lu\n", key1, retrieved_value);
    }
    
    if (address_map_get(map, key2, &retrieved_value)) {
        printf("キー %u -> バリュー %lu\n", key2, retrieved_value);
    }
    
    // エントリの更新
    uint64_t new_value = 0x3000000000ULL;
    address_map_set(map, key1, new_value);
    printf("キー %u を更新\n", key1);
    
    // エントリの削除
    address_map_remove(map, key2);
    printf("キー %u を削除\n", key2);
    printf("削除後のマッピング数: %zu\n", address_map_size(map));
    
    // マップの内容を印刷
    address_map_print(map);
    
    address_map_destroy(map);
}

void example_forbidden_list() {
    printf("\n=== チェックポイント禁止リストの例 ===\n");
    
    // 禁止リストを作成
    CheckpointForbiddenList* list = forbidden_list_create(32);
    
    // 禁止する64ビットアドレスを定義
    ForbiddenAddress forbidden_addr1 = 50;   // アドレス50
    ForbiddenAddress forbidden_addr2 = 150;  // アドレス150
    ForbiddenAddress allowed_addr = 75;      // 許可されるアドレス
    
    // 禁止リストに追加
    forbidden_list_add(list, forbidden_addr1);
    forbidden_list_add(list, forbidden_addr2);
    
    printf("禁止リストのサイズ: %zu\n", forbidden_list_size(list));
    
    // チェックポイント許可確認
    printf("アドレス%luでのチェックポイント: %s\n", 
           forbidden_addr1,
           forbidden_list_contains(list, forbidden_addr1) ? "禁止" : "許可");
    
    printf("アドレス%luでのチェックポイント: %s\n", 
           allowed_addr,
           forbidden_list_contains(list, allowed_addr) ? "禁止" : "許可");
    
    // リストの内容を印刷
    forbidden_list_print(list);
    
    // 禁止を解除
    forbidden_list_remove(list, forbidden_addr1);
    printf("禁止解除後のサイズ: %zu\n", forbidden_list_size(list));
    
    forbidden_list_destroy(list);
}

void example_state_queue() {
    printf("\n=== 状態管理キューの例 ===\n");
    
    // 状態管理キューを作成
    StateManagementQueue* queue = state_queue_create();
    
    // 未確定の命令を定義
    InstructionAddress pending_addr1 = {1, 300};
    InstructionAddress pending_addr2 = {2, 400};
    
    IntermediateRepresentation pending_ir1 = {0x10, 3, 0};   // call 3
    IntermediateRepresentation pending_ir2 = {0x1A, 0, 0};   // drop
    
    // キューに追加
    state_queue_enqueue(queue, pending_addr1, pending_ir1);
    state_queue_enqueue(queue, pending_addr2, pending_ir2);
    
    printf("キューサイズ: %zu\n", state_queue_size(queue));
    
    // 未確定命令を確認
    printf("命令[%u, %lu]を確認中...\n", pending_addr1.func_idx, pending_addr1.offset);
    state_queue_confirm_pending(queue, pending_addr1);
    
    // キューの内容を印刷
    state_queue_print(queue);
    
    // キューから取得（FIFO順）
    InstructionAddress dequeued_addr;
    IntermediateRepresentation dequeued_ir;
    
    while (!state_queue_is_empty(queue)) {
        if (state_queue_dequeue(queue, &dequeued_addr, &dequeued_ir)) {
            printf("デキュー: アドレス[%u, %lu], IR[opcode=%u]\n", 
                   dequeued_addr.func_idx, dequeued_addr.offset, dequeued_ir.opcode);
        }
    }
    
    state_queue_destroy(queue);
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
