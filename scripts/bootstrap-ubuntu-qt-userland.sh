#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
venv_dir="${root_dir}/.venv-tools"
deb_dir="${root_dir}/.local-tools/debs"
sysroot_dir="${root_dir}/.local-tools/sysroot"

if [[ ! -d "${venv_dir}" ]]; then
    python3 -m venv "${venv_dir}"
fi

"${venv_dir}/bin/pip" install ninja

mkdir -p "${deb_dir}" "${sysroot_dir}"

pushd "${deb_dir}" >/dev/null
apt download \
    qt6-base-dev \
    qt6-base-dev-tools \
    qmake6 \
    qmake6-bin \
    qt6-qpa-plugins \
    libqt6concurrent6t64 \
    libqt6core6t64 \
    libqt6dbus6t64 \
    libqt6gui6t64 \
    libqt6network6t64 \
    libqt6opengl6t64 \
    libqt6openglwidgets6t64 \
    libqt6printsupport6t64 \
    libqt6sql6t64 \
    libqt6test6t64 \
    libqt6widgets6t64 \
    libqt6xml6t64 \
    libdouble-conversion3 \
    libvulkan-dev
popd >/dev/null

find "${deb_dir}" -name '*.deb' -print0 | while IFS= read -r -d '' deb; do
    dpkg-deb -x "${deb}" "${sysroot_dir}"
done

echo "Userland Qt sysroot prepared at: ${sysroot_dir}"

