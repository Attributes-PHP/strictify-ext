<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\ModelConfigs;
use DateTime;

use function Attributes\Strictify\validate;

it('BaseModel has all required methods', function () {
    $model = new class extends BaseModel {};

    expect(method_exists($model, 'beforeValidation'))->toBeTrue();
    expect(method_exists($model, 'afterValidation'))->toBeTrue();
    expect(method_exists($model, 'toDict'))->toBeTrue();
});

it('toDict returns public properties as array', function () {
    $model = new class extends BaseModel {
        public string $name;
        public int $age;
    };

    validate(['name' => 'John', 'age' => 30], $model);
    expect($model->toDict())->toBe(['name' => 'John', 'age' => 30]);
});

it('toDict uses resolved field names with Alias attribute', function () {
    $model = new class extends BaseModel {
        #[Alias('user_name')]
        public string $name;
    };

    validate(['user_name' => 'John'], $model);
    expect($model->toDict())->toBe(['user_name' => 'John']);
});

it('toDict uses resolved field names with aliasGenerator', function () {
    $model = new
        #[ModelConfigs(aliasGenerator: 'snake')]
        class extends BaseModel {
            public string $firstName;
            public string $lastName;
        };

    validate(['first_name' => 'John', 'last_name' => 'Doe'], $model);
    expect($model->toDict())->toBe(['first_name' => 'John', 'last_name' => 'Doe']);
});

it('toDict skips uninitialized properties', function () {
    $model = new class extends BaseModel {
        public string $name = 'default';
        public ?int $age = null;
    };

    validate(['name' => 'John', 'age' => null], $model);
    expect($model->toDict())->toBe(['name' => 'John', 'age' => null]);
});

it('toDict round-trips through validate', function () {
    $model = new class extends BaseModel {
        public string $name;
        public int $age;
    };

    validate(['name' => 'John', 'age' => 30], $model);
    $dict = $model->toDict();

    $model2 = new class extends BaseModel {
        public string $name;
        public int $age;
    };

    validate($dict, $model2);
    expect($model2->name)->toBe('John');
    expect($model2->age)->toBe(30);
});

it('toDict converts DateTime to ISO 8601 string', function () {
    $model = new class extends BaseModel {
        public string $name;
        public ?DateTime $birthday;
    };

    validate(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00'], $model);
    expect($model->toDict())->toBe(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00']);
});

it('toDict DateTime output round-trips through validate', function () {
    $model = new class extends BaseModel {
        public ?DateTime $birthday;
    };

    validate(['birthday' => '2000-06-15T12:30:00+02:00'], $model);
    $dict = $model->toDict();

    $model2 = new class extends BaseModel {
        public ?DateTime $birthday;
    };

    validate($dict, $model2);
    expect($model2->birthday)->toBeInstanceOf(DateTime::class);
    expect($model2->birthday->format('Y-m-d\\TH:i:sP'))->toBe('2000-06-15T12:30:00+02:00');
});
