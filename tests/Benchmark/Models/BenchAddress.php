<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchAddress extends BaseModel
{
    public string $street;

    public string $city;
}
