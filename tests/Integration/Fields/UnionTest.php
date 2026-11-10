<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Fields;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Fields\Intersection;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\ModelConfigs;
use Attributes\Strictify\Tests\Models\Enums\EnumStrTwoValues;
use Attributes\Strictify\Tests\Models\Interfaces\CountableInterface;
use Attributes\Strictify\Tests\Models\Interfaces\IterableInterface;
use Attributes\Strictify\Tests\Models\IterableCountableClass;
use Attributes\Strictify\Type;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

use function Attributes\Strictify\validate;

use const Attributes\Strictify\bool;
use const Attributes\Strictify\int;
use const Attributes\Strictify\string;

class UnionAddress extends BaseModel
{
    public string $street;
}

describe('Union constructor argument validation', function () {
    it('accepts every supported arm type', function ($arm) {
        expect(new Union($arm))->toBeInstanceOf(Union::class);
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

    it('throws a TypeError when an arm type is unsupported', function () {
        new Union(Type::int, 3.14);
    })->throws(
        TypeError::class,
        'Union::__construct(): Argument #2 must be of type int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict, float given',
    );

    it('throws a ValueError when a string arm is not a valid class', function () {
        new Union(Type::int, 'DoesNotExist');
    })->throws(
        ValueError::class,
        'Union::__construct(): Argument #2 must be a valid class, interface or enum, "DoesNotExist" given',
    );
});

describe('Union attribute array data validation', function () {
    it('accepts an array of ints and strings, keeping raw types', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(int, string)]
                public array $items;
            };

        $result = validate(['items' => [1, 'a', '30', 2]], $model);
        expect($result->items)->toBe([1, 'a', '30', 2]);
    });

    it('hydrates raw arrays through a BaseModel class arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(UnionAddress::class, int)]
                public array $items;
            };

        $result = validate(['items' => [['street' => 'Main St'], 7]], $model);
        expect($result->items[0])->toBeInstanceOf(UnionAddress::class);
        expect($result->items[0]->street)->toBe('Main St');
        expect($result->items[1])->toBe(7);
    });

    it('reports nested model errors from a class arm with dot notation paths', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(UnionAddress::class, int)]
                public array $items;
            };

        try {
            validate(['items' => [[]]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0.street' => ['Field is required'],
            ]);
        }
    });

    it('accepts enum cases for an enum arm and rejects scalars', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(EnumStrTwoValues::class, int)]
                public array $items;
            };

        $result = validate(['items' => [EnumStrTwoValues::One, 3]], $model);
        expect($result->items)->toBe([EnumStrTwoValues::One, 3]);

        try {
            validate(['items' => ['one']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0' => ['Must be Attributes\Strictify\Tests\Models\Enums\EnumStrTwoValues or integer'],
            ]);
        }
    });

    it('accepts a composed Intersection arm describing a nested array', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool, new Intersection(CountableInterface::class, IterableInterface::class))]
                public array $items;
            };

        $result = validate(['items' => [true, [new IterableCountableClass]]], $model);
        expect($result->items[0])->toBeTrue();
        expect($result->items[1][0])->toBeInstanceOf(IterableCountableClass::class);

        try {
            validate(['items' => [[new stdClass]]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0.0' => [
                    'Must be Attributes\Strictify\Tests\Models\Interfaces\CountableInterface and'
                        .' Attributes\Strictify\Tests\Models\Interfaces\IterableInterface',
                ],
            ]);
        }
    });

    it('rejects scalar coercion in strict mode but keeps exact matches', function () {
        $model = new
            #[ModelConfigs(strict: true)]
            class extends BaseModel {
                #[Union(bool, int)]
                public array $items;
            };

        $result = validate(['items' => [true, 1]], $model);
        expect($result->items)->toBe([true, 1]);

        try {
            validate(['items' => ['1']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0' => ['Must be boolean or integer'],
            ]);
        }
    });

    it('accepts an empty array', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(int, string)]
                public array $items;
            };

        $result = validate(['items' => []], $model);
        expect($result->items)->toBeEmpty();
    });
});
