#ifndef WASMIG_TABLE_V3_H
#define WASMIG_TABLE_V3_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


// 1. アドレスマップ: u32キーとu64バリューを対応付けるための構造
typedef struct {
    void* impl; // 実装詳細を隠蔽するopaque pointer
} AddressMap;

// アドレスマップの関数
AddressMap* wasmig_address_map_create(size_t initial_capacity);
void wasmig_address_map_destroy(AddressMap* map);
bool wasmig_address_map_set(AddressMap* map, uint32_t key, uint64_t value);
bool wasmig_address_map_get(AddressMap* map, uint32_t key, uint64_t* out_value);
bool wasmig_address_map_remove(AddressMap* map, uint32_t key);
size_t wasmig_address_map_size(AddressMap* map);
void wasmig_address_map_print(AddressMap* map);

// 2. チェックポイント禁止リスト: 64ビットアドレスを保持する集合
typedef struct {
    void* impl; // 実装詳細を隠蔽するopaque pointer
} CheckpointForbiddenList;

// 禁止リストの関数
CheckpointForbiddenList* wasmig_forbidden_list_create(size_t initial_capacity);
void wasmig_forbidden_list_destroy(CheckpointForbiddenList* list);
bool wasmig_forbidden_list_add(CheckpointForbiddenList* list, uint64_t addr);
bool wasmig_forbidden_list_contains(CheckpointForbiddenList* list, uint64_t addr);
bool wasmig_forbidden_list_remove(CheckpointForbiddenList* list, uint64_t addr);
size_t wasmig_forbidden_list_size(CheckpointForbiddenList* list);
void wasmig_forbidden_list_print(CheckpointForbiddenList* list);

// 3. 状態管理キュー: 未確定の命令位置を一時的に保持し、後続命令生成時に対応付けを確定するための構造

// キューエントリの型定義
typedef struct {
    uint32_t offset;
    bool is_confirmed;
} StateQueueEntry;

typedef struct {
    void* impl; // 実装詳細を隠蔽するopaque pointer
} StateManagementQueue;

// 状態管理キューの関数
StateManagementQueue* wasmig_state_queue_create();
void wasmig_state_queue_destroy(StateManagementQueue* queue);
bool wasmig_state_queue_enqueue(StateManagementQueue* queue, uint32_t offset);
bool wasmig_state_queue_dequeue(StateManagementQueue* queue, uint32_t* out_offset);
bool wasmig_state_queue_confirm_pending(StateManagementQueue* queue, uint32_t offset);
bool wasmig_state_queue_is_empty(StateManagementQueue* queue);
size_t wasmig_state_queue_size(StateManagementQueue* queue);
void wasmig_state_queue_print(StateManagementQueue* queue);

#ifdef __cplusplus
}
#endif

#endif // WASMIG_TABLE_V3_H
