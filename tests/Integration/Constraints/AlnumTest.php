<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Alnum;

use function Attributes\Strictify\validate;

describe('Alnum constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Alnum]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'abc123',
        'ABC',
        '',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Alnum]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must contain only letters and numbers']]);
        }
    })->with([
        'abc-123',
        'a b',
        42,
    ]);
});

describe('Alnum constraint extra characters', function () {
    it('accepts declared extra characters', function () {
        $model = new class extends BaseModel {
            #[Alnum('-')]
            public mixed $value;
        };

        expect(validate(['value' => 'a-b1'], $model)->value)->toBe('a-b1');
    });
});
