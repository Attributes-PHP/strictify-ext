#include "as_positive.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_Positive_ce;

/*
 * #[Positive]: see docs/constraints.md.
 */
bool as_validate_Positive_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    const bool is_number = Z_TYPE_P(value) == IS_LONG || Z_TYPE_P(value) == IS_DOUBLE;
    if (is_number && ((Z_TYPE_P(value) == IS_LONG ? Z_LVAL_P(value) > 0 : Z_DVAL_P(value) > 0.0))) {
        return true;
    }
    as_add_field_error_with_prefix(AS_ERROR_POSITIVE, field, property, errors);
    return false;
}

AS_REGISTER_CONSTRAINT_CLASS(Positive, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS)
