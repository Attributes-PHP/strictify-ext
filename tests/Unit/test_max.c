#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_max.h"
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
TEST_CASE(9, 10)
TEST_CASE(0, 0)
TEST_CASE(-10, -5)
void test_max_accepts_numbers_at_most_the_bound(zend_long num, zend_long bound)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, bound);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_long(num);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Max_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

TEST_CASE(11, 10)
TEST_CASE(-4, -5)
void test_max_rejects_numbers_above_the_bound(zend_long num, zend_long bound)
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
    as_add_field_error_with_expected_Expect(AS_ERROR_MAX, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Max_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_max_rejects_non_numeric_values(void)
{
    zval *constraint = stub_make_constraint();
    zval bound_zv;
    ZVAL_LONG(&bound_zv, 10);
    stub_set_property("value", &bound_zv);

    zval *value = stub_make_string("5");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&bound_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_MAX, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Max_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(constraint->value.obj);
    free(constraint);
}
