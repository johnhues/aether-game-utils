cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# aether-game-utils project
project(aether-game-utils LANGUAGES CXX C VERSION 0.1.0)
set(AE_LEAN_AND_MEAN ON CACHE BOOL "Disables building tests and examples")
set(AE_LOADERS_OFBX OFF CACHE BOOL "Enables aether FBX utilitiy and ofbx dependency")
set(AE_LOADERS_STB OFF CACHE BOOL "Enables aether PNG utilitiy and stb dependency")
set(AE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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
	list(APPEND AE_DEPS_LIBRARIES version Winmm Imm32 Setupapi Ws2_32)
endif()
if(APPLE)
	add_definitions(-DAE_USE_MODULES)
	add_compile_options(-fmodules)
	add_compile_options(-fcxx-modules)
	set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
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
	list(APPEND AE_DEPS_INCLUDE ${AE_GLEW_SRC}/include)
	list(APPEND AE_DEPS_LIBRARIES ${AE_GLEW_SRC}/lib/Release/x64/glew32s.lib)
	list(APPEND AE_DEPS_TARGETS glew)
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
elseif(NOT EMSCRIPTEN AND NOT MSVC)
	find_package(OpenAL REQUIRED)
	list(APPEND AE_DEPS_LIBRARIES ${OPENAL_LIBRARY})
endif()

# ae library
add_library(ae STATIC "")
set_target_properties(ae PROPERTIES DEBUG_POSTFIX "d")
if (AE_DEPS_LIBRARIES)
	target_link_libraries(ae ${AE_DEPS_LIBRARIES})
endif()
if (AE_DEPS_TARGETS)
	add_dependencies(ae ${AE_DEPS_TARGETS})
endif()
if (AE_DEPS_INCLUDE)
	target_include_directories(ae PUBLIC ${AE_DEPS_INCLUDE})
endif()
target_include_directories(ae PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(APPLE)
	target_sources(ae PRIVATE aether.h aether.mm)
else()
	target_sources(ae PRIVATE aether.h aether.cpp)
endif()

# Examples, extra utilities, and tests
add_subdirectory(loaders)
if(NOT ${AE_LEAN_AND_MEAN})
	add_subdirectory(extras)
	add_subdirectory(net)
	add_subdirectory(examples)
	add_subdirectory(test)
endif()
