---
title: "Guide: serializing models"
description: "Turn a hydrated model back into a plain array with toDict(): field names, useAlias, DateTime formatting, nesting and sensitive fields."
---

# Serializing models

`BaseModel::toDict()` returns the model's public properties as an array, so a hydrated model can be serialized back to plain data. See the [toDict() reference](/toDict/) for the full signature.

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

print_r($user->toDict()); // Array ( [name] => John [age] => 30 )
```

## Field names: property names vs aliases

By default the output is keyed by the **raw PHP property names**. Pass `useAlias: true` to key it by the resolved field names ([aliases](/guides/aliases/) and `aliasGenerator` are applied; each nested model uses only its own generator, never the enclosing model's) — the output then round-trips through `validate()`:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\ModelConfigs;
use function Attributes\Strictify\validate;

#[ModelConfigs(aliasGenerator: 'snake')]
class User extends BaseModel
{
    #[Alias('user_name')]
    public string $name;
    public string $firstName;
}

$user = validate(['user_name' => 'John', 'first_name' => 'John'], new User());

print_r($user->toDict());               // Array ( [name] => John [firstName] => John )
print_r($user->toDict(useAlias: true)); // Array ( [user_name] => John [first_name] => John )

validate($user->toDict(useAlias: true), new User()); // round-trips
```

## Value serialization

- Scalars and arrays (including `Sequence`/`Dict` shapes) are returned as-is, recursing into nested elements.
- `DateTimeInterface` values are formatted as ISO 8601 strings (`Y-m-d\TH:i:sP`), or with the format/timezone of a `#[DateTime]` constraint on the property.
- Nested `BaseModel` properties are serialized recursively.
- Properties marked with [`#[Sensitive]`](/guides/sensitive-data/) are omitted unless `includeSensitive: true` is passed (the flag propagates into nested models and arrays).
- Uninitialized typed properties are omitted from the output.

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Constraints\DateTime;
use function Attributes\Strictify\validate;

class Address extends BaseModel
{
    public string $street;
}

class User extends BaseModel
{
    public string $name;
    public Address $address;
    #[DateTime(format: 'Y-m-d')]
    public ?\DateTime $birthday = null;
}

$user = validate([
    'name' => 'John',
    'address' => ['street' => 'Main St'],
    'birthday' => '1994-01-01',
], new User());

print_r($user->toDict());
// Array ( [name] => John [address] => Array ( [street] => Main St ) [birthday] => 1994-01-01 )
```

## See also

- [Aliases and aliasGenerator](/guides/aliases/)
- [Sensitive data](/guides/sensitive-data/)
- [toDict() reference](/toDict/)
