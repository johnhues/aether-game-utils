cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# project name and supported languages
project(aether-game-utils LANGUAGES CXX VERSION 0.1.0)
set(AE_LIBRARY_NAME ae)
set(AE_LIBRARIES ${AE_LIBRARY_NAME})

# guard against in-source builds and bad build-type strings
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/safeguards.cmake)

# get git hash
include(cmake/git_revision.cmake)

include(ExternalProject)

# codesigning configuration
if(APPLE)
	set(AE_CODESIGN_IDENTITY "" CACHE STRING "Codesigning identity to use for apple builds")
endif()

set(AE_LEAN_AND_MEAN OFF CACHE BOOL "Disables building tests and examples")

# find dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (IOS_TOOLCHAIN_HAS_RUN)
	set(OpenGLES "${CMAKE_OSX_SYSROOT_INT}/System/Library/Frameworks/OpenGLES.framework")
	if (EXISTS ${OpenGLES})
		message(STATUS "Found OpenGLES")
	else()
		message(SEND_ERROR "Could not find OpenGLES")
		set(OpenGLES "")
	endif()
else()
	set(OpenGL_GL_PREFERENCE GLVND) # See https://cmake.org/cmake/help/latest/policy/CMP0072.html
	find_package(OpenGL REQUIRED)
endif()

if (MSVC)
	set(AE_GLEW_SRC ${AE_THIRDPARTY_DIR}/glew)
	ExternalProject_Add(glew
		PREFIX glew
		SOURCE_DIR ${AE_GLEW_SRC}
		URL https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0-win32.zip
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	set(GLEW_INCLUDE_DIR ${AE_GLEW_SRC}/include)
	set(GLEW_LIBRARY ${AE_GLEW_SRC}/lib/Release/x64/glew32s.lib)
	list(APPEND AE_DEPS_TARGETS glew)
endif()

if(NOT EMSCRIPTEN)
	if(UNIX AND NOT APPLE)
		find_package(uuid REQUIRED)
	endif()

	# build sdl2 to be statically linked
	set(SDL_STATIC ON CACHE BOOL "" FORCE)
	set(SDL_SHARED OFF CACHE BOOL "" FORCE)
	add_subdirectory(deps/sdl)
endif()

# some parts of assimp are not needed and should be disabled
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF) # doesn't work with ios builds
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF) # doesn't work with ios builds
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
add_subdirectory(deps/assimp)

# enet
add_subdirectory(deps/enet)

# OpenAL
if (IOS_TOOLCHAIN_HAS_RUN)
	set(OPENAL_LIBRARY "${CMAKE_OSX_SYSROOT_INT}/System/Library/Frameworks/OpenAL.framework")
	if (EXISTS ${OPENAL_LIBRARY})
		message(STATUS "Found OpenAL")
	else()
		message(SEND_ERROR "Could not find OpenAL")
		set(OPENAL_LIBRARY "")
	endif()
elseif(APPLE)
	find_package(OpenAL REQUIRED)
elseif(NOT EMSCRIPTEN)
	# @NOTE: It would be convenient to use MakeAvailable but openal defines a project called 'common' which can easily cause conflicts
	# FetchContent_Declare(
	# 	openal
	# 	GIT_REPOSITORY https://github.com/kcat/openal-soft.git
	# 	GIT_TAG openal-soft-1.20.1
	# 	CMAKE_ARGS -DLIBTYPE=STATIC
	# )
	# FetchContent_MakeAvailable(openal)
	set(AE_OPENAL_SRC ${AE_THIRDPARTY_DIR}/openal)
	set(AE_OPENAL_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/openal)
	ExternalProject_Add(openal
		PREFIX openal
		SOURCE_DIR ${AE_OPENAL_SRC}
		GIT_REPOSITORY https://github.com/kcat/openal-soft.git
		GIT_TAG openal-soft-1.20.1
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_OPENAL_INSTALL}
	)
	set(OPENAL_INCLUDE_DIR ${AE_OPENAL_INSTALL}/include)
	if (MSVC)
		set(OPENAL_LIBRARY ${AE_OPENAL_INSTALL}/lib/OpenAL32.lib)
	else()
		set(OPENAL_LIBRARY ${AE_OPENAL_INSTALL}/lib/${CMAKE_SHARED_MODULE_PREFIX}openal${CMAKE_SHARED_LIBRARY_SUFFIX})
	endif()
	list(APPEND AE_DEPS_TARGETS openal)
endif()

# dependency include directories
set(AE_DEPS_LIBRARY_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/deps/enet/include
	${GLEW_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
)

# dependency libraries
set(AE_DEPS_LIBARARIES
	${OPENGL_LIBRARIES}
	${OpenGLES}
	assimp
	enet
)
if(APPLE)
	list(APPEND AE_DEPS_LIBARARIES
		${OPENAL_LIBRARY}
		"-framework Cocoa"
	)
elseif(MSVC)
	list(APPEND AE_DEPS_LIBARARIES
		version
		Winmm
		Imm32
		Setupapi
		Ws2_32
		${GLEW_LIBRARY}
		${OPENAL_LIBRARY}
	)
elseif(UNIX)
	list(APPEND AE_DEPS_LIBARARIES
		${OPENAL_LIBRARY}
		uuid
	)
endif()
if(NOT EMSCRIPTEN)
	list(APPEND AE_DEPS_LIBARARIES SDL2main SDL2-static)
endif()

# use c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# use unicode
add_definitions(-D_UNICODE -DUNICODE)
# disable standard library warnings on windows
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# library, tests, and examples
add_subdirectory(src)
if(NOT ${AE_LEAN_AND_MEAN})
	add_subdirectory(test)
	add_subdirectory(example)
endif()
