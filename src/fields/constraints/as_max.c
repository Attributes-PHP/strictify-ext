#include "as_max.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Max_ce;

static zend_always_inline double zval_to_double(zval *value);
zend_always_inline static void add_invalid_max_error_message(zval *bound, as_field *field, as_property_info *property, zval *errors);

static zend_always_inline double zval_to_double(zval *value)
{
    return Z_TYPE_P(value) == IS_LONG ? (double)Z_LVAL_P(value) : Z_DVAL_P(value);
}

/*
 * #[Max(value)]: the value must be a number at most value. Non-numeric
 * values fail the constraint.
 */
bool as_validate_Max_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *bound = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "value", sizeof("value") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_LONG && Z_TYPE_P(value) != IS_DOUBLE) {
        add_invalid_max_error_message(bound, field, property, errors);
        return false;
    }

    if (zval_to_double(value) > zval_to_double(bound)) {
        add_invalid_max_error_message(bound, field, property, errors);
        return false;
    }

    return true;
}

zend_always_inline static void add_invalid_max_error_message(zval *bound, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_value_to_string(bound);
    as_add_field_error_with_expected(AS_ERROR_MAX, field, property, errors, expected);
    as_string_release(expected);
}

ZEND_METHOD(AS_Fields_Constraints_Max, __construct)
{
    zval *value;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_NUMBER(value)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(AS_Fields_Constraints_Max_ce, Z_OBJ_P(getThis()), "value", sizeof("value") - 1, value);
}

void as_register_Max_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Max", class_AS_Fields_Constraints_Max_methods);
    AS_Fields_Constraints_Max_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Max_ce != NULL);
    AS_Fields_Constraints_Max_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Max_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Max_ce, "value", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_LONG | MAY_BE_DOUBLE));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Max_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
