<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration;

use ArgumentCountError;
use Attributes\Strictify\Exceptions\CallException;
use Attributes\Strictify\Exceptions\InvalidBaseModelException;
use Attributes\Strictify\Exceptions\MaxDepthException;
use Attributes\Strictify\Exceptions\StrictifyThrowable;
use Attributes\Strictify\Exceptions\ValidationException;
use Error;
use Exception;
use ReflectionClass;

/*
 * The hierarchy mirrors the engine's own split: a data failure extends
 * Exception (like JsonException), a programming error extends Error (like
 * TypeError and ValueError). StrictifyThrowable is the marker both sides
 * share, so one catch still covers every error the extension raises.
 */
it('exposes StrictifyThrowable as the shared marker', function () {
    $reflection = new ReflectionClass(StrictifyThrowable::class);

    expect($reflection->isInterface())->toBeTrue()
        ->and($reflection->implementsInterface(\Throwable::class))
        ->toBeTrue()
        ->and(class_exists('Attributes\Strictify\Exceptions\BaseException'))
        ->toBeFalse();
});

it('throws ValidationException from the Exception branch', function () {
    $exception = new ValidationException(['number' => 'Invalid number']);

    expect($exception)->toBeInstanceOf(Exception::class)
        ->and($exception)
        ->toBeInstanceOf(StrictifyThrowable::class)
        ->and($exception->getMessage())
        ->toBe('Invalid data')
        ->and($exception->getErrors())
        ->toBe(['number' => 'Invalid number']);
});

it('throws InvalidBaseModelException from the Error branch', function () {
    $exception = new InvalidBaseModelException('Testing');

    expect($exception)->toBeInstanceOf(Error::class)
        ->and($exception)
        ->not
        ->toBeInstanceOf(Exception::class)
        ->and($exception)
        ->toBeInstanceOf(StrictifyThrowable::class)
        ->and($exception->getMessage())
        ->toBe('Testing');
});

it('throws MaxDepthException from the Error branch', function () {
    $exception = new MaxDepthException('Testing');

    expect($exception)->toBeInstanceOf(Error::class)
        ->and($exception)
        ->toBeInstanceOf(StrictifyThrowable::class)
        ->and($exception->getMessage())
        ->toBe('Testing');
});

it('throws CallException from the ArgumentCountError branch', function () {
    $exception = new CallException('Testing');

    expect($exception)->toBeInstanceOf(ArgumentCountError::class)
        ->and($exception)
        ->toBeInstanceOf(StrictifyThrowable::class)
        ->and($exception->getMessage())
        ->toBe('Testing');
});

it('is missed by a plain Exception catch', function () {
    $caught = null;

    try {
        throw new InvalidBaseModelException('Testing');
    } catch (Exception $e) {
        $caught = 'exception';
    } catch (Error $e) {
        $caught = 'error';
    }

    expect($caught)->toBe('error');
});
