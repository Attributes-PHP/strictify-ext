#include "as_url.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"
#include <string.h>

zend_class_entry *AS_Fields_Constraints_Url_ce;

/* Copies a string|array|null constructor argument onto dest, rejecting other
 * types and arrays holding anything but strings. dest must be released. */
zend_always_inline static bool as_url_read_list_arg(zval *arg, uint32_t arg_num, zval *dest)
{
    if (arg == NULL) {
        ZVAL_NULL(dest);
        return true;
    }

    if (Z_TYPE_P(arg) == IS_ARRAY) {
        zval *entry;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arg), entry)
        {
            if (Z_TYPE_P(entry) != IS_STRING) {
                zend_argument_type_error(arg_num, "must contain only strings, %s given", zend_zval_type_name(entry));
                return false;
            }
        }
        ZEND_HASH_FOREACH_END();
        ZVAL_COPY(dest, arg);
        return true;
    }

    if (Z_TYPE_P(arg) == IS_STRING) {
        ZVAL_COPY(dest, arg);
        return true;
    }

    zend_argument_type_error(arg_num, "must be of type string|array|null, %s given", zend_zval_type_name(arg));
    return false;
}

/* Case-insensitive match of a URL part against a declared string or list.
 * Protocol entries tolerate the separators people write: 'https', 'https:'
 * and 'https://' all mean the same scheme. */
zend_always_inline static bool as_url_matches(zval *allowed, const char *part, size_t part_len, bool trim_separator)
{
    const char *candidate;
    size_t candidate_len;

    if (Z_TYPE_P(allowed) == IS_ARRAY) {
        zval *entry;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(allowed), entry)
        {
            candidate = Z_STRVAL_P(entry);
            candidate_len = Z_STRLEN_P(entry);
            if (trim_separator) {
                if (candidate_len > 3 && memcmp(candidate + candidate_len - 3, "://", 3) == 0) {
                    candidate_len -= 3;
                } else if (candidate_len > 1 && candidate[candidate_len - 1] == ':') {
                    candidate_len--;
                }
            }
            if (zend_binary_strcasecmp(part, part_len, candidate, candidate_len) == 0) {
                return true;
            }
        }
        ZEND_HASH_FOREACH_END();
        return false;
    }

    candidate = Z_STRVAL_P(allowed);
    candidate_len = Z_STRLEN_P(allowed);
    if (trim_separator) {
        if (candidate_len > 3 && memcmp(candidate + candidate_len - 3, "://", 3) == 0) {
            candidate_len -= 3;
        } else if (candidate_len > 1 && candidate[candidate_len - 1] == ':') {
            candidate_len--;
        }
    }

    return zend_binary_strcasecmp(part, part_len, candidate, candidate_len) == 0;
}

/* Reports a rejected part, rendering the declared string or list through the
 * usual quoted value list. */
zend_always_inline static void as_url_add_list_error(int error_type, zval *allowed, as_field *field, as_property_info *property, zval *errors)
{
    zval list;

    if (Z_TYPE_P(allowed) == IS_ARRAY) {
        ZVAL_COPY(&list, allowed);
    } else {
        array_init(&list);
        add_next_index_str(&list, zend_string_copy(Z_STR_P(allowed)));
    }

    zend_string *expected = as_values_to_list_string(&list);
    as_add_field_error_with_expected(error_type, field, property, errors, expected);
    as_string_release(expected);
    zval_ptr_dtor(&list);
}

/*
 * #[Url(protocol: ...|[...], domain: ...|[...])]: both default to null, which
 * allows any. Elements are validated here so the validator can compare blindly.
 */
ZEND_METHOD(AS_Fields_Constraints_Url, __construct)
{
    zval *protocol = NULL;
    zval *domain = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL_OR_NULL(protocol)
    Z_PARAM_ZVAL_OR_NULL(domain)
    ZEND_PARSE_PARAMETERS_END();

    zval protocol_z;
    zval domain_z;

    if (!as_url_read_list_arg(protocol, 1, &protocol_z)) {
        RETURN_THROWS();
    }
    if (!as_url_read_list_arg(domain, 2, &domain_z)) {
        zval_ptr_dtor(&protocol_z);
        RETURN_THROWS();
    }

    zend_update_property(AS_Fields_Constraints_Url_ce, Z_OBJ_P(getThis()), "protocol", sizeof("protocol") - 1, &protocol_z);
    zend_update_property(AS_Fields_Constraints_Url_ce, Z_OBJ_P(getThis()), "domain", sizeof("domain") - 1, &domain_z);
    zval_ptr_dtor(&protocol_z);
    zval_ptr_dtor(&domain_z);
}

/*
 * #[Url]: see docs/constraints.md.
 */
bool as_validate_Url_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_URL, field, property, errors);
        return false;
    }
    static zend_string *as_pattern = NULL;
    if (!as_pattern_match(&as_pattern, "~^[A-Za-z][A-Za-z0-9+.-]*:\\/\\/[^\\s]+$~", Z_STR_P(value))) {
        as_add_field_error_with_prefix(AS_ERROR_URL, field, property, errors);
        return false;
    }

    zval *protocol = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "protocol", sizeof("protocol") - 1, false, NULL);
    zval *domain = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "domain", sizeof("domain") - 1, false, NULL);
    const bool check_protocol = protocol != NULL && Z_TYPE_P(protocol) != IS_NULL;
    const bool check_domain = domain != NULL && Z_TYPE_P(domain) != IS_NULL;
    if (!check_protocol && !check_domain) {
        return true;
    }

    /* the format above guarantees a scheme followed by "://" */
    const char *start = Z_STRVAL_P(value);
    const char *end = start + Z_STRLEN_P(value);
    const char *separator = as_memnstr(start, "://", 3, end);
    const char *scheme_end = separator;

    if (check_protocol && !as_url_matches(protocol, start, (size_t)(scheme_end - start), true)) {
        as_url_add_list_error(AS_ERROR_URL_PROTOCOL, protocol, field, property, errors);
        return false;
    }

    if (check_domain) {
        const char *authority = separator + 3;
        const char *authority_end = authority;
        while (authority_end < end && *authority_end != '/' && *authority_end != '?' && *authority_end != '#') {
            authority_end++;
        }

        const char *host = authority;
        for (const char *cursor = authority; cursor < authority_end; cursor++) {
            if (*cursor == '@') {
                host = cursor + 1;
            }
        }

        const char *host_end = host;
        while (host_end < authority_end && *host_end != ':') {
            host_end++;
        }

        if (!as_url_matches(domain, host, (size_t)(host_end - host), false)) {
            as_url_add_list_error(AS_ERROR_URL_DOMAIN, domain, field, property, errors);
            return false;
        }
    }

    return true;
}

AS_REGISTER_CONSTRAINT_CLASS(
    Url, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS,
    as_declare_typed_property(AS_Fields_Constraints_Url_ce, "protocol", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_STRING | MAY_BE_ARRAY | MAY_BE_NULL));
    as_declare_typed_property(AS_Fields_Constraints_Url_ce, "domain", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_STRING | MAY_BE_ARRAY | MAY_BE_NULL));
)
