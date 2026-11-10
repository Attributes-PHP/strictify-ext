<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Models;

use Attributes\Strictify\Tests\Models\Interfaces\CountableInterface;
use Attributes\Strictify\Tests\Models\Interfaces\IterableInterface;

class IterableCountableClass implements CountableInterface, IterableInterface
{
    public function count(): string
    {
        return 'IterableCountableClass->class(...)';
    }

    public function iterate(): string
    {
        return 'IterableCountableClass->iterate(...)';
    }
}
