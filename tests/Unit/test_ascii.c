#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_ascii.h"
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

TEST_CASE("hello")
TEST_CASE("")
TEST_CASE("~!@# 0x7F DEL \x7f")
void test_ascii_accepts_7bit_strings(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Ascii_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

TEST_CASE("h\xc3\xa9llo") // é in UTF-8: bytes > 0x7F
TEST_CASE("\x80")
void test_ascii_rejects_non_ascii_bytes(const char *input)
{
    zval *value = stub_make_string(input);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_ASCII, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Ascii_constraint(NULL, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
}

void test_ascii_rejects_non_string_values(void)
{
    zval *value = stub_make_long(65);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_add_field_error_with_prefix_Expect(AS_ERROR_ASCII_TYPE, &field, NULL, &errors);
    TEST_ASSERT_FALSE(as_validate_Ascii_constraint(NULL, &field, NULL, &errors));

    free(value);
}
