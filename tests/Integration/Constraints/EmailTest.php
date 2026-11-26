<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Email;
use Attributes\Strictify\Fields\ErrorMessage;
use TypeError;

use function Attributes\Strictify\validate;

describe('Email constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Email]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'andre@example.com',
        'a.b+tag@sub.domain.co',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Email]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid email address']]);
        }
    })->with([
        'not-an-email',
        'missing@tld',
        '@example.com',
        'a b@example.com',
        42,
    ]);
});

describe('Email $domain restriction', function () {
    it('defaults to null and keeps the argument as given', function () {
        expect((new Email)->domain)->toBeNull()
            ->and((new Email(null))->domain)
            ->toBeNull()
            ->and((new Email('example.com'))->domain)
            ->toBe('example.com')
            ->and((new Email(['example.com', 'example.org']))->domain)
            ->toBe(['example.com', 'example.org']);
    });

    it('throws a TypeError for unsupported argument types', function (mixed $domain) {
        expect(fn () => new Email($domain))->toThrow(TypeError::class);
    })->with([
        'int' => [42],
        'bool' => [true],
        'object' => [new \stdClass],
        'array of non-strings' => [['example.com', 1]],
    ]);

    it('accepts addresses on the declared string domain, ignoring case', function (string $value) {
        $model = new class extends BaseModel {
            #[Email(domain: 'example.com')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'andre@example.com',
        'andre@EXAMPLE.COM',
    ]);

    it('accepts addresses on any declared array domain', function (string $value) {
        $model = new class extends BaseModel {
            #[Email(domain: ['example.com', 'example.org'])]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'andre@example.com',
        'andre@example.org',
    ]);

    it('reports an error for a domain outside the declared string', function () {
        $model = new class extends BaseModel {
            #[Email(domain: 'example.com')]
            public string $value;
        };

        try {
            validate(['value' => 'andre@gmail.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => [
                "Must be an email address on an allowed domain: 'example.com'",
            ]]);
        }
    });

    it('reports an error listing every allowed domain', function () {
        $model = new class extends BaseModel {
            #[Email(domain: ['example.com', 'example.org'])]
            public string $value;
        };

        try {
            validate(['value' => 'andre@gmail.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => [
                "Must be an email address on an allowed domain: 'example.com', 'example.org'",
            ]]);
        }
    });

    it('matches the whole domain: subdomains are not implied', function () {
        $model = new class extends BaseModel {
            #[Email(domain: 'example.com')]
            public string $value;
        };

        try {
            validate(['value' => 'andre@mail.example.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => [
                "Must be an email address on an allowed domain: 'example.com'",
            ]]);
        }
    });

    it('reports the format error when the address is malformed', function () {
        $model = new class extends BaseModel {
            #[Email(domain: 'example.com')]
            public string $value;
        };

        try {
            validate(['value' => 'a b@example.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid email address']]);
        }
    });

    it('overrides the domain error message with the email_domain key', function () {
        $model = new class extends BaseModel {
            #[Email(domain: 'example.com')]
            #[ErrorMessage(emailDomain: 'Use your work address')]
            public string $value;
        };

        try {
            validate(['value' => 'andre@gmail.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Use your work address']]);
        }
    });
});
