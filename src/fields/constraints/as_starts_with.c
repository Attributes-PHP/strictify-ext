#include "as_starts_with.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"

zend_class_entry *AS_Fields_Constraints_StartsWith_ce;

/*
 * $values: string|int|float
 *
 * Enforced per element because PHP 8.2 arginfo cannot express a
 * union-typed variadic parameter.
 */
ZEND_METHOD(AS_Fields_Constraints_StartsWith, __construct)
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
                continue;
            default:
                zend_argument_type_error(i + 1, "must be of type string|int|float, %s given", zend_zval_type_name(&values[i]));
                RETURN_THROWS();
        }
    }

    zval member;
    array_init_size(&member, values_count);
    for (uint32_t i = 0; i < values_count; i++) {
        add_next_index_zval(&member, &values[i]);
        Z_TRY_ADDREF(values[i]);
    }

    zend_update_property(AS_Fields_Constraints_StartsWith_ce, Z_OBJ_P(getThis()), "values", sizeof("values") - 1, &member);
    zval_ptr_dtor(&member);
}

zend_always_inline static void add_invalid_starts_with_error_message(zval *prefixes, as_field *field, as_property_info *property, zval *errors);

zend_always_inline static void add_invalid_starts_with_error_message(zval *prefixes, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_values_to_list_string(prefixes);
    as_add_field_error_with_expected(AS_ERROR_STARTS_WITH, field, property, errors, expected);
    as_string_release(expected);
}

/*
 * #[StartsWith(...values)]: the string value must start with one of the
 * declared values (numeric values match by their string
 * representation). Non-string values fail the constraint.
 */
bool as_validate_StartsWith_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *prefixes = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "values", sizeof("values") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        add_invalid_starts_with_error_message(prefixes, field, property, errors);
        return false;
    }

    zval *prefix;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(prefixes), prefix)
    {
        zend_string *prefix_str = zval_get_string(prefix);
        const bool matches = ZSTR_LEN(prefix_str) <= Z_STRLEN_P(value) && memcmp(Z_STRVAL_P(value), ZSTR_VAL(prefix_str), ZSTR_LEN(prefix_str)) == 0;
        zend_string_release(prefix_str);

        if (matches)
            return true;
    }
    ZEND_HASH_FOREACH_END();

    add_invalid_starts_with_error_message(prefixes, field, property, errors);
    return false;
}

AS_REGISTER_CONSTRAINT_CLASS(
    StartsWith, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS, as_declare_typed_property(AS_Fields_Constraints_StartsWith_ce, "values", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
)
