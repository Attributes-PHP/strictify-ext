<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchProfile extends BaseModel
{
    public string $username;

    public string $email;

    public int $loginCount;
}
