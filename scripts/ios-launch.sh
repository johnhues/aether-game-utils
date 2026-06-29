#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# ios-launch.sh — launch an installed iOS .app on a device.
# Usage: ios-launch.sh [--device <UUID>] [--start-paused] [--console] <path>
#   <path> may be the .app bundle, the inner binary, or cmake-tools'
#   launchTargetPath form — see ios-resolve-exe.sh.
#
# Reads CFBundleIdentifier from the bundle's Info.plist and runs
# `xcrun devicectl device process launch`.
#
# Default (no flag): plain launch; the app runs and foregrounds normally —
# this is the "run on device, no debugger" path.
#
# --start-paused: launches the app, holds it at its entry point (pre-`main`,
# inside dyld), and returns immediately with the process suspended. You can
# then attach with lldb by name using ios-debug.sh or the `iOS: Debug` config;
# because the process is already running and addressable by name, the attach
# needs no --waitfor and has no ordering race. Before that attach (which must
# be plain, not --continue), `process handle SIGSTOP -s false -n true` is set
# so lldb auto-resumes the process past the launch hold once attached. Use
# this when you need genuine pre-`main` debugging.
#
# --console: capture the device process's stdout/stderr via
# `devicectl ... --console` and redirect it to a log file (default
# /tmp/aether-ios-console.log, overridable via $AE_IOS_CONSOLE_LOG). devicectl
# is detached (nohup + disown) so this script can exit cleanly once the launch
# is confirmed; devicectl keeps running and writing to the log until the app
# exits. A separate VS Code task (`iOS: Console`) is responsible for
# `tail -F`ing the log into a foregrounded panel. Any prior devicectl streamer
# for the same bundle is pkilled first so two streamers don't interleave
# writes. This is the only path to host-visible app stdout — iOS routes app
# stdout to launchd by default and lldb attaches *after* posix_spawn so it
# cannot reparent FDs.
#
# Device UUID resolution order:
#   1. --device <UUID> flag
#   2. $AE_IOS_DEVICE env var
#   3. scripts/config.env IOS_DEVICE (see scripts/config.env.example)
#   4. Auto-detect the first connected non-unavailable device
#-------------------------------------------------------------------------------
set -euo pipefail

# Best-effort warning if the device's symbols/shared cache haven't been
# extracted by Xcode yet. While extraction is pending, lldb reads system
# images out of process memory — attach/backtrace crawl and the session can
# look frozen. Xcode stages the extraction under
#   ~/Library/Developer/Xcode/iOS DeviceSupport/<model> <ver> (<build>)/
# with marker files: .processing_lock (in progress), .finalized (complete).
# Keyed on the OS build (e.g. 23F77), which lives in `devicectl device info`,
# not in `devicectl list devices`. Never fails the launch — returns 0 always.
warn_if_symbols_not_ready() {
    local udid="$1"
    local tmp build dir
    tmp="$( mktemp -t ios-devinfo.XXXXXX.json )"
    if ! xcrun devicectl device info details --device "$udid" \
            --json-output "$tmp" >/dev/null 2>&1; then
        rm -f "$tmp"
        return 0
    fi
    build="$( python3 - "$tmp" <<'PY'
import json, re, sys
try:
    doc = json.load(open(sys.argv[1]))
except Exception:
    sys.exit(0)
def walk(o):
    if isinstance(o, dict):
        for k, v in o.items():
            yield k, v
            yield from walk(v)
    elif isinstance(o, list):
        for v in o:
            yield from walk(v)
pat = re.compile(r"\d{2}[A-Z]\d+[a-z]?")
best = ""
for k, v in walk(doc):
    if isinstance(v, str) and "build" in k.lower() and pat.fullmatch(v):
        best = v
        if k.lower() == "osbuildupdate":
            break
print(best)
PY
)"
    rm -f "$tmp"
    if [[ -z "$build" ]]; then
        return 0
    fi
    dir="$( ls -d ~/Library/Developer/Xcode/iOS\ DeviceSupport/*"($build)" \
        2>/dev/null | head -1 )"
    if [[ -z "$dir" ]]; then
        echo "==> WARNING: no extracted symbols for iOS build $build." >&2
        echo "    Debugging will be slow until Xcode finishes preparing the" >&2
        echo "    device (Xcode > Window > Devices & Simulators)." >&2
    elif [[ ! -f "$dir/.finalized" ]]; then
        echo "==> WARNING: symbol extraction for iOS build $build still in" >&2
        echo "    progress — debugging may be slow or look frozen until done." >&2
    fi
}

DEV_ID_OVERRIDE=""
APP_PATH=""
START_PAUSED=0
CONSOLE=0
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
        --start-paused)
            START_PAUSED=1
            shift
            ;;
        --console)
            CONSOLE=1
            shift
            ;;
        -h|--help)
            sed -n '2,32p' "$0"
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
    echo "Usage: ios-launch.sh [--device <UUID>] [--start-paused] <path/to/App.app>" >&2
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
APP_PATH="$( "$SCRIPT_DIR/ios-resolve-exe.sh" "$APP_PATH" app )"
BUNDLE_ID="$( "$SCRIPT_DIR/ios-resolve-exe.sh" "$APP_PATH" bundle-id )"

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

warn_if_symbols_not_ready "$DEV_ID"

LAUNCH_ARGS=( --device "$DEV_ID" --terminate-existing )
if [[ "$START_PAUSED" -eq 1 ]]; then
    echo "==> Launching $BUNDLE_ID (held at entry for debugger attach)"
    LAUNCH_ARGS+=( --start-stopped )
else
    echo "==> Launching $BUNDLE_ID"
fi

# Reap any prior streamer/dev-app on this device before the new launch — one
# dev app per device, so a new launch supersedes whatever was previously held.
# Without this, cross-bundle handoffs (e.g. SmallEngine → ReadMe) leave the
# prior streamer holding the device's tunnel/usage assertion and the new
# launch hangs in devicectl's handshake. Applies to plain and --console
# paths alike; see ios-stop.sh for the SIGHUP-propagation mechanism.
"$SCRIPT_DIR/ios-stop.sh" --device "$DEV_ID"

if [[ "$CONSOLE" -eq 1 ]]; then
    LAUNCH_ARGS+=( --console )
    LOG_FILE="${AE_IOS_CONSOLE_LOG:-/tmp/aether-ios-console.log}"

    # Truncate the log; tail -F follows by name and recovers cleanly.
    : > "$LOG_FILE"
    echo "==> Streaming device stdout/stderr to $LOG_FILE"

    # Detach devicectl into its own session via daemonize.sh so the VS Code
    # task pty closing doesn't deliver SIGHUP to it. devicectl installs its
    # own SIGHUP handler that forwards to the iOS app, so plain nohup+disown
    # is not enough — see daemonize.sh for the full reasoning.
    "$SCRIPT_DIR/daemonize.sh" "$LOG_FILE" \
        xcrun devicectl device process launch "${LAUNCH_ARGS[@]}" "$BUNDLE_ID"

    DEADLINE=$(( SECONDS + 30 ))
    while (( SECONDS < DEADLINE )); do
        if grep -q "Launched application with" "$LOG_FILE" 2>/dev/null; then
            echo "==> ios-launch: app launched; console streaming to $LOG_FILE"
            exit 0
        fi
        if grep -q "App terminated due to signal" "$LOG_FILE" 2>/dev/null \
                || grep -qE "(error|ERROR|Failed)" "$LOG_FILE" 2>/dev/null; then
            echo "ERROR: launch failed; see $LOG_FILE" >&2
            tail -20 "$LOG_FILE" >&2 || true
            exit 1
        fi
        sleep 0.2
    done
    echo "ERROR: timed out waiting for launch sentinel after 30s; see $LOG_FILE" >&2
    exit 1
else
    xcrun devicectl device process launch "${LAUNCH_ARGS[@]}" "$BUNDLE_ID"
fi
