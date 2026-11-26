---
title: "Exceptions"
description: "Reference for the exceptions Strictify throws: ValidationException and its getErrors() structure, InvalidBaseModelException, MaxDepthException, ValueError cases and the BaseException root."
---

# Exceptions

## At a glance

| Exception | Extends | Raised when |
| --------- | ------- | ----------- |
| `ValidationException` | `BaseException` | a field fails validation: missing key, wrong type, failing constraint, `Sequence`/`Dict` shape errors, unknown keys under `extra: 'forbid'`. Carries `getErrors()` |
| `InvalidBaseModelException` | `BaseException` | the model declaration is malformed: any attribute argument the declaration rejects — a wrong argument type (`#[Union(1.5)]`), an unresolvable class arm, an unsupported `Type` value, a `Dict` key type outside the permitted ones, empty `Sequence`/`Dict` arguments, an invalid `Sequence(sorted:)`, a timezone or `Ip`/`DateTime` argument, a constraint argument that is unsatisfiable (inverted/`NAN` bounds, empty lists, a pattern that does not compile) — plus `Union` on an `array` property or with arms that differ from its hint, `Sequence`/`Dict`/`Intersection` on a non-array-like property, an empty `#[Alias]`, two properties resolving to the same key, and a `#[ModelConfigs]` option with an unknown value or the wrong type |
| `MaxDepthException` | `BaseException` | the payload nests models deeper than the limit (512 by default, or `$maxDepth`) |
| `BaseException` | `\Exception` | nothing directly — catch it for every extension error of the three above |
| `ValueError` | `\ValueError` | a call argument is unusable (`$maxDepth` below 1), or an `ErrorTranslator` callable returns something other than `?string` |
| `TypeError` | `\TypeError` | `call()` receives a `$dependencies` value that is neither `array` nor `ArrayAccess`, registering a non-callable translator, or an attribute object is constructed directly in userland with a wrong argument type |
| `Error` | `\Error` | `call()` cannot build the argument list: a required parameter has no dependency (`No value provided for parameter $x of f().`) |

`catch (BaseException $e)` covers every error caused by a malformed model — including the argument errors the attribute constructors raise, which are rewritten into `InvalidBaseModelException` when the attribute is read from a declaration. `ValueError`, `TypeError` and `Error` cover bad call arguments and plain `new Email(...)` construction in userland.

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
- a `#[ModelConfigs]` option that cannot be used — an unknown `aliasGenerator` or `extra` value (the message lists the valid ones), a non-boolean for a `bool` option (`strict: 1`, `strToLower: 'yes'`), a non-string for a `?string` option, or a string longer than the accepted length. Attribute arguments bypass the engine's parameter checks, so these are model-definition errors rather than argument errors,
- an attribute argument with the wrong type — `#[Min('fifteen')]`, `#[Union(1.5)]`, `#[Alias(123)]`,
- an array type-hint spec that cannot be built — an unresolvable class-name arm in `Union`/`Intersection`/`Dict`, an unsupported `Type` value, a `Dict` key type outside the permitted ones, empty `Sequence`/`Dict` arguments, a `Sequence(sorted:)` value outside `'asc'`/`'desc'`/`null`,
- a constraint argument that makes the constraint unsatisfiable or a no-op — an inverted or `NAN` bound, a negative `Size` bound, an empty `In`/`StartsWith`/`EndsWith` list, an empty prefix or suffix, an empty `Email`/`Url` restriction, an invalid `DateTime` timezone, an `Ip` `version` outside `'v4'`/`'v6'`, and a `Regex` pattern that does not compile (see [malformed constraint arguments](/constraints/#malformed-constraint-arguments)).

The same constraint constructors called directly from userland keep the engine's `TypeError`/`ValueError`: `new Size('x')` and `new Union('NoSuchClass')` are not model declarations.

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

Attribute arguments declared on a model now raise `InvalidBaseModelException`; a `ValueError` is left for the calls that are not model declarations:

- a `$maxDepth` argument below 1,
- an `ErrorTranslator` callable that returns something other than `?string`,
- constructing a constraint object directly in userland with an argument the constructor rejects as a value — `new Union('NoSuchClass')`, `new DateTime(timezone: 'Nope/Nope')`.

The same arguments in a declaration (`#[Union('NoSuchClass')]`, `#[DateTime(timezone: 'Nope/Nope')]`) raise `InvalidBaseModelException` instead.

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
