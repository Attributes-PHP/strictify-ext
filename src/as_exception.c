#include "as_exception.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_types.h"

zend_class_entry *class_AS_Exceptions_StrictifyThrowable;
zend_class_entry *class_AS_Exceptions_ValidationException;
zend_class_entry *class_AS_Exceptions_InvalidBaseModelException;
zend_class_entry *class_AS_Exceptions_MaxDepthException;
zend_class_entry *class_AS_Exceptions_CallCountException;

static void register_StrictifyThrowable_interface(void);
static void register_ValidationException_class(void);
static void register_InvalidBaseModelException_class(void);
static void register_MaxDepthException_class(void);
static void register_CallCountException_class(void);

void as_register_all_exception_classes(void)
{
    register_StrictifyThrowable_interface();
    register_ValidationException_class();
    register_InvalidBaseModelException_class();
    register_MaxDepthException_class();
    register_CallCountException_class();
    ZEND_ASSERT(class_AS_Exceptions_StrictifyThrowable != NULL && class_AS_Exceptions_ValidationException != NULL && class_AS_Exceptions_InvalidBaseModelException != NULL);
}

void as_throw_validation_exception(zval *errors)
{
    zval exception_val;

    object_init_ex(&exception_val, class_AS_Exceptions_ValidationException);
    zend_call_method_with_1_params(Z_OBJ(exception_val), class_AS_Exceptions_ValidationException, NULL, "__construct", NULL, errors);
    zend_throw_exception_object(&exception_val);
}

/*
 * The marker the four classes share, so one catch covers every error the
 * extension raises on purpose. They cannot share a parent: a malformed model,
 * an exceeded nesting limit and an unbuildable argument list are programming
 * errors (\Error family, like the engine's own TypeError/ValueError), while a
 * failed validation is a data error (\Exception family, like JsonException).
 */
static void register_StrictifyThrowable_interface(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Exceptions", "StrictifyThrowable", class_AS_Exceptions_StrictifyThrowable_methods);
    class_AS_Exceptions_StrictifyThrowable = zend_register_internal_interface(&ce);
    zend_class_implements(class_AS_Exceptions_StrictifyThrowable, 1, zend_ce_throwable);
}

static void register_InvalidBaseModelException_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Exceptions", "InvalidBaseModelException", class_AS_Exceptions_InvalidBaseModelException_methods);
    class_AS_Exceptions_InvalidBaseModelException = zend_register_internal_class_ex(&ce, zend_ce_error);
    zend_class_implements(class_AS_Exceptions_InvalidBaseModelException, 1, class_AS_Exceptions_StrictifyThrowable);
}

static void register_MaxDepthException_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Exceptions", "MaxDepthException", class_AS_Exceptions_MaxDepthException_methods);
    class_AS_Exceptions_MaxDepthException = zend_register_internal_class_ex(&ce, zend_ce_error);
    zend_class_implements(class_AS_Exceptions_MaxDepthException, 1, class_AS_Exceptions_StrictifyThrowable);
}

static void register_CallCountException_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Exceptions", "CallCountException", class_AS_Exceptions_CallCountException_methods);
    class_AS_Exceptions_CallCountException = zend_register_internal_class_ex(&ce, zend_ce_argument_count_error);
    zend_class_implements(class_AS_Exceptions_CallCountException, 1, class_AS_Exceptions_StrictifyThrowable);
}

static void register_ValidationException_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Exceptions", "ValidationException", class_AS_Exceptions_ValidationException_methods);
    class_AS_Exceptions_ValidationException = zend_register_internal_class_ex(&ce, zend_ce_exception);
    zend_class_implements(class_AS_Exceptions_ValidationException, 1, class_AS_Exceptions_StrictifyThrowable);

    zval property_allErrors_default_value;
    ZVAL_EMPTY_ARRAY(&property_allErrors_default_value);
    zend_string *property_allErrors_name = zend_string_init("allErrors", sizeof("allErrors") - 1, 1);
    zend_declare_typed_property(class_AS_Exceptions_ValidationException, property_allErrors_name, &property_allErrors_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type)ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
    zend_string_release(property_allErrors_name);
}

ZEND_METHOD(AS_Exceptions_ValidationException, __construct)
{
    zval *all_errors;
    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ARRAY(all_errors)
    ZEND_PARSE_PARAMETERS_END();

    zend_object *this = Z_OBJ_P(getThis());

    zend_update_property(class_AS_Exceptions_ValidationException, this, "allErrors", sizeof("allErrors") - 1, all_errors);

    zval default_error_message;
    ZVAL_STRING(&default_error_message, "Invalid data");
    zend_call_method_with_1_params(this, zend_ce_exception, NULL, "__construct", NULL, &default_error_message);
    zval_ptr_dtor(&default_error_message);
}

ZEND_METHOD(AS_Exceptions_ValidationException, getErrors)
{
    zval *allErrors, rv;
    zend_object *this = Z_OBJ_P(getThis());

    allErrors = zend_read_property(class_AS_Exceptions_ValidationException, this, "allErrors", sizeof("allErrors") - 1, 0, &rv);

    if (Z_TYPE_P(allErrors) == IS_NULL) {
        array_init(return_value);
        return;
    }

    RETURN_ZVAL(allErrors, 1, 0);
}

void as_convert_attribute_exception(void)
{
    zend_object *exception = EG(exception);

    if (exception == NULL) {
        return;
    }

    zend_class_entry *ce = exception->ce;
    if (!instanceof_function(ce, zend_ce_type_error) && !instanceof_function(ce, zend_ce_value_error)) {
        return;
    }

    /* Reading through the object's own class: the inherited Exception::$message
     * is not readable with the base class as scope. */
    zval rv;
    zval *message = zend_read_property(ce, exception, "message", sizeof("message") - 1, 1, &rv);
    zend_string *text = message != NULL && Z_TYPE_P(message) == IS_STRING ? zend_string_copy(Z_STR_P(message)) : NULL;

    zend_clear_exception();

    if (text == NULL) {
        zend_throw_exception(class_AS_Exceptions_InvalidBaseModelException, "The model declaration is invalid.", 0);
        return;
    }

    zend_throw_exception_ex(class_AS_Exceptions_InvalidBaseModelException, 0, "%s", ZSTR_VAL(text));
    zend_string_release(text);
}
