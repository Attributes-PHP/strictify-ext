<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Models\Enums;

enum EnumIntTwoValues: int
{
    case One = 1;
    case Two = 2;
}
