# AGENTS.md

Guidance for coding agents working in this repository. Project background, build
prerequisites and platform notes live in [DEVELOPMENT.md](DEVELOPMENT.md).

## Project shape

- `src/` — C extension sources, `include/` — public headers, `stubs/` — PHP stubs that generate arginfo.
- `tests/Integration/` — Pest suites; `tests/Docs/` — runs the `// doctest: run` blocks of `docs/`.
- `docs/` — Zensical site sources; `docs/api.md` is generated from `stubs/`.

## Commands

| Task | Command |
| --- | --- |
| Build the extension | `task build` |
| Pest suites + docs doctests | `task test:integration` |
| Lint C and PHP | `task test:lint` |
| Format C and PHP | `task format` |
| Regenerate arginfo from stubs | `task stubs` |
| Regenerate `docs/api.md` | `python tools/generate_api_docs.py` |

Keep the source lists of `config.m4` and `config.w32` in sync.

## Rules

- **Load the `/ponytail` skill once per session, before your first code change.**
  It stays active for every later change, so do not reload it. It enforces the
  laziest solution that works: reuse what already exists here, reach for stdlib
  and native features before new code, keep the diff shortest. It does not relax
  correctness, input validation, error handling or tests — a non-trivial change
  still leaves one runnable check behind.
- **`docs/changelog.md` style:** every sentence brief, one fact per sentence.
  Newest first, under `Added` / `Changed`. Where the prose alone does not show
  the change — a new argument, a new error key, a behavior break — add a short
  example directly under the entry. Changes that break existing code start with
  `**BC note:**`. Code blocks there are illustrative fragments, so they carry no
  `// doctest: run` marker.
