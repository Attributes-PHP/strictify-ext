#ifndef AS_FIELDS_Field_H
#define AS_FIELDS_Field_H

#include "php.h"

extern zend_class_entry *AS_Fields_Field_ce;

static const zend_function_entry class_AS_Fields_Field_methods[] = {ZEND_FE_END};

/* Registration function */
void as_register_Field_interface(void);

#endif /* AS_FIELDS_Alias_H */