---
title: "FAQ"
description: "Frequently asked questions about Strictify: strict vs loose mode, extra keys, error paths, property hooks, performance and caching."
---

# FAQ

## Why is my numeric string rejected?

Validation is **strict by default**: a value must already have the declared type, so `validate(['age' => '30'], ...)` on an `int $age` property fails with `Must be integer`. Declare `#[ModelConfigs(strict: false)]` on the model to enable loose-mode coercion. See [Strict and loose mode](/guides/validating-payloads/#strict-and-loose-mode).

## I get "Extra inputs are not permitted" — what does that mean?

Your input contains a key that resolves to no property of the model. Control this with the `extra` option of [`#[ModelConfigs]`](/model-configs/#extra): `'ignore'` (default) silently drops unknown keys, `'allow'` stores them as dynamic properties, `'forbid'` reports them as errors.

## Why do error keys look like `address.street` or `users.0.email`?

`ValidationException::getErrors()` keys errors by dot-notation field path, so nested models and array elements report exactly where the failure happened. Paths use the resolved field name — the [alias](/guides/aliases/) when one is declared.

## My payload uses `snake_case` but my properties are `camelCase`

Use `#[ModelConfigs(aliasGenerator: 'snake')]` on the model, or `#[Alias('...')]` on individual properties. See [Aliases and aliasGenerator](/guides/aliases/).

## Does toDict() output work as input to validate() again?

Yes, with `useAlias: true`: `toDict()` is keyed by raw property names by default, and by the resolved alias names with `useAlias: true`, which is what `validate()` expects. See [Serializing models](/guides/serializing-models/).

## Do property hooks work?

Yes — on PHP 8.4+, hooked (including virtual) properties run their `get` hook when hydrated by `validate()` and when read by `toDict()`. On older versions, plain properties are used.

## Is validation slow on the second call?

No. The model class is compiled once per request (field names, constraints, type arms are resolved and cached), so steady-state validation is a flat iteration over the property slots.

## Can I use Strictify without Composer/PIE?

The extension is distributed through PIE (`pie install attributes-php/strictify`). Building from source: `phpize && ./configure --enable-attributes-strictify && make`, then load `modules/attributes_strictify.so`. See [DEVELOPMENT.md](https://github.com/Attributes-PHP/strictify-ext/blob/main/DEVELOPMENT.md).

## Where do I report bugs or request features?

Open an issue on [GitHub](https://github.com/Attributes-PHP/strictify-ext/issues).
