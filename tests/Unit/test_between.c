#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_between.h"
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

static zval *make_constraint(zend_long min, zend_long max, zval *min_zv, zval *max_zv)
{
    ZVAL_LONG(min_zv, min);
    ZVAL_LONG(max_zv, max);
    stub_clear_properties();
    stub_add_property("min", min_zv);
    stub_add_property("max", max_zv);
    return stub_make_constraint();
}

TEST_CASE(1, 5, 1)
TEST_CASE(1, 5, 5)
TEST_CASE(1, 5, 3)
TEST_CASE(-10, 10, 0)
void test_between_accepts_numbers_within_bounds(zend_long min, zend_long max, zend_long num)
{
    zval min_zv, max_zv;
    zval *constraint = make_constraint(min, max, &min_zv, &max_zv);

    zval *value = stub_make_long(num);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Between_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

TEST_CASE(1, 5, 0)
TEST_CASE(1, 5, 6)
TEST_CASE(-10, 10, -11)
void test_between_rejects_numbers_outside_bounds(zend_long min, zend_long max, zend_long num)
{
    zval min_zv, max_zv;
    zval *constraint = make_constraint(min, max, &min_zv, &max_zv);

    zval *value = stub_make_long(num);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&min_zv, stub_expected_string);
    as_value_to_string_ExpectAndReturn(&max_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_BETWEEN, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Between_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_between_rejects_non_numeric_values_with_type_error(void)
{
    zval min_zv, max_zv;
    zval *constraint = make_constraint(1, 5, &min_zv, &max_zv);

    zval *value = stub_make_string("3");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&min_zv, stub_expected_string);
    as_value_to_string_ExpectAndReturn(&max_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_BETWEEN_TYPE, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Between_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_between_accepts_float_bounds(void)
{
    zval min_zv, max_zv;
    ZVAL_DOUBLE(&min_zv, 0.5);
    ZVAL_DOUBLE(&max_zv, 2.5);
    stub_clear_properties();
    stub_add_property("min", &min_zv);
    stub_add_property("max", &max_zv);
    zval *constraint = stub_make_constraint();

    zval *value = stub_make_double(1.5);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Between_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}
