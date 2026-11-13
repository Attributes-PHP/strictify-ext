<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Validate;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Regex;
use Attributes\Strictify\ModelConfigs;
use DateTime;
use ValueError;

use function Attributes\Strictify\validate;

class OptionsInnerModel extends BaseModel
{
    public string $street;
}

afterEach(function () {
    ModelConfigs::setDefaultErrorMessages(fn (string $key): ?string => null);
});

describe('ModelConfigs extra option', function () {
    it('ignores unknown keys by default', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        $result = validate(['name' => 'John', 'junk' => 1], $model);
        expect($result->name)->toBe('John');
    });

    it('accepts unknown keys with extra=allow without storing them', function () {
        $model = new
            #[ModelConfigs(extra: 'allow')]
            class extends BaseModel {
                public string $name;
            };

        $result = validate(['name' => 'John', 'junk' => 1], $model);
        expect($result->name)->toBe('John')
            ->and(isset($result->junk))
            ->toBeFalse();
    });

    it('reports every unknown key with extra=forbid', function () {
        $model = new
            #[ModelConfigs(extra: 'forbid')]
            class extends BaseModel {
                public string $name;
            };

        try {
            validate(['name' => 'John', 'junk' => 1, 'extra2' => 2], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'junk' => ['Extra inputs are not permitted'],
                'extra2' => ['Extra inputs are not permitted'],
            ]);
        }
    });

    it('reports integer keys with extra=forbid', function () {
        $model = new
            #[ModelConfigs(extra: 'forbid')]
            class extends BaseModel {
                public string $name;
            };

        try {
            validate(['name' => 'John', 5 => 'x'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect($errors)->toHaveCount(1)
                ->and((string) array_key_first($errors))
                ->toBe('5')
                ->and(array_values($errors)[0])
                ->toBe(['Extra inputs are not permitted']);
        }
    });

    it('treats generated alias keys as known with extra=forbid', function () {
        $model = new
            #[ModelConfigs(extra: 'forbid', aliasGenerator: 'snake')]
            class extends BaseModel {
                public string $firstName;
            };

        $result = validate(['first_name' => 'John'], $model);
        expect($result->firstName)->toBe('John');

        try {
            validate(['first_name' => 'John', 'lastName' => 'Doe'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['lastName' => ['Extra inputs are not permitted']]);
        }
    });

    it('reports nested unknown keys under dot paths with extra=forbid', function () {
        $model = new
            #[ModelConfigs(extra: 'forbid')]
            class extends BaseModel {
                public string $name;

                public OptionsInnerModel $addr;
            };

        try {
            validate(['name' => 'J', 'addr' => ['street' => 's', 'zip' => '1'], 'top' => 1], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'addr.zip' => ['Extra inputs are not permitted'],
                'top' => ['Extra inputs are not permitted'],
            ]);
        }
    });

    it('reports only the first unknown key with extra=forbid and stopAtFirstError', function () {
        $model = new
            #[ModelConfigs(extra: 'forbid', stopAtFirstError: true)]
            class extends BaseModel {
                public string $name;
            };

        try {
            validate(['name' => 'J', 'a' => 1, 'b' => 2], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['a' => ['Extra inputs are not permitted']]);
        }
    });

    it('treats #[Alias] names as known and the property name as extra with extra=forbid', function () {
        $model = new
            #[ModelConfigs(extra: 'forbid')]
            class extends BaseModel {
                #[\Attributes\Strictify\Fields\Alias('user_name')]
                public string $name;
            };

        $result = validate(['user_name' => 'John'], $model);
        expect($result->name)->toBe('John');

        try {
            validate(['user_name' => 'John', 'name' => 'Doe'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['name' => ['Extra inputs are not permitted']]);
        }
    });

    it('does not report missing keys of default-valued properties with extra=forbid', function () {
        $model = new
            #[ModelConfigs(extra: 'forbid')]
            class extends BaseModel {
                public string $name = 'anon';
            };

        $result = validate([], $model);
        expect($result->name)->toBe('anon');

        try {
            validate(['junk' => 1], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['junk' => ['Extra inputs are not permitted']]);
        }
    });

    it('throws ValueError for an invalid extra value', function () {
        $model = new
            #[ModelConfigs(extra: 'invalid')]
            class extends BaseModel {
                public string $name;
            };

        validate(['name' => 'John'], $model);
    })->throws(ValueError::class, 'ignore, forbid, allow');

    it('translates the extra error with the "extra" key', function () {
        ModelConfigs::setDefaultErrorMessages(
            fn (string $key): ?string => $key === 'extra' ? 'Campo extra não permitido: {field}' : null,
        );

        $model = new
            #[ModelConfigs(extra: 'forbid')]
            class extends BaseModel {
                public string $name;
            };

        try {
            validate(['name' => 'John', 'junk' => 1], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['junk' => ['Campo extra não permitido: junk']]);
        }
    });
});

describe('ModelConfigs string normalization options', function () {
    it('lowercases string values with strToLower', function () {
        $model = new
            #[ModelConfigs(strToLower: true)]
            class extends BaseModel {
                public string $name;
            };

        expect(validate(['name' => 'JOHN Doe'], $model)->name)->toBe('john doe');
    });

    it('uppercases string values with strToUpper, winning over strToLower', function () {
        $model = new
            #[ModelConfigs(strToUpper: true)]
            class extends BaseModel {
                public string $name;
            };

        expect(validate(['name' => 'john'], $model)->name)->toBe('JOHN');

        $both = new
            #[ModelConfigs(strToLower: true, strToUpper: true)]
            class extends BaseModel {
                public string $name;
            };

        expect(validate(['name' => 'John'], $both)->name)->toBe('JOHN');
    });

    it('trims string values with stripWhitespace', function () {
        $model = new
            #[ModelConfigs(stripWhitespace: true)]
            class extends BaseModel {
                public string $name;
            };

        expect(validate(['name' => "  John\t\n"], $model)->name)->toBe('John');
    });

    it('combines stripWhitespace and strToLower', function () {
        $model = new
            #[ModelConfigs(stripWhitespace: true, strToLower: true)]
            class extends BaseModel {
                public string $name;

                public int $age;
            };

        $result = validate(['name' => '  JOHN DOE  ', 'age' => 30], $model);
        expect($result->name)->toBe('john doe')->and($result->age)->toBe(30);
    });

    it('normalizes before constraints run', function () {
        $model = new
            #[ModelConfigs(strToLower: true)]
            class extends BaseModel {
                #[Regex('/^[a-z ]+$/')]
                public string $slug;
            };

        expect(validate(['slug' => 'ABC DEF'], $model)->slug)->toBe('abc def');
    });

    it('does not touch non-string values', function () {
        $model = new
            #[ModelConfigs(strToLower: true, strToUpper: true, stripWhitespace: true)]
            class extends BaseModel {
                public int $age;

                public array $tags;
            };

        $result = validate(['age' => 30, 'tags' => ['A', 'B']], $model);
        expect($result->age)->toBe(30)->and($result->tags)->toBe(['A', 'B']);
    });

    it('does not transform strings bound for non-string hints like DateTime', function () {
        $model = new
            #[ModelConfigs(strToLower: true)]
            class extends BaseModel {
                public ?DateTime $birthday;
            };

        $result = validate(['birthday' => '1994-01-01T09:00:00+00:00'], $model);
        expect($result->birthday->format('c'))->toBe('1994-01-01T09:00:00+00:00');
    });

    it('transforms string arms of union types only', function () {
        $model = new
            #[ModelConfigs(strToLower: true)]
            class extends BaseModel {
                public string|int $value;
            };

        expect(validate(['value' => 'ABC'], $model)->value)->toBe('abc')
            ->and(validate(['value' => 42], $model)->value)
            ->toBe(42);
    });

    it('transforms strings of untyped properties', function () {
        $model = new
            #[ModelConfigs(strToUpper: true)]
            class extends BaseModel {
                public mixed $value;
            };

        expect(validate(['value' => 'abc'], $model)->value)->toBe('ABC');
    });

    it('applies in loose mode as well', function () {
        $model = new
            #[ModelConfigs(strict: false, stripWhitespace: true, strToLower: true)]
            class extends BaseModel {
                public string $name;
            };

        expect(validate(['name' => '  JOHN  '], $model)->name)->toBe('john');
    });
});
