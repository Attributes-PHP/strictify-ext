---
title: "validate()"
description: "Reference for the Attributes\\Strictify\\validate() function: signature, parameters, return value, exceptions, and a map of the validation rules."
---

# validate()

Validates a raw data array against the type hints and attributes of a `BaseModel` instance, and returns the hydrated model.

```php
<?php
namespace Attributes\\Strictify;

function validate(array $rawData, BaseModel $model, ?int $maxDepth = null): BaseModel
```

```php
<?php
use function Attributes\\Strictify\\validate;

$user = validate($rawData, new User());
```

New to Strictify? Start with the [Quickstart](/quickstart/), then the [guides](/guides/validating-payloads/).

## Parameters

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `$rawData` | `array` | The input data, keyed by property name (or by [alias](/guides/aliases/)). Missing keys fail validation unless the property is nullable or declares a default value. |
| `$model` | `BaseModel` | The model instance to hydrate. Its public property type hints and attribute declarations define the validation schema. |
| `$maxDepth` | `?int` | Nesting limit for this run: how many models deep the payload may nest before a `MaxDepthException` is thrown. `null` (the default) uses 512; the value must be greater than 0. The limit only applies to this call — a nested `validate()` from a hook starts from the default again. |

## Return value

Returns the passed `$model` instance with every property populated from `$rawData`. Values are written with the type declared on the property:

- Scalars are coerced to the declared type in [loose mode](/guides/validating-payloads/#strict-and-loose-mode).
- `DateTimeInterface`-typed properties hydrate from date strings such as `1994-01-01T09:00:00+00:00`.
- Properties typed with a `BaseModel` class hydrate recursively from nested arrays.
- Union-typed properties keep the raw value's type when it already matches one of the arms; otherwise the engine coerces them in its fixed order (`int` → `float` → `string` → `bool`), never in declaration order — see [Strict and loose mode](/guides/validating-payloads/#strict-and-loose-mode).

## Exceptions

| Exception | Thrown when |
| --------- | ----------- |
| `Attributes\Strictify\Exceptions\ValidationException` | Any field fails validation. Call `getErrors()` for the error list — see [Exceptions](/exceptions/). |
| `Attributes\Strictify\Exceptions\InvalidBaseModelException` | The model declaration is malformed, for example a `#[ModelConfigs]` option with an unknown value or the wrong type. |
| `Attributes\Strictify\Exceptions\MaxDepthException` | The payload nests models deeper than `$maxDepth` (512 by default). |
| `ValueError` | An attribute spec is malformed, for example an unresolvable class-name arm, an unsupported `Type` value in an array attribute, or a `Dict` key type outside integer and string. |

## Validation rules at a glance

| Rule | Behavior | Guide |
| ---- | -------- | ----- |
| Required fields | Every property without a default value is required (typed or nullable); defaulted properties are optional | [Validating payloads](/guides/validating-payloads/#required-fields) |
| Union on union hints | `#[Union(...)]` on a union-typed property must repeat the hint's type-hints exactly, else `InvalidBaseModelException` | [Nested models and arrays](/guides/nested-models/#union-on-non-array-properties) |
| Strict vs loose mode | Strict by default; `#[ModelConfigs(strict: false)]` enables scalar coercion | [Validating payloads](/guides/validating-payloads/#strict-and-loose-mode) |
| Aliases | `#[Alias]` maps one input key; `aliasGenerator` transforms all keys; resolved names appear in error paths | [Aliases](/guides/aliases/) |
| Nested models & arrays | `BaseModel` properties hydrate recursively; `Sequence`/`Union`/`Intersection`/`Dict` shape arrays | [Nested models and arrays](/guides/nested-models/) |
| Constraints | Value-level rules via constraint attributes, running after the type check | [Constraints](/constraints/) |
| Error messages | `Must be <type>` defaults; per-property `ErrorMessage`; translatable via `ErrorTranslator::set()` | [Error messages](/guides/error-messages/) |
| Sensitive fields | `#[Sensitive]` keeps values out of `toDict()`, error messages and debug output | [Sensitive data](/guides/sensitive-data/) |
| Model configuration | `#[ModelConfigs(...)]` options: `aliasGenerator`, `strict`, `stopAtFirstError`, `extra`, `strToLower`/`strToUpper`/`stripWhitespace`. Resolved per model class — inherited from parent classes only, never from an enclosing model | [ModelConfigs](/model-configs/) |
| Hooks | `beforeValidation()` / `afterValidation()` around the validation run | [Validating payloads](/guides/validating-payloads/#hooks) |

## Serializing back to an array

`BaseModel::toDict()` returns the model's public properties as an array. See [toDict()](/toDict/) and the [serializing guide](/guides/serializing-models/).

## See also

- [call()](/call/) — invoke a callable with hydrated model parameters
- [Exceptions and error catalog](/exceptions/)
- [API reference](/api/)
