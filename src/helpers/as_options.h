#ifndef AS_HELPERS_OPTIONS_H
#define AS_HELPERS_OPTIONS_H

#include <stdbool.h>
#include <stddef.h>
#include "testing.h"

typedef struct {
    char *class_name;
    char *method_name;
    int parameter_number;
    char *name;
} as_invalid_method_parameter;

/* Registration function */
bool as_validate_method_parameter(char *value, char **all_options, size_t length, as_invalid_method_parameter *invalid_parameter_error);
STATIC_INLINE void throw_invalid_parameter_exception(char *value, char **all_options, size_t length, as_invalid_method_parameter *invalid_parameter_error);

#endif /* AS_HELPERS_OPTIONS_H */
