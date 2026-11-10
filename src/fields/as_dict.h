#ifndef AS_FIELDS_Dict_H
#define AS_FIELDS_Dict_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Dict_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Dict___construct, 0, 0, 2)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, of)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Dict, __construct);

static const zend_function_entry class_AS_Fields_Dict_methods[] = {ZEND_ME(AS_Fields_Dict, __construct, arginfo_class_AS_Fields_Dict___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void as_register_Dict_class(void);

#endif /* AS_FIELDS_Dict_H */
