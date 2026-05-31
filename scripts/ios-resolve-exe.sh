#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# ios-resolve-exe.sh — resolve a path or target/display name to an on-disk
# iOS Xcode-generator bundle and print the requested piece.
#
# Usage: ios-resolve-exe.sh <path|name> [exe|name|app|bundle-id]
#   <path|name>:
#     path  — anything containing '/' or ending in .app; .app bundle, inner
#             binary, or cmake-tools' launchTargetPath form (see below).
#     name  — CMake target name (e.g. '06_triangle') or PRODUCT_NAME display
#             name (e.g. 'Triangle'); resolved to the built .app via
#             ios-resolve-app.py (pbxproj lookup; assumes Debug config).
#   exe        (default) absolute path to the inner executable inside the .app
#   name       CFBundleExecutable (the process name lldb sees on device)
#   app        absolute path to the .app bundle
#   bundle-id  CFBundleIdentifier (devicectl process launch identifier)
#
# Why path-mode is needed:
#   cmake-tools' ${command:cmake.launchTargetPath} on Xcode-generator iOS
#   targets returns paths like:
#     <root>/build_ios/examples/Debug${EFFECTIVE_PLATFORM_NAME}/Triangle.app/Triangle
#   The placeholder is an Xcode build variable Xcode expands at compile time
#   into "-iphoneos" or "-iphonesimulator". CMake leaves it literal, so the
#   path cmake-tools reports does not exist on disk.
#
# This script:
#   1. If the input has no '/' and no .app extension, treats it as a CMake
#      target / display name and shells to ios-resolve-app.py for the .app.
#   2. Strips the ${EFFECTIVE_PLATFORM_NAME} placeholder.
#   3. Walks up to the .app component.
#   4. Tries Debug-iphoneos / Debug-iphonesimulator (or Release-*, etc.) if the
#      placeholder-stripped path is missing.
#   5. Reads CFBundleExecutable from the resolved bundle's Info.plist.
#   6. Prints the requested piece to stdout. Errors to stderr.
#-------------------------------------------------------------------------------
set -euo pipefail

INPUT="${1:?usage: ios-resolve-exe.sh <path|name> [exe|name|app|bundle-id]}"
MODE="${2:-exe}"

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

# Inputs with no '/' and no .app extension are CMake target / display names;
# resolve to the built .app via the python helper.
if [[ "$INPUT" != */* && "${INPUT##*.}" != "app" ]]; then
    INPUT="$( python3 "$SCRIPT_DIR/ios-resolve-app.py" "$INPUT" )"
fi

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

case "$MODE" in
    exe)
        EXE_NAME="$( plutil -extract CFBundleExecutable raw -o - "$APP_PATH/Info.plist" )"
        echo "$APP_PATH/$EXE_NAME"
        ;;
    name)
        plutil -extract CFBundleExecutable raw -o - "$APP_PATH/Info.plist"
        ;;
    app)
        echo "$APP_PATH"
        ;;
    bundle-id)
        plutil -extract CFBundleIdentifier raw -o - "$APP_PATH/Info.plist"
        ;;
    *)
        echo "ERROR: unknown mode '$MODE' (expected: exe|name|app|bundle-id)" >&2
        exit 1
        ;;
esac
