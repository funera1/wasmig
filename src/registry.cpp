#include "wasmig/registry.h"
#include <vector>
#include <memory>

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
