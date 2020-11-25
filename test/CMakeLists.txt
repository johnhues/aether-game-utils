cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# get catch2 dependency
set(AE_CATCH_SRC ${CMAKE_BINARY_DIR}/thirdparty/Catch2)
set(AE_CATCH_INSTALL ${CMAKE_BINARY_DIR}/Catch2)
ExternalProject_Add(Catch2
	PREFIX Catch2
	SOURCE_DIR ${AE_CATCH_SRC}
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG v2.12.4
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_CATCH_INSTALL}
)
include_directories(${AE_CATCH_INSTALL}/include)

# unit test executable
add_executable(unit_tests
	MetaTest.cpp
	SignalTest.cpp
	StringTest.cpp
	test.cpp
)

# build dependencies
add_dependencies(unit_tests
	${AE_LIBRARY_NAME}
	Catch2
)

# ae library
target_link_libraries(unit_tests ${AE_LIBRARIES})

# run unit test
add_custom_command(
	TARGET unit_tests
	COMMENT "Run tests"
	POST_BUILD 
	COMMAND unit_tests
)
