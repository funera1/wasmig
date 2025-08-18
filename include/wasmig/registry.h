#ifndef WASMIG_REGISTRY_H
#define WASMIG_REGISTRY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "wasmig/stack.h"
#include "wasmig/table_v3.h"

#ifdef __cplusplus
extern "C" {
#endif

// Note: ModuleTables abstraction removed; use singletons for AddressMap and ForbiddenList.

// Backward-compatible wrappers for previous registry functions
// Singletons (one per process/module system): AddressMap and ForbiddenList
bool wasmig_address_map_save(AddressMap map);
AddressMap wasmig_address_map_load();
bool wasmig_address_map_exists();
void wasmig_address_map_clear();

bool wasmig_forbidden_list_save(CheckpointForbiddenList list);
CheckpointForbiddenList wasmig_forbidden_list_load();
bool wasmig_forbidden_list_exists();
void wasmig_forbidden_list_clear();

#ifdef __cplusplus
}
#endif

#endif // WASMIG_REGISTRY_H
