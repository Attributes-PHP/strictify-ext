#include "as_config_aware_constraint.h"
#include "as_constraint.h"
#include "Zend/zend_API.h"

zend_class_entry *AS_Fields_ConfigAwareConstraint_ce;

void as_register_ConfigAwareConstraint_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "ConfigAwareConstraint", class_AS_Fields_ConfigAwareConstraint_methods);
    AS_Fields_ConfigAwareConstraint_ce = zend_register_internal_interface(&ce);
    ZEND_ASSERT(AS_Fields_ConfigAwareConstraint_ce != NULL);

    ZEND_ASSERT(AS_Fields_Constraint_ce != NULL);
    zend_class_implements(AS_Fields_ConfigAwareConstraint_ce, 1, AS_Fields_Constraint_ce);
}
