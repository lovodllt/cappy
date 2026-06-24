#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${1:-build-verify}"
report_path="${2:-}"

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy not found in PATH" >&2
    exit 1
fi

compile_commands="${root_dir}/${build_dir}/compile_commands.json"
if [[ ! -f "${compile_commands}" ]]; then
    echo "Missing compile_commands.json in ${build_dir}" >&2
    echo "Configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON" >&2
    exit 1
fi

mapfile -t files < <(
    find "${root_dir}/src" \
        \( -name '*.cpp' \) \
        -type f | sort
)

if [[ "${#files[@]}" -eq 0 ]]; then
    echo "No C++ source files found under src/"
    exit 0
fi

if [[ -n "${report_path}" ]]; then
    mkdir -p "$(dirname "${report_path}")"
    clang-tidy -p "${root_dir}/${build_dir}" "${files[@]}" | tee "${report_path}"
else
    clang-tidy -p "${root_dir}/${build_dir}" "${files[@]}"
fi
