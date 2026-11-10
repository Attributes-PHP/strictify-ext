#include "as_field.h"

zend_class_entry *AS_Fields_Field_ce;

void as_register_Field_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "Field", class_AS_Fields_Field_methods);
    AS_Fields_Field_ce = zend_register_internal_interface(&ce);
    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
}