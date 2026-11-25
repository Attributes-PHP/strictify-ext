#include "as_constraint.h"
#include "as_config_aware_constraint.h"
#include "as_field.h"
#include "constraints/as_between.h"
#include "constraints/as_email.h"
#include "constraints/as_ends_with.h"
#include "constraints/as_in.h"
#include "constraints/as_max.h"
#include "constraints/as_min.h"
#include "constraints/as_multiple_of.h"
#include "constraints/as_regex.h"
#include "constraints/as_size.h"
#include "constraints/as_starts_with.h"
#include "constraints/as_url.h"
#include "../as_model_configs.h"
#include "../helpers/as_error_messages.h"
#include "../helpers/as_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include <ext/pcre/php_pcre.h>
#include <math.h>
#include <string.h>

zend_class_entry *AS_Fields_Constraint_ce;

void as_register_Constraint_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields", "Constraint", class_AS_Fields_Constraint_methods);
    AS_Fields_Constraint_ce = zend_register_internal_interface(&ce);
    ZEND_ASSERT(AS_Fields_Constraint_ce != NULL);

    ZEND_ASSERT(AS_Fields_Field_ce != NULL);
    zend_class_implements(AS_Fields_Constraint_ce, 1, AS_Fields_Field_ce);
}

static void as_set_model_configs_if_aware(zval *constraint, as_property_info *property);
static void add_invalid_custom_error_message(zval *constraint, as_field *field, as_property_info *property, zval *errors);

/*
 * ConfigAwareConstraint constraints receive the owning model's resolved configs
 * before validate() runs (and thus before message()). Exceptions thrown
 * by setModelConfigs() propagate to the caller of validate() like model
 * hook errors.
 */
static void as_set_model_configs_if_aware(zval *constraint, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(constraint);
    if (!instanceof_function(ce, AS_Fields_ConfigAwareConstraint_ce)) {
        return;
    }

    zval configs;
    as_model_configs_properties properties;
    as_get_model_configs(&configs, property->model, &properties, true);
    if (UNEXPECTED(EG(exception) != NULL)) {
        return;
    }

    zend_function *set_model_configs = zend_hash_str_find_ptr(&ce->function_table, "setmodelconfigs", sizeof("setmodelconfigs") - 1);
    ZEND_ASSERT(set_model_configs != NULL);

    zend_call_known_instance_method_with_1_params(set_model_configs, Z_OBJ_P(constraint), NULL, &configs);
    zval_ptr_dtor(&configs);
}

static void add_invalid_custom_error_message(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zend_class_entry *ce = Z_OBJCE_P(constraint);
    zend_function *message = zend_hash_str_find_ptr(&ce->function_table, "message", sizeof("message") - 1);
    ZEND_ASSERT(message != NULL);

    zval message_value;
    ZVAL_UNDEF(&message_value);
    zend_call_known_instance_method_with_0_params(message, Z_OBJ_P(constraint), &message_value);
    if (EG(exception) != NULL) {
        if (Z_TYPE(message_value) != IS_UNDEF) {
            zval_ptr_dtor(&message_value);
        }
        return;
    }

    ZEND_ASSERT(Z_TYPE(message_value) == IS_STRING);
    zend_string *rendered = as_replace_placeholders(Z_STRVAL(message_value), Z_STRLEN(message_value), field, property);
    zval_ptr_dtor(&message_value);

    as_add_field_message(rendered, field, errors);
    zend_string_release(rendered);
}

/*
 * Userland constraint validation: the interface contract guarantees the
 * validate() and message() methods, with the engine enforcing their
 * signatures since PHP classes cannot implement an interface without
 * them. Exceptions thrown by either method propagate to the caller of
 * validate() like model hook errors.
 */
bool as_validate_userland_constraint(zval *constraint, as_field *field, as_property_info *property, zval *errors)
{
    zend_class_entry *ce = Z_OBJCE_P(constraint);

    as_set_model_configs_if_aware(constraint, property);
    if (UNEXPECTED(EG(exception) != NULL)) {
        return false;
    }

    zend_function *validate = zend_hash_str_find_ptr(&ce->function_table, "validate", sizeof("validate") - 1);
    ZEND_ASSERT(validate != NULL);

    zval result;
    ZVAL_UNDEF(&result);
    zend_call_known_instance_method_with_1_params(validate, Z_OBJ_P(constraint), &result, field->value);

    const bool valid = Z_TYPE(result) == IS_TRUE;
    if (Z_TYPE(result) != IS_UNDEF) {
        zval_ptr_dtor(&result);
    }

    if (EG(exception) != NULL) {
        return false;
    }

    if (!valid) {
        add_invalid_custom_error_message(constraint, field, property, errors);
        return false;
    }

    return true;
}

/* =========================================================================
 * Constraint attribute argument validation
 *
 * Attribute arguments reach the constraint constructors when the field is
 * first compiled, so a rejected argument throws once per compilation and
 * keeps throwing on every validate() call (see docs/constraints.md). The
 * constructors only enforce their stub signatures; the value rules below
 * make sure a model cannot carry a constraint that can never be satisfied
 * (or can never fail), which would otherwise stay silent forever.
 * ========================================================================= */

static void as_throw_constraint_arg_error(zend_class_entry *constraint_ce, as_property_info *property, const char *format, ...);

static void as_throw_constraint_arg_error(zend_class_entry *constraint_ce, as_property_info *property, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    zend_string *message = zend_vstrpprintf(0, format, args);
    va_end(args);

    zend_throw_exception_ex(zend_ce_value_error, 0, "Attribute \"%s\" on %s::$%s: %s", ZSTR_VAL(constraint_ce->name), ZSTR_VAL(property->model_ce->name), ZSTR_VAL(property->property->name), ZSTR_VAL(message));
    zend_string_release(message);
}

static zend_always_inline double as_number_of(zval *value)
{
    return Z_TYPE_P(value) == IS_LONG ? (double)Z_LVAL_P(value) : Z_DVAL_P(value);
}

static zend_always_inline bool as_is_nan(zval *value)
{
    return Z_TYPE_P(value) == IS_DOUBLE && isnan(Z_DVAL_P(value));
}

/* Rejects NAN bounds: every comparison against NAN is false, so a NAN bound
 * silently turns the constraint into an always-fail (or, for Max/Min, an
 * always-pass) rule. */
static bool as_check_numeric_bound(zval *bound, zend_class_entry *ce, as_property_info *property, const char *argument)
{
    if (!as_is_nan(bound)) {
        return true;
    }

    as_throw_constraint_arg_error(ce, property, "$%s must be a number, NAN given.", argument);
    return false;
}

static bool as_check_between_args(zval *instance, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *min = zend_read_property(ce, Z_OBJ_P(instance), "min", sizeof("min") - 1, false, NULL);
    zval *max = zend_read_property(ce, Z_OBJ_P(instance), "max", sizeof("max") - 1, false, NULL);

    if (!as_check_numeric_bound(min, ce, property, "min") || !as_check_numeric_bound(max, ce, property, "max")) {
        return false;
    }

    if (as_number_of(min) <= as_number_of(max)) {
        return true;
    }

    zend_string *min_str = as_value_to_string(min);
    zend_string *max_str = as_value_to_string(max);
    as_throw_constraint_arg_error(ce, property, "$min (%s) must be less than or equal to $max (%s).", ZSTR_VAL(min_str), ZSTR_VAL(max_str));
    as_string_release(min_str);
    as_string_release(max_str);

    return false;
}

static bool as_check_single_bound_args(zval *instance, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *bound = zend_read_property(ce, Z_OBJ_P(instance), "value", sizeof("value") - 1, false, NULL);

    return as_check_numeric_bound(bound, ce, property, "value");
}

static bool as_check_size_args(zval *instance, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *min = zend_read_property(ce, Z_OBJ_P(instance), "min", sizeof("min") - 1, false, NULL);
    zval *max = zend_read_property(ce, Z_OBJ_P(instance), "max", sizeof("max") - 1, false, NULL);
    const bool has_min = Z_TYPE_P(min) != IS_NULL;
    const bool has_max = Z_TYPE_P(max) != IS_NULL;

    if (has_min && Z_LVAL_P(min) < 0) {
        as_throw_constraint_arg_error(ce, property, "$min must not be negative, " ZEND_LONG_FMT " given.", Z_LVAL_P(min));
        return false;
    }

    if (has_max && Z_LVAL_P(max) < 0) {
        as_throw_constraint_arg_error(ce, property, "$max must not be negative, " ZEND_LONG_FMT " given.", Z_LVAL_P(max));
        return false;
    }

    if (has_min && has_max && Z_LVAL_P(min) > Z_LVAL_P(max)) {
        as_throw_constraint_arg_error(ce, property, "$min (" ZEND_LONG_FMT ") must be less than or equal to $max (" ZEND_LONG_FMT ").", Z_LVAL_P(min), Z_LVAL_P(max));
        return false;
    }

    return true;
}

/* #[In], #[StartsWith] and #[EndsWith] need at least one declared value:
 * an empty list makes the constraint fail every input. */
static bool as_check_declared_values(zval *instance, as_property_info *property, const char *what)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *values = zend_read_property(ce, Z_OBJ_P(instance), "values", sizeof("values") - 1, false, NULL);

    if (zend_hash_num_elements(Z_ARRVAL_P(values)) > 0) {
        return true;
    }

    as_throw_constraint_arg_error(ce, property, "at least one %s must be declared.", what);
    return false;
}

/* A declared numeric prefix/suffix is rendered with its string
 * representation, so a float that stringifies to "" cannot exist; empty
 * strings are the only needless entries: they match every value. */
static bool as_check_no_empty_affix(zval *instance, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *values = zend_read_property(ce, Z_OBJ_P(instance), "values", sizeof("values") - 1, false, NULL);
    zval *entry;

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), entry)
    {
        if (Z_TYPE_P(entry) == IS_STRING && Z_STRLEN_P(entry) == 0) {
            as_throw_constraint_arg_error(ce, property, "an empty string matches every value: declare only non-empty values.");
            return false;
        }
    }
    ZEND_HASH_FOREACH_END();

    return true;
}

/* #[Email(domain: ...)] and #[Url(protocol: ..., domain: ...)]: an empty
 * string or an empty list allows no domain/protocol at all, so the
 * constraint fails every input (with a dangling ": " in the message). */
static bool as_check_allowed_list(zval *allowed, zend_class_entry *ce, as_property_info *property, const char *argument)
{
    if (Z_TYPE_P(allowed) == IS_NULL) {
        return true;
    }

    if (Z_TYPE_P(allowed) == IS_STRING) {
        if (Z_STRLEN_P(allowed) > 0) {
            return true;
        }

        as_throw_constraint_arg_error(ce, property, "$%s must not be empty.", argument);
        return false;
    }

    if (zend_hash_num_elements(Z_ARRVAL_P(allowed)) == 0) {
        as_throw_constraint_arg_error(ce, property, "$%s must declare at least one entry.", argument);
        return false;
    }

    zval *entry;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(allowed), entry)
    {
        if (Z_TYPE_P(entry) == IS_STRING && Z_STRLEN_P(entry) == 0) {
            as_throw_constraint_arg_error(ce, property, "$%s must not contain an empty string.", argument);
            return false;
        }
    }
    ZEND_HASH_FOREACH_END();

    return true;
}

static bool as_check_email_args(zval *instance, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *domain = zend_read_property(ce, Z_OBJ_P(instance), "domain", sizeof("domain") - 1, false, NULL);

    return as_check_allowed_list(domain, ce, property, "domain");
}

static bool as_check_url_args(zval *instance, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *protocol = zend_read_property(ce, Z_OBJ_P(instance), "protocol", sizeof("protocol") - 1, false, NULL);
    zval *domain = zend_read_property(ce, Z_OBJ_P(instance), "domain", sizeof("domain") - 1, false, NULL);

    return as_check_allowed_list(protocol, ce, property, "protocol") && as_check_allowed_list(domain, ce, property, "domain");
}

/* Modifiers PHP accepts after the closing delimiter (the "e" modifier is
 * gone since PHP 7). Anything else is an error at compile time. */
static const char AS_REGEX_MODIFIERS[] = "imsxADSUXJunr";

/*
 * Validates a preg pattern without touching PHP's compiled-regex cache:
 * compiling through it raises an E_WARNING on failure (and, under a
 * warning-to-exception error handler, that warning would escape from
 * validate() on every call). Delimiter parsing mirrors preg's rules and
 * PCRE2 does the syntax check.
 */
static bool as_regex_pattern_is_valid(zend_string *pattern, zend_string **reason)
{
    const char *value = ZSTR_VAL(pattern);
    const size_t length = ZSTR_LEN(pattern);

    if (length == 0) {
        *reason = zend_string_init("the pattern is empty", sizeof("the pattern is empty") - 1, 0);
        return false;
    }

    const char delimiter = value[0];
    if ((delimiter >= 'a' && delimiter <= 'z') || (delimiter >= 'A' && delimiter <= 'Z') || (delimiter >= '0' && delimiter <= '9') || delimiter == '\\' || delimiter == ' ' || delimiter == '\t' || delimiter == '\n' ||
        delimiter == '\r' || delimiter == '\v' || delimiter == '\f') {
        *reason = zend_string_init("the delimiter must not be alphanumeric, a backslash or whitespace", sizeof("the delimiter must not be alphanumeric, a backslash or whitespace") - 1, 0);
        return false;
    }

    char closing = delimiter;
    switch (delimiter) {
        case '(':
            closing = ')';
            break;
        case '[':
            closing = ']';
            break;
        case '{':
            closing = '}';
            break;
        case '<':
            closing = '>';
            break;
        default:
            break;
    }

    /* The closing delimiter is the first unescaped occurrence, as in preg;
     * bracket-style delimiters nest and must be balanced. */
    size_t body_end = 0;
    bool found = false;
    uint32_t depth = 1;
    for (size_t i = 1; i < length; i++) {
        if (value[i] == '\\') {
            i++;
            continue;
        }

        if (closing != delimiter && value[i] == delimiter) {
            depth++;
            continue;
        }

        if (value[i] != closing) {
            continue;
        }

        if (closing != delimiter && --depth > 0) {
            continue;
        }

        body_end = i;
        found = true;
        break;
    }

    if (!found) {
        *reason = zend_string_init("no ending delimiter was found", sizeof("no ending delimiter was found") - 1, 0);
        return false;
    }

    uint32_t flags = 0;
    bool seen[sizeof(AS_REGEX_MODIFIERS)] = {false};
    for (size_t i = body_end + 1; i < length; i++) {
        const char *modifier = memchr(AS_REGEX_MODIFIERS, value[i], sizeof(AS_REGEX_MODIFIERS) - 1);
        if (modifier == NULL) {
            *reason = zend_strpprintf(0, "unknown modifier '%c'", value[i]);
            return false;
        }

        const size_t index = (size_t)(modifier - AS_REGEX_MODIFIERS);
        if (seen[index]) {
            *reason = zend_strpprintf(0, "duplicate modifier '%c'", value[i]);
            return false;
        }
        seen[index] = true;

        switch (value[i]) {
            case 'i':
                flags |= PCRE2_CASELESS;
                break;
            case 'm':
                flags |= PCRE2_MULTILINE;
                break;
            case 's':
                flags |= PCRE2_DOTALL;
                break;
            case 'x':
                flags |= PCRE2_EXTENDED;
                break;
            case 'A':
                flags |= PCRE2_ANCHORED;
                break;
            case 'D':
                flags |= PCRE2_DOLLAR_ENDONLY;
                break;
            case 'U':
                flags |= PCRE2_UNGREEDY;
                break;
            case 'J':
                flags |= PCRE2_DUPNAMES;
                break;
            case 'n':
                flags |= PCRE2_NO_AUTO_CAPTURE;
                break;
            case 'u':
                flags |= PCRE2_UTF | PCRE2_UCP;
                break;
#ifdef PCRE2_EXTRA_CASELESS_RESTRICT
            case 'r':
                /* Set through the compile context below: PCRE2 has no
                 * plain option bit for it on older releases. */
                break;
#endif
            default:
                /* S and X are accepted no-ops (study / extra) */
                break;
        }
    }

    const char *body = value + 1;
    const size_t body_length = body_end - 1;
    if (memchr(body, '\0', body_length) != NULL) {
        *reason = zend_string_init("the pattern contains a NUL byte", sizeof("the pattern contains a NUL byte") - 1, 0);
        return false;
    }

    int error_code = 0;
    PCRE2_SIZE error_offset = 0;
    pcre2_code *compiled = pcre2_compile((PCRE2_SPTR)body, body_length, flags, &error_code, &error_offset, php_pcre_cctx());

    if (compiled == NULL) {
        PCRE2_UCHAR error_message[256];
        pcre2_get_error_message(error_code, error_message, sizeof(error_message));
        *reason = zend_strpprintf(0, "the pattern does not compile: %s at offset " ZEND_LONG_FMT, (const char *)error_message, (zend_long)error_offset);
        return false;
    }

    pcre2_code_free(compiled);
    return true;
}

static bool as_check_regex_args(zval *instance, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(instance);
    zval *pattern = zend_read_property(ce, Z_OBJ_P(instance), "regex", sizeof("regex") - 1, false, NULL);

    zend_string *reason = NULL;
    if (as_regex_pattern_is_valid(Z_STR_P(pattern), &reason)) {
        return true;
    }

    as_throw_constraint_arg_error(ce, property, "%s.", ZSTR_VAL(reason));
    zend_string_release(reason);
    return false;
}

/*
 * Runs the argument rules of a compiled constraint instance. Returns false
 * with a pending ValueError when an argument makes the constraint
 * unsatisfiable (or a no-op); userland constraints and the constraints
 * whose arguments are always meaningful return true.
 */
bool as_validate_constraint_attribute_args(zval *constraint, as_property_info *property)
{
    zend_class_entry *ce = Z_OBJCE_P(constraint);

    if (ce == AS_Fields_Constraints_Between_ce) {
        return as_check_between_args(constraint, property);
    }
    if (ce == AS_Fields_Constraints_Max_ce || ce == AS_Fields_Constraints_Min_ce || ce == AS_Fields_Constraints_MultipleOf_ce) {
        return as_check_single_bound_args(constraint, property);
    }
    if (ce == AS_Fields_Constraints_Size_ce) {
        return as_check_size_args(constraint, property);
    }
    if (ce == AS_Fields_Constraints_In_ce) {
        return as_check_declared_values(constraint, property, "value");
    }
    if (ce == AS_Fields_Constraints_StartsWith_ce) {
        return as_check_declared_values(constraint, property, "prefix") && as_check_no_empty_affix(constraint, property);
    }
    if (ce == AS_Fields_Constraints_EndsWith_ce) {
        return as_check_declared_values(constraint, property, "suffix") && as_check_no_empty_affix(constraint, property);
    }
    if (ce == AS_Fields_Constraints_Regex_ce) {
        return as_check_regex_args(constraint, property);
    }
    if (ce == AS_Fields_Constraints_Email_ce) {
        return as_check_email_args(constraint, property);
    }
    if (ce == AS_Fields_Constraints_Url_ce) {
        return as_check_url_args(constraint, property);
    }

    return true;
}
