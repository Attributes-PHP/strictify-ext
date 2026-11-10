#include "as_size.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Size_ce;

typedef struct {
    zval *min;
    zval *max;
    bool has_min;
    bool has_max;
} min_max_constraints;

/*
 * #[Size(min: ?, max: ?)]: the size of the value must respect the
 * declared bounds (inclusive). Strings are measured in bytes, arrays
 * in element count. With only one bound set, only that bound applies;
 * with neither set the constraint always passes. Other value types
 * fail the constraint.
 */
ZEND_METHOD(AS_Fields_Constraints_Size, __construct)
{
    zend_long min = 0;
    zend_long max = 0;
    bool min_null = true;
    bool max_null = true;

    ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG_OR_NULL(min, min_null)
    Z_PARAM_LONG_OR_NULL(max, max_null)
    ZEND_PARSE_PARAMETERS_END();

    if (min_null) {
        zend_update_property_null(AS_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "min", sizeof("min") - 1);
    } else {
        zend_update_property_long(AS_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "min", sizeof("min") - 1, min);
    }

    if (max_null) {
        zend_update_property_null(AS_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "max", sizeof("max") - 1);
    } else {
        zend_update_property_long(AS_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "max", sizeof("max") - 1, max);
    }
}

static void add_invalid_size_error_message(min_max_constraints size_constraints, as_field *field, as_property_info *property, zval *errors);

static void add_invalid_size_error_message(min_max_constraints size_constraints, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected;

    if (size_constraints.has_min && size_constraints.has_max) {
        zend_string *min_str = as_value_to_string(size_constraints.min);
        zend_string *max_str = as_value_to_string(size_constraints.max);
        zend_string *part = as_string_concat3(ZSTR_VAL(min_str), ZSTR_LEN(min_str), " and ", sizeof(" and ") - 1, ZSTR_VAL(max_str), ZSTR_LEN(max_str));
        expected = as_string_concat3("between ", sizeof("between ") - 1, ZSTR_VAL(part), ZSTR_LEN(part), "", 0);
        as_string_release(part);
        as_string_release(min_str);
        as_string_release(max_str);
    } else if (size_constraints.has_min) {
        zend_string *min_str = as_value_to_string(size_constraints.min);
        expected = as_string_concat3("of at least ", sizeof("of at least ") - 1, ZSTR_VAL(min_str), ZSTR_LEN(min_str), "", 0);
        as_string_release(min_str);
    } else {
        zend_string *max_str = as_value_to_string(size_constraints.max);
        expected = as_string_concat3("of at most ", sizeof("of at most ") - 1, ZSTR_VAL(max_str), ZSTR_LEN(max_str), "", 0);
        as_string_release(max_str);
    }

    as_add_field_error_with_expected(AS_ERROR_SIZE, field, property, errors, expected);
    as_string_release(expected);
}

bool as_validate_Size_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    min_max_constraints size_constraints;
    size_constraints.min = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "min", sizeof("min") - 1, false, NULL);
    size_constraints.max = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "max", sizeof("max") - 1, false, NULL);
    size_constraints.has_min = Z_TYPE_P(size_constraints.min) != IS_NULL;
    size_constraints.has_max = Z_TYPE_P(size_constraints.max) != IS_NULL;

    if (!size_constraints.has_min && !size_constraints.has_max) {
        return true;
    }

    zval *value = field->value;
    zend_long size;
    if (Z_TYPE_P(value) == IS_STRING) {
        size = (zend_long)Z_STRLEN_P(value);
    } else if (Z_TYPE_P(value) == IS_ARRAY) {
        size = (zend_long)zend_hash_num_elements(Z_ARRVAL_P(value));
    } else {
        add_invalid_size_error_message(size_constraints, field, property, errors);
        return false;
    }

    if (size_constraints.has_min && size < Z_LVAL_P(size_constraints.min)) {
        add_invalid_size_error_message(size_constraints, field, property, errors);
        return false;
    }

    if (size_constraints.has_max && size > Z_LVAL_P(size_constraints.max)) {
        add_invalid_size_error_message(size_constraints, field, property, errors);
        return false;
    }

    return true;
}

void as_register_Size_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Size", class_AS_Fields_Constraints_Size_methods);
    AS_Fields_Constraints_Size_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Size_ce != NULL);
    AS_Fields_Constraints_Size_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Size_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Size_ce, "min", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_LONG | MAY_BE_NULL));
    as_declare_typed_property(AS_Fields_Constraints_Size_ce, "max", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_LONG | MAY_BE_NULL));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Size_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
