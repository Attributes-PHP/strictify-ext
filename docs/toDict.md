---
title: "toDict()"
description: 'Reference for the Attributes\\Strictify\\BaseModel::toDict() method: array serialization, field names, DateTime formatting, nesting and property hooks.'

---

# toDict()

`BaseModel::toDict()` returns the model's public properties as an array, so a hydrated model can be serialized back to plain data.

```php
<?php
namespace Attributes\Strictify;

abstract class BaseModel
{
    public function toDict(): array {}
}
```

```php
<?php
use function Attributes\Strictify\validate;

$user = validate($rawData, new User());
$array = $user->toDict();
```

## Field names

The output is keyed by the same resolved field names [validate()](/validate/) expects: [aliases](/validate/#aliases) and the `aliasGenerator` from `#[ModelConfigs]` are applied, and nested models with their own `#[ModelConfigs(aliasGenerator: ...)]` use their own generator. The output therefore round-trips through `validate()`:

```php
<?php
class User extends BaseModel
{
    public string $name;
    public ?DateTime $birthday;
}

$user = validate(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00'], new User());

$user->toDict();
// ['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00']

validate($user->toDict(), new User()); // works
```

## Value serialization

- Scalars and arrays (including `Sequence`/`Dict` shapes) are returned as-is, recursing into nested elements.
- `DateTimeInterface` values are formatted as ISO 8601 strings (`Y-m-d\TH:i:sP`).
- Nested `BaseModel` properties are serialized recursively.
- Uninitialized typed properties are omitted from the output.

```php
<?php
class Address extends BaseModel
{
    public string $street;
}

class User extends BaseModel
{
    public string $name;
    public Address $address;
}

$user = validate(['name' => 'John', 'address' => ['street' => 'Main St']], new User());

$user->toDict();
// ['name' => 'John', 'address' => ['street' => 'Main St']]
```

### DateTime constraints

When a property has a `#[DateTime(format: ..., timezone: ...)]` constraint, `toDict()` uses that format and timezone instead of the ISO 8601 default:

```php
<?php
class Event extends BaseModel
{
    #[DateTime(format: 'Y-m-d', timezone: 'Europe/Madrid')]
    public ?DateTime $startsAt;
}

$event = validate(['startsAt' => '1994-01-01'], new Event());
$event->toDict();
// ['startsAt' => '1994-01-01'] (Madrid is UTC+1 in January, formatted as Y-m-d)
```

