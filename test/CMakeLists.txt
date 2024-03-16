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

# test library (asserts throw exceptions for unit tests)
add_ae_library(ae_test STATIC)
target_compile_options(ae_test PUBLIC "-Wno-exceptions") # All asserts throw exceptions for unit tests
target_compile_definitions(ae_test PUBLIC AE_USER_CONFIG="TestConfig.h") # All asserts throw exceptions for unit tests
target_include_directories(ae_test PUBLIC
	"${AE_ROOT_DIR}/extras/include"
	"${CURRENT_SOURCE_DIR}"
)
if(EMSCRIPTEN)
	target_compile_options(ae_test PUBLIC
		-sDISABLE_EXCEPTION_CATCHING=0
	)
	target_link_options(ae_test PUBLIC
		-sALLOW_MEMORY_GROWTH
		-sINITIAL_MEMORY=26214400
		-sSTACK_SIZE=2097152
		-sWASM=1
		-sASSERTIONS=0
		-sDISABLE_EXCEPTION_CATCHING=0
	)
endif()

# unit test executable
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.h" "*.cpp")
add_executable(test ${TEST_SOURCES})
if(EMSCRIPTEN)
	set_target_properties(test PROPERTIES SUFFIX ".js")
endif()
target_link_libraries(test PRIVATE
	ae_test
	Catch2::Catch2
)

# run tests
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
include(Catch)
catch_discover_tests(test)
