---
title: "Exceptions"
description: "Reference for the exceptions Strictify throws: ValidationException and its getErrors() structure, InvalidBaseModelException and ValueError cases."
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

## InvalidBaseModelException

`Attributes\Strictify\Exceptions\InvalidBaseModelException` extends `BaseException` and is thrown when a model declares an attribute on an invalid property type — a malformed model definition, not a data error:

- `Union` declared directly on an `array` property, or receiving an array value on `mixed`/untyped (use `Sequence(of: new Union(...))` / `Dict(of: ...)` instead),
- `Sequence`/`Dict` declared on a property that is not array-like (not `array`, a union containing `array`, `mixed` or untyped).

## ValueError

A `ValueError` is thrown when an attribute spec is malformed, for example:

- an unresolvable class-name arm in `Union`/`Intersection`/`Dict`,
- an unsupported `Type` value in an array attribute,
- a `Dict` key type outside integer and string,
- an invalid `ModelConfigs` option value (the message lists the valid options),
- an invalid timezone name in a `DateTime` constraint (at construction).

## See also

- [Error messages guide](/guides/error-messages/) — `ErrorMessage`, translators and the default error catalog
- [validate() reference](/validate/)
