cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Catch2 dependency
add_subdirectory(../deps/Catch2 Catch2)

# unit test executable
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.h" "*.cpp")
add_executable(test
	${TEST_SOURCES}
	${AE_MISC_DIR}/ae/aeTesting.h
	${AE_MISC_DIR}/aeTesting.cpp
)

target_include_directories(test PRIVATE
	${AE_MISC_DIR}
)

# ae library
target_link_libraries(test PRIVATE
	${AE_LIBRARIES}
	Catch2::Catch2
)

# run tests
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../deps/Catch2/contrib)
include(Catch)
catch_discover_tests(test)
