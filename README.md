# Strictify Extension

<p align="center">
    <img src="https://raw.githubusercontent.com/Attributes-PHP/strictify/main/docs/assets/images/strictify-wallpaper.png" alt="strictify logo" width="300">
</p>
<p align="center">
    <a href="https://github.com/Attributes-PHP/strictify/actions"><img alt="GitHub Actions Workflow Status (main)" src="https://img.shields.io/github/actions/workflow/status/Attributes-PHP/strictify/tests.yml"></a>
    <a href="https://codecov.io/gh/Attributes-PHP/strictify" ><img alt="Code Coverage" src="https://codecov.io/gh/Attributes-PHP/strictify/graph/badge.svg?token=8N7N9NMGLG"/></a>
    <a href="https://packagist.org/packages/Attributes-PHP/strictify"><img alt="Latest Version" src="https://img.shields.io/packagist/v/Attributes-PHP/strictify"></a>
    <a href="https://packagist.org/packages/Attributes-PHP/strictify"><img alt="Supported PHP Versions" src="https://img.shields.io/badge/8.2%2B?label=versions"></a>
    <a href="https://opensource.org/licenses/MIT"><img alt="Software License" src="https://img.shields.io/badge/Licence-MIT-brightgreen"></a>
</p>

------

**Strictify Extension** is a Pydantic-inspired PHP library for validating data via type hints at lightning speed ⚡️

## Prerequisites

- PHP 8.2 or later

We aim to support versions that haven't reached their end-of-life.

## How it works?

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
var_dump($user->birthday); // object(DateTime) { ["date"] => string(26) "1994-01-01 09:00:00.000000", (...) }
```

### Need to call a function?

```php
<?php

use function Attributes\Strictify\call;

function myFunction(User $user) {
    var_dump($user->age);      // int(30)
    var_dump($user->birthday); // object(DateTime) { ["date"] => string(26) "1994-01-01 09:00:00.000000", (...) }
}

call("myFunction", $rawData);
```

## Installation

```bash
pie install attributes-php/strictify
```

Strictify Extension was created by **[André Gil](https://www.linkedin.com/in/andre-gil/)** and is open-sourced software licensed under the **[MIT license](https://opensource.org/licenses/MIT)**.
