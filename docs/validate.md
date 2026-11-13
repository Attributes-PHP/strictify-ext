---
title: "validate()"
description: 'Reference for the Attributes\Strictify\validate() function: parameters, return value, validation rules and exceptions.'

---

# validate()

Validates a raw data array against the type hints and attributes of a `BaseModel` instance, and returns the hydrated model.

```php
<?php
namespace Attributes\Strictify;

function validate(array $rawData, BaseModel $model): BaseModel
```

```php
<?php
use function Attributes\Strictify\validate;

$user = validate($rawData, new User());
```

## Parameters

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `$rawData` | `array` | The input data, keyed by property name (or by [alias](#aliases)). Missing keys fail validation unless the property is nullable or declares a default value. |
| `$model` | `BaseModel` | The model instance to hydrate. Its public property type hints and attribute declarations define the validation schema. |

## Return value

Returns the passed `$model` instance with every property populated from `$rawData`. Values are written with the type declared on the property:

- Scalars are coerced to the declared type in [loose mode](#strict-and-loose-mode).
- `DateTimeInterface`-typed properties hydrate from date strings such as `1994-01-01T09:00:00+00:00`.
- Properties typed with a `BaseModel` class hydrate recursively from nested arrays.
- Union-typed properties keep the raw value's type when it already matches one of the arms; otherwise the first arm that can coerce it wins.

## Exceptions

| Exception | Thrown when |
| --------- | ----------- |
| `Attributes\Strictify\Exceptions\ValidationException` | Any field fails validation. Call `getErrors()` on the exception for the error list. |
| `ValueError` | An attribute spec is malformed, for example an unresolvable class-name arm, an unsupported `Type` value in an array attribute, or a `Dict` key type outside integer and string. |

`ValidationException::getErrors()` returns an array keyed by field path, each value holding one or more messages:

```php
<?php
use Attributes\Strictify\Exceptions\ValidationException;

try {
    validate($rawData, new User());
} catch (ValidationException $e) {
    $e->getErrors();
    // [
    //     'name' => ['Field is required'],
    //     'address.street' => ['Must be string'],
    // ]
}
```

Nested structures report dot-notation paths, including array element keys:

```php
<?php
// ['users.0.email' => ['Must be string'], 'scores.5' => ['Must be integer']]
```

## Validation rules

### Required fields

Every typed, non-nullable property is required. A missing key produces a `Field is required` error. Nullable properties accept a missing key or an explicit `null`, and properties with a declared default value keep it when the key is missing.

### Strict and loose mode

Validation is **strict by default**: a value must already have the declared type. Declare loose mode on the model to enable scalar coercion:

```php
<?php
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(strict: false)]
class User extends BaseModel
{
    public int $age;
    public string $name;
}

validate(['age' => '30', 'name' => 123], new User())->age; // int(30)
// ->name is the string "123"
```

In loose mode:

- Exact type matches always win over coercion, regardless of declaration order.
- When coercion is needed on a union property, arms are tried in declaration order: `int|bool` coerces `'1'` to `int(1)`, while `bool|int` coerces it to `bool(true)`.

`DateTimeInterface` properties coerce from date strings in both modes.

### Aliases

Map an input key to a property with the `Alias` attribute:

```php
<?php
use Attributes\Strictify\Fields\Alias;

class User extends BaseModel
{
    #[Alias('user_name')]
    public string $name;
}
```

`ModelConfigs` also offers automatic alias generators (`aliasGenerator: 'snake' | 'camel' | 'pascal'`).

### Error messages

Default messages follow the `Must be <type>` pattern, joined for unions (`Must be boolean, integer or string`). Override them per property with the `ErrorMessage` attribute:

```php
<?php
use Attributes\Strictify\Fields\ErrorMessage;

class User extends BaseModel
{
    #[ErrorMessage(required: 'Name is mandatory', type: 'Expected {expected}')]
    public string $name;
}
```

The `{expected}` placeholder renders the expected type list.

#### Translating error messages

Register a translator once per request with `ModelConfigs::setDefaultErrorMessages()`. The callable receives a message key and returns the translated template, or `null` to keep the default:

```php
<?php
use Attributes\Strictify\ModelConfigs;

ModelConfigs::setDefaultErrorMessages(
    fn (string $key): ?string => Catalog::translate($key, locale: 'pt_BR') // null = no translation
);
```

Built-in errors use these stable keys — one per error type:

| Key | Default message |
| --- | --- |
| `required` | `Field is required` |
| `type` | `Must be {expected}` |
| `alpha_dash` / `alpha_dash_type` | `Must only contain letters, numbers, dashes and underscores` |
| `ascii` / `ascii_type` | `Must only contain ASCII characters` |
| `between` / `between_type` | `Must be between {expected}` |
| `contains` | `Must contain {expected}` |
| `datetime` | `Must be a datetime in the format {expected}` |
| `ends_with` | `Must end with one of: {expected}` |
| `in` | `Must be one of: {expected}` |
| `max` / `min` | `Must be at most/least {expected}` |
| `multiple_of` | `Must be a multiple of {expected}` |
| `regex` | `Must match the pattern {expected}` |
| `size` | `Must have a size {expected}` |
| `starts_with` | `Must start with one of: {expected}` |
| `ulid` / `uuid` | `Must be a valid ULID/UUID` |

Single language — one catalog, registered once; keys it doesn't know keep the built-in English message:

```php
<?php
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\ModelConfigs;

use function Attributes\Strictify\validate;

ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => match ($key) {
    'required' => 'Campo obrigatório',
    'type' => 'Deve ser {expected}',
    'min' => 'Deve ser no mínimo {expected}',
    default => null,
});

class User extends BaseModel
{
    #[Min(18)]
    public int $age;

    public string $name;
}

validate(['age' => 5], new User());
// ValidationException: ['age' => ['Deve ser no mínimo 18'], 'name' => ['Campo obrigatório']]
```

Multiple languages — a catalog keyed by locale; switch locale by re-registering the translator (takes effect immediately, the per-property template cache is cleared):

```php
<?php
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\ModelConfigs;

use function Attributes\Strictify\validate;

final class Catalog
{
    private const MESSAGES = [
        'en' => [
            'required' => 'Field is required',
            'type' => 'Must be {expected}',
            'min' => 'Must be at least {expected}',
        ],
        'pt_BR' => [
            'required' => 'Campo obrigatório',
            'type' => 'Deve ser {expected}',
            'min' => 'Deve ser no mínimo {expected}',
        ],
    ];

    public static function useLocale(string $locale): void
    {
        ModelConfigs::setDefaultErrorMessages(
            fn (string $key): ?string => self::MESSAGES[$locale][$key] ?? null
        );
    }
}

Catalog::useLocale('pt_BR');

class User extends BaseModel
{
    #[Min(18)]
    public int $age;
}

validate(['age' => 5], new User());
// ValidationException: ['age' => ['Deve ser no mínimo 18']]

Catalog::useLocale('en');
validate(['age' => 5], new User());
// ValidationException: ['age' => ['Must be at least 18']]
```

Notes:

- `{field}`, `{value}` and `{expected}` placeholders are substituted after translation, so translated templates use them exactly like the defaults.
- `ErrorMessage` attribute values are also passed to the translator as keys. Since attribute arguments must be constant expressions (no callables), use a key string or a class constant there; when the translator returns `null`, the literal attribute string is used as-is, so existing attributes keep working unchanged:

```php
<?php
final class Msg
{
    public const NAME_REQUIRED = 'user.name.required';
    public const NAME_TYPE = 'user.name.type';
}

class User extends BaseModel
{
    #[ErrorMessage(required: Msg::NAME_REQUIRED, type: Msg::NAME_TYPE)]
    public string $name;
}
```

- Re-registering the translator (e.g. switching locale) takes effect immediately: resolved templates are cached per property per request, and the setter clears that cache.
- The translator must return `?string`; any other type throws a `ValueError`, and exceptions thrown inside it propagate to the caller of `validate()`.
- Userland constraints (`Attributes\Strictify\Fields\Constraint`) produce their `message()` in PHP, so the engine does not translate it — call your catalog from `message()` directly.

### Constraints

Value-level rules are declared with constraint attributes from `Attributes\Strictify\Fields\Constraints`. They run after the property's type check, before the value is written, and each failing constraint reports its own error:

```php
<?php
use Attributes\Strictify\Fields\Constraints\Between;
use Attributes\Strictify\Fields\Constraints\Regex;

class User extends BaseModel
{
    #[Between(18, 99)]
    public int $age;

    #[Regex('/^[a-z][a-z0-9-]*$/')]
    public string $username;
}
```

Constraints apply to the property value itself, not to `Sequence`/`Dict` elements. Custom constraints can be written in PHP userland by implementing `Attributes\Strictify\Fields\Constraint`. The [constraints reference](/constraints/) lists every built-in with its semantics and default message, and the userland contract.

## Array type validation

Typed `array` properties declare their element shape with the `Sequence`, `Union`, `Intersection` and `Dict` attributes:

```php
<?php
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\Type;

class Blog extends BaseModel
{
    #[Sequence(Type::string)]
    public array $tags;

    #[Union(int, string, sequence)]
    public array $mixed;

    #[Dict(key: Type::int, of: stdClass::class)]
    public array $lookup;
}
```

Element types come from the `Attributes\Strictify\Type` enum (`Type::int`, `Type::string`, `Type::bool`, `Type::float`, `Type::sequence`, `Type::object`, `Type::dict`) or the matching namespace constants (`int`, `string`, `bool`, `float`, `sequence`, `object`), and may combine with the bitwise OR operator (`bool | int`). Class, interface and enum names are accepted as strings (`DateTime::class`) and autoload on use.

- `Sequence` validates every element against one element type.
- `Union` accepts an element matching any declared arm.
- `Intersection` requires an element to match every declared class or interface. Its arms must name classes or interfaces only — enums are rejected, mirroring PHP intersection type hints.
- `Dict` validates keys against `$key` (integer and string types only) and values against `$of`.

The same attributes compose with `new` expressions to describe nested arrays:

```php
<?php
#[Sequence(of: new Union(string, int, float))]
public array $values;
```

Arguments of a composed `new` expression are validated at construction: an unsupported argument type throws a `TypeError` and a string that names no loadable class, interface or enum throws a `ValueError`.

## Hooks

Models may override two protected hooks, called by `validate()` around the validation run:

```php
<?php
abstract class BaseModel
{
    protected function beforeValidation(array $rawData, ModelConfigs $configs): array {}
    protected function afterValidation(array $rawData, ModelConfigs $configs): void {}
}
```

## Serializing back to an array: toDict()

`BaseModel::toDict()` returns the model's public properties as an array, keyed by the same resolved field names `validate()` expects (aliases and `aliasGenerator` applied — nested models with their own `#[ModelConfigs(aliasGenerator: ...)]` use it). `DateTimeInterface` values are formatted as ISO 8601 strings (`Y-m-d\TH:i:sP`), so the output round-trips through `validate()`:

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

When a property has a `#[DateTime(format: ..., timezone: ...)]` constraint, `toDict()` uses that format and timezone instead of the ISO 8601 default. The same format is also enforced during validation: on a `DateTime`-typed property, only strings matching the constraint's format exactly are accepted for coercion:

```php
<?php
class Event extends BaseModel
{
    #[DateTime(format: 'Y-m-d', timezone: 'Europe/Madrid')]
    public ?DateTime $startsAt;
}

validate(['startsAt' => '1994-01-01'], new Event());        // ok
validate(['startsAt' => '1994-01-01T09:00:00+00:00'], new Event()); // ValidationException

$event = validate(['startsAt' => '1994-01-01'], new Event());
$event->toDict();
// ['startsAt' => '1994-01-01'] (Madrid is UTC+1 in January, formatted as Y-m-d)
```

Nested `BaseModel` properties are serialized recursively. Uninitialized typed properties are omitted from the output.
