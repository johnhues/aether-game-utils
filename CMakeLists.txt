cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

message("") # Empty line to separate aether logs from others
message("aether-game-utils")
message("")

# Configuration variables intended for external use
# AE_LEAN_AND_MEAN mode is disabled when building aether on its own
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	option(AE_LEAN_AND_MEAN "Disables building tests and examples" OFF)
else()
	option(AE_LEAN_AND_MEAN "Disables building tests and examples" ON)
endif()
option(AE_EXTRAS "Enables experimental utilities" OFF)
option(AE_LOADERS_OFBX "Enables aether FBX loader and ofbx dependency" OFF)
option(AE_LOADERS_STB "Enables aether PNG loader and stb dependency" OFF)

# Configuration for building ae on its own
if(NOT ${AE_LEAN_AND_MEAN})
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

	# Enable generation of compile_commands.json for intellisense and clangd
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

	# Xcode specific settings
	if (("${CMAKE_GENERATOR}" STREQUAL "Xcode"))
		set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "" CACHE STRING "") # See Professional CMake 24.8.1. Device-only Bundles
		set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)" CACHE STRING "") # See Professional CMake 24.8.1. Device-only Bundles
		if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
			set(CMAKE_OSX_DEPLOYMENT_TARGET "$(RECOMMENDED_MACOSX_DEPLOYMENT_TARGET)")
		endif()
	endif()
endif()

# aether-game-utils project
project(aether-game-utils LANGUAGES CXX C VERSION 1.0.0)

set(AE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(AE_NAMESPACE aether-game-utils)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT ${AE_LEAN_AND_MEAN})
	if(MSVC)
		add_definitions(-D_UNICODE -DUNICODE) # use unicode
		add_definitions(-D_CRT_SECURE_NO_WARNINGS) # disable standard library warnings on windows
	endif()
	# Disable RTTI (not used in aether)
	add_compile_options(-fno-rtti)
endif()
Include(${AE_ROOT_DIR}/AddBundle.cmake)

# OpenGL dependency
if (IOS_TOOLCHAIN_HAS_RUN)
	set(OPENGL_FRAMEWORK "${CMAKE_OSX_SYSROOT_INT}/System/Library/Frameworks/OpenGLES.framework")
	if (EXISTS ${OPENGL_FRAMEWORK})
		message("Found OpenGLES")
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
		message("Found OpenAL")
		list(APPEND AE_DEPS_LIBRARIES ${OPENAL_LIBRARY})
	else()
		message(SEND_ERROR "Could not find OpenAL")
	endif()
endif()

message("")

# ae library
function(add_ae_library NAME LIB_TYPE)
	set(NAME_ALIAS ${AE_NAMESPACE}::${NAME})
	message("Added target ${NAME}")
	message("  ${NAME} alias:")
	message("    ${NAME_ALIAS}")
	add_library(${NAME} ${LIB_TYPE} "")
	add_library(${NAME_ALIAS} ALIAS ${NAME})
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
	target_precompile_headers(${NAME} PUBLIC "${AE_ROOT_DIR}/aether.h")
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
	elseif(MSVC)
		target_link_libraries(${NAME} version Winmm Imm32 Setupapi Ws2_32)
	else()
		if(UNIX)
			target_compile_options(${NAME} PRIVATE -fPIC) # Fixes errors linking static ae into shared libraries: relocation against symbol 'X' which may bind externally 
		endif()
		target_sources(${NAME} PRIVATE ${AE_ROOT_DIR}/aether.h ${AE_ROOT_DIR}/aether.cpp)
	endif()
	if(NOT ${AE_LEAN_AND_MEAN} AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|AppleClang"))
		target_compile_options(${NAME} PUBLIC
			-Werror=return-type
			-Werror=implicit-fallthrough
			-Werror=implicit-int-conversion
			-Werror=narrowing
			-Werror=uninitialized
			-Werror=pointer-to-int-cast
		)
	endif()
	message("  ${NAME} properties:")
	ae_print_target_properties(${NAME} PREFIX "    ")
	message("")
endfunction()
add_ae_library(ae STATIC) # static library
add_ae_library(ae_shared SHARED) # shared library (for hot loading)
set_target_properties(ae_shared PROPERTIES OUTPUT_NAME ae)
if(NOT ${AE_LEAN_AND_MEAN})
	# Disable exceptions (not used in aether outside of unit tests)
	target_compile_options(ae PUBLIC -fno-exceptions)
	target_compile_options(ae_shared PUBLIC -fno-exceptions)
endif()

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
