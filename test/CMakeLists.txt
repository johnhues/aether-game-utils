cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(FetchContent)

# Catch2
FetchContent_Declare(
	Catch2
	GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
	GIT_TAG 5c88067bd339465513af4aec606bd2292f1b594a
)
FetchContent_MakeAvailable(Catch2)
FetchContent_GetProperties(Catch2 SOURCE_DIR Catch2_SOURCE_DIR)

# unit test executable
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.h" "*.cpp")
add_executable(test ${TEST_SOURCES})

if(EMSCRIPTEN)
	set(_AE_EM_LINKER_FLAGS
		"-s ALLOW_MEMORY_GROWTH"
		"--emrun"
	)
	string (REPLACE ";" " " _AE_EM_LINKER_FLAGS "${_AE_EM_LINKER_FLAGS}")
	set_target_properties(test PROPERTIES
		LINK_FLAGS "${_AE_EM_LINKER_FLAGS}"
		OUTPUT_NAME "index"
		SUFFIX ".html"
	)
endif()

# ae library
target_link_libraries(test PRIVATE
	ae_extras
	Catch2::Catch2
)

# run tests
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
include(Catch)
catch_discover_tests(test)
