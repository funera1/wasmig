#include "wasmig/table_v3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <set>
#include <queue>
#include <stdexcept>

// =============================================================================
// 1. アドレスマップの実装 (C++ 双方向マップ)
// =============================================================================

static inline uint64_t pack_fidx_offset(uint32_t fidx, uint32_t offset) {
    return (uint64_t(fidx) << 32) | uint64_t(offset);
}
static inline void unpack_fidx_offset(uint64_t packed, uint32_t &fidx, uint32_t &offset) {
    fidx = uint32_t(packed >> 32);
    offset = uint32_t(packed & 0xffffffffu);
}

AddressMap wasmig_address_map_create(size_t initial_capacity) {
    spdlog::debug("Creating address map with initial capacity: {}", initial_capacity);
    AddressMap map = (AddressMap)malloc(sizeof(struct address_map_impl));
    if (!map) return nullptr;
    try {
        auto* kv = new std::unordered_map<uint64_t, uint64_t>();
        auto* vk = new std::unordered_map<uint64_t, uint64_t>();
        kv->reserve(initial_capacity);
        vk->reserve(initial_capacity);
        map->kv_impl = kv;
        map->vk_impl = vk;
        return map;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create address map: {}", e.what());
        free(map);
        return nullptr;
    }
}

void wasmig_address_map_destroy(AddressMap map) {
    if (!map) return;
    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    auto* vk = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->vk_impl);
    spdlog::debug("Destroying address map with {} entries", kv->size());
    delete kv;
    delete vk;
    free(map);
}

bool wasmig_address_map_set_bidirect(AddressMap map, uint32_t fidx, uint32_t offset, uint64_t value) {
    if (!map || !map->kv_impl || !map->vk_impl) return false;
    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    auto* vk = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->vk_impl);
    try {
        uint64_t packed = pack_fidx_offset(fidx, offset);
        auto it = kv->find(packed);
        if (it != kv->end()) {
            vk->erase(it->second);
        }
        (*kv)[packed] = value;
        (*vk)[value] = packed;
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to set address map entry: {}", e.what());
        return false;
    }
}

bool wasmig_address_map_set_forward(AddressMap map, uint32_t fidx, uint32_t offset, uint64_t value) {
    if (!map || !map->kv_impl || !map->vk_impl) return false;
    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    try {
        uint64_t packed = pack_fidx_offset(fidx, offset);
        auto it = kv->find(packed);
        (*kv)[packed] = value;
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to set address map entry: {}", e.what());
        return false;
    }
}

bool wasmig_address_map_set_backward(AddressMap map, uint32_t fidx, uint32_t offset, uint64_t value) {
    auto* vk = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->vk_impl);
    try {
        uint64_t packed = pack_fidx_offset(fidx, offset);
        (*vk)[value] = packed;
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to set address map entry: {}", e.what());
        return false;
    }
}

bool wasmig_address_map_get_key(AddressMap map, uint64_t value, uint32_t* out_fidx, uint32_t* out_offset) {
    if (!map || !map->vk_impl || !out_fidx || !out_offset) return false;
    auto* vk = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->vk_impl);
    auto it = vk->find(value);
    if (it != vk->end()) {
        uint32_t fidx, offset;
        unpack_fidx_offset(it->second, fidx, offset);
        *out_fidx = fidx;
        *out_offset = offset;
        spdlog::debug("Found key for value {}: {}:{}", value, fidx, offset);
        return true;
    }
    return false;
}

bool wasmig_address_map_get_value(AddressMap map, uint32_t fidx, uint32_t offset, uint64_t* out_value) {
    if (!map || !map->kv_impl || !out_value) return false;
    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    uint64_t packed = pack_fidx_offset(fidx, offset);
    auto it = kv->find(packed);
    if (it != kv->end()) {
        *out_value = it->second;
        spdlog::debug("Found value for fidx/offset {}:{} -> {}", fidx, offset, *out_value);
        return true;
    }
    return false;
}

bool wasmig_address_map_exist_key(AddressMap map, uint64_t address) {
    if (!map || !map->vk_impl) {
        spdlog::error("AddressMap is null");
        return false;
    }
    auto* vk = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->vk_impl);
    return vk->find(address) != vk->end();
}

bool wasmig_address_map_exist_value(AddressMap map, uint32_t fidx, uint32_t offset) {
    if (!map || !map->kv_impl) {
        spdlog::error("AddressMap is null");
        return false;
    }
    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    uint64_t packed = pack_fidx_offset(fidx, offset);
    return kv->find(packed) != kv->end();
}

bool wasmig_address_map_remove(AddressMap map, uint32_t fidx, uint32_t offset) {
    if (!map || !map->kv_impl || !map->vk_impl) return false;
    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    auto* vk = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->vk_impl);
    uint64_t packed = pack_fidx_offset(fidx, offset);
    auto it = kv->find(packed);
    if (it != kv->end()) {
        vk->erase(it->second);
        kv->erase(it);
        spdlog::debug("Removed fidx/offset {}:{} from address map", fidx, offset);
        return true;
    }
    return false;
}

size_t wasmig_address_map_size(AddressMap map) {
    if (!map || !map->kv_impl) return 0;
    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    return kv->size();
}

void wasmig_address_map_print(AddressMap map) {
    if (!map) {
        printf("AddressMap is null\n");
        return;
    }
    if (!map->kv_impl) {
        printf("AddressMap kv_impl is null\n");
        return;
    }
    if (!map->vk_impl) {
        printf("AddressMap vk_impl is null\n");
        return;
    }

    auto* kv = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->kv_impl);
    auto* vk = static_cast<std::unordered_map<uint64_t, uint64_t>*>(map->vk_impl);
    printf("AddressMap (size: %zu)\n", kv->size());
    for (const auto& pair : *kv) {
        uint32_t fidx, offset;
        unpack_fidx_offset(pair.first, fidx, offset);
        uint64_t address = pair.second;
        
        // 双方向対応をチェック: vk[address] が存在して pair.first と一致するか
        auto vk_it = vk->find(address);
        bool is_bidirectional = (vk_it != vk->end()) && (vk_it->second == pair.first);
        
        const char* arrow = is_bidirectional ? "<->" : "->";
        printf("  %u:%u %s %lu\n", fidx, offset, arrow, address);
    }
}

// =============================================================================
// 2. チェックポイント禁止リストの実装 (C++ std::set)
// =============================================================================

CheckpointForbiddenList wasmig_forbidden_list_create(size_t initial_capacity) {
    spdlog::debug("Creating forbidden list with capacity: {}", initial_capacity);
    CheckpointForbiddenList list = (CheckpointForbiddenList)malloc(sizeof(struct checkpoint_forbidden_list_impl));
    if (!list) return nullptr;
    try {
        auto* impl = new std::set<uint64_t>();
        list->impl = impl;
        return list;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create forbidden list: {}", e.what());
        free(list);
        return nullptr;
    }
}

void wasmig_forbidden_list_destroy(CheckpointForbiddenList list) {
    if (!list) return;
    if (list->impl) {
        auto* impl = static_cast<std::set<uint64_t>*>(list->impl);
        spdlog::debug("Destroying forbidden list with {} entries", impl->size());
        delete impl;
    }
    free(list);
}

bool wasmig_forbidden_list_add(CheckpointForbiddenList list, uint64_t addr) {
    if (!list || !list->impl) return false;
    try {
        auto* impl = static_cast<std::set<uint64_t>*>(list->impl);
        auto result = impl->insert(addr);
        if (result.second) {
            spdlog::debug("Added to forbidden list: address={}", addr);
        }
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to add to forbidden list: {}", e.what());
        return false;
    }
}

bool wasmig_forbidden_list_contains(CheckpointForbiddenList list, uint64_t addr) {
    if (!list || !list->impl) return false;
    auto* impl = static_cast<std::set<uint64_t>*>(list->impl);
    return impl->find(addr) != impl->end();
}

bool wasmig_forbidden_list_remove(CheckpointForbiddenList list, uint64_t addr) {
    if (!list || !list->impl) return false;
    try {
        auto* impl = static_cast<std::set<uint64_t>*>(list->impl);
        size_t erased = impl->erase(addr);
        if (erased > 0) {
            spdlog::debug("Removed from forbidden list: address={}", addr);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        spdlog::error("Failed to remove from forbidden list: {}", e.what());
        return false;
    }
}

size_t wasmig_forbidden_list_size(CheckpointForbiddenList list) {
    if (!list || !list->impl) return 0;
    auto* impl = static_cast<std::set<uint64_t>*>(list->impl);
    return impl->size();
}

void wasmig_forbidden_list_print(CheckpointForbiddenList list) {
    if (!list || !list->impl) return;
    auto* impl = static_cast<std::set<uint64_t>*>(list->impl);
    printf("ForbiddenList (size: %zu)\n", impl->size());
    for (const auto& addr : *impl) {
        printf("  %lu\n", addr);
    }
}

// =============================================================================
// 3. 状態管理キューの実装 (C++ std::queue)
// =============================================================================

StateManagementQueue wasmig_state_queue_create() {
    spdlog::debug("Creating state management queue");
    StateManagementQueue queue = (StateManagementQueue)malloc(sizeof(struct state_management_queue_impl));
    if (!queue) return nullptr;
    try {
        auto* impl = new std::queue<StateQueueEntry>();
        queue->impl = impl;
        return queue;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create state queue: {}", e.what());
        free(queue);
        return nullptr;
    }
}

void wasmig_state_queue_destroy(StateManagementQueue queue) {
    if (!queue) return;
    if (queue->impl) {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        spdlog::debug("Destroying state queue with {} entries", impl->size());
        delete impl;
    }
    free(queue);
}

bool wasmig_state_queue_enqueue(StateManagementQueue queue, uint32_t fidx, uint32_t offset) {
    if (!queue || !queue->impl) return false;
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        StateQueueEntry entry;
        entry.fidx = fidx;
        entry.offset = offset;
        entry.is_confirmed = false;
        impl->push(entry);
        spdlog::debug("Enqueued to state queue: fidx={}, offset={}", fidx, offset);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to enqueue: {}", e.what());
        return false;
    }
}

bool wasmig_state_queue_dequeue(StateManagementQueue queue, uint32_t* out_fidx, uint32_t* out_offset) {
    if (!queue || !queue->impl || !out_fidx || !out_offset) return false;
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        if (impl->empty()) return false;
        StateQueueEntry entry = impl->front();
        impl->pop();
        *out_fidx = entry.fidx;
        *out_offset = entry.offset;
        spdlog::debug("Dequeued from state queue: (fidx={}, offset={})", entry.fidx, entry.offset);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to dequeue: {}", e.what());
        return false;
    }
}

bool wasmig_state_queue_confirm_pending(StateManagementQueue queue, uint32_t fidx, uint32_t offset) {
    if (!queue || !queue->impl) return false;
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        std::queue<StateQueueEntry> temp_queue;
        bool found = false;
        while (!impl->empty()) {
            StateQueueEntry entry = impl->front();
            impl->pop();
            if (entry.fidx == fidx && entry.offset == offset) {
                entry.is_confirmed = true;
                found = true;
                spdlog::debug("Confirmed pending instruction: (fidx={}, offset={})", fidx, offset);
            }
            temp_queue.push(entry);
        }
        *impl = temp_queue;
        return found;
    } catch (const std::exception& e) {
        spdlog::error("Failed to confirm pending: {}", e.what());
        return false;
    }
}

bool wasmig_state_queue_is_empty(StateManagementQueue queue) {
    if (!queue || !queue->impl) return true;
    auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
    return impl->empty();
}

size_t wasmig_state_queue_size(StateManagementQueue queue) {
    if (!queue || !queue->impl) return 0;
    auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
    return impl->size();
}

void wasmig_state_queue_print(StateManagementQueue queue) {
    if (!queue) {
        printf("StateQueue is null\n");
        return;
    }
    if (!queue->impl) {
        printf("StateQueue impl is null\n");
        return;
    }

    auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
    printf("StateQueue (size: %zu)\n", impl->size());
    std::queue<StateQueueEntry> temp_queue = *impl;
    int index = 0;
    while (!temp_queue.empty()) {
        StateQueueEntry entry = temp_queue.front();
        temp_queue.pop();
        printf("  [%d] [%u] (confirmed: %s)\n", 
               index++,
               entry.offset,
               entry.is_confirmed ? "yes" : "no");
    }
}

// グローバル管理APIの実装はregistry.cppに分離済みなので不要。
