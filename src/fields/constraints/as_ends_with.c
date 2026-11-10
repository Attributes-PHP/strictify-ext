#include "as_ends_with.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"

zend_class_entry *AS_Fields_Constraints_EndsWith_ce;

/*
 * $values: string|int|float
 *
 * Enforced per element because PHP 8.2 arginfo cannot express a
 * union-typed variadic parameter.
 */
ZEND_METHOD(AS_Fields_Constraints_EndsWith, __construct)
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

    zend_update_property(AS_Fields_Constraints_EndsWith_ce, Z_OBJ_P(getThis()), "values", sizeof("values") - 1, &member);
    zval_ptr_dtor(&member);
}

/*
 * #[EndsWith(...values)]: the string value must end with one of the
 * declared values (numeric values match by their string
 * representation). Non-string values fail the constraint.
 */
bool as_validate_EndsWith_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *suffixes = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "values", sizeof("values") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        add_invalid_ends_with_error_message(suffixes, field, property, errors);
        return false;
    }

    zval *suffix;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(suffixes), suffix)
    {
        zend_string *suffix_str = zval_get_string(suffix);
        const size_t suffix_len = ZSTR_LEN(suffix_str);
        const bool matches = suffix_len <= Z_STRLEN_P(value) && memcmp(Z_STRVAL_P(value) + (Z_STRLEN_P(value) - suffix_len), ZSTR_VAL(suffix_str), suffix_len) == 0;
        zend_string_release(suffix_str);
        if (matches)
            return true;
    }
    ZEND_HASH_FOREACH_END();

    add_invalid_ends_with_error_message(suffixes, field, property, errors);
    return false;
}

zend_always_inline static void add_invalid_ends_with_error_message(zval *suffixes, as_field *field, as_property_info *property, zval *errors);

zend_always_inline static void add_invalid_ends_with_error_message(zval *suffixes, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_values_to_list_string(suffixes);
    as_add_field_error_with_expected(AS_ERROR_ENDS_WITH, field, property, errors, expected);
    as_string_release(expected);
}

void as_register_EndsWith_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "EndsWith", class_AS_Fields_Constraints_EndsWith_methods);
    AS_Fields_Constraints_EndsWith_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_EndsWith_ce != NULL);
    AS_Fields_Constraints_EndsWith_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_EndsWith_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_EndsWith_ce, "values", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_EndsWith_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
