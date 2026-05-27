import os
import subprocess
from pathlib import Path

SCRIPT_DIR  = Path( __file__ ).resolve().parent
REPO_ROOT   = SCRIPT_DIR.parent
PBXPROJ     = REPO_ROOT / "build_ios" / "aether-game-utils.xcodeproj" / "project.pbxproj"
CONFIG_FILE = SCRIPT_DIR / "config.env"

# Source the file and print environment variables
result = subprocess.run(
    f'source {CONFIG_FILE} && env',
    shell=True,
    capture_output=True,
    text=True
)

# Parse the environment variables
env_vars = {}
for line in result.stdout.split('\n'):
    if '=' in line:
        key, value = line.split('=', 1)
        env_vars[key] = value
        os.environ[key] = value

def get_env_var(key, default=None):
	return env_vars.get(key, default)
