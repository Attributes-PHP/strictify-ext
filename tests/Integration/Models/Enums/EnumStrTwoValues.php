<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Models\Enums;

enum EnumStrTwoValues: string
{
    case One = 'one';
    case Two = 'two';
}
