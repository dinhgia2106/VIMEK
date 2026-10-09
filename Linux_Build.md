# VIMEK on Linux

[English](README.md) | [Tiếng Việt](README.vi.md)

VIMEK's Linux alpha is an **IBus input method**, with a GTK settings window.
It supports Unicode, Telex, VNI, Simple Telex 1/2, spelling checks, and
Vietnamese/English switching with Ctrl+Alt or Ctrl+Shift. Text expansion,
per-app mode memory, and legacy encoding conversion are not yet available on Linux.

## Install on Ubuntu/Debian

Download the amd64 `.deb` from [GitHub Releases](https://github.com/dinhgia2106/VIMEK/releases)
or the [website](https://dinhgia2106.github.io/VIMEK/). The package is built on Ubuntu 22.04;
use a compatible x86-64 system with IBus 1.5.20 or later.

```bash
sudo apt install ./VIMEK-0.1.0-alpha.6-Linux-amd64.deb
```

Log out and back in so IBus discovers the new engine. In GNOME/Ubuntu, open
**Settings → Keyboard → Input Sources**, add **Vietnamese → VIMEK**, and select it
from the system input menu. Some versions place Input Sources under **Region & Language**.
If Vietnamese is hidden, enable all input sources:

```bash
gsettings set org.gnome.desktop.input-sources show-all-sources true
```

On other desktops using IBus, open `ibus-setup` and add VIMEK in **Input Method**.
If the desktop uses a different input framework, configure IBus first; the current
VIMEK package does not provide a Fcitx engine. On Ubuntu derivatives that use
`im-config`, choose IBus and log in again. Do not run two input frameworks together.

Open **VIMEK** from the application menu to choose your method, shortcut,
sound, or spelling preferences. The IBus menu exposes the same settings.
Hold Ctrl and tap/release Alt (or Shift) repeatedly to switch Vietnamese/English.
Sound uses the desktop's sound theme when a compatible audio backend is available.
Input is composed through IBus preedit and committed at word boundaries; it does
not require a global keyboard hook. Applications must support the selected IBus
integration. Password/PIN fields that report their purpose bypass composition.

## Build from source

On Ubuntu/Debian:

```bash
sudo apt install build-essential cmake pkg-config file dpkg-dev libibus-1.0-dev libcanberra-dev
bash scripts/build-linux.sh
sudo apt install ./dist/linux/VIMEK-0.1.0-Linux-amd64.deb
```

For a release package, pass the version: `bash scripts/build-linux.sh 0.1.0-alpha.6`.
The script builds, runs the engine/composition tests, and creates a `.deb` in `dist/linux`.

Other distributions can build with CMake, IBus development headers, GLib, and
libcanberra. Install the runtime dependencies (IBus, its GTK module, Python 3,
PyGObject, and GTK 3 introspection) using your distribution's package manager:

```bash
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build/linux --parallel
ctest --test-dir build/linux --output-on-failure
sudo cmake --install build/linux
sudo glib-compile-schemas /usr/share/glib-2.0/schemas
sudo gtk-update-icon-cache -t /usr/share/icons/hicolor
```

Configure the prefix before compiling; IBus component paths are generated from it.
Set `-DVIMEK_BUILD_IBUS=OFF` to build only the shared engine tests.

## Integration tests

After installing the package:

```bash
sudo apt install dbus-x11 xvfb xauth python3-gi python3-cairo python3-gi-cairo gir1.2-gtk-3.0 xdotool
bash tests/run-linux-integration.sh
```

Tests run in a separate D-Bus session and virtual X11 display with temporary
settings. They exercise the installed engine, Unicode commit/preedit, both
shortcuts, focus changes, password fields, real GTK input, and settings controls.
The initial release is verified on Ubuntu with X11; GNOME Wayland and other
desktops still need interactive testing.

For IBus API details, see the [IBus engine documentation](https://ibus.github.io/docs/ibus-1.5/IBusEngine.html).

## Uninstall

```bash
sudo apt remove vimek
```

Remove VIMEK from your input sources and log in again. Preferences can be reset
separately with `gsettings reset-recursively org.vimek.settings` while installed.
