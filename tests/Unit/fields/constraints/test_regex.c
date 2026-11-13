#include "unity.h"
#include "test_helpers.h"
#include "constraint_stubs.h"
#include "helpers/mock_as_wrappers.h"
#include "helpers/mock_as_error_messages.h"
#include "fields/constraints/as_regex.h"
#include <stdlib.h>
#include <string.h>

void setUp(void)
{
    as_string_release_Stub(stub_release_noop);
    as_string_concat3_Stub(stub_concat3_static);
    stub_clear_properties();
    stub_init_expected_string();
    stub_pcre_cache = NULL;
    stub_pcre_match_result = 0;
}

void tearDown(void)
{}

static as_field make_field(zval *value)
{
    as_field field = {NULL, NULL, value};
    return field;
}

static zval *make_constraint_with_pattern(const char *pattern, zval *pattern_zv)
{
    ZVAL_STR(pattern_zv, zend_string_init(pattern, strlen(pattern), 0));
    stub_set_property("regex", pattern_zv);
    return stub_make_constraint();
}

void test_regex_accepts_matching_string(void)
{
    zval pattern_zv;
    zval *constraint = make_constraint_with_pattern("/^a+$/", &pattern_zv);

    stub_pcre_cache = (void *)1; // pattern compiles
    stub_pcre_match_result = 1;  // preg_match() returns 1

    zval *value = stub_make_string("aaa");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    TEST_ASSERT_TRUE(as_validate_Regex_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(pattern_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}

void test_regex_rejects_non_matching_string(void)
{
    zval pattern_zv;
    zval *constraint = make_constraint_with_pattern("/^a+$/", &pattern_zv);

    stub_pcre_cache = (void *)1;
    stub_pcre_match_result = 0; // no match

    zval *value = stub_make_string("bbb");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&pattern_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_REGEX, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Regex_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(pattern_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}

void test_regex_rejects_when_pattern_fails_to_compile(void)
{
    zval pattern_zv;
    zval *constraint = make_constraint_with_pattern("/(/", &pattern_zv);

    stub_pcre_cache = NULL; // pcre_get_compiled_regex_cache fails

    zval *value = stub_make_string("aaa");
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&pattern_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_REGEX, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Regex_constraint(constraint, &field, NULL, &errors));

    free(Z_STR_P(value));
    free(value);
    free(pattern_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}

void test_regex_rejects_non_string_values(void)
{
    zval pattern_zv;
    zval *constraint = make_constraint_with_pattern("/^a+$/", &pattern_zv);

    zval *value = stub_make_long(5);
    as_field field = make_field(value);
    zval errors;
    ZVAL_NULL(&errors);

    as_value_to_string_ExpectAndReturn(&pattern_zv, stub_expected_string);
    as_add_field_error_with_expected_Expect(AS_ERROR_REGEX, &field, NULL, &errors, stub_expected_string);
    TEST_ASSERT_FALSE(as_validate_Regex_constraint(constraint, &field, NULL, &errors));

    free(value);
    free(pattern_zv.value.str);
    free(constraint->value.obj);
    free(constraint);
}
