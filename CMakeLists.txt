cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
if (("${CMAKE_GENERATOR}" STREQUAL "Xcode"))
	set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "" CACHE STRING "") # See Professional CMake 24.8.1. Device-only Bundles
	set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)" CACHE STRING "") # See Professional CMake 24.8.1. Device-only Bundles
	if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
		set(CMAKE_OSX_DEPLOYMENT_TARGET "$(RECOMMENDED_MACOSX_DEPLOYMENT_TARGET)")
	endif()
endif()

# aether-game-utils project
project(aether-game-utils LANGUAGES CXX C VERSION 0.1.0)

# Configuration variables intended for external use
option(AE_LEAN_AND_MEAN "Disables building tests and examples" ON)
option(AE_EXTRAS "Enables experimental utilities" OFF)
option(AE_LOADERS_OFBX "Enables aether FBX loader and ofbx dependency" OFF)
option(AE_LOADERS_STB "Enables aether PNG loader and stb dependency" OFF)

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
if(MSVC)
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
function(add_ae_library NAME LIB_TYPE)
	add_library(${NAME} ${LIB_TYPE} "")
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
	target_include_directories(${NAME} PUBLIC ${AE_ROOT_DIR})
	if(APPLE)
		target_sources(${NAME} PRIVATE ${AE_ROOT_DIR}/aether.h ${AE_ROOT_DIR}/aether.mm)
		if (AE_USE_MODULES)
			target_compile_definitions(${NAME} PUBLIC AE_USE_MODULES)
			target_compile_options(${NAME} PUBLIC "-fmodules;-fcxx-modules")
		else()
			find_library(AE_OPENAL_LIBRARY OpenAL REQUIRED)
			find_library(AE_CARBON_LIBRARY Carbon REQUIRED)
			find_library(AE_APPKIT_LIBRARY AppKit REQUIRED)
			find_library(AE_GAMECONTROLLER_LIBRARY GameController REQUIRED)
			target_link_libraries(${NAME} ${AE_OPENAL_LIBRARY} ${AE_CARBON_LIBRARY} ${AE_APPKIT_LIBRARY} ${AE_GAMECONTROLLER_LIBRARY})
		endif()
		set_target_properties(${NAME} PROPERTIES XCODE_ATTRIBUTE_SKIP_INSTALL "YES") # See Professional CMake 24.7. Creating And Exporting Archives
	else()
		if(UNIX)
			target_compile_options(${NAME} PRIVATE -fPIC) # Fixes errors linking static ae into shared libraries: relocation against symbol 'X' which may bind externally 
		endif()
		target_sources(${NAME} PRIVATE ${AE_ROOT_DIR}/aether.h ${AE_ROOT_DIR}/aether.cpp)
	endif()
endfunction()
add_ae_library(ae STATIC) # static library
add_ae_library(ae_shared SHARED) # shared library (for hot loading)
set_target_properties(ae_shared PROPERTIES OUTPUT_NAME ae)

# Examples, extra utilities, and tests
add_subdirectory(loaders)
if(${AE_EXTRAS} OR (NOT ${AE_LEAN_AND_MEAN}))
	add_subdirectory(extras)
endif()
if(NOT ${AE_LEAN_AND_MEAN})
	add_subdirectory(net)
	add_subdirectory(examples)
	add_subdirectory(test)
endif()
