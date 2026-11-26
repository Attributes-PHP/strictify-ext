#ifndef AS_VALIDATE_FUNCTION_H
#define AS_VALIDATE_FUNCTION_H

#include "php.h"
#include "as_model_configs.h"
#include "as_exception.h"
#include "helpers/as_structs.h"

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_AS_validate, 0, 2, Attributes\\Strictify\\BaseModel, 0)
ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
ZEND_ARG_OBJ_INFO(0, model, Attributes\\Strictify\\BaseModel, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxDepth, IS_LONG, 0, "512")
ZEND_END_ARG_INFO()

ZEND_FUNCTION(validate);

/**
 * Applies a $maxDepth argument for the duration of one validate()/call()
 * run. Returns the previous limit, which the caller restores once the run is
 * over, so a hook that validates again starts from the default.
 */
uint32_t as_apply_validation_depth_limit(zend_long limit);
void as_restore_validation_depth_limit(uint32_t previous);

/**
 * Runs the full validation flow for one model instance: ModelConfigs,
 * before/after validation hooks and property hydration from raw data.
 *
 * @param raw_data  The raw data to hydrate the model from
 * @param model     The model instance to fill in
 * @return          SUCCESS with the model filled in, or FAILURE after
 *                  throwing (ValidationException or a hook exception)
 */
zend_result as_hydrate_model(zval *raw_data, zval *model);

#endif /* AS_VALIDATE_FUNCTION_H */
