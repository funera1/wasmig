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
// 1. アドレスマップの実装 (C++ std::unordered_map)
// =============================================================================

AddressMap wasmig_address_map_create(size_t initial_capacity) {
    spdlog::debug("Creating address map with initial capacity: {}", initial_capacity);

    AddressMap map = (AddressMap)malloc(sizeof(struct address_map_impl));
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

void wasmig_address_map_destroy(AddressMap map) {
    if (!map) return;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    spdlog::debug("Destroying address map with {} entries", impl->size());
    
    delete impl;
    free(map);
}

bool wasmig_address_map_set(AddressMap map, uint32_t key, uint64_t value) {
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

bool wasmig_address_map_get(AddressMap map, uint32_t key, uint64_t* out_value) {
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

bool wasmig_address_map_remove(AddressMap map, uint32_t key) {
    if (!map || !map->impl) return false;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    
    size_t erased = impl->erase(key);
    if (erased > 0) {
        spdlog::debug("Removed key {} from address map", key);
        return true;
    }
    
    return false;
}

size_t wasmig_address_map_size(AddressMap map) {
    if (!map || !map->impl) return 0;
    
    auto* impl = static_cast<std::unordered_map<uint32_t, uint64_t>*>(map->impl);
    return impl->size();
}

void wasmig_address_map_print(AddressMap map) {
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

bool wasmig_state_queue_enqueue(StateManagementQueue queue, uint32_t offset) {
    if (!queue || !queue->impl) return false;
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        StateQueueEntry entry;
        entry.offset = offset;
        entry.is_confirmed = false;
        impl->push(entry);
        spdlog::debug("Enqueued to state queue: offset={}", offset);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to enqueue: {}", e.what());
        return false;
    }
}

bool wasmig_state_queue_dequeue(StateManagementQueue queue, uint32_t* out_offset) {
    if (!queue || !queue->impl || !out_offset) return false;
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        if (impl->empty()) return false;
        StateQueueEntry entry = impl->front();
        impl->pop();
        *out_offset = entry.offset;
        spdlog::debug("Dequeued from state queue: offset={}", entry.offset);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to dequeue: {}", e.what());
        return false;
    }
}

bool wasmig_state_queue_confirm_pending(StateManagementQueue queue, uint32_t offset) {
    if (!queue || !queue->impl) return false;
    try {
        auto* impl = static_cast<std::queue<StateQueueEntry>*>(queue->impl);
        std::queue<StateQueueEntry> temp_queue;
        bool found = false;
        while (!impl->empty()) {
            StateQueueEntry entry = impl->front();
            impl->pop();
            if (entry.offset == offset) {
                entry.is_confirmed = true;
                found = true;
                spdlog::debug("Confirmed pending instruction: offset={}", offset);
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
    if (!queue || !queue->impl) return;
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

// =============================================================================
// 4. AddressMap Registry (global)
// =============================================================================
#include <unordered_map>
static std::unordered_map<uint32_t, AddressMap> g_address_map_registry;

bool wasmig_address_map_save(uint32_t id, AddressMap map) {
    if (!map) return false;
    auto [it, inserted] = g_address_map_registry.emplace(id, map);
    return inserted;
}

AddressMap wasmig_address_map_load(uint32_t id) {
    auto it = g_address_map_registry.find(id);
    if (it != g_address_map_registry.end()) return it->second;
    return nullptr;
}

bool wasmig_address_map_exists(uint32_t id) {
    return g_address_map_registry.count(id) > 0;
}

void wasmig_address_map_registry_clear() {
    g_address_map_registry.clear();
}

// =============================================================================
// 5. CheckpointForbiddenList Registry (global)
// =============================================================================
static std::unordered_map<uint32_t, CheckpointForbiddenList> g_forbidden_list_registry;

bool wasmig_forbidden_list_save(uint32_t id, CheckpointForbiddenList list) {
    if (!list) return false;
    auto [it, inserted] = g_forbidden_list_registry.emplace(id, list);
    return inserted;
}

CheckpointForbiddenList wasmig_forbidden_list_load(uint32_t id) {
    auto it = g_forbidden_list_registry.find(id);
    if (it != g_forbidden_list_registry.end()) return it->second;
    return nullptr;
}

bool wasmig_forbidden_list_exists(uint32_t id) {
    return g_forbidden_list_registry.count(id) > 0;
}

void wasmig_forbidden_list_registry_clear() {
    g_forbidden_list_registry.clear();
}
