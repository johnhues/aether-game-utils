cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
if (("${CMAKE_GENERATOR}" STREQUAL "Xcode"))
	set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "" CACHE STRING "") # See Professional CMake 24.8.1. Device-only Bundles
	set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)" CACHE STRING "") # See Professional CMake 24.8.1. Device-only Bundles
endif()

# aether-game-utils project
project(aether-game-utils LANGUAGES CXX C VERSION 0.1.0)

# Configuration variables intended for external use
set(AE_LEAN_AND_MEAN ON CACHE BOOL "Disables building tests and examples")
set(AE_LOADERS_OFBX OFF CACHE BOOL "Enables aether FBX loader and ofbx dependency")
set(AE_LOADERS_STB OFF CACHE BOOL "Enables aether PNG loader and stb dependency")

# Configuration for building ae on its own
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	# Safeguard against in-source builds
	set(CMAKE_DISABLE_SOURCE_CHANGES ON)
	set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
	if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
		message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
	endif()

	# Default to RelWithDebInfo when not specified for single-config generators
	if (NOT GENERATOR_IS_MULTI_CONFIG AND (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL ""))
		set(CMAKE_BUILD_TYPE "RelWithDebInfo")
	endif()
endif()

# Defines and configuration
set(AE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17) # use c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
	add_definitions(-D_UNICODE -DUNICODE) # use unicode
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) # disable standard library warnings on windows
	list(APPEND AE_DEPS_LIBRARIES version Winmm Imm32 Setupapi Ws2_32)
endif()

# OpenGL dependency
if (IOS_TOOLCHAIN_HAS_RUN)
	set(OPENGL_FRAMEWORK "${CMAKE_OSX_SYSROOT_INT}/System/Library/Frameworks/OpenGLES.framework")
	if (EXISTS ${OPENGL_FRAMEWORK})
		message(STATUS "Found OpenGLES")
		list(APPEND AE_DEPS_LIBRARIES ${OPENGL_FRAMEWORK})
	else()
		message(SEND_ERROR "Could not find OpenGLES")
	endif()
elseif(EMSCRIPTEN)
	list(APPEND AE_DEPS_LIBRARIES GL)
else()
	set(OpenGL_GL_PREFERENCE GLVND) # See https://cmake.org/cmake/help/latest/policy/CMP0072.html
	find_package(OpenGL REQUIRED)
	list(APPEND AE_DEPS_LIBRARIES ${OPENGL_LIBRARIES})
endif()

# OpenAL dependency
if (IOS_TOOLCHAIN_HAS_RUN)
	set(OPENAL_LIBRARY "${CMAKE_OSX_SYSROOT_INT}/System/Library/Frameworks/OpenAL.framework")
	if (EXISTS ${OPENAL_LIBRARY})
		message(STATUS "Found OpenAL")
		list(APPEND AE_DEPS_LIBRARIES ${OPENAL_LIBRARY})
	else()
		message(SEND_ERROR "Could not find OpenAL")
	endif()
endif()

# ae library
function(add_ae_library NAME)
	set_target_properties(${NAME} PROPERTIES DEBUG_POSTFIX "d")
	if (APPLE)
		set_target_properties(${NAME} PROPERTIES XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET 13.0) # See Professional CMake 24.5.3 Deployment Target
		# @TODO: Other Apple platforms
	endif()
	if (AE_DEPS_LIBRARIES)
		target_link_libraries(${NAME} ${AE_DEPS_LIBRARIES})
	endif()
	if (AE_DEPS_TARGETS)
		add_dependencies(${NAME} ${AE_DEPS_TARGETS})
	endif()
	if (AE_DEPS_INCLUDE)
		target_include_directories(${NAME} PUBLIC ${AE_DEPS_INCLUDE})
	endif()
	target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
	if(APPLE)
		target_sources(${NAME} PRIVATE aether.h aether.mm)
		if (AE_USE_MODULES)
			target_compile_definitions(${NAME} PUBLIC AE_USE_MODULES)
			target_compile_options(${NAME} PUBLIC "-fmodules;-fcxx-modules")
		else()
			find_library(OPENAL_LIBRARY OpenAL REQUIRED)
			find_library(CARBON_LIBRARY Carbon REQUIRED)
			find_library(APPKIT_LIBRARY AppKit REQUIRED)
			find_library(GAMECONTROLLER_LIBRARY GameController REQUIRED)
			target_link_libraries(${NAME} ${OPENAL_LIBRARY} ${CARBON_LIBRARY} ${APPKIT_LIBRARY} ${GAMECONTROLLER_LIBRARY})
		endif()
	else()
		target_sources(${NAME} PRIVATE aether.h aether.cpp)
	endif()
endfunction()
add_library(ae STATIC "") # static library
add_library(ae_shared SHARED "") # shared library (for hot loading)
add_ae_library(ae)
add_ae_library(ae_shared)

# Examples, extra utilities, and tests
add_subdirectory(loaders)
if(NOT ${AE_LEAN_AND_MEAN})
	add_subdirectory(extras)
	add_subdirectory(net)
	add_subdirectory(examples)
	add_subdirectory(test)
endif()
