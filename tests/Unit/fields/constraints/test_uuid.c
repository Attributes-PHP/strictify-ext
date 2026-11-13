#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_uuid.h"
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

TEST_CASE("123e4567-e89b-12d3-a456-426614174000")
TEST_CASE("123E4567-E89B-12D3-A456-426614174000")
TEST_CASE("00000000-0000-0000-0000-000000000000")
void test_uuid_accepts_canonical_uuids(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Uuid_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

TEST_CASE("123e4567-e89b-12d3-a456-42661417400")   // 35 chars
TEST_CASE("123e4567e89b12d3a456426614174000")      // no dashes, 32 chars
TEST_CASE("123e4567-e89b-12d3-a456-4266141740000") // 37 chars
void test_uuid_rejects_wrong_length(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_UUID, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Uuid_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

TEST_CASE("123e4567xe89b-12d3-a456-426614174000") // dash position 8 is not '-'
TEST_CASE("123e4567-e89b-12d3-a456_426614174000") // dash position 23 is not '-'
TEST_CASE("123e4567-e89b-12d3-g456-426614174000") // 'g' is not hex
void test_uuid_rejects_malformed_uuids(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_UUID, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Uuid_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

void test_uuid_rejects_non_string_values(void)
{
    zval *value = stub_make_long(42);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_UUID, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Uuid_constraint(NULL, &field, NULL, &errors));

    free(value);
}
