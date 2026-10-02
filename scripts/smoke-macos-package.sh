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
hdiutil attach "$dmg" -readonly -nobrowse -mountpoint "$mount_dir" -quiet
app="$mount_dir/LeoMoon ParsiNama.app"
test -f "$app/Contents/Resources/parsinama-catalog.sqlite"
test -f "$app/Contents/Resources/app-icon.icns"
test -f "$app/Contents/PlugIns/sqldrivers/libqsqlite.dylib"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"; hdiutil detach "$mount_dir" -quiet || true; rmdir "$mount_dir" || true' EXIT
cd "$work_dir"
env -u PARSINAMA_CATALOG_PATH "$app/Contents/MacOS/LeoMoon ParsiNama" --smoke-test
