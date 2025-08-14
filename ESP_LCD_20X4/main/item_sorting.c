/*==================================================================================================
File Name:	item_sorting.c
Author:		Vraj Patel, Mihir Jariwala
Date:		12/07/2025
Modified:	27/07/2025
© Fanshawe College, 2025

Description: This file contains the implementation of the item sorting module,
including functions for parsing and handling item information.
==================================================================================================*/


#include "item_sorting.h"
#include <string.h>
#include <ctype.h>

/*>>> _strcasecmp: ==========================================================
Author:		Vraj Patel, Mihir Jariwala
Date:		12/07/2025
Modified:	None
Desc:		This function will perform a case-insensitive string comparison.
Input: 		- a: Pointer to the first string
			- b: Pointer to the second string
Returns:	-1 if a < b, 0 if a == b, 1 if a > b
 ============================================================================*/
static int _strcasecmp(const char *a, const char *b) {
    // simple case‑insensitive compare
    while (*a && *b) {
        char ca = tolower((unsigned char)*a++);
        char cb = tolower((unsigned char)*b++);
        if (ca != cb) return (ca < cb) ? -1 : 1;
    }
    return (*a == *b) ? 0 : ((*a) ? 1 : -1);
}// eo _strcasecmp::


/*>>> item_sorting_parse: ==========================================================
Author:		Vraj Patel, Mihir Jariwala
Date:		12/07/2025
Modified:	None
Desc:		This function will parse a text string into an item_info_t structure.
Input: 		- txt: Pointer to the input text string
			- out: Pointer to the output item_info_t structure
Returns:	true on success, false on failure.
 ============================================================================*/
bool item_sorting_parse(const char *txt, item_info_t *out) {
    if (!txt || !out) return false;

    // Copy into modifiable buffer
    char buf[64];
    size_t len = strlen(txt);
    if (len >= sizeof(buf)) return false;
    memcpy(buf, txt, len + 1);

    // Split on commas
    char *saveptr;
    char *tok = strtok_r(buf, ",", &saveptr);
    if (!tok) return false;

    // SIZE
    if      (_strcasecmp(tok, "SMALL")  == 0) out->size = SIZE_SMALL;
    else if (_strcasecmp(tok, "MEDIUM") == 0) out->size = SIZE_MEDIUM;
    else if (_strcasecmp(tok, "LARGE")  == 0) out->size = SIZE_LARGE;
    else return false;

    // TYPE
    tok = strtok_r(NULL, ",", &saveptr);
    if (!tok) return false;
    if      (_strcasecmp(tok, "FROZEN") == 0) out->type = TYPE_FROZEN;
    else if (_strcasecmp(tok, "NORMAL")    == 0) out->type = TYPE_DRY;
    else return false;

    // PHASE
    tok = strtok_r(NULL, ",", &saveptr);
    if (!tok) return false;
    if      (_strcasecmp(tok, "SOLID")  == 0) out->phase = PHASE_SOLID;
    else if (_strcasecmp(tok, "LIQUID") == 0) out->phase = PHASE_LIQUID;
    else return false;

    // Must not have a fourth token
    if (strtok_r(NULL, ",", &saveptr) != NULL) {
        // Extra data after third comma → fail
        return false;
    }

    return true;
}// eo item_sorting_parse::

/*>>> item_sorting_size_string: ==========================================================
Author:		Vraj Patel, Mihir Jariwala
Date:		12/07/2025
Modified:	None
Desc:		This function will return a string representation of the item size.
Input: 		- s: The item size enum value
Returns:	A pointer to the corresponding string.
 ============================================================================*/
const char* item_sorting_size_string(item_size_t s) {
    switch (s) {
    case SIZE_SMALL:  return "SMALL";
    case SIZE_MEDIUM: return "MEDIUM";
    case SIZE_LARGE:  return "LARGE";
    default:          return "UNKNOWN";
    }
}// eo item_sorting_size_string::


/*>>> item_sorting_type_string: ==========================================================
Author:		Vraj Patel, Mihir Jariwala
Date:		12/07/2025
Modified:	27/07/2025
Desc:		This function will return a string representation of the item type.
Input: 		- t: The item type enum value
Returns:	A pointer to the corresponding string.
 ============================================================================*/
const char* item_sorting_type_string(item_type_t t) {
    switch (t) {
    case TYPE_FROZEN: return "FROZEN";
    case TYPE_DRY:    return "NORMAL";
    default:          return "UNKNOWN";
    }
}// eo item_sorting_type_string::


/*>>> item_sorting_phase_string: ==========================================================
Author:		Vraj Patel, Mihir Jariwala
Date:		12/07/2025
Modified:	27/07/2025
Desc:		This function will return a string representation of the item phase.
Input: 		- p: The item phase enum value
Returns:	A pointer to the corresponding string.
 ============================================================================*/
const char* item_sorting_phase_string(item_phase_t p) {
    switch (p) {
    case PHASE_SOLID:  return "SOLID";
    case PHASE_LIQUID: return "LIQUID";
    default:           return "UNKNOWN";
    }
}// eo item_sorting_phase_string::
