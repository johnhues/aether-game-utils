#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# lldb-dap.sh — wrapper around Apple's lldb-dap.
#
# Two jobs:
#   1. Resolve lldb-dap via `xcrun -f lldb-dap` so the user's currently-selected
#      Xcode (xcode-select / DEVELOPER_DIR) wins — no hardcoded Xcode.app path.
#   2. Read the iOS device UDID from scripts/local.ini and inject
#      `device select <UDID>` as a --pre-init-command. lldb requires an
#      explicit device selection before `device process attach`; with the UDID
#      checked into the workspace file removed, this wrapper closes the gap.
#-------------------------------------------------------------------------------
set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
INI_FILE="$SCRIPT_DIR/local.ini"
LLDB_DAP="$( xcrun -f lldb-dap )"

DEVICE=""
if [[ -f "$INI_FILE" ]]; then
    DEVICE="$( awk -F '[[:space:]]*=[[:space:]]*' '
        /^\[/ { section = $0 }
        section == "[ios]" && /^[[:space:]]*device[[:space:]]*=/ { print $2; exit }
    ' "$INI_FILE" )"
fi

if [[ -n "$DEVICE" ]]; then
    exec "$LLDB_DAP" --pre-init-command "device select $DEVICE" "$@"
else
    exec "$LLDB_DAP" "$@"
fi
