# Development

## Dependencies

- PHP 8.2+
- [Composer](https://getcomposer.org)
- [Task](https://taskfile.dev)
- [Reflex](https://github.com/cespare/reflex)

## Documentation

The docs under `docs/` are built with [Zensical](https://zensical.org) (`task` has no docs task; run `pip install zensical && zensical serve`).

`docs/api.md` is generated from the PHP stubs in `stubs/` — after changing a stub, regenerate it with:

```bash
python tools/generate_api_docs.py
```

Code blocks in `docs/quickstart.md` and `docs/guides/*.md` marked with `// doctest: run` (or `// doctest: expect-throw`) are executed by `tests/Docs/DocExamplesTest.php` as part of `task test:integration`, so keep examples self-contained and runnable.
