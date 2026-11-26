---
title: "call()"
description: 'Reference for the Attributes\Strictify\call() function: model hydration, dependencies and lazy ArrayAccess containers.'

---

# call()

Calls a function or callable with validated models: `BaseModel` parameters hydrate from the raw data array through the same engine [validate()](/validate/) uses, and every other parameter is injected from a dependencies array or `ArrayAccess` container.

```php
<?php
namespace Attributes\Strictify;

function call(string|callable $function, array $params, array|ArrayAccess $dependencies = [], int $maxDepth = 512): mixed
```

```php
<?php
use function Attributes\Strictify\call;

$result = call(myFunction(...), $rawData, $dependencies);
```

## Parameters

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `$function` | `string\|callable` | The function name or callable to invoke. |
| `$params` | `array` | The raw data array, hydrated into every `BaseModel` parameter exactly like [validate()](/validate/). |
| `$dependencies` | `array\|ArrayAccess` | Values for the non-model parameters, matched by parameter name first and then positionally. Defaults to `[]`. |
| `$maxDepth` | `int` | Nesting limit for this run: how many models deep the hydrated parameters may nest before a `MaxDepthException` is thrown. Defaults to 512; the value must be greater than 0. |

## Model hydration

Every `BaseModel`-typed parameter of the callable is hydrated from `$params` through the full validate() flow: strict/loose mode, [aliases](/guides/aliases/), array type-hint attributes, hooks and `ValidationException` with dot-notation field paths all behave identically.

```php
<?php
use Attributes\Strictify\BaseModel;
use function Attributes\Strictify\call;

class User extends BaseModel
{
    public int $age;
    public string $name;
}

function registerUser(User $user): string
{
    return "registered {$user->name}";
}

call('registerUser', ['age' => 30, 'name' => 'Andre']);
// "registered Andre"
```

A `ValidationException` thrown while hydrating a parameter propagates out of `call()` unchanged.

## Dependencies

Non-model parameters take their values from `$dependencies`:

- Matched by parameter name first, then in positional order.
- Internal functions (whose signatures cannot declare models) receive them positionally: `call('str_repeat', [], ['x', 3])` returns `xxx`.
- A missing value for an optional parameter lets the engine apply the declared default.
- A missing value for a required parameter throws a `CallCountException` (an `ArgumentCountError` subclass): `No value provided for parameter $second of registerUser().`.
- Variadic targets receive the leftover positional dependencies.

## Lazy ArrayAccess containers

Instead of an array, pass any `ArrayAccess` instance — `ArrayObject` or a lazy DI container. The container is only asked for the dependencies the call actually consumes: `offsetGet()` runs solely for parameters that look a value up by name or position, so a service inside a lazy container is constructed only when a parameter needs it.

```php
<?php
$container = new ArrayObject(['logger' => $logger]);

function ship(stdClass $logger): string
{
    return "shipping with {$logger->name}";
}

call('ship', [], $container);
```

Exceptions thrown by a container's `offsetExists()`/`offsetGet()` propagate as-is. Passing anything other than an `array` or `ArrayAccess` (an integer, a plain object, a string) throws a `TypeError`.

## Sensitive parameters

When the target's parameter is marked `#\SensitiveParameter`, `call()` keeps it out of the errors it raises itself: a missing required dependency is reported positionally (`No value provided for parameter 2 of login().`) instead of naming the parameter. Stack-trace redaction is unchanged — the engine replaces the argument with a `\SensitiveParameterValue` object in `getTrace()`/`getTraceAsString()` of any exception the callable throws, while the callable still receives the real value:

```php
<?php
function login(string $user, #[\SensitiveParameter] string $password): void
{
    throw new RuntimeException('failed');
}

call('login', [], ['user' => 'andre', 'password' => $secret]);
// RuntimeException trace frame: login('andre', Object(SensitiveParameterValue))
```

## Return value

`call()` returns the callable's return value, unchanged.

## See also

- [Calling functions guide](/guides/calling-functions/)
- [validate() reference](/validate/)
- [Sensitive data](/guides/sensitive-data/)
