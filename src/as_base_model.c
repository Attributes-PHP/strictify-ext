#include "as_base_model.h"
#include "as_compiled_model.h"
#include "helpers/as_structs.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_hash.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_object_handlers.h"
#include "Zend/zend_objects.h"
#include <string.h>

zend_class_entry *AS_BaseModel_ce;

static zend_object_handlers as_base_model_object_handlers;

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
    bool include_sensitive = false;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(include_sensitive)
    ZEND_PARSE_PARAMETERS_END();

    if (!as_compiled_model_to_dict(getThis(), include_sensitive, return_value)) {
        RETURN_THROWS();
    }
}

/*
 * Whether the property carries the #[Sensitive] attribute: its value is
 * masked in debug output (see as_base_model_get_properties_for), like
 * #[\\SensitiveParameter] masks arguments in stack traces.
 */
static bool as_property_info_is_sensitive(zend_property_info *prop)
{
    return prop != NULL && prop->attributes != NULL &&
           zend_get_attribute_str(prop->attributes, "attributes\\strictify\\fields\\sensitive", sizeof("attributes\\strictify\\fields\\sensitive") - 1) != NULL;
}

/*
 * Debug output (var_dump, print_r, debug_zval_dump, and model objects dumped
 * inside exception traces) fetches the properties with ZEND_PROP_PURPOSE_DEBUG.
 * That view replaces every #[Sensitive] property with AS_SENSITIVE_VALUE_MASK
 * on a temporary copy, so the real values stay reachable through property
 * access, foreach, (array) casts, get_object_vars, json_encode, serialize and
 * var_export, which use other purposes and fall through to the standard
 * handler unchanged. prop->name is already the (possibly mangled) key used in
 * the object properties hash, for any visibility.
 */
static zend_array *as_base_model_get_properties_for(zend_object *object, zend_prop_purpose purpose)
{
    zend_array *properties = zend_std_get_properties_for(object, purpose);

    if (purpose != ZEND_PROP_PURPOSE_DEBUG || properties == NULL) {
        return properties;
    }

    zend_array *debug = zend_array_dup(properties);
    zend_property_info *prop;

    ZEND_HASH_MAP_FOREACH_PTR(&object->ce->properties_info, prop)
    {
        if ((prop->flags & ZEND_ACC_STATIC) != 0 || !as_property_info_is_sensitive(prop)) {
            continue;
        }

        zval mask;
        ZVAL_STRING(&mask, AS_SENSITIVE_VALUE_MASK);
        zend_hash_update(debug, prop->name, &mask);
    }
    ZEND_HASH_FOREACH_END();

    return debug;
}

static HashTable *as_base_model_get_debug_info(zend_object *object, int *is_temp)
{
    HashTable *properties = as_base_model_get_properties_for(object, ZEND_PROP_PURPOSE_DEBUG);

    if (is_temp != NULL) {
        *is_temp = 1;
    }

    return properties;
}

static zend_object *as_base_model_create_object(zend_class_entry *ce)
{
    zend_object *object = zend_objects_new(ce);

    object_properties_init(object, ce);
    object->handlers = &as_base_model_object_handlers;

    return object;
}

/* Registration function */
void as_register_BaseModel_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify", "BaseModel", class_AS_BaseModel_methods);
    AS_BaseModel_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_BaseModel_ce != NULL);
    AS_BaseModel_ce->ce_flags |= ZEND_ACC_ABSTRACT;

    memcpy(&as_base_model_object_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    as_base_model_object_handlers.get_properties_for = as_base_model_get_properties_for;
    as_base_model_object_handlers.get_debug_info = as_base_model_get_debug_info;
    AS_BaseModel_ce->create_object = as_base_model_create_object;
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
