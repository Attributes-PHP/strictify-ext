<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Fields;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Fields\Intersection;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\ErrorTranslator;
use Attributes\Strictify\Tests\Models\Enums\EnumStrTwoValues;
use Attributes\Strictify\Type;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

use function Attributes\Strictify\validate;

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

describe('Sequence constructor flags', function () {
    it('defaults sequential to true and uniqueValues to false', function () {
        $sequence = new Sequence(Type::int);

        expect($sequence->sequential)->toBeTrue()
            ->and($sequence->uniqueValues)->toBeFalse();
    });

    it('accepts the flags as named and positional arguments', function () {
        $named = new Sequence(Type::int, uniqueValues: true, sequential: false);
        expect($named->sequential)->toBeFalse()
            ->and($named->uniqueValues)->toBeTrue();

        $positional = new Sequence(Type::int, true, false);
        expect($positional->sequential)->toBeFalse()
            ->and($positional->uniqueValues)->toBeTrue();
    });
});

describe('Sequence sequential key validation', function () {
    afterEach(function () {
        ErrorTranslator::set(fn (?string $key, string $message): ?string => null);
    });

    it('accepts a list by default', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public array $values;
        };

        expect(validate(['values' => [1, 2, 3]], $model)->values)->toBe([1, 2, 3]);
    });

    it('accepts an empty array by default', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public array $values;
        };

        expect(validate(['values' => []], $model)->values)->toBe([]);
    });

    it('rejects string keys by default', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public array $values;
        };

        try {
            validate(['values' => [1, 'x' => 2]], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values.x' => ['Must be integer']]);
        }
    });

    it('rejects gaps in the integer keys by default', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::string)]
            public array $tags;
        };

        try {
            validate(['tags' => [0 => 'a', 2 => 'b']], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['tags.2' => ['Must be a sequential list (keys 0, 1, 2, ...)']]);
        }
    });

    it('accepts gapped integer keys with sequential: false', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, sequential: false)]
            public array $values;
        };

        expect(validate(['values' => [0 => 123, 2 => 654]], $model)->values)->toBe([0 => 123, 2 => 654]);
    });

    it('rejects string keys with sequential: false', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, sequential: false)]
            public array $values;
        };

        try {
            validate(['values' => ['a' => 1, 5 => 2]], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values.a' => ['Must be integer']]);
        }
    });

    it('applies the sequential rule to nested Sequence specs', function () {
        $model = new class extends BaseModel {
            #[Sequence(of: new Sequence(Type::int))]
            public array $matrix;
        };

        expect(validate(['matrix' => [[1, 2], [3]]], $model)->matrix)->toBe([[1, 2], [3]]);

        try {
            validate(['matrix' => [['a' => 1]]], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['matrix.0.a' => ['Must be integer']]);
        }
    });

    it('translates the sequence_keys error key', function () {
        ErrorTranslator::set(fn (?string $key, string $message): ?string => $key === 'sequence_keys' ? 'Deve ser uma lista sequencial' : null);

        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public array $values;
        };

        try {
            validate(['values' => [5 => 1]], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values.5' => ['Deve ser uma lista sequencial']]);
        }
    });
});
