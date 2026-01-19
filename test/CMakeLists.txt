cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(FetchContent)

message("")
message("aether-game-utils test")
message("")

# Catch2
set(CMAKE_CATCH_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
FetchContent_Declare(
	Catch2
	GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
	GIT_TAG v3.5.2
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(Catch2)
FetchContent_GetProperties(Catch2 SOURCE_DIR Catch2_SOURCE_DIR)

message("")

# test library (can't use ae library because special exception settings are needed for tests)
add_ae_library(ae_test STATIC)
target_include_directories(ae_test PUBLIC
	"${AE_ROOT_DIR}/extras/include"
	"${CURRENT_SOURCE_DIR}"
)
# Define aeAssertImpl to throw exceptions for REQUIRE_THROWS() tests
target_compile_definitions(ae_test PUBLIC AE_CONFIG_FILE="TestConfig.h")
if(EMSCRIPTEN)
	target_compile_options(ae_test PUBLIC
		-sDISABLE_EXCEPTION_CATCHING=0 # Needed for callstack traces on asserts with node
	)
	target_link_options(ae_test PUBLIC
		-sALLOW_MEMORY_GROWTH
		-sINITIAL_MEMORY=26214400
		-sSTACK_SIZE=2097152
		-sWASM=1
		-sASSERTIONS=2
		-sSAFE_HEAP=1
		-sDISABLE_EXCEPTION_CATCHING=0 # Needed for callstack traces on asserts with node
	)
endif()

# unit test executable
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.h" "*.cpp")
add_executable(test ${TEST_SOURCES})
if(CMAKE_GENERATOR STREQUAL Xcode)
	add_custom_command(TARGET test
		POST_BUILD
		COMMAND codesign --force -s - "$<TARGET_FILE:test>"
		DEPENDS test
		VERBATIM
	)
elseif(EMSCRIPTEN)
	set_target_properties(test PROPERTIES SUFFIX ".js")
endif()
target_link_libraries(test PRIVATE
	ae_test
	Catch2::Catch2
)

# run tests
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test)
