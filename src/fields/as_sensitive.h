#ifndef AS_FIELDS_Sensitive_H
#define AS_FIELDS_Sensitive_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Sensitive_ce;

static const zend_function_entry class_AS_Fields_Sensitive_methods[] = {ZEND_FE_END};

/* Registration function */
void as_register_Sensitive_class(void);

#endif /* AS_FIELDS_Sensitive_H */
