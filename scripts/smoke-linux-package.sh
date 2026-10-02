#!/usr/bin/env bash
set -euo pipefail

appimage="$1"
if [[ ! -f "$appimage" ]]; then
    echo "AppImage does not exist: $appimage" >&2
    exit 1
fi
appimage="$(realpath "$appimage")"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT
cd "$work_dir"
env -u PARSINAMA_CATALOG_PATH APPIMAGE_EXTRACT_AND_RUN=1 \
    QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME= \
    QT_QUICK_BACKEND=software QT_QUICK_CONTROLS_STYLE=Basic \
    xvfb-run -a "$appimage" --smoke-test
