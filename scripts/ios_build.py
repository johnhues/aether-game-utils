#!/usr/bin/env python3
"""
ios_build.py TARGET [CONFIG]

Builds an iOS target via the matching 'ios-<config>' build preset in
CMakePresets.json. The preset carries -allowProvisioningUpdates (so Xcode
auto-handles provisioning) and -destination generic/platform=iOS as
nativeToolOptions. Signing identity + team are baked into the Xcode project
at configure time by examples/CMakeLists.txt (which defaults the team from
$AE_APPLE_DEVELOPMENT_TEAM or scripts/config.env) and scripts/AddBundle.cmake,
so this script does not pass any signing overrides.

Accepts either the CMake/Xcode target name (e.g. '06_triangle') or the
bundle display name (e.g. 'Triangle'). cmake-tools'
${command:cmake.launchTargetName} hands us the display name; xcodebuild
needs the CMake target name. Resolves via project.pbxproj.

CONFIG is the Xcode/CMake configuration (default 'Debug'); cmake-tools'
${command:cmake.buildType} supplies it from the VS Code task. Must match an
ios build preset (currently Debug or RelWithDebInfo).

CLI: python3 scripts/ios_build.py Triangle [Debug|RelWithDebInfo]
"""
import subprocess
import sys
import ae_config
import ios_pbxproj


def main():
    if len( sys.argv ) not in ( 2, 3 ):
        sys.exit( "Usage: ios_build.py <target-or-display-name> [config=Debug]" )
    objects = ios_pbxproj.load_objects()

    target_name = sys.argv[ 1 ]

    config = sys.argv[ 2 ] if len( sys.argv ) == 3 else ""
    if config:
        valid = ios_pbxproj.project_configs( objects )
        if config not in valid:
            sys.exit(
                f"ERROR: '{config}' is not a valid configuration; "
                f"expected one of: {', '.join( valid )}"
            )
    config = config or "Debug"

    target, _ = ios_pbxproj.resolve_target( objects, target_name, config )

    preset = f"ios-{config.lower()}"
    print( f"==> Building {target} ({preset})" )
    rc = subprocess.run(
        [
            "cmake", "--build", "--preset", preset,
            "--target", target,
        ],
        cwd=str( ae_config.REPO_ROOT ),
    ).returncode
    sys.exit( rc )


if __name__ == "__main__":
    main()
