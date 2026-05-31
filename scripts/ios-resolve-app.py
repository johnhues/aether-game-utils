#!/usr/bin/env python3
"""
ios-resolve-app.py NAME [CONFIG]

Resolves a CMake target name (e.g. '06_triangle') or bundle display name
(e.g. 'Triangle') to the .app path under build_ios. Static: derived from
pbxproj (CONFIGURATION_BUILD_DIR + PRODUCT_NAME); does not check disk, so
works after configure even before the target has been built.

CONFIG defaults to 'Debug'.

CLI: python3 scripts/ios-resolve-app.py 06_triangle [Debug|RelWithDebInfo]
"""
import sys
import ios_pbxproj


def main():
    if len( sys.argv ) not in ( 2, 3 ):
        sys.exit( "Usage: ios-resolve-app.py <name> [config=Debug]" )
    name = sys.argv[ 1 ]
    config = sys.argv[ 2 ] if len( sys.argv ) == 3 else "Debug"
    objects = ios_pbxproj.load_objects()
    print( ios_pbxproj.resolve_app_path( objects, name, config ) )


if __name__ == "__main__":
    main()
