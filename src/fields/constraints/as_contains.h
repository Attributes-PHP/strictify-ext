#ifndef AS_Fields_Constraints_Contains_H
#define AS_Fields_Constraints_Contains_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_Contains_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Constraints_Contains___construct, 0, 0, 1)
/* Union-typed variadics need PHP 8.3 arginfo macros; untyped on 8.2 */
ZEND_ARG_VARIADIC_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Constraints_Contains, __construct);

static const zend_function_entry class_AS_Fields_Constraints_Contains_methods[] = {ZEND_ME(AS_Fields_Constraints_Contains, __construct, arginfo_class_AS_Fields_Constraints_Contains___construct, ZEND_ACC_PUBLIC)
                                                                                       ZEND_FE_END};

bool as_validate_Contains_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);
void as_register_Contains_class(void);

zend_always_inline static void add_invalid_contains_error_message(zval *required, as_field *field, as_property_info *property, zval *errors);

#endif /* AS_Fields_Constraints_Contains_H */
