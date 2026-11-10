#include "as_base_model.h"
#include "as_compiled_model.h"
#include "Zend/zend_API.h"
#include "Zend/zend_interfaces.h"

zend_class_entry *AS_BaseModel_ce;

/* Method implementations */
ZEND_METHOD(AS_BaseModel, beforeValidation)
{
    RETURN_NULL();
}

ZEND_METHOD(AS_BaseModel, afterValidation)
{
    RETURN_NULL();
}

ZEND_METHOD(AS_BaseModel, toDict)
{
    ZEND_PARSE_PARAMETERS_NONE();

    if (!as_compiled_model_to_dict(getThis(), return_value)) {
        RETURN_THROWS();
    }
}

/* Registration function */
void as_register_BaseModel_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify", "BaseModel", class_AS_BaseModel_methods);
    AS_BaseModel_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_BaseModel_ce != NULL);
    AS_BaseModel_ce->ce_flags |= ZEND_ACC_ABSTRACT;
}

/**
 * Calls beforeValidation method from BaseModel e.g. $rawData = $model->beforeValidation($rawData, $configs)
 * This hook can modify the rawData before validation begins
 **/
void as_call_before_validation_hook(zval *model, zval *raw_data, zval *configs)
{
    zval before_validation_result;
    zend_class_entry *model_ce = Z_OBJCE_P(model);

    ZVAL_UNDEF(&before_validation_result);
    zend_call_method_with_2_params(Z_OBJ_P(model), model_ce, NULL, "beforeValidation", &before_validation_result, raw_data, configs);
    if (EG(exception)) {
        zval_ptr_dtor(&before_validation_result);
        return;
    }

    if (Z_TYPE(before_validation_result) == IS_ARRAY) {
        ZVAL_COPY(raw_data, &before_validation_result);
    }

    zval_ptr_dtor(&before_validation_result);
}

/** Calls afterValidation method from BaseModel e.g. $model->afterValidation($rawData) **/
void as_call_after_validation_hook(zval *model, zval *raw_data, zval *configs)
{
    zend_call_method_with_2_params(Z_OBJ_P(model), Z_OBJCE_P(model), NULL, "afterValidation", NULL, raw_data, configs);
}

/*
 * Whether the model class overrides a validation hook. Inherited (but not
 * overridden) entries stay scoped to BaseModel, so comparing the declaring
 * scope detects overrides without instantiation.
 */
bool as_model_overrides_hooks(zend_class_entry *model_ce)
{
    zend_function *before = zend_hash_str_find_ptr(&model_ce->function_table, "beforevalidation", sizeof("beforevalidation") - 1);
    zend_function *after = zend_hash_str_find_ptr(&model_ce->function_table, "aftervalidation", sizeof("aftervalidation") - 1);

    return (before != NULL && before->common.scope != AS_BaseModel_ce) || (after != NULL && after->common.scope != AS_BaseModel_ce);
}
