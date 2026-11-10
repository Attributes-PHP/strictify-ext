#ifndef AS_HELPERS_STRING_H
#define AS_HELPERS_STRING_H

#include <Zend/zend_types.h>
#include "testing.h"

typedef struct {
    char *value;
    size_t length;
} as_string;

/* Registration function */
zend_string *as_to_pascal_case(zend_string *str);
zend_string *as_to_camel_case(zend_string *str);
zend_string *as_to_snake_case(zend_string *str);
zend_string *as_to_kebab_case(zend_string *str);
zend_string *as_string_dot_concat(zend_string *first, zend_string *second);

STATIC_ZEND_INLINE bool is_uppercase(char c);
STATIC_ZEND_INLINE bool is_lowercase(char c);
STATIC_ZEND_INLINE bool is_alphanumeric(char c);
STATIC_INLINE void to_pascal_case(as_string *input, as_string *output);
STATIC_INLINE void to_camel_case(as_string *input, as_string *output);
STATIC_INLINE void to_snake_case(as_string *input, as_string *output);
STATIC_INLINE void to_kebab_case(as_string *input, as_string *output);

#endif /* AS_HELPERS_STRING_H */
