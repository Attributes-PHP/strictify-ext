---
title: "ModelConfigs"
description: "Reference for the #[ModelConfigs] attribute: every option, its default, inheritance rules and the resolved-config getters."
---

# ModelConfigs

Declare `#[ModelConfigs(...)]` on the model class to change how `validate()` resolves and checks input. Every option is optional, and a child class inherits its parent's `ModelConfigs` unless it declares its own:

```php
<?php
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(strict: false)]
class ParentModel extends BaseModel
{
    public int $age;
}

class ChildModel extends ParentModel {} // inherits strict: false
```

An invalid option value throws a `ValueError` listing the valid options (e.g. `aliasGenerator` accepts `pascal, camel, snake, kebab`).

## Options

| Option | Type | Default | Description |
| ------ | ---- | ------- | ----------- |
| `aliasGenerator` | `?string` | `null` | `'pascal'` \| `'camel'` \| `'snake'` \| `'kebab'` — derives the expected input keys from property names, see [Aliases](/guides/aliases/) |
| `strict` | `bool` | `true` | `false` enables loose-mode coercion, see [Strict and loose mode](/guides/validating-payloads/#strict-and-loose-mode) |
| `stopAtFirstError` | `bool` | `false` | `true` aborts the validation run at the first error |
| `extra` | `?string` | `null` | `null` resolves to `'allow'` when the class is marked `#[\AllowDynamicProperties]`, `'ignore'` otherwise; `'ignore'` \| `'forbid'` \| `'allow'` — what to do with unknown input keys |
| `strToLower` | `bool` | `false` | lowercase string values before validation |
| `strToUpper` | `bool` | `false` | uppercase string values before validation (wins over `strToLower`) |
| `stripWhitespace` | `bool` | `false` | trim string values before validation |

`defaultErrorMessages` is not an attribute argument: it is a static callable set with `ModelConfigs::setDefaultErrorMessages()` and read back with `ModelConfigs::getDefaultErrorMessages()` (null until set) — see [Error messages](/guides/error-messages/).

## aliasGenerator

Transforms property names into the keys `validate()` expects, so payloads can use a different naming convention than the PHP properties. An explicit `#[Alias]` on a property always wins over the generator. Resolved names are also used in error paths and by `toDict(useAlias: true)`:

```php
<?php
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(aliasGenerator: 'snake')]
class User extends BaseModel
{
    public string $firstName;
    public int $user2Id;
}

validate(['first_name' => 'John', 'user2_id' => 123], new User()); // ok
validate(['firstName' => 'John'], new User());
// ValidationException: ['first_name' => ['Field is required']]
```

## strict

`strict: false` enables loose mode: scalar values are coerced to the declared type instead of rejected. See [Strict and loose mode](/guides/validating-payloads/#strict-and-loose-mode) for the coercion rules.

## stopAtFirstError

By default every failing field is collected into `ValidationException::getErrors()`. With `stopAtFirstError: true` the run aborts at the first error:

```php
<?php
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(stopAtFirstError: true)]
class User extends BaseModel
{
    public string $name;
    public string $email;
}

validate([], new User());
// ValidationException: ['name' => ['Field is required']] — email is not reported
```

## extra

Controls what happens with input keys that resolve to no property of the model:

- `null` (default): resolved per model class — `'allow'` when the class is marked `#[\AllowDynamicProperties]`, `'ignore'` otherwise.
- `'ignore'`: unknown keys are silently dropped.
- `'allow'`: unknown keys are stored as dynamic properties on the model instance. Integer keys have no property name and keys naming a declared property (e.g. the raw property name when `aliasGenerator` renames the expected key) are never stored.
- `'forbid'`: every unknown key produces an error keyed by the raw key (dot path for nested models), with the default message `Extra inputs are not permitted`, translatable under the `extra` key. Keys produced by `aliasGenerator`/`Alias` count as known, integer keys are reported under their numeric name, and `stopAtFirstError` reports only the first one.

A nested model that shares the parent's `ModelConfigs` (it declares none of its own) inherits a non-null `extra` as-is; a `null` extra is instead resolved per nested model class, against each nested class's own `#[\AllowDynamicProperties]`.

```php
<?php
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(extra: 'forbid')]
class User extends BaseModel
{
    public string $name;
}

validate(['name' => 'John', 'junk' => 1], new User());
// ValidationException: ['junk' => ['Extra inputs are not permitted']]
```

## strToLower, strToUpper, stripWhitespace

Normalize string input values before the type check, so constraints and the written property see the normalized value. Applied only to values of properties whose hint accepts `string` (including `mixed` and string arms of unions) — non-string values, and strings bound for non-string hints such as `DateTime` coercion, are left untouched. The order is strip → lower → upper, so `strToUpper` wins when both case flags are set. Works in strict and loose mode, and propagates to nested models:

```php
<?php
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(stripWhitespace: true, strToLower: true)]
class User extends BaseModel
{
    public string $name;

    public int $age;
}

validate(['name' => '  JOHN DOE  ', 'age' => 30], new User())->name; // "john doe"
```

## Resolved-config getters

All options are also exposed through getters on the resolved `ModelConfigs` object — `getExtra()`, `isStrToLower()`, `isStrToUpper()`, `isStripWhitespace()` (plus `getAliasGenerator()`, `isStrict()`, `isStopAtFirstError()`) — which `validate()` passes to the model hooks (`beforeValidation()`/`afterValidation()`, see [Hooks](/guides/validating-payloads/#hooks)) and to `ConfigAwareConstraint::setModelConfigs()`.

## See also

- [Aliases and aliasGenerator guide](/guides/aliases/)
- [Error messages guide](/guides/error-messages/)
- [validate() reference](/validate/)
