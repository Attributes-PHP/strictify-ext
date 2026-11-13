#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_in.h"
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

void test_in_accepts_long_member(void)
{
    zval items[2];
    ZVAL_LONG(&items[0], 1);
    ZVAL_LONG(&items[1], 2);
    zval values;
    zval *constraint = make_constraint_with_values(items, 2, &values);

    zval *value = stub_make_long(2);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_In_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}

void test_in_accepts_string_member(void)
{
    zval items[2];
    ZVAL_STR(&items[0], zend_string_init("a", 1, 0));
    ZVAL_STR(&items[1], zend_string_init("b", 1, 0));
    zval values;
    zval *constraint = make_constraint_with_values(items, 2, &values);

    zval *value = stub_make_string("b");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_In_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(items[0].value.str);
    free(items[1].value.str);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}

void test_in_rejects_non_member_with_list_error(void)
{
    zval items[2];
    ZVAL_LONG(&items[0], 1);
    ZVAL_LONG(&items[1], 2);
    zval values;
    zval *constraint = make_constraint_with_values(items, 2, &values);

    zval *value = stub_make_long(3);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_values_to_list_string_ExpectAndReturn(&values, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_IN, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_In_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}

void test_in_uses_strict_comparison(void)
{
    zval items[1];
    ZVAL_LONG(&items[0], 1);
    zval values;
    zval *constraint = make_constraint_with_values(items, 1, &values);

    // "1" (string) is not identical to 1 (int)
    zval *value = stub_make_string("1");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_values_to_list_string_ExpectAndReturn(&values, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_IN, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_In_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(Z_ARRVAL(values));
    free(constraint->value.obj);
    free(constraint);
}
