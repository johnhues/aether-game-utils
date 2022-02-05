cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/safeguards.cmake)
Include(FetchContent)
Include(AddBundle)

# stb
add_library(stb INTERFACE)
target_sources(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

# imgui
add_definitions(-DSTB_TEXTEDIT_memmove=memmove) # This fixes a c++ modules issue where std string.h is included inside another namespace causing a conflict
FetchContent_Declare(
	imgui
	GIT_REPOSITORY "https://github.com/johnhues/imgui.git"
	GIT_TAG e25fc1e2a1ebdd38db1003c2ae2f7e281d285df5
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
	GIT_TAG v1.1.0
)
FetchContent_Populate(rapidjson) # Download but don't build
file(GLOB_RECURSE rapidjson_SOURCES CONFIGURE_DEPENDS "${rapidjson_SOURCE_DIR}/include/*")
add_library(rapidjson INTERFACE) # Header only
target_include_directories(rapidjson INTERFACE "${rapidjson_SOURCE_DIR}/include")

# openfbx
FetchContent_Declare(
	openfbx
	GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
	GIT_TAG 5ce5c3490a90d346366abcda48e1d77526d09a15
)
FetchContent_Populate(openfbx) # Download but don't build
FetchContent_GetProperties(openfbx SOURCE_DIR OPENFBX_SOURCE_DIR)
file(GLOB_RECURSE OPENFBX_SOURCES CONFIGURE_DEPENDS "${OPENFBX_SOURCE_DIR}/src/*")
add_library(openfbx ${OPENFBX_SOURCES})
target_include_directories(openfbx INTERFACE "${OPENFBX_SOURCE_DIR}/src")

# Example common
set(AE_EXAMPLE_SOURCES Common.h)

# Hello World
add_bundle("Hello World"
	"00_hello_world"
	"ae.00_hello_world"
	"0.0.0"
	"Icon"
	"00_HelloWorld.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Window
add_bundle("Example"
	"01_example"
	"ae.01_example"
	"0.0.0"
	"Icon"
	"01_Example.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Sprites
add_bundle("Sprites"
	"02_sprites"
	"ae.02_sprites"
	"0.0.0"
	"Icon"
	"02_Sprites.cpp;${AE_EXAMPLE_SOURCES};${AE_LOADER_INCLUDE};${AE_STB_LOADER_SRC}"
	"data/Icon.icns;data/circle.png"
	"ae;stb"
	""
)

# Client / Server
add_bundle("Client"
	"03_client"
	"ae.03_client"
	"0.0.0"
	"Icon"
	"03_Client.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)
add_bundle("Server"
	"03_server"
	"ae.03_server"
	"0.0.0"
	"Icon"
	"03_Server.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Network Objects
add_bundle("NetObject Client"
	"04_net_object_client"
	"ae.04_net_object_client"
	"0.0.0"
	"Icon"
	"04_NetObjectClient.cpp;04_NetObjectCommon.h;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)
add_bundle("NetObject Server"
	"04_net_object_server"
	"ae.04_net_object_server"
	"0.0.0"
	"Icon"
	"04_NetObjectServer.cpp;04_NetObjectCommon.h;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Input
add_bundle("Text Input"
	"05_text_input"
	"ae.05_text_input"
	"0.0.0"
	"Icon"
	"05_TextInput.cpp;${AE_EXAMPLE_SOURCES};${AE_LOADER_INCLUDE};${AE_STB_LOADER_SRC}"
	"data/Icon.icns;data/font.png"
	"ae;stb"
	""
)

# Triangle
add_bundle("Triangle"
	"06_triangle"
	"ae.06_triangle"
	"0.0.0"
	"Icon"
	"06_Triangle.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Cube
add_bundle("Cube"
	"07_cube"
	"ae.07_cube"
	"0.0.0"
	"Icon"
	"07_Cube.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Audio
add_bundle("Audio"
	"08_audio"
	"ae.08_audio"
	"0.0.0"
	"Icon"
	"08_Audio.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns;data/music.wav;data/sound.wav"
	"ae"
	""
)

# Platformer2D
add_bundle("Platformer2D"
	"09_platformer2d"
	"ae.09_platformer2d"
	"0.0.0"
	"Icon"
	"09_Platformer2D.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Splines
add_bundle("Splines"
	"10_splines"
	"ae.10_splines"
	"0.0.0"
	"Icon"
	"10_Splines.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns;data/circle.png"
	"ae"
	""
)

# Terrain
add_bundle("Terrain"
	"11_terrain"
	"ae.11_terrain"
	"0.0.0"
	"Icon"
	"11_terrain.cpp;${AE_EXAMPLE_SOURCES};${AE_LOADER_INCLUDE};${AE_STB_LOADER_SRC}"
	"data/Icon.icns;data/font.png"
	"ae;imgui;imguizmo;stb"
	""
)

# Geometry
add_bundle("Geometry"
	"12_geometry"
	"ae.12_geometry"
	"0.0.0"
	"Icon"
	"12_Geometry.cpp;${AE_EXAMPLE_SOURCES};${AE_LOADER_INCLUDE};${AE_STB_LOADER_SRC}"
	"data/Icon.icns;data/font.png"
	"ae;stb"
	""
)

# FileDialogs
add_bundle("File Dialogs"
	"13_file_dialogs"
	"ae.13_file_dialogs"
	"0.0.0"
	"Icon"
	"13_FileDialogs.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Emscripten
add_subdirectory(14_Emscripten)

# OBJViewer
add_bundle("OBJViewer"
	"15_obj_viewer"
	"ae.15_obj_viewer"
	"0.0.0"
	"Icon"
	"15_OBJViewer.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns;data/bunny.obj"
	"ae;imgui;imguizmo"
	""
)

# SkinnedMesh
add_bundle("SkinnedMesh"
	"16_skinned_mesh"
	"ae.16_skinned_mesh"
	"0.0.0"
	"Icon"
	"16_SkinnedMesh.cpp;${AE_EXAMPLE_SOURCES};${AE_LOADER_INCLUDE};${AE_OFBX_LOADER_SRC}"
	"data/Icon.icns;data/character.fbx;data/character.tga"
	"ae;openfbx"
	""
)

# IK
add_bundle("IK"
	"17_ik"
	"ae.17_ik"
	"0.0.0"
	"Icon"
	"17_IK.cpp;${AE_EXAMPLE_SOURCES};${AE_LOADER_INCLUDE};${AE_OFBX_LOADER_SRC}"
	"data/Icon.icns;data/character.fbx;data/character.tga"
	"ae;openfbx;imgui;imguizmo"
	""
)

# Editor
add_bundle("Editor"
	"18_editor"
	"ae.18_editor"
	"0.0.0"
	"Icon"
	"18_editor.cpp;${CMAKE_SOURCE_DIR}/src/aether.mm;${CMAKE_SOURCE_DIR}/include/ae/Editor.h;${CMAKE_SOURCE_DIR}/src/Editor.cpp;${CMAKE_SOURCE_DIR}/src/Entity.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns;data/example.level;data/bunny.obj;data/character.obj;data/character.tga"
	"ae;imgui;imguizmo;rapidjson"
	"${CMAKE_SOURCE_DIR}/include"
)

# Mouse
add_bundle("Mouse"
	"19_mouse"
	"ae.19_mouse"
	"0.0.0"
	"Icon"
	"19_Mouse.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Render Target
add_bundle("Render Target"
	"20_render_target"
	"ae.20_render_target"
	"0.0.0"
	"Icon"
	"20_RenderTarget.cpp;${AE_EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)
