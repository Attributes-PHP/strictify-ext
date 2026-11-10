<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Type;

class BenchTagList extends BaseModel
{
    public string $name;

    #[Sequence(Type::string)]
    public array $tags;
}
