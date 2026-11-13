#ifndef AS_ERROR_MESSAGES_H
#define AS_ERROR_MESSAGES_H

#include "as_structs.h"
#include "php.h"
#include "zend_compile.h"
#include "zend_types.h"

typedef enum {
    AS_ERROR_REQUIRED,
    AS_ERROR_TYPE,
    /* Constraint error types: one per constraint, each with its own
     * default template. The #[ErrorMessage] attribute only declares
     * the required and type arguments, so constraint errors always use
     * their default template. */
    AS_ERROR_ALPHA_DASH,
    /* Non-string value under #[AlphaDash]: reported with its own template
     * because it is a type failure, not a character-content failure */
    AS_ERROR_ALPHA_DASH_TYPE,
    AS_ERROR_ASCII,
    /* Non-string value under #[Ascii]: reported with its own template
     * because it is a type failure, not a character-content failure */
    AS_ERROR_ASCII_TYPE,
    AS_ERROR_BETWEEN,
    /* Non-numeric value under #[Between]: reported with its own template
     * because it is a type failure, not a range failure */
    AS_ERROR_BETWEEN_TYPE,
    AS_ERROR_CONTAINS,
    AS_ERROR_DATETIME,
    AS_ERROR_ENDS_WITH,
    AS_ERROR_IN,
    AS_ERROR_MAX,
    AS_ERROR_MIN,
    AS_ERROR_MULTIPLE_OF,
    AS_ERROR_REGEX,
    AS_ERROR_SIZE,
    AS_ERROR_STARTS_WITH,
    AS_ERROR_ULID,
    AS_ERROR_UUID,
    AS_ERROR_TYPE_COUNT
} as_error_type;

typedef struct {
    uint32_t mask;
    const char *name;
    size_t length;
} as_basic_type_mapping;

// Default error message templates, one per error type (defined in as_error_messages.c).
// Supported placeholders: {field}, {value} and {expected}
extern const char *as_default_error_type_messages[];

// Stable message keys, one per error type, passed to the translator registered
// via ModelConfigs::setDefaultErrorMessages() (defined in as_error_messages.c).
extern const char *as_error_type_names[];

// Placeholder substitution
zend_string *as_replace_placeholders(const char *template, size_t length, as_field *field, as_property_info *prop_info);

// Error message generation
zend_string *build_union_type_string(zend_type property_type);

// Value conversion
zend_string *as_value_to_string(zval *value);

// Renders a list of scalar values as "a, b, c" (each through
// as_value_to_string), for constraint messages listing allowed or
// searched values. The result is a freshly allocated zend_string the
// caller must release.
zend_string *as_values_to_list_string(zval *values);

// Updates errors
void as_add_field_error_with_prefix(as_error_type type, as_field *field, as_property_info *property, zval *errors);

// Frees the per-property error template cache (request shutdown)
void as_clear_error_templates_cache(void);

// Error message generation with an explicit {expected} substitution, for
// errors whose expected type cannot be derived from the native type hint
void as_add_field_error_with_expected(as_error_type type, as_field *field, as_property_info *property, zval *errors, const zend_string *expected);

// Appends an already-rendered message under the field's dot path
void as_add_field_message(zend_string *message, as_field *field, zval *errors);

#endif /* AS_ERROR_MESSAGES_H */