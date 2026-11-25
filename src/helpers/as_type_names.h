#ifndef AS_HELPERS_AS_TYPE_NAMES_H
#define AS_HELPERS_AS_TYPE_NAMES_H

#include <Zend/zend.h>
#include <Zend/zend_string.h>
#include <Zend/zend_type_info.h>
#include "as_wrappers.h"

/*
 * Single source of truth for rendering native type hints in messages.
 * The table order is the order the names appear in ("boolean, integer or
 * float"), so every caller that names a native type gets the same wording.
 */
typedef struct {
    uint32_t mask;
    const char *name;
    size_t length;
} as_type_name;

static const as_type_name as_type_names[] = {
    {MAY_BE_BOOL, "boolean", sizeof("boolean") - 1}, {MAY_BE_LONG, "integer", sizeof("integer") - 1}, {MAY_BE_DOUBLE, "float", sizeof("float") - 1}, {MAY_BE_STRING, "string", sizeof("string") - 1},
    {MAY_BE_ARRAY, "array", sizeof("array") - 1},    {MAY_BE_OBJECT, "object", sizeof("object") - 1}, {MAY_BE_NULL, "null", sizeof("null") - 1},
};

#define AS_TYPE_NAMES_COUNT (sizeof(as_type_names) / sizeof(as_type_names[0]))

/* Number of basic types set in the mask. */
static zend_always_inline uint32_t as_type_mask_count(uint32_t mask)
{
    uint32_t total = 0;

    for (size_t i = 0; i < AS_TYPE_NAMES_COUNT; i++) {
        if (mask & as_type_names[i].mask) {
            total += 1;
        }
    }

    return total;
}

/* The single type an exact mask names, or NULL when the mask is not exactly one type. */
static zend_always_inline const as_type_name *as_type_name_for(uint32_t mask)
{
    for (size_t i = 0; i < AS_TYPE_NAMES_COUNT; i++) {
        if (mask == as_type_names[i].mask) {
            return &as_type_names[i];
        }
    }

    return NULL;
}

/*
 * Expands a mask into its type names: "boolean, integer or float" with
 * or_last (union semantics), "boolean, integer, float" without, and "mixed"
 * when nothing in the mask has a name.
 */
static zend_always_inline zend_string *as_type_mask_to_string(uint32_t mask, bool or_last)
{
    const uint32_t total = as_type_mask_count(mask);

    if (total == 0) {
        return as_string_init("mixed", sizeof("mixed") - 1, 0);
    }

    size_t max_string_size = 0;
    for (size_t i = 0; i < AS_TYPE_NAMES_COUNT; i++) {
        if (mask & as_type_names[i].mask) {
            max_string_size += as_type_names[i].length;
        }
    }

    // Every type but the last is followed by ", "; the last by " or " (union
    // list) or ", " (plain list).
    if (total >= 2) {
        max_string_size += (total - 2) * (sizeof(", ") - 1) + (or_last ? (sizeof(" or ") - 1) : (sizeof(", ") - 1));
    }

    zend_string *result = as_string_alloc(max_string_size, 0);
    char *output = ZSTR_VAL(result);
    size_t output_pos = 0;
    uint32_t remaining = total;

    for (size_t i = 0; i < AS_TYPE_NAMES_COUNT; i++) {
        if (!(mask & as_type_names[i].mask)) {
            continue;
        }

        if (output_pos > 0) {
            const char *separator = (or_last && remaining == 1) ? " or " : ", ";
            const size_t separator_len = (or_last && remaining == 1) ? (sizeof(" or ") - 1) : (sizeof(", ") - 1);
            memcpy(output + output_pos, separator, separator_len);
            output_pos += separator_len;
        }

        memcpy(output + output_pos, as_type_names[i].name, as_type_names[i].length);
        output_pos += as_type_names[i].length;
        remaining -= 1;
    }

    output[output_pos] = '\0';

    ZEND_ASSERT(output_pos == max_string_size);
    return as_string_truncate(result, output_pos, 0);
}

#endif /* AS_HELPERS_AS_TYPE_NAMES_H */
