#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
source_root="$(cd "${1:-$root}" && pwd)"
output="$root/data-archive"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

cd "$source_root"
test -f data/manifest.json || { echo "Expected data/manifest.json under $source_root" >&2; exit 1; }
zip -qr -s 70m "$work/data.zip" data
python3 - "$work" "$output" <<'PY'
import hashlib
import json
import pathlib
import shutil
import sys

source, destination = map(pathlib.Path, sys.argv[1:])
parts = sorted(source.glob('data.z[0-9][0-9]')) + [source / 'data.zip']
manifest = []
for part in parts:
    digest = hashlib.file_digest(part.open('rb'), 'sha256').hexdigest()
    manifest.append({'name': part.name, 'size': part.stat().st_size, 'sha256': digest})
destination.mkdir(exist_ok=True)
for old in list(destination.glob('data.z[0-9][0-9]')) + [destination / 'data.zip']:
    old.unlink(missing_ok=True)
for part in parts:
    shutil.move(str(part), destination / part.name)
(destination / 'manifest.json').write_text(json.dumps({'parts': manifest}, indent=2) + '\n')
PY
