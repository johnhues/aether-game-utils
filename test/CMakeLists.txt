cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Catch2 dependency
add_subdirectory(../deps/Catch2 Catch2)

# unit test executable
file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "*.h")
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(unit_tests
	${TEST_HEADERS}
	${TEST_SOURCES}
)

# ae library
target_link_libraries(unit_tests PRIVATE
	${AE_LIBRARIES}
	Catch2::Catch2
)

# run unit test
add_custom_command(
	TARGET unit_tests
	COMMENT "Run tests"
	POST_BUILD 
	COMMAND unit_tests
)
