<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\InvalidBaseModelException;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Regex;
use TypeError;

use function Attributes\Strictify\validate;

describe('Regex constructor argument validation', function () {
    it('promotes regex to a public property', function () {
        $constraint = new Regex('/^[a-z]+$/');

        expect($constraint->regex)->toBe('/^[a-z]+$/');
    });

    it('throws a TypeError for non-string patterns', function () {
        expect(fn () => new Regex(123))->toThrow(TypeError::class);
    });
});

describe('Regex validation', function () {
    it('accepts strings matching the pattern', function (string $value) {
        $model = new class extends BaseModel {
            #[Regex('/^[a-z]+$/')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with(['abc', 'a', 'abcdefghijklmnopqrstuvwxyz']);

    it('reports an error for non-matching strings', function (string $value) {
        $model = new class extends BaseModel {
            #[Regex('/^[a-z]+$/')]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must match the pattern '/^[a-z]+$/'"]]);
        }
    })->with(['ABC', 'abc1', 'a b', '']);

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[Regex('/x/')]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must match the pattern '/x/'"]]);
        }
    });

    it('rejects a pattern that does not compile when the field is first compiled', function () {
        $model = new class extends BaseModel {
            #[Regex('/(/')]
            public string $value;
        };

        // The pattern is validated at compile time, so validate() raises no
        // PCRE warning and the malformed model fails with a ValueError.
        expect(fn () => validate(['value' => 'x'], $model))->toThrow(
            InvalidBaseModelException::class,
            '::$value: the pattern does not compile: missing closing parenthesis at offset 1.',
        );
    });
});
