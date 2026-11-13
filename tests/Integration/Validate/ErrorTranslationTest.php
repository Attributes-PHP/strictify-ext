<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Validate;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraint;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\ErrorMessage;
use Attributes\Strictify\ModelConfigs;
use RuntimeException;
use TypeError;
use ValueError;

use function Attributes\Strictify\validate;

final class TranslationMsg
{
    public const NAME_REQUIRED = 'user.name.required';
    public const NAME_TYPE = 'user.name.type';
}

#[\Attribute(\Attribute::TARGET_PROPERTY)]
final class RejectWithOwnMessage implements Constraint
{
    public function validate(mixed $value): bool
    {
        return $value === 'ok';
    }

    public function message(): string
    {
        return 'custom.constrain.rejected';
    }
}

afterEach(function () {
    // The translator is a static property: reset it so other tests are unaffected.
    ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => null);
});

describe('error message translation', function () {
    it('uses built-in default messages when the translator returns null', function () {
        ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => null);

        $model = new class extends BaseModel {
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['Field is required']]);
        }
    });

    it('translates built-in error keys and still substitutes placeholders', function () {
        ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => match ($key) {
            'required' => '{field} é obrigatório',
            'type' => 'Deve ser {expected}',
            'min' => 'Deve ser no mínimo {expected}',
            default => null,
        });

        $model = new class extends BaseModel {
            #[Min(18)]
            public int $age;

            public string $name;
        };

        try {
            validate(['age' => 5], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'age' => ['Deve ser no mínimo 18'],
                'name' => ['name é obrigatório'],
            ]);
        }

        try {
            validate(['age' => 30, 'name' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['Deve ser string']]);
        }
    });

    it('treats the ErrorMessage attribute value as a translation key', function () {
        ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => match ($key) {
            TranslationMsg::NAME_REQUIRED => 'O nome é obrigatório',
            TranslationMsg::NAME_TYPE => 'O nome deve ser {expected}',
            default => null,
        });

        $model = new class extends BaseModel {
            #[ErrorMessage(required: TranslationMsg::NAME_REQUIRED, type: TranslationMsg::NAME_TYPE)]
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['O nome é obrigatório']]);
        }

        try {
            validate(['name' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['O nome deve ser string']]);
        }
    });

    it('keeps the literal ErrorMessage value when the translator does not know the key', function () {
        ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => null);

        $model = new class extends BaseModel {
            #[ErrorMessage(required: 'Name is mandatory', type: 'Expected {expected}')]
            public string $name;
        };

        try {
            validate(['name' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['Expected string']]);
        }
    });

    it('applies a newly registered translator immediately (cache is cleared)', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => 'primeira tradução');
        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['primeira tradução']]);
        }

        ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => 'segunda tradução');
        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['segunda tradução']]);
        }
    });

    it('propagates exceptions thrown by the translator', function () {
        ModelConfigs::setDefaultErrorMessages(function (string $key): ?string {
            throw new RuntimeException("boom {$key}");
        });

        $model = new class extends BaseModel {
            public string $name;
        };

        validate([], $model);
    })->throws(RuntimeException::class, 'boom required');

    it('throws ValueError when the translator returns a non-string', function () {
        ModelConfigs::setDefaultErrorMessages(fn (string $key): mixed => 42);

        $model = new class extends BaseModel {
            public string $name;
        };

        validate([], $model);
    })->throws(ValueError::class, 'must return ?string');

    it('throws TypeError when registering a non-callable translator', function () {
        ModelConfigs::setDefaultErrorMessages('not a callable');
    })->throws(TypeError::class, 'must be callable');

    it('does not translate userland constraint messages', function () {
        $seen = [];
        ModelConfigs::setDefaultErrorMessages(function (string $key) use (&$seen): ?string {
            $seen[] = $key;

            return null;
        });

        $model = new class extends BaseModel {
            #[RejectWithOwnMessage]
            public string $name;
        };

        try {
            validate(['name' => 'nope'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['custom.constrain.rejected']]);
        }

        expect($seen)->not->toContain('custom.constrain.rejected');
    });
});
