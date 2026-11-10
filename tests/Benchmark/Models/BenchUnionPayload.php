<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\ModelConfigs;

use const Attributes\Strictify\bool;
use const Attributes\Strictify\float;
use const Attributes\Strictify\int;
use const Attributes\Strictify\string;

#[ModelConfigs(strict: false)]
class BenchUnionPayload extends BaseModel
{
    #[Union(int, string, bool)]
    public array $mixed;

    #[Union(int, float)]
    public array $coerced;
}
