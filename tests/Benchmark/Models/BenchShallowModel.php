<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchShallowModel extends BaseModel
{
    public string $name;

    public int $count;

    public float $ratio;
}
