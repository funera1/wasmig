#ifndef WASMIG_TABLE_V3_H
#define WASMIG_TABLE_V3_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
#include <unordered_map>
#include <set>
#include <queue>
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

#ifdef __cplusplus
// C++比較演算子の定義（InstructionAddress用）
inline bool operator<(const InstructionAddress& a, const InstructionAddress& b) {
    if (a.func_idx != b.func_idx) return a.func_idx < b.func_idx;
    return a.offset < b.offset;
}

inline bool operator==(const InstructionAddress& a, const InstructionAddress& b) {
    return a.func_idx == b.func_idx && a.offset == b.offset;
}
#endif

// 1. アドレスマップ: u32キーとu64バリューを対応付けるための構造
#ifdef __cplusplus
// C++実装：std::unordered_mapを使用
typedef std::unordered_map<uint32_t, uint64_t> AddressMapImpl;
#else
// C実装のフォールバック（使用されない）
typedef struct AddressMapEntry {
    uint32_t key;
    uint64_t value;
    struct AddressMapEntry* next;
} AddressMapEntry;

typedef struct {
    AddressMapEntry** buckets;
    size_t bucket_count;
    size_t size;
} AddressMapImpl;
#endif

typedef struct {
    void* impl; // AddressMapImpl* のopaque pointer
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
#ifdef __cplusplus
// C++実装：std::setを使用
typedef std::set<ForbiddenAddress> ForbiddenListImpl;
#else
// C実装のフォールバック（使用されない）
typedef struct ForbiddenListEntry {
    ForbiddenAddress addr;
    struct ForbiddenListEntry* next;
} ForbiddenListEntry;

typedef struct {
    ForbiddenListEntry** buckets;
    size_t bucket_count;
    size_t size;
} ForbiddenListImpl;
#endif

typedef struct {
    void* impl; // ForbiddenListImpl* のopaque pointer
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

#ifdef __cplusplus
// C++実装：std::queueを使用
typedef std::queue<StateQueueEntry> StateQueueImpl;
#else
// C実装のフォールバック（使用されない）
typedef struct StateQueueNode {
    InstructionAddress pending_addr;
    IntermediateRepresentation pending_ir;
    bool is_confirmed;
    struct StateQueueNode* next;
} StateQueueNode;

typedef struct {
    StateQueueNode* head;
    StateQueueNode* tail;
    size_t size;
} StateQueueImpl;
#endif

typedef struct {
    void* impl; // StateQueueImpl* のopaque pointer
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
