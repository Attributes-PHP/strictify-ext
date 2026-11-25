#include "as_iban.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Iban_ce;

/*
 * #[Iban]: see docs/constraints.md.
 */
bool as_validate_Iban_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_IBAN, field, property, errors);
        return false;
    }

    /* IBANs are case-insensitive in practice (printed and pasted in either
     * case): the checksum is computed on the uppercased form. */
    char uppercased[64];
    const size_t length = Z_STRLEN_P(value);
    if (length >= sizeof(uppercased)) {
        as_add_field_error_with_prefix(AS_ERROR_IBAN, field, property, errors);
        return false;
    }

    const char *source = Z_STRVAL_P(value);
    for (size_t i = 0; i < length; i++) {
        char c = source[i];
        uppercased[i] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
    }

    zend_string *normalized = zend_string_init(uppercased, length, 0);
    static zend_string *as_pattern = NULL;
    if (!as_pattern_match(&as_pattern, "~^[A-Z]{2}[0-9]{2}[A-Z0-9]{11,30}$~", normalized)) {
        zend_string_release(normalized);
        as_add_field_error_with_prefix(AS_ERROR_IBAN, field, property, errors);
        return false;
    }
    /* mod-97 checksum over the rearranged string (first 4 chars moved to the end) */
    const char *s = ZSTR_VAL(normalized);
    size_t len = ZSTR_LEN(normalized);
    int remainder = 0;
    for (size_t i = 4; i < len; i++) {
        char c = s[i];
        if (c >= '0' && c <= '9') {
            remainder = (remainder * 10 + (c - '0')) % 97;
        } else {
            int n = c - 'A' + 10;
            remainder = (remainder * 10 + n / 10) % 97;
            remainder = (remainder * 10 + n % 10) % 97;
        }
    }
    for (size_t i = 0; i < 4; i++) {
        char c = s[i];
        if (c >= '0' && c <= '9') {
            remainder = (remainder * 10 + (c - '0')) % 97;
        } else {
            int n = c - 'A' + 10;
            remainder = (remainder * 10 + n / 10) % 97;
            remainder = (remainder * 10 + n % 10) % 97;
        }
    }
    zend_string_release(normalized);

    if (remainder != 1) {
        as_add_field_error_with_prefix(AS_ERROR_IBAN, field, property, errors);
        return false;
    }
    return true;
}

AS_REGISTER_CONSTRAINT_CLASS(Iban, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS)
