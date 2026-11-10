/*
 * Mockable wrappers for Zend internals and macros to ease unit testing.
 *
 * In production builds (TESTING not defined), simple passthrough functions are
 * implemented as macros that directly call their Zend equivalents for zero overhead.
 * In test builds (TESTING defined), they are real functions that can be mocked
 * using CMock.
 *
 * Functions with actual logic (not just passthrough) remain as real functions
 * in both modes.
 */

#ifndef AS_HELPERS_AS_WRAPPERS_H
#define AS_HELPERS_AS_WRAPPERS_H

#include <Zend/zend_types.h>

/*
 * =============================================================================
 * SIMPLE PASS-THROUGH WRAPPERS
 * =============================================================================
 * These are implemented as macros in production for zero overhead, and as
 * functions during testing for mockability.
 */

#ifndef TESTING
/* Production mode: macros that directly call Zend functions */

#define as_string_init(str, len, persistent)                              zend_string_init(str, len, persistent)
#define as_string_release(s)                                              zend_string_release(s)
#define as_emalloc(size)                                                  emalloc(size)
#define as_efree(ptr)                                                     efree(ptr)
#define as_string_alloc(length, persistent)                               zend_string_alloc(length, persistent)
#define as_string_truncate(s, length, persistent)                         zend_string_truncate(s, length, persistent)
#define as_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len) zend_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len)
#define as_string_copy(s)                                                 zend_string_copy(s)
#define as_long_to_str(num)                                               zend_long_to_str(num)
#define as_double_to_str(num)                                             zend_double_to_str(num)
#define as_is_stringable(instance_ce)                                     instanceof_function(instance_ce, zend_ce_stringable)
#define as_rsrc_list_get_rsrc_type(res)                                   zend_rsrc_list_get_rsrc_type(res)
#define as_zval_ptr_dtor(zval_ptr)                                        zval_ptr_dtor(zval_ptr)
#define as_memnstr(haystack, needle, needle_len, end)                     php_memnstr(haystack, needle, needle_len, end)
#define as_hash_find(ht, key)                                             zend_hash_find(ht, key)
#define as_hash_next_index_insert(ht, pData)                              zend_hash_next_index_insert(ht, pData)
#define as_hash_add(ht, key, pData)                                       zend_hash_add(ht, key, pData)
#define as_new_array(size)                                                zend_new_array(size)
#define as_lookup_class_ex(name, lcname, flags)                           zend_lookup_class_ex(name, lcname, flags)
#define as_update_class_constants(ce)                                     zend_update_class_constants(ce)
#define as_zval_update_constant_ex(zv, scope)                             zval_update_constant_ex(zv, scope)
#define as_get_attribute_str(attributes, str, len)                        zend_get_attribute_str(attributes, str, len)
#define as_get_attribute_value(ret, attribute, arg_num, scope)            zend_get_attribute_value(ret, attribute, arg_num, scope)
#define as_throw_value_error(message)                                     zend_throw_exception_ex(zend_ce_value_error, 0, "%s", message)
#define as_binary_strcasecmp(s1, len1, s2, len2)                          zend_binary_strcasecmp(s1, len1, s2, len2)
#define as_hash_init(ht, size, dtor, persistent)                          zend_hash_init(ht, size, NULL, dtor, persistent)
#define as_hash_destroy(ht)                                               zend_hash_destroy(ht)
#define as_hash_index_find_ptr(ht, h)                                     zend_hash_index_find_ptr(ht, h)
#define as_hash_index_add_ptr(ht, h, data)                                zend_hash_index_add_ptr(ht, h, data)
#define as_has_exception()                                                (EG(exception) != NULL)

#else
/* Testing mode: function declarations for CMock */

zend_string *as_string_init(const char *str, size_t len, bool persistent);
void as_string_release(zend_string *s);
void *as_emalloc(size_t size);
void as_efree(void *ptr);
zend_string *as_string_alloc(size_t length, bool persistent);
zend_string *as_string_truncate(zend_string *s, size_t length, bool persistent);

/* Additional wrappers used by as_value_to_string to allow mocking in unit tests. */
zend_string *as_string_concat3(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len);
zend_string *as_string_copy(zend_string *s);
zend_string *as_long_to_str(zend_long num);
zend_string *as_double_to_str(double num);
bool as_is_stringable(const zend_class_entry *instance_ce);
const char *as_rsrc_list_get_rsrc_type(zend_resource *res);
void as_zval_ptr_dtor(zval *zval_ptr);

/* Additional wrappers used by as_replace_placeholders to allow mocking in unit tests. */
const char *as_memnstr(const char *haystack, const char *needle, size_t needle_len, const char *end);

/* Additional wrappers used by as_error_messages to keep the unit-test build
 * free of unresolved Zend/library symbols. Every Zend Hash Table, class lookup
 * and constant evaluation call in as_error_messages.c goes through these so
 * the whole translation unit can be linked into Ceedling tests. */
zval *as_hash_find(const HashTable *ht, zend_string *key);
zval *as_hash_next_index_insert(HashTable *ht, zval *pData);
zval *as_hash_add(HashTable *ht, zend_string *key, zval *pData);
HashTable *as_new_array(uint32_t size);
zend_class_entry *as_lookup_class_ex(zend_string *name, zend_string *lcname, uint32_t flags);
zend_result as_update_class_constants(zend_class_entry *ce);
zend_result as_zval_update_constant_ex(zval *zv, zend_class_entry *scope);

/* Additional wrappers used by the error message template logic to read
 * #[ErrorMessage] attributes and to throw exceptions without pulling real
 * Zend symbols into the unit-test build. The attribute pointer is passed as
 * void * so the mockable declarations stay free of Zend attribute types. */
void *as_get_attribute_str(HashTable *attributes, const char *str, size_t len);
zend_result as_get_attribute_value(zval *ret, void *attribute, uint32_t arg_num, zend_class_entry *scope);
void as_throw_value_error(const char *message);
int as_binary_strcasecmp(const char *s1, size_t len1, const char *s2, size_t len2);

/* Additional wrappers used by the per-property error templates cache in
 * as_error_messages.c, so its hash table operations stay mockable and the
 * unit-test build remains free of unresolved Zend/library symbols. */
void as_hash_init(HashTable *ht, uint32_t size, dtor_func_t dtor, bool persistent);
void as_hash_destroy(HashTable *ht);
void *as_hash_index_find_ptr(HashTable *ht, zend_ulong h);
void *as_hash_index_add_ptr(HashTable *ht, zend_ulong h, void *data);
bool as_has_exception(void);

#endif /* TESTING */

/*
 * =============================================================================
 * FUNCTIONS WITH LOGIC
 * =============================================================================
 * These always remain as real functions because they contain actual logic,
 * not just passthrough to Zend functions.
 */

zend_result as_call_tostring(zend_object *object, zval *retval);
void as_zval_stringl(zval *z, const char *str, size_t len);
void as_declare_typed_property(zend_class_entry *ce, const char *name, int flags, zend_type type);

#endif /* AS_HELPERS_AS_WRAPPERS_H */
