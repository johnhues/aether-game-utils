cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

INCLUDE(FindOpenGL)
INCLUDE(FindPNG)
INCLUDE(FindSDL2)
find_package(ASSIMP REQUIRED)
link_directories(${ASSIMP_LIBRARY_DIRS})
include_directories(${ASSIMP_INCLUDE_DIRS})

add_executable(example example.cpp)
target_link_libraries(
  example
  ${AE_LIBRARY_NAME}
  ${OPENGL_LIBRARIES}
  ${PNG_LIBRARIES}
  ${SDL2_LIBRARY}
  ${ASSIMP_LIBRARIES}
)
