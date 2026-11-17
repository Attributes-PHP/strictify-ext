<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\Fields\Constraints\AlphaDash;
use Attributes\Strictify\Fields\Constraints\Ascii;
use Attributes\Strictify\Fields\Constraints\Between;
use Attributes\Strictify\Fields\Constraints\Contains;
use Attributes\Strictify\Fields\Constraints\DateTime;
use Attributes\Strictify\Fields\Constraints\EndsWith;
use Attributes\Strictify\Fields\Constraints\In;
use Attributes\Strictify\Fields\Constraints\Max;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\Constraints\MultipleOf;
use Attributes\Strictify\Fields\Constraints\Regex;
use Attributes\Strictify\Fields\Constraints\Size;
use Attributes\Strictify\Fields\Constraints\StartsWith;
use Attributes\Strictify\Fields\Constraints\Ulid;
use Attributes\Strictify\Fields\Constraints\Uuid;
use Attributes\Strictify\Fields\Field;

$allConstraintClasses = [
    AlphaDash::class,
    Ascii::class,
    Between::class,
    Contains::class,
    DateTime::class,
    EndsWith::class,
    In::class,
    Max::class,
    Min::class,
    MultipleOf::class,
    Regex::class,
    Size::class,
    StartsWith::class,
    Ulid::class,
    Uuid::class,
];

describe('built-in constraint classes', function () use ($allConstraintClasses) {
    it('are final: userland constraints implement the Constraint interface instead of extending built-ins', function (string $class) {
        expect((new \ReflectionClass($class))->isFinal())->toBeTrue()
            ->and(is_subclass_of($class, Field::class))
            ->toBeTrue();
    })->with($allConstraintClasses);

    it('are readonly: their properties are typed, readonly and initialized by the constructor', function (string $class) {
        $reflection = new \ReflectionClass($class);

        expect($reflection->isReadOnly())->toBe($class !== DateTime::class);

        foreach ($reflection->getProperties() as $property) {
            $isInternalProperty = ! $property->isPublic();
            if ($isInternalProperty)
                continue;

            expect($property->isReadOnly())->toBeTrue()
                ->and($property->hasType())
                ->toBeTrue()
                ->and($property->hasDefaultValue())
                ->toBeFalse();
        }
    })->with($allConstraintClasses);

    it('reject property writes after construction', function () {
        $constraint = new Between(1, 10);

        $constraint->min = 5;
    })->throws(\Error::class);
});
