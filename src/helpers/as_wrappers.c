#include "as_wrappers.h"
#include "../as_model_configs.h"
#include "php.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_hash.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_list.h"
#include "Zend/zend_operators.h"
#include "Zend/zend_variables.h"

/*
 * Wrapper implementations for Zend internals.
 *
 * Simple passthrough functions are only compiled in TESTING mode.
 * In production, they are macros defined in as_wrappers.h.
 *
 * Functions with actual logic remain as real functions in both modes.
 */

#ifdef TESTING

/* Simple passthrough wrappers - only needed for testing/mocking */

zend_string *as_string_init(const char *str, size_t len, bool persistent)
{
    return zend_string_init(str, len, persistent);
}

void as_string_release(zend_string *s)
{
    zend_string_release(s);
}

void *as_emalloc(size_t size)
{
    return emalloc(size);
}

void as_efree(void *ptr)
{
    efree(ptr);
}

zend_string *as_string_alloc(size_t length, bool persistent)
{
    return zend_string_alloc(length, persistent);
}

zend_string *as_string_truncate(zend_string *s, size_t length, bool persistent)
{
    return zend_string_truncate(s, length, persistent);
}

zend_string *as_string_concat3(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len)
{
    return zend_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len);
}

zend_string *as_string_copy(zend_string *s)
{
    return zend_string_copy(s);
}

zend_string *as_long_to_str(zend_long num)
{
    return zend_long_to_str(num);
}

zend_string *as_double_to_str(double num)
{
    return zend_double_to_str(num);
}

bool as_is_stringable(const zend_class_entry *instance_ce)
{
    return instanceof_function(instance_ce, zend_ce_stringable);
}

const char *as_rsrc_list_get_rsrc_type(zend_resource *res)
{
    return zend_rsrc_list_get_rsrc_type(res);
}

void as_zval_ptr_dtor(zval *zval_ptr)
{
    zval_ptr_dtor(zval_ptr);
}

const char *as_memnstr(const char *haystack, const char *needle, size_t needle_len, const char *end)
{
    return php_memnstr(haystack, needle, needle_len, end);
}

zval *as_hash_find(const HashTable *ht, zend_string *key)
{
    return zend_hash_find(ht, key);
}

zval *as_hash_next_index_insert(HashTable *ht, zval *pData)
{
    return zend_hash_next_index_insert(ht, pData);
}

zval *as_hash_add(HashTable *ht, zend_string *key, zval *pData)
{
    return zend_hash_add(ht, key, pData);
}

HashTable *as_new_array(uint32_t size)
{
    return zend_new_array(size);
}

zend_class_entry *as_lookup_class_ex(zend_string *name, zend_string *lcname, uint32_t flags)
{
    return zend_lookup_class_ex(name, lcname, flags);
}

zend_result as_update_class_constants(zend_class_entry *ce)
{
    return zend_update_class_constants(ce);
}

zend_result as_zval_update_constant_ex(zval *zv, zend_class_entry *scope)
{
    return zval_update_constant_ex(zv, scope);
}

void *as_get_attribute_str(HashTable *attributes, const char *str, size_t len)
{
    return zend_get_attribute_str(attributes, str, len);
}

zend_result as_get_attribute_value(zval *ret, void *attribute, uint32_t arg_num, zend_class_entry *scope)
{
    return zend_get_attribute_value(ret, (const zend_attribute *)attribute, arg_num, scope);
}

void as_throw_value_error(const char *message)
{
    zend_throw_exception_ex(zend_ce_value_error, 0, "%s", message);
}

int as_binary_strcasecmp(const char *s1, size_t len1, const char *s2, size_t len2)
{
    return zend_binary_strcasecmp(s1, len1, s2, len2);
}

#endif /* TESTING */

/*
 * =============================================================================
 * FUNCTIONS WITH LOGIC
 * =============================================================================
 * These always remain as real functions because they contain actual logic,
 * not just passthrough to Zend functions. They are needed in both modes.
 */

zend_result as_call_tostring(zend_object *object, zval *retval)
{
    ZVAL_UNDEF(retval);
    zend_call_method_with_0_params(object, NULL, NULL, "__tostring", retval);
    if (Z_TYPE_P(retval) != IS_STRING) {
        if (Z_TYPE_P(retval) != IS_UNDEF) {
            zval_ptr_dtor(retval);
            ZVAL_UNDEF(retval);
        }
        return FAILURE;
    }
    return SUCCESS;
}

void as_zval_stringl(zval *z, const char *str, size_t len)
{
    ZVAL_NEW_STR(z, as_string_init(str, len, 0));
}

void as_declare_typed_property(zend_class_entry *ce, const char *name, int flags, zend_type type)
{
    zval default_value;
    ZVAL_UNDEF(&default_value);
    zend_string *name_str = zend_string_init(name, strlen(name), 1);
    zend_declare_typed_property(ce, name_str, &default_value, flags, NULL, type);
    zend_string_release(name_str);
}

zend_string *as_translate_error_key(const char *key, size_t key_len)
{
    zval *translator = zend_read_static_property(AS_ModelConfigs_ce, "defaultErrorMessages", sizeof("defaultErrorMessages") - 1, true);
    if (translator == NULL || Z_TYPE_P(translator) == IS_NULL || !zend_is_callable(translator, 0, NULL)) {
        return NULL;
    }

    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    char *error = NULL;
    if (zend_fcall_info_init(translator, 0, &fci, &fcc, NULL, &error) != SUCCESS) {
        if (error != NULL) {
            zend_throw_exception(zend_ce_type_error, error, 0);
            efree(error);
        }
        return NULL;
    }

    zval key_zv;
    ZVAL_STRINGL(&key_zv, key, key_len);

    zval retval;
    ZVAL_UNDEF(&retval);

    fci.size = sizeof(fci);
    fci.object = fcc.object;
    fci.retval = &retval;
    fci.param_count = 1;
    fci.params = &key_zv;
    fci.named_params = NULL;

    zend_result call_result = zend_call_function(&fci, &fcc);
    zval_ptr_dtor(&key_zv);

    if (call_result != SUCCESS || UNEXPECTED(EG(exception) != NULL)) {
        if (Z_TYPE(retval) != IS_UNDEF) {
            zval_ptr_dtor(&retval);
        }
        return NULL;
    }

    if (Z_TYPE(retval) == IS_NULL) {
        return NULL;
    }

    if (Z_TYPE(retval) == IS_STRING) {
        zend_string *translated = zend_string_copy(Z_STR(retval));
        zval_ptr_dtor(&retval);
        return translated;
    }

    zval_ptr_dtor(&retval);
    as_throw_value_error("The default error messages translator must return ?string");
    return NULL;
}
