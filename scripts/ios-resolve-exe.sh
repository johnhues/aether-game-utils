#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# ios-resolve-exe.sh — resolve cmake-tools' launchTargetPath to a real on-disk
# path for an iOS Xcode-generator bundle, and print the requested piece.
#
# Usage: ios-resolve-exe.sh <path> [exe|name|app]
#   exe   (default) absolute path to the inner executable inside the .app
#   name  CFBundleExecutable (the process name lldb sees on device)
#   app   absolute path to the .app bundle
#
# Why this is needed:
#   cmake-tools' ${command:cmake.launchTargetPath} on Xcode-generator iOS
#   targets returns paths like:
#     <root>/build_ios/examples/Debug${EFFECTIVE_PLATFORM_NAME}/Triangle.app/Triangle
#   The placeholder is an Xcode build variable Xcode expands at compile time
#   into "-iphoneos" or "-iphonesimulator". CMake leaves it literal, so the
#   path cmake-tools reports does not exist on disk.
#
# This script:
#   1. Strips the ${EFFECTIVE_PLATFORM_NAME} placeholder.
#   2. Walks up to the .app component.
#   3. Tries Debug-iphoneos / Debug-iphonesimulator (or Release-*, etc.) if the
#      placeholder-stripped path is missing.
#   4. Reads CFBundleExecutable from the resolved bundle's Info.plist.
#   5. Prints the requested piece to stdout. Errors to stderr.
#-------------------------------------------------------------------------------
set -euo pipefail

INPUT="${1:?usage: ios-resolve-exe.sh <path> [exe|name|app]}"
MODE="${2:-exe}"

# Strip Xcode's ${EFFECTIVE_PLATFORM_NAME} placeholder.
APP_PATH="${INPUT//\$\{EFFECTIVE_PLATFORM_NAME\}/}"

# Walk up to the .app component.
candidate="$APP_PATH"
while [[ -n "$candidate" && "$candidate" != "/" && "${candidate##*.}" != "app" ]]; do
    candidate="$( dirname "$candidate" )"
done
if [[ "${candidate##*.}" == "app" ]]; then
    APP_PATH="$candidate"
fi

# Xcode appends the SDK name to the config dir (Debug → Debug-iphoneos).
if [[ ! -d "$APP_PATH" && "${APP_PATH##*.}" == "app" ]]; then
    bundle_name="$( basename "$APP_PATH" )"
    cfg_dir="$( dirname "$APP_PATH" )"
    parent_dir="$( dirname "$cfg_dir" )"
    cfg_name="$( basename "$cfg_dir" )"
    for variant in "${cfg_name}-iphoneos" "${cfg_name}-iphonesimulator"; do
        if [[ -d "$parent_dir/$variant/$bundle_name" ]]; then
            APP_PATH="$parent_dir/$variant/$bundle_name"
            break
        fi
    done
fi

if [[ ! -d "$APP_PATH" ]]; then
    echo "ERROR: no .app bundle resolved from '$INPUT' (last tried: $APP_PATH)" >&2
    exit 1
fi

EXE_NAME="$( plutil -extract CFBundleExecutable raw -o - "$APP_PATH/Info.plist" )"

case "$MODE" in
    exe)  echo "$APP_PATH/$EXE_NAME" ;;
    name) echo "$EXE_NAME" ;;
    app)  echo "$APP_PATH" ;;
    *)    echo "ERROR: unknown mode '$MODE' (expected: exe|name|app)" >&2; exit 1 ;;
esac
