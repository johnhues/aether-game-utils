#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# ios-stop.sh — stop dev apps spawned by ios-launch.sh.
# Usage: ios-stop.sh (--device <UUID> | --all)
#
# --device <UUID>  stop only dev apps on that device
# --all            stop every dev app on every connected device (any
#                  devicectl streamer that ios-launch.sh could have spawned)
#
# Sends SIGHUP to every `devicectl device process launch` host process
# matching the target scope. devicectl's SIGHUP handler forwards to the iOS
# app it launched, so the on-device process exits too (see daemonize.sh for
# the inverse case — why ios-launch.sh detaches devicectl through that
# helper specifically to *avoid* this propagation when the VS Code pty
# closes).
#
# Scope is per-device (or all-devices), not per-bundle: the invariant is
# one dev app per device, so a new launch supersedes whatever was previously
# held. Bundle-scoped cleanup used to leave cross-bundle handoffs (e.g.
# SmallEngine → ReadMe) with the prior streamer still holding the device's
# tunnel/usage assertion, which made the new launch hang in devicectl's
# handshake.
#-------------------------------------------------------------------------------
set -euo pipefail

print_help() {
    sed -n '2,23p' "$0"
}

if [[ $# -eq 0 ]]; then
    print_help
    exit 1
fi

DEV_ID=""
STOP_ALL=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --device)
            DEV_ID="${2:?--device requires a UUID argument}"
            shift 2
            ;;
        --device=*)
            DEV_ID="${1#--device=}"
            shift
            ;;
        --all)
            STOP_ALL=1
            shift
            ;;
        -h|--help)
            print_help
            exit 0
            ;;
        *)
            echo "ERROR: unexpected argument '$1'" >&2
            print_help >&2
            exit 1
            ;;
    esac
done

if [[ "$STOP_ALL" -eq 1 && -n "$DEV_ID" ]]; then
    echo "ERROR: --device and --all are mutually exclusive" >&2
    exit 1
fi
if [[ "$STOP_ALL" -eq 0 && -z "$DEV_ID" ]]; then
    echo "ERROR: must specify --device <UUID> or --all" >&2
    exit 1
fi

if [[ "$STOP_ALL" -eq 1 ]]; then
    PATTERN="devicectl device process launch"
    SCOPE="all devices"
else
    PATTERN="devicectl device process launch.*--device $DEV_ID"
    SCOPE="$DEV_ID"
fi

PIDS="$( pgrep -f "$PATTERN" 2>/dev/null || true )"
if [[ -z "$PIDS" ]]; then
    echo "==> ios-stop: no dev apps streaming on $SCOPE"
    exit 0
fi

# shellcheck disable=SC2086 # word-splitting is intentional for the PID list
echo "==> ios-stop: SIGHUP to streamer pid(s) $( echo $PIDS | tr '\n' ' ' )on $SCOPE"
pkill -HUP -f "$PATTERN" 2>/dev/null || true

# Wait briefly for the streamer (and the iOS app it forwards SIGHUP to) to exit.
for _ in 1 2 3 4 5; do
    pgrep -f "$PATTERN" >/dev/null 2>&1 || exit 0
    sleep 0.1
done
echo "==> ios-stop: WARNING: some devicectl streamer(s) still running after 0.5s" >&2
exit 0
