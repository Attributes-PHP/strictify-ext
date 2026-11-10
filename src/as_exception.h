#ifndef AS_EXCEPTION_H
#define AS_EXCEPTION_H

#include "php.h"

/* Class entries */
extern zend_class_entry *class_AS_Exceptions_BaseException;
extern zend_class_entry *class_AS_Exceptions_ValidationException;

/* Method declaration */
ZEND_METHOD(AS_Exceptions_ValidationException, __construct);
ZEND_METHOD(AS_Exceptions_ValidationException, getErrors);

/* Argument info for ValidationException methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Exceptions_ValidationException___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, allErrors, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_Exceptions_ValidationException_getErrors, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry class_AS_Exceptions_BaseException_methods[] = {ZEND_FE_END};

static const zend_function_entry class_AS_Exceptions_ValidationException_methods[] = {ZEND_ME(AS_Exceptions_ValidationException, __construct, arginfo_class_AS_Exceptions_ValidationException___construct, ZEND_ACC_PUBLIC)
                                                                                          ZEND_ME(AS_Exceptions_ValidationException, getErrors, arginfo_class_AS_Exceptions_ValidationException_getErrors, ZEND_ACC_PUBLIC)
                                                                                              ZEND_FE_END};

/* Registration functions */
void as_register_all_exception_classes(void);
void as_throw_validation_exception(zval *errors);

#endif /* AS_EXCEPTION_H */
