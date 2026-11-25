#include "as_email.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"
#include <string.h>

zend_class_entry *AS_Fields_Constraints_Email_ce;

/*
 * #[Email(domain: 'example.com'|['example.com', ...])]: $domain stays null
 * (any domain) unless the attribute declares an allowed string or array of
 * strings. Elements are validated here so the validator can compare blindly.
 */
ZEND_METHOD(AS_Fields_Constraints_Email, __construct)
{
    zval *domain = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(domain)
    ZEND_PARSE_PARAMETERS_END();

    zval domain_z;

    if (domain == NULL) {
        ZVAL_NULL(&domain_z);
    } else if (Z_TYPE_P(domain) == IS_ARRAY) {
        zval *allowed;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(domain), allowed)
        {
            if (Z_TYPE_P(allowed) != IS_STRING) {
                zend_argument_type_error(1, "must contain only strings, %s given", zend_zval_type_name(allowed));
                RETURN_THROWS();
            }
        }
        ZEND_HASH_FOREACH_END();
        ZVAL_COPY(&domain_z, domain);
    } else if (Z_TYPE_P(domain) == IS_STRING) {
        ZVAL_COPY(&domain_z, domain);
    } else {
        zend_argument_type_error(1, "must be of type string|array|null, %s given", zend_zval_type_name(domain));
        RETURN_THROWS();
    }

    zend_update_property(AS_Fields_Constraints_Email_ce, Z_OBJ_P(getThis()), "domain", sizeof("domain") - 1, &domain_z);
    zval_ptr_dtor(&domain_z);
}

/*
 * #[Email]: see docs/constraints.md.
 */
bool as_validate_Email_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_EMAIL, field, property, errors);
        return false;
    }
    static zend_string *as_pattern = NULL;
    if (!as_pattern_match(&as_pattern, "~^[A-Za-z0-9.!#$%&'*+\\/?^_`{|}\\~-]+@[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?(?:\\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)+$~", Z_STR_P(value))) {
        as_add_field_error_with_prefix(AS_ERROR_EMAIL, field, property, errors);
        return false;
    }

    zval *domain = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "domain", sizeof("domain") - 1, false, NULL);
    if (domain == NULL || Z_TYPE_P(domain) == IS_NULL) {
        return true;
    }

    /* the format above allows exactly one '@', so the first one ends the local part */
    const char *at = memchr(Z_STRVAL_P(value), '@', Z_STRLEN_P(value));
    const char *value_domain = at + 1;
    const size_t value_domain_len = Z_STRLEN_P(value) - (size_t)(value_domain - Z_STRVAL_P(value));

    bool allowed = false;
    if (Z_TYPE_P(domain) == IS_ARRAY) {
        zval *candidate;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(domain), candidate)
        {
            if (zend_binary_strcasecmp(value_domain, value_domain_len, Z_STRVAL_P(candidate), Z_STRLEN_P(candidate)) == 0) {
                allowed = true;
                break;
            }
        }
        ZEND_HASH_FOREACH_END();
    } else {
        allowed = zend_binary_strcasecmp(value_domain, value_domain_len, Z_STRVAL_P(domain), Z_STRLEN_P(domain)) == 0;
    }

    if (allowed) {
        return true;
    }

    zval allowed_list;
    if (Z_TYPE_P(domain) == IS_ARRAY) {
        ZVAL_COPY(&allowed_list, domain);
    } else {
        array_init(&allowed_list);
        add_next_index_str(&allowed_list, zend_string_copy(Z_STR_P(domain)));
    }

    zend_string *expected = as_values_to_list_string(&allowed_list);
    as_add_field_error_with_expected(AS_ERROR_EMAIL_DOMAIN, field, property, errors, expected);
    as_string_release(expected);
    zval_ptr_dtor(&allowed_list);

    return false;
}

void as_register_Email_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Email", class_AS_Fields_Constraints_Email_methods);
    AS_Fields_Constraints_Email_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Email_ce != NULL);
    AS_Fields_Constraints_Email_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Email_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Email_ce, "domain", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_STRING | MAY_BE_ARRAY | MAY_BE_NULL));

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Email_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
