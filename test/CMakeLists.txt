cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

include_directories(${CATCH_INCLUDE_DIRS})

add_executable(
	unit_tests
	SignalTest.cpp
	test.cpp
)

add_dependencies(unit_tests
	${AE_LIBRARY_NAME}
	Catch2
)

target_link_libraries(unit_tests ${AE_LIBRARIES})

add_custom_command(
	TARGET unit_tests
	COMMENT "Run tests"
	POST_BUILD 
	COMMAND unit_tests
)
