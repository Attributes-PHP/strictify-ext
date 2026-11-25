---
title: "Exceptions"
description: "Reference for the exceptions Strictify throws: ValidationException and its getErrors() structure, InvalidBaseModelException, ValueError cases and the BaseException nesting-depth guard."
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

`Attributes\Strictify\Exceptions\InvalidBaseModelException` extends `BaseException` and is thrown when a model declares an attribute on an invalid property type, or when two properties claim the same input key — a malformed model definition, not a data error:

- `Union` declared directly on an `array` property, or receiving an array value on `mixed`/untyped (use `Sequence(of: new Union(...))` / `Dict(of: ...)` instead),
- `Union` declared directly on a union-typed property whose arms do not repeat the hint's type-hints exactly,
- `Sequence`/`Dict`/`Intersection` declared on a property that is not array-like (not `array`, a union containing `array`, `mixed` or untyped),
- an empty `#[Alias('')]` on a property,
- two properties resolving to the same input key — `#[Alias('id')]` on `$a` and `$b`, or `aliasGenerator: 'snake'` mapping `$aB` and `$a_b` to `a_b`. The message names both properties and the key.

```php
<?php
class User extends BaseModel
{
    #[Alias('')]
    public string $name;
}

validate(['name' => 'John'], new User());
// InvalidBaseModelException: The #[Alias] name of User::$name must not be empty.
```

## ValueError

A `ValueError` is thrown when an attribute spec is malformed, for example:

- an unresolvable class-name arm in `Union`/`Intersection`/`Dict`,
- an unsupported `Type` value in an array attribute,
- a `Dict` key type outside integer and string,
- a `Sequence`/`Dict`/`Intersection` attribute argument outside its allowed values — e.g. `#[Sequence(sorted: 'up')]`, which accepts only `'asc'`, `'desc'` or `null`,
- a constraint argument that makes the constraint unsatisfiable or a no-op — an inverted or `NAN` bound, a negative `Size` bound, an empty `In`/`StartsWith`/`EndsWith` list, an empty prefix or suffix, an empty `Email`/`Url` restriction, and a `Regex` pattern that does not compile (see [malformed constraint arguments](/constraints/#malformed-constraint-arguments)),
- an invalid `ModelConfigs` option value: an unknown value for `aliasGenerator`/`extra` (the message lists the valid options), or a wrong type — `strict: 1` and `strToLower: 'yes'` are rejected rather than cast,
- an invalid timezone name in a `DateTime` constraint (at construction).

## BaseException

`Attributes\Strictify\Exceptions\BaseException` is the extension's own exception root: `ValidationException` and `InvalidBaseModelException` extend it, so `catch (BaseException $e)` handles every error the extension raises on purpose.

It is also thrown directly when a payload nests models more than 512 levels deep:

```php
<?php
// BaseException: Maximum validation nesting depth of 512 exceeded:
//   the payload nests models or arrays too deeply.
```

Depth is counted per `validate()` call and unwound on the way out, including when the deep run fails with a `ValidationException`, so a caught deep failure does not affect the next call. The limit turns an otherwise unbounded recursion (a self-referencing model fed a deep payload) into a catchable error instead of a stack overflow.

## See also

- [Error messages guide](/guides/error-messages/) — `ErrorMessage`, translators and the default error catalog
- [validate() reference](/validate/)
