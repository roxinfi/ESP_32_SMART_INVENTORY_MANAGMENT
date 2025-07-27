#include "item_sorting.h"
#include <string.h>
#include <ctype.h>

static int _strcasecmp(const char *a, const char *b) {
    // simple case‑insensitive compare
    while (*a && *b) {
        char ca = tolower((unsigned char)*a++);
        char cb = tolower((unsigned char)*b++);
        if (ca != cb) return (ca < cb) ? -1 : 1;
    }
    return (*a == *b) ? 0 : ((*a) ? 1 : -1);
}

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
}

const char* item_sorting_size_string(item_size_t s) {
    switch (s) {
    case SIZE_SMALL:  return "SMALL";
    case SIZE_MEDIUM: return "MEDIUM";
    case SIZE_LARGE:  return "LARGE";
    default:          return "UNKNOWN";
    }
}

const char* item_sorting_type_string(item_type_t t) {
    switch (t) {
    case TYPE_FROZEN: return "FROZEN";
    case TYPE_DRY:    return "NORMAL";
    default:          return "UNKNOWN";
    }
}

const char* item_sorting_phase_string(item_phase_t p) {
    switch (p) {
    case PHASE_SOLID:  return "SOLID";
    case PHASE_LIQUID: return "LIQUID";
    default:           return "UNKNOWN";
    }
}
