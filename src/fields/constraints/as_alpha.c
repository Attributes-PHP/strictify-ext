#include "as_alpha.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"


zend_class_entry *AS_Fields_Constraints_Alpha_ce;

/*
 * #[Alpha]: see docs/constraints.md.
 */
bool as_validate_Alpha_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_ALPHA, field, property, errors);
        return false;
    }
    zval *extra_z = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "extra", sizeof("extra") - 1, false, NULL);
    const char *extra = Z_TYPE_P(extra_z) == IS_STRING ? Z_STRVAL_P(extra_z) : "";
    const size_t extra_len = Z_TYPE_P(extra_z) == IS_STRING ? Z_STRLEN_P(extra_z) : 0;
    const char *s = Z_STRVAL_P(value);
    const size_t len = Z_STRLEN_P(value);
    for (size_t i = 0; i < len; i++) {
        const char c = s[i];
        if (!((((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) || (extra_len > 0 && memchr(extra, c, extra_len) != NULL)))) {
            as_add_field_error_with_prefix(AS_ERROR_ALPHA, field, property, errors);
            return false;
        }
    }
    return true;
}
ZEND_METHOD(AS_Fields_Constraints_Alpha, __construct)
{
    zend_string *extra = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR(extra)
    ZEND_PARSE_PARAMETERS_END();

    zval extra_z;
    if (extra != NULL) {
        ZVAL_STR(&extra_z, extra);
    } else {
        ZVAL_STRING(&extra_z, "");
    }
    zend_update_property(AS_Fields_Constraints_Alpha_ce, Z_OBJ_P(getThis()), "extra", sizeof("extra") - 1, &extra_z);
}

void as_register_Alpha_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Alpha", class_AS_Fields_Constraints_Alpha_methods);
    AS_Fields_Constraints_Alpha_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Alpha_ce != NULL);
    AS_Fields_Constraints_Alpha_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Alpha_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Alpha_ce, "extra", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Alpha_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
