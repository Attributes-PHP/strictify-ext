#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_starts_with.h"
#include <stdlib.h>

void setUp(void)
{
    as_string_release_Stub(stub_release_noop);
    as_string_concat3_Stub(stub_concat3_static);
    stub_clear_properties();
    stub_init_expected_string();
}

void tearDown(void)
{}

static as_field make_field(zval *value)
{
    as_field field = {NULL, NULL, value};
    return field;
}

static zval *make_constraint_with_values(const zval *items, uint32_t count, zval *values_storage)
{
    stub_make_array(values_storage, items, count);
    stub_set_property("values", values_storage);
    return stub_make_constraint();
}

void test_starts_with_accepts_matching_prefix(void)
{
    zval items[2];
    ZVAL_STR(&items[0], zend_string_init("pre", 3, 0));
    ZVAL_STR(&items[1], zend_string_init("hello", 5, 0));
    zval values;
    zval *constraint = make_constraint_with_values(items, 2, &values);

    zval *value = stub_make_string("hello world");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_StartsWith_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(items[0].value.str);
    free(items[1].value.str);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}

void test_starts_with_matches_numeric_prefix_by_string_representation(void)
{
    zval items[1];
    ZVAL_LONG(&items[0], 42);
    zval values;
    zval *constraint = make_constraint_with_values(items, 1, &values);

    zval *value = stub_make_string("4242");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_StartsWith_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}

void test_starts_with_rejects_non_matching_prefix(void)
{
    zval items[1];
    ZVAL_STR(&items[0], zend_string_init("xyz", 3, 0));
    zval values;
    zval *constraint = make_constraint_with_values(items, 1, &values);

    zval *value = stub_make_string("hello");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_values_to_list_string_ExpectAndReturn(&values, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_STARTS_WITH, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_StartsWith_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(items[0].value.str);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}

void test_starts_with_rejects_prefix_longer_than_value(void)
{
    zval items[1];
    ZVAL_STR(&items[0], zend_string_init("hello world!", 12, 0));
    zval values;
    zval *constraint = make_constraint_with_values(items, 1, &values);

    zval *value = stub_make_string("hello");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_values_to_list_string_ExpectAndReturn(&values, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_STARTS_WITH, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_StartsWith_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(items[0].value.str);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}

void test_starts_with_rejects_non_string_values(void)
{
    zval items[1];
    ZVAL_STR(&items[0], zend_string_init("a", 1, 0));
    zval values;
    zval *constraint = make_constraint_with_values(items, 1, &values);

    zval *value = stub_make_long(1);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_values_to_list_string_ExpectAndReturn(&values, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_STARTS_WITH, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_StartsWith_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(items[0].value.str);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}
