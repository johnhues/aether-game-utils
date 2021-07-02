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

# Hello World
set(AE_CURRENT_BUNDLE_NAME "Hello World")
set(AE_CURRENT_EXECUTABLE_NAME 00_hello_world)
set(AE_CURRENT_SOURCE_FILES 00_HelloWorld.cpp)
set(AE_RESOURCES "")
include(AddExecutable)

# Window
set(AE_CURRENT_BUNDLE_NAME "Example")
set(AE_CURRENT_EXECUTABLE_NAME 01_example)
set(AE_CURRENT_SOURCE_FILES 01_Example.cpp)
set(AE_RESOURCES "")
include(AddExecutable)

# Sprites
set(AE_CURRENT_BUNDLE_NAME "Sprites")
set(AE_CURRENT_EXECUTABLE_NAME 02_sprites)
set(AE_CURRENT_SOURCE_FILES 02_Sprites.cpp)
set(AE_RESOURCES circle.png)
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

# Network Replication
set(AE_CURRENT_BUNDLE_NAME "Replication Client")
set(AE_CURRENT_EXECUTABLE_NAME 04_replication_client)
set(AE_CURRENT_SOURCE_FILES 04_ReplicationClient.cpp 04_ReplicationCommon.h)
set(AE_RESOURCES "")
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Replication Server")
set(AE_CURRENT_EXECUTABLE_NAME 04_replication_server)
set(AE_CURRENT_SOURCE_FILES 04_ReplicationServer.cpp 04_ReplicationCommon.h)
set(AE_RESOURCES "")
include(AddExecutable)

# Input
set(AE_CURRENT_BUNDLE_NAME "Text Input")
set(AE_CURRENT_EXECUTABLE_NAME 05_text_input)
set(AE_CURRENT_SOURCE_FILES 05_TextInput.cpp)
set(AE_RESOURCES font.png)
include(AddExecutable)

# Triangle
set(AE_CURRENT_BUNDLE_NAME "Triangle")
set(AE_CURRENT_EXECUTABLE_NAME 06_triangle)
set(AE_CURRENT_SOURCE_FILES 06_Triangle.cpp)
set(AE_RESOURCES "")
include(AddExecutable)

# Cube
set(AE_CURRENT_BUNDLE_NAME "Cube")
set(AE_CURRENT_EXECUTABLE_NAME 07_cube)
set(AE_CURRENT_SOURCE_FILES 07_Cube.cpp)
set(AE_RESOURCES "")
include(AddExecutable)

# Audio
set(AE_CURRENT_BUNDLE_NAME "Audio")
set(AE_CURRENT_EXECUTABLE_NAME 08_audio)
set(AE_CURRENT_SOURCE_FILES 08_Audio.cpp)
set(AE_RESOURCES music.wav sound.wav)
include(AddExecutable)

# Platformer2D
set(AE_CURRENT_BUNDLE_NAME "Platformer2D")
set(AE_CURRENT_EXECUTABLE_NAME 09_platformer2d)
set(AE_CURRENT_SOURCE_FILES 09_Platformer2D.cpp)
set(AE_RESOURCES "")
include(AddExecutable)

# Splines
set(AE_CURRENT_BUNDLE_NAME "Splines")
set(AE_CURRENT_EXECUTABLE_NAME 10_splines)
set(AE_CURRENT_SOURCE_FILES 10_Splines.cpp)
set(AE_RESOURCES circle.png)
include(AddExecutable)

# Terrain
set(AE_CURRENT_BUNDLE_NAME "Terrain")
set(AE_CURRENT_EXECUTABLE_NAME 11_terrain)
set(AE_CURRENT_SOURCE_FILES 11_Terrain.cpp)
set(AE_RESOURCES terrain.png font.png)
include(AddExecutable)

# Geometry
set(AE_CURRENT_BUNDLE_NAME "Geometry")
set(AE_CURRENT_EXECUTABLE_NAME 12_Geometry)
set(AE_CURRENT_SOURCE_FILES 12_Geometry.cpp)
set(AE_RESOURCES font.png)
include(AddExecutable)

# FileDialogs
set(AE_CURRENT_BUNDLE_NAME "FileDialogs")
set(AE_CURRENT_EXECUTABLE_NAME 13_FileDialogs)
set(AE_CURRENT_SOURCE_FILES 13_FileDialogs.cpp)
set(AE_RESOURCES "")
include(AddExecutable)
