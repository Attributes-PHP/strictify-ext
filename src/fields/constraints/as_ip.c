#include "as_ip.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include <string.h>

zend_class_entry *AS_Fields_Constraints_Ip_ce;

static bool as_is_ipv4(zend_string *str)
{
    const char *s = ZSTR_VAL(str);
    size_t len = ZSTR_LEN(str);
    int dots = 0;
    size_t part_start = 0;
    for (size_t i = 0; i <= len; i++) {
        if (i == len || s[i] == '.') {
            size_t part_len = i - part_start;
            if (part_len < 1 || part_len > 3) {
                return false;
            }
            if (part_len > 1 && s[part_start] == '0') {
                return false;
            }
            int octet = 0;
            for (size_t j = part_start; j < i; j++) {
                if (s[j] < '0' || s[j] > '9') {
                    return false;
                }
                octet = octet * 10 + (s[j] - '0');
            }
            if (octet > 255) {
                return false;
            }
            if (i == len) {
                break;
            }
            dots++;
            part_start = i + 1;
        }
    }
    return dots == 3;
}

static bool as_hex_group(const char *s, size_t len)
{
    if (len < 1 || len > 4) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

static bool as_is_ipv6(zend_string *str)
{
    const char *s = ZSTR_VAL(str);
    size_t len = ZSTR_LEN(str);
    if (len == 0 || len > 45) {
        return false;
    }
    /* embedded IPv4 tail, e.g. ::ffff:192.168.0.1 */
    const char *last_colon = NULL;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == ':') {
            last_colon = s + i;
        }
    }
    size_t groups = 0;
    bool compressed = false;
    size_t i = 0;
    if (len >= 2 && s[0] == ':' && s[1] == ':') {
        compressed = true;
        i = 2;
    } else if (s[0] == ':') {
        return false;
    }
    while (i < len) {
        size_t start = i;
        while (i < len && s[i] != ':') {
            i++;
        }
        size_t glen = i - start;
        if (glen == 0) {
            return false;
        }
        /* possible IPv4 tail */
        bool has_digit = false, has_dot = false;
        for (size_t j = start; j < i; j++) {
            if (s[j] == '.') has_dot = true;
            if (s[j] >= '0' && s[j] <= '9') has_digit = true;
        }
        if (has_dot) {
            if (!has_digit) return false;
            zend_string *tail = zend_string_init(s + start, glen, 0);
            bool ok = as_is_ipv4(tail);
            zend_string_release(tail);
            if (!ok) return false;
            groups += 2;
        } else {
            if (!as_hex_group(s + start, glen)) {
                return false;
            }
            groups++;
        }
        if (i < len) {
            if (i + 1 < len && s[i + 1] == ':') {
                if (compressed) return false;
                compressed = true;
                i += 2;
                if (i == len) break; /* trailing :: */
            } else {
                i++;
                if (i == len) return false; /* trailing single colon */
            }
        }
    }
    if (compressed) {
        return groups <= 7;
    }
    return groups == 8;
}

/*
 * #[Ip]: see docs/constraints.md.
 */
bool as_validate_Ip_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_IP, field, property, errors);
        return false;
    }

    zval *version = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "version", sizeof("version") - 1, false, NULL);
    const char *v = (Z_TYPE_P(version) == IS_STRING) ? Z_STRVAL_P(version) : NULL;
    const bool want_v4 = v != NULL && strcmp(v, "v4") == 0;
    const bool want_v6 = v != NULL && strcmp(v, "v6") == 0;

    if (!want_v6 && as_is_ipv4(Z_STR_P(value))) {
        return true;
    }
    if (!want_v4 && as_is_ipv6(Z_STR_P(value))) {
        return true;
    }

    as_add_field_error_with_prefix(AS_ERROR_IP, field, property, errors);
    return false;
}
ZEND_METHOD(AS_Fields_Constraints_Ip, __construct)
{
    zend_string *version = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR_OR_NULL(version)
    ZEND_PARSE_PARAMETERS_END();

    if (version != NULL && strcmp(ZSTR_VAL(version), "v4") != 0 && strcmp(ZSTR_VAL(version), "v6") != 0) {
        zend_argument_value_error(1, "must be \"v4\", \"v6\" or null, %s given", ZSTR_VAL(version));
        RETURN_THROWS();
    }

    zval version_z;
    if (version != NULL) {
        ZVAL_STR(&version_z, version);
    } else {
        ZVAL_NULL(&version_z);
    }
    zend_update_property(AS_Fields_Constraints_Ip_ce, Z_OBJ_P(getThis()), "version", sizeof("version") - 1, &version_z);
}

void as_register_Ip_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", "Ip", class_AS_Fields_Constraints_Ip_methods);
    AS_Fields_Constraints_Ip_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AS_Fields_Constraints_Ip_ce != NULL);
    AS_Fields_Constraints_Ip_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS;

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraints_Ip_ce, 1, AS_Fields_Field_ce);

    as_declare_typed_property(AS_Fields_Constraints_Ip_ce, "version", ZEND_ACC_PUBLIC | ZEND_ACC_READONLY, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_STRING | MAY_BE_NULL));
    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AS_Fields_Constraints_Ip_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
