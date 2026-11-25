#ifndef AS_FIELDS_CONSTRAINTS_PATTERN_HELPERS_H
#define AS_FIELDS_CONSTRAINTS_PATTERN_HELPERS_H

#include "php.h"
#include "Zend/zend_types.h"
#include <ext/pcre/php_pcre.h>
#include <string.h>

/* Compile-once pattern matcher: *slot holds a persistent zend_string used as
 * the PCRE cache key (the cache addrefs it; the slot itself is never freed). */
static zend_always_inline bool as_pattern_match(zend_string **slot, const char *pattern, zend_string *subject)
{
    if (*slot == NULL) {
        *slot = zend_string_init(pattern, strlen(pattern), 1);
    }
    pcre_cache_entry *pce = pcre_get_compiled_regex_cache(*slot);
    if (pce == NULL) {
        return false;
    }
    zval match;
    ZVAL_UNDEF(&match);
#if PHP_VERSION_ID >= 80400
    php_pcre_match_impl(pce, subject, &match, NULL, 0, 0, 0);
#else
    php_pcre_match_impl(pce, subject, &match, NULL, 0, 0, 0, 0);
#endif
    return Z_TYPE(match) == IS_LONG && Z_LVAL(match) > 0;
}

#endif /* AS_FIELDS_CONSTRAINTS_PATTERN_HELPERS_H */
