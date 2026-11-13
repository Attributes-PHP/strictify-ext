#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_alpha_dash.h"
#include <stdlib.h>

void setUp(void)
{
    as_string_release_Stub(string_release_stub);
    stub_clear_properties();
}

void tearDown(void)
{}

static as_field make_field(zval *value)
{
    as_field field;
    field.parent = NULL;
    field.name = NULL;
    field.value = value;
    return field;
}

TEST_CASE("abc-DEF_123")
TEST_CASE("")
TEST_CASE("a")
void test_alpha_dash_accepts_valid_strings(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_AlphaDash_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

TEST_CASE("ab c")
TEST_CASE("hello!")
TEST_CASE("with.dot")
void test_alpha_dash_rejects_invalid_characters(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_ALPHA_DASH, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_AlphaDash_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

void test_alpha_dash_rejects_non_string_values(void)
{
    zval *value = stub_make_long(42);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_ALPHA_DASH_TYPE, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_AlphaDash_constraint(NULL, &field, NULL, &errors));

    free(value);
}
