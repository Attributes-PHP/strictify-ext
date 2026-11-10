<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Fields;

use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Fields\Intersection;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\Tests\Models\Enums\EnumStrTwoValues;
use Attributes\Strictify\Type;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

describe('Sequence constructor argument validation', function () {
    it('accepts every supported $of type', function ($of) {
        expect(new Sequence($of))->toBeInstanceOf(Sequence::class);
    })->with([
        Type::int->value,
        stdClass::class,
        DateTimeInterface::class,
        Type::class,
        Type::int,
        new Sequence(Type::int),
        new Union(Type::int, Type::string),
        new Intersection(DateTimeInterface::class),
        new Dict(key: Type::int, of: Type::int),
        EnumStrTwoValues::class,
    ]);

    it('throws a TypeError when the $of type is unsupported', function () {
        new Sequence(3.14);
    })->throws(
        TypeError::class,
        'Sequence::__construct(): Argument #1 ($of) must be of type int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict, float given',
    );

    it('throws a ValueError when the $of string is not a valid class', function () {
        new Sequence('DoesNotExist');
    })->throws(
        ValueError::class,
        'Sequence::__construct(): Argument #1 ($of) must be a valid class, interface or enum, "DoesNotExist" given',
    );
});
