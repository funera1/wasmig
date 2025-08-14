#include "wasmig/table_v3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <set>
#include <queue>
#include <stdexcept>

// アドレス比較関数（confirm_pending用）
static bool address_equals(InstructionAddress a, InstructionAddress b) {
    return a.func_idx == b.func_idx && a.offset == b.offset;
}

// =============================================================================
// 1. アドレスマップの実装 (C++ std::unordered_map)
// =============================================================================

AddressMap* address_map_create(size_t initial_capacity) {
    spdlog::debug("Creating address map with initial capacity: {}", initial_capacity);
    
    AddressMap* map = (AddressMap*)malloc(sizeof(AddressMap));
    if (!map) return nullptr;
    
    try {
        auto* impl = new std::unordered_map<uint32_t, uint64_t>();
        impl->reserve(initial_capacity);
        map->impl = impl;
        return map;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create address map: {}", e.what());
        free(map);
        return nullptr;
    }
}

void address_map_destroy(AddressMap* map) {
    if (!map) return;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    spdlog::debug("Destroying address map with {} entries", impl->size());
    
    delete impl;
    free(map);
}

bool address_map_set(AddressMap* map, uint32_t key, uint64_t value) {
    if (!map || !map->impl) return false;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    spdlog::debug("Setting address map: key={}, value={}", key, value);
    
    try {
        (*impl)[key] = value;
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to set address map entry: {}", e.what());
        return false;
    }
}

bool address_map_get(AddressMap* map, uint32_t key, uint64_t* out_value) {
    if (!map || !map->impl || !out_value) return false;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    
    auto it = impl->find(key);
    if (it != impl->end()) {
        *out_value = it->second;
        spdlog::debug("Found value for key {}: {}", key, *out_value);
        return true;
    }
    
    return false;
}

bool address_map_remove(AddressMap* map, uint32_t key) {
    if (!map || !map->impl) return false;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    
    size_t erased = impl->erase(key);
    if (erased > 0) {
        spdlog::debug("Removed key {} from address map", key);
        return true;
    }
    
    return false;
}

size_t address_map_size(AddressMap* map) {
    if (!map || !map->impl) return 0;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    return impl->size();
}

void address_map_print(AddressMap* map) {
    if (!map || !map->impl) return;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    printf("AddressMap (size: %zu)\n", impl->size());
    
    for (const auto& pair : *impl) {
        printf("  %u -> %lu\n", pair.first, pair.second);
    }
}

// =============================================================================
// 2. チェックポイント禁止リストの実装 (C++ std::set)
// =============================================================================

CheckpointForbiddenList* forbidden_list_create(size_t initial_capacity) {
    spdlog::debug("Creating forbidden list with capacity: {}", initial_capacity);
    
    CheckpointForbiddenList* list = (CheckpointForbiddenList*)malloc(sizeof(CheckpointForbiddenList));
    if (!list) return nullptr;
    
    try {
        auto* impl = new std::set<ForbiddenAddress>();
        list->impl = impl;
        return list;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create forbidden list: {}", e.what());
        free(list);
        return nullptr;
    }
}

void forbidden_list_destroy(CheckpointForbiddenList* list) {
    if (!list) return;
    
    if (list->impl) {
        auto* impl = static_cast<std::set<ForbiddenAddress>*>(list->impl);
        spdlog::debug("Destroying forbidden list with {} entries", impl->size());
        delete impl;
    }
    
    free(list);
}

bool forbidden_list_add(CheckpointForbiddenList* list, ForbiddenAddress addr) {
    if (!list || !list->impl) return false;
    
    try {
        auto* impl = static_cast<std::set<ForbiddenAddress>*>(list->impl);
        
        auto result = impl->insert(addr);
        if (result.second) {
            spdlog::debug("Added to forbidden list: address={}", addr);
        }
        
        return true; // 既に存在していても成功とみなす
    } catch (const std::exception& e) {
        spdlog::error("Failed to add to forbidden list: {}", e.what());
        return false;
    }
}

bool forbidden_list_contains(CheckpointForbiddenList* list, ForbiddenAddress addr) {
    if (!list || !list->impl) return false;
    
    auto* impl = static_cast<std::set<ForbiddenAddress>*>(list->impl);
    return impl->find(addr) != impl->end();
}

bool forbidden_list_remove(CheckpointForbiddenList* list, ForbiddenAddress addr) {
    if (!list || !list->impl) return false;
    
    try {
        auto* impl = static_cast<std::set<ForbiddenAddress>*>(list->impl);
        
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

size_t forbidden_list_size(CheckpointForbiddenList* list) {
    if (!list || !list->impl) return 0;
    
    auto* impl = static_cast<std::set<ForbiddenAddress>*>(list->impl);
    return impl->size();
}

void forbidden_list_print(CheckpointForbiddenList* list) {
    if (!list || !list->impl) return;
    
    auto* impl = static_cast<std::set<ForbiddenAddress>*>(list->impl);
    printf("ForbiddenList (size: %zu)\n", impl->size());
    
    for (const auto& addr : *impl) {
        printf("  %lu\n", addr);
    }
}

// =============================================================================
// 3. 状態管理キューの実装 (C++ std::queue)
// =============================================================================

StateManagementQueue* state_queue_create() {
    spdlog::debug("Creating state management queue");
    
    StateManagementQueue* queue = (StateManagementQueue*)malloc(sizeof(StateManagementQueue));
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

void state_queue_destroy(StateManagementQueue* queue) {
    if (!queue) return;
    
    if (queue->impl) {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        spdlog::debug("Destroying state queue with {} entries", impl->size());
        delete impl;
    }
    
    free(queue);
}

bool state_queue_enqueue(StateManagementQueue* queue, InstructionAddress addr, IntermediateRepresentation ir) {
    if (!queue || !queue->impl) return false;
    
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        
        StateQueueEntry entry;
        entry.addr = addr;
        entry.ir = ir;
        entry.is_confirmed = false;
        
        impl->push(entry);
        
        spdlog::debug("Enqueued to state queue: func_idx={}, offset={}, opcode={}", 
                      addr.func_idx, addr.offset, ir.opcode);
        
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to enqueue: {}", e.what());
        return false;
    }
}

bool state_queue_dequeue(StateManagementQueue* queue, InstructionAddress* out_addr, IntermediateRepresentation* out_ir) {
    if (!queue || !queue->impl || !out_addr || !out_ir) return false;
    
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        
        if (impl->empty()) return false;
        
        StateQueueEntry entry = impl->front();
        impl->pop();
        
        *out_addr = entry.addr;
        *out_ir = entry.ir;
        
        spdlog::debug("Dequeued from state queue: func_idx={}, offset={}, opcode={}", 
                      entry.addr.func_idx, entry.addr.offset, entry.ir.opcode);
        
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to dequeue: {}", e.what());
        return false;
    }
}

bool state_queue_confirm_pending(StateManagementQueue* queue, InstructionAddress addr) {
    if (!queue || !queue->impl) return false;
    
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        
        // std::queueは直接要素にアクセスできないため、
        // 一時的にdequeを使用してconfirm操作を実現
        std::queue<StateQueueEntry> temp_queue;
        bool found = false;
        
        while (!impl->empty()) {
            StateQueueEntry entry = impl->front();
            impl->pop();
            
            if (address_equals(entry.addr, addr)) {
                entry.is_confirmed = true;
                found = true;
                spdlog::debug("Confirmed pending instruction: func_idx={}, offset={}", addr.func_idx, addr.offset);
            }
            
            temp_queue.push(entry);
        }
        
        // 元のキューに戻す
        *impl = temp_queue;
        
        return found;
    } catch (const std::exception& e) {
        spdlog::error("Failed to confirm pending: {}", e.what());
        return false;
    }
}

bool state_queue_is_empty(StateManagementQueue* queue) {
    if (!queue || !queue->impl) return true;
    
    auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
    return impl->empty();
}

size_t state_queue_size(StateManagementQueue* queue) {
    if (!queue || !queue->impl) return 0;
    
    auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
    return impl->size();
}

void state_queue_print(StateManagementQueue* queue) {
    if (!queue || !queue->impl) return;
    
    auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
    printf("StateQueue (size: %zu)\n", impl->size());
    
    // std::queueは直接イテレートできないため、一時的にコピーして表示
    std::queue<StateQueueEntry> temp_queue = *impl;
    int index = 0;
    
    while (!temp_queue.empty()) {
        StateQueueEntry entry = temp_queue.front();
        temp_queue.pop();
        
        printf("  [%d] [%u, %lu] -> [%u, %u, %u] (confirmed: %s)\n", 
               index++,
               entry.addr.func_idx, entry.addr.offset,
               entry.ir.opcode, entry.ir.operand1, entry.ir.operand2,
               entry.is_confirmed ? "yes" : "no");
    }
}
