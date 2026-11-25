#include "as_isbn.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"


zend_class_entry *AS_Fields_Constraints_Isbn_ce;

/*
 * #[Isbn]: see docs/constraints.md.
 */
bool as_validate_Isbn_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_ISBN, field, property, errors);
        return false;
    }
    /* strip dashes and spaces */
    char buf[16];
    size_t n = 0;
    const char *s = Z_STRVAL_P(value);
    size_t len = Z_STRLEN_P(value);
    for (size_t i = 0; i < len && n < sizeof(buf); i++) {
        if (s[i] != '-' && s[i] != ' ') {
            buf[n++] = s[i];
        }
    }
    if (n == 10) {
        int sum = 0;
        for (size_t i = 0; i < 10; i++) {
            int d;
            if (buf[i] >= '0' && buf[i] <= '9') {
                d = buf[i] - '0';
            } else if (buf[i] == 'X' || buf[i] == 'x') {
                if (i != 9) { as_add_field_error_with_prefix(AS_ERROR_ISBN, field, property, errors); return false; }
                d = 10;
            } else {
                as_add_field_error_with_prefix(AS_ERROR_ISBN, field, property, errors);
                return false;
            }
            sum += (int)(i + 1) * d;
        }
        if (sum % 11 == 0) {
            return true;
        }
    } else if (n == 13) {
        int sum = 0;
        for (size_t i = 0; i < 13; i++) {
            if (buf[i] < '0' || buf[i] > '9') {
                as_add_field_error_with_prefix(AS_ERROR_ISBN, field, property, errors);
                return false;
            }
            sum += (buf[i] - '0') * (i % 2 == 0 ? 1 : 3);
        }
        if (sum % 10 == 0) {
            return true;
        }
    }
    as_add_field_error_with_prefix(AS_ERROR_ISBN, field, property, errors);
    return false;
}


void as_register_Isbn_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Isbn", class_AS_Fields_Constraints_Isbn_methods);
    AS_Fields_Constraints_Isbn_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Isbn_ce != NULL);
    AS_Fields_Constraints_Isbn_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Isbn_ce, 1, AS_Fields_Field_ce);


    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Isbn_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
