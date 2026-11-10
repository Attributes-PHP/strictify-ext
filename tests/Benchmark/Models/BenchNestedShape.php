<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Type;

class BenchNestedShape extends BaseModel
{
    #[Sequence(new Sequence(Type::int))]
    public array $matrix;

    #[Dict(key: Type::string, of: new Sequence(Type::int))]
    public array $groups;
}
