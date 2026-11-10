#include "as_error_message.h"
#include "as_field.h"
#include <string.h>
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "../helpers/as_error_messages.h"

zend_class_entry *AS_Fields_ErrorMessage_ce;

ZEND_METHOD(AS_Fields_ErrorMessage, __construct)
{
    zend_string *required, *type;

    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_STR(required)
    Z_PARAM_STR(type)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_string(AS_Fields_ErrorMessage_ce, Z_OBJ_P(getThis()), "required", sizeof("required") - 1, ZSTR_VAL(required));
    zend_update_property_string(AS_Fields_ErrorMessage_ce, Z_OBJ_P(getThis()), "type", sizeof("type") - 1, ZSTR_VAL(type));
}

void as_register_ErrorMessage_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "ErrorMessage", class_AS_Fields_ErrorMessage_methods);
    AS_Fields_ErrorMessage_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_ErrorMessage_ce != NULL);
    AS_Fields_ErrorMessage_ce->ce_flags |= ZEND_ACC_READONLY_CLASS;
    zend_class_implements(AS_Fields_ErrorMessage_ce, 1, AS_Fields_Field_ce);

    /* Register $required and $type properties */
    zend_declare_property_stringl(
        AS_Fields_ErrorMessage_ce, "required", sizeof("required") - 1, as_default_error_type_messages[AS_ERROR_REQUIRED], strlen(as_default_error_type_messages[AS_ERROR_REQUIRED]), ZEND_ACC_PUBLIC | ZEND_ACC_READONLY
    );
    zend_declare_property_stringl(
        AS_Fields_ErrorMessage_ce, "type", sizeof("type") - 1, as_default_error_type_messages[AS_ERROR_TYPE], strlen(as_default_error_type_messages[AS_ERROR_TYPE]), ZEND_ACC_PUBLIC | ZEND_ACC_READONLY
    );

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_ErrorMessage_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}