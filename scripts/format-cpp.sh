#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="${1:-write}"

case "${mode}" in
    write|check)
        ;;
    *)
        echo "Usage: $0 [write|check]" >&2
        exit 1
        ;;
esac

if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found in PATH" >&2
    exit 1
fi

mapfile -t files < <(
    find "${root_dir}/src" \
        \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
        -type f | sort
)

if [[ "${#files[@]}" -eq 0 ]]; then
    echo "No C++ files found under src/"
    exit 0
fi

case "${mode}" in
    write)
        clang-format -i "${files[@]}"
        echo "Formatted ${#files[@]} C++ files."
        ;;
    check)
        clang-format --dry-run --Werror "${files[@]}"
        echo "clang-format check passed for ${#files[@]} C++ files."
        ;;
esac
