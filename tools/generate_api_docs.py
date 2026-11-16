#!/usr/bin/env python3
"""Generate docs/api.md from the PHP stubs in stubs/.

The stubs are the single source of truth for the extension's public API
(signatures are enforced by the engine via @generate-class-entries), so the
API reference is generated from them and can never drift from the code.

Run from the repository root:  python tools/generate_api_docs.py
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
STUBS = ROOT / "stubs"
OUT = ROOT / "docs" / "api.md"

FILE_ORDER = [
    "functions.stub.php",
    "base_model.stub.php",
    "model_configs.stub.php",
    "type.stub.php",
    "exceptions.stub.php",
]


def strip_comments(src: str) -> str:
    """Remove PHP comments while respecting single/double quoted strings."""
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c in ("'", '"'):
            j = i + 1
            while j < n and src[j] != c:
                j += 2 if src[j] == "\\" else 1
            out.append(src[i : j + 1])
            i = j + 1
        elif src.startswith("//", i) or (src.startswith("#", i) and not src.startswith("#[", i)):
            j = src.find("\n", i)
            i = n if j == -1 else j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            i = n if j == -1 else j + 2
            out.append(" ")
        else:
            out.append(c)
            i += 1
    return "".join(out)


def brace_match(src: str, open_idx: int) -> int:
    """Return the index of the closing brace matching src[open_idx] == '{'."""
    depth = 0
    i = open_idx
    while i < len(src):
        if src[i] == "{":
            depth += 1
        elif src[i] == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError("unbalanced braces")


TYPE_RE = re.compile(
    r"(?P<attrs>(?:#\[[^\]]*\]\s*)*)"
    r"(?P<mods>(?:(?:abstract|final|readonly)\s+)*)"
    r"(?P<kind>class|interface|enum)\s+"
    r"(?P<name>\w+)"
    r"(?P<backing>\s*:\s*[\w\\\\]+)?"
    r"(?:\s+extends\s+(?P<extends>[\w\\\\]+))?"
    r"(?:\s+implements\s+(?P<implements>[\w\\\\,\s]+?))?"
    r"\s*\{"
)
FUNC_RE = re.compile(
    r"(?P<attrs>(?:#\[[^\]]*\]\s*)*)function\s+(?P<name>\w+)\s*\((?P<params>[^)]*)\)\s*(?::\s*(?P<ret>[^{]+?))?\s*\{\s*\}"
)
METHOD_RE = re.compile(
    r"(?P<vis>public|protected|private)(?P<static>\s+static)?\s+function\s+"
    r"(?P<name>\w+)\s*\((?P<params>[^)]*)\)\s*(?::\s*(?P<ret>[^{;]+?))?\s*[;{]"
)
PROP_RE = re.compile(
    r"(?P<vis>public|protected|private)(?P<static>\s+static)?(?P<ro>\s+readonly)?\s+"
    r"(?P<type>[^$;\n{}]+?)\s+\$(?P<name>\w+)(?P<default>\s*=\s*[^;]+)?;"
)
CASE_RE = re.compile(r"case\s+(?P<name>\w+)\s*=\s*(?P<value>[^;]+);")
ATTR_RE = re.compile(r"#\[\\?Attribute(?:\([^\]]*\))?\]")


def parse_namespace(src: str):
    m = re.search(r"namespace\s+([\w\\\\]+)\s*\{", src)
    if not m:
        return None, ""
    open_idx = src.index("{", m.end() - 1)
    close_idx = brace_match(src, open_idx)
    return m.group(1), src[open_idx + 1 : close_idx]


def parse_body(body: str):
    """Split a namespace body into class/enum/interface decls and functions."""
    decls = []
    class_ranges = []
    pos = 0
    for m in TYPE_RE.finditer(body):
        # skip matches nested inside a previous declaration's body
        if m.start() < pos:
            continue
        open_idx = body.index("{", m.end() - 1)
        close_idx = brace_match(body, open_idx)
        decls.append(
            {
                "kind": m.group("kind"),
                "name": m.group("name"),
                "attrs": m.group("attrs"),
                "mods": m.group("mods").strip(),
                "backing": (m.group("backing") or "").strip().lstrip(":").strip(),
                "extends": (m.group("extends") or "").strip(),
                "implements": (m.group("implements") or "").strip(),
                "inner": body[open_idx + 1 : close_idx],
            }
        )
        class_ranges.append((m.start(), close_idx))
        pos = close_idx + 1
    functions = []
    for m in FUNC_RE.finditer(body):
        if any(a <= m.start() <= b for a, b in class_ranges):
            continue
        functions.append(
            {
                "name": m.group("name"),
                "params": " ".join(m.group("params").split()),
                "ret": (m.group("ret") or "").strip(),
                "attrs": m.group("attrs"),
            }
        )
    return decls, functions


def parse_class_inner(inner: str):
    methods, props, cases = [], [], []
    for m in METHOD_RE.finditer(inner):
        vis = m.group("vis")
        static = "static " if m.group("static") else ""
        ret = f": {m.group('ret')}" if m.group("ret") else ""
        params = " ".join(m.group("params").split())
        methods.append(
            f"`{vis} {static}function {m.group('name')}({params}){ret}`"
        )
    for m in PROP_RE.finditer(inner):
        if "function" in m.group("type"):
            continue
        vis = m.group("vis")
        static = " static" if m.group("static") else ""
        ro = " readonly" if m.group("ro") else ""
        default = (m.group("default") or "").strip().lstrip("=").strip()
        ptype = " ".join(m.group("type").split())
        props.append(
            (f"`{vis}{static}{ro} {ptype} ${m.group('name')}`", default)
        )
    for m in CASE_RE.finditer(inner):
        cases.append((f"`{m.group('name')}`", f"`{m.group('value').strip()}`"))
    return methods, props, cases


def main() -> int:
    files = FILE_ORDER + sorted(
        str(p.relative_to(STUBS))
        for p in STUBS.rglob("*.stub.php")
        if str(p.relative_to(STUBS)) not in FILE_ORDER
    )

    namespaces = {}  # ns -> {"functions": [...], "classes": [...]}
    order = []
    for rel in files:
        src = strip_comments((STUBS / rel).read_text())
        ns, body = parse_namespace(src)
        if ns is None:
            continue
        decls, functions = parse_body(body)
        entry = namespaces.setdefault(ns, {"functions": [], "classes": []})
        if ns not in order:
            order.append(ns)
        entry["functions"].extend(functions)
        entry["classes"].extend(decls)

    lines = [
        "---",
        'title: "API"',
        'description: "Generated API reference for the Strictify extension: functions, classes, attributes and enums, produced from stubs/*.stub.php."',
        "---",
        "",
        "# API reference",
        "",
        "> Auto-generated from `stubs/*.stub.php` by `tools/generate_api_docs.py` — do not edit by hand.",
        "> Regenerate with `python tools/generate_api_docs.py`.",
        "",
    ]

    for ns in order:
        entry = namespaces[ns]
        lines.append(f"## `{ns}`")
        lines.append("")
        if entry["functions"]:
            lines.append("### Functions")
            lines.append("")
            lines.append("| Function | Signature |")
            lines.append("| -------- | --------- |")
            for f in entry["functions"]:
                ret = f": {f['ret']}" if f["ret"] else ""
                lines.append(
                    f"| `{f['name']}()` | `function {f['name']}({f['params']}){ret}` |"
                )
            lines.append("")
        for decl in entry["classes"]:
            fqcn = f"{ns}\\{decl['name']}"
            is_attr = "yes" if ATTR_RE.search(decl["attrs"]) else "no"
            kind = decl["kind"]
            mods = (decl["mods"] + " " if decl["mods"] else "") + kind
            suffix = ""
            if decl["extends"]:
                suffix += f" extends `{decl['extends']}`"
            if decl["implements"]:
                suffix += f" implements {decl['implements']}"
            if decl["backing"]:
                suffix += f" (backed by `{decl['backing']}`)"
            lines.append(f"### `{fqcn}` — {mods}{suffix}")
            lines.append("")
            lines.append(f"- Attribute: {is_attr}")
            lines.append("")
            methods, props, cases = parse_class_inner(decl["inner"])
            if cases:
                lines.append("**Cases**")
                lines.append("")
                lines.append("| Case | Value |")
                lines.append("| ---- | ----- |")
                for name, value in cases:
                    lines.append(f"| {name} | {value} |")
                lines.append("")
            if props:
                lines.append("**Properties**")
                lines.append("")
                lines.append("| Property | Default |")
                lines.append("| -------- | ------- |")
                for sig, default in props:
                    lines.append(f"| {sig} | {f'`{default}`' if default else '—'} |")
                lines.append("")
            if methods:
                lines.append("**Methods**")
                lines.append("")
                lines.append("| Method |")
                lines.append("| ------ |")
                for sig in methods:
                    lines.append(f"| {sig} |")
                lines.append("")

    OUT.write_text("\n".join(lines) + "\n")
    print(f"wrote {OUT.relative_to(ROOT)} ({len(lines)} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
