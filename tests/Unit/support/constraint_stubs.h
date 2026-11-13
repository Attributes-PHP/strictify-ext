#ifndef CONSTRAINT_STUBS_H
#define CONSTRAINT_STUBS_H

#include "php.h"
#include "Zend/zend_types.h"

/*
 * Shared harness for constraint unit tests.
 *
 * Constraint objects in tests are plain zvals; their "properties" are
 * registered by name in a small map backing the zend_read_property stub.
 */
/* Resets the map and registers a single property */
void stub_set_property(const char *name, zval *value);
void stub_clear_properties(void);
/* Appends one property (call stub_clear_properties() first for a fresh map) */
void stub_add_property(const char *name, zval *value);

/* Controllable results for engine stubs */
extern bool stub_instanceof_result;
extern void *stub_pcre_cache; /* NULL = pattern fails to compile */
extern zend_long stub_pcre_match_result;
extern bool stub_date_initialize_result;
extern const char *stub_date_format_seen;
extern const char *stub_date_time_seen;

/* Value builders (heap-allocated, caller owns) */
zval *stub_make_string(const char *str);
zval *stub_make_long(zend_long value);
zval *stub_make_double(double value);
/* Dummy object zval to stand in for the constraint instance */
zval *stub_make_constraint(void);
/* Packed array zval copying the given items */
void stub_make_array(zval *arr, const zval *items, uint32_t count);

/* CMock stub callbacks: a release that frees nothing (mocked
 * as_value_to_string/as_values_to_list_string return the shared static
 * string) and a concat3 that always returns that same static string, so
 * tests can assert the exact pointer handed to as_add_field_error_with_expected */
extern zend_string *stub_expected_string;
void stub_init_expected_string(void);
void stub_release_noop(zend_string *s, int num_calls);
zend_string *stub_concat3_static(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len, int num_calls);

#endif /* CONSTRAINT_STUBS_H */
