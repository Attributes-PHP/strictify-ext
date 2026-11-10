#include "as_constraint.h"
#include "as_config_aware_constraint.h"
#include "as_field.h"
#include "../as_model_configs.h"
#include "../helpers/as_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"

zend_class_entry *AS_Fields_Constraint_ce;

void as_register_Constraint_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "Constraint", class_AS_Fields_Constraint_methods);
    AS_Fields_Constraint_ce = zend_register_internal_interface(&ce);
    ZEND_ASSERT(AS_Fields_Constraint_ce != NULL);

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraint_ce, 1, AS_Fields_Field_ce);
}

static void as_set_model_configs_if_aware(zval *constraint, as_property_info *property);
static void add_invalid_custom_error_message(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/*
 * ConfigAwareConstraint constraints receive the owning model's resolved configs
 * before validate() runs (and thus before message()). Exceptions thrown
 * by setModelConfigs() propagate to the caller of validate() like model
 * hook errors.
 */
static void as_set_model_configs_if_aware(zval *constraint, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(constraint);
    if (!instanceof_function(ce, AS_Fields_ConfigAwareConstraint_ce)) {
        return;
    }

    zval configs;
    as_model_configs_properties properties;
    as_get_model_configs(&configs, property->model, &properties, true);
    if (UNEXPECTED(EG(exception) != NULL)) {
        return;
    }

    zend_function *set_model_configs = zend_hash_str_find_ptr(&ce->function_table, "setmodelconfigs", sizeof("setmodelconfigs") - 1);
    ZEND_ASSERT(set_model_configs != NULL);

    zend_call_known_instance_method_with_1_params(set_model_configs, Z_OBJ_P(constraint), NULL, &configs);
    zval_ptr_dtor(&configs);
}

static void add_invalid_custom_error_message(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zend_class_entry *ce = Z_OBJCE_P(constraint);
    zend_function *message = zend_hash_str_find_ptr(&ce->function_table, "message", sizeof("message") - 1);
    ZEND_ASSERT(message != NULL);

    zval message_value;
    ZVAL_UNDEF(&message_value);
    zend_call_known_instance_method_with_0_params(message, Z_OBJ_P(constraint), &message_value);
    if (EG(exception) != NULL) {
        if (Z_TYPE(message_value) != IS_UNDEF) {
            zval_ptr_dtor(&message_value);
        }
        return;
    }

    ZEND_ASSERT(Z_TYPE(message_value) == IS_STRING);
    zend_string *rendered = as_replace_placeholders(Z_STRVAL(message_value), Z_STRLEN(message_value), field, property);
    zval_ptr_dtor(&message_value);

    as_add_field_message(rendered, field, errors);
    zend_string_release(rendered);
}

/*
 * Userland constraint validation: the interface contract guarantees the
 * validate() and message() methods, with the engine enforcing their
 * signatures since PHP classes cannot implement an interface without
 * them. Exceptions thrown by either method propagate to the caller of
 * validate() like model hook errors.
 */
bool as_validate_userland_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zend_class_entry *ce = Z_OBJCE_P(constraint);

    as_set_model_configs_if_aware(constraint, property);
    if (UNEXPECTED(EG(exception) != NULL)) {
        return false;
    }

    zend_function *validate = zend_hash_str_find_ptr(&ce->function_table, "validate", sizeof("validate") - 1);
    ZEND_ASSERT(validate != NULL);

    zval result;
    ZVAL_UNDEF(&result);
    zend_call_known_instance_method_with_1_params(validate, Z_OBJ_P(constraint), &result, field->value);

    const bool valid = Z_TYPE(result) == IS_TRUE;
    if (Z_TYPE(result) != IS_UNDEF) {
        zval_ptr_dtor(&result);
    }

    if (EG(exception) != NULL) {
        return false;
    }

    if (!valid) {
        add_invalid_custom_error_message(constraint, field, property, errors);
        return false;
    }

    return true;
}
