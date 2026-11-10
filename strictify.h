/*
 * Header for attributes-strictify PHP extension
 */

#ifndef STRICTIFY_H
#define STRICTIFY_H

#define EXTENSION_NAME    "attributes-strictify"
#define EXTENSION_VERSION "1.0.0"

#include "src/as_call_function.h"
#include "src/as_validate_function.h"

static const zend_function_entry ext_functions[] = {ZEND_NS_FE("Attributes\\Strictify", validate, arginfo_AS_validate) ZEND_NS_FE("Attributes\\Strictify", call, arginfo_AS_call) ZEND_FE_END};

#endif /* STRICTIFY_H */
