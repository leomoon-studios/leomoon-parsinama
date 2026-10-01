#!/bin/sh
set -eu

source_svg=$1
output_icns=$2
work_dir=$(mktemp -d)
trap 'rm -rf "$work_dir"' EXIT

qlmanage -t -s 1024 -o "$work_dir" "$source_svg" >/dev/null 2>&1
master_png="$work_dir/$(basename "$source_svg").png"
iconset="$work_dir/app-icon.iconset"
mkdir -p "$iconset"

for size in 16 32 128 256 512; do
    double_size=$((size * 2))
    sips -z "$size" "$size" "$master_png" --out "$iconset/icon_${size}x${size}.png" >/dev/null
    sips -z "$double_size" "$double_size" "$master_png" --out "$iconset/icon_${size}x${size}@2x.png" >/dev/null
done

iconutil -c icns "$iconset" -o "$output_icns"
