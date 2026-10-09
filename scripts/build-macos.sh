#!/usr/bin/env bash
set -euo pipefail
vimek_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$vimek_root"
mkdir -p build/macos dist/macos

# Test the engine using the actual macOS key-code table.
xcrun clang++ -std=c++14 -O2 -Wno-deprecated-declarations \
  -ISources/VIMEK/engine Sources/VIMEK/engine/*.cpp tests/engine_tests.cpp \
  -o build/macos/engine_tests
build/macos/engine_tests

xcodebuild -project Sources/VIMEK/macOS/VIMEK.xcodeproj \
  -target VIMEK -configuration Release \
  "CONFIGURATION_BUILD_DIR=$vimek_root/dist/macos" \
  "OBJROOT=$vimek_root/build/macos/obj" \
  'ARCHS=arm64 x86_64' ONLY_ACTIVE_ARCH=NO \
  MACOSX_DEPLOYMENT_TARGET=11.0 CODE_SIGNING_ALLOWED=NO \
  CODE_SIGNING_REQUIRED=NO CODE_SIGN_IDENTITY= DEVELOPMENT_TEAM=

# Upstream project currently has only the main target; build the login helper
# explicitly so the VIMEK startup option has a matching bundle.
vimek_helper="dist/macos/VIMEK.app/Contents/Library/LoginItems/VIMEKHelper.app"
mkdir -p "$vimek_helper/Contents/MacOS" "$vimek_helper/Contents/Resources/Base.lproj"
xcrun clang -fobjc-arc -arch arm64 -arch x86_64 -mmacosx-version-min=11.0 \
  -framework Cocoa Sources/VIMEK/macOS/VIMEKHelper/main.m \
  Sources/VIMEK/macOS/VIMEKHelper/AppDelegate.m \
  -o "$vimek_helper/Contents/MacOS/VIMEKHelper"
xcrun ibtool --compile "$vimek_helper/Contents/Resources/Base.lproj/MainMenu.nib" \
  Sources/VIMEK/macOS/VIMEKHelper/Base.lproj/MainMenu.xib
cp Sources/VIMEK/macOS/VIMEKHelper/Info.plist "$vimek_helper/Contents/Info.plist"
plutil -replace CFBundleExecutable -string VIMEKHelper "$vimek_helper/Contents/Info.plist"
plutil -replace CFBundleName -string VIMEKHelper "$vimek_helper/Contents/Info.plist"
plutil -replace CFBundleIdentifier -string org.vimek.inputmethod.helper "$vimek_helper/Contents/Info.plist"
plutil -replace CFBundleDevelopmentRegion -string vi "$vimek_helper/Contents/Info.plist"
plutil -replace LSMinimumSystemVersion -string 11.0 "$vimek_helper/Contents/Info.plist"
codesign --force --sign - "$vimek_helper"
codesign --force --sign - dist/macos/VIMEK.app
codesign --verify --deep --strict dist/macos/VIMEK.app
cp LICENSE NOTICE.md dist/macos/
ditto -c -k --sequesterRsrc --keepParent dist/macos/VIMEK.app dist/macos/VIMEK-macOS.zip
printf '%s\n' 'Built dist/macos/VIMEK.app (Intel + Apple Silicon).'
