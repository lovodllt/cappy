#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:-build}"
stage_dir="${2:-${build_dir}/stage}"

cmake -S . -B "${build_dir}" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "${build_dir}"
cmake --install "${build_dir}" --prefix "${stage_dir}"
cpack --config "${build_dir}/CPackConfig.cmake" -G DEB

package_path="$(find . -maxdepth 1 -type f -name 'cappy_*.deb' | sort | tail -n 1)"
if [[ -n "${package_path}" ]]; then
    "${PWD}/scripts/verify-deb-package.sh" "${package_path}"
fi

echo "Staged files:"
find "${stage_dir}" -maxdepth 4 -type f | sort

echo "Generated packages:"
find "${build_dir}" -maxdepth 1 -type f \\( -name '*.deb' -o -name '*.tar.gz' \\) | sort
