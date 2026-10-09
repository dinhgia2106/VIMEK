#!/usr/bin/env bash
# Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail
cd "$(dirname "$0")/.."
vimek_test_config="$(mktemp -d)"
trap 'rm -rf -- "$vimek_test_config"' EXIT
export XDG_CONFIG_HOME="$vimek_test_config"
export NO_AT_BRIDGE=1
export GTK_IM_MODULE=ibus
dbus-run-session -- xvfb-run -a python3 tests/linux_ibus_tests.py
GSETTINGS_BACKEND=memory dbus-run-session -- xvfb-run -a python3 tests/linux_settings_tests.py
