---
title: "Overview"
description: "Pydantic-inspired PHP extension that validates data via type hints at native speed."
---

# Strictify

![Strictify](assets/images/strictify-wallpaper.png)

**Strictify** is a Pydantic-inspired PHP extension for validating data via type hints at lightning speed. The validation logic is implemented in C and compiled into the PHP engine, so your model types are the schema: no configuration files, no YAML.

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

- PHP 8.2 or later

## Installation

```bash
pie install attributes-php/strictify
```

New here? The [Quickstart](/quickstart/) takes under a minute.

## What's next

- [Quickstart](/quickstart/) — install, first model, first validation error.
- **Guides** — task-oriented: [validating payloads](/guides/validating-payloads/), [nested models and arrays](/guides/nested-models/), [aliases](/guides/aliases/), [error messages](/guides/error-messages/), [sensitive data](/guides/sensitive-data/), [calling functions](/guides/calling-functions/), [serializing models](/guides/serializing-models/).
- **Reference** — [validate()](/validate/), [toDict()](/toDict/), [call()](/call/), [ModelConfigs](/model-configs/), [Constraints](/constraints/), [Exceptions](/exceptions/), [API](/api/).
- [FAQ](/faq/) and [Changelog](/changelog/).

## Running this documentation locally

```bash
pip install zensical
python tools/generate_api_docs.py   # regenerate docs/api.md from the stubs
zensical serve
```

The site serves at `http://localhost:8000` and reloads as you edit the Markdown under `docs/`.
