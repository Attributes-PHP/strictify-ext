---
title: "Guide: validating payloads"
description: "How validate() checks raw data: required fields, strict vs loose mode, return value semantics and validation hooks."
---

# Validating payloads

`validate($rawData, $model)` checks a raw array against the model's public property type hints and attributes, and returns the hydrated model. See the [validate() reference](/validate/) for the signature.

## Required fields

Every typed, non-nullable property is required. A missing key produces a `Field is required` error. Nullable properties accept a missing key or an explicit `null`, and properties with a declared default value keep it when the key is missing:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use function Attributes\Strictify\validate;

class Profile extends BaseModel
{
    public string $name;          // required
    public ?string $nickname = null; // optional (nullable + default)
}

try {
    validate([], new Profile());
} catch (ValidationException $e) {
    print_r($e->getErrors()); // Array ( [name] => Array ( [0] => Field is required ) )
}
```

## Strict and loose mode

Validation is **strict by default**: a value must already have the declared type. Declare loose mode on the model to enable scalar coercion:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ModelConfigs;
use function Attributes\Strictify\validate;

#[ModelConfigs(strict: false)]
class User extends BaseModel
{
    public int $age;
    public string $name;
}

$user = validate(['age' => '30', 'name' => 123], new User());
var_dump($user->age);  // int(30)
var_dump($user->name); // string(3) "123"
```

In loose mode:

- Exact type matches always win over coercion, regardless of declaration order.
- When coercion is needed on a union property, arms are tried in declaration order: `int|bool` coerces `'1'` to `int(1)`, while `bool|int` coerces it to `bool(true)`.

`DateTimeInterface` properties coerce from date strings in both modes.

!!! tip "When to use loose mode"
    Loose mode is convenient for form and query-string input, where everything arrives as a string. Keep strict mode for JSON API payloads, where a number in a string field usually signals a client bug.

## Return value semantics

`validate()` returns the passed `$model` instance with every property populated from `$rawData`. Values are written with the type declared on the property:

- Scalars are coerced to the declared type in loose mode.
- `DateTimeInterface`-typed properties hydrate from date strings such as `1994-01-01T09:00:00+00:00`.
- Properties typed with a `BaseModel` class hydrate recursively from nested arrays — see [Nested models and arrays](/guides/nested-models/).
- Union-typed properties keep the raw value's type when it already matches one of the arms; otherwise the first arm that can coerce it wins.

## Hooks

Models may override two protected hooks, called by `validate()` around the validation run:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ModelConfigs;
use function Attributes\Strictify\validate;

class Signup extends BaseModel
{
    public string $email;

    protected function beforeValidation(array $rawData, ModelConfigs $configs): array
    {
        // normalize input before validation, e.g. lowercase the email
        if (isset($rawData['email']) && is_string($rawData['email'])) {
            $rawData['email'] = strtolower($rawData['email']);
        }
        return $rawData;
    }

    protected function afterValidation(array $rawData, ModelConfigs $configs): void
    {
        // runs after a successful validation, e.g. audit logging
    }
}

$user = validate(['email' => 'JOHN@EXAMPLE.COM'], new Signup());
echo $user->email, "\n"; // john@example.com
```

`beforeValidation()` may return a modified copy of `$rawData`; `afterValidation()` runs only when validation succeeded. The resolved `ModelConfigs` object is passed to both — see [ModelConfigs](/model-configs/).

## See also

- [Nested models and arrays](/guides/nested-models/)
- [Aliases and aliasGenerator](/guides/aliases/)
- [Error messages](/guides/error-messages/)
- [Exceptions and error catalog](/exceptions/)
- [validate() reference](/validate/)
