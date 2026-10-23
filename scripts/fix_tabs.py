#!/usr/bin/env python3
"""Normalize whitespace in C/C++ sources:
   - leading runs of 4 spaces -> tabs
   - strip trailing whitespace (whitespace-only lines become empty)
   Line endings (LF or CRLF) are preserved as-is."""
import re
import sys
from pathlib import Path

EXTS    = {".cpp", ".hpp", ".h", ".c", ".cc", ".inl", ".mm"}
EXCLUDE = {"vendor"}                      # any path component with this name is skipped
LEAD    = re.compile(r"^[ \t]+")
TAB_WIDTH = 4

def normalize_lead(lead: str) -> str:
    col = 0
    for ch in lead:
        col = (col // TAB_WIDTH + 1) * TAB_WIDTH if ch == "\t" else col + 1
    return "\t" * (col // TAB_WIDTH) + " " * (col % TAB_WIDTH)

def fix_line(part: str) -> str:
    cr = "\r" if part.endswith("\r") else ""
    body = part[:-1] if cr else part
    body = body.rstrip(" \t")
    m = LEAD.match(body)
    if m:
        body = normalize_lead(m.group(0)) + body[m.end():]
    return body + cr

def process(path: Path, dry: bool) -> bool:
    try:
        with open(path, encoding="utf-8", newline="") as f:
            text = f.read()
    except UnicodeDecodeError:
        print(f"skipped (not UTF-8): {path}")
        return False
    new = "\n".join(fix_line(p) for p in text.split("\n"))
    if new == text:
        return False
    if not dry:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(new)
    print(("would fix: " if dry else "fixed: ") + str(path))
    return True

def main():
    args = sys.argv[1:]
    dry  = "--dry-run" in args
    roots = [a for a in args if a != "--dry-run"] or ["src"]
    count = 0
    for root in roots:
        for p in sorted(Path(root).rglob("*")):
            if p.is_file() and p.suffix in EXTS and not EXCLUDE & set(p.parts):
                count += process(p, dry)
    print(f"{count} file(s) {'would change' if dry else 'changed'}")

if __name__ == "__main__":
    main()
