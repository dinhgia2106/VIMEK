# VIMEK

**English** | [Tiếng Việt](README.vi.md)

An open-source Vietnamese input method for Windows and macOS, developed by
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

## Download

Download VIMEK from the [website](https://dinhgia2106.github.io/VIMEK/)
or [GitHub Releases](https://github.com/dinhgia2106/VIMEK/releases).

- **Windows:** choose x64 or x86, extract the ZIP, and run VIMEK.
- **macOS:** the universal build supports Intel and Apple Silicon on macOS 11 or later. Extract the ZIP and move VIMEK to **Applications**.

Current Windows alpha builds are unsigned. macOS builds are signed ad-hoc and
are not notarized with an Apple Developer ID.

## Usage

Open VIMEK and choose an input method in the control panel or tray menu.
The **V** icon indicates Vietnamese mode; **E** indicates English mode.
To avoid conflicts, disable other input methods and select **ENG** on Windows
or **ABC/U.S.** on macOS.

The default shortcut is **Ctrl+Alt** on Windows and **Control+Option** on macOS.
Hold Ctrl/Control and press and release Alt/Option to switch; repeat while
holding Ctrl/Control to switch again. Change the shortcut or disable the sound
in **Advanced (Nâng cao)**.

| Input method | Type | Result |
| --- | --- | --- |
| Telex | `Tooi yeeu tieengs Vieetj` | Tôi yêu tiếng Việt |
| VNI | `tie6ng1 Vie6t5` | tiếng Việt |

On macOS, grant **Accessibility** permission in **System Settings → Privacy & Security**
when prompted, then reopen VIMEK.

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

## Contributing

Bug reports, feature suggestions, and pull requests are welcome.
See [CONTRIBUTING.md (Vietnamese)](CONTRIBUTING.md) to get started.

## Author and license

VIMEK is developed and maintained by **GrazT**.
The project is licensed under [GNU GPL v3](LICENSE) and builds on
[OpenKey](https://github.com/tuyenvm/OpenKey) by Mai Vũ Tuyên.
See [NOTICE.md](NOTICE.md) for copyright and third-party acknowledgments.
