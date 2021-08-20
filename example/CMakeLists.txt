cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/safeguards.cmake)
Include(FetchContent)

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

# Example common
set(EXAMPLE_SOURCES Common.h stb_image.h)

# Hello World
set(AE_CURRENT_BUNDLE_NAME "Hello World")
set(AE_CURRENT_EXECUTABLE_NAME 00_hello_world)
set(AE_CURRENT_SOURCE_FILES 00_HelloWorld.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

# Window
set(AE_CURRENT_BUNDLE_NAME "Example")
set(AE_CURRENT_EXECUTABLE_NAME 01_example)
set(AE_CURRENT_SOURCE_FILES 01_Example.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

# Sprites
set(AE_CURRENT_BUNDLE_NAME "Sprites")
set(AE_CURRENT_EXECUTABLE_NAME 02_sprites)
set(AE_CURRENT_SOURCE_FILES 02_Sprites.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/circle.png)
include(AddExecutable)

# Client / Server
set(AE_CURRENT_BUNDLE_NAME "Client")
set(AE_CURRENT_EXECUTABLE_NAME 03_client)
set(AE_CURRENT_SOURCE_FILES 03_Client.cpp)
set(AE_RESOURCES "")
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Server")
set(AE_CURRENT_EXECUTABLE_NAME 03_server)
set(AE_CURRENT_SOURCE_FILES 03_Server.cpp)
set(AE_RESOURCES "")
include(AddExecutable)

# Network Objects
set(AE_CURRENT_BUNDLE_NAME "NetObject Client")
set(AE_CURRENT_EXECUTABLE_NAME 04_net_object_client)
set(AE_CURRENT_SOURCE_FILES 04_NetObjectClient.cpp 04_NetObjectCommon.h ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "NetObject Server")
set(AE_CURRENT_EXECUTABLE_NAME 04_net_object_server)
set(AE_CURRENT_SOURCE_FILES 04_NetObjectServer.cpp 04_NetObjectCommon.h ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

# Input
set(AE_CURRENT_BUNDLE_NAME "Text Input")
set(AE_CURRENT_EXECUTABLE_NAME 05_text_input)
set(AE_CURRENT_SOURCE_FILES 05_TextInput.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/font.png)
include(AddExecutable)

# Triangle
set(AE_CURRENT_BUNDLE_NAME "Triangle")
set(AE_CURRENT_EXECUTABLE_NAME 06_triangle)
set(AE_CURRENT_SOURCE_FILES 06_Triangle.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

# Cube
set(AE_CURRENT_BUNDLE_NAME "Cube")
set(AE_CURRENT_EXECUTABLE_NAME 07_cube)
set(AE_CURRENT_SOURCE_FILES 07_Cube.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

# Audio
set(AE_CURRENT_BUNDLE_NAME "Audio")
set(AE_CURRENT_EXECUTABLE_NAME 08_audio)
set(AE_CURRENT_SOURCE_FILES 08_Audio.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/music.wav data/sound.wav)
include(AddExecutable)

# Platformer2D
set(AE_CURRENT_BUNDLE_NAME "Platformer2D")
set(AE_CURRENT_EXECUTABLE_NAME 09_platformer2d)
set(AE_CURRENT_SOURCE_FILES 09_Platformer2D.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

# Splines
set(AE_CURRENT_BUNDLE_NAME "Splines")
set(AE_CURRENT_EXECUTABLE_NAME 10_splines)
set(AE_CURRENT_SOURCE_FILES 10_Splines.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/circle.png)
include(AddExecutable)

# Terrain
set(AE_CURRENT_BUNDLE_NAME "Terrain")
set(AE_CURRENT_EXECUTABLE_NAME 11_terrain)
set(AE_CURRENT_SOURCE_FILES 11_Terrain.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/terrain.png data/font.png)
include(AddExecutable)

# Geometry
set(AE_CURRENT_BUNDLE_NAME "Geometry")
set(AE_CURRENT_EXECUTABLE_NAME 12_Geometry)
set(AE_CURRENT_SOURCE_FILES 12_Geometry.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/font.png)
include(AddExecutable)

# FileDialogs
set(AE_CURRENT_BUNDLE_NAME "FileDialogs")
set(AE_CURRENT_EXECUTABLE_NAME 13_FileDialogs)
set(AE_CURRENT_SOURCE_FILES 13_FileDialogs.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES "")
include(AddExecutable)

add_subdirectory(14_Emscripten)
