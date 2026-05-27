#!/usr/bin/env python3
"""
ios_build.py TARGET

Builds an iOS target with -allowProvisioningUpdates so Xcode auto-handles
provisioning. Apple Developer Team comes from (in order):
    1. $AE_APPLE_DEVELOPMENT_TEAM env var
    2. scripts/config.env AE_APPLE_DEVELOPMENT_TEAM (gitignored; see config.env.example)
    3. Whatever was baked at configure time (empty unless CMakePresets sets it)
When a team is resolved, it is injected as a build-time
`DEVELOPMENT_TEAM=<id>` override to xcodebuild, overriding the configure-time
XCODE_ATTRIBUTE_DEVELOPMENT_TEAM set by scripts/AddBundle.cmake.

Accepts either the CMake/Xcode target name (e.g. '06_triangle') or the
bundle display name (e.g. 'Triangle'). cmake-tools'
${command:cmake.launchTargetName} hands us the display name; xcodebuild
needs the CMake target name. Resolves via project.pbxproj.

CLI: python3 scripts/ios_build.py Triangle
"""
import json
import os
import subprocess
import sys
import ae_config
from pathlib import Path

def resolve_cmake_target( name: str, config: str = "Debug" ) -> str:
    result = subprocess.run(
        [ "plutil", "-convert", "json", "-o", "-", str( ae_config.PBXPROJ ) ],
        capture_output=True, check=True,
    )
    objects = json.loads( result.stdout ).get( "objects", {} )

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
    if len( sys.argv ) != 2:
        sys.exit( "Usage: ios_build.py <target-or-display-name>" )

    target = resolve_cmake_target( sys.argv[ 1 ] )
    team   = ae_config.get_env_var("AE_APPLE_DEVELOPMENT_TEAM")

    xcode_args = [ "-allowProvisioningUpdates", "-destination", "generic/platform=iOS" ]
    if team:
        xcode_args.insert( 0, f"DEVELOPMENT_TEAM={team}" )
        print( f"==> DEVELOPMENT_TEAM={team}" )
    else:
        print( "==> No team set (env $AE_APPLE_DEVELOPMENT_TEAM or scripts/config.env); xcodebuild may fail to sign" )

    print( f"==> Building {target}" )
    rc = subprocess.run(
        [
            "cmake", "--build", str( ae_config.REPO_ROOT / "build_ios" ),
            "--config", "Debug",
            "--target", target,
            "--",
            *xcode_args,
        ],
        cwd=str( ae_config.REPO_ROOT ),
    ).returncode
    sys.exit( rc )


if __name__ == "__main__":
    main()
