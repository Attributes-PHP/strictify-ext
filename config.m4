PHP_ARG_ENABLE(attributes-strictify, whether to enable attributes-strictify support,
[  --enable-attributes-strictify   Enable attributes-strictify support])

if test "$PHP_ATTRIBUTES_STRICTIFY" != "no"; then
  PHP_REQUIRE_CC()
  
  # Require PHP 8.3 or later (ZEND_MODULE_API_NO 20230831)
  if test "$PHP_API_VERSION" -lt 20230831; then
    AC_MSG_ERROR([PHP 8.3 or later is required])
  fi

  dnl The Regex constraint compiles patterns through ext/pcre, whose
  dnl php_pcre.h includes pcre2.h. Package-managed PHP builds (Homebrew,
  dnl some Linux distributions) install pcre2.h outside the PHP include
  dnl tree, so probe the usual locations.
  AC_MSG_CHECKING([for pcre2.h])
  PCRE2_INCLUDE=""
  for PCRE2_DIR in "$PHP_PREFIX/include" "$PHP_PREFIX/../include" /usr/include /usr/local/include /opt/homebrew/include; do
    if test -f "$PCRE2_DIR/pcre2.h"; then
      PCRE2_INCLUDE="$PCRE2_DIR"
      break
    fi
  done
  if test -z "$PCRE2_INCLUDE"; then
    AC_MSG_ERROR([pcre2.h not found; install the PCRE2 development headers])
  fi
  AC_MSG_RESULT([$PCRE2_INCLUDE])
  PHP_ADD_INCLUDE([$PCRE2_INCLUDE])
  
  PHP_ADD_LIBRARY(stdc++, 1, ATTRIBUTES_STRICTIFY_SHARED_LIBADD)
  dnl The DateTime coercion in src/validators/as_typehint_validator.c calls
  dnl date_create_from_format() and resolves DateTimeInterface, so the build
  dnl requires ext/date (always compiled into PHP, declared here for ordering).
  PHP_ADD_EXTENSION_DEP(attributes_strictify, date, true)
  PHP_NEW_EXTENSION(attributes_strictify, strictify.c src/as_validate_function.c src/as_compiled_model.c src/as_call_function.c src/as_base_model.c src/as_exception.c src/as_model_configs.c src/as_error_translator.c src/as_type.c src/helpers/as_wrappers.c src/helpers/as_options.c src/helpers/as_string.c src/helpers/as_error_messages.c src/fields/as_constraint.c src/fields/as_config_aware_constraint.c src/fields/as_field.c src/fields/as_alias.c src/fields/as_error_message.c src/fields/as_sequence.c src/fields/as_union.c src/fields/as_intersection.c src/fields/as_dict.c src/fields/as_sensitive.c src/fields/constraints/as_alpha_dash.c src/fields/constraints/as_ascii.c src/fields/constraints/as_between.c src/fields/constraints/as_contains.c src/fields/constraints/as_datetime.c src/fields/constraints/as_ends_with.c src/fields/constraints/as_in.c src/fields/constraints/as_max.c src/fields/constraints/as_min.c src/fields/constraints/as_multiple_of.c src/fields/constraints/as_regex.c src/fields/constraints/as_size.c src/fields/constraints/as_starts_with.c src/fields/constraints/as_ulid.c src/fields/constraints/as_uuid.c src/fields/constraints/as_email.c src/fields/constraints/as_url.c src/fields/constraints/as_domain.c src/fields/constraints/as_ip.c src/fields/constraints/as_iban.c src/fields/constraints/as_isbn.c src/fields/constraints/as_credit_card.c src/fields/constraints/as_version.c src/fields/constraints/as_mac_address.c src/fields/constraints/as_alnum.c src/fields/constraints/as_alpha.c src/fields/constraints/as_digit.c src/fields/constraints/as_xdigit.c src/fields/constraints/as_slug.c src/fields/constraints/as_base64.c src/fields/constraints/as_json.c src/fields/constraints/as_hex_rgb_color.c src/fields/constraints/as_even.c src/fields/constraints/as_odd.c src/fields/constraints/as_positive.c src/fields/constraints/as_negative.c src/fields/constraints/as_prime_number.c src/fields/constraints/as_contains_any.c src/validators/as_typehint_validator.c src/validators/as_array_typehint_validator.c, $ext_shared)
fi
