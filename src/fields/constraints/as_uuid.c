#include "as_uuid.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Uuid_ce;

static zend_always_inline bool is_hex_digit(char c);

static zend_always_inline bool is_hex_digit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/*
 * #[Uuid]: the value must be a canonical UUID string: 36 characters
 * laid out as 8-4-4-4-12 hexadecimal groups separated by dashes, both
 * letter cases accepted (for example
 * 123e4567-e89b-12d3-a456-426614174000). Non-string values fail the
 * constraint.
 */
bool as_validate_Uuid_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_UUID, field, property, errors);
        return false;
    }

    if (Z_STRLEN_P(value) != 36) {
        as_add_field_error_with_prefix(AS_ERROR_UUID, field, property, errors);
        return false;
    }

    const char *str = Z_STRVAL_P(value);
    for (size_t i = 0; i < 36; i++) {
        const char c = str[i];

        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') {
                as_add_field_error_with_prefix(AS_ERROR_UUID, field, property, errors);
                return false;
            }
            continue;
        }

        if (!is_hex_digit(c)) {
            as_add_field_error_with_prefix(AS_ERROR_UUID, field, property, errors);
            return false;
        }
    }

    return true;
}

void as_register_Uuid_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Uuid", class_AS_Fields_Constraints_Uuid_methods);
    AS_Fields_Constraints_Uuid_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Uuid_ce != NULL);
    AS_Fields_Constraints_Uuid_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Uuid_ce, 1, AS_Fields_Field_ce);

    zend_internal_attribute_register(AS_Fields_Constraints_Uuid_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
