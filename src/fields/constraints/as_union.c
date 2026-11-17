#include "as_union.h"
#include "../as_field.h"
#include "../../as_type.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_enum.h"
#include "Zend/zend_exceptions.h"
#include <string.h>

zend_class_entry *AS_Fields_Constraints_Union_ce;

/*
 * #[Union(...$of)] on a scalar property (e.g. public int|float $example):
 * the value must match at least one declared arm. Arms are the Type
 * constants (int masks), Type enum cases or class name strings — the same
 * vocabulary as the Fields\\Union array attribute, minus the nested array
 * shapes, which describe array elements, not scalar values.
 *
 * The constraint runs after the native type check, so on an int|float
 * property the value is already int or float: the constraint narrows which
 * arms are acceptable (#[Union(Type::int)] rejects floats).
 */

static const struct {
    uint32_t mask;
    const char *name;
    size_t length;
} union_basic_type_names[] = {
    {MAY_BE_BOOL, "boolean", sizeof("boolean") - 1}, {MAY_BE_LONG, "integer", sizeof("integer") - 1}, {MAY_BE_DOUBLE, "float", sizeof("float") - 1}, {MAY_BE_STRING, "string", sizeof("string") - 1},
    {MAY_BE_ARRAY, "array", sizeof("array") - 1},    {MAY_BE_OBJECT, "object", sizeof("object") - 1}, {MAY_BE_NULL, "null", sizeof("null") - 1},
};

#define UNION_BASIC_TYPE_NAMES_SIZE (sizeof(union_basic_type_names) / sizeof(union_basic_type_names[0]))

static bool is_valid_mask_arm(zend_long type_value, uint32_t arg_num);
static bool is_resolvable_class_name(zval *value, uint32_t arg_num);
static uint32_t normalize_mask(zend_long type_value);
static bool arm_matches_value(zval *arm, zval *value);
static zend_string *union_arms_to_expected_string(zval *arms);

/*
 * A mask arm must be a positive engine type mask (the private dict bit is
 * accepted and folded into MAY_BE_ARRAY, like the array validator does).
 */
static bool is_valid_mask_arm(zend_long type_value, uint32_t arg_num)
{
    if (type_value <= 0 || ((uint32_t)type_value & ~AS_TYPE_ALL_BITS) != 0) {
        zend_argument_value_error(arg_num, "must be a valid type value, " ZEND_LONG_FMT " given", type_value);
        return false;
    }

    return true;
}

/*
 * A string arm must name a loadable class, interface or enum: a missing
 * name autoloads, and a name that still cannot be resolved throws a
 * ValueError (an autoloader error propagates as-is).
 */
static bool is_resolvable_class_name(zval *value, uint32_t arg_num)
{
    if (zend_lookup_class(Z_STR_P(value)) != NULL) {
        return true;
    }

    if (EG(exception) == NULL) {
        zend_argument_value_error(arg_num, "must be a valid class, interface or enum, \"%s\" given", ZSTR_VAL(Z_STR_P(value)));
    }

    return false;
}

static uint32_t normalize_mask(zend_long type_value)
{
    uint32_t mask = (uint32_t)type_value;
    if ((mask & (uint32_t)AS_TYPE_DICT) != 0) {
        mask = (mask & ~(uint32_t)AS_TYPE_DICT) | MAY_BE_ARRAY;
    }
    return mask;
}

/*
 * Exact type match only: the constraint runs after the property's native
 * type check, so the value is never coerced here. Bool arms match both
 * IS_TRUE and IS_FALSE through MAY_BE_BOOL.
 */
static bool arm_matches_value(zval *arm, zval *value)
{
    switch (Z_TYPE_P(arm)) {
        case IS_LONG:
            return (normalize_mask(Z_LVAL_P(arm)) & (1u << Z_TYPE_P(value))) != 0;

        case IS_OBJECT: {
            zval *case_value = zend_enum_fetch_case_value(Z_OBJ_P(arm));
            if (Z_TYPE_P(case_value) != IS_LONG) {
                ZEND_ASSERT(false);
                return false;
            }
            return (normalize_mask(Z_LVAL_P(case_value)) & (1u << Z_TYPE_P(value))) != 0;
        }

        case IS_STRING: {
            if (Z_TYPE_P(value) != IS_OBJECT) {
                return false;
            }
            zend_class_entry *ce = zend_lookup_class(Z_STR_P(arm));
            if (ce == NULL) {
                if (EG(exception) == NULL) {
                    zend_throw_exception_ex(zend_ce_value_error, 0, "Unsupported class name \"%s\" in a Union constraint.", ZSTR_VAL(Z_STR_P(arm)));
                }
                return false;
            }
            return instanceof_function(Z_OBJCE_P(value), ce);
        }

        default:
            return false;
    }
}

/*
 * Renders the arm list like a union type: mask arms expand to their basic
 * type names, class arms render as written, parts joined with ", " and a
 * trailing " or ".
 */
static zend_string *union_arms_to_expected_string(zval *arms)
{
    uint32_t count = zend_hash_num_elements(Z_ARRVAL_P(arms));
    if (count == 0) {
        return zend_string_init("mixed", sizeof("mixed") - 1, 0);
    }

    zend_string *result = NULL;
    uint32_t index = 0;

    zval *arm;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arms), arm)
    {
        zend_string *part = NULL;

        if (Z_TYPE_P(arm) == IS_STRING) {
            part = zend_string_copy(Z_STR_P(arm));
        } else {
            zend_long mask_value = Z_TYPE_P(arm) == IS_OBJECT ? Z_LVAL_P(zend_enum_fetch_case_value(Z_OBJ_P(arm))) : Z_LVAL_P(arm);
            uint32_t mask = normalize_mask(mask_value);

            zend_string *bit_result = NULL;
            for (size_t i = 0; i < UNION_BASIC_TYPE_NAMES_SIZE; i++) {
                if ((mask & union_basic_type_names[i].mask) != union_basic_type_names[i].mask) {
                    continue;
                }
                if (bit_result == NULL) {
                    bit_result = zend_string_init(union_basic_type_names[i].name, union_basic_type_names[i].length, 0);
                } else {
                    zend_string *joined = as_string_concat3(ZSTR_VAL(bit_result), ZSTR_LEN(bit_result), ", ", 2, union_basic_type_names[i].name, union_basic_type_names[i].length);
                    zend_string_release(bit_result);
                    bit_result = joined;
                }
            }
            part = bit_result != NULL ? bit_result : zend_string_init("mixed", sizeof("mixed") - 1, 0);
        }

        index++;
        const char *separator = index == count ? " or " : ", ";
        if (result == NULL) {
            result = part;
        } else {
            zend_string *joined = as_string_concat3(ZSTR_VAL(result), ZSTR_LEN(result), separator, strlen(separator), ZSTR_VAL(part), ZSTR_LEN(part));
            zend_string_release(result);
            zend_string_release(part);
            result = joined;
        }
    }
    ZEND_HASH_FOREACH_END();

    return result;
}

/*
 * Each variadic arm: int|string|\\Attributes\\Strictify\\Type
 *
 * Checked eagerly so a malformed Union fails at construction instead of at
 * validation time.
 */
ZEND_METHOD(AS_Fields_Constraints_Union, __construct)
{
    zval *arms;
    uint32_t arms_count;

    ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('z', arms, arms_count)
    ZEND_PARSE_PARAMETERS_END();

    for (uint32_t i = 0; i < arms_count; i++) {
        switch (Z_TYPE_P(&arms[i])) {
            case IS_LONG:
                if (!is_valid_mask_arm(Z_LVAL_P(&arms[i]), i + 1)) {
                    RETURN_THROWS();
                }
                continue;

            case IS_STRING:
                if (!is_resolvable_class_name(&arms[i], i + 1)) {
                    RETURN_THROWS();
                }
                continue;

            case IS_OBJECT:
                if (Z_OBJCE_P(&arms[i]) == AS_Type_ce) {
                    continue;
                }
                zend_argument_type_error(i + 1, "must be of type int|string|\\Attributes\\Strictify\\Type, %s given", zend_zval_type_name(&arms[i]));
                RETURN_THROWS();

            default:
                zend_argument_type_error(i + 1, "must be of type int|string|\\Attributes\\Strictify\\Type, %s given", zend_zval_type_name(&arms[i]));
                RETURN_THROWS();
        }
    }

    zval of;
    array_init_size(&of, arms_count);
    for (uint32_t i = 0; i < arms_count; i++) {
        add_next_index_zval(&of, &arms[i]);
        Z_TRY_ADDREF(arms[i]);
    }

    zend_update_property(AS_Fields_Constraints_Union_ce, Z_OBJ_P(getThis()), "of", sizeof("of") - 1, &of);
    zval_ptr_dtor(&of);
}

/*
 * #[Union(...$of)]: the value must match at least one arm. Runs after the
 * property's native type check, so matching is exact — no coercion.
 */
bool as_validate_Union_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *arms = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "of", sizeof("of") - 1, false, NULL);

    zval *value = field->value;
    zval *arm;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arms), arm)
    {
        if (arm_matches_value(arm, value)) {
            return true;
        }
        if (UNEXPECTED(EG(exception) != NULL)) {
            return false;
        }
    }
    ZEND_HASH_FOREACH_END();

    zend_string *expected = union_arms_to_expected_string(arms);
    as_add_field_error_with_expected(AS_ERROR_UNION, field, property, errors, expected);
    zend_string_release(expected);
    return false;
}

void as_register_Constraints_Union_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Union", class_AS_Fields_Constraints_Union_methods);
    AS_Fields_Constraints_Union_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Union_ce != NULL);
    AS_Fields_Constraints_Union_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Union_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Union_ce, "of", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Union_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
