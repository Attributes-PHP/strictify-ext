#ifndef AS_STRUCTS_H
#define AS_STRUCTS_H

#include "php.h"

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