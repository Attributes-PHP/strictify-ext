---
title: "Guide: nested models and arrays"
description: "Validate nested BaseModel properties and typed arrays with Sequence, Union, Intersection and Dict."
---

# Nested models and arrays

## Nested models

A property typed with another `BaseModel` class hydrates recursively from a nested array. Errors report dot-notation paths, including array element keys (`users.0.email`):

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use function Attributes\Strictify\validate;

class Address extends BaseModel
{
    public string $street;
    public string $city;
}

class User extends BaseModel
{
    public string $name;
    public Address $address;
}

try {
    validate([
        'name' => 'John',
        'address' => ['street' => 'Main St', 'city' => 123],
    ], new User());
} catch (ValidationException $e) {
    print_r($e->getErrors());
    // Array ( [address.city] => Array ( [0] => Must be string ) )
}
```

Each nested model resolves its own [`#[ModelConfigs]`](/model-configs/) (for example its own `aliasGenerator`) — configs are inherited only from parent *classes*, never from the enclosing model. See [Aliases](/guides/aliases/).

## Typed arrays: Sequence, Union, Intersection, Dict

Typed `array` properties declare their element shape with attributes from `Attributes\Strictify\Fields`:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\Type;
use function Attributes\Strictify\validate;

class Blog extends BaseModel
{
    #[Sequence(Type::string)]
    public array $tags;

    #[Sequence(of: new Union(Type::int, Type::string, Type::sequence))]
    public array $mixed;

    #[Dict(key: Type::int, of: stdClass::class)]
    public array $lookup;
}

$blog = validate([
    'tags' => ['php', 'dev'],
    'mixed' => [1, 'two', [3]],
    'lookup' => [1 => new stdClass()],
], new Blog());
```

- `Sequence` validates every element against one element type.
- `Union` accepts an element matching any declared arm — but only nested inside `Sequence(of: new Union(...))` or `Dict(of: new Union(...))`. Declared directly on an `array` property it throws a `ValueError`; on a non-array property it checks the whole value instead (below).
- `Intersection` requires an element to match every declared class or interface. Its arms must name classes or interfaces only — enums are rejected, mirroring PHP intersection type hints.
- `Dict` validates keys against `$key` (integer and string types only) and values against `$of`.

`Sequence` and `Dict` may only be declared on array-like properties (an `array` hint, a union containing `array`, `mixed` or untyped); anywhere else they throw a `ValueError`. On `mixed` or untyped properties a non-array value fails with `Must be array`.

## Union on non-array properties

Declared directly on a property whose value is not an array — `int|float`, `string`, `mixed`, untyped — `#[Union(...)]` checks the whole value against its arms, exactly (never coerced; in loose mode it sees the coerced value):

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\Type;
use function Attributes\Strictify\validate;

class Measurement extends BaseModel
{
    #[Union(Type::int, Type::float)] // 1 and 1.5 pass
    public int|float $value;

    #[Union(Type::int)] // narrows int|float to integers only: 1.5 fails
    public int|float $count;
}

$m = validate(['value' => 1.5, 'count' => 3], new Measurement());
```

An array value under a directly declared `Union` throws a `ValueError` — wrap the array in `Sequence(of: new Union(...))` instead.

Element types come from the `Attributes\Strictify\Type` enum (`Type::int`, `Type::string`, `Type::bool`, `Type::float`, `Type::sequence`, `Type::object`, `Type::dict`) or the matching namespace constants (`int`, `string`, `bool`, `float`, `sequence`, `object`), and may combine with the bitwise OR operator (`bool | int`). Class, interface and enum names are accepted as strings (`DateTime::class`) and autoload on use.

The same attributes compose with `new` expressions to describe nested arrays:

```php
<?php
#[Sequence(of: new Union(string, int, float))]
public array $values;
```

Arguments of a composed `new` expression are validated at construction: an unsupported argument type throws a `TypeError` and a string that names no loadable class, interface or enum throws a `ValueError`.

## See also

- [Validating payloads](/guides/validating-payloads/)
- [Constraints](/constraints/) — value-level rules on elements' parent property
- [validate() reference](/validate/)
