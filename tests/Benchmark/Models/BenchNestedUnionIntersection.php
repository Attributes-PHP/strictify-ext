<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Intersection;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;

use const Attributes\Strictify\T_INT;

class BenchNestedUnionIntersection extends BaseModel
{
    #[Sequence(of: new Union(T_INT, new Intersection(BenchServiceInterface::class, BenchBootableInterface::class)))]
    public array $payloads;
}
