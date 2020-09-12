cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(imguizmo LANGUAGES CXX)
include(ExternalProject)

# imgui: It's difficult for two non-cmake external projects to
# have real dependencies on each other, so clone imgui (again)
# just for the headers.
ExternalProject_Add(imgui
	PREFIX ${CMAKE_BINARY_DIR}/thirdparty/imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.76
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
ExternalProject_Get_Property(imgui SOURCE_DIR)
set(IMGUI_INCLUDE_DIRS ${SOURCE_DIR})

add_library(imguizmo)

target_include_directories(imguizmo
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${IMGUI_INCLUDE_DIRS}
)

add_dependencies(imguizmo imgui)

target_sources(imguizmo
	PRIVATE
		ImGuizmo.cpp
	PUBLIC
		ImGuizmo.h
)
