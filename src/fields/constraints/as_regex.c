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

    zend_update_property_string(AS_Fields_Constraints_Regex_ce, Z_OBJ_P(getThis()), "regex", sizeof("regex") - 1, ZSTR_VAL(regex));
}

void as_register_Regex_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Regex", class_AS_Fields_Constraints_Regex_methods);
    AS_Fields_Constraints_Regex_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Regex_ce != NULL);
    AS_Fields_Constraints_Regex_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Regex_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Regex_ce, "regex", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_STRING));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Regex_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
