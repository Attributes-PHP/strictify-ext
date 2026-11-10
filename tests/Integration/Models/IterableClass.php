<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Models;

use Attributes\Strictify\Tests\Models\Interfaces\IterableInterface;

class IterableClass implements IterableInterface
{
    public function iterate(): string
    {
        return 'IterableClass->iterate(...)';
    }
}
