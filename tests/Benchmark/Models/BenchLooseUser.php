<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ModelConfigs;

#[ModelConfigs(strict: false)]
class BenchLooseUser extends BaseModel
{
    public int $age;

    public string $name;

    public string $email;

    public bool $active;

    public float $balance;
}
