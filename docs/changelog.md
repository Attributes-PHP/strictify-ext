---
title: "Changelog"
description: "Release notes for Strictify: new features, fixes and behavior changes, newest first."
---

# Changelog

All notable changes to Strictify are documented here, newest first. Install a released version with `pie install attributes-php/strictify`.

## Unreleased

### Added

- `BaseModel::toDict()` gains a `useAlias` parameter: `toDict(bool $includeSensitive = false, bool $useAlias = false): array`. The output is keyed by the raw PHP property names by default; pass `useAlias: true` to key it by the resolved field names (`#[Alias]` / `aliasGenerator`, including each nested model's own generator), which round-trips through `validate()`.

### Changed

- **BC note:** the `Attributes\Strictify` type constants are renamed with a `T_` prefix so they no longer collide with PHP's reserved type-hint keywords: `Attributes\Strictify\T_BOOL`, `T_INT`, `T_FLOAT`, `T_STRING`, `T_SEQUENCE`, `T_DICT`, `T_OBJECT` (previously `bool`, `int`, `float`, `string`, `sequence`, `dict`, `object`). Update `use const` imports and attribute arguments accordingly, e.g. `#[Union(T_INT, T_STRING)]` instead of `#[Union(int, string)]`.
- **BC note:** the default error-message translator moved off `ModelConfigs` onto the new final class `Attributes\Strictify\ErrorTranslator`: `ErrorTranslator::set(callable $translator): void` and `ErrorTranslator::get(): ?callable`. `ModelConfigs::setDefaultErrorMessages()` / `ModelConfigs::getDefaultErrorMessages()` and the static `$defaultErrorMessages` property were removed.
- **BC note:** plain `toDict()` on models that use `#[Alias]` or `aliasGenerator` now returns property-name keys instead of alias keys. Pass `useAlias: true` for the previous behavior.
- **BC note:** `#[ModelConfigs]` is now inherited **only via the class hierarchy** (parent classes). A nested model (a `BaseModel` property of another model) no longer inherits the enclosing model's configs — `aliasGenerator`, `strict`, `extra`, the string normalizations and `stopAtFirstError` each resolve per model class. This changes the expected input keys of nested models (raw property names unless the nested class declares its own generator) and the nested keys in `toDict(useAlias: true)` output.

## Recent history

- Mask `#[Sensitive]` properties as `***` in debug output (`var_dump()`, `print_r()`, `debug_zval_dump()`, and models inside exception traces); property access, `json_encode()`, `serialize()` etc. keep the real values.
- Restore string concatenation in `CallFunctionHooksUser::afterValidation`.
- `call()` respects `#[\SensitiveParameter]` on target parameters: missing dependencies are reported positionally and stack traces redact the argument.
- Add `#[Sensitive]` field attribute: `toDict()` omits it unless `includeSensitive: true`; the `{value}` error placeholder is redacted to `***`.
- `ModelConfigs::getDefaultErrorMessages()` static getter; `ModelConfigs` `extra` defaults to `null`, resolved per class; `'allow'` stores unknown keys as dynamic properties.
- Resolve a `null` `ModelConfigs` `extra` per nested model class at validation time.

See the [GitHub releases and pull requests](https://github.com/Attributes-PHP/strictify-ext/pulls?q=is%3Apr+is%3Amered) for the full history.
