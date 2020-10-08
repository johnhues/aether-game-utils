cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(imgui LANGUAGES CXX)
include(ExternalProject)

# C++ settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_UNICODE -DUNICODE)

find_package(OpenGL REQUIRED)

if(WIN32 AND NOT CYGWIN)
	ExternalProject_Add(glew
		PREFIX glew
		URL https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0-win32.zip
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	set(GLEW_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/glew/src/glew/include)
endif()

add_library(imgui)
if(WIN32 AND NOT CYGWIN)
	add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)
elseif(APPLE)
	add_definitions("-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM=<OpenGL/gl3.h>")
elseif(UNIX)
	add_definitions("-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM=<GLES3/gl3.h>")
endif()

target_include_directories(imgui
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/examples
		${OPENGL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIRS}
)

target_sources(imgui
	PRIVATE
		imgui.cpp
		imgui_demo.cpp
		imgui_draw.cpp
		imgui_widgets.cpp
		examples/imgui_impl_opengl3.cpp
	PUBLIC
		imgui.h
)
