cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
Include(FetchContent)
Include(${AE_ROOT_DIR}/AddBundle.cmake)

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
	set(AE_EXAMPLE_RESOURCES "index.html;index.webmanifest;data/Icon.png")
elseif(APPLE)
	set(AE_EXAMPLE_RESOURCES "data/Icon.icns")
endif()

# Hello World
add_bundle("Hello World"
	"00_hello_world"
	"ae.00_hello_world"
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
		"" # No Apple Development Team
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
		"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
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
	"" # No Apple Development Team
	"0.0.0"
	"data/Icon.icns"
	"21_Instancing.cpp"
	"${AE_EXAMPLE_RESOURCES}"
	"ae"
	""
)

# Read Me
if(APPLE)
	set_source_files_properties("22_ReadMe.cpp" PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()
add_bundle("Read Me"
	"22_read_me"
	"ae.22_read_me"
	"" # No Apple Development Team
	"0.0.0"
	"data/Icon.icns"
	"22_ReadMe.cpp"
	"${AE_EXAMPLE_RESOURCES};data/level.obj;data/level.tga"
	""
	"${AE_ROOT_DIR}"
)
target_compile_definitions("22_read_me" PUBLIC DATA_DIR="data")
# target_compile_definitions(22_read_me PUBLIC DATA_DIR="https://raw.githubusercontent.com/johnhues/aether-game-utils/master/examples/data")
