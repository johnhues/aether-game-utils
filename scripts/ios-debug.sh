#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# ios-debug.sh — attach lldb to an iOS .app held at its entry point.
# Usage: ios-debug.sh [--device <UUID>] <path>
#   <path> may be the .app bundle, the inner binary, or cmake-tools'
#   launchTargetPath form — see ios-resolve-exe.sh.
#
# Assumes the app is already installed (ios-install.sh) and already launched
# held at entry (ios-launch.sh --start-paused). Attaches by name to the
# suspended process and lets it run. `process handle SIGSTOP -s false -n true`
# is set before a plain attach (no --waitfor, no --continue) so lldb
# auto-resumes past the launch hold — this is what makes a --start-paused
# process actually run under the debugger instead of only on detach. Gives
# genuine pre-`main` debugging: set breakpoints in static initializers / `main`
# and they halt before any of their code runs.
#
# Sequential command-line flow:
#   scripts/ios-install.sh <app>
#   scripts/ios-launch.sh --start-paused <app>
#   scripts/ios-debug.sh <app>
#
# Device UUID resolution order:
#   1. --device <UUID> flag
#   2. $AE_IOS_DEVICE env var
#   3. scripts/config.env IOS_DEVICE (see scripts/config.env.example)
#   4. Auto-detect the first connected non-unavailable device
#-------------------------------------------------------------------------------
set -euo pipefail

DEV_ID_OVERRIDE=""
APP_PATH=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --device)
            DEV_ID_OVERRIDE="${2:?--device requires a UUID argument}"
            shift 2
            ;;
        --device=*)
            DEV_ID_OVERRIDE="${1#--device=}"
            shift
            ;;
        -h|--help)
            sed -n '2,26p' "$0"
            exit 0
            ;;
        *)
            if [[ -z "$APP_PATH" ]]; then
                APP_PATH="$1"
            else
                echo "ERROR: unexpected argument '$1'" >&2
                exit 1
            fi
            shift
            ;;
    esac
done

if [[ -z "$APP_PATH" ]]; then
    echo "Usage: ios-debug.sh [--device <UUID>] <path/to/App.app>" >&2
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
EXE_PATH="$( "$SCRIPT_DIR/ios-resolve-exe.sh" "$APP_PATH" exe )"
EXE_NAME="$( "$SCRIPT_DIR/ios-resolve-exe.sh" "$APP_PATH" name )"

CONFIG_FILE="$SCRIPT_DIR/config.env"
IOS_DEVICE=""
if [[ -f "$CONFIG_FILE" ]]; then
    # shellcheck source=/dev/null
    . "$CONFIG_FILE"
fi

if [[ -n "$DEV_ID_OVERRIDE" ]]; then
    DEV_ID="$DEV_ID_OVERRIDE"
    echo "==> Using device from --device flag: $DEV_ID"
elif [[ -n "${AE_IOS_DEVICE:-}" ]]; then
    DEV_ID="$AE_IOS_DEVICE"
    echo "==> Using device from \$AE_IOS_DEVICE: $DEV_ID"
elif [[ -n "$IOS_DEVICE" ]]; then
    DEV_ID="$IOS_DEVICE"
    echo "==> Using device from $CONFIG_FILE: $DEV_ID"
else
    DEV_ID="$( xcrun devicectl list devices \
        --hide-headers --columns Identifier Name --hide-default-columns \
        --filter "State != 'unavailable'" 2>/dev/null \
        | head -1 \
        | grep -oE '[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}' \
        | head -1 )"
    if [[ -z "$DEV_ID" ]]; then
        echo "ERROR: no connected iOS device found" >&2
        exit 1
    fi
    echo "==> Auto-detected device: $DEV_ID"
fi

echo "==> Attaching lldb to $EXE_NAME"
exec xcrun lldb "$EXE_PATH" \
    -o "device select $DEV_ID" \
    -o "settings set target.preload-symbols false" \
    -o "process handle SIGSTOP -s false -n true" \
    -o "device process attach --name \"$EXE_NAME\""
