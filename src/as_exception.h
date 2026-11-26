#ifndef AS_EXCEPTION_H
#define AS_EXCEPTION_H

#include "php.h"

/* Class entries */
extern zend_class_entry *class_AS_Exceptions_BaseException;
extern zend_class_entry *class_AS_Exceptions_ValidationException;
extern zend_class_entry *class_AS_Exceptions_InvalidBaseModelException;
extern zend_class_entry *class_AS_Exceptions_MaxDepthException;

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

static const zend_function_entry class_AS_Exceptions_InvalidBaseModelException_methods[] = {ZEND_FE_END};

static const zend_function_entry class_AS_Exceptions_MaxDepthException_methods[] = {ZEND_FE_END};

static const zend_function_entry class_AS_Exceptions_ValidationException_methods[] = {ZEND_ME(AS_Exceptions_ValidationException, __construct, arginfo_class_AS_Exceptions_ValidationException___construct, ZEND_ACC_PUBLIC)
                                                                                          ZEND_ME(AS_Exceptions_ValidationException, getErrors, arginfo_class_AS_Exceptions_ValidationException_getErrors, ZEND_ACC_PUBLIC)
                                                                                              ZEND_FE_END};

/* Registration functions */
void as_register_all_exception_classes(void);
void as_throw_validation_exception(zval *errors);

/*
 * Rewrites a pending TypeError or ValueError into
 * InvalidBaseModelException, keeping the message.
 *
 * Attribute arguments are part of the model declaration: a constructor that
 * rejects one — an engine TypeError for the stub signature, a ValueError from
 * a nested attribute or from a custom argument rule — reports a malformed
 * model, not a bad call. Called at the attribute-evaluation choke points, so
 * plain `new Size('x')` in userland keeps the engine's TypeError. Any other
 * pending exception (an autoloader error, a userland Error) is left alone.
 */
void as_convert_attribute_exception(void);

#endif /* AS_EXCEPTION_H */
