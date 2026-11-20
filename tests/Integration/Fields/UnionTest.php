<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Fields;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\BaseException;
use Attributes\Strictify\Exceptions\InvalidBaseModelException;
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
use DateTimeImmutable;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

use function Attributes\Strictify\validate;

use const Attributes\Strictify\T_BOOL;
use const Attributes\Strictify\T_FLOAT;
use const Attributes\Strictify\T_INT;
use const Attributes\Strictify\T_NULL;
use const Attributes\Strictify\T_STRING;

class UnionAddress extends BaseModel
{
    public string $street;
}

class UnionHintMismatch extends BaseModel
{
    #[Union(T_INT)]
    public int|float $example;
}

describe('Union constructor argument validation', function () {
    it('accepts every supported arm type', function ($arm) {
        expect(new Union($arm))->toBeInstanceOf(Union::class);
    })->with([
        Type::int->value,
        Type::null->value,
        T_NULL,
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
                #[Sequence(of: new Union(T_INT, T_STRING))]
                public array $items;
            };

        $result = validate(['items' => [1, 'a', '30', 2]], $model);
        expect($result->items)->toBe([1, 'a', '30', 2]);
    });

    it('hydrates raw arrays through a BaseModel class arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(of: new Union(UnionAddress::class, T_INT))]
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
                #[Sequence(of: new Union(UnionAddress::class, T_INT))]
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
                #[Sequence(of: new Union(EnumStrTwoValues::class, T_INT))]
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
                #[Sequence(of: new Union(T_BOOL, new Intersection(CountableInterface::class, IterableInterface::class)))]
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
                #[Sequence(of: new Union(T_BOOL, T_INT))]
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
                #[Sequence(of: new Union(T_INT, T_STRING))]
                public array $items;
            };

        $result = validate(['items' => []], $model);
        expect($result->items)->toBeEmpty();
    });
});

describe('Union declared directly on a non-array property', function () {
    it('checks the whole value against the arms on an int|float property', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int, Type::float)]
            public int|float $example;
        };

        expect(validate(['example' => 1], $model)->example)->toBe(1)
            ->and(validate(['example' => 1.5], $model)->example)
            ->toBe(1.5);
    });

    it('throws InvalidBaseModelException when the arms are a subset of the union hint', function () {
        $model = new class extends BaseModel {
            #[Union(T_INT)]
            public int|float $example;
        };

        validate(['example' => 42], $model);
    })->throws(
        InvalidBaseModelException::class,
        'must contain the same type-hints as the native union hint (integer or float); got (integer).'
    );

    it('throws InvalidBaseModelException when the arms add a type the hint does not have', function () {
        $model = new class extends BaseModel {
            #[Union(T_INT, T_FLOAT, T_STRING)]
            public int|float $example;
        };

        validate(['example' => 42], $model);
    })->throws(
        InvalidBaseModelException::class,
        'must contain the same type-hints as the native union hint (integer or float); got (integer, float or string).'
    );

    it('requires T_NULL in the arms of a nullable hint', function () {
        $model = new class extends BaseModel {
            #[Union(T_STRING)]
            public ?string $example;
        };

        validate(['example' => 'a'], $model);
    })->throws(
        InvalidBaseModelException::class,
        'must contain the same type-hints as the native union hint (string or null); got (string).'
    );

    it('reports the hint and arm union types in the full error message', function () {
        try {
            validate(['example' => 1], new UnionHintMismatch());
            expect(false)->toBeTrue();
        } catch (InvalidBaseModelException $e) {
            expect($e->getMessage())->toBe('Union attribute on property Attributes\\Strictify\\Tests\\Integration\\Fields\\UnionHintMismatch::$example must contain the same type-hints as the native union hint (integer or float); got (integer).');
        }
    });

    it('accepts T_NULL as an arm of a nullable hint', function () {
        $model = new class extends BaseModel {
            #[Union(T_STRING, T_NULL)]
            public ?string $example;
        };

        expect(validate(['example' => null], $model)->example)->toBeNull()
            ->and(validate(['example' => 'a'], $model)->example)
            ->toBe('a');
    });

    it('includes T_NULL for a nullable union hint', function () {
        $model = new class extends BaseModel {
            #[Union(T_INT | T_FLOAT | T_NULL)]
            public int|float|null $example;
        };

        expect(validate(['example' => null], $model)->example)->toBeNull()
            ->and(validate(['example' => 1.5], $model)->example)
            ->toBe(1.5);
    });

    it('accepts a bitmask arm combining several types', function () {
        $model = new class extends BaseModel {
            #[Union(T_INT | T_FLOAT)]
            public int|float $example;
        };

        expect(validate(['example' => 1], $model)->example)->toBe(1)
            ->and(validate(['example' => 1.5], $model)->example)
            ->toBe(1.5);
    });

    it('sees the coerced value in loose mode', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(T_INT | T_FLOAT)]
                public int|float $example;
            };

        expect(validate(['example' => '1'], $model)->example)->toBe(1)
            ->and(validate(['example' => '1.5'], $model)->example)
            ->toBe(1.5);
    });

    it('works on an untyped property', function () {
        $model = new class extends BaseModel {
            #[Union(T_INT, T_STRING)]
            public $example;
        };

        expect(validate(['example' => 'a'], $model)->example)->toBe('a');

        try {
            validate(['example' => 1.5], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['example' => ['Must be integer or string']]);
        }
    });

    it('works on a mixed property with a scalar value', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int)]
            public mixed $example;
        };

        expect(validate(['example' => 1], $model)->example)->toBe(1);

        try {
            validate(['example' => 'a'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['example' => ['Must be integer']]);
        }
    });

    it('matches class arms with instanceof', function () {
        $model = new class extends BaseModel {
            #[Union(stdClass::class, DateTimeImmutable::class)]
            public stdClass|DateTimeImmutable $example;
        };

        expect(validate(['example' => new stdClass], $model)->example)->toBeInstanceOf(stdClass::class)
            ->and(validate(['example' => new DateTimeImmutable()], $model)->example)
            ->toBeInstanceOf(DateTimeImmutable::class);
    });

    it('throws InvalidBaseModelException when class arms do not match the union hint', function () {
        $model = new class extends BaseModel {
            #[Union(stdClass::class)]
            public stdClass|DateTimeImmutable $example;
        };

        validate(['example' => new stdClass()], $model);
    })->throws(
        InvalidBaseModelException::class,
        'must contain the same type-hints as the native union hint (stdClass or DateTimeImmutable); got (stdClass).'
    );

    it('joins the arm names in the error message', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int, Type::string, DateTimeInterface::class)]
            public mixed $value;
        };

        try {
            validate(['value' => 1.5], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be integer, string or DateTimeInterface']]);
        }
    });

    it('throws a ValueError for an array value instead of treating the arms as an element spec', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int)]
            public mixed $example;
        };

        validate(['example' => [1]], $model);
    })->throws(
        InvalidBaseModelException::class,
        'Union cannot be applied directly to array values; use Sequence(of: new Union(...)) or Dict(of: ...) instead.',
    );

    it('throws a ValueError when declared directly on an array property', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int)]
            public array $values;
        };

        validate(['values' => [1]], $model);
    })->throws(InvalidBaseModelException::class, 'Union cannot be declared directly on an array property');
});

describe('array attribute placement rules', function () {
    it('throws a ValueError for Sequence on a non-array-like property', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public int $count;
        };

        validate(['count' => 5], $model);
    })->throws(InvalidBaseModelException::class, 'Sequence/Dict can only be declared on array-like properties');

    it('throws a ValueError for Dict on a union property without an array arm', function () {
        $model = new class extends BaseModel {
            #[Dict(key: T_INT, of: T_STRING)]
            public int|float $ratio;
        };

        validate(['ratio' => 1.5], $model);
    })->throws(InvalidBaseModelException::class, 'Sequence/Dict can only be declared on array-like properties');

    it('reports Must be array for a non-array value on a mixed property with Sequence', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public mixed $values;
        };

        try {
            validate(['values' => 5], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values' => ['Must be array']]);
        }
    });

    it('enforces Sequence on an untyped property when the value is an array', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public $values;
        };

        expect(validate(['values' => [1, 2]], $model)->values)->toBe([1, 2]);

        try {
            validate(['values' => [1, 'a']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values.1' => ['Must be integer']]);
        }
    });

    it('reports Must be array for a non-array value on an untyped property with Sequence', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public $values;
        };

        try {
            validate(['values' => 5], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values' => ['Must be array']]);
        }
    });
});
