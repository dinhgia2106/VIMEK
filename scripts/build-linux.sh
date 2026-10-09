#!/usr/bin/env bash
# Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail
cd "$(dirname "$0")/.."
vimek_version="${1:-0.1.0}"
if [[ ! "$vimek_version" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[A-Za-z0-9.-]+)?$ ]]; then
    echo 'Invalid release version.' >&2; exit 1
fi
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
    -DVIMEK_RELEASE_VERSION="$vimek_version"
cmake --build build/linux --parallel
ctest --test-dir build/linux --output-on-failure
mkdir -p dist/linux
cpack --config build/linux/CPackConfig.cmake -G DEB -B dist/linux
