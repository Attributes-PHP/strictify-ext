#include "as_type.h"
#include "Zend/zend_API.h"
#include "Zend/zend_constants.h"
#include "Zend/zend_enum.h"
#include "zend_types.h"

zend_class_entry *AS_Type_ce;

static const zend_function_entry class_AS_Type_methods[] = {ZEND_FE_END};

/*
 * The enum cases and namespace constants carry the engine MAY_BE_* type
 * masks (plus the private dict bit), so comparisons against engine masks
 * need no translation.
 */
static const struct {
    const char *case_name;
    const char *const_name;
    zend_long value;
} type_cases[] = {
    {"bool", "T_BOOL", AS_TYPE_BOOL},       {"int", "T_INT", AS_TYPE_INT},          {"float", "T_FLOAT", AS_TYPE_FLOAT},
    {"string", "T_STRING", AS_TYPE_STRING}, {"null", "T_NULL", AS_TYPE_NULL},       {"sequence", "T_SEQUENCE", AS_TYPE_SEQUENCE},
    {"dict", "T_DICT", AS_TYPE_DICT},       {"object", "T_OBJECT", AS_TYPE_OBJECT},
};

#define TYPE_CASES_COUNT (sizeof(type_cases) / sizeof(type_cases[0]))

void as_register_Type_enum(void)
{
    AS_Type_ce = zend_register_internal_enum("Attributes\\Strictify\\Type", IS_LONG, class_AS_Type_methods);
    ZEND_ASSERT(AS_Type_ce != NULL);

    for (size_t i = 0; i < TYPE_CASES_COUNT; i++) {
        zval case_value;
        ZVAL_LONG(&case_value, type_cases[i].value);
        zend_enum_add_case_cstr(AS_Type_ce, type_cases[i].case_name, &case_value);
    }
}

/*
 * Registers the Attributes\\Strictify\{T_BOOL,T_INT,T_FLOAT,T_STRING,T_NULL,T_SEQUENCE,T_DICT,T_OBJECT}
 * constants holding the matching Type case values, so attribute arguments
 * can use them (#[Union(T_BOOL, T_INT, T_STRING)]) and combine them with
 * the bitwise OR operator (#[Union(T_BOOL | T_INT | T_STRING)]) through
 * const imports. The T_ prefix keeps the names clear of the reserved
 * type-hint keywords (bool, int, float, string, object) they would
 * otherwise collide with. CONST_CS keeps the exact spelling significant.
 */
void as_register_type_constants(int module_number)
{
    // The REGISTER_NS_LONG_CONSTANT macro only accepts literal names, so the
    // "Attributes\\Strictify\<constant>" names are composed by hand here
    static const char namespace_prefix[] = "Attributes\\Strictify\\";
    char name[sizeof(namespace_prefix) + 16];

    for (size_t i = 0; i < TYPE_CASES_COUNT; i++) {
        memcpy(name, namespace_prefix, sizeof(namespace_prefix) - 1);
        strcpy(name + sizeof(namespace_prefix) - 1, type_cases[i].const_name);
        zend_register_long_constant(name, strlen(name), type_cases[i].value, CONST_CS | CONST_PERSISTENT, module_number);
    }
}
