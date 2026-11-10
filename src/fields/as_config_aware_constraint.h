#ifndef AS_FIELDS_ConfigAwareConstraint_H
#define AS_FIELDS_ConfigAwareConstraint_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AS_Fields_ConfigAwareConstraint_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_Fields_ConfigAwareConstraint_setModelConfigs, 0, 1, IS_VOID, 0)
ZEND_ARG_OBJ_INFO(0, configs, Attributes\\Strictify\\ModelConfigs, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry class_AS_Fields_ConfigAwareConstraint_methods[] = {
    ZEND_ABSTRACT_ME_WITH_FLAGS(AS_Fields_ConfigAwareConstraint, setModelConfigs, arginfo_class_AS_Fields_ConfigAwareConstraint_setModelConfigs, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT) ZEND_FE_END
};

/* Registration function */
void as_register_ConfigAwareConstraint_interface(void);

#endif /* AS_FIELDS_ConfigAwareConstraint_H */
