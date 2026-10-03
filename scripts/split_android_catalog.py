#!/usr/bin/env python3
"""Split a large SQLite catalog into Android asset files Gradle can handle."""

import argparse
import hashlib
from pathlib import Path

CHUNK_BYTES = 128 * 1024 * 1024


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("catalog", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    source = args.catalog.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    stamp = output / ".source-stamp"
    stat = source.stat()
    signature = f"{source}\n{stat.st_size}\n{stat.st_mtime_ns}\n"
    if stamp.exists() and stamp.read_text() == signature and (output / "parsinama-catalog.manifest").exists():
        return

    for stale in output.glob("parsinama-catalog-*.part"):
        stale.unlink()
    digest = hashlib.sha256()
    count = 0
    with source.open("rb") as catalog:
        while chunk := catalog.read(CHUNK_BYTES):
            digest.update(chunk)
            (output / f"parsinama-catalog-{count:04d}.part").write_bytes(chunk)
            count += 1
    if count == 0:
        raise SystemExit("Catalog is empty")
    (output / "parsinama-catalog.manifest").write_text(
        f"{digest.hexdigest()} {stat.st_size} {count}\n", encoding="ascii"
    )
    stamp.write_text(signature)
    print(f"Split {stat.st_size:,} bytes into {count} Android assets")


if __name__ == "__main__":
    main()
