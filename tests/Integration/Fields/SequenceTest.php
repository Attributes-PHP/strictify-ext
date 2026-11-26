<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Fields;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ErrorTranslator;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Dict;
use Attributes\Strictify\Fields\Intersection;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\ModelConfigs;
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

        expect($sequence->sequential)->toBeTrue()->and($sequence->uniqueValues)->toBeFalse();
    });

    it('accepts the flags as named and positional arguments', function () {
        $named = new Sequence(Type::int, uniqueValues: true, sequential: false);
        expect($named->sequential)->toBeFalse()->and($named->uniqueValues)->toBeTrue();

        $positional = new Sequence(Type::int, true, false);
        expect($positional->sequential)->toBeFalse()->and($positional->uniqueValues)->toBeTrue();
    });
});

describe('Sequence $sorted', function () {
    it('defaults to null', function () {
        expect((new Sequence(Type::int))->sorted)->toBeNull();
    });

    it('accepts every supported direction', function (string $sorted) {
        expect((new Sequence(Type::int, sorted: $sorted))->sorted)->toBe($sorted);
    })->with(['asc', 'ASC', 'desc', 'DESC']);

    it('accepts the direction as the fourth positional argument', function () {
        expect((new Sequence(Type::int, false, true, 'desc'))->sorted)->toBe('desc');
    });

    it('throws a ValueError for an unsupported direction', function () {
        new Sequence(Type::int, sorted: 'up');
    })->throws(
        ValueError::class,
        'Sequence::__construct(): Argument #4 ($sorted) must be "asc", "ASC", "desc", "DESC" or null, "up" given',
    );

    it('sorts the values ascending', function (array $input, array $expected) {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, sorted: 'asc')]
            public array $values;
        };

        expect(validate(['values' => $input], $model)->values)->toBe($expected);
    })->with([
        [[3, 1, 2], [1, 2, 3]],
        [[1, 2, 3], [1, 2, 3]],
        [[2, 2, 1], [1, 2, 2]],
        [[], []],
    ]);

    it('sorts the values descending', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::string, sorted: 'DESC')]
            public array $tags;
        };

        expect(validate(['tags' => ['b', 'c', 'a']], $model)->tags)->toBe(['c', 'b', 'a']);
    });

    it('keeps the input order and keys when $sorted is null', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, sorted: null, sequential: false)]
            public array $values;
        };

        expect(validate(['values' => [0 => 3, 5 => 1]], $model)->values)->toBe([0 => 3, 5 => 1]);
    });

    it('renumbers the keys after sorting', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, sorted: 'asc', sequential: false)]
            public array $values;
        };

        expect(validate(['values' => [4 => 3, 9 => 1]], $model)->values)->toBe([1, 3]);
    });

    it('applies sorting to a nested Sequence spec', function () {
        $model = new class extends BaseModel {
            #[Sequence(of: new Sequence(Type::int, sorted: 'desc'))]
            public array $matrix;
        };

        expect(validate(['matrix' => [[1, 2], [3, 0]]], $model)->matrix)->toBe([[2, 1], [3, 0]]);
    });

    it('does not sort when the array is invalid', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, sorted: 'asc')]
            public array $values;
        };

        try {
            validate(['values' => [3, 'x']], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values.1' => ['Must be integer']]);
        }
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

        expect(validate(['values' => []], $model)->values)->toBeEmpty();
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

    it('translates the sequenceKeys error key', function () {
        ErrorTranslator::set(fn (?string $key, string $message): ?string => $key === 'sequenceKeys'
            ? 'Deve ser uma lista sequencial'
            : null);

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

describe('Sequence $uniqueValues', function () {
    it('defaults to false', function () {
        expect((new Sequence(Type::int))->uniqueValues)->toBeFalse();
    });

    it('accepts unique values', function (array $input) {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, uniqueValues: true)]
            public array $values;
        };

        expect(validate(['values' => $input], $model)->values)->toBe($input);
    })->with([
        [[]],
        [[1]],
        [[1, 2, 3]],
    ]);

    it('reports duplicate values on the array property', function (array $input) {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, uniqueValues: true)]
            public array $values;
        };

        try {
            validate(['values' => $input], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values' => ['Must contain only unique values']]);
        }
    })->with([
        [[1, 1]],
        [[1, 2, 1]],
        [[3, 2, 3, 2]],
    ]);

    it('compares strictly, like the other membership rules', function () {
        // loose-mode coercion runs before the check, so '1' and 1 collide
        $loose = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::int, uniqueValues: true)]
                public array $values;
            };

        try {
            validate(['values' => [1, '1']], $loose);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['values' => ['Must contain only unique values']]);
        }
    });

    it('keeps duplicates allowed without the flag', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int)]
            public array $values;
        };

        expect(validate(['values' => [1, 1]], $model)->values)->toBe([1, 1]);
    });

    it('applies to a nested Sequence object', function () {
        $model = new class extends BaseModel {
            #[Sequence(of: new Sequence(Type::int, uniqueValues: true))]
            public array $matrix;
        };

        expect(validate(['matrix' => [[1, 2], [3]]], $model)->matrix)->toBe([[1, 2], [3]]);

        try {
            validate(['matrix' => [[1, 1]]], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['matrix.0' => ['Must contain only unique values']]);
        }
    });

    it('runs before sorting', function () {
        $model = new class extends BaseModel {
            #[Sequence(Type::int, uniqueValues: true, sorted: 'asc')]
            public array $values;
        };

        expect(validate(['values' => [3, 1, 2]], $model)->values)->toBe([1, 2, 3]);
    });
});
