<?php

declare(strict_types=1);

namespace Attributes\Strictify\Tests\Integration\Validate;

use Attributes\Strictify\BaseModel;
use Attributes\Strictify\ErrorTranslator;
use Attributes\Strictify\Exceptions\ValidationException;
use Attributes\Strictify\Fields\Constraints\Alnum;
use Attributes\Strictify\Fields\Constraints\Alpha;
use Attributes\Strictify\Fields\Constraints\AlphaDash;
use Attributes\Strictify\Fields\Constraints\Ascii;
use Attributes\Strictify\Fields\Constraints\Base64;
use Attributes\Strictify\Fields\Constraints\Between;
use Attributes\Strictify\Fields\Constraints\Contains;
use Attributes\Strictify\Fields\Constraints\ContainsAny;
use Attributes\Strictify\Fields\Constraints\CreditCard;
use Attributes\Strictify\Fields\Constraints\DateTime;
use Attributes\Strictify\Fields\Constraints\Digit;
use Attributes\Strictify\Fields\Constraints\Domain;
use Attributes\Strictify\Fields\Constraints\Email;
use Attributes\Strictify\Fields\Constraints\EndsWith;
use Attributes\Strictify\Fields\Constraints\Even;
use Attributes\Strictify\Fields\Constraints\HexRgbColor;
use Attributes\Strictify\Fields\Constraints\Iban;
use Attributes\Strictify\Fields\Constraints\In;
use Attributes\Strictify\Fields\Constraints\Ip;
use Attributes\Strictify\Fields\Constraints\Isbn;
use Attributes\Strictify\Fields\Constraints\Json;
use Attributes\Strictify\Fields\Constraints\MacAddress;
use Attributes\Strictify\Fields\Constraints\Max;
use Attributes\Strictify\Fields\Constraints\Min;
use Attributes\Strictify\Fields\Constraints\MultipleOf;
use Attributes\Strictify\Fields\Constraints\Negative;
use Attributes\Strictify\Fields\Constraints\Odd;
use Attributes\Strictify\Fields\Constraints\Positive;
use Attributes\Strictify\Fields\Constraints\PrimeNumber;
use Attributes\Strictify\Fields\Constraints\Regex;
use Attributes\Strictify\Fields\Constraints\Size;
use Attributes\Strictify\Fields\Constraints\Slug;
use Attributes\Strictify\Fields\Constraints\StartsWith;
use Attributes\Strictify\Fields\Constraints\Ulid;
use Attributes\Strictify\Fields\Constraints\Url;
use Attributes\Strictify\Fields\Constraints\Uuid;
use Attributes\Strictify\Fields\Constraints\Version;
use Attributes\Strictify\Fields\Constraints\Xdigit;
use Attributes\Strictify\Fields\ErrorMessage;
use Attributes\Strictify\Fields\Sequence;
use Attributes\Strictify\Fields\Union;
use Attributes\Strictify\Type;

use function Attributes\Strictify\validate;

/*
 * Every constraint error key, one property per key on the two models below:
 * one declares an #[ErrorMessage] override, the other leaves the message to
 * the translator. Adding a constraint error type without adding it here fails
 * the count assertion, so the catalog cannot grow silently.
 */
final class ConstraintMessageKeys
{
    public const KEYS = [
        'alpha_dash',
        'alpha_dash_type',
        'ascii',
        'ascii_type',
        'between',
        'between_type',
        'contains',
        'datetime',
        'ends_with',
        'in',
        'max',
        'min',
        'multiple_of',
        'regex',
        'size',
        'starts_with',
        'ulid',
        'uuid',
        'email',
        'email_domain',
        'url',
        'url_protocol',
        'url_domain',
        'domain',
        'ip',
        'iban',
        'isbn',
        'credit_card',
        'version',
        'mac_address',
        'alnum',
        'alpha',
        'digit',
        'xdigit',
        'slug',
        'base64',
        'json',
        'hex_rgb_color',
        'even',
        'odd',
        'positive',
        'negative',
        'prime_number',
        'contains_any',
        'union',
        'sequence_keys',
        'unique_values',
    ];

    /* Inputs that fail exactly the property's own constraint, with the value
     * type the error key expects (a wrong type reports *_type instead). */
    public const INVALID_PAYLOAD = [
        'alpha_dash' => 'with space',
        'alpha_dash_type' => 123,
        'ascii' => "caf\u{e9}",
        'ascii_type' => 123,
        'between' => 0,
        'between_type' => 'abc',
        'contains' => 'haystack',
        'datetime' => 'nope',
        'ends_with' => 'zzz',
        'in' => 'c',
        'max' => 6,
        'min' => 4,
        'multiple_of' => 4,
        'regex' => 'b',
        'size' => 'abc',
        'starts_with' => 'zzz',
        'ulid' => 'nope',
        'uuid' => 'nope',
        'email' => 'nope',
        'email_domain' => 'someone@other.test',
        'url' => 'nope',
        'url_protocol' => 'ftp://example.com',
        'url_domain' => 'https://other.test',
        'domain' => 'nope',
        'ip' => 'nope',
        'iban' => 'nope',
        'isbn' => 'nope',
        'credit_card' => 'nope',
        'version' => 'nope',
        'mac_address' => 'nope',
        'alnum' => 'a-b',
        'alpha' => 'a1',
        'digit' => '12a',
        'xdigit' => 'xyz',
        'slug' => 'Not a slug',
        'base64' => '!!!!',
        'json' => '{',
        'hex_rgb_color' => '#12345',
        'even' => 3,
        'odd' => 2,
        'positive' => -1,
        'negative' => 1,
        'prime_number' => 4,
        'contains_any' => 'zzz',
        'union' => 1.5,
        'sequence_keys' => [5 => 1],
        'unique_values' => [1, 1],
    ];
}

final class CustomConstraintMessages extends BaseModel
{
    #[AlphaDash, ErrorMessage(alphaDash: 'custom:alpha_dash')]
    public string $alpha_dash;

    #[AlphaDash, ErrorMessage(alphaDashType: 'custom:alpha_dash_type')]
    public mixed $alpha_dash_type;

    #[Ascii, ErrorMessage(ascii: 'custom:ascii')]
    public string $ascii;

    #[Ascii, ErrorMessage(asciiType: 'custom:ascii_type')]
    public mixed $ascii_type;

    #[Between(1, 5), ErrorMessage(between: 'custom:between')]
    public int $between;

    #[Between(1, 5), ErrorMessage(betweenType: 'custom:between_type')]
    public mixed $between_type;

    #[Contains('needle'), ErrorMessage(contains: 'custom:contains')]
    public string $contains;

    #[DateTime('Y-m-d'), ErrorMessage(datetime: 'custom:datetime')]
    public string $datetime;

    #[EndsWith('x', 'y'), ErrorMessage(endsWith: 'custom:ends_with')]
    public string $ends_with;

    #[In('a', 'b'), ErrorMessage(in: 'custom:in')]
    public string $in;

    #[Max(5), ErrorMessage(max: 'custom:max')]
    public int $max;

    #[Min(5), ErrorMessage(min: 'custom:min')]
    public int $min;

    #[MultipleOf(3), ErrorMessage(multipleOf: 'custom:multiple_of')]
    public int $multiple_of;

    #[Regex('/^a$/'), ErrorMessage(regex: 'custom:regex')]
    public string $regex;

    #[Size(5), ErrorMessage(size: 'custom:size')]
    public string $size;

    #[StartsWith('x', 'y'), ErrorMessage(startsWith: 'custom:starts_with')]
    public string $starts_with;

    #[Ulid, ErrorMessage(ulid: 'custom:ulid')]
    public string $ulid;

    #[Uuid, ErrorMessage(uuid: 'custom:uuid')]
    public string $uuid;

    #[Email, ErrorMessage(email: 'custom:email')]
    public string $email;

    #[Email(domain: 'example.com'), ErrorMessage(emailDomain: 'custom:email_domain')]
    public string $email_domain;

    #[Url, ErrorMessage(url: 'custom:url')]
    public string $url;

    #[Url(protocol: 'https'), ErrorMessage(urlProtocol: 'custom:url_protocol')]
    public string $url_protocol;

    #[Url(domain: 'example.com'), ErrorMessage(urlDomain: 'custom:url_domain')]
    public string $url_domain;

    #[Domain, ErrorMessage(domain: 'custom:domain')]
    public string $domain;

    #[Ip, ErrorMessage(ip: 'custom:ip')]
    public string $ip;

    #[Iban, ErrorMessage(iban: 'custom:iban')]
    public string $iban;

    #[Isbn, ErrorMessage(isbn: 'custom:isbn')]
    public string $isbn;

    #[CreditCard, ErrorMessage(creditCard: 'custom:credit_card')]
    public string $credit_card;

    #[Version, ErrorMessage(version: 'custom:version')]
    public string $version;

    #[MacAddress, ErrorMessage(macAddress: 'custom:mac_address')]
    public string $mac_address;

    #[Alnum, ErrorMessage(alnum: 'custom:alnum')]
    public string $alnum;

    #[Alpha, ErrorMessage(alpha: 'custom:alpha')]
    public string $alpha;

    #[Digit, ErrorMessage(digit: 'custom:digit')]
    public string $digit;

    #[Xdigit, ErrorMessage(xdigit: 'custom:xdigit')]
    public string $xdigit;

    #[Slug, ErrorMessage(slug: 'custom:slug')]
    public string $slug;

    #[Base64, ErrorMessage(base64: 'custom:base64')]
    public string $base64;

    #[Json, ErrorMessage(json: 'custom:json')]
    public string $json;

    #[HexRgbColor, ErrorMessage(hexRgbColor: 'custom:hex_rgb_color')]
    public string $hex_rgb_color;

    #[Even, ErrorMessage(even: 'custom:even')]
    public int $even;

    #[Odd, ErrorMessage(odd: 'custom:odd')]
    public int $odd;

    #[Positive, ErrorMessage(positive: 'custom:positive')]
    public int $positive;

    #[Negative, ErrorMessage(negative: 'custom:negative')]
    public int $negative;

    #[PrimeNumber, ErrorMessage(primeNumber: 'custom:prime_number')]
    public int $prime_number;

    #[ContainsAny('x', 'y'), ErrorMessage(containsAny: 'custom:contains_any')]
    public string $contains_any;

    #[Union(Type::int, Type::string), ErrorMessage(union: 'custom:union')]
    public $union;

    #[Sequence(Type::int), ErrorMessage(sequenceKeys: 'custom:sequence_keys')]
    public array $sequence_keys;

    #[Sequence(Type::int, uniqueValues: true), ErrorMessage(uniqueValues: 'custom:unique_values')]
    public array $unique_values;
}

final class DefaultConstraintMessages extends BaseModel
{
    #[AlphaDash]
    public string $alpha_dash;

    #[AlphaDash]
    public mixed $alpha_dash_type;

    #[Ascii]
    public string $ascii;

    #[Ascii]
    public mixed $ascii_type;

    #[Between(1, 5)]
    public int $between;

    #[Between(1, 5)]
    public mixed $between_type;

    #[Contains('needle')]
    public string $contains;

    #[DateTime('Y-m-d')]
    public string $datetime;

    #[EndsWith('x', 'y')]
    public string $ends_with;

    #[In('a', 'b')]
    public string $in;

    #[Max(5)]
    public int $max;

    #[Min(5)]
    public int $min;

    #[MultipleOf(3)]
    public int $multiple_of;

    #[Regex('/^a$/')]
    public string $regex;

    #[Size(5)]
    public string $size;

    #[StartsWith('x', 'y')]
    public string $starts_with;

    #[Ulid]
    public string $ulid;

    #[Uuid]
    public string $uuid;

    #[Email]
    public string $email;

    #[Email(domain: 'example.com')]
    public string $email_domain;

    #[Url]
    public string $url;

    #[Url(protocol: 'https')]
    public string $url_protocol;

    #[Url(domain: 'example.com')]
    public string $url_domain;

    #[Domain]
    public string $domain;

    #[Ip]
    public string $ip;

    #[Iban]
    public string $iban;

    #[Isbn]
    public string $isbn;

    #[CreditCard]
    public string $credit_card;

    #[Version]
    public string $version;

    #[MacAddress]
    public string $mac_address;

    #[Alnum]
    public string $alnum;

    #[Alpha]
    public string $alpha;

    #[Digit]
    public string $digit;

    #[Xdigit]
    public string $xdigit;

    #[Slug]
    public string $slug;

    #[Base64]
    public string $base64;

    #[Json]
    public string $json;

    #[HexRgbColor]
    public string $hex_rgb_color;

    #[Even]
    public int $even;

    #[Odd]
    public int $odd;

    #[Positive]
    public int $positive;

    #[Negative]
    public int $negative;

    #[PrimeNumber]
    public int $prime_number;

    #[ContainsAny('x', 'y')]
    public string $contains_any;

    #[Union(Type::int, Type::string)]
    public $union;

    #[Sequence(Type::int)]
    public array $sequence_keys;

    #[Sequence(Type::int, uniqueValues: true)]
    public array $unique_values;
}

/* Path the error is reported under: the array keys of a #[Sequence] are
 * reported per element, every other constraint on its property. */
function constraint_message_path(string $key): string
{
    return $key === 'sequence_keys' ? 'sequence_keys.5' : $key;
}

function constraint_message_errors(BaseModel $model): array
{
    try {
        validate(ConstraintMessageKeys::INVALID_PAYLOAD, $model);
    } catch (ValidationException $e) {
        return $e->getErrors();
    }

    return [];
}

afterEach(function () {
    ErrorTranslator::set(fn (?string $key, string $message): ?string => null);
});

describe('constraint error messages', function () {
    it('reports exactly one error per declared constraint key', function () {
        $errors = constraint_message_errors(new DefaultConstraintMessages);

        expect(array_keys($errors))->toEqualCanonicalizing(array_map(
            fn (string $key): string => constraint_message_path($key),
            ConstraintMessageKeys::KEYS,
        ))
            ->and(array_filter($errors, fn (array $messages): bool => count($messages) !== 1))
            ->toBeEmpty();
    });

    it('overrides every constraint message with #[ErrorMessage]', function () {
        $errors = constraint_message_errors(new CustomConstraintMessages);

        $unexpected = [];
        foreach (ConstraintMessageKeys::KEYS as $key) {
            $expected = ['custom:'.$key];
            if (($errors[constraint_message_path($key)] ?? null) !== $expected) {
                $unexpected[$key] = $errors[constraint_message_path($key)] ?? null;
            }
        }

        expect($unexpected)->toBeEmpty();
    });

    it('translates every constraint message by its key through ErrorTranslator', function () {
        ErrorTranslator::set(fn (?string $key, string $message): ?string => $key === null ? null : 'translated:'.$key);

        $errors = constraint_message_errors(new DefaultConstraintMessages);

        $unexpected = [];
        foreach (ConstraintMessageKeys::KEYS as $key) {
            $expected = ['translated:'.lcfirst(str_replace('_', '', ucwords($key, '_')))];
            if (($errors[constraint_message_path($key)] ?? null) !== $expected) {
                $unexpected[$key] = $errors[constraint_message_path($key)] ?? null;
            }
        }

        expect($unexpected)->toBeEmpty();
    });

    it('passes a null key and the ErrorMessage value to the translator', function () {
        $seen = [];
        ErrorTranslator::set(function (?string $key, string $message) use (&$seen): ?string {
            $seen[] = [$key, $message];

            return $key === null ? 'translated:'.$message : null;
        });

        $model = new class extends BaseModel {
            #[Min(5), ErrorMessage(min: 'too.small')]
            public int $age;

            #[Max(5)]
            public int $score;
        };

        try {
            validate(['age' => 1, 'score' => 9], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'age' => ['translated:too.small'],
                'score' => ['Must be at most 5'],
            ]);
        }

        // The template arrives unsubstituted: placeholders are filled after the translator returns.
        expect($seen)->toBe([
            [null,  'too.small'],
            ['max', 'Must be at most {expected}'],
        ]);
    });

    it('keeps the literal ErrorMessage value when the translator declines it', function () {
        ErrorTranslator::set(fn (?string $key, string $message): ?string => null);

        $model = new class extends BaseModel {
            #[Email, ErrorMessage(email: 'Type a real address')]
            public string $email;
        };

        try {
            validate(['email' => 'nope'], $model);
            expect(false)->toBeTrue('ValidationException expected');
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['email' => ['Type a real address']]);
        }
    });
});
