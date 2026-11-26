<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Constraints;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Url;
use Attributes\Strictify\Fields\ErrorMessage;
use TypeError;

use function Attributes\Strictify\validate;

describe('Url constraint', function () {
    it('accepts valid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Url]
            public mixed $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'https://example.com/path?q=1',
        'http://localhost:8080',
        'ftp://files.example.com/x',
    ]);

    it('reports an error for invalid values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[Url]
            public mixed $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid URL']]);
        }
    })->with([
        'example.com',
        'https://',
        '://no-scheme',
        'https://exa mple.com',
        42,
    ]);
});

describe('Url $protocol restriction', function () {
    it('accepts schemes on the declared string, ignoring case and separators', function (string $value) {
        $model = new class extends BaseModel {
            #[Url(protocol: 'https')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'https://example.com',
        'HTTPS://example.com',
        'https://example.com/path?q=1',
    ]);

    it('accepts any scheme in the declared array', function (string $value) {
        $model = new class extends BaseModel {
            #[Url(protocol: ['http', 'https'])]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'http://example.com',
        'https://example.com',
    ]);

    it('tolerates the separators written in the declared protocol', function () {
        $withSlashes = new class extends BaseModel {
            #[Url(protocol: 'https://')]
            public string $value;
        };
        $withColon = new class extends BaseModel {
            #[Url(protocol: 'https:')]
            public string $value;
        };
        $uppercase = new class extends BaseModel {
            #[Url(protocol: 'HTTPS')]
            public string $value;
        };

        foreach ([$withSlashes, $withColon, $uppercase] as $model) {
            expect(validate(['value' => 'https://example.com'], $model)->value)->toBe('https://example.com');
        }
    });

    it('reports an error for a scheme outside the declared list', function () {
        $model = new class extends BaseModel {
            #[Url(protocol: ['http', 'https'])]
            public string $value;
        };

        try {
            validate(['value' => 'ftp://files.example.com/x'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must be a URL using an allowed protocol: 'http', 'https'"]]);
        }
    });
});

describe('Url $domain restriction', function () {
    it('accepts the declared host, ignoring port and userinfo', function (string $value) {
        $model = new class extends BaseModel {
            #[Url(domain: 'example.com')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'https://example.com',
        'https://EXAMPLE.com/path',
        'http://example.com:8443/x',
        'https://user:pass@example.com/x',
    ]);

    it('accepts any host in the declared array', function (string $value) {
        $model = new class extends BaseModel {
            #[Url(domain: ['example.com', 'example.org'])]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'https://example.com',
        'https://example.org',
    ]);

    it('reports an error for a host outside the declared list', function () {
        $model = new class extends BaseModel {
            #[Url(domain: ['example.com', 'example.org'])]
            public string $value;
        };

        try {
            validate(['value' => 'https://gmail.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => [
                "Must be a URL on an allowed domain: 'example.com', 'example.org'",
            ]]);
        }
    });

    it('matches the whole host: subdomains are not implied', function () {
        $model = new class extends BaseModel {
            #[Url(domain: 'example.com')]
            public string $value;
        };

        try {
            validate(['value' => 'https://mail.example.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must be a URL on an allowed domain: 'example.com'"]]);
        }
    });
});

describe('Url $protocol and $domain combined', function () {
    it('accepts a URL satisfying both restrictions', function () {
        $model = new class extends BaseModel {
            #[Url(protocol: 'https', domain: 'example.com')]
            public string $value;
        };

        $result = validate(['value' => 'https://example.com/x'], $model);
        expect($result->value)->toBe('https://example.com/x');
    });

    it('reports the protocol error first', function () {
        $model = new class extends BaseModel {
            #[Url(protocol: 'https', domain: 'example.com')]
            public string $value;
        };

        try {
            validate(['value' => 'http://gmail.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must be a URL using an allowed protocol: 'https'"]]);
        }
    });

    it('reports the domain error when only the host is wrong', function () {
        $model = new class extends BaseModel {
            #[Url(protocol: 'https', domain: 'example.com')]
            public string $value;
        };

        try {
            validate(['value' => 'https://gmail.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must be a URL on an allowed domain: 'example.com'"]]);
        }
    });

    it('keeps the format error for malformed URLs', function () {
        $model = new class extends BaseModel {
            #[Url(protocol: 'https', domain: 'example.com')]
            public string $value;
        };

        try {
            validate(['value' => 'https://exa mple.com'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid URL']]);
        }
    });

    it('overrides the new error messages with their keys', function () {
        $model = new class extends BaseModel {
            #[Url(protocol: 'https')]
            #[ErrorMessage(urlProtocol: 'Only https links please')]
            public string $value;
        };

        try {
            validate(['value' => 'ftp://files.example.com/x'], $model);
            expect(false)->toBeTrue('expected a ValidationException');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Only https links please']]);
        }
    });
});

describe('Url constructor arguments', function () {
    it('defaults both to null and keeps them as given', function () {
        expect((new Url)->protocol)->toBeNull()
            ->and((new Url)->domain)
            ->toBeNull()
            ->and((new Url(null, null))->protocol)
            ->toBeNull()
            ->and((new Url('https'))->protocol)
            ->toBe('https')
            ->and((new Url('https'))->domain)
            ->toBeNull()
            ->and((new Url(null, 'example.com'))->domain)
            ->toBe('example.com')
            ->and((new Url(['http', 'https'], ['example.com']))->protocol)
            ->toBe(['http', 'https'])
            ->and((new Url(['http', 'https'], ['example.com']))->domain)
            ->toBe(['example.com']);
    });

    it('throws a TypeError for unsupported argument types', function (mixed $protocol, mixed $domain) {
        expect(fn () => new Url($protocol, $domain))->toThrow(TypeError::class);
    })->with([
        'protocol int' => [42, null],
        'protocol bool' => [true, null],
        'protocol object' => [new \stdClass, null],
        'protocol array of non-strings' => [['https', 1], null],
        'domain int' => [null, 42],
        'domain array of non-strings' => [null, ['example.com', null]],
    ]);
});
