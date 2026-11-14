<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\Fields\ErrorMessage;
use Attributes\Strictify\Fields\Field;
use Attributes\Strictify\Fields\Sensitive;
use ReflectionClass;

use function Attributes\Strictify\validate;

it('Sensitive is a valid PHP attribute for class properties', function () {
    $reflection = new ReflectionClass(TestClassWithSensitiveProperty::class);
    $attributes = $reflection->getProperty('password')->getAttributes(Sensitive::class);

    expect($attributes)->toHaveCount(1);
    $instance = $attributes[0]->newInstance();
    expect($instance)->toBeInstanceOf(Sensitive::class);
    expect($instance)->toBeInstanceOf(Field::class);
});

it('keeps the real value on the property and omits it from toDict() by default', function () {
    $secret = str_repeat('x', 8);
    $model = new class extends BaseModel {
        public string $name;

        #[Sensitive]
        public string $password;
    };

    validate(['name' => 'andre', 'password' => $secret], $model);

    expect($model->password)->toBe($secret);
    expect($model->toDict())->toBe(['name' => 'andre']);
});

it('includes sensitive values in toDict() when includeSensitive is true', function () {
    $secret = str_repeat('x', 8);
    $model = new class extends BaseModel {
        public string $name;

        #[Sensitive]
        public string $password;
    };

    validate(['name' => 'andre', 'password' => $secret], $model);

    expect($model->toDict(includeSensitive: true))->toBe(['name' => 'andre', 'password' => $secret]);
    expect($model->toDict(true))->toBe(['name' => 'andre', 'password' => $secret]);
});

it('omits aliased sensitive fields from toDict() unless includeSensitive is set', function () {
    $secret = str_repeat('k', 16);
    $model = new class extends BaseModel {
        #[Alias('public_id')]
        #[Sensitive]
        public string $apiKey;
    };

    validate(['public_id' => $secret], $model);

    expect($model->toDict())->toBe([]);
    expect($model->toDict(includeSensitive: true))->toBe(['public_id' => $secret]);
});

it('omits sensitive values inside nested models unless includeSensitive is set', function () {
    $token = str_repeat('t', 8);
    $secret = str_repeat('x', 8);
    $model = new SensitiveOuterModel();

    validate(['inner' => ['token' => $token, 'label' => 'x'], 'password' => $secret], $model);

    expect($model->inner->token)->toBe($token);
    expect($model->toDict())->toBe(['inner' => ['label' => 'x']]);
    expect($model->toDict(includeSensitive: true))->toBe([
        'inner' => ['token' => $token, 'label' => 'x'],
        'password' => $secret,
    ]);
});

it('redacts the {value} placeholder in error messages of sensitive properties', function () {
    $model = new class extends BaseModel {
        #[Sensitive]
        #[ErrorMessage(type: 'Invalid value: {value}')]
        public int $pin;
    };

    try {
        validate(['pin' => 'not-a-number'], $model);
        expect(false)->toBeTrue('ValidationException expected');
    } catch (ValidationException $e) {
        expect($e->getErrors())->toBe(['pin' => ['Invalid value: ***']]);
    }
});

it('does not redact values of non-sensitive properties in error messages', function () {
    $model = new class extends BaseModel {
        #[ErrorMessage(type: 'Invalid value: {value}')]
        public int $age;
    };

    try {
        validate(['age' => 'abc'], $model);
        expect(false)->toBeTrue('ValidationException expected');
    } catch (ValidationException $e) {
        expect($e->getErrors())->toBe(['age' => ["Invalid value: 'abc'"]]);
    }
});

class TestClassWithSensitiveProperty
{
    #[Sensitive]
    public string $password;
}

class SensitiveInnerModel extends BaseModel
{
    #[Sensitive]
    public string $token;

    public string $label;
}

class SensitiveOuterModel extends BaseModel
{
    public SensitiveInnerModel $inner;

    #[Sensitive]
    public string $password;
}
