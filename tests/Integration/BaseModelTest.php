<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\Fields\Constraints\DateTime;
use Attributes\Strictify\ModelConfigs;
use DateTimeZone;

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

it('toDict uses resolved field names with both Alias and aliasGenerator', function () {
    $model = new
        #[ModelConfigs(aliasGenerator: 'snake')]
        class extends BaseModel {
            public string $firstName;
            #[Alias('last')]
            public string $lastName;
        };

    validate(['first_name' => 'John', 'last' => 'Doe'], $model);
    expect($model->toDict())->toBe(['first_name' => 'John', 'last' => 'Doe']);
});

it('toDict skips uninitialized properties', function () {
    $model = new class extends BaseModel {
        public string $name = 'default';
        public ?int $age = null;
        public string $hello;
    };

    expect($model->toDict())->toBe(['name' => 'default', 'age' => null]);
});

it('toDict skips private/protected properties', function () {
    $model = new class extends BaseModel {
        public bool $public = true;
        private bool $private = false;
        protected bool $protected = false;
    };

    expect($model->toDict())->toBe(['public' => true]);
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

describe('toDict converts DateTime to ISO 8601 string', function () {
    it('default', function () {
        $model = new class extends BaseModel {
            public string $name;
            #[DateTime]
            public ?\DateTime $birthday;
        };

        validate(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00'], $model);
        expect($model->toDict())->toBe(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00']);
    });

    it('Europe/London', function () {
        $model = new class extends BaseModel {
            public string $name;
            #[DateTime(timezone: 'Europe/London')]
            public ?\DateTime $birthday;
        };

        validate(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00'], $model);
        expect($model->toDict())->toBe(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00']);
    });

    it('Europe/Madrid', function () {
        $model = new class extends BaseModel {
            public string $name;
            #[DateTime(timezone: 'Europe/Madrid')]
            public ?\DateTime $birthday;
        };

        validate(['name' => 'John', 'birthday' => '1994-01-01T09:00:00+00:00'], $model);
        // Europe/Madrid is UTC+1 in January
        expect($model->toDict())->toBe(['name' => 'John', 'birthday' => '1994-01-01T10:00:00+01:00']);
    });
});

it('toDict DateTime output round-trips through validate', function () {
    $model = new class extends BaseModel {
        public ?\DateTime $birthday;
    };

    validate(['birthday' => '2000-06-15T12:30:00+02:00'], $model);
    $dict = $model->toDict();

    $model2 = new class extends BaseModel {
        public ?\DateTime $birthday;
    };

    validate($dict, $model2);
    expect($model2->birthday)->toBeInstanceOf(\DateTime::class);
    expect($model2->birthday->format('Y-m-d\\TH:i:sP'))->toBe('2000-06-15T12:30:00+02:00');
});

it('toDict supports nested models', function () {
    class User extends BaseModel
    {
        #[Alias('fullName')]
        public string $name;
        #[DateTime(timezone: 'Europe/Madrid', format: 'Y-m-d')]
        public ?string $dateAdded;
    }

    $model = new
        #[ModelConfigs(aliasGenerator: 'snake')]
        class extends BaseModel {
            public User $user;
            #[DateTime(timezone: 'Europe/London')]
            public ?\DateTime $timestamp;
        };

    $expectedResult = [
        'user' => [
            'fullName' => 'Andre',
            'date_added' => '2025-10-10',
        ],
        'timestamp' => '1994-01-01T09:00:00+00:00',
    ];
    validate($expectedResult, $model);
    expect($model->toDict())->toBe([
        'user' => [
            'fullName' => 'Andre',
            'date_added' => '2025-10-10',
        ],
        'timestamp' => '1994-01-01T09:00:00+00:00',
    ]);
});

it('nested models use their own aliasGenerator', function () {
    #[ModelConfigs(aliasGenerator: 'camel')]
    class CamelAddress extends BaseModel
    {
        public string $postCode;
    }

    $model = new
        #[ModelConfigs(aliasGenerator: 'snake')]
        class extends BaseModel {
            public CamelAddress $address;
        };

    // Root uses snake, nested CamelAddress uses camel (postCode stays postCode)
    $input = ['address' => ['postCode' => 'M00']];
    validate($input, $model);
    expect($model->address->postCode)->toBe('M00');
    expect($model->toDict())->toBe(['address' => ['postCode' => 'M00']]);
});

it('toDict uses DateTime constraint format and timezone', function () {
    $model = new class extends BaseModel {
        #[DateTime(format: 'Y-m-d', timezone: 'Europe/Madrid')]
        public ?\DateTime $birthday;
    };

    validate(['birthday' => '1994-01-01T09:00:00+00:00'], $model);
    // Europe/Madrid is UTC+1 in January, format Y-m-d
    expect($model->toDict())->toBe(['birthday' => '1994-01-01']);
});

it('toDict uses DateTime constraint timezone with default format', function () {
    $model = new class extends BaseModel {
        #[DateTime(timezone: 'Europe/Madrid')]
        public ?\DateTime $ts;
    };

    // 1994-01-01 09:00 UTC = 1994-01-01 10:00 Europe/Madrid (UTC+1)
    validate(['ts' => '1994-01-01T09:00:00+00:00'], $model);
    expect($model->toDict())->toBe(['ts' => '1994-01-01T10:00:00+01:00']);
});

it('toDict does not mutate the original DateTime when applying constraint timezone', function () {
    $model = new class extends BaseModel {
        #[DateTime(timezone: 'Europe/Madrid')]
        public ?\DateTime $ts;
    };

    validate(['ts' => '1994-01-01T09:00:00+00:00'], $model);
    $model->toDict();
    // Original object timezone must still be +00:00
    expect($model->ts->getTimezone()->getName())->toBe('+00:00');
});
