#include "as_json.h"
#include "../as_field.h"
#include "../../helpers/as_error_messages.h"
#include "../../helpers/as_wrappers.h"
#include "as_pattern_helpers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"

zend_class_entry *AS_Fields_Constraints_Json_ce;

static bool as_call_json_fn(const char *name, size_t name_len, zval *params, uint32_t param_count, zval *retval)
{
    zend_string *fname = zend_string_init(name, name_len, 0);
    zend_function *func = zend_hash_find_ptr(EG(function_table), fname);
    zend_string_release(fname);
    if (func == NULL) {
        return false;
    }
    zend_fcall_info fci = {0};
    zend_fcall_info_cache fcc = {0};
    fci.size = sizeof(fci);
    fci.retval = retval;
    fci.params = params;
    fci.param_count = param_count;
    fcc.function_handler = func;
    fcc.calling_scope = NULL;
    fcc.called_scope = NULL;
    fcc.object = NULL;
    zend_call_function(&fci, &fcc);
    return EG(exception) == NULL;
}

static bool as_is_valid_json(zend_string *subject)
{
    zval params[1];
    zval retval;
    ZVAL_STR(&params[0], subject);
    ZVAL_UNDEF(&retval);
    if (!as_call_json_fn("json_decode", sizeof("json_decode") - 1, params, 1, &retval)) {
        zend_clear_exception();
        return false;
    }
    zval_ptr_dtor(&retval);

    zval err_ret;
    ZVAL_UNDEF(&err_ret);
    if (!as_call_json_fn("json_last_error", sizeof("json_last_error") - 1, NULL, 0, &err_ret)) {
        zend_clear_exception();
        return false;
    }
    const bool ok = Z_TYPE(err_ret) == IS_LONG && Z_LVAL(err_ret) == 0 /* JSON_ERROR_NONE */;
    zval_ptr_dtor(&err_ret);
    return ok;
}

/*
 * #[Json]: see docs/constraints.md.
 */
bool as_validate_Json_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        as_add_field_error_with_prefix(AS_ERROR_JSON, field, property, errors);
        return false;
    }
    if (!as_is_valid_json(Z_STR_P(value))) {
        as_add_field_error_with_prefix(AS_ERROR_JSON, field, property, errors);
        return false;
    }
    return true;
}

AS_REGISTER_CONSTRAINT_CLASS(Json, ZEND_ACC_FINAL | ZEND_ACC_READONLY_CLASS)
