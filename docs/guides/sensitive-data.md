---
title: "Guide: sensitive data"
description: "Keep secrets out of toDict() output, error messages and debug dumps with #[Sensitive] and #[\\SensitiveParameter]."
---

# Sensitive data

Mark a property with `#[Sensitive]` to keep its value out of the library's output — like `#[\SensitiveParameter]` keeps parameter values out of stack traces. The property itself still holds the real value after `validate()`:

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Sensitive;
use function Attributes\Strictify\validate;

class Credentials extends BaseModel
{
    public string $username;

    #[Sensitive]
    public string $password;
}

$model = validate(['username' => 'andre', 'password' => 'hunter2'], new Credentials());

echo $model->password, "\n";                       // hunter2
print_r($model->toDict());                         // Array ( [username] => andre ) — password omitted
print_r($model->toDict(includeSensitive: true));   // Array ( [username] => andre [password] => hunter2 )
```

## What is masked, and where

- **`toDict()`** omits sensitive properties entirely; pass `includeSensitive: true` to serialize their real values. The omission composes with [aliases](/guides/aliases/) (the key — property name by default, alias with `useAlias: true` — is dropped) and with nested models (each level omits its own sensitive fields).
- **Error messages**: the `{value}` placeholder in error message templates is always redacted to `***` for sensitive properties, so secrets do not leak into `ValidationException` messages.
- **Debug output** masks them as well: `var_dump()`, `print_r()`, `debug_zval_dump()` — including model objects dumped as arguments inside exception traces — render a sensitive property as `***`, for every property type (ints, floats, strings, bools, arrays, objects and nested `BaseModel` properties):

```php
<?php
// doctest: run
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Sensitive;
use function Attributes\Strictify\validate;

class Credentials extends BaseModel
{
    public string $username;

    #[Sensitive]
    public string $password;
}

$model = validate(['username' => 'andre', 'password' => 'hunter2'], new Credentials());

print_r($model, true); // Credentials Object ( [username] => andre [password] => *** )
json_encode($model);   // {"username":"andre","password":"hunter2"} — real values
```

`BaseModel` installs a custom `get_properties_for` object handler that only alters the engine's debug view (`ZEND_PROP_PURPOSE_DEBUG`); property access, `foreach`, `(array)` casts, `get_object_vars()`, `json_encode()`, `serialize()` and `var_export()` all keep the real values.

## Sensitive parameters in call()

When the target's parameter is marked `#[\SensitiveParameter]`, [call()](/call/) keeps it out of the errors it raises itself and the engine redacts it in stack traces. See [Calling functions](/guides/calling-functions/).

## See also

- [Serializing models](/guides/serializing-models/)
- [Error messages](/guides/error-messages/)
- [toDict() reference](/toDict/)
