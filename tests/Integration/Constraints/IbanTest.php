<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Iban;

use function Attributes\Strictify\validate;

describe('Iban constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Iban]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'DE89370400440532013000',
        'GB29NWBK60161331926819',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Iban]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid IBAN']]);
        }
    })->with([
        'DE89370400440532013001',
        'XX00123',
        'DE89 3704',
        42,
    ]);
});
