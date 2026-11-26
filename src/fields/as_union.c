#include "as_union.h"
#include "as_field.h"
#include "../as_exception.h"
#include "../as_type.h"
#include "../helpers/as_error_messages.h"
#include "../helpers/as_type_names.h"
#include "../helpers/as_wrappers.h"
#include "as_dict.h"
#include "as_intersection.h"
#include "as_sequence.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_enum.h"
#include "Zend/zend_exceptions.h"
#include <string.h>

zend_class_entry *AS_Fields_Union_ce;

static bool is_valid_union_arm(zval *value);
static bool is_resolvable_class_name(zval *value, uint32_t arg_num);

/*
 * Each variadic arm: int|string|\Attributes\\Strictify\Type|Sequence|Union|Intersection|Dict
 *
 * Checked eagerly so a malformed Union fails at construction instead of at
 * validation time; the validator still applies the semantic rules (valid
 * masks, resolvable class names).
 */
static bool is_valid_union_arm(zval *value)
{
    switch (Z_TYPE_P(value)) {
        case IS_LONG:
        case IS_STRING:
            return true;

        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(value);
            return ce == AS_Type_ce || ce == AS_Fields_Sequence_ce || ce == AS_Fields_Union_ce || ce == AS_Fields_Intersection_ce || ce == AS_Fields_Dict_ce;
        }

        default:
            return false;
    }
}

/*
 * A string arm must name a loadable class, interface or enum: a missing
 * name autoloads, and a name that still cannot be resolved throws a
 * ValueError (an autoloader error propagates as-is). Read from a declaration
 * the ValueError reaches the caller as InvalidBaseModelException.
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

ZEND_METHOD(AS_Fields_Union, __construct)
{
    zval *arms;
    uint32_t arms_count;

    ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('z', arms, arms_count)
    ZEND_PARSE_PARAMETERS_END();

    for (uint32_t i = 0; i < arms_count; i++) {
        if (!is_valid_union_arm(&arms[i])) {
            zend_argument_type_error(i + 1, "must be of type int|string|\\Attributes\\Strictify\\Type|Sequence|Union|Intersection|Dict, %s given", zend_zval_type_name(&arms[i]));
            RETURN_THROWS();
        }

        if (Z_TYPE_P(&arms[i]) == IS_STRING && !is_resolvable_class_name(&arms[i], i + 1)) {
            RETURN_THROWS();
        }
    }

    zval of;
    array_init_size(&of, arms_count);
    for (uint32_t i = 0; i < arms_count; i++) {
        add_next_index_zval(&of, &arms[i]);
        Z_TRY_ADDREF(arms[i]);
    }

    zend_update_property(AS_Fields_Union_ce, Z_OBJ_P(getThis()), "of", sizeof("of") - 1, &of);
    zval_ptr_dtor(&of);
}

/*
 * Scalar path (Union declared directly on a non-array property): exact
 * type match per arm, never a coercion — the constraint runs after the
 * native type check, so the value is already of the declared type.
 */
static uint32_t union_normalize_mask(zend_long type_value);
static bool union_arm_matches_scalar(zval *arm, zval *value);
static zend_string *union_scalar_arms_to_expected_string(zval *arms);

static uint32_t union_normalize_mask(zend_long type_value)
{
    uint32_t mask = (uint32_t)type_value;
    if ((mask & (uint32_t)AS_TYPE_DICT) != 0) {
        mask = (mask & ~(uint32_t)AS_TYPE_DICT) | MAY_BE_ARRAY;
    }
    return mask;
}

static bool union_arm_matches_scalar(zval *arm, zval *value)
{
    switch (Z_TYPE_P(arm)) {
        case IS_LONG:
            return (union_normalize_mask(Z_LVAL_P(arm)) & (1u << Z_TYPE_P(value))) != 0;

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

        case IS_OBJECT: {
            zend_class_entry *arm_ce = Z_OBJCE_P(arm);

            if (arm_ce == AS_Type_ce) {
                zval *case_value = zend_enum_fetch_case_value(Z_OBJ_P(arm));
                if (Z_TYPE_P(case_value) != IS_LONG) {
                    ZEND_ASSERT(false);
                    return false;
                }
                return (union_normalize_mask(Z_LVAL_P(case_value)) & (1u << Z_TYPE_P(value))) != 0;
            }

            if (arm_ce == AS_Fields_Intersection_ce) {
                // An intersection arm matches when the value is an object
                // that is an instance of every declared class
                if (Z_TYPE_P(value) != IS_OBJECT) {
                    return false;
                }
                zval *of = zend_read_property(arm_ce, Z_OBJ_P(arm), "of", sizeof("of") - 1, false, NULL);
                zval *entry;
                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(of), entry)
                {
                    if (Z_TYPE_P(entry) != IS_STRING) {
                        continue;
                    }
                    zend_class_entry *ce = zend_lookup_class(Z_STR_P(entry));
                    if (ce == NULL || !instanceof_function(Z_OBJCE_P(value), ce)) {
                        return false;
                    }
                }
                ZEND_HASH_FOREACH_END();
                return true;
            }

            // Sequence/Union/Dict arms describe nested arrays: they can
            // never match here because array values are rejected up front
            return false;
        }

        default:
            return false;
    }
}

/*
 * Renders the arm list like a union type: mask arms expand to their basic
 * type names, class arms render as written, intersection arms join their
 * class names with " and ", array-shape arms render as "array"; parts are
 * joined with ", " and a trailing " or ".
 */
static zend_string *union_scalar_arms_to_expected_string(zval *arms)
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

        switch (Z_TYPE_P(arm)) {
            case IS_STRING:
                part = zend_string_copy(Z_STR_P(arm));
                break;

            case IS_OBJECT: {
                zend_class_entry *arm_ce = Z_OBJCE_P(arm);
                if (arm_ce == AS_Fields_Intersection_ce) {
                    zval *of = zend_read_property(arm_ce, Z_OBJ_P(arm), "of", sizeof("of") - 1, false, NULL);
                    zval *entry;
                    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(of), entry)
                    {
                        if (Z_TYPE_P(entry) != IS_STRING) {
                            continue;
                        }
                        if (part == NULL) {
                            part = zend_string_copy(Z_STR_P(entry));
                        } else {
                            zend_string *joined = as_string_concat3(ZSTR_VAL(part), ZSTR_LEN(part), " and ", 5, ZSTR_VAL(Z_STR_P(entry)), ZSTR_LEN(Z_STR_P(entry)));
                            zend_string_release(part);
                            part = joined;
                        }
                    }
                    ZEND_HASH_FOREACH_END();
                    if (part == NULL) {
                        part = zend_string_init("mixed", sizeof("mixed") - 1, 0);
                    }
                } else if (arm_ce == AS_Type_ce) {
                    zval *case_value = zend_enum_fetch_case_value(Z_OBJ_P(arm));
                    zend_long mask_value = Z_TYPE_P(case_value) == IS_LONG ? Z_LVAL_P(case_value) : 0;
                    part = as_type_mask_to_string(union_normalize_mask(mask_value), false);
                } else {
                    // Sequence/Union/Dict nested-shape arm
                    part = zend_string_init("array", sizeof("array") - 1, 0);
                }
                break;
            }

            default:
                part = as_type_mask_to_string(union_normalize_mask(Z_LVAL_P(arm)), false);
                break;
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
 * #[Union(...)] declared directly on a property: the whole value must
 * match at least one arm exactly. Array values are a usage error —
 * element specs belong inside Sequence(of: new Union(...)) or
 * Dict(of: ...) — and throw a ValueError.
 */
bool as_validate_Union_scalar(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;

    if (Z_TYPE_P(value) == IS_ARRAY) {
        zend_throw_exception_ex(class_AS_Exceptions_InvalidBaseModelException, 0, "Union cannot be applied directly to array values; use Sequence(of: new Union(...)) or Dict(of: ...) instead.");
        return false;
    }

    zval *arms = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "of", sizeof("of") - 1, false, NULL);

    zval *arm;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arms), arm)
    {
        if (union_arm_matches_scalar(arm, value)) {
            return true;
        }
        if (UNEXPECTED(EG(exception) != NULL)) {
            return false;
        }
    }
    ZEND_HASH_FOREACH_END();

    zend_string *expected = union_scalar_arms_to_expected_string(arms);
    as_add_field_error_with_expected(AS_ERROR_UNION, field, property, errors, expected);
    zend_string_release(expected);
    return false;
}

/*
 * Compile-time consistency rule: a Union attribute declared directly on a
 * property whose native hint is a union must contain the same type-hints as
 * the hint (order-insensitive; a nullable hint requires the T_NULL arm,
 * since nullability is part of the hint; class names compared
 * case-insensitively). Hints with an intersection (DNF) arm are not
 * comparable and skip the rule. On mismatch throws InvalidBaseModelException.
 */

/* The Union's class-name arms must equal the hint list's named arms (case-insensitive) */
static bool union_class_arms_match_hint(zval *arms, const zend_type_list *list)
{
    uint32_t hint_count = 0;
    for (uint32_t i = 0; i < list->num_types; i++) {
        if (ZEND_TYPE_HAS_NAME(list->types[i])) {
            hint_count++;
        }
    }

    uint32_t union_count = 0;
    zval *arm;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arms), arm)
    {
        if (Z_TYPE_P(arm) != IS_STRING) {
            continue;
        }
        union_count++;
        bool found = false;
        for (uint32_t i = 0; i < list->num_types; i++) {
            if (ZEND_TYPE_HAS_NAME(list->types[i]) && zend_string_equals_ci(ZEND_TYPE_NAME(list->types[i]), Z_STR_P(arm))) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    ZEND_HASH_FOREACH_END();

    return union_count == hint_count;
}

bool as_check_union_matches_hint(zval *union_instance, zend_type hint, zend_class_entry *declaring_ce, zend_string *prop_name)
{
    zval *arms = zend_read_property(AS_Fields_Union_ce, Z_OBJ_P(union_instance), "of", sizeof("of") - 1, false, NULL);
    ZEND_ASSERT(Z_TYPE_P(arms) == IS_ARRAY);

    uint32_t hint_mask = ZEND_TYPE_PURE_MASK(hint);
    uint32_t union_mask = 0;
    bool union_has_shape_arm = false;

    zval *arm;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arms), arm)
    {
        if (Z_TYPE_P(arm) == IS_STRING) {
            continue; // class-name arms are compared separately below
        }
        if (Z_TYPE_P(arm) == IS_LONG) {
            union_mask |= union_normalize_mask(Z_LVAL_P(arm));
            continue;
        }
        if (Z_TYPE_P(arm) == IS_OBJECT && Z_OBJCE_P(arm) == AS_Type_ce) {
            zval *case_value = zend_enum_fetch_case_value(Z_OBJ_P(arm));
            union_mask |= union_normalize_mask(Z_TYPE_P(case_value) == IS_LONG ? Z_LVAL_P(case_value) : 0);
            continue;
        }
        // Intersection/Sequence/Union/Dict shape arm: cannot correspond to
        // a native hint arm on a union property
        union_has_shape_arm = true;
    }
    ZEND_HASH_FOREACH_END();

    bool matches = !union_has_shape_arm && union_mask == hint_mask;

    if (matches && ZEND_TYPE_HAS_LIST(hint)) {
        const zend_type_list *list = ZEND_TYPE_LIST(hint);
        for (uint32_t i = 0; i < list->num_types; i++) {
            if (ZEND_TYPE_IS_INTERSECTION(list->types[i])) {
                return true; // DNF arm: rule not applicable
            }
        }
        matches = union_class_arms_match_hint(arms, list);
    }

    if (!matches) {
        zend_string *expected = build_union_type_string(hint);
        zend_string *got = union_scalar_arms_to_expected_string(arms);
        zend_throw_exception_ex(
            class_AS_Exceptions_InvalidBaseModelException, 0, "Union attribute on property %s::$%s must contain the same type-hints as the native union hint (%s); got (%s).", ZSTR_VAL(declaring_ce->name),
            ZSTR_VAL(prop_name), ZSTR_VAL(expected), ZSTR_VAL(got)
        );
        zend_string_release(expected);
        zend_string_release(got);
        return false;
    }

    return true;
}

void as_register_Union_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "Union", class_AS_Fields_Union_methods);
    AS_Fields_Union_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Union_ce != NULL);
    AS_Fields_Union_ce->ce_flags |= ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Union_ce, 1, AS_Fields_Field_ce);

    /* Register $of property */
    zend_declare_property_null(AS_Fields_Union_ce, "of", sizeof("of") - 1, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Union_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
