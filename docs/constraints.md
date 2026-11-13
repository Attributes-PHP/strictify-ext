---
title: "Constraints"
description: 'Reference for the Strictify constraint attributes: the built-in constraints and how to write custom constraints in PHP userland.'

---

# Constraints

Constraints are attributes that add value-level rules on top of the native type hints. They run after the property's type check succeeds (so they always see a value of the declared type, or the coerced value in loose mode) and before the value is written to the model — a failing constraint keeps the value out of the model.

```php
use Attributes\Strictify\BaseModel;
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

Constraints compose: every constraint declared on a property runs, and each failing one reports its own error under the resolved field name (the `Alias` name, when one is declared). `ModelConfigs(stopFirstError: true)` aborts the run at the first error, as with type errors.

Attribute constructor arguments are validated at construction — an argument rejected by the constructor (`#[Between('a', 'b')]`) throws a `TypeError` on the first `validate()` call and keeps throwing on every call until the attribute is fixed.

## Built-in constraints

All built-ins live in `Attributes\Strictify\Fields\Constraints` and may be declared on properties and parameters.

| Attribute | Constructor | Fails when |
| --------- | ----------- | ---------- |
| [`AlphaDash`](#alphadash) | `()` | the string contains characters other than letters, numbers, dashes and underscores |
| [`Ascii`](#ascii) | `()` | the string contains bytes outside 7-bit ASCII (0x00-0x7F) |
| [`Between`](#between) | `(int\|float $min, int\|float $max)` | the number is outside the inclusive range |
| [`Contains`](#contains) | `(string\|int\|float ...$values, at least one)` | an array is missing one of the declared values, or a string is missing one of them as a substring |
| [`DateTime`](#datetime) | `(string $format = 'X-m-d\TH:i:sP', DateTimeZone\|string\|null $timezone = null)` | the value does not parse in the declared format and timezone |
| [`EndsWith`](#endswith) | `(string\|int\|float ...$values)` | the string ends with none of the declared values |
| [`In`](#in) | `(string\|int\|float\|bool ...$values)` | the value matches none of the declared values |
| [`Max`](#max) | `(int\|float $value)` | the number exceeds the inclusive upper bound |
| [`Min`](#min) | `(int\|float $value)` | the number is below the inclusive lower bound |
| [`MultipleOf`](#multipleof) | `(int\|float $value)` | the number is not a multiple of the divisor |
| [`Regex`](#regex) | `(string $regex)` | the string does not match the pattern (`preg_match` semantics: a partial match passes) |
| [`Size`](#size) | `(?int $min = null, ?int $max = null)` | the string length or array count is outside the inclusive bounds |
| [`StartsWith`](#startswith) | `(string\|int\|float ...$values)` | the string starts with none of the declared values |
| [`Ulid`](#ulid) | `()` | the string is not a valid ULID (26 Crockford base32 characters, either letter case) |
| [`Uuid`](#uuid) | `()` | the string is not a canonical UUID (8-4-4-4-12 hex groups separated by dashes, either letter case) |

Named constructor arguments work everywhere: `#[Size(min: 3)]` validates only the lower bound, `#[DateTime(timezone: 'Europe/London')]` parses in that timezone.

### Examples

One section per built-in constraint, covering every scenario of that constraint. All examples assume `use Attributes\Strictify\BaseModel;` and the constraint class imported from `Attributes\Strictify\Fields\Constraints`.

#### `AlphaDash`

Letters, numbers, dashes and underscores only.

```php
class Profile extends BaseModel
{
    #[AlphaDash] // 'user-1_name' passes
    public string $slug;
}
```

Fails: `'user name'` (space), `'user.name'` (dot), and any non-string value.

#### `Ascii`

7-bit ASCII bytes (0x00-0x7F) only.

```php
class Label extends BaseModel
{
    #[Ascii] // 'hello ~!@#' passes
    public string $text;
}
```

Fails: `'héllo'` (UTF-8 bytes above 0x7F), and any non-string value.

#### `Between`

Inclusive numeric range; works for `int` and `float`.

```php
class User extends BaseModel
{
    #[Between(18, 99)] // 18 and 99 pass (inclusive), 17 and 100 fail
    public int $age;

    #[Between(0.5, 2.5)] // 1.5 passes, 3.0 fails
    public float $ratio;
}
```

Fails: any non-numeric value, with a dedicated type error.

#### `Contains`

Arrays and strings are both supported.

```php
class Article extends BaseModel
{
    // array: every declared value must be present
    #[Contains('php', 'dev')] // ['php', 'dev', 'oss'] passes, ['php'] fails
    public array $tags;

    // string: every declared value must appear as a substring
    #[Contains('example')] // 'see example.com' passes, 'see other.org' fails
    public string $link;
}
```

Fails: any value that is neither array nor string.

#### `DateTime`

Parses in the declared format and timezone; `DateTimeInterface` instances pass as-is.

```php
class Event extends BaseModel
{
    #[DateTime(format: 'Y-m-d')] // '1994-01-01' passes, '01/01/1994' fails
    public string $date;

    #[DateTime(timezone: 'Europe/London')] // parses in that timezone
    public string $startsAt;

    #[DateTime(format: 'Y-m-d')] // hydrated to a DateTime, then the constraint sees the instance and passes
    public \DateTime $birthday;
}
```

Fails: a string that does not parse in the declared format, and any value that is neither string nor `DateTimeInterface`.

#### `EndsWith`

The string must end with one of the declared values; numeric values match by their string representation.

```php
class Contact extends BaseModel
{
    #[EndsWith('.com', '.net')] // 'andre@example.com' passes, 'andre@example.org' fails
    public string $email;

    #[EndsWith(42)] // 'answer 42' passes, 'answer 43' fails
    public string $note;
}
```

Fails: non-string values, and strings ending with none of the declared values (including a suffix longer than the value).

#### `In`

Strict membership in the declared values.

```php
class Grade extends BaseModel
{
    #[In('a', 'b', 'c')] // 'b' passes, 'd' fails
    public string $letter;

    #[In(1, 2, true)] // 1 and true pass; strict: the string '1' fails
    public int|bool $code;
}
```

Fails: any value identical to none of the declared values.

#### `Max`

Inclusive numeric upper bound.

```php
class Score extends BaseModel
{
    #[Max(100)] // 100 passes, 101 fails
    public float $points;
}
```

Fails: numbers above the bound and non-numeric values.

#### `Min`

Inclusive numeric lower bound.

```php
class Cart extends BaseModel
{
    #[Min(1)] // 1 passes, 0 fails
    public int $quantity;
}
```

Fails: numbers below the bound and non-numeric values.

#### `MultipleOf`

Integer modulo for int pairs, `fmod` with a small tolerance for float pairs.

```php
class Pack extends BaseModel
{
    #[MultipleOf(5)] // 20 passes, 21 fails
    public int $size;

    #[MultipleOf(0.1)] // 0.3 passes (float tolerance), 0.35 fails
    public float $weight;

    #[MultipleOf(0)] // only 0 passes
    public int $zero;
}
```

Fails: non-multiples and non-numeric values.

#### `Regex`

`preg_match` semantics: a partial match passes.

```php
class Account extends BaseModel
{
    #[Regex('/^[a-z][a-z0-9-]*$/')] // 'user-name' passes, 'User Name' fails
    public string $username;

    #[Regex('/a+/')] // 'bbbaaabb' passes (partial match), 'bbb' fails
    public string $hasA;
}
```

Fails: non-matching strings, strings when the pattern fails to compile, and non-string values.

#### `Size`

String length in bytes or array element count, within inclusive bounds; either bound may be omitted.

```php
class Profile extends BaseModel
{
    #[Size(min: 3, max: 20)] // 'andre' passes, 'ab' and a 21-character string fail
    public string $name;

    #[Size(min: 1)] // only the lower bound applies: [] fails, ['a'] passes
    public array $tags;

    #[Size(max: 5)] // only the upper bound applies: [] passes, 6 elements fail
    public array $scores;

    #[Size] // no bounds: any string or array passes
    public array|string $anything;
}
```

Fails: sizes outside the bounds and values that are neither string nor array.

#### `StartsWith`

The string must start with one of the declared values; numeric values match by their string representation.

```php
class Link extends BaseModel
{
    #[StartsWith('https://')] // 'https://example.com' passes, 'http://example.com' fails
    public string $url;

    #[StartsWith(42)] // '4242' passes, '142' fails
    public string $code;
}
```

Fails: non-string values, non-matching strings, and prefixes longer than the value.

#### `Ulid`

Exactly 26 Crockford base32 characters, either letter case.

```php
class Event extends BaseModel
{
    #[Ulid] // '01ARZ3NDEKTSV4RRFFQ69G5FAV' (or lowercase) passes
    public string $id;
}
```

Fails: wrong length (25 or 27 chars), the letters `I`, `L`, `O`, `U` in either case, any other character, and non-string values.

#### `Uuid`

Canonical 8-4-4-4-12 hexadecimal UUID, either letter case.

```php
class Request extends BaseModel
{
    #[Uuid] // '123e4567-e89b-12d3-a456-426614174000' and '123E4567-E89B-12D3-A456-426614174000' pass
    public string $id;
}
```

Fails: wrong length, a dash in the wrong position, a non-hex character, and non-string values.

### Value semantics

- **Numeric constraints** (`Between`, `Max`, `Min`, `MultipleOf`) apply to `int` and `float` values; any other type fails. `MultipleOf` compares integer pairs with integer modulo and float pairs with `fmod` plus a small tolerance, so `0.3` counts as a multiple of `0.1`. Only `0` is a multiple of `0`.
- **String constraints** (`AlphaDash`, `Ascii`, `Regex`, `StartsWith`, `EndsWith`, `Ulid`, `Uuid`) fail for non-string values. `StartsWith`/`EndsWith` accept numeric declared values and match them by their string representation.
- **Membership constraints** (`In`, `Contains`) compare strictly: `In('1')` rejects the integer `1`. On arrays, `Contains` requires every declared value to be present; on strings, every declared value must appear as a substring.
- **`DateTime`** parses with the date extension in the declared format and timezone. `DateTimeInterface` instances pass as-is, and an invalid timezone name throws a `ValueError` at construction.
- **`Size`** measures strings in bytes and arrays in element count. With only one bound declared, only that bound applies; with neither, every value passes.

### Error messages

Each constraint has a default message following the `Must be ...` pattern, with the constraint parameters rendered in place of `{expected}`:

```php
// ['age' => ['Must be between 18 and 99']]
// ['username' => ["Must match the pattern '/^[a-z][a-z0-9-]*$/'"]]
// ['size' => ['Must have a size of at least 3']
// ['tag' => ["Must be one of: 'a', 'b', true"]]
```

The `ErrorMessage` attribute only declares `required` and `type` templates, so constraint errors always use their default messages. Constraints applied through userland classes can return any message they want (see below).

## Custom constraints

Write a constraint in PHP userland by implementing `Attributes\Strictify\Fields\Constraint`. Any attribute class implementing it is applied during `validate()` exactly like the built-ins — the same run order, error collection and `stopFirstError` handling. PHP enforces the interface methods and their signatures at class declaration time.

```php
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Constraint;

#[Attribute(Attribute::TARGET_PROPERTY | Attribute::TARGET_PARAMETER)]
final class Even implements Constraint
{
    public function validate(mixed $value): bool
    {
        return is_int($value) && $value % 2 === 0;
    }

    public function message(): string
    {
        return 'Must be an even number';
    }
}

class Product extends BaseModel
{
    #[Even]
    public int $quantity;
}

validate(['quantity' => 3], new Product());
// ValidationException: ['quantity' => ['Must be an even number']]
```

The contract:

- `validate(mixed $value): bool` receives the value after the native type check and returns whether it satisfies the constraint.
- `message(): string` is called only on failure and returns the error message. It supports the `{field}` and `{value}` placeholders, substituted the same way as in built-in templates.
- Constructor arguments are passed through unchanged, named arguments included: `#[LessThan(bound: 10)]` reaches the constructor as a named parameter.
- Exceptions thrown inside `validate()` or `message()` propagate to the caller of `validate()`, like model hook errors.

The built-in constraint classes are final: custom constraints implement the `Constraint` interface rather than extending a built-in.

Unlike the built-ins, userland constraints may declare `Attribute::IS_REPEATABLE` and be declared multiple times on one property. Instances are compiled once per property and reused for every `validate()` call of the request.

Attribute classes that do not exist or do not implement `Constraint` are ignored, exactly like any other attribute the validation engine does not know — a property can carry unrelated documentation attributes without affecting validation.
