---
title: "Changelog"
description: "Release notes for Strictify: new features, fixes and behavior changes, newest first."
---

# Changelog

All notable changes to Strictify are documented here, newest first. Install the pure-PHP library with `composer require attributes-php/strictify`, or the extension with `pie install attributes-php/strictify-ext`.

## Unreleased

### Added

- `CallException`: a new `BaseException` subclass. `call()` throws it when a required parameter has no dependency. The message is `No value provided for parameter $x of f().` It replaces the previous plain `Error`, so `catch (BaseException $e)` now covers the whole call path. Errors thrown inside the callable or inside a dependency container still propagate unwrapped.

  ```php
  function hello(string $name): string { return "Hi $name"; }

  try {
      call('hello', []);
  } catch (CallException $e) {
      // No value provided for parameter $name of hello().
  }
  ```

- New built-in constraints ported from Respect/Validation: `Email`, `Url`, `Domain`, `Ip` (optional `v4`/`v6`), `Iban`, `Isbn`, `CreditCard` (Luhn), `Version` (semver), `MacAddress`, `Alnum`/`Alpha` (optional extra characters), `Digit`, `Xdigit`, `Slug`, `Base64`, `Json`, `HexRgbColor`, `Even`, `Odd`, `Positive`, `Negative`, `PrimeNumber`, `ContainsAny`. Each has its own default message and translation key.

  ```php
  #[Email] public string $email;
  #[Ip(version: 'v4')] public string $ip;
  #[ContainsAny('admin', 'owner')] public array $roles;
  ```

- `#[Url(...)]` gains `$protocol` and `$domain` (`string|array|null`, default `null`). Both restrict the accepted URLs. Comparison is full and case-insensitive, with ports and userinfo ignored. `https:`, `https://` and `https` count as the same protocol. Rejections use two new keys: `urlProtocol` (`Must be a URL using an allowed protocol: {expected}`) and `urlDomain` (`Must be a URL on an allowed domain: {expected}`).

  ```php
  #[Url(protocol: 'https', domain: ['example.com', 'example.org'])]
  public string $homepage; // 'ftp://example.com' and 'https://evil.test' both fail
  ```

- `#[Email(...)]` gains `$domain` (`string|array|null`, default `null`). It restricts the part after the `@`, compared in full and case-insensitively. A rejection reports the new `emailDomain` key (`Must be an email address on an allowed domain: {expected}`).

  ```php
  #[Email(domain: 'example.com')] public string $email; // 'a@other.test' fails
  ```

- `#[Sequence(sorted: ...)]` gains `$sorted`. `'asc'`/`'ASC'` or `'desc'`/`'DESC'` sorts the validated values and renumbers the keys `0, 1, 2, ...`. The default `null` keeps the input order. It replaces the `Sorted` constraint, which is dropped.

  ```php
  #[Sequence(of: T_INT, sorted: 'asc')] public array $scores;
  // [3, 1, 2] -> [0 => 1, 1 => 2, 2 => 3]
  ```

- `#[Sequence(...)]` and `#[Dict(...)]` now enforce `$uniqueValues` (default `false`). With `uniqueValues: true`, a repeated element is rejected on the array property with the new `uniqueValues` key (`Must contain only unique values`). `Dict::__construct()` takes the flag as its third argument.

  ```php
  #[Sequence(of: T_STRING, uniqueValues: true)] public array $tags;
  // ['a', 'b'] passes, ['a', 'a'] fails
  ```

- New `Type::null` case and `T_NULL` constant (`MAY_BE_NULL`). It works as a `Union` arm, and is required when a nullable hint declares `#[Union(...)]`. It also works as a `Dict` key spec.
- `Dict` key specs now accept every PHP-castable key type: integer, string, float, boolean, null. Per [language.types.array](https://www.php.net/manual/en/language.types.array.php) the engine casts float and bool keys to int and null to `""`. Those arms therefore match the post-cast key.

  ```php
  #[Dict(key: T_INT | T_STRING | T_NULL, of: T_STRING)] public array $meta;
  // [2.0 => 'a'] becomes key 2, [null => 'a'] becomes key ""
  ```

- `BaseModel::toDict()` gains a `$useAlias` parameter: `toDict(bool $includeSensitive = false, bool $useAlias = false): array`. Output is keyed by the raw property names by default. Pass `useAlias: true` to key it by the resolved field names (`#[Alias]` and `aliasGenerator`, each nested model using its own generator). That output round-trips through `validate()`.

  ```php
  $user->toDict();               // ['name' => 'John']
  $user->toDict(useAlias: true); // ['user_name' => 'John']
  ```

- `#[ErrorMessage]` accepts every key of the default error catalog as a named argument: `required`, `type`, `alphaDash`, `between`, `min`, `max`, `sequenceKeys`, and so on. Each defaults to that key's message, so any per-property template can be overridden, constraint errors included. The `extra` key is discarded: extra inputs have no owning property and keep the translator or default message.

  ```php
  #[ErrorMessage(required: 'Name is mandatory', min: 'At least {expected}')]
  public string $name;
  ```

- Constraint attribute arguments are validated once per field compile. An argument that makes a constraint unsatisfiable or a no-op throws an `InvalidBaseModelException` naming the attribute, the model and the property. Covered: inverted or `NAN` numeric bounds, a negative or inverted `Size` range, an empty `In`/`StartsWith`/`EndsWith` list, an empty prefix or suffix, an empty `Email`/`Url` domain or protocol restriction, and a `Regex` pattern that does not compile (previously a PHP warning on every call).

  ```php
  #[Between(10, 5)] public int $age; // InvalidBaseModelException: inverted bounds
  ```

- Two alias mistakes now throw `InvalidBaseModelException`: an empty `#[Alias('')]`, and two properties resolving to the same input key. The second covers `#[Alias('id')]` on two properties, or a generator collapsing `$aB` and `$a_b` onto `a_b`. Each used to produce a field that could never be satisfied, or two fields that silently shadowed each other.

  ```php
  #[Alias('id')] public int $a;
  #[Alias('id')] public int $b; // InvalidBaseModelException
  ```

- New nesting depth guard. A payload nesting models more than 512 levels deep throws a catchable `MaxDepthException` (a new `BaseException` subclass) instead of exhausting the C stack. Siblings do not add depth. The counter unwinds on every exit path, including a failing deep run.
- `validate()` and `call()` accept an `int $maxDepth` argument (default `512`) to override the nesting limit for one run. Raise it for legitimately deep payloads, or lower it to reject deep nesting earlier. A value below 1 throws a `ValueError`. The exception message reports the limit that fired.

  ```php
  validate($payload, new Tree(), maxDepth: 32);
  ```

### Changed

- **BC note:** error catalog keys are camelCase everywhere: the `#[ErrorMessage]` arguments and properties, and the key handed to `ErrorTranslator::set()` (`alphaDash`, `sequenceKeys`, `urlProtocol`, `uniqueValues`, ...). A translator registered under a snake_case key (`'sequence_keys'`) no longer matches.

  ```php
  ErrorTranslator::set(fn (?string $key, string $message): ?string => $key === 'sequenceKeys' ? 'Bad order' : null);
  ```

- **BC note:** a malformed model declaration now always throws `InvalidBaseModelException`, never a `ValueError` or `TypeError`. Covered: arguments rejected by a constructor signature (`#[Union(1.5)]`, `#[Min('fifteen')]`), unresolvable class arms, unsupported `Type` values, unsupported `Dict` key types, empty `Sequence`/`Dict` arguments, an invalid `Sequence(sorted:)`, a `DateTime` timezone argument, and the constraint argument rules above. Messages are unchanged; the rule just moved onto the extension's exception hierarchy, so `catch (BaseException $e)` covers every malformed model. `ValueError` and `TypeError` remain for call arguments (`$maxDepth` below 1, the translator, `call()` dependencies) and for constructing an attribute object directly in userland.

  ```php
  #[Min('fifteen')] public int $age; // InvalidBaseModelException, not TypeError
  ```

- **BC note:** the `Attributes\Strictify` type constants are renamed with a `T_` prefix, so they no longer collide with PHP's reserved type-hint keywords: `T_BOOL`, `T_INT`, `T_FLOAT`, `T_STRING`, `T_SEQUENCE`, `T_DICT`, `T_OBJECT` (previously `bool`, `int`, `float`, `string`, `sequence`, `dict`, `object`). Update `use const` imports and attribute arguments.

  ```php
  #[Union(T_INT, T_STRING)] // was #[Union(int, string)]
  ```

- **BC note:** the `ErrorTranslator::set()` translator is now called as `fn (?string $key, string $message): ?string`. The first parameter is the stable key, `null` for custom `#[ErrorMessage]` values (whose text arrives as `$message`). The second is the default message template. Declare the first parameter nullable.
- **BC note:** `#[Union(...)]` on a union-typed property (e.g. `int|float`) must now repeat the native hint's type-hints exactly. A subset or an extra arm throws `InvalidBaseModelException` instead of narrowing or widening the accepted values. A nullable hint counts `null` as a type-hint, so a `?string` property with `#[Union(...)]` must include `T_NULL`.

  ```php
  #[Union(T_STRING, T_NULL)] public ?string $name; // ok
  #[Union(T_STRING)] public ?string $name;         // InvalidBaseModelException
  ```

- **BC note:** the default error-message translator moved off `ModelConfigs` onto the new final class `Attributes\Strictify\ErrorTranslator`: `ErrorTranslator::set(callable $translator): void` and `ErrorTranslator::get(): ?callable`. `ModelConfigs::setDefaultErrorMessages()`, `ModelConfigs::getDefaultErrorMessages()` and the static `$defaultErrorMessages` property were removed.
- **BC note:** plain `toDict()` on models that use `#[Alias]` or `aliasGenerator` now returns property-name keys, not alias keys. Pass `useAlias: true` for the previous behavior.

  ```php
  $user->toDict();               // ['name' => 'John']
  $user->toDict(useAlias: true); // ['user_name' => 'John']
  ```

- **BC note:** `#[ModelConfigs]` is now inherited **only via the class hierarchy** (parent classes). A nested model no longer inherits the enclosing model's configs: `aliasGenerator`, `strict`, `extra`, the string normalizations and `stopAtFirstError` each resolve per model class. This changes the expected input keys of nested models, and their keys in `toDict(useAlias: true)` output.

  ```php
  #[ModelConfigs(aliasGenerator: 'snake')]
  class Order extends BaseModel
  {
      public Customer $customer; // Customer keeps raw names unless it declares its own generator
  }
  ```

- **BC note:** `#[Sequence]` arrays now require **integer keys**. A string key reports `Must be integer` under that key's path. They are also **sequential by default**: keys must run `0, 1, 2, ...` in order, otherwise a `sequenceKeys` error is reported. Pass `sequential: false` to allow any integer keys, gaps included. String keys stay rejected.

  ```php
  #[Sequence(of: T_STRING)] public array $tags;
  // ['a', 'b']             ok
  // [1 => 'b', 0 => 'a']  sequenceKeys error
  // ['x' => 'a']          Must be integer

  #[Sequence(of: T_STRING, sequential: false)] public array $sparse;
  // [1 => 'b', 5 => 'a'] ok
  ```

- `ModelConfigs` option values are type-checked instead of cast. A `bool` option accepts only `true`/`false`; a `?string` option accepts only a string (at most 127 chars) or `null`. So `strict: 1` or `strToLower: 'yes'` throws `InvalidBaseModelException` instead of flipping a flag.

  ```php
  #[ModelConfigs(strict: 1)] // InvalidBaseModelException
  ```

- `#[Sequence(sorted: ...)]` rejects a value outside `'asc'`/`'desc'`/`null` (case-insensitive) with an `InvalidBaseModelException`, instead of silently leaving the array unsorted.

  ```php
  #[Sequence(of: T_INT, sorted: 'up')] // InvalidBaseModelException
  ```

- `#[Sequence]`/`#[Dict]`/`#[Intersection]` on a non-array-like property now throws `InvalidBaseModelException`. That covers `string`, `int` and `object` hints, and an intersection type without `array`. Previously the rule missed those hints and the attribute was ignored.

  ```php
  #[Sequence(of: T_INT)] public string $ids; // InvalidBaseModelException
  ```

- Validator fixes:
  - `MultipleOf(0.0)` accepts `0.0`, not only the integer `0`.
  - `DateTime` rejects a date that does not exist (`1994-02-30`, `2023-02-29`) instead of accepting the rolled-over date.
  - `Iban` accepts either letter case.
  - `Ulid` requires a first character in `0`-`7` (the 128-bit range).
  - `Email` requires a top-level domain of at least two characters, like `Domain`.
  - A `Regex` pattern containing a NUL byte is stored and matched in full.
- Constraints are skipped for an explicit `null`, exactly like a missing key.

  ```php
  #[Regex('/^\w+$/')] public ?string $name = null;
  validate(['name' => null], new User()); // passes
  ```

- Error messages render floats at full round-trip precision (`0.30000000000000004`, not `0.3`).
- `call()` names the parameter of an internal function in its `No value provided` messages.

## Recent history

- `#[Sensitive]` properties are masked as `***` in debug output: `var_dump()`, `print_r()`, `debug_zval_dump()`, and models inside exception traces. Property access, `json_encode()`, `serialize()` etc. keep the real values.
- Restored string concatenation in `CallFunctionHooksUser::afterValidation`.
- `call()` respects `#[\SensitiveParameter]` on target parameters. Missing dependencies are reported positionally, and stack traces redact the argument.
- New `#[Sensitive]` field attribute. `toDict()` omits it unless `includeSensitive: true`. The `{value}` error placeholder is redacted to `***`.
- New `ModelConfigs::getDefaultErrorMessages()` static getter.
- `ModelConfigs` `extra` now defaults to `null` and resolves per class. `'allow'` stores unknown keys as dynamic properties.
- A `null` `ModelConfigs` `extra` now resolves per nested model class at validation time.

See the [GitHub releases and pull requests](https://github.com/Attributes-PHP/strictify-ext/pulls?q=is%3Apr+is%3Amered) for the full history.
