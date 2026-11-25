<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\HexRgbColor;

use function Attributes\Strictify\validate;

describe('HexRgbColor constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[HexRgbColor]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        '#fff',
        '#FFFFFF',
        '#ff000080',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[HexRgbColor]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid hex RGB color']]);
        }
    })->with([
        '#ff',
        '#fffff',
        'fff',
        '#ggg',
        42,
    ]);
});
