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

#[ModelConfigs(strToLower: true)] // its own attribute wins: strict is back to the default true
class OtherChildModel extends ParentModel {}
```

Inheritance is all-or-nothing: a child without its own `#[ModelConfigs]` uses the closest ancestor's declaration as a whole, and a child that declares one uses **only** that declaration — the options it omits fall back to the defaults in the table below, not to the parent's values.

!!! note "Inheritance is class-based only"
    `ModelConfigs` is inherited **only through the class hierarchy** (a child class from its parent class). A nested model — a `BaseModel` used as a property of another model — **never** inherits the enclosing model's configs: each model class resolves its own `#[ModelConfigs]` (or its parent class's), so `aliasGenerator`, `strict`, `extra`, the string normalizations and `stopAtFirstError` apply per model class.

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

Error message translation is not a `ModelConfigs` option: it is a static callable on the dedicated `ErrorTranslator` class, set with `ErrorTranslator::set()` and read back with `ErrorTranslator::get()` (null until set) — see [Error messages](/guides/error-messages/).

Option values are read from the attribute arguments and checked against the types above: a `bool` option only accepts `true`/`false`, a `?string` option only a string (at most 127 characters) or `null`. A wrong type throws a `ValueError` instead of being cast, so `#[ModelConfigs(strict: 1)]` or `#[ModelConfigs(strToLower: 'yes')]` never silently flips a flag:

```php
<?php
#[ModelConfigs(strict: 1)]
class User extends BaseModel {}

validate([], new User());
// ValueError: Attributes\Strictify\ModelConfigs::__construct():
//   Argument #6 ($strict) must be of type bool, int given
```

An unknown value for an enumerated option (`aliasGenerator`, `extra`) throws a `ValueError` listing the valid options (e.g. `aliasGenerator` accepts `pascal, camel, snake, kebab`).

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

Each model validates with its own class's configs: a `null` extra is resolved per model class at validation time, against that class's own `#[\AllowDynamicProperties]`, and a non-null `extra` declared on an enclosing model never applies to its nested models. A subclass without its own `#[ModelConfigs]` inherits the ancestor's `extra` along with the rest of the declaration; when nothing in the chain declares it, it stays `null` and each class resolves it against its own `#[\AllowDynamicProperties]` flag.

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

Normalize string input values before the type check, so constraints and the written property see the normalized value. Applied only to values of properties whose hint accepts `string` (including `mixed` and string arms of unions) — non-string values, and strings bound for non-string hints such as `DateTime` coercion, are left untouched. The order is strip → lower → upper, so `strToUpper` wins when both case flags are set. Works in strict and loose mode. Normalization applies to the model's own properties only — nested models resolve their own configs:

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

Case conversion is byte-wise ASCII: `'ÄNDRE'` lowercases to `'Ändre'`, not `'ändre'`, and no locale rule applies. Array elements are never normalized — only the string value bound to a property is, so a `string` property holding a serialized list keeps its inner casing. The normalized value is what the constraints and the type check see.

## Resolved-config getters

All options are also exposed through getters on the resolved `ModelConfigs` object — `getExtra()`, `isStrToLower()`, `isStrToUpper()`, `isStripWhitespace()` (plus `getAliasGenerator()`, `isStrict()`, `isStopAtFirstError()`) — which `validate()` passes to the model hooks (`beforeValidation()`/`afterValidation()`, see [Hooks](/guides/validating-payloads/#hooks)) and to `ConfigAwareConstraint::setModelConfigs()`.

## See also

- [Aliases and aliasGenerator guide](/guides/aliases/)
- [Error messages guide](/guides/error-messages/)
- [validate() reference](/validate/)
