# Provides safeguards against in-source builds and bad build types.
#
# Variables used::
#
#   PROJECT_SOURCE_DIR
#   PROJECT_BINARY_DIR
#   CMAKE_BUILD_TYPE

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Default to Release. This is important because the install config is determined on configure.
# This will cause the install step to fail if the release bundle has not been build yet.
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
