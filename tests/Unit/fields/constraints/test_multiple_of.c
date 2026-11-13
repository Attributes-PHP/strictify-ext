#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_multiple_of.h"
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

TEST_CASE(10, 5)
TEST_CASE(10, 1)
TEST_CASE(0, 5)
TEST_CASE(-9, 3)
void test_multiple_of_accepts_integer_multiples(zend_long num, zend_long bound)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, bound);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_long(num);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_MultipleOf_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

TEST_CASE(7, 5)
TEST_CASE(-7, 5)
void test_multiple_of_rejects_integer_non_multiples(zend_long num, zend_long bound)
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
    as_add_field_error_with_expected_Expect(AS_ERROR_MULTIPLE_OF, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_MultipleOf_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_multiple_of_zero_bound_rejects_non_zero(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, 0);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_long(3);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&bound_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_MULTIPLE_OF, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_MultipleOf_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_multiple_of_zero_bound_accepts_zero(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, 0);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_long(0);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_MultipleOf_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_multiple_of_accepts_float_multiple_within_epsilon(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_DOUBLE(&bound_zv, 0.1);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_double(0.3); // fmod noise stays within tolerance
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_MultipleOf_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_multiple_of_rejects_float_non_multiple(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_DOUBLE(&bound_zv, 0.1);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_double(0.35);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&bound_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_MULTIPLE_OF, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_MultipleOf_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_multiple_of_rejects_non_numeric_values(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, 2);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_string("4");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&bound_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_MULTIPLE_OF, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_MultipleOf_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(constraint->value.obj);
    free(constraint);
}
