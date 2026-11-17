---
title: "Exceptions"
description: "Reference for the exceptions Strictify throws: ValidationException and its getErrors() structure, ValueError cases, and the default error message catalog with translation keys."
---

# Exceptions

## ValidationException

`Attributes\Strictify\Exceptions\ValidationException` is thrown by [validate()](/validate/) and [call()](/call/) when any field fails validation. Call `getErrors()` for the error list:

```php
<?php
use Attributes\Strictify\Exceptions\ValidationException;
use function Attributes\Strictify\validate;

try {
    validate($rawData, new User());
} catch (ValidationException $e) {
    $e->getErrors();
    // [
    //     'name' => ['Field is required'],
    //     'address.street' => ['Must be string'],
    // ]
}
```

`getErrors()` returns an array keyed by field path, each value holding one or more messages. Nested structures report dot-notation paths, including array element keys:

```php
<?php
// ['users.0.email' => ['Must be string'], 'scores.5' => ['Must be integer']]
```

Field paths use the resolved field names — the [alias](/guides/aliases/) when one is declared.

## ValueError

A `ValueError` is thrown when an attribute spec is malformed, for example:

- an unresolvable class-name arm in `Union`/`Intersection`/`Dict`,
- an unsupported `Type` value in an array attribute,
- a `Dict` key type outside integer and string,
- an invalid `ModelConfigs` option value (the message lists the valid options),
- an invalid timezone name in a `DateTime` constraint (at construction).

## Default error messages

Default messages follow the `Must be <type>` pattern, joined for unions (`Must be boolean, integer or string`). Each has a stable translation key passed to the translator registered with `ErrorTranslator::set()` — see [Error messages](/guides/error-messages/):

| Key | Default message |
| --- | --- |
| `required` | `Field is required` |
| `type` | `Must be {expected}` |
| `alpha_dash` / `alpha_dash_type` | `Must only contain letters, numbers, dashes and underscores` |
| `ascii` / `ascii_type` | `Must only contain ASCII characters` |
| `between` / `between_type` | `Must be between {expected}` |
| `contains` | `Must contain {expected}` |
| `datetime` | `Must be a datetime in the format {expected}` |
| `ends_with` | `Must end with one of: {expected}` |
| `in` | `Must be one of: {expected}` |
| `max` / `min` | `Must be at most/least {expected}` |
| `multiple_of` | `Must be a multiple of {expected}` |
| `regex` | `Must match the pattern {expected}` |
| `size` | `Must have a size {expected}` |
| `starts_with` | `Must start with one of: {expected}` |
| `ulid` / `uuid` | `Must be a valid ULID/UUID` |
| `union` | `Must be {expected}` |
| `extra` | `Extra inputs are not permitted` (unknown key under `extra: 'forbid'`) |

Placeholders: `{field}` (resolved field name), `{value}` (the rejected value — redacted to `***` for [`#[Sensitive]`](/guides/sensitive-data/) properties), `{expected}` (constraint parameters / expected types).

## See also

- [Error messages guide](/guides/error-messages/) — `ErrorMessage` and translators
- [validate() reference](/validate/)
