<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Sequence;

class BenchOrder extends BaseModel
{
    public int $id;

    #[Sequence(BenchAddress::class)]
    public array $addresses;
}
