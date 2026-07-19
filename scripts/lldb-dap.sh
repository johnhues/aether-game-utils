#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# lldb-dap.sh — wrapper around Apple's lldb-dap.
#
# Two jobs:
#   1. Resolve lldb-dap via `xcrun -f lldb-dap` so the user's currently-selected
#      Xcode (xcode-select / DEVELOPER_DIR) wins — no hardcoded Xcode.app path.
#   2. Read the iOS device UDID from scripts/config.env and inject
#      `device select <UDID>` as a --pre-init-command. lldb requires an
#      explicit device selection before `device process attach`; doing it here
#      keeps the workspace launch config free of per-machine UDID state.
#-------------------------------------------------------------------------------
set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
CONFIG_FILE="$SCRIPT_DIR/config.env"
LLDB_DAP="$( xcrun -f lldb-dap )"

IOS_DEVICE=""
if [[ -f "$CONFIG_FILE" ]]; then
    # shellcheck source=/dev/null
    . "$CONFIG_FILE"
fi

if [[ -n "$IOS_DEVICE" ]]; then
    exec "$LLDB_DAP" --pre-init-command "device select $IOS_DEVICE" "$@"
else
    exec "$LLDB_DAP" "$@"
fi
