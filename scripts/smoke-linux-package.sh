#!/usr/bin/env bash
set -euo pipefail

appimage="$1"
if [[ ! -f "$appimage" ]]; then
    echo "AppImage does not exist: $appimage" >&2
    exit 1
fi
appimage="$(realpath "$appimage")"
work_dir="$(mktemp -d)"
weston_pid=
cleanup() {
    if [[ -n "$weston_pid" ]]; then
        kill "$weston_pid" 2>/dev/null || true
        wait "$weston_pid" 2>/dev/null || true
    fi
    rm -rf "$work_dir"
}
trap cleanup EXIT
cd "$work_dir"
env -u PARSINAMA_CATALOG_PATH APPIMAGE_EXTRACT_AND_RUN=1 \
    QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME= \
    QT_QUICK_BACKEND=software QT_QUICK_CONTROLS_STYLE=Basic \
    xvfb-run -a "$appimage" --smoke-test

runtime_dir="$work_dir/runtime"
mkdir -p "$runtime_dir"
chmod 700 "$runtime_dir"
XDG_RUNTIME_DIR="$runtime_dir" weston --backend=headless-backend.so \
    --socket=wayland-ci --idle-time=0 --log="$runtime_dir/weston.log" &
weston_pid=$!
for _ in {1..100}; do
    [[ -S "$runtime_dir/wayland-ci" ]] && break
    sleep 0.1
done
if [[ ! -S "$runtime_dir/wayland-ci" ]]; then
    cat "$runtime_dir/weston.log" >&2
    echo "Headless Wayland compositor did not start" >&2
    exit 1
fi
env -u PARSINAMA_CATALOG_PATH APPIMAGE_EXTRACT_AND_RUN=1 \
    XDG_RUNTIME_DIR="$runtime_dir" WAYLAND_DISPLAY=wayland-ci \
    QT_QPA_PLATFORM=wayland QT_QPA_PLATFORMTHEME= \
    QT_QUICK_BACKEND=software QT_QUICK_CONTROLS_STYLE=Basic \
    "$appimage" --smoke-test
