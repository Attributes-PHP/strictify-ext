#include "as_in.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"

zend_class_entry *AS_Fields_Constraints_In_ce;

/*
 * $values: string|int|float|bool
 *
 * Enforced per element because PHP 8.2 arginfo cannot express a
 * union-typed variadic parameter.
 */
ZEND_METHOD(AS_Fields_Constraints_In, __construct)
{
    zval *values;
    uint32_t values_count;

    ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('z', values, values_count)
    ZEND_PARSE_PARAMETERS_END();

    for (uint32_t i = 0; i < values_count; i++) {
        switch (Z_TYPE(values[i])) {
            case IS_STRING:
            case IS_LONG:
            case IS_DOUBLE:
            case IS_TRUE:
            case IS_FALSE:
                continue;
            default:
                zend_argument_type_error(i + 1, "must be of type string|int|float|bool, %s given", zend_zval_type_name(&values[i]));
                RETURN_THROWS();
        }
    }

    zval member;
    array_init_size(&member, values_count);
    for (uint32_t i = 0; i < values_count; i++) {
        add_next_index_zval(&member, &values[i]);
        Z_TRY_ADDREF(values[i]);
    }

    zend_update_property(AS_Fields_Constraints_In_ce, Z_OBJ_P(getThis()), "values", sizeof("values") - 1, &member);
    zval_ptr_dtor(&member);
}

/*
 * #[In(...values)]: the value must be strictly identical to one of the
 * declared values.
 */
bool as_validate_In_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *allowed = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "values", sizeof("values") - 1, false, NULL);

    zval *value = field->value;
    zval *entry;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(allowed), entry)
    {
        if (zend_is_identical(entry, value)) {
            return true;
        }
    }
    ZEND_HASH_FOREACH_END();

    zend_string *expected = as_values_to_list_string(allowed);
    as_add_field_error_with_expected(AS_ERROR_IN, field, property, errors, expected);
    as_string_release(expected);
    return false;
}

AS_REGISTER_CONSTRAINT_CLASS(
    In, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS, as_declare_typed_property(AS_Fields_Constraints_In_ce, "values", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
)
