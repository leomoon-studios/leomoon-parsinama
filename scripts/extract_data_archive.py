#!/usr/bin/env python3
"""Validate and extract the split catalog source archive."""
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
import zipfile


def main():
    archive = pathlib.Path(sys.argv[1]).resolve()
    destination = pathlib.Path(sys.argv[2]).resolve()
    parts = json.loads((archive / 'manifest.json').read_text(encoding='utf-8'))['parts']
    names = [part['name'] for part in parts]
    if not names or names[-1] != 'data.zip' or names[:-1] != [
        f'data.z{i:02d}' for i in range(1, len(names))
    ]:
        raise ValueError('Archive manifest must list consecutive split parts followed by data.zip')
    for part in parts:
        path = archive / part['name']
        if not path.is_file():
            raise ValueError(f'Missing archive part: {path}')
        with path.open('rb') as stream:
            digest = hashlib.sha256()
            for block in iter(lambda: stream.read(1024 * 1024), b''):
                digest.update(block)
        if path.stat().st_size != part['size'] or digest.hexdigest() != part['sha256']:
            raise ValueError(f'Archive part failed size or SHA-256 check: {path}')

    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='catalog-extract-', dir=destination.parent) as temp:
        temp = pathlib.Path(temp)
        merged = temp / 'data.zip'
        subprocess.run(['zip', '-q', '-s', '0', str(archive / 'data.zip'), '--out', str(merged)], check=True)
        extracted = temp / 'source'
        extracted.mkdir()
        with zipfile.ZipFile(merged) as z:
            for entry in z.infolist():
                path = pathlib.PurePosixPath(entry.filename)
                if path.is_absolute() or '..' in path.parts or not path.parts or path.parts[0] != 'data' or '\\' in entry.filename:
                    raise ValueError(f'Invalid archive path: {entry.filename}')
            z.extractall(extracted)
        if not (extracted / 'data' / 'manifest.json').is_file():
            raise ValueError('Archive does not contain data/manifest.json')
        if destination.exists():
            shutil.rmtree(destination)
        extracted.rename(destination)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError, subprocess.SubprocessError, zipfile.BadZipFile) as error:
        sys.exit(f'Catalog archive extraction failed: {error}')
