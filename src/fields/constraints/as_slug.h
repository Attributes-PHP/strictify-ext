#ifndef AS_Fields_Constraints_Slug_H
#define AS_Fields_Constraints_Slug_H

#include "php.h"
#include "../../helpers/as_structs.h"

/* Class entry */
extern zend_class_entry *AS_Fields_Constraints_Slug_ce;

static const zend_function_entry class_AS_Fields_Constraints_Slug_methods[] = {ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool as_validate_Slug_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/* Registration function */
void as_register_Slug_class(void);

#endif /* AS_Fields_Constraints_Slug_H */
