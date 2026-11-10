#ifndef AS_COMPILED_MODEL_H
#define AS_COMPILED_MODEL_H

#include "php.h"
#include "as_model_configs.h"
#include "helpers/as_structs.h"

/*
 * Compiled form of a BaseModel class, built once per class per alias
 * generator on first validation and reused for every subsequent
 * validate() call of the request.
 *
 * The plan flattens the per-call interpretation the validator used to
 * perform: the properties_info walk over the class hierarchy, the flag
 * checks, the field-name resolution and the zend_type interpretation.
 * At steady state, validating a model means one cache lookup by class
 * entry plus a flat iteration over the field slots.
 */

/* Forward declaration: the array attribute spec, owned by the spec cache */
typedef struct as_spec as_spec;

/*
 * A constraint attribute instantiated once per field compile: the
 * instance zval is owned by the plan, the validator runs the
 * constraint's logic against the field value at validation time.
 */
typedef bool (*as_constraint_validator_t)(zval *constraint, as_field *field, as_property_info *property, zval *errors);

typedef struct {
    zval instance; // owned constraint object, constructed via its C constructor
    as_constraint_validator_t validate;
} as_compiled_constraint;

typedef enum {
    AS_ARM_BASIC = 1,    // arm->mask holds the pure type mask
    AS_ARM_CLASS,        // arm->ce / arm->origin hold the class arm
    AS_ARM_INTERSECTION, // arm->origin holds the intersection type
} as_compiled_arm_kind;

typedef struct {
    uint8_t kind;
    uint32_t mask;           // AS_ARM_BASIC
    zend_class_entry *ce;    // AS_ARM_CLASS: resolved CE, NULL while unresolved
    const zend_type *origin; // AS_ARM_CLASS / AS_ARM_INTERSECTION: the original type arm
} as_compiled_arm;

typedef enum {
    AS_ARRAY_UNPROBED, // array attribute checks not performed yet
    AS_ARRAY_ABSENT,   // property declares no array type-hint attributes
    AS_ARRAY_SPEC,     // array_spec borrowed from the spec cache
} as_array_state;

typedef struct {
    zend_property_info *prop;
    zend_class_entry *declaring_ce;

    zend_string *name;      // resolved field name, owned; NULL until compiled
    uint32_t contains_code; // bit per type code accepted by the native hint
    as_compiled_arm *arms;  // flattened zend_type arms, NULL when untyped
    uint32_t arms_count;
    bool untyped;
    bool is_intersection_group; // pure intersection hint: every class arm must match
    bool has_default;

    as_array_state array_state;
    as_spec *array_spec; // borrowed, owned by the array spec cache

    as_compiled_constraint *constraints; // owned, NULL when the field declares none
    uint32_t constraints_count;
} as_compiled_field;

typedef struct {
    as_compiled_field *fields;
    uint32_t count;
    char generator;                      // alias generator the field names are compiled under
    bool is_root;                        // compiled for a top-level validate() call
    as_model_configs_properties configs; // valid when is_root
    bool hooks_overridden;
} as_compiled_model;

/*
 * Plans per class entry, one per alias generator the class has been
 * validated under. Index 0 is the "no generator" slot; a root compile
 * stores its configs and reuses the slot of its own generator.
 */
typedef struct {
    as_compiled_model *by_generator[5];
} as_class_plans;

/*
 * Returns the root plan for the model's class, compiling the skeleton
 * and resolving ModelConfigs on first use. Returns NULL with a pending
 * exception when the configs cannot be parsed.
 */
as_compiled_model *as_get_root_compiled_model(zval *model);

/*
 * Validates raw_data against the model's compiled plan. properties are
 * the configs in effect (the root model's, propagated into nested
 * validation), matching the previous semantics.
 */
bool as_compiled_validate_model(zval *raw_data, zval *model, as_model_configs_properties *properties, zval *errors, zend_string *parent_path);
bool as_compiled_model_to_dict(zval *model, zval *return_value);

void as_clear_compiled_model_cache(void);
void as_clear_field_name_cache(void);

#endif /* AS_COMPILED_MODEL_H */
