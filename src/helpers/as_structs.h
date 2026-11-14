#ifndef AS_STRUCTS_H
#define AS_STRUCTS_H

#include "php.h"

/*
 * Placeholder rendered instead of the real value of a #[Sensitive] field
 * wherever the library exposes it (toDict() output, the {value} error
 * message placeholder), mirroring how #[\SensitiveParameter] redacts
 * parameter values in stack traces.
 */
#define AS_SENSITIVE_VALUE_MASK "***"

typedef struct {
    zend_string *parent;
    zend_string *name;
    zval *value;
} as_field;

typedef struct {
    zval *model;
    zend_class_entry *model_ce;
    zend_property_info *property;
} as_property_info;

#endif /* AS_STRUCTS_H */