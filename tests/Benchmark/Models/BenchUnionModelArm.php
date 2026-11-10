<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;

use const Attributes\Strictify\int;

class BenchUnionModelArm extends BaseModel
{
    #[Union(BenchAddress::class, int)]
    public array $items;
}
