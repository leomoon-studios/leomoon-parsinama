#!/usr/bin/env bash
set -euo pipefail

if (( $# != 4 )); then
    echo "Usage: $0 ARTIFACT_DIRECTORY OUTPUT_DIRECTORY VERSION COMMIT_SHA" >&2
    exit 2
fi

artifact_root="$1"
output_dir="$2"
version="$3"
commit_sha="$4"

[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || { echo "Invalid version: $version" >&2; exit 1; }
[[ "$commit_sha" =~ ^[0-9a-f]{40}$ ]] || { echo "Invalid commit SHA: $commit_sha" >&2; exit 1; }
[[ -d "$artifact_root" && ! -e "$output_dir" ]] || { echo "Missing artifacts or output already exists" >&2; exit 1; }

artifact_names=(linux-x64 linux-arm64 windows-x64 windows-arm64 macos)
asset_names=(
    "leomoon-parsinama-$version-linux-x86_64.AppImage"
    "leomoon-parsinama-$version-linux-aarch64.AppImage"
    "leomoon-parsinama-$version-windows-x64-setup.exe"
    "leomoon-parsinama-$version-windows-arm64-setup.exe"
    "leomoon-parsinama-$version-macos-universal.dmg"
)

mkdir "$output_dir"
for index in "${!artifact_names[@]}"; do
    artifact_dir="$artifact_root/leomoon-parsinama-${artifact_names[$index]}-$commit_sha"
    asset_name="${asset_names[$index]}"
    [[ -f "$artifact_dir/$asset_name" && -f "$artifact_dir/SHA256SUMS" ]] || {
        echo "Missing release asset or checksum: $artifact_dir/$asset_name" >&2
        exit 1
    }
    (cd "$artifact_dir" && sha256sum --check --status SHA256SUMS)
    cp "$artifact_dir/$asset_name" "$output_dir/"
done

signed_dir="$artifact_root/leomoon-parsinama-android-arm64-signed-$commit_sha"
if [[ -d "$signed_dir" ]]; then
    android_dir="$signed_dir"
    android_asset="leomoon-parsinama-$version-android-arm64-v8a.apk"
else
    android_dir="$artifact_root/leomoon-parsinama-android-arm64-$commit_sha"
    android_asset="leomoon-parsinama-$version-android-arm64-v8a.apk"
fi
[[ -f "$android_dir/$android_asset" && -f "$android_dir/SHA256SUMS" ]] || {
    echo "Missing Android release asset or checksum: $android_dir/$android_asset" >&2
    exit 1
}
(cd "$android_dir" && sha256sum --check --status SHA256SUMS)
cp "$android_dir/$android_asset" "$output_dir/"

(cd "$output_dir" && sha256sum ./* > SHA256SUMS)
echo "Collected six verified platform packages and SHA256SUMS in $output_dir"
