#include "as_prime_number.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_PrimeNumber_ce;

static bool as_is_prime(zend_long n)
{
    if (n < 2) {
        return false;
    }
    if (n < 4) {
        return true;
    }
    if (n % 2 == 0) {
        return false;
    }
    for (zend_long d = 3; d * d <= n; d += 2) {
        if (n % d == 0) {
            return false;
        }
    }
    return true;
}

/*
 * #[PrimeNumber]: see docs/constraints.md.
 */
bool as_validate_PrimeNumber_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    const bool is_number = Z_TYPE_P(value) == IS_LONG || Z_TYPE_P(value) == IS_DOUBLE;
    if (is_number && (as_is_prime(Z_TYPE_P(value) == IS_LONG ? Z_LVAL_P(value) : (zend_long)Z_DVAL_P(value)))) {
        return true;
    }
    as_add_field_error_with_prefix(AS_ERROR_PRIME_NUMBER, field, property, errors);
    return false;
}

AS_REGISTER_CONSTRAINT_CLASS(PrimeNumber, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS)
