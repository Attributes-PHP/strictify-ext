#ifndef AS_Fields_Constraints_Email_H
#define AS_Fields_Constraints_Email_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_Email_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Constraints_Email___construct, 0, 0, 0)
ZEND_ARG_TYPE_MASK(0, domain, MAY_BE_STRING | MAY_BE_ARRAY | MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Constraints_Email, __construct);

static const zend_function_entry class_AS_Fields_Constraints_Email_methods[] = {ZEND_ME(AS_Fields_Constraints_Email, __construct, arginfo_class_AS_Fields_Constraints_Email___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool as_validate_Email_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_Email_class(void);

#endif /* AS_Fields_Constraints_Email_H */
