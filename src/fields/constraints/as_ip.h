#ifndef AS_Fields_Constraints_Ip_H
#define AS_Fields_Constraints_Ip_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_Ip_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Constraints_Ip___construct, 0, 0, 0)
ZEND_ARG_TYPE_INFO(0, version, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Constraints_Ip, __construct);

static const zend_function_entry class_AS_Fields_Constraints_Ip_methods[] = {ZEND_ME(AS_Fields_Constraints_Ip, __construct, arginfo_class_AS_Fields_Constraints_Ip___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool as_validate_Ip_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_Ip_class(void);

#endif /* AS_Fields_Constraints_Ip_H */
