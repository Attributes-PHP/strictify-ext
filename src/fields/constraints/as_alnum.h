#ifndef AS_Fields_Constraints_Alnum_H
#define AS_Fields_Constraints_Alnum_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_Alnum_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Constraints_Alnum___construct, 0, 0, 0)
ZEND_ARG_TYPE_INFO(0, extra, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Constraints_Alnum, __construct);

static const zend_function_entry class_AS_Fields_Constraints_Alnum_methods[] = {ZEND_ME(AS_Fields_Constraints_Alnum, __construct, arginfo_class_AS_Fields_Constraints_Alnum___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool as_validate_Alnum_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_Alnum_class(void);

#endif /* AS_Fields_Constraints_Alnum_H */
