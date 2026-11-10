<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Models\Enums;

enum EnumStrThreeValues: string
{
    case One = 'one';
    case Two = 'two';
    case Three = 'three';
}
