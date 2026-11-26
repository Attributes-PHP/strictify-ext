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
use function Attributes\Strictify\validate;

use const Attributes\Strictify\T_INT;
use const Attributes\Strictify\T_SEQUENCE;
use const Attributes\Strictify\T_STRING;

class Blog extends BaseModel
{
    #[Sequence(T_STRING)]
    public array $tags;

    #[Sequence(of: new Union(T_INT, T_STRING, T_SEQUENCE))]
    public array $mixed;

    #[Dict(key: T_INT, of: stdClass::class)]
    public array $lookup;
}

$blog = validate([
    'tags' => ['php', 'dev'],
    'mixed' => [1, 'two', [3]],
    'lookup' => [1 => new stdClass()],
], new Blog());
```

- `Sequence` validates every element against one element type. Keys must always be integers — a string key reports `Must be integer` under that key's path (e.g. `tags.x`). By default the array must additionally be a **list** with keys `0, 1, 2, ...` in order; a gap or out-of-order key reports a `sequence_keys` error. Pass `sequential: false` to allow any integer keys. Pass `sorted: 'asc'` (or `'ASC'` / `'desc'` / `'DESC'`) to sort the validated values and renumber the keys `0, 1, 2, ...`; the default `null` keeps the input order, and any other value throws an `InvalidBaseModelException` instead of being ignored. Pass `uniqueValues: true` to reject repeated elements (`unique_values` error on the property).
- `Union` accepts an element matching any declared arm — but only nested inside `Sequence(of: new Union(...))` or `Dict(of: new Union(...))`. Declared directly on an `array` property it throws an `InvalidBaseModelException`; on a non-array property it checks the whole value instead (below).
- `Intersection` requires an element to match every declared class or interface. Its arms must name classes or interfaces only — enums are rejected, mirroring PHP intersection type hints.
- `Dict` validates keys against `$key` and values against `$of`. The key spec accepts every PHP-castable key type — integer, string, float, boolean and null ([language.types.array](https://www.php.net/manual/en/language.types.array.php)). The engine casts float and bool keys to int and null to the empty string, so those arms match the post-cast key: a `T_FLOAT` or `T_BOOL` arm accepts integer keys, a `T_NULL` arm accepts the `""` key (`[null => ...]`). E.g. `#[Dict(key: T_INT | T_STRING | T_NULL, of: ...)]`. Pass `uniqueValues: true` to reject repeated values (`unique_values` error on the property).

`Sequence`, `Dict` and `Intersection` may only be declared on array-like properties — an `array` hint (alone or inside a union), `mixed`, or untyped. On anything else they throw an `InvalidBaseModelException` naming the property, whether the hint is scalar (`string`, `int`), the `object` class type, or an intersection type without `array`. On `mixed` or untyped properties a non-array value fails with `Must be array`.

## Union on non-array properties

Declared directly on a property whose value is not an array — `int|float`, `string`, `mixed`, untyped — `#[Union(...)]` checks the whole value against its arms, exactly (never coerced; in loose mode it sees the coerced value):

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;
use function Attributes\Strictify\validate;

use const Attributes\Strictify\T_FLOAT;
use const Attributes\Strictify\T_INT;

class Measurement extends BaseModel
{
    #[Union(T_INT, T_FLOAT)] // 1 and 1.5 pass
    public int|float $value;
}

$m = validate(['value' => 1.5], new Measurement());
```

On a **union-typed** property the arms must repeat the native hint's type-hints exactly — order-insensitive, class names matched case-insensitively. A subset or an extra type is a malformed model and throws an `InvalidBaseModelException` at validation. A **nullable** hint (`?string`) counts `null` as one of its type-hints, so the arms must include `T_NULL`:

```php
<?php
class Bad extends BaseModel
{
    #[Union(T_INT)] // missing float: InvalidBaseModelException
    public int|float $value;
}

class AlsoBad extends BaseModel
{
    #[Union(T_STRING)] // missing T_NULL on a ?string hint: InvalidBaseModelException
    public ?string $name;
}
```

An array value under a directly declared `Union` throws an `InvalidBaseModelException` — wrap the array in `Sequence(of: new Union(...))` instead.

Element types are named by the namespace constants `T_INT`, `T_STRING`, `T_BOOL`, `T_FLOAT`, `T_NULL`, `T_SEQUENCE`, `T_OBJECT`, `T_DICT` — the `T_` prefix keeps them clear of PHP's reserved type-hint keywords — which hold the same values as the `Attributes\Strictify\Type` enum cases (`Type::int`, `Type::string`, ..., `Type::null`). Constants may combine with the bitwise OR operator (`T_BOOL | T_INT`). Class, interface and enum names are accepted as strings (`DateTime::class`) and autoload on use.

When the declaration order of the arms is irrelevant, the `Union` wrapper can be dropped and the combined type mask passed directly to `of:` — or to `key:`, which accepts any PHP-castable key-type mask (integer, string, float, boolean, null):

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Fields\Sequence;
use function Attributes\Strictify\validate;

use const Attributes\Strictify\T_INT;
use const Attributes\Strictify\T_SEQUENCE;
use const Attributes\Strictify\T_STRING;

class Shapes extends BaseModel
{
    #[Sequence(of: T_INT | T_STRING | T_SEQUENCE)]
    public array $mixed;

    #[Dict(key: T_INT | T_STRING, of: T_INT | T_STRING)]
    public array $lookup;
}

validate([
    'mixed' => [1, 'two', [3]],
    'lookup' => [1 => 'a', 'k' => 2],
], new Shapes());
```

(Masks are built from the `T_*` constants — enum cases don't support `|`, so `Type::int | Type::string` is not valid PHP; write `T_INT | T_STRING`.)

The bitmask form loses the declaration order that a `new Union(...)` argument list keeps: values that already match an arm are still accepted as-is, but in loose mode coercions then run in a fixed order (bool, integer, float, string) instead of declaration order — see [Strict and loose mode](/guides/validating-payloads/#strict-and-loose-mode).

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Sequence;
use function Attributes\Strictify\validate;

use const Attributes\Strictify\T_STRING;

class Tags extends BaseModel
{
    #[Sequence(T_STRING)]                    // list only: keys 0, 1, 2, ...
    public array $list;

    #[Sequence(T_STRING, sequential: false)] // any integer keys allowed
    public array $map;
}

$model = validate(['list' => ['a', 'b'], 'map' => [7 => 'a']], new Tags());
echo count($model->list), ' ', count($model->map), "\n"; // 2 1
```

The same attributes compose with `new` expressions to describe nested arrays:

```php
<?php
#[Sequence(of: new Union(T_STRING, T_INT, T_FLOAT))]
public array $values;
```

Arguments of a composed `new` expression are validated at construction: an unsupported argument type and a string that names no loadable class, interface or enum both throw an `InvalidBaseModelException`.

## Recursion and the depth limit

A self-referencing model (`public ?Node $child`) validates as deep as the payload goes. To keep a hostile payload from exhausting the C stack, `validate()` stops 512 nested model levels deep and throws a catchable `Attributes\Strictify\Exceptions\MaxDepthException`:

```php
<?php
// MaxDepthException: Maximum validation nesting depth of 512 exceeded:
//   the payload nests models or arrays too deeply.
```

Only nesting adds depth: siblings do not. A list of ten thousand models is one level deep and validates fine, while a chain of 512 nested models throws. The counter is per `validate()` call and unwinds on the way out, so a caught failure — including one that ends in a `ValidationException` — does not affect the next call.

The limit is the `$maxDepth` argument of [`validate()`](/validate/) and [`call()`](/call/), 512 by default. Raise it when a legitimately deep payload is expected, or lower it to reject deep nesting earlier than the stack would; the value must be greater than 0, and the exception message reports the limit that fired (`... depth of 10 exceeded ...`).

## See also

- [Validating payloads](/guides/validating-payloads/)
- [Constraints](/constraints/) — value-level rules on elements' parent property
- [validate() reference](/validate/)
