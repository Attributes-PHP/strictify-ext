#include "as_error_translator.h"
#include "helpers/as_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_types.h"

zend_class_entry *AS_ErrorTranslator_ce;

ZEND_METHOD(AS_ErrorTranslator, get)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_COPY(zend_read_static_property(AS_ErrorTranslator_ce, "translator", sizeof("translator") - 1, 1));
}

ZEND_METHOD(AS_ErrorTranslator, set)
{
    zval *translator;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(translator)
    ZEND_PARSE_PARAMETERS_END();

    if (!zend_is_callable(translator, 0, NULL)) {
        zend_throw_exception_ex(zend_ce_type_error, 0, "Argument 1 passed to Attributes\\Strictify\\ErrorTranslator::set() must be callable");
        RETURN_THROWS();
    }

    zend_update_static_property(AS_ErrorTranslator_ce, "translator", sizeof("translator") - 1, translator);

    /* Templates resolved with the previous translator are cached per property:
     * drop the cache so the new translator (e.g. a new locale) takes effect. */
    as_clear_error_templates_cache();
}

/* Registration function */
void as_register_ErrorTranslator_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify", "ErrorTranslator", class_AS_ErrorTranslator_methods);
    AS_ErrorTranslator_ce = zend_register_internal_class(&ce);
    ZEND_ASSERT(AS_ErrorTranslator_ce != NULL);
    AS_ErrorTranslator_ce->ce_flags |= ZEND_ACC_FINAL;

    /* Declare translator as a static mixed property (callable is not a valid property type) */
    zval translator_default;
    ZVAL_NULL(&translator_default);
    zend_type property_type = (zend_type)ZEND_TYPE_INIT_MASK(0);
    zend_string *property_name = zend_string_init("translator", sizeof("translator") - 1, 1);
    zend_declare_typed_property(AS_ErrorTranslator_ce, property_name, &translator_default, ZEND_ACC_PRIVATE | ZEND_ACC_STATIC, NULL, property_type);
    zend_string_release(property_name);
}
