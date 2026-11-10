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

describe('Dict constructor argument validation', function () {
    it('accepts every supported $key type', function ($key) {
        expect(new Dict(key: $key, of: Type::string))->toBeInstanceOf(Dict::class);
    })->with([
        Type::int->value & Type::float->value,
        Type::int,
        new Union(Type::int, Type::string),
    ]);

    it('accepts every supported $of type', function ($of) {
        expect(new Dict(key: Type::string, of: $of))->toBeInstanceOf(Dict::class);
    })->with([
        Type::int->value & Type::dict->value,
        stdClass::class,
        DateTimeInterface::class,
        Type::class,
        Type::int,
        new Union(Type::int, Type::string),
        new Intersection(DateTimeInterface::class),
        new Sequence(Type::int),
        new Dict(key: Type::int, of: Type::int),
        EnumStrTwoValues::class,
    ]);

    it('throws a TypeError when the $key type is unsupported', function () {
        new Dict(key: 3.14, of: Type::string);
    })->throws(
        TypeError::class,
        'Dict::__construct(): Argument #1 ($key) must be of type int|\Attributes\Strictify\Type|Union, float given',
    );

    it('throws a TypeError when the $key is a string', function () {
        new Dict(key: stdClass::class, of: Type::string);
    })->throws(
        TypeError::class,
        'Dict::__construct(): Argument #1 ($key) must be of type int|\Attributes\Strictify\Type|Union, string given',
    );

    it('throws a TypeError when the $of type is unsupported', function () {
        new Dict(key: Type::string, of: 3.14);
    })->throws(
        TypeError::class,
        'Dict::__construct(): Argument #2 ($of) must be of type int|string|\Attributes\Strictify\Type|Union|Intersection|Sequence|Dict, float given',
    );

    it('throws a ValueError when the $of string is not a valid class', function () {
        new Dict(key: Type::string, of: 'DoesNotExist');
    })->throws(
        ValueError::class,
        'Dict::__construct(): Argument #2 ($of) must be a valid class, interface or enum, "DoesNotExist" given',
    );
});
