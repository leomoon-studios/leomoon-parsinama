#!/usr/bin/env bash
set -euo pipefail

dmg="$1"
if [[ ! -f "$dmg" ]]; then
    echo "DMG does not exist: $dmg" >&2
    exit 1
fi
dmg="$(cd "$(dirname "$dmg")" && pwd)/$(basename "$dmg")"
mount_dir="$(mktemp -d)"
trap 'hdiutil detach "$mount_dir" -quiet || true; rmdir "$mount_dir" || true' EXIT
hdiutil attach "$dmg" -readonly -nobrowse -mountpoint "$mount_dir"
app="$mount_dir/LeoMoon ParsiNama.app"
for file in "$app/Contents/Resources/parsinama-catalog.sqlite" \
    "$app/Contents/Resources/app-icon.icns" \
    "$app/Contents/PlugIns/sqldrivers/libqsqlite.dylib"; do
    if [[ ! -f "$file" ]]; then
        echo "Missing packaged file: $file" >&2
        exit 1
    fi
done
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"; hdiutil detach "$mount_dir" -quiet || true; rmdir "$mount_dir" || true' EXIT
cd "$work_dir"
if ! env -u PARSINAMA_CATALOG_PATH "$app/Contents/MacOS/LeoMoon ParsiNama" --smoke-test > "$work_dir/app.log" 2>&1; then
    cat "$work_dir/app.log" >&2
    echo "Packaged app smoke test failed" >&2
    exit 1
fi
