#include "as_hex_rgb_color.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"


zend_class_entry *AS_Fields_Constraints_HexRgbColor_ce;

/*
 * #[HexRgbColor]: see docs/constraints.md.
 */
bool as_validate_HexRgbColor_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_HEX_RGB_COLOR, field, property, errors);
        return false;
    }
    static zend_string *as_pattern = NULL;
    if (!as_pattern_match(&as_pattern, "~^#(?:[0-9A-Fa-f]{3,4}|[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$~", Z_STR_P(value))) {
        as_add_field_error_with_prefix(AS_ERROR_HEX_RGB_COLOR, field, property, errors);
        return false;
    }
    return true;
}


void as_register_HexRgbColor_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "HexRgbColor", class_AS_Fields_Constraints_HexRgbColor_methods);
    AS_Fields_Constraints_HexRgbColor_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_HexRgbColor_ce != NULL);
    AS_Fields_Constraints_HexRgbColor_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_HexRgbColor_ce, 1, AS_Fields_Field_ce);


    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_HexRgbColor_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
