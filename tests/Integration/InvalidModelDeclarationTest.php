<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\InvalidBaseModelException;
use Attributes\Strictify\Exceptions\StrictifyThrowable;
use Attributes\Strictify\Fields\Constraints\Between;
use Attributes\Strictify\Fields\Constraints\Email;
use Attributes\Strictify\Fields\Constraints\In;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\Constraints\Size;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\Type;
use TypeError;
use ValueError;

use function Attributes\Strictify\validate;

/*
 * Attribute arguments belong to the model declaration, so a rejected argument
 * throws InvalidBaseModelException — an engine TypeError (wrong argument type
 * for the stub signature), a ValueError from a nested attribute or a custom
 * argument rule included. Constructing the same attribute object in userland
 * is not a declaration: there the engine's TypeError/ValueError still apply.
 */
describe('invalid model declarations', function () {
    it('wraps an engine argument type error', function () {
        $model = new class extends BaseModel {
            #[Min('fifteen')]
            public int $age;
        };

        expect(fn () => validate(['age' => 20], $model))->toThrow(
            InvalidBaseModelException::class,
            'Argument #1 ($value) must be of type int|float, string given',
        );
    });

    it('wraps an unsupported arm value', function () {
        $model = new class extends BaseModel {
            #[Union(Type::int, 1.5)]
            public $value;
        };

        expect(fn () => validate(['value' => 1], $model))->toThrow(InvalidBaseModelException::class, 'Argument #2');
    });

    it('wraps a class arm that cannot be resolved', function () {
        $model = new class extends BaseModel {
            #[Union('Attributes\Strictify\Tests\Integration\NoSuchClass')]
            public $value;
        };

        expect(fn () => validate(['value' => 1], $model))->toThrow(
            InvalidBaseModelException::class,
            'must be a valid class, interface or enum',
        );
    });

    it('wraps a nested attribute that fails its own constructor', function () {
        $model = new class extends BaseModel {
            #[Sequence(of: new Between('a', 'b'))]
            public array $items;
        };

        expect(fn () => validate(['items' => [1]], $model))->toThrow(
            InvalidBaseModelException::class,
            'must be of type int|float',
        );
    });

    it('wraps an argument rule of a constraint', function () {
        $model = new class extends BaseModel {
            #[In]
            public string $value;
        };

        expect(fn () => validate(['value' => 'a'], $model))->toThrow(
            InvalidBaseModelException::class,
            'at least one value must be declared',
        );
    });

    it('keeps the exception inside the extension hierarchy', function () {
        $model = new class extends BaseModel {
            #[Email(domain: [])]
            public string $email;
        };

        try {
            validate(['email' => 'a@b.com'], $model);
            expect(false)->toBeTrue('InvalidBaseModelException expected');
        } catch (StrictifyThrowable $e) {
            expect($e)->toBeInstanceOf(InvalidBaseModelException::class);
        }
    });
});

describe('constructing an attribute outside a model', function () {
    it('keeps the engine TypeError for a wrong argument type', function () {
        expect(fn () => new Size('x'))->toThrow(TypeError::class);
    });

    it('keeps the engine TypeError for an unsupported arm', function () {
        expect(fn () => new Union(1.5))->toThrow(TypeError::class);
    });

    it('keeps the engine ValueError for an unresolvable class arm', function () {
        expect(fn () => new Union('Attributes\Strictify\Tests\Integration\NoSuchClass'))->toThrow(ValueError::class);
    });

    it('does not run the constraint argument rules until the field is compiled', function () {
        $constraint = new In;

        expect($constraint->values)->toBeEmpty();
    });
});
