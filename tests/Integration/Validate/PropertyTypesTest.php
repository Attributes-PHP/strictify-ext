<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Validate;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Tests\Models\Enums\EnumStrThreeValues;
use DateTime;
use DateTimeInterface;

use function Attributes\Strictify\validate;

describe('property types', function () {
    it('hydrates and round-trips an int property', function () {
        $model = new class extends BaseModel {
            public int $age;
        };

        validate(['age' => 30], $model);

        expect($model->age)->toBe(30);
        expect($model->toDict())->toBe(['age' => 30]);
        expect(validate($model->toDict(), new class extends BaseModel {
            public int $age;
        })->age)->toBe(30);
    });

    it('rejects a non-int value for an int property', function () {
        $model = new class extends BaseModel {
            public int $age;
        };

        try {
            validate(['age' => 'thirty'], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['age' => ['Must be integer']]);
        }
    });

    it('hydrates and round-trips a float property', function () {
        $model = new class extends BaseModel {
            public float $price;
        };

        validate(['price' => 19.99], $model);

        expect($model->price)->toBe(19.99);
        expect($model->toDict())->toBe(['price' => 19.99]);
    });

    it('rejects a non-numeric value for a float property', function () {
        $model = new class extends BaseModel {
            public float $price;
        };

        try {
            validate(['price' => 'free'], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['price' => ['Must be float']]);
        }
    });

    it('hydrates and round-trips a string property', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        validate(['name' => 'andre'], $model);

        expect($model->name)->toBe('andre');
        expect($model->toDict())->toBe(['name' => 'andre']);
    });

    it('rejects a non-string value for a string property in strict mode', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        try {
            validate(['name' => 123], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['Must be string']]);
        }
    });

    it('hydrates and round-trips a bool property', function () {
        $model = new class extends BaseModel {
            public bool $active;
        };

        validate(['active' => true], $model);

        expect($model->active)->toBeTrue();
        expect($model->toDict())->toBe(['active' => true]);
    });

    it('rejects a non-bool value for a bool property', function () {
        $model = new class extends BaseModel {
            public bool $active;
        };

        try {
            validate(['active' => 'yes'], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['active' => ['Must be boolean']]);
        }
    });

    it('hydrates and round-trips a plain array property', function () {
        $model = new class extends BaseModel {
            public array $tags;
        };

        validate(['tags' => ['a', 'b', 3]], $model);

        expect($model->tags)->toBe(['a', 'b', 3]);
        expect($model->toDict())->toBe(['tags' => ['a', 'b', 3]]);
    });

    it('rejects a non-array value for an array property', function () {
        $model = new class extends BaseModel {
            public array $tags;
        };

        try {
            validate(['tags' => 'a'], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['tags' => ['Must be array']]);
        }
    });

    it('hydrates a nested BaseModel property from an array and serializes it back', function () {
        $model = new PropertyTypesUser();

        validate(['name' => 'andre', 'address' => ['street' => 'Main St', 'city' => 'London']], $model);

        expect($model->address)->toBeInstanceOf(PropertyTypesAddress::class);
        expect($model->address->street)->toBe('Main St');
        expect($model->toDict())->toBe([
            'name' => 'andre',
            'address' => ['street' => 'Main St', 'city' => 'London'],
        ]);

        $roundTripped = validate($model->toDict(), new PropertyTypesUser());
        expect($roundTripped->address->city)->toBe('London');
    });

    it('reports nested model errors under the dot path', function () {
        $model = new PropertyTypesUser();

        try {
            validate(['name' => 'andre', 'address' => ['street' => 123, 'city' => 'London']], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['address.street' => ['Must be string']]);
        }
    });

    it('hydrates a DateTimeInterface property from a string and formats it in toDict()', function () {
        $model = new class extends BaseModel {
            public DateTimeInterface $when;
        };

        validate(['when' => '2024-01-02T03:04:05+00:00'], $model);

        expect($model->when)->toBeInstanceOf(DateTime::class);
        expect($model->toDict())->toBe(['when' => '2024-01-02T03:04:05+00:00']);

        $roundTripped = validate($model->toDict(), new class extends BaseModel {
            public DateTimeInterface $when;
        });
        expect($roundTripped->when->format('Y-m-d'))->toBe('2024-01-02');
    });

    it('hydrates a backed enum property from the enum case', function () {
        $model = new class extends BaseModel {
            public EnumStrThreeValues $role;
        };

        validate(['role' => EnumStrThreeValues::Two], $model);

        expect($model->role)->toBe(EnumStrThreeValues::Two);
        expect($model->toDict()['role'])->toBe(EnumStrThreeValues::Two);
    });

    it('rejects an unknown enum value', function () {
        $model = new class extends BaseModel {
            public EnumStrThreeValues $role;
        };

        try {
            validate(['role' => 'four'], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['role' => ["Must be 'one', 'two' or 'three'"]]);
        }
    });

    it('accepts null and a value for a nullable property', function () {
        $model = new class extends BaseModel {
            public ?string $nickname;
        };

        validate(['nickname' => null], $model);
        expect($model->nickname)->toBeNull();
        expect($model->toDict())->toBe(['nickname' => null]);

        validate(['nickname' => 'andy'], $model);
        expect($model->nickname)->toBe('andy');
        expect($model->toDict())->toBe(['nickname' => 'andy']);
    });

    it('accepts every arm of a union property', function () {
        $model = new class extends BaseModel {
            public int|string $id;
        };

        validate(['id' => 42], $model);
        expect($model->id)->toBe(42);
        expect($model->toDict())->toBe(['id' => 42]);

        validate(['id' => 'abc'], $model);
        expect($model->id)->toBe('abc');
        expect($model->toDict())->toBe(['id' => 'abc']);
    });

    it('rejects a value outside the union', function () {
        $model = new class extends BaseModel {
            public int|string $id;
        };

        try {
            validate(['id' => true], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['id' => ['Must be integer or string']]);
        }
    });

    it('accepts any value for an untyped property', function () {
        $model = new class extends BaseModel {
            public $anything;
        };

        validate(['anything' => ['nested' => true]], $model);

        expect($model->anything)->toBe(['nested' => true]);
        expect($model->toDict())->toBe(['anything' => ['nested' => true]]);
    });
});

class PropertyTypesAddress extends BaseModel
{
    public string $street;

    public string $city;
}

class PropertyTypesUser extends BaseModel
{
    public string $name;

    public PropertyTypesAddress $address;
}
