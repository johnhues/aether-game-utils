cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
Include(FetchContent)
Include(${AE_ROOT_DIR}/AddBundle.cmake)

# imgui
add_definitions(-DSTB_TEXTEDIT_memmove=memmove) # This fixes a c++ modules issue where std string.h is included inside another namespace causing a conflict
FetchContent_Declare(
	imgui
	GIT_REPOSITORY "https://github.com/johnhues/imgui.git"
	GIT_TAG 7a4fe3d930fcc93d9579df06f046d0738ef06391
)
FetchContent_MakeAvailable(imgui)

# imguizmo
FetchContent_Declare(
	imguizmo
	GIT_REPOSITORY "https://github.com/johnhues/ImGuizmo.git"
	GIT_TAG 6e1ea45af646c06bee0e70d93d8cc5d3f2ae45c2
)
FetchContent_MakeAvailable(imguizmo)

# rapidjson
FetchContent_Declare(
	rapidjson
	GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
	GIT_TAG 27c3a8dc0e2c9218fe94986d249a12b5ed838f1d
)
FetchContent_Populate(rapidjson) # Download but don't build
file(GLOB_RECURSE rapidjson_SOURCES CONFIGURE_DEPENDS "${rapidjson_SOURCE_DIR}/include/*")
add_library(rapidjson INTERFACE) # Header only
target_include_directories(rapidjson INTERFACE "${rapidjson_SOURCE_DIR}/include")

# Config
set(AE_EXAMPLE_DEVELOPMENT_TEAM "" CACHE STRING "Apple Development Team ID for aether examples")
set(AE_EXAMPLE_SOURCES Common.h)
if(EMSCRIPTEN)
	add_compile_definitions(DATA_DIR="https://raw.githubusercontent.com/johnhues/aether-game-utils/master/examples/data")
endif()
if(EMSCRIPTEN)
	set(AE_EXAMPLE_RESOURCES "index.html;index.webmanifest;data/Icon.png")
elseif(APPLE)
	set(AE_EXAMPLE_RESOURCES "data/Icon.icns")
endif()

# Window
ae_add_bundle(
	BUNDLE_NAME "Example"
	EXECUTABLE_NAME "01_example"
	BUNDLE_ID "ae.01_example"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "01_Example.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)

# Sprites
ae_add_bundle(
	BUNDLE_NAME "Sprites"
	EXECUTABLE_NAME "02_sprites"
	BUNDLE_ID "ae.02_sprites"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "02_Sprites.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/circle.tga"
	LIBS "ae;ae_extras"
)

# Client / Server
ae_add_bundle(
	BUNDLE_NAME "Client"
	EXECUTABLE_NAME "03_client"
	BUNDLE_ID "ae.03_client"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "03_Client.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)
if(NOT EMSCRIPTEN)
	ae_add_bundle(
		BUNDLE_NAME "Server"
		EXECUTABLE_NAME "03_server"
		BUNDLE_ID "ae.03_server"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		ICNS_FILE "data/Icon.icns"
		SRC_FILES "03_Server.cpp"
		RESOURCES "${AE_EXAMPLE_RESOURCES}"
		LIBS "ae"
	)
endif()

# Network Objects
ae_add_bundle(
	BUNDLE_NAME "NetObject Client"
	EXECUTABLE_NAME "04_net_object_client"
	BUNDLE_ID "ae.04_net_object_client"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "04_NetObjectClient.cpp;04_NetObjectCommon.h"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae_net"
)
if(NOT EMSCRIPTEN)
	ae_add_bundle(
		BUNDLE_NAME "NetObject Server"
		EXECUTABLE_NAME "04_net_object_server"
		BUNDLE_ID "ae.04_net_object_server"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		ICNS_FILE "data/Icon.icns"
		SRC_FILES "04_NetObjectServer.cpp;04_NetObjectCommon.h"
		RESOURCES "${AE_EXAMPLE_RESOURCES}"
		LIBS "ae_net"
	)
endif()

# Text Input
ae_add_bundle(
	BUNDLE_NAME "Text Input"
	EXECUTABLE_NAME "05_text_input"
	BUNDLE_ID "ae.05_text_input"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "05_TextInput.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/font.tga"
	LIBS "ae"
)

# Triangle
ae_add_bundle(
	BUNDLE_NAME "Triangle"
	EXECUTABLE_NAME "06_triangle"
	BUNDLE_ID "ae.06_triangle"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "06_Triangle.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)

# Cube
ae_add_bundle(
	BUNDLE_NAME "Cube"
	EXECUTABLE_NAME "07_cube"
	BUNDLE_ID "ae.07_cube"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "07_Cube.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)

# Audio
ae_add_bundle(
	BUNDLE_NAME "Audio"
	EXECUTABLE_NAME "08_audio"
	BUNDLE_ID "ae.08_audio"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "08_Audio.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/music.wav;data/sound.wav"
	LIBS "ae"
)

# Platformer2D
ae_add_bundle(
	BUNDLE_NAME "Platformer2D"
	EXECUTABLE_NAME "09_platformer2d"
	BUNDLE_ID "ae.09_platformer2d"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "09_Platformer2D.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae_extras"
)

# Splines
ae_add_bundle(
	BUNDLE_NAME "Splines"
	EXECUTABLE_NAME "10_splines"
	BUNDLE_ID "ae.10_splines"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "10_Splines.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/circle.tga"
	LIBS "ae;ae_extras"
)

# Terrain
ae_add_bundle(
	BUNDLE_NAME "Terrain"
	EXECUTABLE_NAME "11_terrain"
	BUNDLE_ID "ae.11_terrain"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "11_Terrain.cpp;${AE_ROOT_DIR}/extras/aeImGui.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/font.tga"
	LIBS "ae;ae_extras;imgui;imguizmo"
)

# Geometry
ae_add_bundle(
	BUNDLE_NAME "Geometry"
	EXECUTABLE_NAME "12_geometry"
	BUNDLE_ID "ae.12_geometry"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "12_Geometry.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/font.tga"
	LIBS "ae;ae_extras"
)

if(NOT EMSCRIPTEN)
	# FileDialogs
	ae_add_bundle(
		BUNDLE_NAME "File Dialogs"
		EXECUTABLE_NAME "13_file_dialogs"
		BUNDLE_ID "ae.13_file_dialogs"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		ICNS_FILE "data/Icon.icns"
		SRC_FILES "13_FileDialogs.cpp"
		RESOURCES "${AE_EXAMPLE_RESOURCES}"
		LIBS "ae"
	)
endif()

# OBJViewer
ae_add_bundle(
	BUNDLE_NAME "OBJViewer"
	EXECUTABLE_NAME "15_obj_viewer"
	BUNDLE_ID "ae.15_obj_viewer"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "15_OBJViewer.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/bunny.obj"
	LIBS "ae"
)

# SkinnedMesh
if(AE_LOADERS_OFBX)
	ae_add_bundle(
		BUNDLE_NAME "SkinnedMesh"
		EXECUTABLE_NAME "16_skinned_mesh"
		BUNDLE_ID "ae.16_skinned_mesh"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		ICNS_FILE "data/Icon.icns"
		SRC_FILES "16_SkinnedMesh.cpp"
		RESOURCES "${AE_EXAMPLE_RESOURCES};data/character.fbx;data/character.tga"
		LIBS "ae;ae_loaders"
	)

	# IK
	ae_add_bundle(
		BUNDLE_NAME "IK"
		EXECUTABLE_NAME "17_ik"
		BUNDLE_ID "ae.17_ik"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		ICNS_FILE "data/Icon.icns"
		SRC_FILES "17_IK.cpp;${AE_ROOT_DIR}/extras/aeImGui.cpp"
		RESOURCES "${AE_EXAMPLE_RESOURCES};data/character.fbx;data/character.tga"
		LIBS "ae;ae_loaders;ae_extras;imgui;imguizmo"
	)
endif()

# Editor
ae_add_bundle(
	BUNDLE_NAME "Editor"
	EXECUTABLE_NAME "18_editor"
	BUNDLE_ID "ae.18_editor"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "18_Editor.cpp;${AE_ROOT_DIR}/extras/Editor.cpp;${AE_ROOT_DIR}/extras/Entity.cpp;${AE_ROOT_DIR}/extras/aeImGui.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/example.level;data/bunny.obj;data/character.obj;data/character.tga"
	LIBS "ae_extras;imgui;imguizmo;rapidjson"
)

# Mouse
ae_add_bundle(
	BUNDLE_NAME "Mouse"
	EXECUTABLE_NAME "19_mouse"
	BUNDLE_ID "ae.19_mouse"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "19_Mouse.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)

# Render Target
ae_add_bundle(
	BUNDLE_NAME "Render Target"
	EXECUTABLE_NAME "20_render_target"
	BUNDLE_ID "ae.20_render_target"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "20_RenderTarget.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)

# Instancing
ae_add_bundle(
	BUNDLE_NAME "Instancing"
	EXECUTABLE_NAME "21_instancing"
	BUNDLE_ID "ae.21_instancing"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "21_Instancing.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)

# Read Me
ae_add_bundle(
	BUNDLE_NAME "Read Me"
	EXECUTABLE_NAME "22_read_me"
	BUNDLE_ID "ae.22_read_me"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "22_ReadMe.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES};data/level.obj;data/level.tga"
	INCLUDE_DIRS "${AE_ROOT_DIR}"
)
if(APPLE)
	set_source_files_properties("22_ReadMe.cpp" PROPERTIES COMPILE_FLAGS "-x objective-c++")
	target_compile_options("22_read_me" PUBLIC "-fmodules;-fcxx-modules")
elseif(EMSCRIPTEN)
elseif(UNIX)
	target_link_libraries("22_read_me" PUBLIC GL)
endif()

if(NOT EMSCRIPTEN)
	# Frameworks
	ae_add_shared_library(
		NAME "23_Framework"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		SRC_FILES "23_SharedLibrary.cpp"
		LIBS "ae"
	)
	ae_add_bundle(
		BUNDLE_NAME "Shared Library"
		EXECUTABLE_NAME "23_shared_library"
		BUNDLE_ID "ae.23_shared_library"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		ICNS_FILE "data/Icon.icns"
		SRC_FILES "23_Loader.cpp"
		RESOURCES "${AE_EXAMPLE_RESOURCES}"
		PACKAGE_LIBS "23_Framework"
	)

	# Hot Load
	add_library(HotLoadable SHARED "24_HotLoadable.cpp;24_HotLoad.h")
	target_compile_options(HotLoadable PUBLIC "-fvisibility=hidden") # See AE_EXPORT for more info
	set_target_properties(HotLoadable PROPERTIES PREFIX "")
	target_link_libraries(HotLoadable PUBLIC ae_shared) # Use shared library version of aether so symbols aren't duplicated
	ae_add_bundle(
		BUNDLE_NAME "Hot Load"
		EXECUTABLE_NAME "24_hot_load"
		BUNDLE_ID "ae.24_hot_load"
		APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
		MAJOR_MINOR_PATCH_VERSION "0.0.0"
		ICNS_FILE "data/Icon.icns"
		SRC_FILES "24_HotLoad.cpp;24_HotLoad.h"
		RESOURCES "${AE_EXAMPLE_RESOURCES}"
		PACKAGE_LIBS "ae_shared" # Use shared library version of aether so symbols aren't duplicated in HotLoadable library
	)
	add_dependencies(24_hot_load HotLoadable) # Make sure HotLoadable is built
	if(APPLE)
		set_target_properties(HotLoadable PROPERTIES
			# Output built HotLoadable library into app bundle
			XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
			XCODE_ATTRIBUTE_INSTALL_PATH "Applications/Hot Load.app/Contents/Resources/data"
			ARCHIVE_OUTPUT_DIRECTORY "$<TARGET_FILE:24_hot_load>/../../Resources/data"
			LIBRARY_OUTPUT_DIRECTORY "$<TARGET_FILE:24_hot_load>/../../Resources/data"
			RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE:24_hot_load>/../../Resources/data"
		)
	else()
		# @TODO
	endif()

endif()

# A Star
ae_add_bundle(
	BUNDLE_NAME "A Star"
	EXECUTABLE_NAME "25_a_star"
	BUNDLE_ID "ae.25_a_star"
	APPLE_DEVELOPMENT_TEAM "${AE_EXAMPLE_DEVELOPMENT_TEAM}"
	MAJOR_MINOR_PATCH_VERSION "0.0.0"
	ICNS_FILE "data/Icon.icns"
	SRC_FILES "25_AStar.cpp"
	RESOURCES "${AE_EXAMPLE_RESOURCES}"
	LIBS "ae"
)
