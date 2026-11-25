#include "as_sequence.h"
#include "as_field.h"
#include "../as_type.h"
#include "../helpers/as_wrappers.h"
#include "as_dict.h"
#include "as_intersection.h"
#include "as_union.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Sequence_ce;

static bool is_valid_sequence_of(zval *value);
static bool is_resolvable_class_name(zval *value);

/*
 * $of: int|string|\Attributes\\Strictify\Type|Sequence|Union|Intersection|Dict
 *
 * Checked eagerly so a malformed Sequence fails at construction instead of
 * at validation time; the validator still applies the semantic rules
 * (valid masks, resolvable class names).
 */
static bool is_valid_sequence_of(zval *value)
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
 * A string $of must name a loadable class, interface or enum: a missing
 * name autoloads, and a name that still cannot be resolved throws a
 * ValueError (an autoloader error propagates as-is).
 */
static bool is_resolvable_class_name(zval *value)
{
    if (zend_lookup_class(Z_STR_P(value)) != NULL) {
        return true;
    }

    if (EG(exception) == NULL) {
        zend_argument_value_error(1, "must be a valid class, interface or enum, \"%s\" given", ZSTR_VAL(Z_STR_P(value)));
    }

    return false;
}

ZEND_METHOD(AS_Fields_Sequence, __construct)
{
    zval *of;
    bool unique_values = false;
    bool sequential = true;
    zend_string *sorted = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 4)
    Z_PARAM_ZVAL(of)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(unique_values)
    Z_PARAM_BOOL(sequential)
    Z_PARAM_STR_OR_NULL(sorted)
    ZEND_PARSE_PARAMETERS_END();

    if (!is_valid_sequence_of(of)) {
        zend_argument_type_error(1, "must be of type int|string|\\Attributes\\Strictify\\Type|Sequence|Union|Intersection|Dict, %s given", zend_zval_type_name(of));
        RETURN_THROWS();
    }

    if (Z_TYPE_P(of) == IS_STRING && !is_resolvable_class_name(of)) {
        RETURN_THROWS();
    }

    if (sorted != NULL && !zend_string_equals_literal_ci(sorted, "asc") && !zend_string_equals_literal_ci(sorted, "desc")) {
        zend_argument_value_error(4, "must be \"asc\", \"ASC\", \"desc\", \"DESC\" or null, \"%s\" given", ZSTR_VAL(sorted));
        RETURN_THROWS();
    }

    zend_update_property(AS_Fields_Sequence_ce, Z_OBJ_P(getThis()), "of", sizeof("of") - 1, of);
    zend_update_property_bool(AS_Fields_Sequence_ce, Z_OBJ_P(getThis()), "uniqueValues", sizeof("uniqueValues") - 1, unique_values);
    zend_update_property_bool(AS_Fields_Sequence_ce, Z_OBJ_P(getThis()), "sequential", sizeof("sequential") - 1, sequential);

    if (sorted == NULL) {
        zend_update_property_null(AS_Fields_Sequence_ce, Z_OBJ_P(getThis()), "sorted", sizeof("sorted") - 1);
    } else {
        zval sorted_value;
        ZVAL_STR_COPY(&sorted_value, sorted);
        zend_update_property(AS_Fields_Sequence_ce, Z_OBJ_P(getThis()), "sorted", sizeof("sorted") - 1, &sorted_value);
        zval_ptr_dtor(&sorted_value);
    }
}

void as_register_Sequence_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "Sequence", class_AS_Fields_Sequence_methods);
    AS_Fields_Sequence_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Sequence_ce != NULL);
    AS_Fields_Sequence_ce->ce_flags |= ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Sequence_ce, 1, AS_Fields_Field_ce);

    /* Register $of property */
    zend_declare_property_null(AS_Fields_Sequence_ce, "of", sizeof("of") - 1, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);

    /* Register the $uniqueValues and $sequential flags. No default value:
     * on a readonly class a default would initialize the property, making
     * the constructor's single write fail; the constructor always sets them. */
    as_declare_typed_property(AS_Fields_Sequence_ce, "uniqueValues", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
    as_declare_typed_property(AS_Fields_Sequence_ce, "sequential", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
    as_declare_typed_property(AS_Fields_Sequence_ce, "sorted", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_STRING | MAY_BE_NULL));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Sequence_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
