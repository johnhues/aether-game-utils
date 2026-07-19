#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# daemonize.sh — fork a command into a fully detached session.
# Usage: daemonize.sh <log-path> <cmd> [args...]
#
# Runs the classic Unix double-fork + setsid + tty detach so the child
# survives the caller's pty closing (no SIGHUP) and is reparented to launchd.
# stdout/stderr go to <log-path> (append mode); stdin is /dev/null.
#
# Needed for `xcrun devicectl device process launch --console`: devicectl
# installs its own SIGHUP handler (to forward "catchable signals" to the iOS
# app), which overrides nohup's SIG_IGN inheritance. When the VS Code task
# script exits, the pty closes, the foreground pgroup receives SIGHUP, and
# devicectl forwards it — killing the iOS app with signal 1. setsid puts
# devicectl in a new session with no controlling tty, so the pty close
# doesn't reach it.
#
# macOS doesn't ship setsid(1), so we use perl's POSIX::setsid.
#-------------------------------------------------------------------------------
set -euo pipefail

LOG="${1:?daemonize: log path required}"
shift

if [[ "$#" -eq 0 ]]; then
    echo "daemonize: no command" >&2
    exit 2
fi

exec perl -e '
use POSIX qw(setsid);
my $log = shift @ARGV;
defined(my $pid = fork) or die "fork: $!";
exit 0 if $pid;                       # original caller returns immediately
setsid() or die "setsid: $!";         # new session, no controlling tty
defined($pid = fork) or die "fork: $!";
exit 0 if $pid;                       # not session leader, cannot reacquire tty
chdir "/" or die "chdir: $!";
open STDIN,  "<", "/dev/null" or die "stdin: $!";
open STDOUT, ">>", $log or die "stdout: $!";
open STDERR, ">>&STDOUT" or die "stderr: $!";
exec { $ARGV[0] } @ARGV or die "exec $ARGV[0]: $!";
' "$LOG" "$@"
