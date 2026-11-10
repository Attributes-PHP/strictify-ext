#ifndef AS_BASEMODEL_H
#define AS_BASEMODEL_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AS_BaseModel_ce;

/* Argument info for BaseModel methods */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_BaseModel_beforeValidation, 0, 2, IS_ARRAY, 0)
ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
ZEND_ARG_OBJ_INFO(0, configs, Attributes\\Strictify\\ModelConfigs, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_BaseModel_afterValidation, 0, 2, IS_VOID, 0)
ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
ZEND_ARG_OBJ_INFO(0, configs, Attributes\\Strictify\\ModelConfigs, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_BaseModel_toDict, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_BaseModel, beforeValidation);
ZEND_METHOD(AS_BaseModel, afterValidation);
ZEND_METHOD(AS_BaseModel, toDict);

static const zend_function_entry class_AS_BaseModel_methods[] = {ZEND_ME(AS_BaseModel, beforeValidation, arginfo_class_AS_BaseModel_beforeValidation, ZEND_ACC_PROTECTED)
                                                                     ZEND_ME(AS_BaseModel, afterValidation, arginfo_class_AS_BaseModel_afterValidation, ZEND_ACC_PROTECTED)
                                                                     ZEND_ME(AS_BaseModel, toDict, arginfo_class_AS_BaseModel_toDict, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void as_register_BaseModel_class(void);
void as_call_before_validation_hook(zval *model, zval *raw_data, zval *configs);
void as_call_after_validation_hook(zval *model, zval *raw_data, zval *configs);
bool as_model_overrides_hooks(zend_class_entry *model_ce);

#endif /* AS_BASEMODEL_H */
