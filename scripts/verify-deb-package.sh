#!/usr/bin/env bash
set -euo pipefail

package_path="${1:-}"

if [[ -z "${package_path}" ]]; then
    echo "Usage: $0 <package.deb>" >&2
    exit 1
fi

if [[ ! -f "${package_path}" ]]; then
    echo "Package not found: ${package_path}" >&2
    exit 1
fi

echo "Inspecting package metadata..."
dpkg-deb -I "${package_path}"

echo
echo "Inspecting package contents..."
dpkg-deb -c "${package_path}"

if command -v lintian >/dev/null 2>&1; then
    echo
    echo "Running lintian..."
    lintian "${package_path}"
else
    echo
    echo "lintian not found in PATH, skipping lintian check."
fi
