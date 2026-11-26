---
title: "API"
description: "Generated API reference for the Strictify extension: functions, classes, attributes and enums, produced from stubs/*.stub.php."
---

# API reference

> Auto-generated from `stubs/*.stub.php` by `tools/generate_api_docs.py` — do not edit by hand.
> Regenerate with `python tools/generate_api_docs.py`.

## `Attributes\Strictify`

### Functions

| Function | Signature |
| -------- | --------- |
| `validate()` | `function validate(array $rawData, BaseModel $model, int $maxDepth = 512): BaseModel` |
| `call()` | `function call(string|callable $function, array $params, array|\ArrayAccess $dependencies = [], int $maxDepth = 512): mixed` |

### `Attributes\Strictify\BaseModel` — abstract class

- Attribute: no

**Properties**

| Property | Default |
| -------- | ------- |
| `protected array $errors` | — |

**Methods**

| Method |
| ------ |
| `protected function beforeValidation(array $rawData, ModelConfigs $configs): array` |
| `protected function afterValidation(array $rawData, ModelConfigs $configs): void` |
| `public function toDict(bool $includeSensitive = false, bool $useAlias = false): array` |

### `Attributes\Strictify\ModelConfigs` — abstract class

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `private ?string $aliasGenerator` | `null` |
| `private bool $strToLower` | `false` |
| `private bool $strToUpper` | `false` |
| `private bool $stripWhitespace` | `false` |
| `private ?string $extra` | `null` |
| `private bool $strict` | `true` |
| `private bool $stopAtFirstError` | `false` |

**Methods**

| Method |
| ------ |
| `public function getAliasGenerator(): ?string` |
| `public function isStrToLower(): bool` |
| `public function isStrToUpper(): bool` |
| `public function isStripWhitespace(): bool` |
| `public function getExtra(): ?string` |
| `public function isStrict(): bool` |
| `public function isStopAtFirstError(): bool` |

### `Attributes\Strictify\ErrorTranslator` — final class

- Attribute: no

**Properties**

| Property | Default |
| -------- | ------- |
| `private static ?callable $translator` | `null` |

**Methods**

| Method |
| ------ |
| `public static function get(): ?callable` |
| `public static function set(callable $translator): void` |

### `Attributes\Strictify\Type` — enum (backed by `int`)

- Attribute: no

**Cases**

| Case | Value |
| ---- | ----- |
| `bool` | `12` |
| `int` | `16` |
| `float` | `32` |
| `string` | `64` |
| `null` | `2` |
| `sequence` | `128` |
| `dict` | `268435456` |
| `object` | `256` |

## `Attributes\Strictify\Exceptions`

### `Attributes\Strictify\Exceptions\StrictifyThrowable` — interface extends `\Throwable`

- Attribute: no

### `Attributes\Strictify\Exceptions\ValidationException` — class extends `\Exception` implements StrictifyThrowable

- Attribute: no

**Methods**

| Method |
| ------ |
| `public function __construct(array $allErrors)` |
| `public function getErrors(): array` |

### `Attributes\Strictify\Exceptions\InvalidBaseModelException` — class extends `\Error` implements StrictifyThrowable

- Attribute: no

### `Attributes\Strictify\Exceptions\MaxDepthException` — class extends `\Error` implements StrictifyThrowable

- Attribute: no

### `Attributes\Strictify\Exceptions\CallException` — class extends `\ArgumentCountError` implements StrictifyThrowable

- Attribute: no

## `Attributes\Strictify\Fields`

### `Attributes\Strictify\Fields\Alias` — readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public string $name` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string $name)` |

### `Attributes\Strictify\Fields\ConfigAwareConstraint` — interface extends `Constraint`

- Attribute: no

**Methods**

| Method |
| ------ |
| `public function setModelConfigs(\Attributes\Strictify\ModelConfigs $configs): void` |

### `Attributes\Strictify\Fields\Constraint` — interface extends `Field`

- Attribute: no

**Methods**

| Method |
| ------ |
| `public function validate(mixed $value): bool` |
| `public function message(): string` |

### `Attributes\Strictify\Fields\Dict` — readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public int|\Attributes\Strictify\Type|Union $key` | — |
| `public int|string|\Attributes\Strictify\Type|Union|Intersection|Sequence|Dict $of` | — |
| `public bool $uniqueValues` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(int|\Attributes\Strictify\Type|Union $key, int|string|\Attributes\Strictify\Type|Union|Intersection|Sequence|Dict $of, bool $uniqueValues = false)` |

### `Attributes\Strictify\Fields\ErrorMessage` — readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public string $required` | `'Field is required'` |
| `public string $type` | `'Must be {expected}'` |
| `public string $alphaDash` | `'Must only contain letters, numbers, dashes and underscores'` |
| `public string $alphaDashType` | `'Must be a string containing only letters, numbers, dashes and underscores'` |
| `public string $ascii` | `'Must only contain ASCII characters'` |
| `public string $asciiType` | `'Must be a string containing only ASCII characters'` |
| `public string $between` | `'Must be between {expected}'` |
| `public string $betweenType` | `'Must be a number between {expected}'` |
| `public string $contains` | `'Must contain {expected}'` |
| `public string $datetime` | `'Must be a datetime in the format {expected}'` |
| `public string $endsWith` | `'Must end with one of: {expected}'` |
| `public string $in` | `'Must be one of: {expected}'` |
| `public string $max` | `'Must be at most {expected}'` |
| `public string $min` | `'Must be at least {expected}'` |
| `public string $multipleOf` | `'Must be a multiple of {expected}'` |
| `public string $regex` | `'Must match the pattern {expected}'` |
| `public string $size` | `'Must have a size {expected}'` |
| `public string $startsWith` | `'Must start with one of: {expected}'` |
| `public string $ulid` | `'Must be a valid ULID'` |
| `public string $uuid` | `'Must be a valid UUID'` |
| `public string $email` | `'Must be a valid email address'` |
| `public string $emailDomain` | `'Must be an email address on an allowed domain: {expected}'` |
| `public string $url` | `'Must be a valid URL'` |
| `public string $urlProtocol` | `'Must be a URL using an allowed protocol: {expected}'` |
| `public string $urlDomain` | `'Must be a URL on an allowed domain: {expected}'` |
| `public string $domain` | `'Must be a valid domain name'` |
| `public string $ip` | `'Must be a valid IP address'` |
| `public string $iban` | `'Must be a valid IBAN'` |
| `public string $isbn` | `'Must be a valid ISBN'` |
| `public string $creditCard` | `'Must be a valid credit card number'` |
| `public string $version` | `'Must be a valid version string'` |
| `public string $macAddress` | `'Must be a valid MAC address'` |
| `public string $alnum` | `'Must contain only letters and numbers'` |
| `public string $alpha` | `'Must contain only letters'` |
| `public string $digit` | `'Must contain only digits'` |
| `public string $xdigit` | `'Must contain only hexadecimal digits'` |
| `public string $slug` | `'Must be a valid slug'` |
| `public string $base64` | `'Must be valid base64'` |
| `public string $json` | `'Must be valid JSON'` |
| `public string $hexRgbColor` | `'Must be a valid hex RGB color'` |
| `public string $even` | `'Must be an even number'` |
| `public string $odd` | `'Must be an odd number'` |
| `public string $positive` | `'Must be a positive number'` |
| `public string $negative` | `'Must be a negative number'` |
| `public string $primeNumber` | `'Must be a prime number'` |
| `public string $containsAny` | `'Must contain at least one of: {expected}'` |
| `public string $union` | `'Must be {expected}'` |
| `public string $sequenceKeys` | `'Must be a sequential list (keys 0, 1, 2, ...)'` |
| `public string $uniqueValues` | `'Must contain only unique values'` |

### `Attributes\Strictify\Fields\Field` — interface

- Attribute: no

### `Attributes\Strictify\Fields\Intersection` — readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public array $of` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string ...$of)` |

### `Attributes\Strictify\Fields\Sensitive` — readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Sequence` — readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict $of` | — |
| `public bool $uniqueValues` | — |
| `public bool $sequential` | — |
| `public ?string $sorted` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict $of, bool $uniqueValues = false, bool $sequential = true, ?string $sorted = null)` |

### `Attributes\Strictify\Fields\Union` — readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public array $of` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(int|string|\Attributes\Strictify\Type|Sequence|Union|Intersection|Dict ...$of)` |

## `Attributes\Strictify\Fields\Constraints`

### `Attributes\Strictify\Fields\Constraints\Alnum` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public string $extra` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string $extra = '')` |

### `Attributes\Strictify\Fields\Constraints\Alpha` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public string $extra` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string $extra = '')` |

### `Attributes\Strictify\Fields\Constraints\AlphaDash` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Ascii` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Base64` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Between` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public int|float $min` | — |
| `public int|float $max` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(int|float $min, int|float $max)` |

### `Attributes\Strictify\Fields\Constraints\Contains` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public array $values` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string|int|float ...$values)` |

### `Attributes\Strictify\Fields\Constraints\ContainsAny` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public array $values` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string|int|float ...$values)` |

### `Attributes\Strictify\Fields\Constraints\CreditCard` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\DateTime` — final class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public readonly string $format` | — |
| `public readonly \DateTimeZone|string|null $timezone` | — |
| `private \DateTimeZone|string|null $timezone_obj` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string $format = 'X-m-d\\TH:i:sP', \DateTimeZone|string|null $timezone = null)` |

### `Attributes\Strictify\Fields\Constraints\Digit` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Domain` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Email` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public string|array|null $domain` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string|array|null $domain = null)` |

### `Attributes\Strictify\Fields\Constraints\EndsWith` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public array $values` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string|int|float ...$values)` |

### `Attributes\Strictify\Fields\Constraints\Even` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\HexRgbColor` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Iban` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\In` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public array $values` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string|int|float|bool ...$values)` |

### `Attributes\Strictify\Fields\Constraints\Ip` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public ?string $version` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(?string $version = null)` |

### `Attributes\Strictify\Fields\Constraints\Isbn` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Json` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\MacAddress` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Max` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public int|float $value` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(int|float $value)` |

### `Attributes\Strictify\Fields\Constraints\Min` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public int|float $value` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(int|float $value)` |

### `Attributes\Strictify\Fields\Constraints\MultipleOf` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public int|float $value` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(int|float $value)` |

### `Attributes\Strictify\Fields\Constraints\Negative` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Odd` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Positive` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\PrimeNumber` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Regex` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public string $regex` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string $regex)` |

### `Attributes\Strictify\Fields\Constraints\Size` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public ?int $min` | — |
| `public ?int $max` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(?int $min = null, ?int $max = null)` |

### `Attributes\Strictify\Fields\Constraints\Slug` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\StartsWith` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public array $values` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string|int|float ...$values)` |

### `Attributes\Strictify\Fields\Constraints\Ulid` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Url` — final readonly class implements Field

- Attribute: yes

**Properties**

| Property | Default |
| -------- | ------- |
| `public string|array|null $protocol` | — |
| `public string|array|null $domain` | — |

**Methods**

| Method |
| ------ |
| `public function __construct(string|array|null $protocol = null, string|array|null $domain = null)` |

### `Attributes\Strictify\Fields\Constraints\Uuid` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Version` — final readonly class implements Field

- Attribute: yes

### `Attributes\Strictify\Fields\Constraints\Xdigit` — final readonly class implements Field

- Attribute: yes

