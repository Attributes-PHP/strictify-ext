#include "as_base64.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"


zend_class_entry *AS_Fields_Constraints_Base64_ce;

/*
 * #[Base64]: see docs/constraints.md.
 */
bool as_validate_Base64_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_BASE64, field, property, errors);
        return false;
    }
    static zend_string *as_pattern = NULL;
    if (Z_STRLEN_P(value) % 4 != 0 || !as_pattern_match(&as_pattern, "~^[A-Za-z0-9+/]*={0,2}$~", Z_STR_P(value))) {
        as_add_field_error_with_prefix(AS_ERROR_BASE64, field, property, errors);
        return false;
    }
    return true;
}


void as_register_Base64_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Base64", class_AS_Fields_Constraints_Base64_methods);
    AS_Fields_Constraints_Base64_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Base64_ce != NULL);
    AS_Fields_Constraints_Base64_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Base64_ce, 1, AS_Fields_Field_ce);


    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Base64_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
