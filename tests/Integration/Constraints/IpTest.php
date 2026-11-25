<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Ip;

use function Attributes\Strictify\validate;

describe('Ip constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Ip]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        '192.168.0.1',
        '::1',
        '2001:db8::ff00:42:8329',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Ip]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid IP address']]);
        }
    })->with([
        '256.1.1.1',
        '1.2.3',
        'hello',
        42,
    ]);
});

describe('Ip constraint version parameter', function () {
    it('restricts to IPv4 when version is v4', function () {
        $model = new class extends BaseModel {
            #[Ip('v4')]
            public mixed $value;
        };

        expect(validate(['value' => '10.0.0.1'], $model)->value)->toBe('10.0.0.1');

        try {
            validate(['value' => '::1'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid IP address']]);
        }
    });

    it('restricts to IPv6 when version is v6', function () {
        $model = new class extends BaseModel {
            #[Ip('v6')]
            public mixed $value;
        };

        expect(validate(['value' => '::1'], $model)->value)->toBe('::1');

        try {
            validate(['value' => '10.0.0.1'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid IP address']]);
        }
    });
});
