#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# ios-install.sh — install a built iOS .app onto a device. Install only;
# spawning the process is ios-launch.sh's job.
# Usage: ios-install.sh [--device <UUID>] <path>
#   <path> may be the .app bundle, the inner binary, or cmake-tools'
#   launchTargetPath form (with the literal ${EFFECTIVE_PLATFORM_NAME}
#   placeholder and the un-suffixed config dir) — see ios-resolve-exe.sh.
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
            sed -n '2,15p' "$0"
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
    echo "Usage: ios-install.sh [--device <UUID>] <path/to/App.app>" >&2
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
APP_PATH="$( "$SCRIPT_DIR/ios-resolve-exe.sh" "$APP_PATH" app )"

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

echo "==> Installing $APP_PATH"
xcrun devicectl device install app --device "$DEV_ID" "$APP_PATH" | tail -5
