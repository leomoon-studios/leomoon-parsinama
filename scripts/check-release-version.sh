#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="$(tr -d '[:space:]' < "$repo_dir/metadata/VERSION")"
if [[ ! "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "metadata/VERSION must contain a semantic version" >&2
    exit 1
fi
if [[ $# -gt 0 && -n "$1" ]]; then
    if [[ "$1" != "v$version" ]]; then
        echo "Tag $1 does not match metadata/VERSION ($version)" >&2
        exit 1
    fi
fi
printf '%s\n' "$version"
