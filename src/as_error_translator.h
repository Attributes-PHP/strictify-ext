#ifndef AS_ERROR_TRANSLATOR_H
#define AS_ERROR_TRANSLATOR_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AS_ErrorTranslator_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ErrorTranslator_get, 0, 0, IS_CALLABLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ErrorTranslator_set, 0, 1, IS_VOID, 0)
ZEND_ARG_TYPE_INFO(0, translator, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_ErrorTranslator, get);
ZEND_METHOD(AS_ErrorTranslator, set);

static const zend_function_entry class_AS_ErrorTranslator_methods[] = {
    ZEND_ME(AS_ErrorTranslator, get, arginfo_class_AS_ErrorTranslator_get, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) ZEND_ME(AS_ErrorTranslator, set, arginfo_class_AS_ErrorTranslator_set, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) ZEND_FE_END
};

/* Registration function */
void as_register_ErrorTranslator_class(void);

#endif /* AS_ERROR_TRANSLATOR_H */
