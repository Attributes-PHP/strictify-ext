#ifndef AS_FIELDS_Field_H
#define AS_FIELDS_Field_H

#include "php.h"
#include "Zend/zend_attributes.h"

extern zend_class_entry *AS_Fields_Field_ce;

static const zend_function_entry class_AS_Fields_Field_methods[] = {ZEND_FE_END};

/* Registration function */
void as_register_Field_interface(void);

/*
 * Registers a constraint class: final, readonly, implementing Field, usable
 * as an attribute on properties and parameters. The variadic arguments are
 * the constructor's typed property declarations, if any.
 */
/* clang-format off */
#define AS_REGISTER_CONSTRAINT_CLASS(Name, flags, ...)                                                                             \
    void as_register_##Name##_class(void)                                                                                          \
    {                                                                                                                              \
        zend_class_entry ce;                                                                                                       \
                                                                                                                                   \
        INIT_NS_CLASS_ENTRY(ce, "Attributes\\Strictify\\Fields\\Constraints", #Name, class_AS_Fields_Constraints_##Name##_methods); \
        AS_Fields_Constraints_##Name##_ce = zend_register_internal_class_ex(&ce, NULL);                                             \
        ZEND_ASSERT(AS_Fields_Constraints_##Name##_ce != NULL);                                                                    \
        AS_Fields_Constraints_##Name##_ce->ce_flags |= flags;                                                                       \
                                                                                                                                   \
        ZEND_ASSERT(AS_Fields_Field_ce != NULL);                                                                                   \
        zend_class_implements(AS_Fields_Constraints_##Name##_ce, 1, AS_Fields_Field_ce);                                            \
                                                                                                                                   \
        __VA_ARGS__                                                                                                                \
        zend_internal_attribute_register(AS_Fields_Constraints_##Name##_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER); \
    }
/* clang-format on */

#endif /* AS_FIELDS_Alias_H */