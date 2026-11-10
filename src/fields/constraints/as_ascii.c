#include "as_ascii.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Ascii_ce;

/*
 * #[Ascii]: the value must only contain 7-bit ASCII characters
 * (bytes 0x00-0x7F). Non-string values fail the constraint.
 */
bool as_validate_Ascii_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_ASCII_TYPE, field, property, errors);
        return false;
    }

    const unsigned char *str = (const unsigned char *)Z_STRVAL_P(value);
    size_t len = Z_STRLEN_P(value);

    for (size_t i = 0; i < len; i++) {
        if (str[i] <= 0x7F) {
            continue;
        }

        as_add_field_error_with_prefix(AS_ERROR_ASCII, field, property, errors);
        return false;
    }

    return true;
}

void as_register_Ascii_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Ascii", class_AS_Fields_Constraints_Ascii_methods);
    AS_Fields_Constraints_Ascii_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Ascii_ce != NULL);
    AS_Fields_Constraints_Ascii_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Ascii_ce, 1, AS_Fields_Field_ce);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Ascii_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
