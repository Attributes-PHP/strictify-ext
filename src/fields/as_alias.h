#ifndef AS_FIELDS_Alias_H
#define AS_FIELDS_Alias_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Alias_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Alias___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Alias, __construct);

static const zend_function_entry class_AS_Fields_Alias_methods[] = {ZEND_ME(AS_Fields_Alias, __construct, arginfo_class_AS_Fields_Alias___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void as_register_Alias_class(void);

#endif /* AS_FIELDS_Alias_H */
