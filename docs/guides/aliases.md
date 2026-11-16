---
title: "Guide: aliases and aliasGenerator"
description: "Accept differently-named input keys with #[Alias] and the ModelConfigs aliasGenerator, and serialize back with toDict(useAlias: true)."
---

# Aliases and aliasGenerator

Payloads often use a different naming convention than the PHP properties (a JSON API sending `first_name` for a `$firstName` property). Strictify resolves input keys through aliases:

1. An explicit `#[Alias('...')]` on the property always wins.
2. Otherwise, the model's `aliasGenerator` transforms the property name (`'pascal'`, `'camel'`, `'snake'`, `'kebab'`).
3. Otherwise the property name is used as-is.

## The Alias attribute

Map one input key to one property:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Alias;
use function Attributes\Strictify\validate;

class User extends BaseModel
{
    #[Alias('user_name')]
    public string $name;
}

$user = validate(['user_name' => 'John'], new User());
echo $user->name, "\n"; // John
```

## The aliasGenerator option

Transform every property name with one `#[ModelConfigs]` declaration:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ModelConfigs;
use function Attributes\Strictify\validate;

#[ModelConfigs(aliasGenerator: 'snake')]
class User extends BaseModel
{
    public string $firstName;
    public int $user2Id;
}

$user = validate(['first_name' => 'John', 'user2_id' => 123], new User());
echo $user->firstName, ' ', $user->user2Id, "\n"; // John 123
```

Resolved names are also used in **error paths**, so clients see the key they sent:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\ModelConfigs;
use function Attributes\Strictify\validate;

#[ModelConfigs(aliasGenerator: 'snake')]
class User extends BaseModel
{
    public string $firstName;
    #[Alias('last')]
    public string $lastName;
}

try {
    validate(['first_name' => 'John'], new User());
} catch (ValidationException $e) {
    print_r($e->getErrors()); // Array ( [last] => Array ( [0] => Field is required ) )
}
```

## Serializing back: toDict(useAlias: true)

`toDict()` is keyed by the raw property names by default. Pass `useAlias: true` to get the resolved names, so the output round-trips through `validate()`:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ModelConfigs;
use function Attributes\Strictify\validate;

#[ModelConfigs(aliasGenerator: 'snake')]
class User extends BaseModel
{
    public string $firstName;
    public string $lastName;
}

$user = validate(['first_name' => 'John', 'last_name' => 'Doe'], new User());

print_r($user->toDict());               // Array ( [firstName] => John [lastName] => Doe )
print_r($user->toDict(useAlias: true)); // Array ( [first_name] => John [last_name] => Doe )

validate($user->toDict(useAlias: true), new User()); // works
```

## Nested models

A nested model with its own `#[ModelConfigs(aliasGenerator: ...)]` uses its own generator, both when validating and when serializing with `useAlias: true`. See [toDict()](/toDict/) and the nested-generator tests' behavior in [Nested models](/guides/nested-models/).

## See also

- [Serializing models](/guides/serializing-models/)
- [ModelConfigs reference](/model-configs/)
- [validate() reference](/validate/)
