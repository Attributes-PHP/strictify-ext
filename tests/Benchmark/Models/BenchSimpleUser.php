<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchSimpleUser extends BaseModel
{
    public int $age;

    public string $name;

    public string $email;

    public bool $active;

    public float $balance;
}
