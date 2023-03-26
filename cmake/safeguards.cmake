# Provides safeguards against in-source builds and bad build types.

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Default to Release. This is important because the install config is determined on configure.
# This will cause the install step to fail if the release bundle has not been built yet.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
	# @TODO: This should cause Xcode to default to Release but it doesn't
	# https://cmake.org/pipermail/cmake/2006-December/012288.html
	set(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo" CACHE STRING "" FORCE)
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_tolower)
string(TOUPPER "${CMAKE_BUILD_TYPE}" cmake_build_type_toupper)

if(NOT cmake_build_type_tolower STREQUAL "debug" AND
		NOT cmake_build_type_tolower STREQUAL "release" AND
		NOT cmake_build_type_tolower STREQUAL "minsizerel" AND
		NOT cmake_build_type_tolower STREQUAL "relwithdebinfo"
	)
	message(FATAL_ERROR "Unknown build type \"${CMAKE_BUILD_TYPE}\". Allowed values are Debug, Release, RelWithDebInfo, and MinSizeRel (case-insensitive).")
endif()
if(cmake_build_type_tolower STREQUAL "debug")
	set(AE_IS_DEBUG 1)
else()
	set(AE_IS_DEBUG 0)
endif()

# Helpers
get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# @NOTE: Build both Release and Debug at the same time for multi-config generators. This is necessary because Xcode will only build ae once.
# (https://stackoverflow.com/questions/16842218/how-do-i-use-cmake-externalproject-add-or-alternatives-in-a-cross-platform-way)
if(GENERATOR_IS_MULTI_CONFIG)
	set(AE_MULTI_CONFIG_BUILD_COMMAND
		BUILD_COMMAND ${CMAKE_COMMAND} --build . --config RelWithDebInfo
			COMMAND ${CMAKE_COMMAND} --build . --config Debug)
	set(AE_MULTI_CONFIG_INSTALL_COMMAND
		INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install --config RelWithDebInfo
			COMMAND ${CMAKE_COMMAND} --build . --target install --config Debug)
else()
	set(AE_SINGLE_CONFIG_BUILD_TYPE -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# Directory for external dependencies
# @TODO: Remove and use _deps
set(AE_THIRDPARTY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
