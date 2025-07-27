#ifndef SHELF_MANAGER_H
#define SHELF_MANAGER_H

#include <stdbool.h>
#include "item_sorting.h"

/// Total number of physical slots tracked
#define SHELF_SLOTS   10

/// Call once at startup to clear all occupancy.
void shelf_manager_init(void);

/// Update internal occupancy from an array of IR sensor states:
/// true = occupied, false = free. Must be length SHELF_SLOTS.
void shelf_manager_update_from_sensors(const bool ir_states[SHELF_SLOTS]);

/**
 * Find the first free slot appropriate for `info`.
 * - SMALL  → slots 0–2
 * - MEDIUM → slots 3–5
 * - LARGE  → if SOLID then 6–8, else (LIQUID) slot 9
 *
 * Returns the slot index [0..9], or –1 if none free.
 */
int  shelf_manager_find_slot(const item_info_t *info);

/// Mark a slot as occupied (e.g. after you place the item there).
void shelf_manager_mark_occupied(int slot);

/// Returns true if the given slot index is currently occupied.
bool shelf_manager_is_slot_occupied(int slot);


/// Returns true if all SHELF_SLOTS are occupied.
bool shelf_manager_is_full(void);

/// Human‑readable name for a slot index (e.g. "SM1", "MD2", "LG3", "LG_spill", etc.)
const char* shelf_manager_slot_string(int slot);

#endif // SHELF_MANAGER_H
