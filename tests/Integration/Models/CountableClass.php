<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Models;

use Attributes\Strictify\Tests\Models\Interfaces\CountableInterface;

class CountableClass implements CountableInterface
{
    public function count(): string
    {
        return 'CountableClass->count(...)';
    }
}
