#include "as_options.h"
#include <string.h>
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_smart_str.h"
#include "ext/spl/spl_exceptions.h"
#include "as_wrappers.h"
#include "../as_exception.h"

bool as_validate_method_parameter(char *option, char **all_valid_options, size_t length, as_invalid_method_parameter *invalid_parameter_error)
{
    for (size_t i = 0; i < length; i++) {
        if (as_binary_strcasecmp(option, strlen(option), all_valid_options[i], strlen(all_valid_options[i])) == 0) {
            return true;
        }
    }

    throw_invalid_parameter_exception(option, all_valid_options, length, invalid_parameter_error);
    return false;
}

STATIC_INLINE void throw_invalid_parameter_exception(char *option, char **all_options, size_t length, as_invalid_method_parameter *invalid_option_error)
{
    smart_str options_list = {0};

    for (size_t i = 0; i < length; i++) {
        if (i > 0) {
            smart_str_appends(&options_list, ", ");
        }
        smart_str_appendl(&options_list, all_options[i], strlen(all_options[i]));
    }

    smart_str_0(&options_list);
    zend_throw_exception_ex(
        class_AS_Exceptions_InvalidBaseModelException, 0, "%s::%s: Argument #%d must be of one of the following options: %s", invalid_option_error->class_name, invalid_option_error->method_name,
        invalid_option_error->parameter_number, ZSTR_VAL(options_list.s)
    );
    smart_str_free(&options_list);
}
