<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Type;

class BenchBrokenDict extends BaseModel
{
    #[Dict(key: Type::int)]
    public array $items;
}
