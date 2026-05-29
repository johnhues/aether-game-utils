#!/usr/bin/env python3
"""
ios_configure.py

Configures the iOS build by running `cmake --preset ios`, with the Apple
Developer Team injected from (in order):
    1. $AE_APPLE_DEVELOPMENT_TEAM env var
    2. scripts/config.env AE_APPLE_DEVELOPMENT_TEAM (gitignored; see config.env.example)
The team is passed as `-DAE_APPLE_DEVELOPMENT_TEAM=<id>`, which examples/CMakeLists.txt
forwards to scripts/AddBundle.cmake so the generated Xcode project has
XCODE_ATTRIBUTE_DEVELOPMENT_TEAM baked in. No mandatory shell variable: when
nothing is set the preset is configured with an empty team (signing will fail
at build time, same as a bare `cmake --preset ios`).

CLI: python3 scripts/ios_configure.py
"""
import subprocess
import sys
import ae_config


def main():
    team = ae_config.get_env_var( "AE_APPLE_DEVELOPMENT_TEAM" )

    cmake_args = [ "cmake", "--preset", "ios" ]
    if team:
        cmake_args.append( f"-DAE_APPLE_DEVELOPMENT_TEAM={team}" )
        print( f"==> AE_APPLE_DEVELOPMENT_TEAM={team}" )
    else:
        print( "==> No team set (env $AE_APPLE_DEVELOPMENT_TEAM or scripts/config.env); generated project will be unsigned" )

    rc = subprocess.run( cmake_args, cwd=str( ae_config.REPO_ROOT ) ).returncode
    sys.exit( rc )


if __name__ == "__main__":
    main()
