---
title: "Quickstart"
description: "Install Strictify, define your first model and validate data in under a minute."
---

# Quickstart

## Installation

Install the extension with [PIE](https://php-pie.org):

```bash
pie install attributes-php/strictify
```

Requires PHP 8.2 or later.

## Your first model

A model is a class extending `BaseModel`. Its public property type hints **are** the schema — no configuration files, no YAML:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use function Attributes\Strictify\validate;

class User extends BaseModel
{
    public string $name;
    public int $age;
    public ?DateTime $birthday = null;
}

$user = validate([
    'name' => 'John',
    'age' => 30,
    'birthday' => '1994-01-01T09:00:00+00:00',
], new User());

echo $user->name, "\n";                    // John
echo $user->age, "\n";                     // 30
echo $user->birthday->format('Y-m-d'), "\n"; // 1994-01-01
```

`validate()` returns the same model instance with every property hydrated to its declared type: `DateTimeInterface` properties hydrate from date strings, nested models hydrate recursively.

## Invalid data

When any field fails, `validate()` throws a `ValidationException`. `getErrors()` returns every problem at once, keyed by field path:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use function Attributes\Strictify\validate;

class Signup extends BaseModel
{
    public string $email;
    public int $age;
}

try {
    validate(['email' => 123, 'age' => 'old'], new Signup());
} catch (ValidationException $e) {
    print_r($e->getErrors());
    // Array
    // (
    //     [email] => Array ( [0] => Must be string )
    //     [age] => Array ( [0] => Must be integer )
    // )
}
```

Nested structures report dot-notation paths (`address.street`, `users.0.email`).

## Serialize back to an array

Once hydrated, a model serializes back to a plain array with `toDict()`:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use function Attributes\Strictify\validate;

class User extends BaseModel
{
    public string $name;
    public int $age;
}

$user = validate(['name' => 'John', 'age' => 30], new User());

print_r($user->toDict());
// Array ( [name] => John [age] => 30 )
```

## What's next

- [Validating payloads](/guides/validating-payloads/) — required fields, strict vs loose mode, hooks.
- [Nested models and arrays](/guides/nested-models/) — `Sequence`, `Union`, `Dict`, nested models.
- [Aliases and aliasGenerator](/guides/aliases/) — accept `snake_case` payloads for `camelCase` properties.
- [Error messages](/guides/error-messages/) — custom and translated messages.
- [Reference: validate()](/validate/), [toDict()](/toDict/), [call()](/call/), [Constraints](/constraints/).
