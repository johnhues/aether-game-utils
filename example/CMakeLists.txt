cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/safeguards.cmake)
include(ExternalProject)

# imgui
if (NOT TARGET imgui)
	set(IMGUI_GIT_TAG 640bc056a97829bb38562954a9895f490a72a1bc)
	ExternalProject_Add(imgui
		PREFIX "${AE_THIRDPARTY_DIR}/imgui_${IMGUI_GIT_TAG}"
		GIT_REPOSITORY "https://github.com/johnhues/imgui.git"
		GIT_TAG "${IMGUI_GIT_TAG}"
		${AE_MULTI_CONFIG_BUILD_COMMAND}
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(imgui SOURCE_DIR)
	ExternalProject_Get_Property(imgui BINARY_DIR)
	set(IMGUI_INCLUDE_DIRS
		${SOURCE_DIR}
		${SOURCE_DIR}/examples
	)
	if(GENERATOR_IS_MULTI_CONFIG)
		set(IMGUI_LIBRARIES 
			optimized "${BINARY_DIR}/RelWithDebInfo/${CMAKE_STATIC_LIBRARY_PREFIX}imgui${CMAKE_STATIC_LIBRARY_SUFFIX}"
			debug "${BINARY_DIR}/Debug/${CMAKE_STATIC_LIBRARY_PREFIX}imgui${CMAKE_STATIC_LIBRARY_SUFFIX}"
		)
	else()
		set(IMGUI_LIBRARIES "${BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}imgui${CMAKE_STATIC_LIBRARY_SUFFIX}")
	endif()
endif()

# imguizmo
if (NOT TARGET imguizmo)
	set(IMGUIZMO_GIT_TAG d7a1c40e0e99f8c3c471cbe7060fbae1d11899ae)
	ExternalProject_Add(imguizmo
		PREFIX "${AE_THIRDPARTY_DIR}/imguizmo_${IMGUIZMO_GIT_TAG}"
		GIT_REPOSITORY "https://github.com/johnhues/ImGuizmo.git"
		GIT_TAG "${IMGUIZMO_GIT_TAG}"
		${AE_MULTI_CONFIG_BUILD_COMMAND}
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(imguizmo SOURCE_DIR)
	ExternalProject_Get_Property(imguizmo BINARY_DIR)
	set(IMGUIZMO_INCLUDE_DIRS ${SOURCE_DIR})
	if(GENERATOR_IS_MULTI_CONFIG)
		set(IMGUIZMO_LIBRARIES 
			optimized "${BINARY_DIR}/RelWithDebInfo/${CMAKE_STATIC_LIBRARY_PREFIX}imguizmo${CMAKE_STATIC_LIBRARY_SUFFIX}"
			debug "${BINARY_DIR}/Debug/${CMAKE_STATIC_LIBRARY_PREFIX}imguizmo${CMAKE_STATIC_LIBRARY_SUFFIX}"
		)
	else()
		set(IMGUIZMO_LIBRARIES "${BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}imguizmo${CMAKE_STATIC_LIBRARY_SUFFIX}")
	endif()
endif()

# Hello World
set(AE_CURRENT_BUNDLE_NAME "Hello World")
set(AE_CURRENT_EXECUTABLE_NAME 00_hello_world)
set(AE_CURRENT_SOURCE_FILES 00_HelloWorld.cpp)
include(AddExecutable)

# Window
set(AE_CURRENT_BUNDLE_NAME "Example")
set(AE_CURRENT_EXECUTABLE_NAME 01_example)
set(AE_CURRENT_SOURCE_FILES 01_Example.cpp)
include(AddExecutable)

# Sprites
set(AE_CURRENT_BUNDLE_NAME "Sprites")
set(AE_CURRENT_EXECUTABLE_NAME 02_sprites)
set(AE_CURRENT_SOURCE_FILES 02_Sprites.cpp circle.png)
set(AE_RESOURCES circle.png)
include(AddExecutable)

# Client / Server
set(AE_CURRENT_BUNDLE_NAME "Client")
set(AE_CURRENT_EXECUTABLE_NAME 03_client)
set(AE_CURRENT_SOURCE_FILES 03_Client.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Server")
set(AE_CURRENT_EXECUTABLE_NAME 03_server)
set(AE_CURRENT_SOURCE_FILES 03_Server.cpp)
include(AddExecutable)

# Network Replication
set(AE_CURRENT_BUNDLE_NAME "Replication Client")
set(AE_CURRENT_EXECUTABLE_NAME 04_replication_client)
set(AE_CURRENT_SOURCE_FILES 04_ReplicationClient.cpp 04_ReplicationCommon.h)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Replication Server")
set(AE_CURRENT_EXECUTABLE_NAME 04_replication_server)
set(AE_CURRENT_SOURCE_FILES 04_ReplicationServer.cpp 04_ReplicationCommon.h)
include(AddExecutable)

# Input
set(AE_CURRENT_BUNDLE_NAME "Text Input")
set(AE_CURRENT_EXECUTABLE_NAME 05_text_input)
set(AE_CURRENT_SOURCE_FILES 05_TextInput.cpp font.png)
set(AE_RESOURCES font.png)
include(AddExecutable)

# Triangle
set(AE_CURRENT_BUNDLE_NAME "Triangle")
set(AE_CURRENT_EXECUTABLE_NAME 06_triangle)
set(AE_CURRENT_SOURCE_FILES 06_Triangle.cpp)
include(AddExecutable)

# Cube
set(AE_CURRENT_BUNDLE_NAME "Cube")
set(AE_CURRENT_EXECUTABLE_NAME 07_cube)
set(AE_CURRENT_SOURCE_FILES 07_Cube.cpp)
include(AddExecutable)

# Audio
set(AE_CURRENT_BUNDLE_NAME "Audio")
set(AE_CURRENT_EXECUTABLE_NAME 08_audio)
set(AE_CURRENT_SOURCE_FILES 08_Audio.cpp music.wav sound.wav)
set(AE_RESOURCES music.wav sound.wav)
include(AddExecutable)

# Platformer2D
set(AE_CURRENT_BUNDLE_NAME "Platformer2D")
set(AE_CURRENT_EXECUTABLE_NAME 09_platformer2d)
set(AE_CURRENT_SOURCE_FILES 09_Platformer2D.cpp)
include(AddExecutable)

# Splines
set(AE_CURRENT_BUNDLE_NAME "Splines")
set(AE_CURRENT_EXECUTABLE_NAME 10_splines)
set(AE_CURRENT_SOURCE_FILES 10_Splines.cpp)
include(AddExecutable)

# Terrain
set(AE_CURRENT_BUNDLE_NAME "Terrain")
set(AE_CURRENT_EXECUTABLE_NAME 11_terrain)
set(AE_CURRENT_SOURCE_FILES 11_Terrain.cpp terrain.png font.png)
set(AE_RESOURCES terrain.png font.png)
include(AddExecutable)

# Geometry
set(AE_CURRENT_BUNDLE_NAME "Geometry")
set(AE_CURRENT_EXECUTABLE_NAME 12_Geometry)
set(AE_CURRENT_SOURCE_FILES 12_Geometry.cpp font.png)
include(AddExecutable)
