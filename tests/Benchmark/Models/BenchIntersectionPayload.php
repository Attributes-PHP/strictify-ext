<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Intersection;

class BenchIntersectionPayload extends BaseModel
{
    #[Intersection(BenchServiceInterface::class, BenchBootableInterface::class)]
    public array $services;
}
