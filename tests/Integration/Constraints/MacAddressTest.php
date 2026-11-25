<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\MacAddress;

use function Attributes\Strictify\validate;

describe('MacAddress constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[MacAddress]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        '00:1A:2B:3C:4D:5E',
        '00-1a-2b-3c-4d-5e',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[MacAddress]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid MAC address']]);
        }
    })->with([
        '00:1A:2B:3C:4D',
        '001A2B3C4D5E',
        'zz:1a:2b:3c:4d:5e',
        42,
    ]);
});
