#ifndef AS_Fields_Constraints_StartsWith_H
#define AS_Fields_Constraints_StartsWith_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_StartsWith_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Constraints_StartsWith___construct, 0, 0, 0)
/* Union-typed variadics need PHP 8.3 arginfo macros; untyped on 8.2 */
ZEND_ARG_VARIADIC_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Constraints_StartsWith, __construct);

static const zend_function_entry class_AS_Fields_Constraints_StartsWith_methods[] = {ZEND_ME(AS_Fields_Constraints_StartsWith, __construct, arginfo_class_AS_Fields_Constraints_StartsWith___construct, ZEND_ACC_PUBLIC)
                                                                                         ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool as_validate_StartsWith_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_StartsWith_class(void);

#endif /* AS_Fields_Constraints_StartsWith_H */
