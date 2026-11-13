#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_ulid.h"
#include <stdlib.h>

void setUp(void)
{
    as_string_release_Stub(stub_release_noop);
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

TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5FAV")
TEST_CASE("01arz3ndektsv4rrffq69g5fav")
TEST_CASE("7ZZZZZZZZZZZZZZZZZZZZZZZZZ")
void test_ulid_accepts_valid_ulids(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Ulid_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5FA")   // 25 chars
TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5FAVX") // 27 chars
TEST_CASE("")                            // empty
void test_ulid_rejects_wrong_length(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_ULID, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Ulid_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5IAV") // contains I
TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5lAV") // contains l
TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5OAV") // contains O
TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5uAV") // contains u
TEST_CASE("01ARZ3NDEKTSV4RRFFQ69G5-AV") // contains dash
void test_ulid_rejects_invalid_characters(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_ULID, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Ulid_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

void test_ulid_rejects_non_string_values(void)
{
    zval *value = stub_make_long(123);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_ULID, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Ulid_constraint(NULL, &field, NULL, &errors));

    free(value);
}
