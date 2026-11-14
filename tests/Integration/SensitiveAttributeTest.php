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

it('keeps the real value on the property but redacts it in toDict()', function () {
    $model = new class extends BaseModel {
        public string $name;

        #[Sensitive]
        public string $password;
    };

    validate(['name' => 'andre', 'password' => 'hunter2'], $model);

    expect($model->name)->toBe('andre');
    expect($model->password)->toBe('hunter2');
    expect($model->toDict())->toBe(['name' => 'andre', 'password' => '***']);
});

it('redacts sensitive fields keyed by their alias in toDict()', function () {
    $model = new class extends BaseModel {
        #[Alias('api_key')]
        #[Sensitive]
        public string $apiKey;
    };

    validate(['api_key' => 'secret'], $model);

    expect($model->apiKey)->toBe('secret');
    expect($model->toDict())->toBe(['api_key' => '***']);
});

it('redacts sensitive values inside nested models', function () {
    $model = new SensitiveOuterModel();

    validate(['inner' => ['token' => 'tok123', 'label' => 'x'], 'password' => 'pw'], $model);

    expect($model->inner->token)->toBe('tok123');
    expect($model->toDict())->toBe([
        'inner' => ['token' => '***', 'label' => 'x'],
        'password' => '***',
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
