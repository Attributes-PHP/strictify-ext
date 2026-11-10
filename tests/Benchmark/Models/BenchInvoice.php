<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;

class BenchInvoice extends BaseModel
{
    public string $number;

    public string $customer;

    public float $total;
}
