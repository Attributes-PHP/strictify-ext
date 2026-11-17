<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Validate;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ErrorTranslator;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraint;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\ErrorMessage;
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
    ErrorTranslator::set(fn (?string $key, string $message): ?string => null);
});

it('returns the registered translator via ErrorTranslator::get()', function () {
    $translator = fn (?string $key, string $message): ?string => $key === 'extra' ? 'translated' : null;
    ErrorTranslator::set($translator);

    expect(ErrorTranslator::get())->toBe($translator);
});

describe('error message translation', function () {
    it('passes the stable key and the default message template to the translator', function () {
        $seen = [];
        ErrorTranslator::set(function (?string $key, string $message) use (&$seen): ?string {
            $seen[] = [$key, $message];

            return null;
        });

        $model = new class extends BaseModel {
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['Field is required']]);
        }

        expect($seen)->toBe([['required', 'Field is required']]);
    });

    it('uses built-in default messages when the translator returns null', function () {
        ErrorTranslator::set(fn (?string $key, string $message): ?string => null);

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
        ErrorTranslator::set(fn (?string $key, string $message): ?string => match ($key) {
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

    it('passes a null key and the ErrorMessage value as the message for custom messages', function () {
        ErrorTranslator::set(function (?string $key, string $message): ?string {
            expect($key)->toBeNull();

            return match ($message) {
                TranslationMsg::NAME_REQUIRED => 'O nome é obrigatório',
                TranslationMsg::NAME_TYPE => 'O nome deve ser {expected}',
                default => null,
            };
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

    it('keeps the literal ErrorMessage value when the translator returns null', function () {
        ErrorTranslator::set(fn (?string $key, string $message): ?string => null);

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

        ErrorTranslator::set(fn (?string $key, string $message): ?string => 'primeira tradução');
        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['primeira tradução']]);
        }

        ErrorTranslator::set(fn (?string $key, string $message): ?string => 'segunda tradução');
        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['segunda tradução']]);
        }
    });

    it('propagates exceptions thrown by the translator', function () {
        ErrorTranslator::set(function (?string $key, string $message): ?string {
            throw new RuntimeException("boom {$key}");
        });

        $model = new class extends BaseModel {
            public string $name;
        };

        validate([], $model);
    })->throws(RuntimeException::class, 'boom required');

    it('throws ValueError when the translator returns a non-string', function () {
        ErrorTranslator::set(fn (?string $key, string $message): mixed => 42);

        $model = new class extends BaseModel {
            public string $name;
        };

        validate([], $model);
    })->throws(ValueError::class, 'must return ?string');

    it('throws TypeError when registering a non-callable translator', function () {
        ErrorTranslator::set('not a callable');
    })->throws(TypeError::class, 'must be callable');

    it('does not translate userland constraint messages', function () {
        $seen = [];
        ErrorTranslator::set(function (?string $key, string $message) use (&$seen): ?string {
            $seen[] = [$key, $message];

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

        expect($seen)->each->not->toContain('custom.constrain.rejected');
    });
});
