#include "as_regex.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include <ext/pcre/php_pcre.h>

zend_class_entry *AS_Fields_Constraints_Regex_ce;

/*
 * #[Regex(pattern)]: the string value must match the regular expression
 * (a partial match suffices, mirroring preg_match()). Non-string
 * values fail the constraint; an invalid pattern fails the value and
 * raises the PCRE compilation warning.
 */
bool as_validate_Regex_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *regex = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "regex", sizeof("regex") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        add_invalid_regex_error_message(regex, field, property, errors);
        return false;
    }

    pcre_cache_entry *pce = pcre_get_compiled_regex_cache(Z_STR_P(regex));
    if (pce == NULL) {
        add_invalid_regex_error_message(regex, field, property, errors);
        return false;
    }

    zval match;
    ZVAL_UNDEF(&match);
#if PHP_VERSION_ID >= 80400
    /* PHP 8.4 removed the use_flags argument. */
    php_pcre_match_impl(pce, Z_STR_P(value), &match, NULL, 0, 0, 0);
#else
    php_pcre_match_impl(pce, Z_STR_P(value), &match, NULL, 0, 0, 0, 0);
#endif
    const bool is_valid = (Z_TYPE(match) == IS_LONG && Z_LVAL(match) > 0);
    if (!is_valid)
        add_invalid_regex_error_message(regex, field, property, errors);
    return is_valid;
}

zend_always_inline static void add_invalid_regex_error_message(zval *regex, as_field *field, as_property_info *property, zval *errors);

zend_always_inline static void add_invalid_regex_error_message(zval *regex, as_field *field, as_property_info *property, zval *errors)
{
    zend_string *expected = as_value_to_string(regex);
    as_add_field_error_with_expected(AS_ERROR_REGEX, field, property, errors, expected);
    as_string_release(expected);
}

ZEND_METHOD(AS_Fields_Constraints_Regex, __construct)
{
    zend_string *regex;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(regex)
    ZEND_PARSE_PARAMETERS_END();

    /* zend_update_property_str (not _string): a pattern may legitimately
     * carry bytes that strlen would cut at, and truncating it would change
     * the pattern reported by the error message. */
    zend_update_property_str(AS_Fields_Constraints_Regex_ce, Z_OBJ_P(getThis()), "regex", sizeof("regex") - 1, regex);
}

AS_REGISTER_CONSTRAINT_CLASS(
    Regex, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS, as_declare_typed_property(AS_Fields_Constraints_Regex_ce, "regex", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_STRING));
)
