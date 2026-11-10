<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Benchmark\Models;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\Fields\ErrorMessage;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Type;

class BenchAttributeUser extends BaseModel
{
    #[Alias('user_name')]
    public string $name;

    #[ErrorMessage(type: 'Expected {expected}')]
    public int $age;

    #[Sequence(Type::string)]
    public array $tags;
}
