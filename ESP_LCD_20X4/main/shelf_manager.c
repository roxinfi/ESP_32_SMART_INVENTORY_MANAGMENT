/*=====================================================================================================
File Name:	shelf_manager.c
Author:		Vraj Patel
Date:		10/07/2025
Modified:	None
© Fanshawe College, 2025

Description: This file contains the implementation of the shelf manager,
including functions for managing shelf occupancy and item placement.
=====================================================================================================*/

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

/*>>> shelf_manager_init: ==============================================================================
Author: Vraj Patel
Date: 10/07/2025
Modified: None
Desc: Initialize the shelf manager used for managing shelf occupancy and item placement.
Input: None
Return: None
=========================================================================================================*/
void shelf_manager_init(void)
{
    memset(_occupied, 0, sizeof(_occupied));
}

/*>>> shelf_manager_update_from_sensors: ==============================================================================

Author: Vraj Patel
Date: 10/07/2025
Modified: None
Desc: Update the shelf occupancy state from the IR sensor readings.
Input: const bool ir_states[SHELF_SLOTS] - Array of IR sensor states.
Return: None
=========================================================================================================*/
void shelf_manager_update_from_sensors(const bool ir_states[SHELF_SLOTS])
{
    memcpy(_occupied, ir_states, sizeof(_occupied));
} // eo shelf_manager_update_from_sensors::

/*>>> shelf_manager_find_slot: ==============================================================================

Author: Vraj Patel
Date: 10/07/2025
Modified: None
Desc: Find an available slot for a new item based on its size and phase.
Input: const item_info_t *info - Pointer to the item information structure.
Return: int - The index of the available slot, or -1 if none found.
=========================================================================================================*/
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
} // eo shelf_manager_find_slot::


/*>>> shelf_manager_mark_occupied: ==============================================================================
Author: Vraj Patel
Date: 10/07/2025
Modified: None
Desc: Mark a shelf slot as occupied.
Input: int slot - The index of the shelf slot to mark.
Return: None
=========================================================================================================*/
void shelf_manager_mark_occupied(int slot)
{
    if (slot >= 0 && slot < SHELF_SLOTS) {
        _occupied[slot] = true;
    }
} // eo shelf_manager_mark_occupied::

/*>>> shelf_manager_is_full: ==============================================================================
Author: Vraj Patel
Date: 10/07/2025
Modified: None
Desc: Check if all shelf slots are occupied.
Input: None
Return: bool - True if all slots are occupied, false otherwise.
=========================================================================================================*/
bool shelf_manager_is_full(void)
{
    for (int i = 0; i < SHELF_SLOTS; i++) {
        if (!_occupied[i]) {
            return false;
        }
    }
    return true;
} // eo shelf_manager_is_full::

/*>>> shelf_manager_is_slot_occupied: ==============================================================================

Author: Vraj Patel
Date: 10/07/2025
Modified: None
Desc: Check if a specific shelf slot is occupied.
Input: int slot - The index of the shelf slot to check.
Return: bool - True if the slot is occupied, false otherwise.
=========================================================================================================*/
bool shelf_manager_is_slot_occupied(int slot)
{
    if (slot >= 0 && slot < SHELF_SLOTS) {
        return _occupied[slot];
    }
    return false;
} // eo shelf_manager_is_slot_occupied::

/*>>> shelf_manager_slot_string: ==============================================================================
Author: Vraj Patel
Date: 10/07/2025
Modified: None
Desc: Get the string representation of a shelf slot.
Input: int slot - The index of the shelf slot.
Return: const char* - The name of the shelf slot, or "UNKNOWN" if invalid.
=========================================================================================================*/
const char* shelf_manager_slot_string(int slot)
{
    if (slot >= 0 && slot < SHELF_SLOTS) {
        return _slot_names[slot];
    }
    return "UNKNOWN";
} // eo shelf_manager_slot_string::
