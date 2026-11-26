<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\InvalidBaseModelException;
use Attributes\Strictify\Fields\Alias;
use Attributes\Strictify\Fields\Field;
use Attributes\Strictify\ModelConfigs;
use ReflectionClass;
use ReflectionFunction;

use function Attributes\Strictify\validate;

it('Alias is a valid PHP Attribute for class properties', function () {
    $reflection = new ReflectionClass(TestClassWithAliasProperty::class);
    $property = $reflection->getProperty('name');
    $allAttributes = $property->getAttributes();
    $instance = $allAttributes[0]->newInstance();

    expect($instance)->toBeInstanceOf(Alias::class);
    expect($instance)->toBeInstanceOf(Field::class);
    expect($instance->name)->toBe('user_name');
});

it('Alias is a valid PHP Attribute for function parameters', function () {
    $reflection = new ReflectionFunction(test_function_with_alias(...));
    $allParameters = $reflection->getParameters();
    $allAttributes = $allParameters[0]->getAttributes();
    $instance = $allAttributes[0]->newInstance();

    expect($instance)->toBeInstanceOf(Alias::class);
    expect($instance)->toBeInstanceOf(Field::class);
    expect($instance->name)->toBe('email_address');
});

class TestClassWithAliasProperty
{
    #[Alias('user_name')]
    public string $name;
}

function test_function_with_alias(#[Alias('email_address')] string $email)
{
    return $email;
}

describe('Alias resolution rules', function () {
    it('rejects an empty alias name', function () {
        $model = new class extends BaseModel {
            #[Alias('')]
            public string $value;
        };

        expect(fn () => validate(['' => 'x'], $model))->toThrow(
            InvalidBaseModelException::class,
            'The #[Alias] name of',
        );
    });

    it('rejects two properties that resolve to the same input key', function () {
        // snake_case maps both properties to "a_b"
        $snake = new
            #[ModelConfigs(aliasGenerator: 'snake')]
            class extends BaseModel {
                public string $aB;
                public string $a_b;
            };

        expect(fn () => validate(['a_b' => 'x'], $snake))->toThrow(
            InvalidBaseModelException::class,
            'both resolve to the input key "a_b"',
        );

        $alias = new class extends BaseModel {
            #[Alias('other')]
            public string $first;
            public string $other;
        };

        expect(fn () => validate(['other' => 'x'], $alias))->toThrow(
            InvalidBaseModelException::class,
            'both resolve to the input key "other"',
        );

        $duplicate = new class extends BaseModel {
            #[Alias('k')]
            public string $first;
            #[Alias('k')]
            public string $second;
        };

        expect(fn () => validate(['k' => 'x'], $duplicate))->toThrow(
            InvalidBaseModelException::class,
            'both resolve to the input key "k"',
        );
    });

    it('detects the collision even when another field fails first', function () {
        $model = new
            #[ModelConfigs(aliasGenerator: 'snake', stopAtFirstError: true)]
            class extends BaseModel {
                public int $n;
                public string $aB;
                public string $a_b;
            };

        expect(fn () => validate(['n' => 'bad', 'a_b' => 'x'], $model))->toThrow(InvalidBaseModelException::class);
    });

    it('hydrates distinct generated keys as before', function () {
        $model = new
            #[ModelConfigs(aliasGenerator: 'snake', extra: 'forbid')]
            class extends BaseModel {
                public string $aB;
                public string $aC;
            };

        $result = validate(['a_b' => 'x', 'a_c' => 'y'], $model);

        expect($result->aB)->toBe('x')
            ->and($result->aC)
            ->toBe('y');
    });
});
