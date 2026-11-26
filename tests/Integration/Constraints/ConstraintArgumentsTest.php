<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\InvalidBaseModelException;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Between;
use Attributes\Strictify\Fields\Constraints\Email;
use Attributes\Strictify\Fields\Constraints\EndsWith;
use Attributes\Strictify\Fields\Constraints\In;
use Attributes\Strictify\Fields\Constraints\Max;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\Constraints\MultipleOf;
use Attributes\Strictify\Fields\Constraints\Regex;
use Attributes\Strictify\Fields\Constraints\Size;
use Attributes\Strictify\Fields\Constraints\StartsWith;
use Attributes\Strictify\Fields\Constraints\Url;

use function Attributes\Strictify\validate;

/*
 * Attribute arguments bypass the constraint constructors' signature checks
 * only in the sense that a rejected argument surfaces on the first
 * validate() call: the arguments that would make a constraint unsatisfiable
 * (or a no-op) are rejected there, once per field compile.
 */
describe('constraint attribute argument rules', function () {
    it('rejects inverted and NAN bounds', function (string $attribute) {
        $model = match ($attribute) {
            'between-inverted' => new class extends BaseModel {
                #[Between(99, 18)]
                public int $value;
            },
            'between-nan' => new class extends BaseModel {
                #[Between(NAN, NAN)]
                public int $value;
            },
            'max-nan' => new class extends BaseModel {
                #[Max(NAN)]
                public int $value;
            },
            'min-nan' => new class extends BaseModel {
                #[Min(NAN)]
                public int $value;
            },
            'multiple-of-nan' => new class extends BaseModel {
                #[MultipleOf(NAN)]
                public int $value;
            },
        };

        expect(fn () => validate(['value' => 5], $model))->toThrow(InvalidBaseModelException::class);
    })->with(['between-inverted', 'between-nan', 'max-nan', 'min-nan', 'multiple-of-nan']);

    it('rejects inverted and negative size bounds', function (string $attribute) {
        $model = match ($attribute) {
            'inverted' => new class extends BaseModel {
                #[Size(5, 2)]
                public string $value;
            },
            'negative-min' => new class extends BaseModel {
                #[Size(-5)]
                public string $value;
            },
            'negative-max' => new class extends BaseModel {
                #[Size(null, -1)]
                public string $value;
            },
        };

        expect(fn () => validate(['value' => 'abc'], $model))->toThrow(InvalidBaseModelException::class);
    })->with(['inverted', 'negative-min', 'negative-max']);

    it('rejects an empty declared value list', function (string $attribute) {
        $model = match ($attribute) {
            'in' => new class extends BaseModel {
                #[In]
                public string $value;
            },
            'starts-with' => new class extends BaseModel {
                #[StartsWith]
                public string $value;
            },
            'ends-with' => new class extends BaseModel {
                #[EndsWith]
                public string $value;
            },
        };

        expect(fn () => validate(['value' => 'abc'], $model))->toThrow(
            InvalidBaseModelException::class,
            'must be declared.',
        );
    })->with(['in', 'starts-with', 'ends-with']);

    it('rejects an empty prefix or suffix, which matches every value', function () {
        $starts = new class extends BaseModel {
            #[StartsWith('')]
            public string $value;
        };
        $ends = new class extends BaseModel {
            #[EndsWith('')]
            public string $value;
        };

        expect(fn () => validate(['value' => 'abc'], $starts))->toThrow(
            InvalidBaseModelException::class,
            'an empty string matches every value',
        )
            ->and(fn () => validate(['value' => 'abc'], $ends))
            ->toThrow(InvalidBaseModelException::class, 'an empty string matches every value');
    });

    it('rejects a pattern that does not compile, without raising a PHP warning', function (string $pattern) {
        $warnings = [];
        set_error_handler(function (int $no, string $message) use (&$warnings) {
            $warnings[] = $message;

            return true;
        });

        try {
            $broken = match ($pattern) {
                '' => new class extends BaseModel {
                    #[Regex('')]
                    public string $value;
                },
                'abc' => new class extends BaseModel {
                    #[Regex('abc')]
                    public string $value;
                },
                '/[/' => new class extends BaseModel {
                    #[Regex('/[/')]
                    public string $value;
                },
                '/a/z' => new class extends BaseModel {
                    #[Regex('/a/z')]
                    public string $value;
                },
                'nul' => new class extends BaseModel {
                    #[Regex("/a\0bc/")]
                    public string $value;
                },
            };

            expect(fn () => validate(['value' => 'abc'], $broken))->toThrow(InvalidBaseModelException::class);
            expect($warnings)->toBeEmpty();
        } finally {
            restore_error_handler();
        }
    })->with(['', 'abc', '/[/', '/a/z', 'nul']);

    it('rejects an empty email domain list or empty url restriction', function (string $attribute) {
        $model = match ($attribute) {
            'email-list' => new class extends BaseModel {
                #[Email([])]
                public string $value;
            },
            'email-string' => new class extends BaseModel {
                #[Email('')]
                public string $value;
            },
            'url-string' => new class extends BaseModel {
                #[Url('')]
                public string $value;
            },
            'url-lists' => new class extends BaseModel {
                #[Url([], [])]
                public string $value;
            },
        };

        expect(fn () => validate(['value' => 'a@b.com'], $model))->toThrow(InvalidBaseModelException::class);
    })->with(['email-list', 'email-string', 'url-string', 'url-lists']);

    it('keeps rejecting the same model on every call', function () {
        $model = new class extends BaseModel {
            #[Between(99, 18)]
            public int $value;
        };

        expect(fn () => validate(['value' => 5], $model))->toThrow(
            InvalidBaseModelException::class,
            '$min (99) must be less than or equal to $max (18).',
        )
            ->and(fn () => validate(['value' => 5], $model))
            ->toThrow(InvalidBaseModelException::class)
            ->and(fn () => validate(['value' => 5], $model))
            ->toThrow(InvalidBaseModelException::class);
    });

    it('accepts the degenerate-but-satisfiable arguments', function () {
        $model = new class extends BaseModel {
            #[Between(5, 5)]
            public int $equal;
            #[Size]
            public string $unbounded;
            #[MultipleOf(0)]
            public int $zero;
            #[Regex('/^[a-z]+$/iDSUXn')]
            public string $pattern;
            #[Between(-INF, INF)]
            public int $wide;
        };

        $result = validate([
            'equal' => 5,
            'unbounded' => 'anything',
            'zero' => 0,
            'pattern' => 'ABC',
            'wide' => PHP_INT_MIN,
        ], $model);

        expect($result->equal)->toBe(5)
            ->and($result->zero)
            ->toBe(0);
    });
});

describe('constraint messages', function () {
    it('renders float bounds at full precision', function () {
        $model = new class extends BaseModel {
            #[Between(0.1, 0.300_000_000_000_000_04)]
            public float $value;
        };

        try {
            validate(['value' => 9.0], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be between 0.1 and 0.30000000000000004']]);
        }
    });

    it('renders integral floats without a fraction', function () {
        $model = new class extends BaseModel {
            #[Max(1.0)]
            public float $value;
        };

        try {
            validate(['value' => 9.0], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be at most 1']]);
        }
    });
});

describe('constraints on nullable properties', function () {
    it('skips the constraints for an explicit null, like a missing key', function () {
        $model = new class extends BaseModel {
            #[Regex('/^[a-z]+$/')]
            public ?string $name = null;
            #[Between(1, 10)]
            public ?int $age = null;
        };

        expect(validate(['name' => null, 'age' => null], $model)->name)->toBeNull()
            ->and(validate([], $model)->age)
            ->toBeNull();
    });

    it('still validates a non-null value', function () {
        $model = new class extends BaseModel {
            #[Regex('/^[a-z]+$/')]
            public ?string $name = null;
        };

        expect(fn () => validate(['name' => 'ABC'], $model))->toThrow(ValidationException::class);
    });
});
