<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;

use const Attributes\Strictify\int;
use const Attributes\Strictify\string;

class BenchUnionTree extends BaseModel
{
    public string $name;

    #[Union(int, string, BenchUnionTree::class)]
    public array $children;
}
