"""
ios_pbxproj.py

Helpers for reading build_ios/aether-game-utils.xcodeproj. Shared by
ios_build.py and ios-resolve-app.py — pbxproj is an external file-format
contract, so the parsing lives in one place.
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


def _settings_for( objects, target_obj, config ):
    config_list = objects.get( target_obj.get( "buildConfigurationList", "" ), {} )
    for cid in config_list.get( "buildConfigurations", [] ):
        c = objects.get( cid, {} )
        if c.get( "name" ) == config:
            return c.get( "buildSettings", {} )
    return {}


def resolve_target( objects, name, config="Debug" ):
    """Resolve a CMake target name (e.g. '06_triangle') or PRODUCT_NAME
    display name (e.g. 'Triangle') to ( cmake_target_name, product_name ).
    Errors out if no match."""
    targets = [
        obj for obj in objects.values()
        if isinstance( obj, dict ) and obj.get( "isa" ) == "PBXNativeTarget"
    ]
    for t in targets:
        if t.get( "name" ) == name:
            product = _settings_for( objects, t, config ).get( "PRODUCT_NAME" ) or name
            return name, product
    for t in targets:
        if _settings_for( objects, t, config ).get( "PRODUCT_NAME" ) == name:
            return t[ "name" ], name
    sys.exit( f"ERROR: no Xcode target matches '{name}' (checked target name and PRODUCT_NAME)" )


def resolve_app_path( objects, name, config="Debug", platform="-iphoneos" ):
    """Resolve a CMake target name or PRODUCT_NAME display name to the
    expected on-disk .app path, derived statically from pbxproj's per-config
    CONFIGURATION_BUILD_DIR + PRODUCT_NAME. Does not check disk."""
    target_name, product = resolve_target( objects, name, config )
    target = next(
        t for t in objects.values()
        if isinstance( t, dict ) and t.get( "isa" ) == "PBXNativeTarget"
        and t.get( "name" ) == target_name
    )
    settings = _settings_for( objects, target, config )
    build_dir = settings.get( "CONFIGURATION_BUILD_DIR" )
    if not build_dir:
        sys.exit( f"ERROR: no CONFIGURATION_BUILD_DIR for '{target_name}' [{config}]" )
    build_dir = build_dir.replace( "${EFFECTIVE_PLATFORM_NAME}", platform )
    return f"{build_dir}/{product}.app"
