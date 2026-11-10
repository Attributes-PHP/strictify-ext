<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchProduct extends BaseModel
{
    public string $sku;

    public string $title;

    public float $price;

    public int $stock;
}
