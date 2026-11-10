#include "as_validate_function.h"
#include "as_compiled_model.h"
#include "as_exception.h"
#include "as_base_model.h"
#include "as_model_configs.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_types.h"
#include "php.h"

zend_result as_hydrate_model(zval *raw_data, zval *model)
{
    // The compiled plan carries the parsed ModelConfigs and the hook
    // override check; only a model overriding a hook pays for the
    // ModelConfigs object
    as_compiled_model *plan = as_get_root_compiled_model(model);
    if (UNEXPECTED(plan == NULL)) {
        return FAILURE;
    }

    zval configs_obj;
    ZVAL_UNDEF(&configs_obj);

    if (plan->hooks_overridden) {
        as_model_configs_properties hook_properties;
        as_get_model_configs(&configs_obj, model, &hook_properties, true);
        if (UNEXPECTED(EG(exception) != NULL)) {
            zval_ptr_dtor(&configs_obj);
            return FAILURE;
        }

        as_call_before_validation_hook(model, raw_data, &configs_obj);
        if (EG(exception)) {
            zval_ptr_dtor(&configs_obj);
            return FAILURE;
        }
    }

    zval errors;
    array_init(&errors);

    if (!as_compiled_validate_model(raw_data, model, &plan->configs, &errors, NULL)) {
        // A failure with a pending exception comes from a thrown error
        // (unbuildable attribute spec, class not found, hook exception):
        // let it propagate instead of masking it with an empty
        // ValidationException
        if (UNEXPECTED(EG(exception) != NULL)) {
            zval_ptr_dtor(&configs_obj);
            zval_ptr_dtor(&errors);
            return FAILURE;
        }

        ZEND_ASSERT(zend_hash_num_elements(Z_ARRVAL_P(&errors)) > 0);

        as_throw_validation_exception(&errors);
        zval_ptr_dtor(&configs_obj);
        zval_ptr_dtor(&errors);
        return FAILURE;
    }

    if (plan->hooks_overridden) {
        as_call_after_validation_hook(model, raw_data, &configs_obj);
        zval_ptr_dtor(&configs_obj);
    }
    zval_ptr_dtor(&errors);

    if (UNEXPECTED(EG(exception) != NULL)) {
        return FAILURE;
    }

    return SUCCESS;
}

/* Function implementation for validate */
ZEND_FUNCTION(validate)
{
    zval *raw_data;
    zval *model;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_ARRAY(raw_data)
    Z_PARAM_OBJECT_OF_CLASS(model, AS_BaseModel_ce)
    ZEND_PARSE_PARAMETERS_END();

    if (as_hydrate_model(raw_data, model) == FAILURE) {
        RETURN_THROWS();
    }

    RETURN_COPY(model);
}
