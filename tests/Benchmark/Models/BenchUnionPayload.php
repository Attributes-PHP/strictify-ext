<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\ModelConfigs;

use const Attributes\Strictify\T_BOOL;
use const Attributes\Strictify\T_FLOAT;
use const Attributes\Strictify\T_INT;
use const Attributes\Strictify\T_STRING;

#[ModelConfigs(strict: false)]
class BenchUnionPayload extends BaseModel
{
    #[Sequence(of: new Union(T_INT, T_STRING, T_BOOL))]
    public array $mixed;

    #[Sequence(of: new Union(T_INT, T_FLOAT))]
    public array $coerced;
}
