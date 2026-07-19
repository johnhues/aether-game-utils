import os
import shlex
import subprocess
from pathlib import Path

SCRIPT_DIR  = Path( __file__ ).resolve().parent
REPO_ROOT   = SCRIPT_DIR.parent
PBXPROJ     = REPO_ROOT / "build_ios" / "aether-game-utils.xcodeproj" / "project.pbxproj"
CONFIG_FILE = SCRIPT_DIR / "config.env"


def get_env_var( key, default=None ):
    # Real environment variable wins, so an exported shell var can override
    # without being mandatory.
    if os.environ.get( key ):
        return os.environ[ key ]
    if not CONFIG_FILE.exists():
        return default
    # Let the shell interpret config.env ( it is the same file the iOS .sh
    # scripts source ) and echo the one variable directly. Reading the shell
    # variable rather than `env` means non-exported assignments are still
    # picked up, and the shell handles comments/quoting consistently.
    script = f'. {shlex.quote( str( CONFIG_FILE ) )}; printf %s "${{{key}}}"'
    value = subprocess.run(
        [ "sh", "-c", script ], capture_output=True, text=True
    ).stdout
    return value or default
