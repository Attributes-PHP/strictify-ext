#include "as_contains_any.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_ContainsAny_ce;

zend_always_inline static void add_invalid_contains_any_error_message(zval *required, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_values_to_list_string(required);
    as_add_field_error_with_expected(AS_ERROR_CONTAINS_ANY, field, property, errors, expected);
    as_string_release(expected);
}

/* String value: at least one declared value must appear as a substring.
 * Numeric declared values are matched by their string representation. */
zend_always_inline static bool as_contains_any_substring(zval *required, zval *value)
{
    zval *needle;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(required), needle)
    {
        zend_string *needle_str = zval_get_string(needle);
        bool found = as_memnstr(Z_STRVAL_P(value), ZSTR_VAL(needle_str), ZSTR_LEN(needle_str), Z_STRVAL_P(value) + Z_STRLEN_P(value)) != NULL;
        zend_string_release(needle_str);
        if (found) {
            return true;
        }
    }
    ZEND_HASH_FOREACH_END();

    return false;
}

/* Array value: at least one declared value must be present (strict). */
zend_always_inline static bool as_contains_any_element(zval *required, zval *value)
{
    zval *needle;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(required), needle)
    {
        zval *element;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(value), element)
        {
            if (zend_is_identical(needle, element)) {
                return true;
            }
        }
        ZEND_HASH_FOREACH_END();
    }
    ZEND_HASH_FOREACH_END();

    return false;
}

/*
 * #[ContainsAny]: see docs/constraints.md.
 */
bool as_validate_ContainsAny_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    zval *required = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "values", sizeof("values") - 1, false, NULL);

    bool satisfied = false;
    if (Z_TYPE_P(value) == IS_STRING) {
        satisfied = as_contains_any_substring(required, value);
    } else if (Z_TYPE_P(value) == IS_ARRAY) {
        satisfied = as_contains_any_element(required, value);
    }

    if (satisfied) {
        return true;
    }

    add_invalid_contains_any_error_message(required, field, property, errors);
    return false;
}

ZEND_METHOD(AS_Fields_Constraints_ContainsAny, __construct)
{
    zval *values;
    uint32_t values_count;

    ZEND_PARSE_PARAMETERS_START(1, -1)
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

    zend_update_property(AS_Fields_Constraints_ContainsAny_ce, Z_OBJ_P(getThis()), "values", sizeof("values") - 1, &member);
    zval_ptr_dtor(&member);
}

AS_REGISTER_CONSTRAINT_CLASS(
    ContainsAny, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS, as_declare_typed_property(AS_Fields_Constraints_ContainsAny_ce, "values", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
)
