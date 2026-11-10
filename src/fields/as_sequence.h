#ifndef AS_FIELDS_Sequence_H
#define AS_FIELDS_Sequence_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Sequence_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_Sequence___construct, 0, 0, 1)
ZEND_ARG_INFO(0, of)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_Sequence, __construct);

static const zend_function_entry class_AS_Fields_Sequence_methods[] = {ZEND_ME(AS_Fields_Sequence, __construct, arginfo_class_AS_Fields_Sequence___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void as_register_Sequence_class(void);

#endif /* AS_FIELDS_Sequence_H */
