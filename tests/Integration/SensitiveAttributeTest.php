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
use RuntimeException;

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

    expect($model->toDict())->toBeEmpty();
    expect($model->toDict(includeSensitive: true))->toBe(['apiKey' => $secret]);
    expect($model->toDict(includeSensitive: true, useAlias: true))->toBe(['public_id' => $secret]);
});

it('omits sensitive values inside nested models unless includeSensitive is set', function () {
    $token = str_repeat('t', 8);
    $secret = str_repeat('x', 8);
    $model = new SensitiveOuterModel;

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
        $model = new SensitiveScalarTypesModel;

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
        $model = new SensitiveObjectTypesModel;

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

describe('sensitive properties in debug output', function () {
    it('masks sensitive properties in print_r output but keeps the real value on the model', function () {
        $secret = str_repeat('x', 8);
        $model = new SensitiveDebugModel;

        validate(['name' => 'andre', 'password' => $secret], $model);

        $dump = print_r($model, true);

        expect($dump)->toContain('***');
        expect($dump)->not->toContain($secret);
        expect($dump)->toContain('andre');
        expect($model->password)->toBe($secret);
    });

    it('masks sensitive properties in var_dump output', function () {
        $secret = str_repeat('x', 8);
        $model = new SensitiveDebugModel;

        validate(['name' => 'andre', 'password' => $secret], $model);

        ob_start();
        // The dump is the output under test: the model redacts #[Sensitive] values in it.
        // @mago-ignore lint:no-debug-symbols
        var_dump($model);
        $dump = ob_get_clean();

        expect($dump)->toContain('string(3) "***"');
        expect($dump)->not->toContain($secret);
        expect($dump)->toContain('andre');
    });

    it('masks sensitive properties of nested models in dumps', function () {
        $token = str_repeat('t', 8);
        $secret = str_repeat('x', 8);
        $model = new SensitiveOuterModel;

        validate(['inner' => ['token' => $token, 'label' => 'visible'], 'password' => $secret], $model);

        $dump = print_r($model, true);

        expect($dump)->toContain('***');
        expect($dump)->not->toContain($token);
        expect($dump)->not->toContain($secret);
        expect($dump)->toContain('visible');
        expect($model->password)->toBe($secret);
        expect($model->inner->token)->toBe($token);
    });

    it('masks sensitive properties of every type in dumps', function () {
        $secret = str_repeat('x', 8);
        $model = new SensitiveScalarTypesModel;

        validate([
            'label' => 'visible',
            'pin' => 1234,
            'price' => 9.99,
            'note' => $secret,
            'active' => true,
            'tags' => ['a', 'b'],
        ], $model);

        $dump = print_r($model, true);

        expect($dump)->toContain('visible');
        expect($dump)->toContain('***');
        expect(substr_count($dump, '***'))->toBe(5);
        expect($dump)->not->toContain($secret);
        expect($dump)->not->toContain('1234');
        expect($dump)->not->toContain('9.99');
    });

    it('masks sensitive object-typed and BaseModel-typed properties in dumps', function () {
        $model = new SensitiveObjectTypesModel;

        validate([
            'label' => 'visible',
            'when' => '2024-01-02T03:04:05+00:00',
            'role' => EnumStrThreeValues::One,
            'nickname' => null,
            'id' => 42,
            'misc' => ['k' => 1],
            'address' => ['street' => 'Main St', 'city' => 'London'],
        ], $model);

        $dump = print_r($model, true);

        expect($dump)->toContain('visible');
        // All six sensitive properties are masked
        expect(substr_count($dump, '***'))->toBe(6);
        expect($dump)->not->toContain('2024-01-02');
        expect($dump)->not->toContain('Main St');
        expect($dump)->not->toContain('London');
        expect($dump)->not->toContain('42');

        // The model itself still holds the real, typed values
        expect($model->when)->toBeInstanceOf(DateTime::class);
        expect($model->role)->toBe(EnumStrThreeValues::One);
        expect($model->nickname)->toBeNull();
        expect($model->id)->toBe(42);
        expect($model->misc)->toBe(['k' => 1]);
        expect($model->address)->toBeInstanceOf(SensitiveAddress::class);
        expect($model->address->street)->toBe('Main St');
    });

    it('keeps real values for json_encode, array cast, get_object_vars and serialize', function () {
        $secret = str_repeat('x', 8);
        $model = new SensitiveDebugModel;

        validate(['name' => 'andre', 'password' => $secret], $model);

        expect(json_encode($model))->toBe('{"name":"andre","password":"'.$secret.'"}');
        expect((array) $model)->toBe(['name' => 'andre', 'password' => $secret]);
        expect(get_object_vars($model))->toBe(['name' => 'andre', 'password' => $secret]);
        expect(unserialize(serialize($model))->password)->toBe($secret);
    });

    it('masks sensitive properties dumped inside backtrace arguments', function () {
        $secret = str_repeat('x', 8);
        $model = new SensitiveDebugModel;

        validate(['name' => 'andre', 'password' => $secret], $model);

        SensitiveTraceRecorder::$backtrace = null;

        try {
            sensitive_trace_throws($model);
            expect(false)->toBeTrue('RuntimeException expected');
        } catch (RuntimeException $e) {
            $args = SensitiveTraceRecorder::$backtrace[0]['args'];
            $dump = print_r($args, true);

            expect($dump)->toContain('***');
            expect($dump)->not->toContain($secret);
            expect($args[0]->password)->toBe($secret);
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

class SensitiveDebugModel extends BaseModel
{
    public string $name;

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

class SensitiveTraceRecorder
{
    public static ?array $backtrace = null;
}

function sensitive_trace_throws(SensitiveDebugModel $model): void
{
    SensitiveTraceRecorder::$backtrace = debug_backtrace();

    throw new RuntimeException('boom');
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
