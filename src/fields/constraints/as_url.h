#ifndef AS_Fields_Constraints_Url_H
#define AS_Fields_Constraints_Url_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_Url_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Constraints_Url___construct, 0, 0, 0)
ZEND_ARG_TYPE_MASK(0, protocol, MAY_BE_STRING | MAY_BE_ARRAY | MAY_BE_NULL, "null")
ZEND_ARG_TYPE_MASK(0, domain, MAY_BE_STRING | MAY_BE_ARRAY | MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Constraints_Url, __construct);

static const zend_function_entry class_AS_Fields_Constraints_Url_methods[] = {ZEND_ME(AS_Fields_Constraints_Url, __construct, arginfo_class_AS_Fields_Constraints_Url___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool as_validate_Url_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_Url_class(void);

#endif /* AS_Fields_Constraints_Url_H */
