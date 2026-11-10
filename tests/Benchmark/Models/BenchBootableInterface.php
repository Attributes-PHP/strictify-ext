<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

interface BenchBootableInterface
{
    public function boot(): void;
}
