---
title: "Benchmarks"
description: "How Strictify compares with other PHP validation frameworks: benchmark suite, methodology and measured results."
---

# Benchmarks

Strictify ships as a pure-PHP library and as a C extension with the same API; both flavors are measured below. The extension validates in C, so its per-call cost is close to the engine's own type checks. This page documents the benchmark suite and how Strictify compares against the popular pure-PHP validation libraries.

## Running the benchmarks

The suite uses [PHPBench](https://phpbench.readthedocs.io) and lives in `tests/Benchmark`:

```bash
composer install
task build        # phpize && ./configure --enable-attributes-strictify && make
task bench        # phpbench run — the strictify scenarios
```

Each scenario validates the same input arrays defined once in `tests/Benchmark/Scenarios.php`, so every implementation validates identical data. Validators/rules are built once per benchmark class, so subjects measure steady-state per-call cost, not one-off compilation.

The cross-framework comparison benches live in `tests/Benchmark/Frameworks` (Symfony Validator, Respect/Validation, Laravel's validator) and run with:

```bash
vendor/bin/phpbench run --group=frameworks
```

## Measured comparison

Environment: PHP 8.4.24 (NTS, no opcache/xdebug), single pinned core, min of 30x200 interleaved repetitions per subject. Strictify (ext) = the extension built from `main`; strictify (pure PHP) = [attributes-php/strictify](https://github.com/Attributes-PHP/strictify) `dev-main`, measured in a separate process (it redeclares the same API, so it cannot be autoloaded alongside the extension). Framework columns are ordered fastest-first by geomean. Framework versions are the latest stable releases at measurement time: **symfony/validator 8.1.11**, **illuminate/validation 13.43.0** (Laravel), **respect/validation 2.5.0** (3.x requires PHP >= 8.5). Times are per validation call.

| Scenario | Strictify (ext) | strictify (pure PHP) | respect/validation 2.5.0 | symfony/validator 8.1.11 | illuminate/validation 13.43.0 | Strictify vs slowest |
|---|---:|---:|---:|---:|---:|---:|
| SimpleModel (5 scalar fields) | 107ns | 2.01µs | 935ns | 10.62µs | 23.51µs | 221x |
| LooseModel (stringly scalars) | 106ns | 3.12µs | 936ns | 10.75µs | 23.83µs | 225x |
| ConstrainedModel (15 constraints) | 1.08µs | 11.99µs | 6.79µs | 37.32µs | 195.44µs | 181x |
| ConstrainedModelFailing (1 violation) | 1.65µs | 14.73µs | 891ns | 39.68µs | 209.73µs | 127x |
| NestedModel (10 nested models) | 117ns | 15.15µs | 4.57µs | 56.59µs | 169.82µs | 1455x |
| 32FieldModel | 449ns | 9.19µs | 8.98µs | 58.16µs | 123.90µs | 276x |
| NestedSequences (20x50 int matrix) | 3.17µs | 156.45µs | 71.70µs | 1320µs | 6401µs | 2019x |
| DictOfSequences (20 keyed lists) | 3.09µs | 155.41µs | 72.33µs | 1310µs | 6467µs | 2093x |
| UnionExactScalars (100 int/string/bool) | 101ns | 208ns | 8.25µs | 148.36µs | 630µs | 6231x |
| NineDistinctModels | 645ns | 12.56µs | 5.76µs | 69.31µs | 177.84µs | 276x |

Against the pure-PHP strictify library, the extension is on average **~12x faster** (geomean over these scenarios measured in one session; up to **96x** on nested payloads). Against the **fastest** framework per scenario (usually Respect/Validation), Strictify is on average **~12x faster** (geomean; 6x–82x per scenario). Against the second-fastest (Symfony Validator) it is on average **~165x faster** (24x–1467x). The gap grows with payload size — nested lists and unions — because the pure-PHP frameworks pay per-element interpreter overhead while Strictify walks the hash table in C.

One exception: on the *failing* constrained case Respect's `validate()` is ~2x faster than Strictify, because it just returns `false` while Strictify builds the error collection and renders message templates.

## Caveats

- Rules are mapped per framework idiom, not rule-for-rule identical: Symfony never coerces (the "loose" scenario uses union types there), while Laravel/Respect accept numeric strings natively; Laravel has no built-in union rule, so the union scenario uses a closure rule there.
- Absolute numbers vary with hardware and PHP version; the *ratios* are the stable signal. Re-run locally with the commands above to reproduce.
- Versions drift: newer framework majors measured slightly *slower* than the previous ones (symfony 7.4 / laravel 12.73 were ~5–10% faster than 8.1.11 / 13.43.0 in these scenarios).
