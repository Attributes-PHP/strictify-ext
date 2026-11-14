<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\ErrorMessage;
use Attributes\Strictify\Fields\Field;
use Attributes\Strictify\Fields\Sensitive;
use Attributes\Strictify\Tests\Models\Enums\EnumStrThreeValues;
use DateTime;
use DateTimeInterface;
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

describe('sensitive properties of every type', function () {
    it('omits sensitive scalar/array properties and includes them with includeSensitive', function () {
        $secret = str_repeat('x', 8);
        $model = new SensitiveScalarTypesModel();

        validate([
            'label' => 'visible',
            'pin' => 1234,
            'price' => 9.99,
            'note' => $secret,
            'active' => true,
            'tags' => ['a', 'b'],
        ], $model);

        // Only the non-sensitive property is serialized by default
        expect($model->toDict())->toBe(['label' => 'visible']);

        expect($model->toDict(includeSensitive: true))->toBe([
            'label' => 'visible',
            'pin' => 1234,
            'price' => 9.99,
            'note' => $secret,
            'active' => true,
            'tags' => ['a', 'b'],
        ]);

        // The model itself still holds the real, typed values
        expect($model->pin)->toBe(1234);
        expect($model->price)->toBe(9.99);
        expect($model->note)->toBe($secret);
        expect($model->active)->toBeTrue();
        expect($model->tags)->toBe(['a', 'b']);
    });

    it('omits sensitive object-typed properties and includes them with includeSensitive', function () {
        $model = new SensitiveObjectTypesModel();

        validate([
            'label' => 'visible',
            'when' => '2024-01-02T03:04:05+00:00',
            'role' => EnumStrThreeValues::One,
            'nickname' => null,
            'id' => 42,
            'misc' => ['k' => 1],
            'address' => ['street' => 'Main St', 'city' => 'London'],
        ], $model);

        expect($model->toDict())->toBe(['label' => 'visible']);

        expect($model->toDict(includeSensitive: true))->toBe([
            'label' => 'visible',
            'when' => '2024-01-02T03:04:05+00:00',
            'role' => EnumStrThreeValues::One,
            'nickname' => null,
            'id' => 42,
            'misc' => ['k' => 1],
            'address' => ['street' => 'Main St', 'city' => 'London'],
        ]);

        expect($model->when)->toBeInstanceOf(DateTime::class);
        expect($model->role)->toBe(EnumStrThreeValues::One);
        expect($model->nickname)->toBeNull();
        expect($model->id)->toBe(42);
        expect($model->misc)->toBe(['k' => 1]);
        expect($model->address)->toBeInstanceOf(SensitiveAddress::class);
    });

    it('omits a sensitive nested BaseModel property as a whole subtree', function () {
        $model = new class extends BaseModel {
            public string $name;

            #[Sensitive]
            public SensitiveAddress $address;
        };

        validate(['name' => 'andre', 'address' => ['street' => 'Main St', 'city' => 'London']], $model);

        expect($model->address->street)->toBe('Main St');
        expect($model->toDict())->toBe(['name' => 'andre']);
        expect($model->toDict(true))->toBe([
            'name' => 'andre',
            'address' => ['street' => 'Main St', 'city' => 'London'],
        ]);
    });

    it('redacts {value} for sensitive properties of any type, not just strings', function () {
        $model = new class extends BaseModel {
            #[Sensitive]
            #[ErrorMessage(type: 'Bad value: {value}')]
            public array $tags;
        };

        try {
            validate(['tags' => 'not-an-array'], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['tags' => ['Bad value: ***']]);
        }
    });

    it('still runs constraints on sensitive properties without leaking the value', function () {
        $model = new class extends BaseModel {
            #[Sensitive]
            #[Min(5)]
            public int $pin;
        };

        try {
            validate(['pin' => 3], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['pin' => ['Must be at least 5']]);
        }
    });
});

class TestClassWithSensitiveProperty
{
    #[Sensitive]
    public string $password;
}

class SensitiveAddress extends BaseModel
{
    public string $street;

    public string $city;
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

class SensitiveScalarTypesModel extends BaseModel
{
    public string $label;

    #[Sensitive]
    public int $pin;

    #[Sensitive]
    public float $price;

    #[Sensitive]
    public string $note;

    #[Sensitive]
    public bool $active;

    #[Sensitive]
    public array $tags;
}

class SensitiveObjectTypesModel extends BaseModel
{
    public string $label;

    #[Sensitive]
    public DateTimeInterface $when;

    #[Sensitive]
    public EnumStrThreeValues $role;

    #[Sensitive]
    public ?string $nickname;

    #[Sensitive]
    public int|string $id;

    #[Sensitive]
    public $misc;

    #[Sensitive]
    public SensitiveAddress $address;
}
