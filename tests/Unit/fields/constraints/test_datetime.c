#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_datetime.h"
#include <stdlib.h>
#include <string.h>

void setUp(void)
{
    as_string_release_Stub(stub_release_noop);
    as_string_concat3_Stub(stub_concat3_static);
    stub_clear_properties();
    stub_init_expected_string();
    stub_instanceof_result = false;
    stub_date_initialize_result = false;
    stub_date_format_seen = NULL;
    stub_date_time_seen = NULL;
}

void tearDown(void)
{}

static as_field make_field(zval *value)
{
    as_field field = {NULL, NULL, value};
    return field;
}

static zval *make_constraint_with_format(const char *format, zval *format_zv)
{
    ZVAL_STR(format_zv, zend_string_init(format, strlen(format), 0));
    zval *tz_obj = malloc(sizeof(zval));
    ZVAL_NULL(tz_obj);
    stub_clear_properties();
    stub_add_property("format", format_zv);
    stub_add_property("timezone_obj", tz_obj);
    return stub_make_constraint();
}

void test_datetime_accepts_datetime_interface_instances(void)
{
    zval *constraint = stub_make_constraint();
    stub_instanceof_result = true;

    zval *value = malloc(sizeof(zval));
    ZVAL_OBJ(value, calloc(1, sizeof(zend_object)));
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_DateTime_constraint(constraint, &field, NULL, &errors));

    free(value->value.obj);
    free(value);
    free(constraint->value.obj);
    free(constraint);
}

void test_datetime_parses_string_with_declared_format(void)
{
    zval format_zv;
    zval *constraint = make_constraint_with_format("Y-m-d", &format_zv);

    stub_date_initialize_result = true;

    zval *value = stub_make_string("1994-01-01");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_DateTime_constraint(constraint, &field, NULL, &errors));
    TEST_ASSERT_EQUAL_STRING("Y-m-d", stub_date_format_seen);
    TEST_ASSERT_EQUAL_STRING("1994-01-01", stub_date_time_seen);

    free(Z_STR_P(value));
    free(value);
    free(format_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}

void test_datetime_rejects_unparseable_string(void)
{
    zval format_zv;
    zval *constraint = make_constraint_with_format("Y-m-d", &format_zv);

    stub_date_initialize_result = false;

    zval *value = stub_make_string("not a date");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&format_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_DATETIME, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_DateTime_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(format_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}

void test_datetime_rejects_non_string_non_datetime_values(void)
{
    zval format_zv;
    zval *constraint = make_constraint_with_format("Y-m-d", &format_zv);

    zval *value = stub_make_long(19940101);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&format_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_DATETIME, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_DateTime_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(format_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}

void test_datetime_rejects_object_not_implementing_datetime_interface(void)
{
    zval format_zv;
    zval *constraint = make_constraint_with_format("Y-m-d", &format_zv);

    stub_instanceof_result = false;

    zval *value = malloc(sizeof(zval));
    ZVAL_OBJ(value, calloc(1, sizeof(zend_object)));
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&format_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_DATETIME, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_DateTime_constraint(constraint, &field, NULL, &errors));

    free(value->value.obj);
    free(value);
    free(format_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}
