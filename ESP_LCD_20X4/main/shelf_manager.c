#include "shelf_manager.h"
#include <string.h>  // for memset, memcpy

// Internal occupancy flag array: true = occupied
static bool _occupied[SHELF_SLOTS];

// Human‑readable names for each slot index
static const char* _slot_names[SHELF_SLOTS] = {
    // small slots
    "SM1", "SM2", "SM3",
    // medium slots
    "MD1", "MD2", "MD3",
    // large‑solid slots
    "LG1", "LG2", "LG3",
    // large‑liquid spill slot
    "LG_spill"
};

void shelf_manager_init(void)
{
    memset(_occupied, 0, sizeof(_occupied));
}

void shelf_manager_update_from_sensors(const bool ir_states[SHELF_SLOTS])
{
    memcpy(_occupied, ir_states, sizeof(_occupied));
}

int shelf_manager_find_slot(const item_info_t *info)
{
    int start = 0, end = 0;

    switch (info->size) {
        case SIZE_SMALL: 
            start = 0; end = 2; 
            break;
        case SIZE_MEDIUM:
            start = 3; end = 5;
            break;
        case SIZE_LARGE:
            if (info->phase == PHASE_SOLID) {
                // Use one of the three large‑solid shelves
                start = 6; end = 8;
            } else {
                // Liquid → only the single spill slot
                start = 9; end = 9;
            }
            break;
        default:
            // Fallback to medium
            start = 3; end = 5;
            break;
    }

    for (int i = start; i <= end; i++) {
        if (!_occupied[i]) {
            return i;
        }
    }
    return -1;
}

void shelf_manager_mark_occupied(int slot)
{
    if (slot >= 0 && slot < SHELF_SLOTS) {
        _occupied[slot] = true;
    }
}

bool shelf_manager_is_full(void)
{
    for (int i = 0; i < SHELF_SLOTS; i++) {
        if (!_occupied[i]) {
            return false;
        }
    }
    return true;
}

bool shelf_manager_is_slot_occupied(int slot)
{
    if (slot >= 0 && slot < SHELF_SLOTS) {
        return _occupied[slot];
    }
    return false;
}


const char* shelf_manager_slot_string(int slot)
{
    if (slot >= 0 && slot < SHELF_SLOTS) {
        return _slot_names[slot];
    }
    return "UNKNOWN";
}
