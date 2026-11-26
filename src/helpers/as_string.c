#include "as_string.h"
#include "Zend/zend_API.h"
#include "as_wrappers.h"
#include "testing.h"
#include <ctype.h>
#include <stddef.h>
#include "as_structs.h"

/**
 * Checks if a character is uppercase
 */
STATIC_ZEND_INLINE bool is_uppercase(char c)
{
    return c >= 'A' && c <= 'Z';
}

/**
 * Checks if a character is lowercase
 */
STATIC_ZEND_INLINE bool is_lowercase(char c)
{
    return c >= 'a' && c <= 'z';
}

/**
 * Checks if a character is alphanumeric
 */
STATIC_ZEND_INLINE bool is_alphanumeric(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

/*
 * The four alias generators differ in two knobs only: word boundaries are
 * either dropped with the next letter capitalized (pascal/camel) or folded
 * into a separator (snake/kebab), and the first letter is upper-, lower-
 * cased or left alone.
 */
typedef enum {
    AS_CASE_PASCAL,
    AS_CASE_CAMEL,
    AS_CASE_SNAKE,
    AS_CASE_KEBAB,
} as_case;

STATIC_INLINE void to_case(as_string *input, as_string *output, as_case mode)
{
    const bool word_boundary = mode == AS_CASE_PASCAL || mode == AS_CASE_CAMEL;
    const char separator = mode == AS_CASE_SNAKE ? '_' : '-';
    bool next_upper = mode == AS_CASE_PASCAL;
    bool first_char = true;

    for (size_t i = 0; i < input->length; i++) {
        const char c = input->value[i];

        if (c == '_' || c == '-' || c == ' ') {
            if (word_boundary) {
                next_upper = true;
            } else {
                output->value[output->length++] = separator;
            }
            continue;
        }

        if (word_boundary) {
            if (first_char && mode == AS_CASE_CAMEL) {
                output->value[output->length++] = tolower(c);
            } else if (next_upper) {
                output->value[output->length++] = is_lowercase(c) ? toupper(c) : c;
            } else {
                output->value[output->length++] = c;
            }

            first_char = false;
            next_upper = false;
            continue;
        }

        if (is_uppercase(c)) {
            // Add the separator before an uppercase letter, except at the start
            if (output->length > 0 && output->value[output->length - 1] != separator) {
                output->value[output->length++] = separator;
            }
            output->value[output->length++] = tolower(c);
        } else {
            output->value[output->length++] = c;
        }
    }

    output->value[output->length] = '\0';
}

/*
 * Converts a property name to the requested naming convention. Worst case
 * is two bytes per input byte (a separator inserted before every letter),
 * which every mode fits in.
 */
static zend_string *as_to_case(zend_string *str, as_case mode)
{
    as_string input, output;
    input.value = ZSTR_VAL(str);
    input.length = ZSTR_LEN(str);

    output.value = as_emalloc(input.length * 2 + 1);
    output.length = 0;

    to_case(&input, &output, mode);
    zend_string *result = as_string_init(output.value, output.length, 0);
    as_efree(output.value);
    return result;
}

/**
 * Converts a property name to PascalCase
 * Example: "firstName" -> "FirstName", "first_name" -> "FirstName"
 */
zend_string *as_to_pascal_case(zend_string *str)
{
    return as_to_case(str, AS_CASE_PASCAL);
}

/**
 * Converts a property name to camelCase
 * Example: "FirstName" -> "firstName", "first_name" -> "firstName"
 */
zend_string *as_to_camel_case(zend_string *str)
{
    return as_to_case(str, AS_CASE_CAMEL);
}

/**
 * Converts a property name to snake_case
 * Example: "FirstName" -> "first_name", "firstName" -> "first_name"
 */
zend_string *as_to_snake_case(zend_string *str)
{
    return as_to_case(str, AS_CASE_SNAKE);
}

/**
 * Converts a property name to kebab-case
 * Example: "FirstName" -> "first-name", "firstName" -> "first-name"
 */
zend_string *as_to_kebab_case(zend_string *str)
{
    return as_to_case(str, AS_CASE_KEBAB);
}

/**
 * Concats two strings with a dot
 * Example: ("first", "second") -> "first.second"
 */
zend_string *as_string_dot_concat(zend_string *first, zend_string *second)
{
    if (!first || !second) {
        return NULL;
    }

    return as_string_concat3(ZSTR_VAL(first), ZSTR_LEN(first), ".", 1, ZSTR_VAL(second), ZSTR_LEN(second));
}
