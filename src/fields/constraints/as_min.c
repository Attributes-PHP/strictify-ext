#include "as_min.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Min_ce;

static zend_always_inline double zval_to_double(zval *value);
zend_always_inline static void add_invalid_min_error_message(zval *bound, as_field *field, as_property_info *property, zval *errors);

static zend_always_inline double zval_to_double(zval *value)
{
    return Z_TYPE_P(value) == IS_LONG ? (double)Z_LVAL_P(value) : Z_DVAL_P(value);
}

/*
 * #[Min(value)]: the value must be a number at least value. Non-numeric
 * values fail the constraint.
 */
bool as_validate_Min_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *bound = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "value", sizeof("value") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_LONG && Z_TYPE_P(value) != IS_DOUBLE) {
        add_invalid_min_error_message(bound, field, property, errors);
        return false;
    }

    if (zval_to_double(value) < zval_to_double(bound)) {
        add_invalid_min_error_message(bound, field, property, errors);
        return false;
    }

    return true;
}

zend_always_inline static void add_invalid_min_error_message(zval *bound, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_value_to_string(bound);
    as_add_field_error_with_expected(AS_ERROR_MIN, field, property, errors, expected);
    as_string_release(expected);
}

ZEND_METHOD(AS_Fields_Constraints_Min, __construct)
{
    zval *value;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_NUMBER(value)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(AS_Fields_Constraints_Min_ce, Z_OBJ_P(getThis()), "value", sizeof("value") - 1, value);
}

AS_REGISTER_CONSTRAINT_CLASS(
    Min, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS, as_declare_typed_property(AS_Fields_Constraints_Min_ce, "value", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_LONG | MAY_BE_DOUBLE));
)
