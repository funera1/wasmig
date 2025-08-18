#include "wasmig/registry.h"
#include <vector>
#include <memory>
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

// Singletons for AddressMap and CheckpointForbiddenList
static AddressMap g_address_map_singleton = nullptr;
static CheckpointForbiddenList g_forbidden_list_singleton = nullptr;

bool wasmig_address_map_save(AddressMap map) {
    if (g_address_map_singleton) {
        // replace existing
        wasmig_address_map_destroy(g_address_map_singleton);
    }
    g_address_map_singleton = map;
    return true;
}

AddressMap wasmig_address_map_load() {
    return g_address_map_singleton;
}

bool wasmig_address_map_exists() {
    return g_address_map_singleton != nullptr;
}

void wasmig_address_map_clear() {
    if (g_address_map_singleton) {
        wasmig_address_map_destroy(g_address_map_singleton);
        g_address_map_singleton = nullptr;
    }
}

bool wasmig_forbidden_list_save(CheckpointForbiddenList list) {
    if (g_forbidden_list_singleton) {
        wasmig_forbidden_list_destroy(g_forbidden_list_singleton);
    }
    g_forbidden_list_singleton = list;
    return true;
}

CheckpointForbiddenList wasmig_forbidden_list_load() {
    return g_forbidden_list_singleton;
}

bool wasmig_forbidden_list_exists() {
    return g_forbidden_list_singleton != nullptr;
}

void wasmig_forbidden_list_clear() {
    if (g_forbidden_list_singleton) {
        wasmig_forbidden_list_destroy(g_forbidden_list_singleton);
        g_forbidden_list_singleton = nullptr;
    }
}
