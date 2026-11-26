#include "as_mac_address.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Constraints_MacAddress_ce;

/*
 * #[MacAddress]: see docs/constraints.md.
 */
bool as_validate_MacAddress_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_MAC_ADDRESS, field, property, errors);
        return false;
    }
    static zend_string *as_pattern = NULL;
    if (!as_pattern_match(&as_pattern, "~^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$~", Z_STR_P(value))) {
        as_add_field_error_with_prefix(AS_ERROR_MAC_ADDRESS, field, property, errors);
        return false;
    }
    return true;
}

AS_REGISTER_CONSTRAINT_CLASS(MacAddress, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS)
