<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\DateTime;
use Attributes\Strictify\Fields\Constraints\Email;
use Attributes\Strictify\Fields\Constraints\Iban;
use Attributes\Strictify\Fields\Constraints\MultipleOf;
use Attributes\Strictify\Fields\Constraints\Ulid;

use function Attributes\Strictify\validate;

describe('MultipleOf with a zero divisor', function () {
    it('accepts zero for int and float divisors alike', function (int|float $divisor, int|float $value) {
        $model = match (true) {
            is_int($divisor) => new class extends BaseModel {
                #[MultipleOf(0)]
                public int|float $value;
            },
            default => new class extends BaseModel {
                #[MultipleOf(0.0)]
                public int|float $value;
            },
        };

        expect(validate(['value' => $value], $model)->value)->toEqual($value);
    })->with([[0, 0], [0, 0.0], [0.0, 0.0], [0.0, 0]]);

    it('rejects every non-zero value', function (int|float $value) {
        $model = new class extends BaseModel {
            #[MultipleOf(0.0)]
            public int|float $value;
        };

        expect(fn () => validate(['value' => $value], $model))->toThrow(ValidationException::class);
    })->with([1, -1, 0.3, PHP_INT_MIN]);
});

describe('DateTime rejects dates that do not exist', function () {
    it('accepts real dates, including the lenient unpadded form', function (string $value) {
        $model = new class extends BaseModel {
            #[DateTime(format: 'Y-m-d')]
            public string $value;
        };

        expect(validate(['value' => $value], $model)->value)->toBe($value);
    })->with(['1994-01-01', '1994-1-1', '2024-02-29', '0000-01-01']);

    it('rejects dates the date extension would roll over', function (string $value) {
        $model = new class extends BaseModel {
            #[DateTime(format: 'Y-m-d')]
            public string $value;
        };

        expect(fn () => validate(['value' => $value], $model))->toThrow(ValidationException::class);
    })->with(['1994-02-30', '1994-04-31', '1994-13-01', '1994-00-10', '2023-02-29']);

    it('rejects a rolled-over timestamp in the default format', function () {
        $model = new class extends BaseModel {
            #[DateTime]
            public string $value;
        };

        expect(fn () => validate(['value' => '2024-02-31T10:00:00+00:00'], $model))->toThrow(ValidationException::class)
            ->and(validate(['value' => '2024-02-29T10:00:00+00:00'], $model)->value)
            ->toBe('2024-02-29T10:00:00+00:00');
    });
});

describe('Iban case handling', function () {
    it('accepts an IBAN in either letter case', function (string $value) {
        $model = new class extends BaseModel {
            #[Iban]
            public string $value;
        };

        expect(validate(['value' => $value], $model)->value)->toBe($value);
    })->with(['DE89370400440532013000', 'de89370400440532013000', 'De89370400440532013000']);

    it('still rejects a bad checksum or stray separators', function (string $value) {
        $model = new class extends BaseModel {
            #[Iban]
            public string $value;
        };

        expect(fn () => validate(['value' => $value], $model))->toThrow(ValidationException::class);
    })->with(['de89370400440532013001', 'DE89 3704 0044']);
});

describe('Ulid range', function () {
    it('accepts only the 128-bit range', function (string $value) {
        $model = new class extends BaseModel {
            #[Ulid]
            public string $value;
        };

        expect(validate(['value' => $value], $model)->value)->toBe($value);
    })->with(['01ARZ3NDEKTSV4RRFFQ69G5FAV', '7ZZZZZZZZZZZZZZZZZZZZZZZZZ', '01arz3ndektsv4rrffq69g5fav']);

    it('rejects a first character that overflows 128 bits', function (string $value) {
        $model = new class extends BaseModel {
            #[Ulid]
            public string $value;
        };

        expect(fn () => validate(['value' => $value], $model))->toThrow(ValidationException::class);
    })->with(['81ARZ3NDEKTSV4RRFFQ69G5FAV', 'ZZZZZZZZZZZZZZZZZZZZZZZZZZ']);
});

describe('Email top-level domain', function () {
    it('requires a TLD of at least two characters, like Domain', function (string $value) {
        $model = new class extends BaseModel {
            #[Email]
            public string $value;
        };

        expect(validate(['value' => $value], $model)->value)->toBe($value);
    })->with(['a@b.co', 'andre@example.com', 'a@mail.example.com', 'a@b.xn--p1ai']);

    it('rejects a one-letter TLD', function () {
        $model = new class extends BaseModel {
            #[Email]
            public string $value;
        };

        expect(fn () => validate(['value' => 'a@b.c'], $model))->toThrow(ValidationException::class);
    });
});
