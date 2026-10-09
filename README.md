# VIMEK

**English** | [Tiếng Việt](README.vi.md)

An open-source Vietnamese input method for Windows, macOS, and Linux, developed by
[GrazT](https://github.com/dinhgia2106). VIMEK offers a compact interface,
follows your system's light or dark appearance, and makes switching between
Vietnamese and English easy.

> VIMEK is in alpha. Feedback and contributions are welcome.

![VIMEK interface](docs/images/dashboard-light.png)

## Features

- Telex, VNI, Simple Telex 1, and Simple Telex 2.
- Unicode and common Vietnamese character encodings.
- Switch between Vietnamese and English with a shortcut or the V/E tray icon.
- System sound when switching modes, with an option to disable it in **Advanced (Nâng cao)**.
- Spell checking, text expansion, and per-app input mode memory.
- Text encoding conversion and an option to launch at login.
- System light/dark appearance, high-DPI and Retina support.

The Linux alpha uses **IBus** and supports Unicode, the four input methods,
Vietnamese/English switching, spelling checks, sound feedback, and a GTK settings
panel. Text expansion, per-app mode memory, and encoding conversion are currently
available on Windows and macOS.

## Download

Download VIMEK from the [website](https://dinhgia2106.github.io/VIMEK/)
or [GitHub Releases](https://github.com/dinhgia2106/VIMEK/releases).

- **Windows:** choose x64 or x86, extract the ZIP, and run VIMEK.
- **macOS:** the universal build supports Intel and Apple Silicon on macOS 11 or later. Extract the ZIP and move VIMEK to **Applications**.
- **Linux:** the amd64 `.deb` package is built for Ubuntu 22.04 or later and compatible Debian-based systems with IBus. See the [Linux guide](Linux_Build.md).

Current Windows alpha builds are unsigned. macOS builds are signed ad-hoc and
are not notarized with an Apple Developer ID.

## Usage

Open VIMEK and choose an input method in the control panel or tray menu.
The **V** icon indicates Vietnamese mode; **E** indicates English mode.
To avoid conflicts, disable other input methods and select **ENG** on Windows
or **ABC/U.S.** on macOS.

The default shortcut is **Ctrl+Alt** on Windows/Linux and **Control+Option** on macOS.
Hold Ctrl/Control and press and release Alt/Option to switch; repeat while
holding Ctrl/Control to switch again. Change the shortcut or disable the sound
in **Advanced (Nâng cao)**.

On Windows, click the shortcut button in the main panel to alternate between
**Ctrl+Alt** and **Ctrl+Shift**. The change takes effect immediately.
The panel also provides **Run as administrator** and **Start with Windows** toggles.
Enabling administrator mode offers to restart VIMEK and requires Windows UAC approval.

| Input method | Type | Result |
| --- | --- | --- |
| Telex | `Tooi yeeu tieengs Vieetj` | Tôi yêu tiếng Việt |
| VNI | `tie6ng1 Vie6t5` | tiếng Việt |

On macOS, grant **Accessibility** permission in **System Settings → Privacy & Security**
when prompted, then reopen VIMEK.

On Linux, install the `.deb`, log out and back in, then add **Vietnamese → VIMEK**
to your input sources. Open **VIMEK** from the application menu to choose your method
or alternate between **Ctrl+Alt** and **Ctrl+Shift**.

## Build from source

### Windows

Use Visual Studio 2022 or later with the **Desktop development with C++** workload and CMake:

```powershell
cmake -S . -B build/vs -A x64
cmake --build build/vs --config Release
ctest --test-dir build/vs -C Release --output-on-failure
```

Output: `build/vs/Release/VIMEK.exe`.
You can also open `Sources/VIMEK/windows/VIMEK.sln` in Visual Studio.

With [LLVM-MinGW UCRT](https://github.com/mstorsjo/llvm-mingw/releases),
extract the toolchain into `.tools/` or provide its `bin` directory:

```powershell
./scripts/build-windows.ps1
./scripts/build-windows.ps1 -Toolchain C:/llvm-mingw/bin
./scripts/build-windows.ps1 -TestsOnly
```

The default output is `dist/x64/VIMEK64.exe`. Add `-Platform x86` for a 32-bit build.
Close any running VIMEK instance before building into the same directory.

### macOS

Requires macOS 11 or later and Xcode with Command Line Tools:

```bash
bash scripts/build-macos.sh
```

Output: `dist/macos/VIMEK.app`, supporting Intel and Apple Silicon.
See the [macOS guide (Vietnamese)](macOS_Build.md) for installation and signing details.

### Linux

On Ubuntu/Debian, install the build dependencies and create the IBus engine and `.deb` package:

```bash
sudo apt install build-essential cmake pkg-config file dpkg-dev libibus-1.0-dev libcanberra-dev
bash scripts/build-linux.sh
```

Output: `dist/linux/VIMEK-0.1.0-Linux-amd64.deb` on an x86-64 machine.
Install with `sudo apt install ./dist/linux/VIMEK-0.1.0-Linux-amd64.deb`, then log
out and back in and add **Vietnamese → VIMEK** to your input sources.
The [Linux guide](Linux_Build.md) covers IBus setup, tests, and building on other distributions.

## Contributing

Bug reports, feature suggestions, and pull requests are welcome.
See [CONTRIBUTING.md (Vietnamese)](CONTRIBUTING.md) to get started.

## Author and license

VIMEK is developed and maintained by **GrazT**.
The project is licensed under [GNU GPL v3](LICENSE) and builds on
[OpenKey](https://github.com/tuyenvm/OpenKey) by Mai Vũ Tuyên.
See [NOTICE.md](NOTICE.md) for copyright and third-party acknowledgments.
