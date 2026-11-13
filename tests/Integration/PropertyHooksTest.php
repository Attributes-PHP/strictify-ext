<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;

use function Attributes\Strictify\validate;

// Property hooks only exist on PHP 8.4+
if (PHP_VERSION_ID >= 80_400) {
    it('runs the set hook when hydrating a hooked property', function () {
        $model = new class extends BaseModel {
            public string $name {
                set {
                    $this->name = strtoupper($value);
                }
                get => $this->name;
            }
        };

        validate(['name' => 'andre'], $model);

        expect($model->name)->toBe('ANDRE');
    });

    it('runs the get hook on toDict for a hooked property', function () {
        $model = new class extends BaseModel {
            public int $age {
                get => $this->age * 2;
                set => $value;
            }
        };

        validate(['age' => 30], $model);

        expect($model->toDict())->toBe(['age' => 60]);
    });

    it('supports virtual get-only hooked properties without a backing store', function () {
        $model = new class extends BaseModel {
            public string $base = 'hello';

            public string $shout {
                get => strtoupper($this->base);
            }
        };

        validate(['shout' => 'ignored', 'base' => 'hello'], $model);

        expect($model->shout)->toBe('HELLO');
        expect($model->toDict())->toBe(['base' => 'hello', 'shout' => 'HELLO']);
    });

    it('still rejects invalid data for hooked properties', function () {
        $model = new class extends BaseModel {
            public int $age {
                get => $this->age;
                set => $value;
            }
        };

        validate(['age' => 'not-a-number'], $model);
    })->throws(ValidationException::class);

    it('validates models defining __get and __set', function () {
        $model = new class extends BaseModel {
            public string $name;

            public function __get(string $name): mixed
            {
                return $this->$name;
            }

            public function __set(string $name, mixed $value): void
            {
                $this->$name = $value;
            }
        };

        validate(['name' => 'andre'], $model);

        expect($model->name)->toBe('andre');
        expect($model->toDict())->toBe(['name' => 'andre']);
    });
}
