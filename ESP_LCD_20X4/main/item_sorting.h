/*=================================================================================================
File Name:	item_sorting.h
Author:		Vraj Patel, Mihir Jariwala
Date:		12/07/2025
Modified:	None
© Fanshawe College, 2025

Description: This file contains the interface for the item sorting module,
including functions for parsing and handling item information.
=================================================================================================*/

#ifndef ITEM_SORTING_H
#define ITEM_SORTING_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Sizes that the barcode scanner can report.
typedef enum {
    SIZE_SMALL,
    SIZE_MEDIUM,
    SIZE_LARGE
} item_size_t;

/// Types that the barcode scanner can report.
typedef enum {
    TYPE_FROZEN,
    TYPE_DRY
} item_type_t;

/// Phases that the barcode scanner can report.
typedef enum {
    PHASE_SOLID,
    PHASE_LIQUID
} item_phase_t;

/// Combined information extracted from a barcode.
typedef struct {
    item_size_t  size;
    item_type_t  type;
    item_phase_t phase;
} item_info_t;

/**
 * Parse a comma‑separated string of the form
 *   "<SIZE>,<TYPE>,<PHASE>"
 * where
 *   <SIZE>  is one of "SMALL", "MEDIUM", "LARGE"
 *   <TYPE>  is one of "FROZEN", "DRY"
 *   <PHASE> is one of "SOLID", "LIQUID"
 *
 * On success, fills *out and returns true. On failure returns false
 * and leaves *out unmodified.
 */
bool item_sorting_parse(const char *txt, item_info_t *out);

/** Human‑readable name for a size enum. */
const char* item_sorting_size_string(item_size_t s);
/** Human‑readable name for a type enum. */
const char* item_sorting_type_string(item_type_t t);
/** Human‑readable name for a phase enum. */
const char* item_sorting_phase_string(item_phase_t p);

#ifdef __cplusplus
}
#endif

#endif // ITEM_SORTING_H
