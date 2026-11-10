<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchCategory extends BaseModel
{
    public string $slug;

    public string $name;
}
