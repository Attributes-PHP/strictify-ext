#ifndef AS_Fields_Constraints_MultipleOf_H
#define AS_Fields_Constraints_MultipleOf_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_MultipleOf_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Constraints_MultipleOf___construct, 0, 0, 1)
ZEND_ARG_TYPE_MASK(0, value, MAY_BE_LONG | MAY_BE_DOUBLE, NULL)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Constraints_MultipleOf, __construct);

static const zend_function_entry class_AS_Fields_Constraints_MultipleOf_methods[] = {ZEND_ME(AS_Fields_Constraints_MultipleOf, __construct, arginfo_class_AS_Fields_Constraints_MultipleOf___construct, ZEND_ACC_PUBLIC)
                                                                                         ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool as_validate_MultipleOf_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_MultipleOf_class(void);
zend_always_inline static void add_invalid_multiple_of_error_message(zval *bound, as_field *field, as_property_info *property, zval *errors);

#endif /* AS_Fields_Constraints_MultipleOf_H */
