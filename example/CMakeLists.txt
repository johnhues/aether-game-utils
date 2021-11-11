cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/safeguards.cmake)
Include(FetchContent)

# Bundle helper
function(add_bundle _AE_BUNDLE_NAME _AE_EXECUTABLE_NAME _AE_BUNDLE_ID _AE_BUNDLE_VERSION _AE_ICNS_FILE  _AE_SRC_FILES _AE_RESOURCES _AE_LIBS _AE_INCLUDE_DIRS)
	message(STATUS "_AE_BUNDLE_NAME ${_AE_BUNDLE_NAME}")
	message(STATUS "_AE_EXECUTABLE_NAME ${_AE_EXECUTABLE_NAME}")
	message(STATUS "_AE_BUNDLE_ID ${_AE_BUNDLE_ID}")
	message(STATUS "_AE_BUNDLE_VERSION ${_AE_BUNDLE_VERSION}")
	message(STATUS "_AE_ICNS_FILE ${_AE_ICNS_FILE}")
	message(STATUS "_AE_SRC_FILES ${_AE_SRC_FILES}")
	message(STATUS "_AE_RESOURCES ${_AE_RESOURCES}")
	message(STATUS "_AE_LIBS ${_AE_LIBS}")
	message(STATUS "_AE_INCLUDE_DIRS ${_AE_INCLUDE_DIRS}")
	message(STATUS "")

	if(WIN32)
		# Create a regular windowed application instead of the default console subsystem target
		set(_AE_EXE_TYPE WIN32)
		# Use main instead of WinMain
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
	endif()

	add_executable(${_AE_EXECUTABLE_NAME} ${_AE_EXE_TYPE} ${_AE_SRC_FILES} ${_AE_RESOURCES})
	if (_AE_LIBS)
		target_link_libraries(${_AE_EXECUTABLE_NAME} ${_AE_LIBS})
	endif()
	if (_AE_INCLUDE_DIRS)
		target_include_directories(${_AE_EXECUTABLE_NAME} PRIVATE ${_AE_INCLUDE_DIRS})
	endif()

	if(APPLE)
		set(CMAKE_OSX_SYSROOT macosx)
		set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

		set_target_properties(${_AE_EXECUTABLE_NAME} PROPERTIES
			MACOSX_BUNDLE ON
			OUTPUT_NAME "${_AE_BUNDLE_NAME}"
			RESOURCE "${_AE_RESOURCES}"
			XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES"
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "No"
			MACOSX_BUNDLE_BUNDLE_NAME "${_AE_BUNDLE_NAME}" # CFBundleName
			MACOSX_BUNDLE_BUNDLE_VERSION "${_AE_BUNDLE_VERSION}" # CFBundleVersion
			MACOSX_BUNDLE_GUI_IDENTIFIER "${_AE_BUNDLE_ID}" # CFBundleIdentifier
			MACOSX_BUNDLE_ICON_FILE "${_AE_ICNS_FILE}" # CFBundleIconFile (*.icns file name without extension)
			MACOSX_BUNDLE_LONG_VERSION_STRING "${_AE_BUNDLE_VERSION}" # CFBundleLongVersionString
			MACOSX_BUNDLE_SHORT_VERSION_STRING "${_AE_BUNDLE_VERSION}" # CFBundleShortVersionString
			XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
			XCODE_ATTRIBUTE_SKIP_INSTALL "No"
		)

		set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
		if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
			# CMAKE_CURRENT_BINARY_DIR does not include CONFIG path with Xcode builds
			set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE})
		endif()
		install(CODE "
			include(BundleUtilities)
			set(BU_CHMOD_BUNDLE_ITEMS TRUE)
			fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/${_AE_BUNDLE_NAME}.app\" \"\" \"${_AE_DEPS_LIBRARY_DIRS}\")
		" COMPONENT Runtime)
	endif()
endfunction()

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
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

# Window
set(AE_CURRENT_BUNDLE_NAME "Example")
set(AE_CURRENT_EXECUTABLE_NAME 01_example)
set(AE_CURRENT_SOURCE_FILES 01_Example.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

# Sprites
set(AE_CURRENT_BUNDLE_NAME "Sprites")
set(AE_CURRENT_EXECUTABLE_NAME 02_sprites)
set(AE_CURRENT_SOURCE_FILES 02_Sprites.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns data/circle.png)
include(AddExecutable)

# Client / Server
set(AE_CURRENT_BUNDLE_NAME "Client")
set(AE_CURRENT_EXECUTABLE_NAME 03_client)
set(AE_CURRENT_SOURCE_FILES 03_Client.cpp)
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Server")
set(AE_CURRENT_EXECUTABLE_NAME 03_server)
set(AE_CURRENT_SOURCE_FILES 03_Server.cpp)
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

# Network Objects
set(AE_CURRENT_BUNDLE_NAME "NetObject Client")
set(AE_CURRENT_EXECUTABLE_NAME 04_net_object_client)
set(AE_CURRENT_SOURCE_FILES 04_NetObjectClient.cpp 04_NetObjectCommon.h ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "NetObject Server")
set(AE_CURRENT_EXECUTABLE_NAME 04_net_object_server)
set(AE_CURRENT_SOURCE_FILES 04_NetObjectServer.cpp 04_NetObjectCommon.h ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

# Input
add_bundle("Text Input"
	"05_text_input"
	"ae.05_text_input"
	"0.0.0"
	"Icon"
	"05_TextInput.cpp;${EXAMPLE_SOURCES}"
	"data/Icon.icns;data/font.png"
	"ae"
	""
)

# Triangle
set(AE_CURRENT_BUNDLE_NAME "Triangle")
set(AE_CURRENT_EXECUTABLE_NAME 06_triangle)
set(AE_CURRENT_SOURCE_FILES 06_Triangle.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

# Cube
set(AE_CURRENT_BUNDLE_NAME "Cube")
set(AE_CURRENT_EXECUTABLE_NAME 07_cube)
set(AE_CURRENT_SOURCE_FILES 07_Cube.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

# Audio
set(AE_CURRENT_BUNDLE_NAME "Audio")
set(AE_CURRENT_EXECUTABLE_NAME 08_audio)
set(AE_CURRENT_SOURCE_FILES 08_Audio.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns data/music.wav data/sound.wav)
include(AddExecutable)

# Platformer2D
add_bundle("Platformer2D"
	"09_platformer2d"
	"ae.09_platformer2d"
	"0.0.0"
	"Icon"
	"09_Platformer2D.cpp;${EXAMPLE_SOURCES}"
	"data/Icon.icns"
	"ae"
	""
)

# Splines
set(AE_CURRENT_BUNDLE_NAME "Splines")
set(AE_CURRENT_EXECUTABLE_NAME 10_splines)
set(AE_CURRENT_SOURCE_FILES 10_Splines.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns data/circle.png)
include(AddExecutable)

# Terrain
add_bundle("Terrain"
	"11_terrain"
	"ae.11_terrain"
	"0.0.0"
	"Icon"
	"11_terrain.cpp;${EXAMPLE_SOURCES}"
	"data/Icon.icns;data/terrain.png;data/font.png"
	"ae;imgui;imguizmo"
	""
)

# Geometry
set(AE_CURRENT_BUNDLE_NAME "Geometry")
set(AE_CURRENT_EXECUTABLE_NAME 12_Geometry)
set(AE_CURRENT_SOURCE_FILES 12_Geometry.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns data/font.png)
include(AddExecutable)

# FileDialogs
set(AE_CURRENT_BUNDLE_NAME "FileDialogs")
set(AE_CURRENT_EXECUTABLE_NAME 13_FileDialogs)
set(AE_CURRENT_SOURCE_FILES 13_FileDialogs.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES data/Icon.icns)
include(AddExecutable)

# Emscripten
add_subdirectory(14_Emscripten)

# OBJViewer
set(AE_CURRENT_BUNDLE_NAME "OBJViewer")
set(AE_CURRENT_EXECUTABLE_NAME 15_OBJViewer)
set(AE_CURRENT_SOURCE_FILES 15_OBJViewer.cpp ${EXAMPLE_SOURCES})
set(AE_RESOURCES "data/bunny.obj")
include(AddExecutable)
