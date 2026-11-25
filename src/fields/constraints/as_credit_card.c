#include "as_credit_card.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"


zend_class_entry *AS_Fields_Constraints_CreditCard_ce;

/*
 * #[CreditCard]: see docs/constraints.md.
 */
bool as_validate_CreditCard_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_CREDIT_CARD, field, property, errors);
        return false;
    }
    char buf[24];
    size_t n = 0;
    const char *s = Z_STRVAL_P(value);
    size_t len = Z_STRLEN_P(value);
    for (size_t i = 0; i < len && n < sizeof(buf); i++) {
        if (s[i] == ' ' || s[i] == '-') {
            continue;
        }
        if (s[i] < '0' || s[i] > '9') {
            as_add_field_error_with_prefix(AS_ERROR_CREDIT_CARD, field, property, errors);
            return false;
        }
        buf[n++] = s[i];
    }
    if (n < 12 || n > 19) {
        as_add_field_error_with_prefix(AS_ERROR_CREDIT_CARD, field, property, errors);
        return false;
    }
    /* Luhn */
    int sum = 0;
    bool alt = false;
    for (size_t i = n; i-- > 0;) {
        int d = buf[i] - '0';
        if (alt) {
            d *= 2;
            if (d > 9) d -= 9;
        }
        sum += d;
        alt = !alt;
    }
    if (sum % 10 != 0) {
        as_add_field_error_with_prefix(AS_ERROR_CREDIT_CARD, field, property, errors);
        return false;
    }
    return true;
}


void as_register_CreditCard_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "CreditCard", class_AS_Fields_Constraints_CreditCard_methods);
    AS_Fields_Constraints_CreditCard_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_CreditCard_ce != NULL);
    AS_Fields_Constraints_CreditCard_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_CreditCard_ce, 1, AS_Fields_Field_ce);


    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_CreditCard_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
