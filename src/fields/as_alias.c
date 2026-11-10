#include "as_alias.h"
#include "as_field.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Alias_ce;

ZEND_METHOD(AS_Fields_Alias, __construct)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_string(AS_Fields_Alias_ce, Z_OBJ_P(getThis()), "name", sizeof("name") - 1, ZSTR_VAL(name));
}

void as_register_Alias_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "Alias", class_AS_Fields_Alias_methods);
    AS_Fields_Alias_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Alias_ce != NULL);
    AS_Fields_Alias_ce->ce_flags |= ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Alias_ce, 1, AS_Fields_Field_ce);

    /* Register $name property */
    zend_declare_property_null(AS_Fields_Alias_ce, "name", sizeof("name") - 1, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Alias_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}