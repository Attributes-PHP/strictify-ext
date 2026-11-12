/**
 * Type hint validation with custom coercion support
 *
 * This header declares the type hint validation function that handles
 * property type validation with support for:
 * - Simple types with custom coercion (e.g., bool)
 * - Union types (PHP 8.2+) like bool|int
 * - Strict and loose validation modes
 */

#ifndef AS_TYPEHINT_VALIDATION_H
#define AS_TYPEHINT_VALIDATION_H

#include "php.h"
#include "../as_model_configs.h"
#include "../as_validate_function.h"
#include "../as_compiled_model.h"
#include "../as_exception.h"
#include "as_array_typehint_validator.h"

#define AS_EPSILON 1e-15

/*
 * Resolves a named type arm to its class entry: the ce cache when
 * populated, otherwise the declaring-class scoped self/parent lookup.
 * Unresolvable names return NULL.
 */
zend_class_entry *as_get_ce_from_type(zend_property_info *info, const zend_type *type);

bool as_handle_class_by_ce(as_field *field, as_property_info *prop_info, zend_class_entry *ce, as_model_configs_properties *properties, zval *errors, as_compiled_field *cf);
bool as_coerce_bool(as_field *field);

void as_init_typehint_validator(void);
void as_shutdown_typehint_validator(void);

/**
 * Validates that a value matches the property's compiled type arms.
 *
 * For union types, tries each arm in the union.
 *
 * @param field         The field related structure
 * @param cf            The compiled field slot
 * @param prop_info     Property type information (error rendering)
 * @param properties    Model configuration properties (for recursive validation)
 * @param errors        Error collection array
 * @return              true if validation succeeds, false otherwise
 */
bool as_validate_compiled_type_hint(as_field *field, as_compiled_field *cf, as_property_info *prop_info, as_model_configs_properties *properties, zval *errors);

#endif /* AS_TYPEHINT_VALIDATION_H */
