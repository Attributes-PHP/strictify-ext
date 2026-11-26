---
title: "Guide: error messages"
description: "Customize validation error messages per property with ErrorMessage, or translate them all with ErrorTranslator::set()."
---

# Error messages

Default messages follow the `Must be <type>` pattern, joined for unions (`Must be boolean, integer or string`). Every message has a stable translation key — the full catalog is [below](#default-error-messages).

## Default error messages

Each default message has a stable translation key passed as the first argument to the translator registered with `ErrorTranslator::set()` (the default message itself is the second argument):

| Key | Default message |
| --- | --- |
| `required` | `Field is required` |
| `type` | `Must be {expected}` |
| `alphaDash` / `alphaDashType` | `Must only contain letters, numbers, dashes and underscores` |
| `ascii` / `asciiType` | `Must only contain ASCII characters` |
| `between` / `betweenType` | `Must be between {expected}` |
| `contains` | `Must contain {expected}` |
| `datetime` | `Must be a datetime in the format {expected}` |
| `endsWith` | `Must end with one of: {expected}` |
| `in` | `Must be one of: {expected}` |
| `max` / `min` | `Must be at most/least {expected}` |
| `multipleOf` | `Must be a multiple of {expected}` |
| `regex` | `Must match the pattern {expected}` |
| `size` | `Must have a size {expected}` |
| `startsWith` | `Must start with one of: {expected}` |
| `ulid` / `uuid` | `Must be a valid ULID/UUID` |
| `union` | `Must be {expected}` |
| `sequenceKeys` | `Must be a sequential list (keys 0, 1, 2, ...)` (gapped or out-of-order integer key in a `#[Sequence]` array) |
| `extra` | `Extra inputs are not permitted` (unknown key under `extra: 'forbid'`) |

See [Placeholders](#placeholders) for the `{field}`, `{value}` and `{expected}` substitutions available in every template.

## Placeholders

Every template — the defaults, translated messages and `ErrorMessage` overrides — supports the same three placeholders, substituted after translation:

| Placeholder | Renders |
| --- | --- |
| `{field}` | The resolved field name — the dot-notation path for nested fields, the [alias](/guides/aliases/) when one is declared |
| `{value}` | The rejected value: strings are quoted (`'abc'`), `null`/`true`/`false` render literally, numbers as-is, arrays as `array`, stringable objects via `__toString()` — redacted to `***` for [`#[Sensitive]`](/guides/sensitive-data/) properties |
| `{expected}` | For `type`/`union` errors the expected type list (`Must be integer, float or string`); for constraint errors the constraint parameters (`18`, a regex pattern, ...) |

`{expected}` renders as an empty string for the `extra` error (unknown key under `extra: 'forbid'`), which has no owning property. Userland constraint `message()` output goes through the same substitution, so `{field}` and `{value}` work there too — see [Constraints](/constraints/).

## Per-property overrides: ErrorMessage

Override any of a property's templates with the `ErrorMessage` attribute — one named argument per key from the [default catalog](#default-error-messages) above (`required`, `type`, `min`, `max`, `sequenceKeys`, ...), each defaulting to that key's default message:

```php
#[ErrorMessage(required: 'Name is mandatory', min: 'Too small, at least {expected}')]
```

The catalog keys, the `#[ErrorMessage]` arguments and the keys handed to the translator are the same strings, so `#[ErrorMessage(sequenceKeys: ...)]` overrides the message reported under `sequenceKeys`.

The `extra` key is discarded: extra inputs have no owning property, so that error always uses the translator or its default.

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\ErrorMessage;
use function Attributes\Strictify\validate;

class User extends BaseModel
{
    #[ErrorMessage(required: 'Name is mandatory', type: 'Expected {expected}')]
    public string $name;
}

try {
    validate(['name' => 123], new User());
} catch (ValidationException $e) {
    print_r($e->getErrors()); // Array ( [name] => Array ( [0] => Expected string ) )
}

try {
    validate([], new User());
} catch (ValidationException $e) {
    print_r($e->getErrors()); // Array ( [name] => Array ( [0] => Name is mandatory ) )
}
```

The `{expected}` placeholder renders the expected type list. `{field}` and `{value}` are also available.

## Translating error messages

Register a translator once per request with `ErrorTranslator::set()`. The callable receives the stable message key as `?string $key` (`null` for custom `#[ErrorMessage]` values, which have no key) and the default message template as `string $message`, and returns the translated template — or `null` to keep the default:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\ErrorTranslator;
use function Attributes\Strictify\validate;

ErrorTranslator::set(fn (?string $key, string $message): ?string => match ($key) {
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

try {
    validate(['age' => 5], new User());
} catch (ValidationException $e) {
    print_r($e->getErrors());
    // Array ( [age] => Array ( [0] => Deve ser no mínimo 18 )
    //         [name] => Array ( [0] => Campo obrigatório ) )
}
```

Multiple languages — a catalog keyed by locale; switch locale by re-registering the translator (takes effect immediately, the per-property template cache is cleared):

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\ErrorTranslator;
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
        ErrorTranslator::set(
            fn (?string $key, string $message): ?string => $key === null ? null : (self::MESSAGES[$locale][$key] ?? null)
        );
    }
}

class User extends BaseModel
{
    #[Min(18)]
    public int $age;
}

Catalog::useLocale('pt_BR');
try {
    validate(['age' => 5], new User());
} catch (ValidationException $e) {
    print_r($e->getErrors()); // Array ( [age] => Array ( [0] => Deve ser no mínimo 18 ) )
}
```

Notes:

- `ErrorMessage` attribute values have no stable key: the translator receives `null` as `$key` and the attribute value as `$message`. Since attribute arguments must be constant expressions (no callables), use a key string or a class constant there; when the translator returns `null`, the literal attribute string is used as-is, so existing attributes keep working unchanged:

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
- The translator must return `?string`; any other type throws a `ValueError`, and exceptions thrown inside it propagate to the caller of `validate()`. Declare the first parameter as `?string $key` — it is `null` for custom `#[ErrorMessage]` values.
- Userland constraints (`Attributes\Strictify\Fields\Constraint`) produce their `message()` in PHP, so the engine does not translate it — call your catalog from `message()` directly.

## See also

- [Exceptions](/exceptions/) — exception types and the `getErrors()` structure
- [Constraints](/constraints/) — constraint error messages and their keys
- [Sensitive data](/guides/sensitive-data/) — `{value}` redaction for sensitive fields
