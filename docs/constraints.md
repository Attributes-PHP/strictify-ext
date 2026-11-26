---
title: "Constraints"
description: "Reference for the Strictify constraint attributes: the built-in constraints grouped by category, and how to write custom constraints in PHP userland."
---

# Constraints

Constraints are attributes that add value-level rules on top of the native type hints. They run after the property's type check succeeds (so they always see a value of the declared type, or the coerced value in loose mode) and before the value is written to the model — a failing constraint keeps the value out of the model.

```php
<?php
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Constraints\Between;
use Attributes\Strictify\Fields\Constraints\Regex;

class User extends BaseModel
{
    #[Between(18, 99)]
    public int $age;

    #[Regex('/^[a-z][a-z0-9-]*$/')]
    public string $username;
}
```

Constraints compose: every constraint declared on a property runs, and each failing one reports its own error under the resolved field name (the `Alias` name, when one is declared). `ModelConfigs(stopAtFirstError: true)` aborts the run at the first error, as with type errors.

Attribute constructor arguments are type-checked at construction — an argument the constructor signature rejects (`#[Between('a', 'b')]`) throws an `InvalidBaseModelException` on the first `validate()` call and keeps throwing on every call until the attribute is fixed. Arguments that satisfy the signature but can never behave usefully throw the same exception, see below.

## Malformed constraint arguments

A constraint whose declared arguments make it unsatisfiable, or silently turn it into a no-op, is a malformed model definition: `validate()` throws an `InvalidBaseModelException` naming the attribute, the model class and the property, and keeps throwing on every call until the attribute is fixed.

```php
<?php
class User extends BaseModel
{
    #[Between(99, 18)]
    public int $age;
}

validate(['age' => 5], new User());
// InvalidBaseModelException: Attribute "Attributes\Strictify\Fields\Constraints\Between" on User::$age:
//   $min (99) must be less than or equal to $max (18).
```

| Constraint | Rejected arguments |
| ---------- | ------------------ |
| `Between` | a `NAN` bound; `$min` greater than `$max` |
| `Max`, `Min`, `MultipleOf` | a `NAN` bound |
| `Size` | a negative `$min` or `$max`; `$min` greater than `$max` |
| `In` | no declared value |
| `StartsWith`, `EndsWith` | no declared value; an empty string entry (it matches every value) |
| `Regex` | a pattern that cannot compile: missing or unterminated delimiter, unknown or duplicate modifier, invalid body, embedded NUL byte |
| `Email` | `domain: ''`, `domain: []`, or an empty string entry in the list |
| `Url` | `protocol`/`domain` set to `''`, `[]`, or a list holding an empty string |

Degenerate-but-satisfiable arguments are accepted: `Between(5, 5)`, `Between(-INF, INF)`, `Size` with no bounds, `MultipleOf(0)`, `MultipleOf(-5)`, and a pattern with every supported modifier (`/^[a-z]+$/iDSUXn`).

Because the rule runs during the field's first compile, the check is per model class: the `InvalidBaseModelException` fires even when the payload has no value for the property, and it never caches a partial result.

## Built-in constraints at a glance

All built-ins live in `Attributes\Strictify\Fields\Constraints` and may be declared on properties and parameters.

| Attribute | Category | Constructor | Fails when |
| --------- | -------- | ----------- | ---------- |
| [`AlphaDash`](#alphadash) | string | `()` | the string contains characters other than letters, numbers, dashes and underscores |
| [`Ascii`](#ascii) | string | `()` | the string contains bytes outside 7-bit ASCII (0x00-0x7F) |
| [`Regex`](#regex) | string | `(string $regex)` | the string does not match the pattern (`preg_match` semantics: a partial match passes) |
| [`StartsWith`](#startswith) | string | `(string\|int\|float ...$values)` | the string starts with none of the declared values |
| [`EndsWith`](#endswith) | string | `(string\|int\|float ...$values)` | the string ends with none of the declared values |
| [`Ulid`](#ulid) | string | `()` | the string is not a valid ULID (26 Crockford base32 characters, either letter case) |
| [`Uuid`](#uuid) | string | `()` | the string is not a canonical UUID (8-4-4-4-12 hex groups separated by dashes, either letter case) |
| [`Between`](#between) | numeric | `(int\|float $min, int\|float $max)` | the number is outside the inclusive range |
| [`Max`](#max) | numeric | `(int\|float $value)` | the number exceeds the inclusive upper bound |
| [`Min`](#min) | numeric | `(int\|float $value)` | the number is below the inclusive lower bound |
| [`MultipleOf`](#multipleof) | numeric | `(int\|float $value)` | the number is not a multiple of the divisor |
| [`In`](#in) | membership | `(string\|int\|float\|bool ...$values)` | the value matches none of the declared values |
| [`Contains`](#contains) | membership | `(string\|int\|float ...$values, at least one)` | an array is missing one of the declared values, or a string is missing one of them as a substring |
| [`Size`](#size) | size | `(?int $min = null, ?int $max = null)` | the string length or array count is outside the inclusive bounds |
| [`DateTime`](#datetime) | date/time | `(string $format = 'X-m-d\\TH:i:sP', DateTimeZone\|string\|null $timezone = null)` | the value does not parse in the declared format and timezone |
| [`Email`](#email) | format | `(?string\|array $domain = null)` | the string is not a valid email address, or its domain is not in the declared `$domain` list |
| [`Url`](#url) | format | `(?string\|array $protocol = null, ?string\|array $domain = null)` | the string is not a URL with a scheme (`https://...`), or its scheme/host is not in the declared `$protocol`/`$domain` |
| [`Domain`](#domain) | format | `()` | the string is not a valid domain name |
| [`Ip`](#ip) | format | `(?string $version = null)` | the string is not an IP address (`v4`/`v6` restricts the family) |
| [`Iban`](#iban) | format | `()` | the string is not a valid IBAN (mod-97 checksum) |
| [`Isbn`](#isbn) | format | `()` | the string is not a valid ISBN-10 or ISBN-13 |
| [`CreditCard`](#creditcard) | format | `()` | the string is not a 12-19 digit number passing the Luhn check |
| [`Version`](#version) | format | `()` | the string is not a semantic version (`1.2.3`, `v2.0.0-rc.1+build`) |
| [`MacAddress`](#macaddress) | format | `()` | the string is not a MAC address (`00:1A:2B:3C:4D:5E`) |
| [`Alnum`](#alnum) | string | `(string $extra = '')` | the string contains characters other than letters, numbers and `$extra` |
| [`Alpha`](#alpha) | string | `(string $extra = '')` | the string contains non-letter characters (beyond `$extra`) |
| [`Digit`](#digit) | string | `()` | the string is empty or contains non-digit characters |
| [`Xdigit`](#xdigit) | string | `()` | the string is empty or contains non-hexadecimal characters |
| [`Slug`](#slug) | string | `()` | the string is not a lowercase slug (`hello-world-2`) |
| [`Base64`](#base64) | string | `()` | the string is not valid base64 (length a multiple of 4, padding only at the end) |
| [`Json`](#json) | string | `()` | the string is not valid JSON |
| [`HexRgbColor`](#hexrgbcolor) | string | `()` | the string is not a hex RGB(A) color (`#fff`, `#ff0080`) |
| [`Even`](#even) | numeric | `()` | the number is not even |
| [`Odd`](#odd) | numeric | `()` | the number is not odd |
| [`Positive`](#positive) | numeric | `()` | the number is not greater than zero |
| [`Negative`](#negative) | numeric | `()` | the number is not less than zero |
| [`PrimeNumber`](#primenumber) | numeric | `()` | the integer is not prime |
| [`ContainsAny`](#containsany) | membership | `(string\|int\|float ...$values, at least one)` | an array contains none of the declared values, or a string contains none of them as a substring |

Named constructor arguments work everywhere: `#[Size(min: 3)]` validates only the lower bound, `#[DateTime(timezone: 'Europe/London')]` parses in that timezone.

## String constraints

One section per built-in constraint, covering every scenario of that constraint. All examples assume `use Attributes\Strictify\BaseModel;` and the constraint class imported from `Attributes\Strictify\Fields\Constraints`.

### `AlphaDash`

Letters, numbers, dashes and underscores only.

```php
<?php
class Profile extends BaseModel
{
    #[AlphaDash] // 'user-1_name' passes
    public string $slug;
}
```

Passes: `'user-1_name'`, `'A1b2C3'`, `''`. Fails: `'user name'` (space), `'user.name'` (dot), `'héllo'`, and any non-string value.

### `Ascii`

7-bit ASCII bytes (0x00-0x7F) only.

```php
<?php
class Label extends BaseModel
{
    #[Ascii] // 'hello ~!@#' passes
    public string $text;
}
```

Passes: `'plain text'`, `''`, `"tabs\tand\nnewlines"`. Fails: `'héllo'` (UTF-8 bytes above 0x7F), `'em—dash'`, and any non-string value.

### `Regex`

`preg_match` semantics: a partial match passes.

```php
<?php
class Account extends BaseModel
{
    #[Regex('/^[a-z][a-z0-9-]*$/')] // 'user-name' passes, 'User Name' fails
    public string $username;

    #[Regex('/a+/')] // 'bbbaaabb' passes (partial match), 'bbb' fails
    public string $hasA;
}
```

Passes: `'user-name'`, `'bbbaaabb'` (partial match). Fails: `'User Name'`, `'bbb'`, strings when the pattern fails to compile, and non-string values.

The pattern is validated when the property's field is compiled, so a typo surfaces as an `InvalidBaseModelException` naming the attribute, the model and the property — never as a PHP warning. Supported modifiers are `i`, `m`, `s`, `x`, `A`, `D`, `U`, `J`, `n`, `u` and `r`; `S` and `X` are accepted no-ops. See [malformed constraint arguments](#malformed-constraint-arguments).

### `StartsWith`

The string must start with one of the declared values; numeric values match by their string representation.

```php
<?php
class Link extends BaseModel
{
    #[StartsWith('https://')] // 'https://example.com' passes, 'http://example.com' fails
    public string $url;

    #[StartsWith(42)] // '4242' passes, '142' fails
    public string $code;
}
```

Passes: `'https://example.com'`, `'4242'`. Fails: `'http://example.com'`, `'142'`, prefixes longer than the value, and non-string values.

### `EndsWith`

The string must end with one of the declared values; numeric values match by their string representation.

```php
<?php
class Contact extends BaseModel
{
    #[EndsWith('.com', '.net')] // 'andre@example.com' passes, 'andre@example.org' fails
    public string $email;

    #[EndsWith(42)] // 'answer 42' passes, 'answer 43' fails
    public string $note;
}
```

Passes: `'andre@example.com'`, `'answer 42'`. Fails: `'andre@example.org'`, `'answer 43'`, suffixes longer than the value, and non-string values.

### `Ulid`

Exactly 26 Crockford base32 characters, either letter case. A ULID encodes 128 bits, so the first character is limited to `0`-`7`: a larger first character overflows the range.

```php
<?php
class Event extends BaseModel
{
    #[Ulid] // '01ARZ3NDEKTSV4RRFFQ69G5FAV' (or lowercase) passes
    public string $id;
}
```

Passes: `'01ARZ3NDEKTSV4RRFFQ69G5FAV'`, `'01arz3ndektsv4rrffq69g5fav'`. Fails: wrong length (25 or 27 chars), the letters `I`, `L`, `O`, `U` in either case, a first character above `7` (`'Z1ARZ3NDEKTSV4RRFFQ69G5FAV'`), `''`, any other character, and non-string values.

### `Uuid`

Canonical 8-4-4-4-12 hexadecimal UUID, either letter case.

```php
<?php
class Request extends BaseModel
{
    #[Uuid] // '123e4567-e89b-12d3-a456-426614174000' and '123E4567-E89B-12D3-A456-426614174000' pass
    public string $id;
}
```

Passes: `'123e4567-e89b-12d3-a456-426614174000'`, `'123E4567-E89B-12D3-A456-426614174000'`. Fails: wrong length, a dash in the wrong position, a non-hex character, and non-string values.

## Numeric constraints

### `Between`

Inclusive numeric range; works for `int` and `float`.

```php
<?php
class User extends BaseModel
{
    #[Between(18, 99)] // 18 and 99 pass (inclusive), 17 and 100 fail
    public int $age;

    #[Between(0.5, 2.5)] // 1.5 passes, 3.0 fails
    public float $ratio;
}
```

Passes: `18`, `99`, `1.5`. Fails: `17`, `100`, `3.0` against the float range, and any non-numeric value.

### `Max`

Inclusive numeric upper bound.

```php
<?php
class Score extends BaseModel
{
    #[Max(100)] // 100 passes, 101 fails
    public float $points;
}
```

Passes: `100`, `-5`. Fails: `101` and non-numeric values.

### `Min`

Inclusive numeric lower bound.

```php
<?php
class Cart extends BaseModel
{
    #[Min(1)] // 1 passes, 0 fails
    public int $quantity;
}
```

Passes: `1`, `42`. Fails: `0` and non-numeric values.

### `MultipleOf`

Integer modulo for int pairs, `fmod` with a small tolerance for float pairs.

```php
<?php
class Pack extends BaseModel
{
    #[MultipleOf(5)] // 20 passes, 21 fails
    public int $size;

    #[MultipleOf(0.1)] // 0.3 passes (float tolerance), 0.35 fails
    public float $weight;

    #[MultipleOf(0)] // only 0 passes
    public int $zero;
}
```

Passes: `20`, `0.3`. Fails: `21`, `0.35`, and non-numeric values. The divisor may be negative — `MultipleOf(-5)` accepts `-10` — and only `0` is a multiple of `0`, for the integer `0` and the float `0.0` alike.

## Membership constraints

### `In`

Strict membership in the declared values.

```php
<?php
class Grade extends BaseModel
{
    #[In('a', 'b', 'c')] // 'b' passes, 'd' fails
    public string $letter;

    #[In(1, 2, true)] // 1 and true pass; strict: the string '1' fails
    public int|bool $code;
}
```

Passes: `'b'`, `1`, `true`. Fails: `'d'`, `'1'` (strict: not the integer `1`), and any value identical to none of the declared values.

### `Contains`

Arrays and strings are both supported; every declared value must be present.

```php
<?php
class Article extends BaseModel
{
    // array: every declared value must be present
    #[Contains('php', 'dev')] // ['php', 'dev', 'oss'] passes, ['php'] fails
    public array $tags;

    // string: every declared value must appear as a substring
    #[Contains('example')] // 'see example.com' passes, 'see other.org' fails
    public string $link;
}
```

Passes: `['php', 'dev', 'oss']`, `['dev', 'php']`, `'see example.com'`. Fails: `['php']`, `['oss']`, `'see other.org'`, and any value that is neither array nor string.

When one match out of several is enough, use [`ContainsAny`](#containsany) — it is the same check with the "at least one" quantifier, and reports `Must contain at least one of: ...`.

## Size

### `Size`

String length in bytes or array element count, within inclusive bounds; either bound may be omitted.

```php
<?php
class Profile extends BaseModel
{
    #[Size(min: 3, max: 20)] // 'andre' passes, 'ab' and a 21-character string fail
    public string $name;

    #[Size(min: 1)] // only the lower bound applies: [] fails, ['a'] passes
    public array $tags;

    #[Size(max: 5)] // only the upper bound applies: [] passes, 6 elements fail
    public array $scores;

    #[Size] // no bounds: any string or array passes
    public array|string $anything;
}
```

Passes: `'andre'`, `'abc'`, `['a']`, `[]` with a max-only bound. Fails: `'ab'`, a 21-character string, `[]` with a min bound, six elements against `#[Size(max: 5)]`, and values that are neither string nor array.

## Date and time

### `DateTime`

Parses in the declared format and timezone; `DateTimeInterface` instances pass as-is.

```php
<?php
class Event extends BaseModel
{
    #[DateTime(format: 'Y-m-d')] // '1994-01-01' passes, '01/01/1994' fails
    public string $date;

    #[DateTime(timezone: 'Europe/London')] // parses in that timezone
    public string $startsAt;

    #[DateTime(format: 'Y-m-d')] // hydrated to a DateTime, then the constraint sees the instance and passes
    public \DateTime $birthday;
}
```

Passes: `'1994-01-01'`, a `DateTimeInterface` instance, `'2024-03-01T10:00:00+00:00'`. Fails: a string that does not parse in the declared format (`'01/01/1994'`), a date that does not exist (`'1994-02-30'`, `'1994-13-01'`, `'2023-02-29'`) — the date extension would roll it over instead of raising — and any value that is neither string nor `DateTimeInterface`. Unpadded fields are still accepted (`'1994-1-1'` under `format: 'Y-m-d'`).

!!! tip "Type-level rules on union-typed properties"
    Declare [`Union`](/guides/nested-models/#union-on-non-array-properties) directly on the property to check the whole value against its arms — on a union hint such as `int|float` the arms must repeat the hint's type-hints exactly, otherwise an `InvalidBaseModelException` is thrown.

## Format and network constraints

### `Email`

A valid email address, optionally restricted to a domain or a list of domains. Constructor: `(?string|array $domain = null)`.

```php
<?php
class Contact extends BaseModel
{
    #[Email] // 'andre@example.com', 'a.b+tag@sub.domain.co' pass; 'missing@tld' fails
    public string $value;

    #[Email(domain: 'example.com')] // 'andre@example.com' passes; 'andre@gmail.com' fails
    public string $work;

    // several domains: one array entry per allowed domain
    #[Email(domain: ['example.com', 'example.org'])] // 'andre@example.org' passes
    public string $anyOf;
}
```

`$domain` is compared with the part after the `@`, case-insensitively and in full — `domain: 'example.com'` does not allow `andre@mail.example.com`. Leave it `null` (the default) to accept any domain. The top-level domain must be at least two characters long, exactly like [`Domain`](#domain).

Passes: `andre@example.com`, `a.b+tag@sub.domain.co`. Fails: `not-an-email`, `missing@tld`, `@example.com`, `a b@example.com`, and — with `$domain` declared — `andre@gmail.com` (`Must be an email address on an allowed domain: 'example.com'`) or `andre@mail.example.com`.

### `Url`

An absolute URL with a scheme, optionally restricted to a protocol and a host. Constructor: `(?string|array $protocol = null, ?string|array $domain = null)`.

```php
<?php
class Link extends BaseModel
{
    #[Url] // 'https://example.com/path?q=1', 'ftp://files.example.com/x' pass; 'example.com' fails
    public string $value;

    #[Url(protocol: 'https')] // 'https://example.com' passes; 'http://example.com' fails
    public string $secure;

    // several entries: one array element per allowed protocol or host
    #[Url(protocol: ['http', 'https'], domain: ['example.com', 'example.org'])]
    public string $internal;
}
```

`$protocol` is compared with the scheme, `$domain` with the host — both case-insensitively and in full, so `domain: 'example.com'` does not allow `https://mail.example.com`. Ports and userinfo are ignored on the URL side (`https://user:pass@example.com:8443` matches host `example.com`). Protocol entries may be written `https`, `https:` or `https://`; they mean the same scheme. Leave either argument `null` (the default) to accept any.

Passes: any absolute URL with a scheme — `https://example.com/path?q=1`, `http://localhost:8080`, `ftp://files.example.com/x`. Fails: `example.com` (no scheme), `https://` (no host), `://no-scheme`, `https://exa mple.com`, and — with the arguments declared — `http://example.com` under `protocol: 'https'` (`Must be a URL using an allowed protocol: 'https'`) or `https://gmail.com` under `domain: 'example.com'` (`Must be a URL on an allowed domain: 'example.com'`).

### `Domain`

The string is not a valid domain name. Constructor: `()`.

```php
<?php
class Site extends BaseModel
{
    #[Domain] // 'example.com', 'sub.domain.co.uk', 'a-b.io' pass; '-bad.com' fails
    public string $value;
}
```

Passes: `example.com`, `sub.domain.co.uk`, `a-b.io`. Fails: `-bad.com` (leading dash), `example` (no TLD), `example.c` (one-letter TLD), `exa mple.com`.

### `Ip`

The string is not an IP address (`v4`/`v6` restricts the family). Constructor: `(?string $version = null)`.

```php
<?php
class Request extends BaseModel
{
    #[Ip] // '192.168.0.1', '::1' and '2001:db8::ff00:42:8329' pass; '256.1.1.1' fails
    public string $address;

    #[Ip(version: 'v4')] // '192.168.0.1' passes, '::1' fails
    public string $ipv4;

    #[Ip(version: 'v6')] // '::1' passes, '192.168.0.1' fails
    public string $ipv6;
}
```

Passes: any IPv4 or IPv6 literal (`192.168.0.1`, `::1`, `2001:db8::ff00:42:8329`). Fails: `256.1.1.1` (octet above 255), `1.2.3` (incomplete), `hello`. With `version: 'v4'`/`'v6'` the other family fails too.

### `Iban`

The string is not a valid IBAN (mod-97 checksum). Constructor: `()`.

```php
<?php
class BankAccount extends BaseModel
{
    #[Iban] // 'DE89370400440532013000', 'GB29NWBK60161331926819' pass; 'DE89370400440532013001' fails
    public string $value;
}
```

Passes: `DE89370400440532013000`, `GB29NWBK60161331926819`, and the lowercase form of either (`de89370400440532013000`). Fails: `DE89370400440532013001` (mod-97 checksum), `XX00123` (no such country code), `DE89 3704` (spaces).

### `Isbn`

The string is not a valid ISBN-10 or ISBN-13. Constructor: `()`.

```php
<?php
class Book extends BaseModel
{
    #[Isbn] // '978-0-13-468599-1', '0-306-40615-2', '9783161484100' pass; '0-306-40615-3' fails
    public string $value;
}
```

Passes: ISBN-10 (`0-306-40615-2`) and ISBN-13 (`978-0-13-468599-1`, `9783161484100`), with or without dashes. Fails: `0-306-40615-3` (bad check digit), `12345` (too short), `97831614841XX`.

### `CreditCard`

The string is not a 12-19 digit number passing the Luhn check. Constructor: `()`.

```php
<?php
class Card extends BaseModel
{
    #[CreditCard] // '4111 1111 1111 1111', '5500-0000-0000-0004' pass; '4111 1111 1111 1112' fails
    public string $value;
}
```

Passes: 12-19 digits, spaces or dashes allowed, Luhn check — `4111 1111 1111 1111`, `5500-0000-0000-0004`, `378282246310005`. Fails: `4111 1111 1111 1112` (Luhn), `1234` (too short), `abcd` (non-digits).

### `Version`

The string is not a semantic version (`1.2.3`, `v2.0.0-rc.1+build`). Constructor: `()`.

```php
<?php
class Release extends BaseModel
{
    #[Version] // '1.0.0', 'v2.3.1', '1.0.0-rc.1+build.5' pass; '1.0' fails
    public string $value;
}
```

Passes: full semantic versions — `1.0.0`, `v2.3.1`, `1.0.0-rc.1+build.5`. Fails: `1.0` (missing patch), `01.2.3` (leading zero), `1.0.0-` (empty pre-release), `latest`.

### `MacAddress`

The string is not a MAC address (`00:1A:2B:3C:4D:5E`). Constructor: `()`.

```php
<?php
class Device extends BaseModel
{
    #[MacAddress] // '00:1A:2B:3C:4D:5E', '00-1a-2b-3c-4d-5e' pass; '001A2B3C4D5E' fails
    public string $value;
}
```

Passes: six hex pairs separated by `:` or `-`, either letter case — `00:1A:2B:3C:4D:5E`, `00-1a-2b-3c-4d-5e`. Fails: `00:1A:2B:3C:4D` (five pairs), `001A2B3C4D5E` (no separators), `zz:1a:2b:3c:4d:5e` (non-hex).

## More string constraints

### `Alnum`

The string contains characters other than letters, numbers and `$extra`. Constructor: `(string $extra = '')`. Non-string values fail.

```php
<?php
class Username extends BaseModel
{
    #[Alnum] // 'abc123' and '' pass, 'abc-123' fails
    public string $plain;

    #[Alnum(extra: '-_')] // 'abc-123' passes with the extra characters declared
    public string $slug;
}
```

Passes: `abc123`, `ABC`, `''`. Fails: `abc-123`, `a b`.

### `Alpha`

The string contains non-letter characters (beyond `$extra`). Constructor: `(string $extra = '')`. Non-string values fail.

```php
<?php
class Name extends BaseModel
{
    #[Alpha] // 'ABCdef' and '' pass, 'a b' fails
    public string $plain;

    #[Alpha(extra: ' ')] // 'a b' passes with the space declared
    public string $withSpaces;
}
```

Passes: `abc`, `ABCdef`, `''`. Fails: `abc1`, `a b`.

### `Digit`

The string is empty or contains non-digit characters. Constructor: `()`. Non-string values fail.

```php
<?php
class Quantity extends BaseModel
{
    #[Digit] // '0123456789' and '42' pass, '12a' fails
    public string $value;
}
```

Passes: `0123456789`, `42`. Fails: `12a`, `''` (empty).

### `Xdigit`

The string is empty or contains non-hexadecimal characters. Constructor: `()`. Non-string values fail.

```php
<?php
class Color extends BaseModel
{
    #[Xdigit] // '0aF9' and 'FF' pass, '0g' fails
    public string $value;
}
```

Passes: `0aF9`, `FF`. Fails: `0g`, `''` (empty).

### `Slug`

The string is not a lowercase slug (`hello-world-2`). Constructor: `()`. Non-string values fail.

```php
<?php
class Post extends BaseModel
{
    #[Slug] // 'hello-world', 'php8', 'a' pass; 'Hello' fails
    public string $value;
}
```

Passes: `hello-world`, `php8`, `a`. Fails: `Hello` (uppercase), `-lead`, `trail-`, `a__b`.

### `Base64`

The string is not valid base64 (length a multiple of 4, padding only at the end). Constructor: `()`. Non-string values fail.

```php
<?php
class Blob extends BaseModel
{
    #[Base64] // 'aGVsbG8=' and '' pass, 'aGVsbG8' fails
    public string $value;
}
```

Passes: `aGVsbG8=`, `AAAA`, `''`. Fails: `aGVsbG8` (length not a multiple of 4), `aGVsbG8===` (padding), `a b=`.

### `Json`

The string is not valid JSON. Constructor: `()`. Non-string values fail.

```php
<?php
class Payload extends BaseModel
{
    #[Json] // '{"a":1}', '[1,2,3]', '"str"', '42' pass; '{invalid}' fails
    public string $value;
}
```

Passes: `{"a":1}`, `[1,2,3]`, `"str"`, `null`, `42`. Fails: `{invalid}`, `[1,`. Nesting is checked with `json_decode`'s default depth of 512 levels, so a document nested deeper fails as `Must be valid JSON`.

### `HexRgbColor`

The string is not a hex RGB(A) color (`#fff`, `#ff0080`). Constructor: `()`. Non-string values fail.

```php
<?php
class Theme extends BaseModel
{
    #[HexRgbColor] // '#fff', '#FFFFFF', '#ffff' (RGBA shorthand) and '#ff000080' pass; '#ggg' fails
    public string $value;
}
```

Passes: `#fff`, `#ffff` (4-digit RGBA shorthand), `#FFFFFF` (6-digit), `#ff000080` (8-digit, alpha). Fails: `#ff` (two digits), `#fffff` (five), `fff` (no leading `#`), `#ggg` (non-hex).

## More numeric constraints

### `Even`

The number is not even. Constructor: `()`. Non-numeric values fail.

```php
<?php
class Counter extends BaseModel
{
    #[Even] // 2 and -4 pass, 3 fails
    public int|float $value;
}
```

Passes: `2`, `0`, `-4`. Fails: `3`, `2.5`, and non-numeric values.

### `Odd`

The number is not odd. Constructor: `()`. Non-numeric values fail.

```php
<?php
class Ticket extends BaseModel
{
    #[Odd] // 3 and -1 pass, 2 fails
    public int|float $value;
}
```

Passes: `3`, `1`, `-1`. Fails: `2`, `0`, `3.5`, and non-numeric values.

### `Positive`

The number is not greater than zero. Constructor: `()`. Non-numeric values fail.

```php
<?php
class Balance extends BaseModel
{
    #[Positive] // 1 and 0.5 pass, 0 and -1 fail
    public int|float $value;
}
```

Passes: `1`, `0.5`. Fails: `0`, `-1`, and non-numeric values.

### `Negative`

The number is not less than zero. Constructor: `()`. Non-numeric values fail.

```php
<?php
class Debt extends BaseModel
{
    #[Negative] // -1 and -0.5 pass, 0 and 1 fail
    public int|float $value;
}
```

Passes: `-1`, `-0.5`. Fails: `0`, `1`, and non-numeric values.

### `PrimeNumber`

The integer is not prime. Constructor: `()`. Non-numeric values fail.

```php
<?php
class Divisor extends BaseModel
{
    #[PrimeNumber] // 2 and 97 pass, 1 and 9 fail
    public int|float $value;
}
```

Passes: `2`, `97`, `7919`. Fails: `0`, `1`, `4`, `9` (3 × 3), `-7`, and the numeric string `'7'`.

## Array constraints

### `ContainsAny`

At least one of the declared values must be present, in an array or as a substring of a string. Constructor: `(string\|int\|float ...$values, at least one)`.

```php
<?php
class Model extends BaseModel
{
    #[ContainsAny('x', 'y')] // ['y'], [1, 'y'] and 'say hello' pass
    public array|string $value;
}
```

Passes: `['y']`, `[1, 'y']`, `'say hello'`. Fails: `['a', 'b']`, `'nope'`, and any non-array, non-string value such as `42`.

## Value semantics

- **Numeric constraints** (`Between`, `Max`, `Min`, `MultipleOf`, `Even`, `Odd`, `Positive`, `Negative`, `PrimeNumber`) apply to `int` and `float` values; any other type fails. `MultipleOf` compares integer pairs with integer modulo and float pairs with `fmod` plus a small tolerance, so `0.3` counts as a multiple of `0.1`. Only `0` is a multiple of `0`, and a negative divisor is allowed.
- **String constraints** (`AlphaDash`, `Ascii`, `Regex`, `StartsWith`, `EndsWith`, `Ulid`, `Uuid`, `Email`, `Url`, `Domain`, `Ip`, `Iban`, `Isbn`, `CreditCard`, `Version`, `MacAddress`, `Alnum`, `Alpha`, `Digit`, `Xdigit`, `Slug`, `Base64`, `Json`, `HexRgbColor`) fail for non-string values. `StartsWith`/`EndsWith` accept numeric declared values and match them by their string representation.
- **Membership constraints** (`In`, `Contains`) compare strictly: `In('1')` rejects the integer `1`. On arrays, `Contains` requires every declared value to be present; on strings, every declared value must appear as a substring. `ContainsAny` is the "at least one match" variant of the same check.
- **`DateTime`** parses with the date extension in the declared format and timezone. `DateTimeInterface` instances pass as-is, an invalid timezone name throws an `InvalidBaseModelException` as a declaration argument (a `ValueError` when the object is built directly), and a date that does not exist (`1994-02-30`) is rejected instead of rolling over.
- **`Size`** measures strings in bytes and arrays in element count. With only one bound declared, only that bound applies; with neither, every value passes.
- **`null` skips every constraint** on the property, exactly like a missing key: `#[Regex('/^[a-z]+$/')] public ?string $name = null` accepts `['name' => null]` and `[]`. A non-null value is checked as usual.
- **The empty string** is accepted by `Alnum`, `Alpha`, `Ascii` and `Base64`, and rejected by `Digit`, `Xdigit`, `Slug` and `HexRgbColor`. Length-based rules reject it through their bound (`Size(min: 1)`).

## Error messages

Each constraint has a default message following the `Must be ...` pattern, with the constraint parameters rendered in place of `{expected}`:

```php
<?php
// ['age' => ['Must be between 18 and 99']]
// ['username' => ["Must match the pattern '/^[a-z][a-z0-9-]*$/'"]]
// ['size' => ['Must have a size of at least 3']
// ['tag' => ["Must be one of: 'a', 'b', true"]]
```

A constraint error of one property can be overridden with `#[ErrorMessage]` under its catalog key — e.g. `#[ErrorMessage(min: 'At least {expected}')]` — see [per-property overrides](/guides/error-messages/#per-property-overrides-errormessage). Constraints applied through userland classes can return any message they want (see below).

Every default message has a stable translation key (`between`, `min`, `regex`, ...) — see the [error catalog](/guides/error-messages/#default-error-messages) and [translating error messages](/guides/error-messages/). Userland `message()` output is used verbatim and is not passed through the translator; translate inside `message()` yourself.

## Custom constraints

Write a constraint in PHP userland by implementing `Attributes\Strictify\Fields\Constraint`. Any attribute class implementing it is applied during `validate()` exactly like the built-ins — the same run order, error collection and `stopFirstError` handling. PHP enforces the interface methods and their signatures at class declaration time.

```php
<?php
use Attributes\Strictify\BaseModel;
use Attributes\Strictify\Fields\Constraint;

#[Attribute(Attribute::TARGET_PROPERTY | Attribute::TARGET_PARAMETER)]
final class Even implements Constraint
{
    public function validate(mixed $value): bool
    {
        return is_int($value) && $value % 2 === 0;
    }

    public function message(): string
    {
        return 'Must be an even number';
    }
}

class Product extends BaseModel
{
    #[Even]
    public int $quantity;
}

validate(['quantity' => 3], new Product());
// ValidationException: ['quantity' => ['Must be an even number']]
```

The contract:

- `validate(mixed $value): bool` receives the value after the native type check and returns whether it satisfies the constraint.
- `message(): string` is called only on failure and returns the error message. It supports the `{field}` and `{value}` placeholders, substituted the same way as in built-in templates.
- Constructor arguments are passed through unchanged, named arguments included: `#[LessThan(bound: 10)]` reaches the constructor as a named parameter.
- Exceptions thrown inside `validate()` or `message()` propagate to the caller of `validate()`, like model hook errors.

The built-in constraint classes are final: custom constraints implement the `Constraint` interface rather than extending a built-in.

Unlike the built-ins, userland constraints may declare `Attribute::IS_REPEATABLE` and be declared multiple times on one property. Instances are compiled once per property and reused for every `validate()` call of the request.

Attribute classes that do not exist or do not implement `Constraint` are ignored, exactly like any other attribute the validation engine does not know — a property can carry unrelated documentation attributes without affecting validation.
