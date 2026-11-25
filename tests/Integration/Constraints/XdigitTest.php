<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Xdigit;

use function Attributes\Strictify\validate;

describe('Xdigit constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Xdigit]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        '0aF9',
        'FF',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Xdigit]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must contain only hexadecimal digits']]);
        }
    })->with([
        '0g',
        '',
        42,
    ]);
});
