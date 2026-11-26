<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Fields;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\ContainsAny;
use Attributes\Strictify\Fields\Constraints\Digit;
use Attributes\Strictify\Fields\Constraints\Iban;
use Attributes\Strictify\Fields\ErrorMessage;
use ReflectionClass;

use function Attributes\Strictify\validate;

/*
 * The ErrorMessage stub drives docs/api.md: every catalog key the attribute
 * accepts at runtime must be documented there, with the same default
 * template, including the constraints added later (Email, Url, Iban, ...).
 */
it('documents every ErrorMessage key with the runtime default', function () {
    $stubPath = dirname(__DIR__, 3).'/stubs/fields/error_message.stub.php';
    $stub = file_get_contents($stubPath);
    expect($stub)->toBeString();

    preg_match_all("/public string \\\$([A-Za-z_0-9]+) = '([^']*)';/", $stub, $matches, PREG_SET_ORDER);

    $documented = [];
    foreach ($matches as [, $key, $default]) {
        $documented[$key] = $default;
    }

    $reflection = new ReflectionClass(ErrorMessage::class);
    $runtime = [];
    foreach ($reflection->getProperties(\ReflectionProperty::IS_PUBLIC) as $property) {
        $runtime[$property->getName()] = $property->getDefaultValue();
    }

    expect($documented)
        ->not
        ->toBeEmpty()
        ->and(array_keys($documented))
        ->toEqualCanonicalizing(array_keys($runtime))
        ->and($documented)
        ->toBe($runtime);
});

it('overrides the message of the constraints added with the parity batch', function (string $key, string $expected) {
    $model = match ($key) {
        'digit' => new class extends BaseModel {
            #[Digit]
            #[ErrorMessage(digit: 'Only digits, please')]
            public string $value;
        },
        'iban' => new class extends BaseModel {
            #[Iban]
            #[ErrorMessage(iban: 'Bad IBAN')]
            public string $value;
        },
        'contains_any' => new class extends BaseModel {
            #[ContainsAny('x')]
            #[ErrorMessage(containsAny: 'Nothing matched')]
            public array $value;
        },
    };

    try {
        validate(['value' => $key === 'contains_any' ? ['a'] : 'zz'], $model);
        expect(false)->toBeTrue('ValidationException expected');
    } catch (ValidationException $e) {
        expect($e->getErrors())->toBe(['value' => [$expected]]);
    }
})->with([
    ['digit',        'Only digits, please'],
    ['iban',         'Bad IBAN'],
    ['contains_any', 'Nothing matched'],
]);
