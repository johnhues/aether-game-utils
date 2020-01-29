cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# use c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# use unicode
add_definitions(-D_UNICODE -DUNICODE)

add_executable(example example.cpp)
add_dependencies(example ${AE_LIBRARY_NAME})
target_link_libraries(example ${AE_LIBRARIES})
