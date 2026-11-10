#include "as_multiple_of.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "zend_types.h"
#include <math.h>

/* Tolerance for float multiple checks: fmod(0.3, 0.1) keeps enough
 * representation noise that a zero threshold would reject it. */
#define AS_MULTIPLE_OF_EPSILON 1e-9

zend_class_entry *AS_Fields_Constraints_MultipleOf_ce;

/*
 * #[MultipleOf(value)]: the value must be a multiple of value. Integer
 * pairs use integer modulo; mixed or float pairs use fmod with a small
 * epsilon so floating point representation noise stays within bounds.
 * Non-numeric values fail the constraint.
 */
bool as_validate_MultipleOf_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *bound = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "value", sizeof("value") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_LONG && Z_TYPE_P(value) != IS_DOUBLE) {
        add_invalid_multiple_of_error_message(bound, field, property, errors);
        return false;
    }

    if (Z_TYPE_P(value) == IS_LONG && Z_TYPE_P(bound) == IS_LONG) {
        if (Z_LVAL_P(bound) == 0 ? Z_LVAL_P(value) != 0 : Z_LVAL_P(value) % Z_LVAL_P(bound) != 0) {
            add_invalid_multiple_of_error_message(bound, field, property, errors);
            return false;
        }
        return true;
    }

    double number = Z_TYPE_P(value) == IS_LONG ? (double)Z_LVAL_P(value) : Z_DVAL_P(value);
    double divisor = Z_TYPE_P(bound) == IS_LONG ? (double)Z_LVAL_P(bound) : Z_DVAL_P(bound);

    if (divisor == 0 && number != 0) {
        add_invalid_multiple_of_error_message(bound, field, property, errors);
        return false;
    }

    double remainder = fmod(number, divisor);
    double tolerance = fabs(divisor) * AS_MULTIPLE_OF_EPSILON;
    const bool multiple = fabs(remainder) <= tolerance || fabs(fabs(remainder) - fabs(divisor)) <= tolerance;
    if (!multiple) {
        add_invalid_multiple_of_error_message(bound, field, property, errors);
        return false;
    }

    return true;
}

zend_always_inline static void add_invalid_multiple_of_error_message(zval *bound, as_field *field, as_property_info *property, zval *errors);

zend_always_inline static void add_invalid_multiple_of_error_message(zval *bound, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_value_to_string(bound);
    as_add_field_error_with_expected(AS_ERROR_MULTIPLE_OF, field, property, errors, expected);
    as_string_release(expected);
}

ZEND_METHOD(AS_Fields_Constraints_MultipleOf, __construct)
{
    zval *value;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_NUMBER(value)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(AS_Fields_Constraints_MultipleOf_ce, Z_OBJ_P(getThis()), "value", sizeof("value") - 1, value);
}

void as_register_MultipleOf_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "MultipleOf", class_AS_Fields_Constraints_MultipleOf_methods);
    AS_Fields_Constraints_MultipleOf_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_MultipleOf_ce != NULL);
    AS_Fields_Constraints_MultipleOf_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_MultipleOf_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_MultipleOf_ce, "value", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_LONG | MAY_BE_DOUBLE));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_MultipleOf_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
