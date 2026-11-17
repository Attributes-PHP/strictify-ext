<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;

use const Attributes\Strictify\T_INT;

class BenchUnionModelArm extends BaseModel
{
    #[Sequence(of: new Union(BenchAddress::class, T_INT))]
    public array $items;
}
