---
title: "Overview"
description: "Strictify brings data validation via type-hinting to PHP — as a pure-PHP Composer library or, at native speed, as a C extension."
---

# Strictify

![Strictify](assets/images/strictify-wallpaper.png)

**Strictify** validates data via type-hinting at lightning speed. Your model types are the schema: no configuration files, no YAML.

The same `Attributes\Strictify` API ships as two interchangeable packages, and this documentation covers both:

| | Package | Install |
|---|---|---|
| **Strictify** (pure PHP) | [`attributes-php/strictify`](https://github.com/Attributes-PHP/strictify) | `composer require attributes-php/strictify` |
| **Strictify Extension** | [`attributes-php/strictify-ext`](https://github.com/Attributes-PHP/strictify-ext) | `pie install attributes-php/strictify-ext` |

Same models, same functions, same behavior. The library offers the widest compatibility: it runs anywhere Composer runs — shared hosting, managed platforms, serverless. The extension offers maximum performance for those who control the host and can install a PHP extension — on average roughly an order of magnitude faster, up to ~96x on nested payloads (see [Benchmarks](/benchmarks/)).

```php
<?php

use function Attributes\Strictify\validate;
use Attributes\Strictify\BaseModel;

class User extends BaseModel
{
    public float|int $age;
    public ?DateTime $birthday;
}

$rawData = [
    'age' => 30,
    'birthday' => '1994-01-01T09:00:00+00:00',
];

$user = validate($rawData, new User());

var_dump($user->age);      // int(30)
var_dump($user->birthday); // object(DateTime) "1994-01-01 09:00:00.000000" (+00:00)
```

## How it works

```text
$rawData ──▶ validate($rawData, new User())
               │
               ├─ valid ───▶ hydrated User model (typed properties)
               └─ invalid ─▶ ValidationException ─▶ getErrors(): ['field.path' => ['message', ...]]
```

## Requirements

- PHP 8.3 or later

## Installation

Pick one flavor — they expose the same `Attributes\Strictify` API:

```bash
# Pure-PHP library — widest compatibility, runs anywhere Composer runs
composer require attributes-php/strictify

# C extension — maximum performance, for hosts you control
pie install attributes-php/strictify-ext
```

New here? The [Quickstart](/quickstart/) takes under a minute.

## What's next

- [Quickstart](/quickstart/) — install, first model, first validation error.
- **Guides** — task-oriented: [validating payloads](/guides/validating-payloads/), [nested models and arrays](/guides/nested-models/), [aliases](/guides/aliases/), [error messages](/guides/error-messages/), [sensitive data](/guides/sensitive-data/), [calling functions](/guides/calling-functions/), [serializing models](/guides/serializing-models/).
- **Reference** — [validate()](/validate/), [toDict()](/toDict/), [call()](/call/), [ModelConfigs](/model-configs/), [Constraints](/constraints/), [Exceptions](/exceptions/), [API](/api/).
- **Resources** — [Benchmarks](/benchmarks/), [FAQ](/faq/) and [Changelog](/changelog/).

## Using with AI assistants

This documentation is also available in LLM-friendly Markdown: [llms.txt](/llms.txt) indexes every page (each page is also served as Markdown at its own URL), and [llms-full.txt](/llms-full.txt) contains the full documentation in a single file.
