#!/usr/bin/env python3
"""
ios_configure.py

Runs `cmake --preset ios`

CLI: python3 scripts/ios_configure.py
"""
import subprocess
import sys
import ae_config


def main():
    rc = subprocess.run(
        [ "cmake", "--preset", "ios" ],
        cwd=str( ae_config.REPO_ROOT ),
    ).returncode
    sys.exit( rc )


if __name__ == "__main__":
    main()
