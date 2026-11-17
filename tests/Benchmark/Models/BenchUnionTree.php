<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;

use const Attributes\Strictify\T_INT;
use const Attributes\Strictify\T_STRING;

class BenchUnionTree extends BaseModel
{
    public string $name;

    #[Sequence(of: new Union(T_INT, T_STRING, BenchUnionTree::class))]
    public array $children;
}
