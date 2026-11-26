#include "as_ulid.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Ulid_ce;

static zend_always_inline bool is_crockford_base32(char c);

static zend_always_inline bool is_crockford_base32(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/*
 * #[Ulid]: the value must be a valid ULID string: exactly 26 characters
 * from the Crockford base32 alphabet (0-9 A-Z minus I, L, O, U; both
 * letter cases accepted). Non-string values fail the constraint.
 */
bool as_validate_Ulid_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_ULID, field, property, errors);
        return false;
    }

    if (Z_STRLEN_P(value) != 26) {
        as_add_field_error_with_prefix(AS_ERROR_ULID, field, property, errors);
        return false;
    }

    const char *str = Z_STRVAL_P(value);
    for (size_t i = 0; i < 26; i++) {
        const char c = str[i];
        if (c == 'i' || c == 'l' || c == 'o' || c == 'u' || c == 'I' || c == 'L' || c == 'O' || c == 'U') {
            as_add_field_error_with_prefix(AS_ERROR_ULID, field, property, errors);
            return false;
        }

        if (!is_crockford_base32(c)) {
            as_add_field_error_with_prefix(AS_ERROR_ULID, field, property, errors);
            return false;
        }

        /* A ULID encodes 128 bits in 26 base32 characters: the first
         * character can only be 0-7, otherwise the value overflows. */
        if (i == 0 && c > '7') {
            as_add_field_error_with_prefix(AS_ERROR_ULID, field, property, errors);
            return false;
        }
    }

    return true;
}

AS_REGISTER_CONSTRAINT_CLASS(Ulid, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS)
