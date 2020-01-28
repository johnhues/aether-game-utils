cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

add_executable(example example.cpp)
add_dependencies(example ${AE_LIBRARY_NAME})
target_link_libraries(example ${AE_LIBRARIES})
