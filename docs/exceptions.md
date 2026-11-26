---
title: "Exceptions"
description: "Reference for the exceptions Strictify throws: ValidationException and its getErrors() structure, InvalidBaseModelException, MaxDepthException, ValueError cases and the BaseException root."
---

# Exceptions

## At a glance

| Exception | Extends | Raised when |
| --------- | ------- | ----------- |
| `ValidationException` | `BaseException` | a field fails validation: missing key, wrong type, failing constraint, `Sequence`/`Dict` shape errors, unknown keys under `extra: 'forbid'`. Carries `getErrors()` |
| `InvalidBaseModelException` | `BaseException` | the model declaration is malformed: `Union` on an `array` property or with arms that differ from its hint, `Sequence`/`Dict`/`Intersection` on a non-array-like property, an empty `#[Alias]`, two properties resolving to the same key, a `#[ModelConfigs]` option with an unknown value or the wrong type |
| `MaxDepthException` | `BaseException` | the payload nests models deeper than the limit (512 by default, or `$maxDepth`) |
| `BaseException` | `\Exception` | nothing directly — catch it for every extension error of the three above |
| `ValueError` | `\ValueError` | an attribute spec cannot be built: unresolvable class arm, unsupported `Type` value, `Dict` key type, empty `Sequence`/`Dict` arguments, an invalid `Sequence(sorted:)`, a timezone or `Ip`/`DateTime` argument, a constraint argument that is unsatisfiable (inverted/`NAN` bounds, empty lists, a pattern that does not compile), a translator returning a non-string, `$maxDepth` below 1 |
| `TypeError` | `\TypeError` | an attribute constructor argument has the wrong type (`#[Union(1.5)]`), or `call()` receives a `$dependencies` value that is neither `array` nor `ArrayAccess` |
| `Error` | `\Error` | `call()` cannot build the argument list: a required parameter has no dependency (`No value provided for parameter $x of f().`) |

`catch (BaseException $e)` covers every error the extension raises on purpose; `ValueError`, `TypeError` and `Error` come from the engine and are raised for malformed attribute specs and call arguments.

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
- two properties resolving to the same input key — `#[Alias('id')]` on `$a` and `$b`, or `aliasGenerator: 'snake'` mapping `$aB` and `$a_b` to `a_b`. The message names both properties and the key,
- a `#[ModelConfigs]` option that cannot be used — an unknown `aliasGenerator` or `extra` value (the message lists the valid ones), a non-boolean for a `bool` option (`strict: 1`, `strToLower: 'yes'`), a non-string for a `?string` option, or a string longer than the accepted length. Attribute arguments bypass the engine's parameter checks, so these are model-definition errors rather than argument errors.

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
- an invalid timezone name in a `DateTime` constraint, or an `Ip` `version` outside `'v4'`/`'v6'` (at construction),
- a `$maxDepth` argument below 1,
- an `ErrorTranslator` callable that returns something other than `?string`.

## MaxDepthException

`Attributes\Strictify\Exceptions\MaxDepthException` extends `BaseException` and is thrown when the payload nests models deeper than the limit:

```php
<?php
validate($deepPayload, new Node(), maxDepth: 10);
// MaxDepthException: Maximum validation nesting depth of 10 exceeded:
//   the payload nests models or arrays too deeply.
```

The default limit is 512; [`validate()`](/validate/) and [`call()`](/call/) take a `$maxDepth` argument to override it per run, and the message reports the limit that fired. Depth is counted per run and unwound on the way out, including when the deep run fails with a `ValidationException`, so a caught failure does not affect the next call. Only nesting counts — siblings do not — so a long list is fine while a deep chain is not. The limit turns an otherwise unbounded recursion (a self-referencing model fed a deep payload) into a catchable error instead of a stack overflow.

## BaseException

`Attributes\Strictify\Exceptions\BaseException` is the extension's own exception root: `ValidationException`, `InvalidBaseModelException` and `MaxDepthException` extend it, so `catch (BaseException $e)` handles every error the extension raises on purpose. It is not thrown on its own.

## See also

- [Error messages guide](/guides/error-messages/) — `ErrorMessage`, translators and the default error catalog
- [validate() reference](/validate/)
