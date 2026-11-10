<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(strict: false)]
class BenchSignup extends BaseModel
{
    #[Alias('user_name')]
    public string $name;

    public string $email;

    public int $age;
}
