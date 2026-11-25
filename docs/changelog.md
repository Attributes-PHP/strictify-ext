---
title: "Changelog"
description: "Release notes for Strictify: new features, fixes and behavior changes, newest first."
---

# Changelog

All notable changes to Strictify are documented here, newest first. Install the pure-PHP library with `composer require attributes-php/strictify`, or the extension with `pie install attributes-php/strictify-ext`.

## Unreleased

### Added

- New `Type::null` case / `T_NULL` constant (`MAY_BE_NULL`), usable as a `Union` arm — required when a nullable hint (e.g. `?string`) declares `#[Union(...)]` — and as a `Dict` key spec. `Dict` key specs now accept every PHP-castable key type (integer, string, float, boolean, null): `#[Dict(key: T_INT | T_STRING | T_NULL, of: ...)]`. Per [language.types.array](https://www.php.net/manual/en/language.types.array.php) the engine casts float/bool keys to int and null to `""`, so those arms match the post-cast key (`[2.0 => $v]` passes a `T_FLOAT` key spec as key `2`, `[null => $v]` passes a `T_NULL` spec as key `""`).
- `BaseModel::toDict()` gains a `useAlias` parameter: `toDict(bool $includeSensitive = false, bool $useAlias = false): array`. The output is keyed by the raw PHP property names by default; pass `useAlias: true` to key it by the resolved field names (`#[Alias]` / `aliasGenerator`, including each nested model's own generator), which round-trips through `validate()`.
- `#[ErrorMessage]` now accepts every key from the default error catalog as a named argument — `required`, `type`, `alpha_dash`, `between`, `min`, `max`, `sequence_keys`, ... — each defaulting to that key's default message, so any per-property template (constraint errors included) can be overridden. The `extra` key is discarded: extra inputs have no owning property and keep using the translator or the default message.

### Changed

- **BC note:** the `Attributes\Strictify` type constants are renamed with a `T_` prefix so they no longer collide with PHP's reserved type-hint keywords: `Attributes\Strictify\T_BOOL`, `T_INT`, `T_FLOAT`, `T_STRING`, `T_SEQUENCE`, `T_DICT`, `T_OBJECT` (previously `bool`, `int`, `float`, `string`, `sequence`, `dict`, `object`). Update `use const` imports and attribute arguments accordingly, e.g. `#[Union(T_INT, T_STRING)]` instead of `#[Union(int, string)]`.
- **BC note:** the `ErrorTranslator::set()` translator is now called as `fn (?string $key, string $message): ?string` — the stable key (`null` for custom `#[ErrorMessage]` values, whose text now arrives as `$message`) plus the default message template. Declare the first parameter nullable.
- **BC note:** `#[Union(...)]` declared directly on a union-typed property (e.g. `int|float`) must now contain the same type-hints as the native hint; a subset or an extra arm throws `InvalidBaseModelException` instead of narrowing/widening the accepted values. Nullable hints count `null` as a type-hint, so a `?string` property with `#[Union(...)]` must include `T_NULL` in the arms.
- **BC note:** the default error-message translator moved off `ModelConfigs` onto the new final class `Attributes\Strictify\ErrorTranslator`: `ErrorTranslator::set(callable $translator): void` and `ErrorTranslator::get(): ?callable`. `ModelConfigs::setDefaultErrorMessages()` / `ModelConfigs::getDefaultErrorMessages()` and the static `$defaultErrorMessages` property were removed.
- **BC note:** plain `toDict()` on models that use `#[Alias]` or `aliasGenerator` now returns property-name keys instead of alias keys. Pass `useAlias: true` for the previous behavior.
- **BC note:** `#[ModelConfigs]` is now inherited **only via the class hierarchy** (parent classes). A nested model (a `BaseModel` property of another model) no longer inherits the enclosing model's configs — `aliasGenerator`, `strict`, `extra`, the string normalizations and `stopAtFirstError` each resolve per model class. This changes the expected input keys of nested models (raw property names unless the nested class declares its own generator) and the nested keys in `toDict(useAlias: true)` output.
- **BC note:** `#[Sequence]` arrays now require **integer keys** — a string key reports `Must be integer` under that key's path — and are **sequential by default**: keys must be `0, 1, 2, ...` in order, otherwise a `sequence_keys` error is reported. Pass `sequential: false` to allow any integer keys (gaps included); string keys stay rejected.

## Recent history

- Mask `#[Sensitive]` properties as `***` in debug output (`var_dump()`, `print_r()`, `debug_zval_dump()`, and models inside exception traces); property access, `json_encode()`, `serialize()` etc. keep the real values.
- Restore string concatenation in `CallFunctionHooksUser::afterValidation`.
- `call()` respects `#[\SensitiveParameter]` on target parameters: missing dependencies are reported positionally and stack traces redact the argument.
- Add `#[Sensitive]` field attribute: `toDict()` omits it unless `includeSensitive: true`; the `{value}` error placeholder is redacted to `***`.
- `ModelConfigs::getDefaultErrorMessages()` static getter; `ModelConfigs` `extra` defaults to `null`, resolved per class; `'allow'` stores unknown keys as dynamic properties.
- Resolve a `null` `ModelConfigs` `extra` per nested model class at validation time.

See the [GitHub releases and pull requests](https://github.com/Attributes-PHP/strictify-ext/pulls?q=is%3Apr+is%3Amered) for the full history.
