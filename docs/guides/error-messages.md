---
title: "Guide: error messages"
description: "Customize validation error messages per property with ErrorMessage, or translate them all with ErrorTranslator::set()."
---

# Error messages

Default messages follow the `Must be <type>` pattern, joined for unions (`Must be boolean, integer or string`). Every message has a stable translation key — the full catalog is in [Exceptions](/exceptions/#default-error-messages).

## Per-property overrides: ErrorMessage

Override the `required` and `type` templates of one property with the `ErrorMessage` attribute:

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

Register a translator once per request with `ErrorTranslator::set()`. The callable receives a message key and returns the translated template, or `null` to keep the default:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\ErrorTranslator;
use function Attributes\Strictify\validate;

ErrorTranslator::set(fn (string $key): ?string => match ($key) {
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
            fn (string $key): ?string => self::MESSAGES[$locale][$key] ?? null
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

## See also

- [Exceptions and the default error catalog](/exceptions/)
- [Constraints](/constraints/) — constraint error messages and their keys
- [Sensitive data](/guides/sensitive-data/) — `{value}` redaction for sensitive fields
