cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Catch2 dependency
add_subdirectory(../deps/Catch2 Catch2)

# unit test executable
file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "*.h")
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(test
	${TEST_HEADERS}
	${TEST_SOURCES}
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
