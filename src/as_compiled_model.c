#include "as_compiled_model.h"
#include "as_globals.h"
#include "as_base_model.h"
#include "as_exception.h"
#include "as_validate_function.h"
#include "fields/as_constraint.h"
#include "fields/constraints/as_alpha_dash.h"
#include "fields/constraints/as_ascii.h"
#include "fields/constraints/as_between.h"
#include "fields/constraints/as_contains.h"
#include "fields/constraints/as_datetime.h"
#include "fields/constraints/as_ends_with.h"
#include "fields/constraints/as_in.h"
#include "fields/constraints/as_max.h"
#include "fields/constraints/as_min.h"
#include "fields/constraints/as_multiple_of.h"
#include "fields/constraints/as_regex.h"
#include "fields/constraints/as_size.h"
#include "fields/constraints/as_starts_with.h"
#include "fields/constraints/as_ulid.h"
#include "fields/constraints/as_uuid.h"
#include "helpers/as_error_messages.h"
#include "helpers/as_string.h"
#include "validators/as_array_typehint_validator.h"
#include "validators/as_typehint_validator.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_hash.h"
#include <stddef.h>
#include <string.h>

/* =========================================================================
 * Constraint attributes
 * ========================================================================= */

/*
 * Constraint attribute dispatch: attribute lowercase name -> class entry
 * and validation function. The validation logic itself lives in each
 * constraint's own file; this table only wires attributes to it.
 */
typedef struct {
    const char *lcname;
    size_t lcname_len;
    zend_class_entry **ce;
    as_constraint_validator_t validate;
} as_constraint_dispatch_entry;

#define AS_CONSTRAINT_ENTRY(lower, upper) \
    {"attributes\\strictify\\fields\\constraints\\" lower, sizeof("attributes\\strictify\\fields\\constraints\\" lower) - 1, &AS_Fields_Constraints_##upper##_ce, as_validate_##upper##_constraint}

static const as_constraint_dispatch_entry constraint_dispatch_table[] = {
    AS_CONSTRAINT_ENTRY("alphadash", AlphaDash),
    AS_CONSTRAINT_ENTRY("ascii", Ascii),
    AS_CONSTRAINT_ENTRY("between", Between),
    AS_CONSTRAINT_ENTRY("contains", Contains),
    AS_CONSTRAINT_ENTRY("datetime", DateTime),
    AS_CONSTRAINT_ENTRY("endswith", EndsWith),
    AS_CONSTRAINT_ENTRY("in", In),
    AS_CONSTRAINT_ENTRY("max", Max),
    AS_CONSTRAINT_ENTRY("min", Min),
    AS_CONSTRAINT_ENTRY("multipleof", MultipleOf),
    AS_CONSTRAINT_ENTRY("regex", Regex),
    AS_CONSTRAINT_ENTRY("size", Size),
    AS_CONSTRAINT_ENTRY("startswith", StartsWith),
    AS_CONSTRAINT_ENTRY("ulid", Ulid),
    AS_CONSTRAINT_ENTRY("uuid", Uuid),
};

#define CONSTRAINT_DISPATCH_TABLE_SIZE (sizeof(constraint_dispatch_table) / sizeof(constraint_dispatch_table[0]))

/* =========================================================================
 * Field name resolution (moved from as_validate_function.c, compile-time only)
 * ========================================================================= */

static zend_always_inline zend_string *transform_property_name(zend_string *property_name, char alias_generator);
static zend_string *resolve_field_name(as_property_info *property_info, zend_string *property_name, char alias_generator);
static void as_field_name_cache_dtor(zval *entry);
static zend_string *get_property_name(as_property_info *property_info, zend_string *property_name, char alias_generator);
static zend_always_inline bool has_property_default_value(as_property_info *property_info);
static const as_constraint_dispatch_entry *find_constraint_dispatch(const zend_string *lcname);
static bool instantiate_constraint_attribute(zval *instance, zend_class_entry *ce, zend_attribute *attribute, zend_class_entry *scope);
static bool compile_field_constraints(as_compiled_field *cf);
static uint32_t generator_index(char alias_generator);
static void free_compiled_field(as_compiled_field *cf);
static void free_compiled_model(as_compiled_model *plan);
static void class_plans_dtor(zval *entry);
static as_class_plans *get_class_plans(zend_class_entry *ce);
static as_compiled_model *compile_plan_skeleton(zend_class_entry *model_ce, char generator);
static bool compile_field(as_compiled_field *cf, as_compiled_model *plan);

/**
 * Transforms a property name based on the alias generator type
 */
static zend_always_inline zend_string *transform_property_name(zend_string *property_name, char alias_generator)
{
    switch (alias_generator) {
        case AS_PASCAL_CASE:
            return as_to_pascal_case(property_name);
        case AS_CAMEL_CASE:
            return as_to_camel_case(property_name);
        case AS_SNAKE_CASE:
            return as_to_snake_case(property_name);
        case AS_KEBAB_CASE:
            return as_to_kebab_case(property_name);
        default:
            return zend_string_copy(property_name);
    }
}

/**
 * Resolves the field name of a property:
 *  1) If the Alias attribute is set uses that value
 *  2) If aliasGenerator is configured, transforms the property name
 *  3) Otherwise uses the property name as-is
 */
static zend_string *resolve_field_name(as_property_info *property_info, zend_string *property_name, char alias_generator)
{
    zend_string *field_name = NULL;

    // Check for #[Alias] attribute on the property
    if (property_info->property->attributes != NULL) {
        zend_attribute *alias_attr = zend_get_attribute_str(property_info->property->attributes, "attributes\\strictify\\fields\\alias", sizeof("attributes\\strictify\\fields\\alias") - 1);
        if (alias_attr != NULL && alias_attr->argc > 0) {
            zval attr_value;
            if (zend_get_attribute_value(&attr_value, alias_attr, 0, property_info->model_ce) == SUCCESS) {
                if (UNEXPECTED(Z_TYPE(attr_value) != IS_STRING)) {
                    zval_ptr_dtor(&attr_value);
                    zend_argument_type_error(1, "must be of type string, %s given", zend_zval_type_name(&attr_value));
                    return NULL;
                }
                field_name = zend_string_copy(Z_STR(attr_value));
                zval_ptr_dtor(&attr_value);
                return field_name;
            }
        }
    }

    // If no Alias attribute, check for aliasGenerator in ModelConfigs
    if (alias_generator != false) {
        return transform_property_name(property_name, alias_generator);
    }

    return property_name;
}

/*
 * Field name cache, keyed by the property info and stored in the module
 * globals (see as_globals.h): the Alias attribute and the alias
 * generator never change within a request, so resolutions run once per
 * property. The cache owns one reference; hits hand out an additional
 * one. Erroring resolutions stay uncached so their errors keep firing.
 */
static void as_field_name_cache_dtor(zval *entry)
{
    zend_string_release(Z_PTR_P(entry));
}

void as_clear_field_name_cache(void)
{
    if (AS_G(as_field_name_cache) != NULL) {
        zend_hash_destroy(AS_G(as_field_name_cache));
        efree(AS_G(as_field_name_cache));
        AS_G(as_field_name_cache) = NULL;
    }
}

static zend_string *get_property_name(as_property_info *property_info, zend_string *property_name, char alias_generator)
{
    // Properties without attributes and without a generator resolve to
    // the property name directly: nothing to cache
    if (property_info->property->attributes == NULL && alias_generator == false) {
        return property_name;
    }

    if (AS_G(as_field_name_cache) != NULL) {
        zend_string *cached = zend_hash_index_find_ptr(AS_G(as_field_name_cache), (zend_ulong)(uintptr_t)property_info->property);
        if (cached != NULL) {
            return zend_string_copy(cached);
        }
    }

    zend_string *field_name = resolve_field_name(property_info, property_name, alias_generator);

    // The error path (pending exception) must keep firing
    if (field_name == NULL || UNEXPECTED(EG(exception) != NULL)) {
        return field_name;
    }

    // A property with attributes but no Alias and no generator resolves
    // to the property name: cache the miss too, so its attribute lookup
    // is not repeated on every validation
    if (field_name == property_name) {
        field_name = zend_string_copy(property_name);
    }

    if (AS_G(as_field_name_cache) == NULL) {
        AS_G(as_field_name_cache) = emalloc(sizeof(HashTable));
        zend_hash_init(AS_G(as_field_name_cache), 8, NULL, as_field_name_cache_dtor, 0);
    }

    zend_hash_index_add_ptr(AS_G(as_field_name_cache), (zend_ulong)(uintptr_t)property_info->property, field_name);

    return zend_string_copy(field_name);
}

static zend_always_inline bool has_property_default_value(as_property_info *property_info)
{
    // OBJ_PROP_TO_NUM() is only meaningful for declared (non-static) properties
    ZEND_ASSERT(!(property_info->property->flags & ZEND_ACC_STATIC));

    if (!property_info->model_ce->default_properties_table)
        return false;

    const uint32_t index = OBJ_PROP_TO_NUM(property_info->property->offset);
    if (index >= property_info->model_ce->default_properties_count)
        return false;

    const zval *default_value = &property_info->model_ce->default_properties_table[index];
    return Z_TYPE_P(default_value) != IS_UNDEF;
}

static const as_constraint_dispatch_entry *find_constraint_dispatch(const zend_string *lcname)
{
    if (lcname == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < CONSTRAINT_DISPATCH_TABLE_SIZE; i++) {
        const as_constraint_dispatch_entry *entry = &constraint_dispatch_table[i];
        if (ZSTR_LEN(lcname) == entry->lcname_len && memcmp(ZSTR_VAL(lcname), entry->lcname, entry->lcname_len) == 0) {
            return entry;
        }
    }

    return NULL;
}

/*
 * Instantiates a constraint attribute: evaluates its arguments and
 * runs the constraint class's internal constructor, the same way a
 * userland new expression does (the constructor enforces the stub
 * signature). Positional arguments stay positional; named arguments
 * (for example #[Size(min: 3)]) go through the engine's
 * named-parameter machinery.
 *
 * The instance zval is owned by the caller. Returns false with a
 * pending exception when an argument cannot be evaluated or the
 * constructor rejects it.
 */
static bool instantiate_constraint_attribute(zval *instance, zend_class_entry *ce, zend_attribute *attribute, zend_class_entry *scope)
{
    object_init_ex(instance, ce);

    zend_function *constructor = ce->constructor;
    if (constructor == NULL) {
        /* The class has no constructor: as with userland classes, the
         * arguments are ignored. */
        return true;
    }

    uint32_t argc = attribute->argc;
    if (argc == 0) {
        /* The constructor still runs: it initializes the properties
         * (for example In's empty $values array), like a userland
         * new In() expression. */
        zend_call_known_function(constructor, Z_OBJ_P(instance), ce, NULL, 0, NULL, NULL);
        if (EG(exception) != NULL) {
            zval_ptr_dtor(instance);
            return false;
        }
        return true;
    }

    /* The UNDEF pre-initialization is load-bearing: an evaluation
     * failure leaves the failing slot untouched, and named arguments
     * are moved into named_params below with the source slot reset to
     * UNDEF — both keep the cleanup loop exact. */
    zval *params = safe_emalloc(argc, sizeof(zval), 0);
    for (uint32_t i = 0; i < argc; i++) {
        ZVAL_UNDEF(&params[i]);
    }

    HashTable *named_params = NULL;
    uint32_t positional_count = 0;
    bool failed = false;

    /* Positional arguments always precede named ones (the compiler
     * rejects the opposite), so the first positional_count slots of
     * params form a gap-free prefix passed to the constructor. */
    for (uint32_t i = 0; i < argc; i++) {
        zval *param = &params[i];
        if (zend_get_attribute_value(param, attribute, i, scope) != SUCCESS) {
            failed = true;
            break;
        }

        if (attribute->args[i].name == NULL) {
            ZEND_ASSERT(i == positional_count);
            positional_count++;
            continue;
        }

        if (named_params == NULL) {
            named_params = emalloc(sizeof(HashTable));
            zend_hash_init(named_params, argc, NULL, ZVAL_PTR_DTOR, 0);
        }
        zend_hash_add(named_params, attribute->args[i].name, param);
        ZVAL_UNDEF(param);
    }

    if (!failed) {
        /* Constructors cannot return anything meaningful: discard. */
        zend_call_known_function(constructor, Z_OBJ_P(instance), ce, NULL, positional_count, params, named_params);
    }

    for (uint32_t i = 0; i < argc; i++) {
        if (Z_TYPE(params[i]) != IS_UNDEF) {
            zval_ptr_dtor(&params[i]);
        }
    }
    efree(params);
    if (named_params != NULL) {
        zend_hash_destroy(named_params);
        efree(named_params);
    }

    if (failed || EG(exception) != NULL) {
        zval_ptr_dtor(instance);
        return false;
    }

    return true;
}

/*
 * Collects the constraint attributes of a field, instantiating each
 * once (the plan owns the instances). Non-constraint attributes
 * (Alias, ErrorMessage, the array type-hint attributes) are skipped.
 * A failure (pending exception from a bad constructor argument)
 * returns false with the partial array freed.
 */
static bool compile_field_constraints(as_compiled_field *cf)
{
    if (cf->prop->attributes == NULL) {
        return true;
    }

    as_compiled_constraint *constraints = NULL;
    uint32_t count = 0;
    uint32_t capacity = 0;

    zend_attribute *attribute;
    ZEND_HASH_FOREACH_PTR(cf->prop->attributes, attribute)
    {
        const as_constraint_dispatch_entry *entry = find_constraint_dispatch(attribute->lcname);
        as_constraint_validator_t validate;
        zend_class_entry *ce;

        if (entry != NULL) {
            ce = *entry->ce;
            validate = entry->validate;
        } else {
            /*
             * Userland constraint: any attribute class implementing the
             * Constraint interface. Unresolvable classes and classes not
             * implementing the interface are skipped, exactly like any
             * other attribute the engine does not know (doc attributes,
             * typos): the lookup only runs once per field compile.
             */
            zend_class_entry *userland_ce = zend_lookup_class(attribute->name);
            if (userland_ce == NULL || !instanceof_function(userland_ce, AS_Fields_Constraint_ce)) {
                continue;
            }

            ce = userland_ce;
            validate = as_validate_userland_constraint;
        }

        if (count == capacity) {
            capacity = capacity == 0 ? 4 : capacity * 2;
            constraints = erealloc(constraints, capacity * sizeof(as_compiled_constraint));
        }

        if (!instantiate_constraint_attribute(&constraints[count].instance, ce, attribute, cf->declaring_ce)) {
            for (uint32_t i = 0; i < count; i++) {
                zval_ptr_dtor(&constraints[i].instance);
            }
            efree(constraints);
            return false;
        }

        constraints[count].validate = validate;
        count++;
    }
    ZEND_HASH_FOREACH_END();

    cf->constraints = constraints;
    cf->constraints_count = count;
    return true;
}

/* =========================================================================
 * Plan cache and compilation
 * ========================================================================= */

/* Class entry -> as_class_plans*, stored in the module globals (see as_globals.h) */

static uint32_t generator_index(char alias_generator)
{
    switch (alias_generator) {
        case AS_PASCAL_CASE:
            return 1;
        case AS_CAMEL_CASE:
            return 2;
        case AS_SNAKE_CASE:
            return 3;
        case AS_KEBAB_CASE:
            return 4;
        default:
            return 0;
    }
}

static void free_compiled_field(as_compiled_field *cf)
{
    if (cf->name != NULL) {
        zend_string_release(cf->name);
    }
    if (cf->arms != NULL) {
        efree(cf->arms);
    }
    if (cf->constraints != NULL) {
        for (uint32_t i = 0; i < cf->constraints_count; i++) {
            zval_ptr_dtor(&cf->constraints[i].instance);
        }
        efree(cf->constraints);
    }
}

static void free_compiled_model(as_compiled_model *plan)
{
    for (uint32_t i = 0; i < plan->count; i++) {
        free_compiled_field(&plan->fields[i]);
    }
    if (plan->fields != NULL) {
        efree(plan->fields);
    }
    efree(plan);
}

static void class_plans_dtor(zval *entry)
{
    as_class_plans *plans = Z_PTR_P(entry);

    for (uint32_t i = 0; i < 5; i++) {
        if (plans->by_generator[i] != NULL) {
            free_compiled_model(plans->by_generator[i]);
        }
    }
    efree(plans);
}

void as_clear_compiled_model_cache(void)
{
    if (AS_G(as_plans_cache) != NULL) {
        zend_hash_destroy(AS_G(as_plans_cache));
        efree(AS_G(as_plans_cache));
        AS_G(as_plans_cache) = NULL;
    }
}

static as_class_plans *get_class_plans(zend_class_entry *ce)
{
    as_class_plans *plans = NULL;

    if (AS_G(as_plans_cache) != NULL) {
        plans = zend_hash_index_find_ptr(AS_G(as_plans_cache), (zend_ulong)(uintptr_t)ce);
        if (plans != NULL) {
            return plans;
        }
    }

    plans = emalloc(sizeof(as_class_plans));
    memset(plans, 0, sizeof(as_class_plans));

    if (AS_G(as_plans_cache) == NULL) {
        AS_G(as_plans_cache) = emalloc(sizeof(HashTable));
        zend_hash_init(AS_G(as_plans_cache), 8, NULL, class_plans_dtor, 0);
    }

    return zend_hash_index_add_ptr(AS_G(as_plans_cache), (zend_ulong)(uintptr_t)ce, plans);
}

/*
 * Builds the plan skeleton: one slot per public non-static property of
 * the whole hierarchy, in the same order the interpreter walked it
 * (child class first). Slot contents are compiled lazily on first use.
 */
static as_compiled_model *compile_plan_skeleton(zend_class_entry *model_ce, char generator)
{
    uint32_t count = 0;

    for (zend_class_entry *ce = model_ce; ce != NULL && ce != AS_BaseModel_ce; ce = ce->parent) {
        zend_property_info *property;
        ZEND_HASH_FOREACH_PTR(&ce->properties_info, property)
        {
            (void)property;
            count++;
        }
        ZEND_HASH_FOREACH_END();
    }

    as_compiled_model *plan = emalloc(sizeof(as_compiled_model));
    memset(plan, 0, sizeof(as_compiled_model));
    plan->generator = generator;
    plan->count = 0;

    if (count == 0) {
        return plan;
    }

    plan->fields = safe_emalloc(count, sizeof(as_compiled_field), 0);
    memset(plan->fields, 0, count * sizeof(as_compiled_field));

    for (zend_class_entry *ce = model_ce; ce != NULL && ce != AS_BaseModel_ce; ce = ce->parent) {
        zend_string *property_name;
        zend_property_info *property;

        ZEND_HASH_FOREACH_STR_KEY_PTR(&ce->properties_info, property_name, property)
        {
            (void)property_name;
            if (property->flags & ZEND_ACC_STATIC)
                continue;
            if (property->flags & (ZEND_ACC_PROTECTED | ZEND_ACC_PRIVATE))
                continue;

            ZEND_ASSERT(plan->count < count);
            as_compiled_field *cf = &plan->fields[plan->count++];
            cf->prop = property;
            cf->declaring_ce = ce;
        }
        ZEND_HASH_FOREACH_END();
    }

    return plan;
}

/*
 * Compiles one field slot on first use: resolves the field name,
 * flattens the native type hint and precomputes the default-value
 * presence. A failure (pending exception from a bad Alias attribute)
 * leaves the slot uncompiled so the error keeps firing.
 */
static bool compile_field(as_compiled_field *cf, as_compiled_model *plan)
{
    as_property_info property_info = {
        .model = NULL,
        .model_ce = cf->declaring_ce,
        .property = cf->prop,
    };

    zend_string *resolved = get_property_name(&property_info, cf->prop->name, plan->generator);
    if (UNEXPECTED(EG(exception) != NULL)) {
        if (resolved != NULL && resolved != cf->prop->name) {
            zend_string_release(resolved);
        }
        return false;
    }
    if (resolved == NULL) {
        return false;
    }

    zend_type type = cf->prop->type;
    if (!ZEND_TYPE_IS_SET(type)) {
        cf->untyped = true;
    } else {
        // The native-hint containment bitmap: evaluating the engine's
        // own macro once per property replaces re-interpreting the
        // zend_type on every validation
        for (uint32_t code = 0; code < 16; code++) {
            if (ZEND_TYPE_CONTAINS_CODE(type, code)) {
                cf->contains_code |= (1u << code);
            }
        }

        // The arms reference the property's own type storage (the
        // zend_type for single types, its heap-allocated list for
        // unions/intersections): both live as long as the property_info
        cf->is_intersection_group = ZEND_TYPE_IS_INTERSECTION(type);
        const zend_type *arms_src;
        uint32_t arms_count;
        if (ZEND_TYPE_HAS_LIST(type)) {
            arms_src = ZEND_TYPE_LIST(type)->types;
            arms_count = ZEND_TYPE_LIST(type)->num_types;
        } else {
            arms_src = &cf->prop->type;
            arms_count = 1;
        }

        if (arms_count > 0) {
            cf->arms = safe_emalloc(arms_count, sizeof(as_compiled_arm), 0);
            memset(cf->arms, 0, arms_count * sizeof(as_compiled_arm));
            cf->arms_count = arms_count;

            for (uint32_t i = 0; i < arms_count; i++) {
                const zend_type *arm_type = &arms_src[i];
                as_compiled_arm *arm = &cf->arms[i];

                if (ZEND_TYPE_IS_INTERSECTION(*arm_type)) {
                    arm->kind = AS_ARM_INTERSECTION;
                    arm->origin = arm_type;
                } else if (ZEND_TYPE_HAS_NAME(*arm_type)) {
                    arm->kind = AS_ARM_CLASS;
                    arm->origin = arm_type;
                    arm->ce = as_get_ce_from_type(cf->prop, arm_type);
                } else {
                    arm->kind = AS_ARM_BASIC;
                    arm->mask = ZEND_TYPE_PURE_MASK(*arm_type);
                }
            }
        }
    }

    /*
     * Constraint attributes instantiate their objects once per field
     * compile. A failure leaves the slot uncompiled (the name is only
     * assigned below) so the constructor error keeps firing on every
     * validation.
     */
    if (!compile_field_constraints(cf)) {
        if (resolved != cf->prop->name) {
            zend_string_release(resolved);
        }
        return false;
    }

    // The plan owns one reference to a stable string
    cf->name = (resolved == cf->prop->name) ? zend_string_copy(cf->prop->name) : resolved;

    cf->has_default = has_property_default_value(&property_info);
    return true;
}

as_compiled_model *as_get_root_compiled_model(zval *model)
{
    zend_class_entry *ce = Z_OBJCE_P(model);
    as_class_plans *plans = get_class_plans(ce);

    // An existing root compile is reused as-is
    for (uint32_t i = 0; i < 5; i++) {
        as_compiled_model *plan = plans->by_generator[i];
        if (plan != NULL && plan->is_root) {
            return plan;
        }
    }

    zval configs_obj;
    as_model_configs_properties properties;
    as_get_model_configs(&configs_obj, model, &properties, false);
    if (UNEXPECTED(EG(exception) != NULL)) {
        return NULL;
    }

    const uint32_t index = generator_index(properties.alias_generator);

    as_compiled_model *plan = plans->by_generator[index];
    if (plan == NULL) {
        plan = compile_plan_skeleton(ce, properties.alias_generator);
        plans->by_generator[index] = plan;
    }

    plan->is_root = true;
    plan->configs = properties;
    plan->hooks_overridden = as_model_overrides_hooks(ce);

    return plan;
}

/* =========================================================================
 * Runtime
 * ========================================================================= */

bool as_compiled_validate_model(zval *raw_data, zval *model, as_model_configs_properties *properties, zval *errors, zend_string *parent_path)
{
    ZEND_ASSERT(Z_TYPE_P(raw_data) == IS_ARRAY);
    ZEND_ASSERT(Z_TYPE_P(errors) == IS_ARRAY);

    zend_class_entry *ce = Z_OBJCE_P(model);
    as_class_plans *plans = get_class_plans(ce);

    // The nested lookup reuses the root's slot when the generator
    // matches: the baked field names are identical
    const uint32_t index = generator_index(properties->alias_generator);
    as_compiled_model *plan = plans->by_generator[index];
    if (plan == NULL) {
        plan = compile_plan_skeleton(ce, properties->alias_generator);
        plans->by_generator[index] = plan;
    }

    for (uint32_t i = 0; i < plan->count; i++) {
        as_compiled_field *cf = &plan->fields[i];

        if (cf->name == NULL) {
            if (!compile_field(cf, plan)) {
                return false;
            }
        }

        // field->parent keeps the parent-excludes-own-name invariant:
        // error paths and the nested recursion compose the full dot
        // path only where it is needed
        as_field field = {
            .parent = parent_path,
            .name = cf->name,
        };

        field.value = zend_hash_find(Z_ARRVAL_P(raw_data), cf->name);
        if (field.value == NULL || Z_TYPE_P(field.value) == IS_UNDEF) {
            if (cf->has_default) {
                continue;
            }

            as_property_info property_info = {
                .model = model,
                .model_ce = cf->declaring_ce,
                .property = cf->prop,
            };
            as_add_field_error_with_prefix(AS_ERROR_REQUIRED, &field, &property_info, errors);
            if (properties->stop_first_error) {
                return false;
            }
            continue;
        }

        as_property_info property_info = {
            .model = model,
            .model_ce = cf->declaring_ce,
            .property = cf->prop,
        };
        const bool is_valid = as_validate_compiled_type_hint(&field, cf, &property_info, properties, errors);

        if (!is_valid) {
            if (properties->stop_first_error) {
                return false;
            }
            continue;
        }

        /* Constraints run on the (possibly coerced) value; a failing
         * constraint keeps the value out of the model. Every declared
         * constraint reports its own error unless stopFirstError
         * aborts the run. */
        if (cf->constraints_count > 0) {
            bool constraints_valid = true;

            for (uint32_t c = 0; c < cf->constraints_count; c++) {
                as_compiled_constraint *compiled = &cf->constraints[c];
                if (!compiled->validate(&compiled->instance, &field, &property_info, errors)) {
                    constraints_valid = false;
                    if (properties->stop_first_error) {
                        return false;
                    }
                }

                /* A constraint may throw instead of reporting an error
                 * (for example a throwing userland constraint): let the
                 * exception propagate. */
                if (UNEXPECTED(EG(exception) != NULL)) {
                    return false;
                }
            }

            if (!constraints_valid) {
                continue;
            }
        }

        // Direct offset write: the value is already of the declared
        // type, so the property name lookup and the type
        // re-verification of zend_update_property() are redundant
        zval *property_slot = OBJ_PROP(Z_OBJ_P(model), cf->prop->offset);
        zval_ptr_dtor(property_slot);
        ZVAL_COPY(property_slot, field.value);
    }

    return zend_hash_num_elements(Z_ARRVAL_P(errors)) == 0;
}
