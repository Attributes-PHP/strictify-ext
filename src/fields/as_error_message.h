#ifndef AS_FIELDS_ERROR_MESSAGE_H
#define AS_FIELDS_ERROR_MESSAGE_H

#include "php.h"
#include "../helpers/as_error_messages.h"

/* Class entry */
extern zend_class_entry *AS_Fields_ErrorMessage_ce;

/*
 * #[ErrorMessage] keys: one constructor argument per default error catalog
 * key, in as_error_type enum order (so positional arguments map to the enum
 * index). Each entry is (C identifier, error type, PHP-visible key name) —
 * the name differs from the identifier only for `union`, a C keyword. The
 * `extra` key is discarded: extra inputs have no owning property, so the
 * extra error bypasses per-property overrides.
 */
#define AS_ERROR_MESSAGE_KEYS(X)                                    \
    X(required, AS_ERROR_REQUIRED, "required")                      \
    X(type, AS_ERROR_TYPE, "type")                                  \
    X(alpha_dash, AS_ERROR_ALPHA_DASH, "alpha_dash")                \
    X(alpha_dash_type, AS_ERROR_ALPHA_DASH_TYPE, "alpha_dash_type") \
    X(ascii, AS_ERROR_ASCII, "ascii")                               \
    X(ascii_type, AS_ERROR_ASCII_TYPE, "ascii_type")                \
    X(between, AS_ERROR_BETWEEN, "between")                         \
    X(between_type, AS_ERROR_BETWEEN_TYPE, "between_type")          \
    X(contains, AS_ERROR_CONTAINS, "contains")                      \
    X(datetime, AS_ERROR_DATETIME, "datetime")                      \
    X(ends_with, AS_ERROR_ENDS_WITH, "ends_with")                   \
    X(in, AS_ERROR_IN, "in")                                        \
    X(max, AS_ERROR_MAX, "max")                                     \
    X(min, AS_ERROR_MIN, "min")                                     \
    X(multiple_of, AS_ERROR_MULTIPLE_OF, "multiple_of")             \
    X(regex, AS_ERROR_REGEX, "regex")                               \
    X(size, AS_ERROR_SIZE, "size")                                  \
    X(starts_with, AS_ERROR_STARTS_WITH, "starts_with")             \
    X(ulid, AS_ERROR_ULID, "ulid")                                  \
    X(uuid, AS_ERROR_UUID, "uuid")                                  \
    X(union_, AS_ERROR_UNION, "union")                              \
    X(sequence_keys, AS_ERROR_SEQUENCE_KEYS, "sequence_keys")

#define AS_ERROR_MESSAGE_KEY_COUNT 22

/* Same entry as ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0), but with an
 * explicit name string so C keywords (union) can be used as keys */
#define AS_ERROR_MESSAGE_ARG_INFO(c_name, error_type, str_name) {str_name, ZEND_TYPE_INIT_CODE(IS_STRING, 0, _ZEND_ARG_INFO_FLAGS(0, 0, 0)), NULL},

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_ErrorMessage___construct, 0, 0, 0)
AS_ERROR_MESSAGE_KEYS(AS_ERROR_MESSAGE_ARG_INFO)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_ErrorMessage, __construct);

static const zend_function_entry class_AS_Fields_ErrorMessage_methods[] = {ZEND_ME(AS_Fields_ErrorMessage, __construct, arginfo_class_AS_Fields_ErrorMessage___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void as_register_ErrorMessage_class(void);

#endif /* AS_FIELDS_ERROR_MESSAGE_H */
