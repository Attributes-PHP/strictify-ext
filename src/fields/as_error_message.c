#include "as_error_message.h"
#include "as_field.h"
#include <string.h>
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "../helpers/as_error_messages.h"

zend_class_entry *AS_Fields_ErrorMessage_ce;

/* The key list must cover every catalog key except the discarded extra one */
typedef char as_error_message_key_count_check[(AS_ERROR_MESSAGE_KEY_COUNT == AS_ERROR_TYPE_COUNT - 1) ? 1 : -1];

/*
 * One constructor argument per catalog key: parsed when given, otherwise
 * the property keeps its declared default template.
 */
#define AS_ERROR_MESSAGE_DECLARE_ARG(c_name, error_type, str_name) zend_string *c_name = NULL;
#define AS_ERROR_MESSAGE_PARSE_ARG(c_name, error_type, str_name)   Z_PARAM_STR_OR_NULL(c_name)
#define AS_ERROR_MESSAGE_UPDATE_ARG(c_name, error_type, str_name)                                                                     \
    if (c_name != NULL) {                                                                                                             \
        zend_update_property_string(AS_Fields_ErrorMessage_ce, Z_OBJ_P(getThis()), str_name, sizeof(str_name) - 1, ZSTR_VAL(c_name)); \
    }

ZEND_METHOD(AS_Fields_ErrorMessage, __construct)
{
    AS_ERROR_MESSAGE_KEYS(AS_ERROR_MESSAGE_DECLARE_ARG)

    ZEND_PARSE_PARAMETERS_START(0, AS_ERROR_MESSAGE_KEY_COUNT)
    Z_PARAM_OPTIONAL
    AS_ERROR_MESSAGE_KEYS(AS_ERROR_MESSAGE_PARSE_ARG)
    ZEND_PARSE_PARAMETERS_END();

    AS_ERROR_MESSAGE_KEYS(AS_ERROR_MESSAGE_UPDATE_ARG)
}

/* Each key is a public readonly string property defaulting to the catalog's
 * default template for that error type */
#define AS_ERROR_MESSAGE_DECLARE_PROPERTY(c_name, error_type, str_name)                                                                                                                                \
    zend_declare_property_stringl(                                                                                                                                                                     \
        AS_Fields_ErrorMessage_ce, str_name, sizeof(str_name) - 1, as_default_error_type_messages[error_type], strlen(as_default_error_type_messages[error_type]), ZEND_ACC_PUBLIC | ZEND_ACC_READONLY \
    );

void as_register_ErrorMessage_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "ErrorMessage", class_AS_Fields_ErrorMessage_methods);
    AS_Fields_ErrorMessage_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_ErrorMessage_ce != NULL);
    AS_Fields_ErrorMessage_ce->ce_flags |= ZEND_ACC_READONLY_CLASS;
    zend_class_implements(AS_Fields_ErrorMessage_ce, 1, AS_Fields_Field_ce);

    AS_ERROR_MESSAGE_KEYS(AS_ERROR_MESSAGE_DECLARE_PROPERTY)

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_ErrorMessage_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
