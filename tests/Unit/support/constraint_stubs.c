#include "constraint_stubs.h"
#include "Zend/zend_API.h"
#include "Zend/zend_hash.h"
#include "Zend/zend_operators.h"
#include "Zend/zend_globals.h"
#include <ext/pcre/php_pcre.h>
#include <ext/date/php_date.h>
#include <stdlib.h>
#include <string.h>

/* Several Zend entry points are macros in some build configurations
 * (bin-allocator specialization, inline wrappers). Undef them all so the
 * stub definitions below bind to the real symbols. */
#undef _emalloc
#undef _efree
#undef __zend_malloc
#undef zend_new_array
#undef zend_read_property
#undef zend_register_internal_class_ex
#undef zend_class_implements
#undef zend_internal_attribute_register
#undef zend_declare_typed_property
#undef zend_update_property
#undef zend_update_property_long
#undef zend_update_property_null
#undef zend_update_property_string
#undef zend_parse_parameters
#undef zend_argument_type_error
#undef zend_argument_value_error
#undef zend_zval_type_name
#undef zend_hash_next_index_insert
#undef zval_ptr_dtor
#undef object_init_ex
#undef zend_clear_exception
#undef zend_is_identical
#undef zval_get_string_func
#undef instanceof_function_slow
#undef pcre_get_compiled_regex_cache
#undef php_pcre_match_impl
#undef php_date_get_date_ce
#undef php_date_get_interface_ce
#undef php_date_get_timezone_ce
#undef php_date_instantiate
#undef php_date_initialize
#undef femalloc
#undef _femalloc
#undef __femalloc

/*
 * Link-time stubs for the raw Zend/extension symbols referenced by the
 * constraint translation units (registration functions, property access,
 * PCRE and date helpers). The validate functions under test never call the
 * registration/constructor paths; the stubs exist so the test executables
 * link without a Zend engine. Signatures must match the installed headers
 * exactly (several of these are declared with const / different arg types).
 */

/* as_field.c is not linked into constraint tests; its class entry global is
 * only referenced by the registration functions. */
zend_class_entry *AS_Fields_Field_ce = NULL;

/* NTS executor globals, referenced through EG() in as_datetime.c */
zend_executor_globals executor_globals;

bool stub_instanceof_result = false;
void *stub_pcre_cache = NULL;
zend_long stub_pcre_match_result = 0;
bool stub_date_initialize_result = false;
const char *stub_date_format_seen = NULL;
const char *stub_date_time_seen = NULL;

// ---------------------------------------------------------------------------
// Property map backing zend_read_property
// ---------------------------------------------------------------------------

typedef struct {
    char name[64];
    zval *value;
} stub_property;

static stub_property stub_properties[16];
static uint32_t stub_properties_count = 0;
static zval stub_null_zval;

void stub_set_property(const char *name, zval *value)
{
    stub_clear_properties();
    stub_add_property(name, value);
}

void stub_clear_properties(void)
{
    stub_properties_count = 0;
}

void stub_add_property(const char *name, zval *value)
{
    if (stub_properties_count >= 16) {
        return;
    }
    snprintf(stub_properties[stub_properties_count].name, sizeof(stub_properties[0].name), "%s", name);
    stub_properties[stub_properties_count].value = value;
    stub_properties_count++;
}

zval *zend_read_property(zend_class_entry *scope, zend_object *object, const char *name, size_t name_length, bool silent, zval *rv)
{
    (void)scope;
    (void)object;
    (void)silent;
    (void)rv;
    for (uint32_t i = 0; i < stub_properties_count; i++) {
        if (strlen(stub_properties[i].name) == name_length && memcmp(stub_properties[i].name, name, name_length) == 0) {
            return stub_properties[i].value;
        }
    }
    ZVAL_NULL(&stub_null_zval);
    return &stub_null_zval;
}

// ---------------------------------------------------------------------------
// Registration / class-entry stubs (never called by the tests)
// ---------------------------------------------------------------------------

zend_class_entry *zend_register_internal_class_ex(zend_class_entry *class_entry, zend_class_entry *parent_ce)
{
    (void)class_entry;
    (void)parent_ce;
    return NULL;
}

void zend_class_implements(zend_class_entry *class_entry, int num_interfaces, ...)
{
    (void)class_entry;
    (void)num_interfaces;
}

void zend_internal_attribute_register(zend_class_entry *ce, uint32_t flags)
{
    (void)ce;
    (void)flags;
}

zend_property_info *zend_declare_typed_property(zend_class_entry *ce, zend_string *name, zval *property, int access_type, zend_string *doc_comment, zend_type type)
{
    (void)ce;
    (void)name;
    (void)property;
    (void)access_type;
    (void)doc_comment;
    (void)type;
    return NULL;
}

/* PHP 8.5 constified the scope parameter of the zend_update_property*
 * family; the stubs must match the headers of the PHP version under test. */
#if PHP_VERSION_ID >= 80500
#define AS_STUB_UPDATE_PROPERTY_SCOPE const zend_class_entry
#else
#define AS_STUB_UPDATE_PROPERTY_SCOPE zend_class_entry
#endif

/* PHP 8.3 constified the zval parameter of the slow-path argument parsers
 * (bool/long/double) and of zend_is_identical. */
#if PHP_VERSION_ID >= 80300
#define AS_STUB_CONST_ZVAL const zval
#else
#define AS_STUB_CONST_ZVAL zval
#endif

void zend_update_property(AS_STUB_UPDATE_PROPERTY_SCOPE *scope, zend_object *object, const char *name, size_t name_length, zval *value)
{
    (void)scope;
    (void)object;
    (void)name;
    (void)name_length;
    (void)value;
}

void zend_update_property_long(AS_STUB_UPDATE_PROPERTY_SCOPE *scope, zend_object *object, const char *name, size_t name_length, zend_long value)
{
    (void)scope;
    (void)object;
    (void)name;
    (void)name_length;
    (void)value;
}

void zend_update_property_null(AS_STUB_UPDATE_PROPERTY_SCOPE *scope, zend_object *object, const char *name, size_t name_length)
{
    (void)scope;
    (void)object;
    (void)name;
    (void)name_length;
}

void zend_update_property_string(AS_STUB_UPDATE_PROPERTY_SCOPE *scope, zend_object *object, const char *name, size_t name_length, const char *value)
{
    (void)scope;
    (void)object;
    (void)name;
    (void)name_length;
    (void)value;
}

zend_result zend_parse_parameters(uint32_t num_args, const char *type_spec, ...)
{
    (void)num_args;
    (void)type_spec;
    return SUCCESS;
}

/* Slow paths of the ZEND_PARSE_PARAMETERS macros, referenced by the
 * __construct methods (never called by the tests) */
bool zend_parse_arg_bool_slow(AS_STUB_CONST_ZVAL *arg, bool *dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_parse_arg_long_slow(AS_STUB_CONST_ZVAL *arg, zend_long *dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_parse_arg_double_slow(AS_STUB_CONST_ZVAL *arg, double *dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_parse_arg_str_slow(zval *arg, zend_string **dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_parse_arg_number_slow(zval *arg, zval **dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_parse_arg_number_or_str_slow(zval *arg, zval **dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_parse_arg_str_or_long_slow(zval *arg, zend_string **dest_str, zend_long *dest_long, uint32_t arg_num)
{
    (void)arg;
    (void)dest_str;
    (void)dest_long;
    (void)arg_num;
    return false;
}

void zend_wrong_parameters_count_error(uint32_t min_num_args, uint32_t max_num_args)
{
    (void)min_num_args;
    (void)max_num_args;
}

void zend_wrong_parameter_error(int error_code, uint32_t num, char *name, zend_expected_type expected_type, zval *arg)
{
    (void)error_code;
    (void)num;
    (void)name;
    (void)expected_type;
    (void)arg;
}

bool zend_flf_parse_arg_bool_slow(const zval *arg, bool *dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_flf_parse_arg_str_slow(zval *arg, zend_string **dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

bool zend_flf_parse_arg_long_slow(const zval *arg, zend_long *dest, uint32_t arg_num)
{
    (void)arg;
    (void)dest;
    (void)arg_num;
    return false;
}

void zend_call_known_function(zend_function *fn, zend_object *object, zend_class_entry *called_scope, zval *retval_ptr, uint32_t param_count, zval *params, HashTable *named_params)
{
    (void)fn;
    (void)object;
    (void)called_scope;
    (void)retval_ptr;
    (void)param_count;
    (void)params;
    (void)named_params;
}

zval *zend_hash_str_find(const HashTable *ht, const char *key, size_t len)
{
    (void)ht;
    (void)key;
    (void)len;
    return NULL;
}

void zend_argument_type_error(uint32_t arg_num, const char *format, ...)
{
    (void)arg_num;
    (void)format;
}

void zend_argument_value_error(uint32_t arg_num, const char *format, ...)
{
    (void)arg_num;
    (void)format;
}

const char *zend_zval_type_name(const zval *zv)
{
    switch (Z_TYPE_P(zv)) {
        case IS_STRING:
            return "string";
        case IS_LONG:
            return "int";
        case IS_DOUBLE:
            return "float";
        case IS_TRUE:
        case IS_FALSE:
            return "bool";
        case IS_NULL:
            return "null";
        case IS_ARRAY:
            return "array";
        case IS_OBJECT:
            return "object";
        default:
            return "unknown";
    }
}

// ---------------------------------------------------------------------------
// Allocation stubs (inline zend_string_init/zend_string_release and
// pemalloc end up in _emalloc/_efree/__zend_malloc in non-debug builds)
// ---------------------------------------------------------------------------

void *_emalloc(size_t size)
{
    return malloc(size);
}

void _efree(void *ptr)
{
    free(ptr);
}

void *__zend_malloc(size_t size)
{
    return malloc(size);
}

void *_femalloc(size_t size)
{
    return malloc(size);
}

/* Referenced by INIT_CLASS_ENTRY macros and object-handle comparisons */
const zend_object_handlers std_object_handlers;

static zend_string *stub_string_init_interned(const char *str, size_t size, bool permanent)
{
    (void)permanent;
    return zend_string_init(str, size, 0);
}

zend_string_init_interned_func_t zend_string_init_interned = stub_string_init_interned;

/* zend_new_array is a macro over this real symbol */
HashTable *_zend_new_array(uint32_t size)
{
    (void)size;
    return calloc(1, sizeof(HashTable));
}

zval *zend_hash_next_index_insert(HashTable *ht, zval *pData)
{
    (void)ht;
    return pData;
}

void zval_ptr_dtor(zval *zval_ptr)
{
    (void)zval_ptr;
}

zend_result object_init_ex(zval *arg, zend_class_entry *ce)
{
    (void)ce;
    ZVAL_NULL(arg);
    return SUCCESS;
}

void zend_clear_exception(void)
{}

// ---------------------------------------------------------------------------
// Comparison / conversion stubs used by the validate paths
// ---------------------------------------------------------------------------

bool zend_is_identical(AS_STUB_CONST_ZVAL *op1, AS_STUB_CONST_ZVAL *op2)
{
    if (Z_TYPE_P(op1) != Z_TYPE_P(op2)) {
        return false;
    }
    switch (Z_TYPE_P(op1)) {
        case IS_LONG:
            return Z_LVAL_P(op1) == Z_LVAL_P(op2);
        case IS_DOUBLE:
            return Z_DVAL_P(op1) == Z_DVAL_P(op2);
        case IS_STRING:
            return Z_STRLEN_P(op1) == Z_STRLEN_P(op2) && memcmp(Z_STRVAL_P(op1), Z_STRVAL_P(op2), Z_STRLEN_P(op1)) == 0;
        case IS_TRUE:
        case IS_FALSE:
        case IS_NULL:
            return true;
        default:
            return false;
    }
}

/* Called by the inline zval_get_string() for non-string values */
zend_string *zval_get_string_func(zval *op)
{
    char buf[64];
    switch (Z_TYPE_P(op)) {
        case IS_LONG: {
            int len = snprintf(buf, sizeof(buf), "%ld", (long)Z_LVAL_P(op));
            return zend_string_init(buf, (size_t)len, 0);
        }
        case IS_DOUBLE: {
            int len = snprintf(buf, sizeof(buf), "%.14g", Z_DVAL_P(op));
            return zend_string_init(buf, (size_t)len, 0);
        }
        case IS_TRUE:
            return zend_string_init("1", 1, 0);
        case IS_FALSE:
            return zend_string_init("", 0, 0);
        default:
            return zend_string_init("", 0, 0);
    }
}

bool instanceof_function_slow(const zend_class_entry *instance_ce, const zend_class_entry *ce)
{
    (void)instance_ce;
    (void)ce;
    return stub_instanceof_result;
}

// ---------------------------------------------------------------------------
// PCRE stubs
// ---------------------------------------------------------------------------

pcre_cache_entry *pcre_get_compiled_regex_cache(zend_string *regex)
{
    (void)regex;
    return (pcre_cache_entry *)stub_pcre_cache;
}

/* PHP 8.4 removed the use_flags argument and changed global/flags to
 * bool/zend_long; the stub must match the headers of the PHP under test. */
#if PHP_VERSION_ID >= 80400
void php_pcre_match_impl(pcre_cache_entry *pce, zend_string *subject_str, zval *return_value, zval *subpats, bool global, zend_long flags, zend_off_t start_offset)
#else
void php_pcre_match_impl(pcre_cache_entry *pce, zend_string *subject_str, zval *return_value, zval *subpats, int global, int use_flags, zend_long flags, zend_off_t start_offset)
#endif
{
    (void)pce;
    (void)subject_str;
    (void)subpats;
    (void)global;
#if PHP_VERSION_ID < 80400
    (void)use_flags;
#endif
    (void)flags;
    (void)start_offset;
    ZVAL_LONG(return_value, stub_pcre_match_result);
}

// ---------------------------------------------------------------------------
// Date extension stubs
// ---------------------------------------------------------------------------

static zend_class_entry stub_date_ce;

zend_class_entry *php_date_get_date_ce(void)
{
    return &stub_date_ce;
}

zend_class_entry *php_date_get_interface_ce(void)
{
    return &stub_date_ce;
}

zend_class_entry *php_date_get_timezone_ce(void)
{
    return &stub_date_ce;
}

zval *php_date_instantiate(zend_class_entry *pce, zval *object)
{
    (void)pce;
    ZVAL_OBJ(object, calloc(1, sizeof(zend_object)));
    return object;
}

bool php_date_initialize(php_date_obj *dateobj, const char *time_str, size_t time_str_len, const char *format, zval *timezone_object, int flags)
{
    (void)dateobj;
    (void)time_str_len;
    (void)timezone_object;
    (void)flags;
    stub_date_time_seen = time_str;
    stub_date_format_seen = format;
    return stub_date_initialize_result;
}

// ---------------------------------------------------------------------------
// Value builders
// ---------------------------------------------------------------------------

zval *stub_make_string(const char *str)
{
    zval *zv = malloc(sizeof(zval));
    ZVAL_STR(zv, zend_string_init(str, strlen(str), 0));
    return zv;
}

zval *stub_make_long(zend_long value)
{
    zval *zv = malloc(sizeof(zval));
    ZVAL_LONG(zv, value);
    return zv;
}

zval *stub_make_double(double value)
{
    zval *zv = malloc(sizeof(zval));
    ZVAL_DOUBLE(zv, value);
    return zv;
}

void stub_make_array(zval *arr, const zval *items, uint32_t count)
{
    HashTable *ht = calloc(1, sizeof(HashTable));
    /* Packed arrays store contiguous zvals (arPacked), not Buckets */
    zval *packed = calloc(count > 0 ? count : 1, sizeof(zval));
    for (uint32_t i = 0; i < count; i++) {
        packed[i] = items[i];
    }
    ht->arPacked = packed;
    ht->nNumUsed = count;
    ht->nNumOfElements = count;
    ht->nTableMask = HT_MIN_MASK;
    ht->u.flags = HASH_FLAG_PACKED;
    ZVAL_ARR(arr, ht);
}

zval *stub_make_constraint(void)
{
    zval *zv = malloc(sizeof(zval));
    ZVAL_OBJ(zv, calloc(1, sizeof(zend_object)));
    return zv;
}

// ---------------------------------------------------------------------------
// CMock stub callbacks for the error-message path
// ---------------------------------------------------------------------------

zend_string *stub_expected_string = NULL;

void stub_init_expected_string(void)
{
    if (stub_expected_string == NULL) {
        stub_expected_string = zend_string_init("EXPECTED", 8, 0);
    }
}

void stub_release_noop(zend_string *s, int num_calls)
{
    (void)s;
    (void)num_calls;
}

zend_string *stub_concat3_static(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len, int num_calls)
{
    (void)str1;
    (void)str1_len;
    (void)str2;
    (void)str2_len;
    (void)str3;
    (void)str3_len;
    (void)num_calls;
    stub_init_expected_string();
    return stub_expected_string;
}
