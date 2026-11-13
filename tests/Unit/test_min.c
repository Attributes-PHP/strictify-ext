#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_min.h"
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

TEST_CASE(10, 10)
TEST_CASE(11, 10)
TEST_CASE(10, 0)
TEST_CASE(-5, -10)
void test_min_accepts_numbers_at_least_the_bound(zend_long num, zend_long bound)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, bound);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_long(num);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Min_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

TEST_CASE(9, 10)
TEST_CASE(-11, -10)
void test_min_rejects_numbers_below_the_bound(zend_long num, zend_long bound)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, bound);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_long(num);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&bound_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_MIN, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Min_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_min_accepts_float_bound_and_value(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_DOUBLE(&bound_zv, 2.5);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_double(2.5);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Min_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_min_rejects_non_numeric_values(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, 10);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_string("10");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&bound_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_MIN, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Min_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(constraint->value.obj);
    free(constraint);
}
