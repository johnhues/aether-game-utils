cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/safeguards.cmake)
Include(FetchContent)
Include(AddBundle)

# imgui
add_definitions(-DSTB_TEXTEDIT_memmove=memmove) # This fixes a c++ modules issue where std string.h is included inside another namespace causing a conflict
FetchContent_Declare(
	imgui
	GIT_REPOSITORY "https://github.com/johnhues/imgui.git"
	GIT_TAG 526e2764bc15a3482bd7b550471e96f3c5454864
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

# Example source helpers
set(AE_EXAMPLE_SOURCES Common.h)
if(EMSCRIPTEN)
	set(AE_EXAMPLE_RESOURCES "index.html")
elseif(APPLE)
	set(AE_EXAMPLE_RESOURCES "data/Icon.icns")
endif()

# Hello World
add_bundle("Hello World"
	"00_hello_world"
	"ae.00_hello_world"
	"0.0.0"
	"data/Icon.icns"
	"00_HelloWorld.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Window
add_bundle("Example"
	"01_example"
	"ae.01_example"
	"0.0.0"
	"data/Icon.icns"
	"01_Example.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Sprites
add_bundle("Sprites"
	"02_sprites"
	"ae.02_sprites"
	"0.0.0"
	"data/Icon.icns"
	"02_Sprites.cpp"
	"${AE_EXAMPLE_RESOURCES};data/circle.tga"
	"ae;ae_extras"
	""
)

# Client / Server
add_bundle("Client"
	"03_client"
	"ae.03_client"
	"0.0.0"
	"data/Icon.icns"
	"03_Client.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)
add_bundle("Server"
	"03_server"
	"ae.03_server"
	"0.0.0"
	"data/Icon.icns"
	"03_Server.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Network Objects
add_bundle("NetObject Client"
	"04_net_object_client"
	"ae.04_net_object_client"
	"0.0.0"
	"data/Icon.icns"
	"04_NetObjectClient.cpp;04_NetObjectCommon.h"
	"${AE_EXAMPLE_RESOURCES}"
	"ae_net"
	""
)
add_bundle("NetObject Server"
	"04_net_object_server"
	"ae.04_net_object_server"
	"0.0.0"
	"data/Icon.icns"
	"04_NetObjectServer.cpp;04_NetObjectCommon.h"
	"${AE_EXAMPLE_RESOURCES}"
	"ae_net"
	""
)

# Text Input
add_bundle("Text Input"
	"05_text_input"
	"ae.05_text_input"
	"0.0.0"
	"data/Icon.icns"
	"05_TextInput.cpp"
	"${AE_EXAMPLE_RESOURCES};data/font.tga"
	"ae"
	""
)

# Triangle
add_bundle("Triangle"
	"06_triangle"
	"ae.06_triangle"
	"0.0.0"
	"data/Icon.icns"
	"06_Triangle.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Cube
add_bundle("Cube"
	"07_cube"
	"ae.07_cube"
	"0.0.0"
	"data/Icon.icns"
	"07_Cube.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Audio
add_bundle("Audio"
	"08_audio"
	"ae.08_audio"
	"0.0.0"
	"data/Icon.icns"
	"08_Audio.cpp"
	"${AE_EXAMPLE_RESOURCES};data/music.wav;data/sound.wav"
	"ae"
	""
)

# Platformer2D
add_bundle("Platformer2D"
	"09_platformer2d"
	"ae.09_platformer2d"
	"0.0.0"
	"data/Icon.icns"
	"09_Platformer2D.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae_extras"
	""
)

# Splines
add_bundle("Splines"
	"10_splines"
	"ae.10_splines"
	"0.0.0"
	"data/Icon.icns"
	"10_Splines.cpp"
	"${AE_EXAMPLE_RESOURCES};data/circle.tga"
	"ae;ae_extras"
	""
)

# Terrain
add_bundle("Terrain"
	"11_terrain"
	"ae.11_terrain"
	"0.0.0"
	"data/Icon.icns"
	"11_Terrain.cpp"
	"${AE_EXAMPLE_RESOURCES};data/font.tga"
	"ae;ae_extras;imgui;imguizmo"
	""
)

# Geometry
add_bundle("Geometry"
	"12_geometry"
	"ae.12_geometry"
	"0.0.0"
	"data/Icon.icns"
	"12_Geometry.cpp"
	"${AE_EXAMPLE_RESOURCES};data/font.tga"
	"ae;ae_extras"
	""
)

# FileDialogs
add_bundle("File Dialogs"
	"13_file_dialogs"
	"ae.13_file_dialogs"
	"0.0.0"
	"data/Icon.icns"
	"13_FileDialogs.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Emscripten
add_bundle("Emscripten"
	"14_emscripten"
	"ae.14_emscripten"
	"0.0.0"
	"data/Icon.icns"
	"14_Emscripten.cpp"
	"index.html;data/moon.tga;data/test.txt"
	"ae"
	""
)

# OBJViewer
add_bundle("OBJViewer"
	"15_obj_viewer"
	"ae.15_obj_viewer"
	"0.0.0"
	"data/Icon.icns"
	"15_OBJViewer.cpp"
	"${AE_EXAMPLE_RESOURCES};data/bunny.obj"
	"ae"
	""
)

# SkinnedMesh
if(AE_LOADERS_OFBX)
	add_bundle("SkinnedMesh"
		"16_skinned_mesh"
		"ae.16_skinned_mesh"
		"0.0.0"
		"data/Icon.icns"
		"16_SkinnedMesh.cpp"
		"${AE_EXAMPLE_RESOURCES};data/character.fbx;data/character.tga"
		"ae;ae_loaders"
		""
	)

	# IK
	add_bundle("IK"
		"17_ik"
		"ae.17_ik"
		"0.0.0"
		"data/Icon.icns"
		"17_IK.cpp"
		"${AE_EXAMPLE_RESOURCES};data/character.fbx;data/character.tga"
		"ae;ae_loaders;ae_extras;imgui;imguizmo"
		""
	)
endif()

# Editor
add_bundle("Editor"
	"18_editor"
	"ae.18_editor"
	"0.0.0"
	"data/Icon.icns"
	"18_Editor.cpp;${AE_ROOT_DIR}/extras/Editor.cpp;${AE_ROOT_DIR}/extras/Entity.cpp"
	"${AE_EXAMPLE_RESOURCES};data/example.level;data/bunny.obj;data/character.obj;data/character.tga"
	"ae_extras;imgui;imguizmo;rapidjson"
	""
)

# Mouse
add_bundle("Mouse"
	"19_mouse"
	"ae.19_mouse"
	"0.0.0"
	"data/Icon.icns"
	"19_Mouse.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Render Target
add_bundle("Render Target"
	"20_render_target"
	"ae.20_render_target"
	"0.0.0"
	"data/Icon.icns"
	"20_RenderTarget.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Instancing
add_bundle("Instancing"
	"21_instancing"
	"ae.21_instancing"
	"0.0.0"
	"data/Icon.icns"
	"21_Instancing.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)
