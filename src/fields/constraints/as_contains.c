#include "as_contains.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"

zend_class_entry *AS_Fields_Constraints_Contains_ce;

/*
 * $values: string|int|float
 *
 * Enforced per element because PHP 8.2 arginfo cannot express a
 * union-typed variadic parameter. At least one value is required.
 */
ZEND_METHOD(AS_Fields_Constraints_Contains, __construct)
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

    zend_update_property(AS_Fields_Constraints_Contains_ce, Z_OBJ_P(getThis()), "values", sizeof("values") - 1, &member);
    zval_ptr_dtor(&member);
}

/*
 * #[Contains(...values)]: every declared value must be present. Array
 * values must contain each declared value (strict comparison); string
 * values must contain each declared value as a substring (numeric
 * values are matched by their string representation). Other value
 * types fail the constraint.
 */
bool as_validate_Contains_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *required = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "values", sizeof("values") - 1, false, NULL);

    zval *value = field->value;

    if (Z_TYPE_P(value) != IS_ARRAY && Z_TYPE_P(value) != IS_STRING) {
        add_invalid_contains_error_message(required, field, property, errors);
        return false;
    }

    if (Z_TYPE_P(value) == IS_STRING) {
        zval *needle;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(required), needle)
        {
            zend_string *needle_str = zval_get_string(needle);
            bool found = as_memnstr(Z_STRVAL_P(value), ZSTR_VAL(needle_str), ZSTR_LEN(needle_str), Z_STRVAL_P(value) + Z_STRLEN_P(value)) != NULL;
            zend_string_release(needle_str);

            if (!found) {
                add_invalid_contains_error_message(required, field, property, errors);
                return false;
            }
        }
        ZEND_HASH_FOREACH_END();
        return true;
    }

    // Needs to contain all array values
    zval *needle;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(required), needle)
    {
        bool found = false;
        zval *element;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(value), element)
        {
            if (zend_is_identical(needle, element)) {
                found = true;
                break;
            }
        }
        ZEND_HASH_FOREACH_END();

        if (!found) {
            add_invalid_contains_error_message(required, field, property, errors);
            return false;
        }
    }
    ZEND_HASH_FOREACH_END();
    return true;
}

zend_always_inline static void add_invalid_contains_error_message(zval *required, as_field *field, as_property_info *property, zval *errors);

zend_always_inline static void add_invalid_contains_error_message(zval *required, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_values_to_list_string(required);
    as_add_field_error_with_expected(AS_ERROR_CONTAINS, field, property, errors, expected);
    as_string_release(expected);
}

void as_register_Contains_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Contains", class_AS_Fields_Constraints_Contains_methods);
    AS_Fields_Constraints_Contains_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Contains_ce != NULL);
    AS_Fields_Constraints_Contains_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Contains_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Contains_ce, "values", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Contains_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
