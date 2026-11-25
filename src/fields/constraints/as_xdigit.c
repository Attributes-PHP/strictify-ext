#include "as_xdigit.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"


zend_class_entry *AS_Fields_Constraints_Xdigit_ce;

/*
 * #[Xdigit]: see docs/constraints.md.
 */
bool as_validate_Xdigit_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_XDIGIT, field, property, errors);
        return false;
    }
    const char *s = Z_STRVAL_P(value);
    const size_t len = Z_STRLEN_P(value);
    if (len == 0) {
        as_add_field_error_with_prefix(AS_ERROR_XDIGIT, field, property, errors);
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        const char c = s[i];
        if (!(((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))) {
            as_add_field_error_with_prefix(AS_ERROR_XDIGIT, field, property, errors);
            return false;
        }
    }
    return true;
}


void as_register_Xdigit_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Xdigit", class_AS_Fields_Constraints_Xdigit_methods);
    AS_Fields_Constraints_Xdigit_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Xdigit_ce != NULL);
    AS_Fields_Constraints_Xdigit_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Xdigit_ce, 1, AS_Fields_Field_ce);


    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Xdigit_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
