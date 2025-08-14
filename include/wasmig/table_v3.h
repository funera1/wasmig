#ifndef WASMIG_TABLE_V3_H
#define WASMIG_TABLE_V3_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 中間表現の型定義
typedef struct {
    uint32_t opcode;
    uint32_t operand1;
    uint32_t operand2;
    // 必要に応じて他のフィールドを追加
} IntermediateRepresentation;

// 命令位置の型定義（状態管理キュー用）
typedef struct {
    uint32_t func_idx;
    uint64_t offset;
} InstructionAddress;

// 禁止リスト用の64ビットアドレス型
typedef uint64_t ForbiddenAddress;

// 1. アドレスマップ: u32キーとu64バリューを対応付けるための構造
typedef struct {
    void* impl; // 実装詳細を隠蔽するopaque pointer
} AddressMap;

// アドレスマップの関数
AddressMap* address_map_create(size_t initial_capacity);
void address_map_destroy(AddressMap* map);
bool address_map_set(AddressMap* map, uint32_t key, uint64_t value);
bool address_map_get(AddressMap* map, uint32_t key, uint64_t* out_value);
bool address_map_remove(AddressMap* map, uint32_t key);
size_t address_map_size(AddressMap* map);
void address_map_print(AddressMap* map);

// 2. チェックポイント禁止リスト: 64ビットアドレスを保持する集合
typedef struct {
    void* impl; // 実装詳細を隠蔽するopaque pointer
} CheckpointForbiddenList;

// 禁止リストの関数
CheckpointForbiddenList* forbidden_list_create(size_t initial_capacity);
void forbidden_list_destroy(CheckpointForbiddenList* list);
bool forbidden_list_add(CheckpointForbiddenList* list, ForbiddenAddress addr);
bool forbidden_list_contains(CheckpointForbiddenList* list, ForbiddenAddress addr);
bool forbidden_list_remove(CheckpointForbiddenList* list, ForbiddenAddress addr);
size_t forbidden_list_size(CheckpointForbiddenList* list);
void forbidden_list_print(CheckpointForbiddenList* list);

// 3. 状態管理キュー: 未確定の命令位置を一時的に保持し、後続命令生成時に対応付けを確定するための構造

// キューエントリの型定義
typedef struct {
    InstructionAddress addr;
    IntermediateRepresentation ir;
    bool is_confirmed;
} StateQueueEntry;

typedef struct {
    void* impl; // 実装詳細を隠蔽するopaque pointer
} StateManagementQueue;

// 状態管理キューの関数
StateManagementQueue* state_queue_create();
void state_queue_destroy(StateManagementQueue* queue);
bool state_queue_enqueue(StateManagementQueue* queue, InstructionAddress addr, IntermediateRepresentation ir);
bool state_queue_dequeue(StateManagementQueue* queue, InstructionAddress* out_addr, IntermediateRepresentation* out_ir);
bool state_queue_confirm_pending(StateManagementQueue* queue, InstructionAddress addr);
bool state_queue_is_empty(StateManagementQueue* queue);
size_t state_queue_size(StateManagementQueue* queue);
void state_queue_print(StateManagementQueue* queue);

#ifdef __cplusplus
}
#endif

#endif // WASMIG_TABLE_V3_H
