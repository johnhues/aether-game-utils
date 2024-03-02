cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(FetchContent)

# Catch2
FetchContent_Declare(
	Catch2
	GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
	GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)
FetchContent_GetProperties(Catch2 SOURCE_DIR Catch2_SOURCE_DIR)

# unit test executable
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.h" "*.cpp")
list(APPEND TEST_SOURCES "${AE_ROOT_DIR}/aether.h")
if(APPLE)
	list(APPEND TEST_SOURCES "${AE_ROOT_DIR}/aether.mm")
else()
	list(APPEND TEST_SOURCES "${AE_ROOT_DIR}/aether.cpp")
endif()
add_executable(test ${TEST_SOURCES})
target_compile_options(test PRIVATE
	"-fmodules"
	"-fcxx-modules"
	"-Wno-exceptions" # All asserts throw exceptions for unit tests
)
target_include_directories(test PRIVATE
	"${AE_ROOT_DIR}"
	"${AE_ROOT_DIR}/extras/include"
	"${CURRENT_SOURCE_DIR}"
)
target_compile_definitions(test PRIVATE
	AE_USE_MODULES
	AE_USER_CONFIG="TestConfig.h" # All asserts throw exceptions for unit tests
)
target_link_libraries(test PRIVATE
	Catch2::Catch2
)
if(EMSCRIPTEN)
	set(_AE_EM_LINKER_FLAGS
		"-s ALLOW_MEMORY_GROWTH"
		"-s STACK_SIZE=2097152"
		"-s WASM=1"
		"-s ASSERTIONS=1"
		"-s DISABLE_EXCEPTION_CATCHING=0"
	)
	string (REPLACE ";" " " _AE_EM_LINKER_FLAGS "${_AE_EM_LINKER_FLAGS}")
	set_target_properties(test PROPERTIES
		LINK_FLAGS "${_AE_EM_LINKER_FLAGS}"
		SUFFIX ".js"
	)
endif()

# run tests
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
include(Catch)
catch_discover_tests(test)
