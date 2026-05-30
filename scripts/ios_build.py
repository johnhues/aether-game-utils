#!/usr/bin/env python3
"""
ios_build.py TARGET [CONFIG]

Builds an iOS target with -allowProvisioningUpdates so Xcode auto-handles
provisioning. Signing identity + team are baked into the Xcode project at
configure time by examples/CMakeLists.txt (which defaults the team from
$AE_APPLE_DEVELOPMENT_TEAM or scripts/config.env) and scripts/AddBundle.cmake,
so this script does not pass any signing overrides.

Accepts either the CMake/Xcode target name (e.g. '06_triangle') or the
bundle display name (e.g. 'Triangle'). cmake-tools'
${command:cmake.launchTargetName} hands us the display name; xcodebuild
needs the CMake target name. Resolves via project.pbxproj.

CONFIG is the Xcode/CMake configuration (default 'Debug'); cmake-tools'
${command:cmake.buildType} supplies it from the VS Code task.

CLI: python3 scripts/ios_build.py Triangle [Debug|Release]
"""
import json
import subprocess
import sys
import ae_config

def load_objects():
    result = subprocess.run(
        [ "plutil", "-convert", "json", "-o", "-", str( ae_config.PBXPROJ ) ],
        capture_output=True, check=True,
    )
    return json.loads( result.stdout ).get( "objects", {} )


def project_configs( objects ):
    names = []
    for obj in objects.values():
        if isinstance( obj, dict ) and obj.get( "isa" ) == "XCBuildConfiguration":
            name = obj.get( "name" )
            if name and name not in names:
                names.append( name )
    return names


def resolve_cmake_target( objects, name, config="Debug" ):
    targets = [
        ( ident, obj ) for ident, obj in objects.items()
        if isinstance( obj, dict ) and obj.get( "isa" ) == "PBXNativeTarget"
    ]

    def settings_for( target_obj ):
        config_list = objects.get( target_obj.get( "buildConfigurationList", "" ), {} )
        for cid in config_list.get( "buildConfigurations", [] ):
            c = objects.get( cid, {} )
            if c.get( "name" ) == config:
                return c.get( "buildSettings", {} )
        return {}

    for _, t in targets:
        if t.get( "name" ) == name:
            return name

    for _, t in targets:
        if settings_for( t ).get( "PRODUCT_NAME" ) == name:
            return t[ "name" ]

    sys.exit( f"ERROR: no Xcode target matches '{name}' (checked target name and PRODUCT_NAME)" )


def main():
    if len( sys.argv ) not in ( 2, 3 ):
        sys.exit( "Usage: ios_build.py <target-or-display-name> [config=Debug]" )
    objects = load_objects()

    target_name = sys.argv[ 1 ]

    config = sys.argv[ 2 ] if len( sys.argv ) == 3 else ""
    if config:
        valid = project_configs( objects )
        if config not in valid:
            sys.exit(
                f"ERROR: '{config}' is not a valid configuration; "
                f"expected one of: {', '.join( valid )}"
            )
    config = config or "Debug"

    target = resolve_cmake_target( objects, target_name, config )

    print( f"==> Building {target}" )
    rc = subprocess.run(
        [
            "cmake", "--build", str( ae_config.REPO_ROOT / "build_ios" ),
            "--config", config,
            "--target", target,
            "--",
            "-allowProvisioningUpdates",
            "-destination", "generic/platform=iOS",
        ],
        cwd=str( ae_config.REPO_ROOT ),
    ).returncode
    sys.exit( rc )


if __name__ == "__main__":
    main()
