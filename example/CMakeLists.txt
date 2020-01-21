cmake_minimum_required(VERSION 3.10)
add_executable(example example.cpp)
target_link_libraries(example aegame OpenGL png SDL2 assimp)
