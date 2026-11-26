<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Fields;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
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

use function Attributes\Strictify\validate;

use const Attributes\Strictify\T_FLOAT;
use const Attributes\Strictify\T_INT;
use const Attributes\Strictify\T_NULL;

describe('Dict constructor argument validation', function () {
    it('accepts every supported $key type', function ($key) {
        expect(new Dict(key: $key, of: Type::string))->toBeInstanceOf(Dict::class);
    })->with([
        Type::int->value & Type::float->value,
        Type::int,
        Type::string,
        Type::float,
        Type::bool,
        Type::null,
        new Union(Type::int, Type::string),
        new Union(Type::int, Type::null),
        new Union(T_INT, T_FLOAT, T_NULL),
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

describe('Dict $uniqueValues', function () {
    it('defaults to false and accepts the flag', function () {
        expect((new Dict(key: T_INT, of: T_INT))->uniqueValues)->toBeFalse()
            ->and((new Dict(key: T_INT, of: T_INT, uniqueValues: true))->uniqueValues)
            ->toBeTrue();
    });

    it('accepts unique values', function () {
        $model = new class extends BaseModel {
            #[Dict(key: T_INT, of: Type::string, uniqueValues: true)]
            public array $values;
        };

        expect(validate(['values' => [1 => 'a', 2 => 'b']], $model)->values)->toBe([1 => 'a', 2 => 'b']);
    });

    it('reports duplicate values on the array property', function () {
        $model = new class extends BaseModel {
            #[Dict(key: T_INT, of: Type::string, uniqueValues: true)]
            public array $values;
        };

        try {
            validate(['values' => [1 => 'a', 2 => 'b', 3 => 'a']], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values' => ['Must contain only unique values']]);
        }
    });

    it('keeps duplicates allowed without the flag', function () {
        $model = new class extends BaseModel {
            #[Dict(key: T_INT, of: Type::string)]
            public array $values;
        };

        expect(validate(['values' => [1 => 'a', 2 => 'a']], $model)->values)->toBe([1 => 'a', 2 => 'a']);
    });
});
