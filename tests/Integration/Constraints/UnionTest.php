<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Union;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\ModelConfigs;
use Attributes\Strictify\Type;
use DateTimeImmutable;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

use function Attributes\Strictify\validate;

use const Attributes\Strictify\float;
use const Attributes\Strictify\int;

describe('Union constructor argument validation', function () {
    it('collects the variadic arms on the $of property', function () {
        $constraint = new Union(Type::int, Type::float, stdClass::class, int | float);

        expect($constraint->of)->toBe([Type::int, Type::float, stdClass::class, int | float]);
    });

    it('accepts zero arms per the stub', function () {
        $constraint = new Union;

        expect($constraint->of)->toBeEmpty();
    });

    it('throws a TypeError for unsupported arm types', function (mixed $arm) {
        expect(fn () => new Union($arm))->toThrow(TypeError::class);
    })->with([
        'float' => [3.14],
        'array' => [[]],
        'null' => [null],
        'nested array shape' => [new Sequence(Type::int)],
        'plain object' => [new \stdClass],
    ]);

    it('throws a ValueError for an invalid type mask', function (int $mask) {
        expect(fn () => new Union($mask))->toThrow(ValueError::class);
    })->with([
        'zero' => [0],
        'negative' => [-1],
        'unknown bit' => [1 << 30],
    ]);

    it('throws a ValueError when a string arm is not a valid class', function () {
        new Union(Type::int, 'DoesNotExist');
    })->throws(
        ValueError::class,
        'Union::__construct(): Argument #2 must be a valid class, interface or enum, "DoesNotExist" given',
    );
});

describe('Union validation on a union-typed property', function () {
    it('accepts a value matching any declared arm', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int, Type::float)]
            public int|float $example;
        };

        expect(validate(['example' => 1], $model)->example)->toBe(1)
            ->and(validate(['example' => 1.5], $model)->example)
            ->toBe(1.5);
    });

    it('accepts a bitmask arm combining several types', function () {
        $model = new class extends BaseModel {
            #[Union(int | float)]
            public int|float $example;
        };

        expect(validate(['example' => 1], $model)->example)->toBe(1)
            ->and(validate(['example' => 1.5], $model)->example)
            ->toBe(1.5);
    });

    it('narrows the native union to a subset of arms', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int)]
            public int|float $example;
        };

        expect(validate(['example' => 42], $model)->example)->toBe(42);

        try {
            validate(['example' => 1.5], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['example' => ['Must be integer']]);
        }
    });

    it('sees the coerced value in loose mode', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(Type::int)]
                public int|float $example;
            };

        // '1' coerces to int(1) and passes; '1.5' coerces to float(1.5) and fails
        expect(validate(['example' => '1'], $model)->example)->toBe(1);

        try {
            validate(['example' => '1.5'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['example' => ['Must be integer']]);
        }
    });

    it('matches class arms with instanceof', function () {
        $model = new class extends BaseModel {
            #[Union(stdClass::class)]
            public stdClass|DateTimeImmutable $example;
        };

        expect(validate(['example' => new stdClass], $model)->example)->toBeInstanceOf(stdClass::class);

        try {
            validate(['example' => new DateTimeImmutable], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['example' => ['Must be stdClass']]);
        }
    });

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

    it('never matches when no arms are declared', function () {
        $model = new class extends BaseModel {
            #[Union]
            public mixed $value;
        };

        try {
            validate(['value' => 1], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be mixed']]);
        }
    });
});
