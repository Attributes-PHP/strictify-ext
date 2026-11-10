<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchCartItem extends BaseModel
{
    public string $sku;

    public int $quantity;

    public float $unitPrice;
}
