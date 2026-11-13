#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "php.h"
#include "ext/standard/info.h"
#include "strictify.h"

/* Include the component headers */
#include "src/as_globals.h"
#include "src/as_base_model.h"
#include "src/as_compiled_model.h"
#include "src/as_exception.h"
#include "src/as_model_configs.h"
#include "src/as_type.h"
#include "src/fields/as_constraint.h"
#include "src/fields/as_config_aware_constraint.h"
#include "src/fields/as_field.h"
#include "src/fields/as_alias.h"
#include "src/fields/as_dict.h"
#include "src/fields/as_error_message.h"
#include "src/fields/as_intersection.h"
#include "src/fields/as_sequence.h"
#include "src/fields/as_union.h"
#include "src/fields/constraints/as_alpha_dash.h"
#include "src/fields/constraints/as_ascii.h"
#include "src/fields/constraints/as_between.h"
#include "src/fields/constraints/as_contains.h"
#include "src/fields/constraints/as_datetime.h"
#include "src/fields/constraints/as_ends_with.h"
#include "src/fields/constraints/as_in.h"
#include "src/fields/constraints/as_max.h"
#include "src/fields/constraints/as_min.h"
#include "src/fields/constraints/as_multiple_of.h"
#include "src/fields/constraints/as_regex.h"
#include "src/fields/constraints/as_size.h"
#include "src/fields/constraints/as_starts_with.h"
#include "src/fields/constraints/as_ulid.h"
#include "src/fields/constraints/as_uuid.h"
#include "src/validators/as_typehint_validator.h"
#include "src/helpers/as_error_messages.h"

ZEND_DECLARE_MODULE_GLOBALS(strictify)

/*
 * PHP extensions whose C APIs this module calls directly: ext/pcre for the
 * Regex constraint (php_pcre_match_impl), ext/date for the DateTime
 * constraint (php_date_*), and ext/spl headers for exception helpers.
 * Declared as required module deps so the engine refuses to load this
 * module — logging the missing extension's name — when one is absent.
 */
static const zend_module_dep strictify_deps[] = {ZEND_MOD_REQUIRED("pcre") ZEND_MOD_REQUIRED("date") ZEND_MOD_REQUIRED("spl") ZEND_MOD_END};

static PHP_GINIT_FUNCTION(strictify)
{
    strictify_globals->as_model_configs_cache = NULL;
    strictify_globals->as_field_name_cache = NULL;
    strictify_globals->as_plans_cache = NULL;
    strictify_globals->as_spec_cache = NULL;
    strictify_globals->as_error_templates_cache = NULL;
}

/* Module startup */
PHP_MINIT_FUNCTION(strictify)
{
    // Safety net: the module entry declares pcre/date/spl as required deps,
    // so the engine normally refuses to load this module when one is
    // missing. Re-check here and log the missing name(s) before failing,
    // so an absent extension can never crash validation at runtime.
    for (const zend_module_dep *dep = strictify_deps; dep->name != NULL; dep++) {
        if (dep->type == MODULE_DEP_REQUIRED && zend_hash_str_find(&module_registry, dep->name, strlen(dep->name)) == NULL) {
            zend_error(E_CORE_WARNING, "%s: required PHP extension '%s' is not loaded; module will not start", EXTENSION_NAME, dep->name);
            return FAILURE;
        }
    }

    // Register main classes
    as_register_BaseModel_class();
    as_register_ModelConfigs_class();
    as_register_all_exception_classes();

    // Register the Type enum and its namespace constants
    as_register_Type_enum();
    as_register_type_constants(module_number);

    /** Fields **/
    as_register_Field_interface();
    as_register_Constraint_interface();
    as_register_ConfigAwareConstraint_interface();
    as_register_Alias_class();
    as_register_ErrorMessage_class();
    as_register_Sequence_class();
    as_register_Union_class();
    as_register_Intersection_class();
    as_register_Dict_class();
    // Constraints
    as_register_Size_class();
    as_register_In_class();
    // Array-specific
    as_register_Contains_class();
    // String-specific
    as_register_AlphaDash_class();
    as_register_Ascii_class();
    as_register_Ulid_class();
    as_register_Uuid_class();
    as_register_Regex_class();
    as_register_StartsWith_class();
    as_register_EndsWith_class();
    // Number-specific
    as_register_Between_class();
    as_register_Max_class();
    as_register_Min_class();
    as_register_MultipleOf_class();
    // Date-specific
    as_register_DateTime_class();
    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(strictify)
{
    return SUCCESS;
}

/* Request startup */
PHP_RINIT_FUNCTION(strictify)
{
    // Initialize DateTime classes (must run after all MINITs)
    as_init_typehint_validator();
    return SUCCESS;
}

/* Request shutdown */
PHP_RSHUTDOWN_FUNCTION(strictify)
{
    // Plans borrow specs from the array spec cache and names from the
    // field name cache: clearing the plans first is the only ordering
    // that matters, as dtors never dereference borrowed pointers
    as_clear_compiled_model_cache();
    as_clear_model_configs_cache();
    as_clear_array_spec_cache();
    as_clear_field_name_cache();
    as_clear_error_templates_cache();
    as_shutdown_typehint_validator();

    return SUCCESS;
}

/* Module info */
PHP_MINFO_FUNCTION(strictify)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "attributes-strictify", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

/* Module entry */
zend_module_entry strictify_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    strictify_deps,
    EXTENSION_NAME,
    ext_functions,
    PHP_MINIT(strictify),
    PHP_MSHUTDOWN(strictify),
    PHP_RINIT(strictify),
    PHP_RSHUTDOWN(strictify),
    PHP_MINFO(strictify),
    EXTENSION_VERSION,
    PHP_MODULE_GLOBALS(strictify), /* globals descriptor */
    PHP_GINIT(strictify),          /* globals ctor */
    NULL,                          /* globals dtor */
    NULL,                          /* post deactivate */
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_ATTRIBUTES_STRICTIFY
ZEND_GET_MODULE(strictify)
#endif
