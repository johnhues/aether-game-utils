cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Project definition
project(aether-game-utils LANGUAGES CXX C VERSION 0.1.0)
set(AE_LIBRARY_NAME ae)
set(AE_LIBRARIES ${AE_LIBRARY_NAME})
set(AE_LEAN_AND_MEAN ON CACHE BOOL "Disables building tests and examples")

# Includes
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/safeguards.cmake) # guard against in-source builds and bad build-type strings
include(cmake/git_revision.cmake) # print current local git repo status
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake") # find dependencies

# Defines and configuration
set(CMAKE_CXX_STANDARD 17) # use c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
	add_definitions(-D_UNICODE -DUNICODE) # use unicode
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) # disable standard library warnings on windows
endif()
if(APPLE)
	add_definitions(-DAE_USE_MODULES)
	add_compile_options(-fmodules)
	add_compile_options(-fcxx-modules)
	set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
endif()

# OpenGL
if (IOS_TOOLCHAIN_HAS_RUN)
	set(OPENGL_LIBRARIES "${CMAKE_OSX_SYSROOT_INT}/System/Library/Frameworks/OpenGLES.framework")
	if (EXISTS ${OPENGL_LIBRARIES})
		message(STATUS "Found OpenGLES")
	else()
		message(SEND_ERROR "Could not find OpenGLES")
		set(OPENGL_LIBRARIES "")
	endif()
elseif(EMSCRIPTEN)
	set(OPENGL_LIBRARIES GL)
else()
	set(OpenGL_GL_PREFERENCE GLVND) # See https://cmake.org/cmake/help/latest/policy/CMP0072.html
	find_package(OpenGL REQUIRED)
endif()
if (MSVC)
	add_definitions(-DGLEW_STATIC)
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
	list(APPEND OPENGL_LIBRARIES glew ${AE_GLEW_SRC}/lib/Release/x64/glew32s.lib)
endif()

if(AE_NET)
	include(FetchContent)
	# UUID
	if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
		find_package(uuid REQUIRED)
	endif()
	# enet
	FetchContent_Declare(
		enet
		GIT_REPOSITORY "https://github.com/lsalzman/enet.git"
		GIT_TAG cf735e639e5c9e3d2d84d71f1dbf789e8c2f3fd0
	)
	FetchContent_MakeAvailable(enet)
endif()

# OpenAL
if (IOS_TOOLCHAIN_HAS_RUN)
	set(OPENAL_LIBRARY "${CMAKE_OSX_SYSROOT_INT}/System/Library/Frameworks/OpenAL.framework")
	if (EXISTS ${OPENAL_LIBRARY})
		message(STATUS "Found OpenAL")
	else()
		message(SEND_ERROR "Could not find OpenAL")
		set(OPENAL_LIBRARY "")
	endif()
elseif(NOT EMSCRIPTEN)
	find_package(OpenAL REQUIRED)
endif()

# Dependency headers
set(AE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(AE_MISC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/misc)
set(AE_DEPS_LIBRARY_INCLUDE_DIRS
	${GLEW_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
)
# Dependency libraries
set(AE_DEPS
	${OPENGL_LIBRARIES}
	${OPENAL_LIBRARY}
)
if(MSVC)
	list(APPEND AE_DEPS
		version
		Winmm
		Imm32
		Setupapi
		Ws2_32
	)
elseif(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
	list(APPEND AE_DEPS uuid)
endif()

# Loaders
set(AE_LOADER_INCLUDE ${AE_MISC_DIR}/ae/loaders.h)
set(AE_STB_LOADER_SRC ${AE_MISC_DIR}/loaders/stb.cpp)
set(AE_OFBX_LOADER_SRC ${AE_MISC_DIR}/loaders/ofbx.cpp)

# library, tests, and examples
add_subdirectory(misc)
if(NOT ${AE_LEAN_AND_MEAN})
	add_subdirectory(test)
	add_subdirectory(example)
endif()
