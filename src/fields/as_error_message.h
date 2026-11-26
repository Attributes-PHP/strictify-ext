#ifndef AS_FIELDS_ERROR_MESSAGE_H
#define AS_FIELDS_ERROR_MESSAGE_H

#include "php.h"
#include "../helpers/as_error_messages.h"

/* Class entry */
extern zend_class_entry *AS_Fields_ErrorMessage_ce;

/*
 * #[ErrorMessage] arguments: one constructor argument per default error
 * catalog key, in as_error_type enum order (so positional arguments map to the
 * enum index). Each entry is (C identifier, error type, PHP-visible name): the
 * PHP name is the camelCase property / named argument, the same string as the
 * catalog key handed to the translator (see as_error_type_names). The C
 * identifier differs from the PHP name only for `union`, a C keyword. The
 * `extra` key is discarded: extra inputs have no owning property, so the extra
 * error bypasses per-property overrides.
 */
#define AS_ERROR_MESSAGE_KEYS(X)                                  \
    X(required, AS_ERROR_REQUIRED, "required")                    \
    X(type, AS_ERROR_TYPE, "type")                                \
    X(alpha_dash, AS_ERROR_ALPHA_DASH, "alphaDash")               \
    X(alpha_dash_type, AS_ERROR_ALPHA_DASH_TYPE, "alphaDashType") \
    X(ascii, AS_ERROR_ASCII, "ascii")                             \
    X(ascii_type, AS_ERROR_ASCII_TYPE, "asciiType")               \
    X(between, AS_ERROR_BETWEEN, "between")                       \
    X(between_type, AS_ERROR_BETWEEN_TYPE, "betweenType")         \
    X(contains, AS_ERROR_CONTAINS, "contains")                    \
    X(datetime, AS_ERROR_DATETIME, "datetime")                    \
    X(ends_with, AS_ERROR_ENDS_WITH, "endsWith")                  \
    X(in, AS_ERROR_IN, "in")                                      \
    X(max, AS_ERROR_MAX, "max")                                   \
    X(min, AS_ERROR_MIN, "min")                                   \
    X(multiple_of, AS_ERROR_MULTIPLE_OF, "multipleOf")            \
    X(regex, AS_ERROR_REGEX, "regex")                             \
    X(size, AS_ERROR_SIZE, "size")                                \
    X(starts_with, AS_ERROR_STARTS_WITH, "startsWith")            \
    X(ulid, AS_ERROR_ULID, "ulid")                                \
    X(uuid, AS_ERROR_UUID, "uuid")                                \
    X(email, AS_ERROR_EMAIL, "email")                             \
    X(email_domain, AS_ERROR_EMAIL_DOMAIN, "emailDomain")         \
    X(url, AS_ERROR_URL, "url")                                   \
    X(url_protocol, AS_ERROR_URL_PROTOCOL, "urlProtocol")         \
    X(url_domain, AS_ERROR_URL_DOMAIN, "urlDomain")               \
    X(domain, AS_ERROR_DOMAIN, "domain")                          \
    X(ip, AS_ERROR_IP, "ip")                                      \
    X(iban, AS_ERROR_IBAN, "iban")                                \
    X(isbn, AS_ERROR_ISBN, "isbn")                                \
    X(credit_card, AS_ERROR_CREDIT_CARD, "creditCard")            \
    X(version, AS_ERROR_VERSION, "version")                       \
    X(mac_address, AS_ERROR_MAC_ADDRESS, "macAddress")            \
    X(alnum, AS_ERROR_ALNUM, "alnum")                             \
    X(alpha, AS_ERROR_ALPHA, "alpha")                             \
    X(digit, AS_ERROR_DIGIT, "digit")                             \
    X(xdigit, AS_ERROR_XDIGIT, "xdigit")                          \
    X(slug, AS_ERROR_SLUG, "slug")                                \
    X(base64, AS_ERROR_BASE64, "base64")                          \
    X(json, AS_ERROR_JSON, "json")                                \
    X(hex_rgb_color, AS_ERROR_HEX_RGB_COLOR, "hexRgbColor")       \
    X(even, AS_ERROR_EVEN, "even")                                \
    X(odd, AS_ERROR_ODD, "odd")                                   \
    X(positive, AS_ERROR_POSITIVE, "positive")                    \
    X(negative, AS_ERROR_NEGATIVE, "negative")                    \
    X(prime_number, AS_ERROR_PRIME_NUMBER, "primeNumber")         \
    X(contains_any, AS_ERROR_CONTAINS_ANY, "containsAny")         \
    X(union_, AS_ERROR_UNION, "union")                            \
    X(sequence_keys, AS_ERROR_SEQUENCE_KEYS, "sequenceKeys")      \
    X(unique_values, AS_ERROR_UNIQUE_VALUES, "uniqueValues")

#define AS_ERROR_MESSAGE_KEY_COUNT 49

/* Same entry as ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0), but with an
 * explicit name string so C keywords (union) can be used as keys */
#define AS_ERROR_MESSAGE_ARG_INFO(c_name, error_type, str_name) {str_name, ZEND_TYPE_INIT_CODE(IS_STRING, 0, _ZEND_ARG_INFO_FLAGS(0, 0, 0)), NULL},

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AS_Fields_ErrorMessage___construct, 0, 0, 0)
AS_ERROR_MESSAGE_KEYS(AS_ERROR_MESSAGE_ARG_INFO)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_Fields_ErrorMessage, __construct);

static const zend_function_entry class_AS_Fields_ErrorMessage_methods[] = {ZEND_ME(AS_Fields_ErrorMessage, __construct, arginfo_class_AS_Fields_ErrorMessage___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void as_register_ErrorMessage_class(void);

#endif /* AS_FIELDS_ERROR_MESSAGE_H */
