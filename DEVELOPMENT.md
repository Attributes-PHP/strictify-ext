# Development

## Dependencies

- PHP 8.3+
- [Composer](https://getcomposer.org)
- [Task](https://taskfile.dev)
- [Reflex](https://github.com/cespare/reflex)

## Building on Windows

`config.w32` mirrors `config.m4` — keep both source lists in sync. Windows builds need the Visual Studio C++ workload and [php-sdk-binary-tools](https://github.com/php/php-sdk-binary-tools); in a `phpsdk-vs17-x64.bat` shell:

```bat
phpize
configure --enable-attributes-strictify
nmake
```

The DLL lands in `x64\Release_TS\php_attributes_strictify.dll` (or `x64\Release\` for NTS). Run the PHP suites against it with `php -d extension=php_attributes_strictify.dll vendor/bin/pest tests/Integration tests/Docs`, or set `STRICTIFY_EXTENSION` to the DLL path so `tests/Docs` picks it up. The C tooling (`task test:format:c`, `task test:lint:c`, `task test:unit`) is Unix-only and stays in the Linux CI job.

## Documentation

The docs under `docs/` are built with [Zensical](https://zensical.org) (`task` has no docs task; run `pip install zensical && zensical serve`).

`docs/api.md` is generated from the PHP stubs in `stubs/` — after changing a stub, regenerate it with:

```bash
python tools/generate_api_docs.py
```

Code blocks in `docs/quickstart.md` and `docs/guides/*.md` marked with `// doctest: run` (or `// doctest: expect-throw`) are executed by `tests/Docs/DocExamplesTest.php` as part of `task test:integration`, so keep examples self-contained and runnable.
