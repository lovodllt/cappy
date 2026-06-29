#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:-build}"
stage_dir="${2:-${build_dir}/stage}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
venv_bin="${root_dir}/.venv-tools/bin"
sysroot_dir="${root_dir}/.local-tools/sysroot"
cmake_prefix_path="${sysroot_dir}/usr/lib/x86_64-linux-gnu/cmake"
runtime_lib_path="${sysroot_dir}/usr/lib/x86_64-linux-gnu"

cmake_args=(
    -S "${root_dir}"
    -B "${root_dir}/${build_dir}"
    -G Ninja
    -DCMAKE_BUILD_TYPE=Release
)

env_path="${PATH}"
if [[ -d "${venv_bin}" ]]; then
    env_path="${venv_bin}:${env_path}"
fi

env_ld_library_path="${LD_LIBRARY_PATH:-}"
if [[ -d "${cmake_prefix_path}" ]]; then
    cmake_args+=(-DCMAKE_PREFIX_PATH="${cmake_prefix_path}")
fi
if [[ -d "${runtime_lib_path}" ]]; then
    if [[ -n "${env_ld_library_path}" ]]; then
        env_ld_library_path="${runtime_lib_path}:${env_ld_library_path}"
    else
        env_ld_library_path="${runtime_lib_path}"
    fi
fi

PATH="${env_path}" LD_LIBRARY_PATH="${env_ld_library_path}" cmake "${cmake_args[@]}"
PATH="${env_path}" LD_LIBRARY_PATH="${env_ld_library_path}" cmake --build "${root_dir}/${build_dir}"
PATH="${env_path}" LD_LIBRARY_PATH="${env_ld_library_path}" cmake --install "${root_dir}/${build_dir}" --prefix "${root_dir}/${stage_dir}"
PATH="${env_path}" LD_LIBRARY_PATH="${env_ld_library_path}" cpack --config "${root_dir}/${build_dir}/CPackConfig.cmake" -G DEB

package_path="$(find . -maxdepth 1 -type f -name 'cappy_*.deb' | sort | tail -n 1)"
if [[ -n "${package_path}" ]]; then
    "${PWD}/scripts/verify-deb-package.sh" "${package_path}"
fi

echo "Staged files:"
find "${stage_dir}" -maxdepth 4 -type f | sort

echo "Generated packages:"
find "${build_dir}" -maxdepth 1 -type f \( -name '*.deb' -o -name '*.tar.gz' \) | sort
