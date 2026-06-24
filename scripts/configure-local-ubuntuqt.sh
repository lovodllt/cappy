#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${1:-build-ubuntuqt}"
venv_bin="${root_dir}/.venv-tools/bin"
sysroot_dir="${root_dir}/.local-tools/sysroot"
cmake_prefix_path="${sysroot_dir}/usr/lib/x86_64-linux-gnu/cmake"
runtime_lib_path="${sysroot_dir}/usr/lib/x86_64-linux-gnu"

PATH="${venv_bin}:${PATH}" \
LD_LIBRARY_PATH="${runtime_lib_path}:${LD_LIBRARY_PATH:-}" \
cmake -S "${root_dir}" -B "${root_dir}/${build_dir}" -G Ninja \
    -DCMAKE_PREFIX_PATH="${cmake_prefix_path}"

PATH="${venv_bin}:${PATH}" \
LD_LIBRARY_PATH="${runtime_lib_path}:${LD_LIBRARY_PATH:-}" \
cmake --build "${root_dir}/${build_dir}" -j2

echo "Build finished at ${root_dir}/${build_dir}"

