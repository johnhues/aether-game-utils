cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# project name and supported languages
project(aether-game-utils LANGUAGES CXX VERSION 0.1.0)
set(AE_LIBRARY_NAME ae)
set(AE_LIBRARIES ${AE_LIBRARY_NAME})

# guard against in-source builds and bad build-type strings
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/safeguards.cmake)

# get git hash
include(cmake/git_revision.cmake)

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
	find_package(OpenGL REQUIRED)
endif()

# build sdl2 to be statically linked
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/sdl)

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
# set(ASSIMP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include ${CMAKE_CURRENT_BINARY_DIR}/assimp/include)

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
else()
	# build
endif()

# if (NOT AE_BUILD_DEPS)
# 	find_package(PNG REQUIRED)
# 	find_package(uuid REQUIRED)

# 	# @HACK: This should be found by the above, or the following variable names should be fixed
# 	set(UUID_LIBRARY_DIRS /usr/local/Cellar/ossp-uuid/1.6.2_2/lib)

# else()
# 	message(STATUS "AE_THIRDPARTY_DIR: ${AE_THIRDPARTY_DIR}")
# 	set(AE_GLEW_SRC ${AE_THIRDPARTY_DIR}/glew)
# 	set(AE_OPENAL_SRC ${AE_THIRDPARTY_DIR}/openal)
# 	set(AE_ZLIB_SRC ${AE_THIRDPARTY_DIR}/zlib)

# 	set(AE_OPENAL_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/openal)
# 	set(AE_ZLIB_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/zlib)

# 	if(WIN32 AND NOT CYGWIN)
# 		ExternalProject_Add(glew
# 			PREFIX glew
# 			SOURCE_DIR ${AE_GLEW_SRC}
# 			URL https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0-win32.zip
# 			CONFIGURE_COMMAND ""
# 			BUILD_COMMAND ""
# 			INSTALL_COMMAND ""
# 		)
# 		set(GLEW_INCLUDE_DIRS ${AE_GLEW_SRC}/include)
# 		set(GLEW_LIBRARY_DIRS ${AE_GLEW_SRC}/lib/Release/x64)
# 		set(GLEW_LIBRARIES
# 			${GLEW_LIBRARY_DIRS}/glew32s.lib
# 		)

# 		ExternalProject_Add(openal
# 			PREFIX openal
# 			SOURCE_DIR ${AE_OPENAL_SRC}
# 			GIT_REPOSITORY https://github.com/kcat/openal-soft.git
# 			GIT_TAG openal-soft-1.20.1
# 			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_OPENAL_INSTALL}
# 		)
# 		set(OPENAL_INCLUDE_DIR ${AE_OPENAL_INSTALL}/include)
# 		set(OPENAL_LIBRARY_DIR ${AE_OPENAL_INSTALL}/lib)
# 		set(OPENAL_LIBRARY ${OPENAL_LIBRARY_DIR}/OpenAL32.lib)
# 	endif()

# 	ExternalProject_Add(zlib
# 		PREFIX zlib
# 		SOURCE_DIR ${AE_ZLIB_SRC}
# 		GIT_REPOSITORY https://github.com/madler/zlib.git
# 		GIT_TAG v1.2.11
# 		# CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_ZLIB_INSTALL}
# 		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_ZLIB_INSTALL} -DCMAKE_APPLE_SILICON_PROCESSOR=arm64
# 	)
# 	set(ZLIB_INCLUDE_DIRS ${AE_ZLIB_INSTALL}/include)
# 	set(ZLIB_LIBRARY_DIRS ${AE_ZLIB_INSTALL}/lib)
# 	# set(ZLIB_LIBRARIES 
# 	# 	optimized ${ZLIB_LIBRARY_DIRS}/zlibstatic.lib
# 	# 	debug ${ZLIB_LIBRARY_DIRS}/zlibstaticd.lib
# 	# )
# endif()

# dependency include directories
set(AE_DEPS_LIBRARY_INCLUDE_DIRS
# 	${GLEW_INCLUDE_DIRS}
# 	${OPENAL_INCLUDE_DIR}
# 	${PNG_INCLUDE_DIRS}
# 	${ENET_INCLUDE_DIR}
# 	${UUID_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/deps/enet/include
)
# if(NOT EMSCRIPTEN)
# 	list(APPEND AE_DEPS_LIBRARY_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
# endif()

# dependency library directories
# set(AE_DEPS_LIBRARY_DIRS
# 	${GLEW_LIBRARY_DIRS}
# 	# ${OPENAL_LIBRARY_DIR}
# 	${ZLIB_LIBRARY_DIRS}
# 	${PNG_LIBRARY_DIRS}
# 	${SDL2_LIBRARY_DIRS}
# 	# ${ENET_LIBRARY_DIRS}
# 	${UUID_LIBRARY_DIRS}
# )

# dependency libraries
set(AE_DEPS_LIBARARIES ${OPENGL_LIBRARIES})
if(APPLE)
	list(APPEND AE_DEPS_LIBARARIES "-framework Cocoa")
elseif(WIN32 AND NOT CYGWIN)
	list(APPEND AE_DEPS_LIBARARIES
		version
		Winmm
		Imm32
		Setupapi
		Ws2_32
	)
endif()
list(APPEND AE_DEPS_LIBARARIES
	# ${GLEW_LIBRARIES}
	${OpenGLES}
	${OPENAL_LIBRARY}
	# ${ZLIB_LIBRARIES}
	# ${PNG_LIBRARIES}
	# ${UUID_LIBRARY}
	assimp
	enet
)
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
