#include "as_sensitive.h"
#include "as_field.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AS_Fields_Sensitive_ce;

void as_register_Sensitive_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "Sensitive", class_AS_Fields_Sensitive_methods);
    AS_Fields_Sensitive_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Sensitive_ce != NULL);
    AS_Fields_Sensitive_ce->ce_flags |= ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Sensitive_ce, 1, AS_Fields_Field_ce);

    /* Register as an internal attribute that can be used on properties */
    zend_internal_attribute_register(AS_Fields_Sensitive_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY);
}
