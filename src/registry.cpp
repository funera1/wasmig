#include "wasmig/registry.h"
#include <unordered_map>

// StackStateMap global registry
static std::unordered_map<uint32_t, StackStateMap> g_stack_state_map_registry;

bool wasmig_stack_state_map_registry_save(uint32_t id, StackStateMap map) {
    if (!map) return false;
    auto [it, inserted] = g_stack_state_map_registry.emplace(id, map);
    return inserted;
}

StackStateMap wasmig_stack_state_map_registry_load(uint32_t id) {
    auto it = g_stack_state_map_registry.find(id);
    if (it != g_stack_state_map_registry.end()) return it->second;
    return nullptr;
}

bool wasmig_stack_state_map_registry_exists(uint32_t id) {
    return g_stack_state_map_registry.count(id) > 0;
}

void wasmig_stack_state_map_registry_clear() {
    g_stack_state_map_registry.clear();
}

// AddressMap global registry
static std::unordered_map<uint32_t, AddressMap> g_address_map_registry;

bool wasmig_address_map_registry_save(uint32_t id, AddressMap map) {
    if (!map) return false;
    auto [it, inserted] = g_address_map_registry.emplace(id, map);
    return inserted;
}

AddressMap wasmig_address_map_registry_load(uint32_t id) {
    auto it = g_address_map_registry.find(id);
    if (it != g_address_map_registry.end()) return it->second;
    return nullptr;
}

bool wasmig_address_map_registry_exists(uint32_t id) {
    return g_address_map_registry.count(id) > 0;
}

void wasmig_address_map_registry_clear() {
    g_address_map_registry.clear();
}

// CheckpointForbiddenList global registry
static std::unordered_map<uint32_t, CheckpointForbiddenList> g_forbidden_list_registry;

bool wasmig_forbidden_list_registry_save(uint32_t id, CheckpointForbiddenList list) {
    if (!list) return false;
    auto [it, inserted] = g_forbidden_list_registry.emplace(id, list);
    return inserted;
}

CheckpointForbiddenList wasmig_forbidden_list_registry_load(uint32_t id) {
    auto it = g_forbidden_list_registry.find(id);
    if (it != g_forbidden_list_registry.end()) return it->second;
    return nullptr;
}

bool wasmig_forbidden_list_registry_exists(uint32_t id) {
    return g_forbidden_list_registry.count(id) > 0;
}

void wasmig_forbidden_list_registry_clear() {
    g_forbidden_list_registry.clear();
}
