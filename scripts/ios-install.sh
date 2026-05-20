#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# ios-install.sh — install (and optionally launch) a built iOS .app.
# Usage: ios-install.sh <path/to/App.app> [bundle.id]
#   If bundle.id is provided, the app is launched on the device after install.
#-------------------------------------------------------------------------------
set -euo pipefail

APP_PATH="${1:?Usage: ios-install.sh <path/to/App.app> [bundle.id]}"
BUNDLE_ID="${2:-}"

DEV_ID="$( xcrun devicectl list devices \
    --hide-headers --columns Identifier Name --hide-default-columns \
    --filter "State != 'unavailable' && Name BEGINSWITH 'iPhone'" 2>/dev/null \
    | head -1 \
    | grep -oE '[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}' \
    | head -1 )"

if [[ -z "$DEV_ID" ]]; then
    echo "ERROR: no connected iOS device found" >&2
    exit 1
fi

echo "==> Installing $APP_PATH to device $DEV_ID"
xcrun devicectl device install app --device "$DEV_ID" "$APP_PATH" | tail -5

if [[ -n "$BUNDLE_ID" ]]; then
    echo "==> Launching $BUNDLE_ID"
    xcrun devicectl device process launch --device "$DEV_ID" "$BUNDLE_ID"
fi
