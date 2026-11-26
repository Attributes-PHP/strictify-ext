/*
 * Per-request module globals: the request-scoped caches live here
 * instead of file-scope statics, so each thread gets its own copy
 * under ZTS. The engine calls the GINIT ctor (zeros the pointers)
 * when the module starts, or per thread via TSRM under ZTS.
 */

#ifndef AS_GLOBALS_H
#define AS_GLOBALS_H

#include "Zend/zend_API.h"

ZEND_BEGIN_MODULE_GLOBALS(strictify)
HashTable *as_model_configs_cache;
HashTable *as_field_name_cache;
HashTable *as_plans_cache;
HashTable *as_spec_cache;
HashTable *as_error_templates_cache;
/* Current nesting level of as_compiled_validate_model(): nested models and
 * nested array shapes recurse, and the C stack is a finite resource. */
uint32_t validation_depth;
/* Limit for that depth: AS_MAX_VALIDATION_DEPTH by default, overridden for
 * the duration of a validate()/call() run that passes $maxDepth. */
uint32_t validation_depth_limit;
ZEND_END_MODULE_GLOBALS(strictify)

#define AS_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(strictify, v)

ZEND_EXTERN_MODULE_GLOBALS(strictify)

#endif /* AS_GLOBALS_H */
