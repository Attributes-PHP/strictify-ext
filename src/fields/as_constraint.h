#ifndef AS_FIELDS_Constraint_H
#define AS_FIELDS_Constraint_H

#include "php.h"
#include "../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraint_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_Fields_Constraint_validate, 0, 1, _IS_BOOL, 0)
ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_Fields_Constraint_message, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry class_AS_Fields_Constraint_methods[] = {ZEND_ABSTRACT_ME_WITH_FLAGS(AS_Fields_Constraint, validate, arginfo_class_AS_Fields_Constraint_validate, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
                                                                             ZEND_ABSTRACT_ME_WITH_FLAGS(AS_Fields_Constraint, message, arginfo_class_AS_Fields_Constraint_message, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
                                                                                 ZEND_FE_END};

/* Registration function */
void as_register_Constraint_interface(void);

/* Validation logic for userland Constraint implementations: calls the
 * instance's validate() and, on failure, message() methods. */
bool as_validate_userland_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Argument rules of a compiled built-in constraint instance, run once per
 * field compile: rejects the arguments that would make the constraint
 * unsatisfiable or a no-op (inverted or NAN bounds, empty value lists,
 * invalid regex patterns, empty domain lists). Returns false with a
 * pending ValueError. */
bool as_validate_constraint_attribute_args(zval *constraint, as_property_info *property);

#endif /* AS_FIELDS_Constraint_H */
