#ifndef AS_MODEL_CONFIG_H
#define AS_MODEL_CONFIG_H

#include "php.h"
#include "Zend/zend_attributes.h"

/* Class entry */
extern zend_class_entry *AS_ModelConfigs_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_getAliasGenerator, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_isStrToLower, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_isStrToUpper, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_isStripWhitespace, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_getExtra, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_isStrict, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_isStopAtFirstError, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AS_ModelConfigs_setDefaultErrorMessages, 0, 1, IS_VOID, 0)
ZEND_ARG_TYPE_INFO(0, setErrorMessages, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AS_ModelConfigs, getAliasGenerator);
ZEND_METHOD(AS_ModelConfigs, isStrToLower);
ZEND_METHOD(AS_ModelConfigs, isStrToUpper);
ZEND_METHOD(AS_ModelConfigs, isStripWhitespace);
ZEND_METHOD(AS_ModelConfigs, getExtra);
ZEND_METHOD(AS_ModelConfigs, isStrict);
ZEND_METHOD(AS_ModelConfigs, isStopAtFirstError);
ZEND_METHOD(AS_ModelConfigs, setDefaultErrorMessages);

static const zend_function_entry class_AS_ModelConfigs_methods[] = {
    ZEND_ME(AS_ModelConfigs, getAliasGenerator, arginfo_class_AS_ModelConfigs_getAliasGenerator, ZEND_ACC_PUBLIC) ZEND_ME(AS_ModelConfigs, isStrToLower, arginfo_class_AS_ModelConfigs_isStrToLower, ZEND_ACC_PUBLIC)
        ZEND_ME(AS_ModelConfigs, isStrToUpper, arginfo_class_AS_ModelConfigs_isStrToUpper, ZEND_ACC_PUBLIC) ZEND_ME(AS_ModelConfigs, isStripWhitespace, arginfo_class_AS_ModelConfigs_isStripWhitespace, ZEND_ACC_PUBLIC)
            ZEND_ME(AS_ModelConfigs, getExtra, arginfo_class_AS_ModelConfigs_getExtra, ZEND_ACC_PUBLIC) ZEND_ME(AS_ModelConfigs, isStrict, arginfo_class_AS_ModelConfigs_isStrict, ZEND_ACC_PUBLIC)
                ZEND_ME(AS_ModelConfigs, isStopAtFirstError, arginfo_class_AS_ModelConfigs_isStopAtFirstError, ZEND_ACC_PUBLIC)
                    ZEND_ME(AS_ModelConfigs, setDefaultErrorMessages, arginfo_class_AS_ModelConfigs_setDefaultErrorMessages, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) ZEND_FE_END
};

// Alias
#define AS_PASCAL_CASE 'p'
#define AS_CAMEL_CASE  'c'
#define AS_SNAKE_CASE  's'
#define AS_KEBAB_CASE  'k'

// Extra
#define AS_IGNORE 'i'
#define AS_FORBID 'f'
#define AS_ALLOW  'a'

typedef struct {
    char alias_generator;
    bool str_to_lower;
    bool str_to_upper;
    bool strip_whitespace;
    char extra;
    bool strict;
    bool stop_first_error;
} as_model_configs_properties;

/* Registration function */
void as_register_ModelConfigs_class(void);
void as_get_model_configs(zval *configs, zval *model, as_model_configs_properties *properties, bool build_object);
void as_clear_model_configs_cache(void);

#endif /* AS_MODEL_CONFIG_H */
