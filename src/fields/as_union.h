#ifndef AS_FIELDS_Union_H
#define AS_FIELDS_Union_H

#include "php.h"
#include "../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Union_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Union___construct, 0, 0, 0)
ZEND_ARG_VARIADIC_INFO(0, of)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Union, __construct);

static const zend_function_entry class_AS_Fields_Union_methods[] = {ZEND_ME(AS_Fields_Union, __construct, arginfo_class_AS_Fields_Union___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/*
 * Scalar whole-value validation: when a Union attribute is declared
 * directly on a non-array property (int|float, string, mixed, untyped),
 * the constraint dispatch calls this and the value must match at least
 * one arm exactly. Array values are rejected here with a ValueError:
 * array element specs belong inside Sequence(of: ...) or Dict(of: ...).
 */
bool as_validate_Union_scalar(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_Union_class(void);

#endif /* AS_FIELDS_Union_H */
