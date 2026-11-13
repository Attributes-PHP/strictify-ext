#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_size.h"
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

static zval *make_constraint(const zval *min_zv, const zval *max_zv)
{
    stub_clear_properties();
    stub_add_property("min", (zval *)min_zv);
    stub_add_property("max", (zval *)max_zv);
    return stub_make_constraint();
}

static zval *make_null(void)
{
    zval *zv = malloc(sizeof(zval));
    ZVAL_NULL(zv);
    return zv;
}

void test_size_without_bounds_always_passes(void)
{
    zval *null_min = make_null();
    zval *null_max = make_null();
    zval *constraint = make_constraint(null_min, null_max);

    zval *value = stub_make_string("anything");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Size_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(null_min);
    free(null_max);
    free(constraint->value.obj);
    free(constraint);
}

void test_size_accepts_string_within_bounds(void)
{
    zval min_zv, max_zv;
    ZVAL_LONG(&min_zv, 2);
    ZVAL_LONG(&max_zv, 4);
    zval *constraint = make_constraint(&min_zv, &max_zv);

    zval *value = stub_make_string("abc"); // 3 bytes
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Size_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_size_rejects_string_below_min(void)
{
    zval min_zv, max_zv;
    ZVAL_LONG(&min_zv, 4);
    ZVAL_LONG(&max_zv, 10);
    zval *constraint = make_constraint(&min_zv, &max_zv);

    zval *value = stub_make_string("abc");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&min_zv, stub_expected_string);
    as_value_to_string_ExpectAndReturn(&max_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_SIZE, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Size_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_size_rejects_string_above_max_with_max_only(void)
{
    zval *null_min = make_null();
    zval max_zv;
    ZVAL_LONG(&max_zv, 2);
    zval *constraint = make_constraint(null_min, &max_zv);

    zval *value = stub_make_string("abc");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&max_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_SIZE, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Size_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(null_min);
    free(constraint->value.obj);
    free(constraint);
}

void test_size_counts_array_elements(void)
{
    zval min_zv, max_zv;
    ZVAL_LONG(&min_zv, 2);
    ZVAL_LONG(&max_zv, 3);
    zval *constraint = make_constraint(&min_zv, &max_zv);

    zval items[2];
    ZVAL_LONG(&items[0], 1);
    ZVAL_LONG(&items[1], 2);
    zval value;
    stub_make_array(&value, items, 2);
    as_field field = make_field(&value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Size_constraint(constraint, &field, NULL, &errors));

    free(Z_ARRVAL(value));
    free(constraint->value.obj);
    free(constraint);
}

void test_size_rejects_non_string_non_array_values(void)
{
    zval min_zv;
    ZVAL_LONG(&min_zv, 1);
    zval *null_max = make_null();
    zval *constraint = make_constraint(&min_zv, null_max);

    zval *value = stub_make_long(5);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&min_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_SIZE, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Size_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(null_max);
    free(constraint->value.obj);
    free(constraint);
}
