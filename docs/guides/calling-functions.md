---
title: "Guide: calling functions"
description: "Invoke any callable with validated model parameters and injected dependencies using call()."
---

# Calling functions

`call($function, $params, $dependencies)` invokes a function or callable: every `BaseModel`-typed parameter hydrates from `$params` through the same engine [validate()](/validate/) uses, and every other parameter is injected from `$dependencies`. See the [call() reference](/call/) for the signature.

```php
<?php
// doctest: run
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

echo call('registerUser', ['age' => 30, 'name' => 'Andre']), "\n"; // registered Andre
```

A `ValidationException` thrown while hydrating a parameter propagates out of `call()` unchanged, with the same dot-notation field paths.

## Dependencies

Non-model parameters take their values from `$dependencies` (an array or any `ArrayAccess` container):

- Matched by parameter name first, then in positional order.
- Internal functions (whose signatures cannot declare models) receive them positionally: `call('str_repeat', [], ['x', 3])` returns `xxx`.
- A missing value for an optional parameter lets the engine apply the declared default.
- A missing value for a required parameter throws a `CallException` (an `ArgumentCountError` subclass): `No value provided for parameter $second of registerUser().`
- Variadic targets receive the leftover positional dependencies.

```php
<?php
// doctest: run
use function Attributes\Strictify\call;

function greet(string $name, string $punctuation = '!'): string
{
    return "Hello {$name}{$punctuation}";
}

echo call('greet', [], ['name' => 'Andre']), "\n"; // Hello Andre!
```

## Lazy containers

Instead of an array, pass any `ArrayAccess` instance — `ArrayObject` or a lazy DI container. The container is only asked for the dependencies the call actually consumes: `offsetGet()` runs solely for parameters that look a value up, so a service inside a lazy container is constructed only when a parameter needs it:

```php
<?php
// doctest: run
use function Attributes\Strictify\call;

$container = new ArrayObject(['logger' => (object) ['name' => 'app']]);

function ship(stdClass $logger): string
{
    return "shipping with {$logger->name}";
}

echo call('ship', [], $container), "\n"; // shipping with app
```

Exceptions thrown by a container's `offsetExists()`/`offsetGet()` propagate as-is. Passing anything other than an `array` or `ArrayAccess` throws a `TypeError`.

## Sensitive parameters

When the target's parameter is marked `#[\SensitiveParameter]`, `call()` reports a missing dependency positionally (`No value provided for parameter 2 of login().`) instead of naming the parameter, and the engine replaces the argument with a `\SensitiveParameterValue` object in `getTrace()`/`getTraceAsString()` of any exception the callable throws — while the callable still receives the real value:

```php
<?php
function login(string $user, #[\SensitiveParameter] string $password): void
{
    throw new RuntimeException('failed');
}

call('login', [], ['user' => 'andre', 'password' => $secret]);
// RuntimeException trace frame: login('andre', Object(SensitiveParameterValue))
```

## See also

- [call() reference](/call/)
- [Validating payloads](/guides/validating-payloads/)
- [Sensitive data](/guides/sensitive-data/)
