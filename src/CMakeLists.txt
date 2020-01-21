cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# reference opengl
INCLUDE(FindOpenGL)
message(STATUS "Found OpenGL in ${OPENGL_INCLUDE_DIR}")

# reference sdl2
INCLUDE(FindSDL2)
message(STATUS "Found SDL2 in ${SDL2_INCLUDE_DIR}")
include_directories(${SDL2_INCLUDE_DIR})

# reference libpng
INCLUDE(FindPNG)
message(STATUS "Found libpng in ${PNG_INCLUDE_DIRS}")
include_directories(${PNG_INCLUDE_DIRS})

# reference assimp
find_package(ASSIMP REQUIRED)
message(STATUS "Found assimp in ${ASSIMP_INCLUDE_DIRS}")
include_directories(${ASSIMP_INCLUDE_DIRS})

# library
add_library(${AE_LIBRARY_NAME} STATIC "")

target_sources(
  ${AE_LIBRARY_NAME}
  PRIVATE
    aeAlloc.cpp
    aeClock.cpp
    aeCommandLineArgs.cpp
    aeDict.cpp
    aeInput.cpp
    aeLog.cpp
    aeMath.cpp
    aeMesh.cpp
    aeMeta.cpp
    aeRender.cpp
    aeVfs.cpp
    aeWindow.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/aeAlloc.h
    ${CMAKE_CURRENT_LIST_DIR}/aeClock.h
    ${CMAKE_CURRENT_LIST_DIR}/aeCommandLineArgs.h
    ${CMAKE_CURRENT_LIST_DIR}/aeInitializer.h
    ${CMAKE_CURRENT_LIST_DIR}/aeInventoryGrid.h
    ${CMAKE_CURRENT_LIST_DIR}/aeLog.h
    ${CMAKE_CURRENT_LIST_DIR}/aeMath.h
    ${CMAKE_CURRENT_LIST_DIR}/aeMeta.h
    ${CMAKE_CURRENT_LIST_DIR}/aePlatform.h
    ${CMAKE_CURRENT_LIST_DIR}/aeRender.h
    ${CMAKE_CURRENT_LIST_DIR}/aeSignal.h
    ${CMAKE_CURRENT_LIST_DIR}/aeString.h
    ${CMAKE_CURRENT_LIST_DIR}/aeWindow.h
    ${CMAKE_CURRENT_LIST_DIR}/aeArray.h
    ${CMAKE_CURRENT_LIST_DIR}/aeCommand.h
    ${CMAKE_CURRENT_LIST_DIR}/aeDict.h
    ${CMAKE_CURRENT_LIST_DIR}/aeInput.h
    ${CMAKE_CURRENT_LIST_DIR}/aeList.h
    ${CMAKE_CURRENT_LIST_DIR}/aeMap.h
    ${CMAKE_CURRENT_LIST_DIR}/aeMesh.h
    ${CMAKE_CURRENT_LIST_DIR}/aeObjectPool.h
    ${CMAKE_CURRENT_LIST_DIR}/aeRef.h
    ${CMAKE_CURRENT_LIST_DIR}/aeRingBuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/aeSparseGrid.h
    ${CMAKE_CURRENT_LIST_DIR}/aeVfs.h
)

target_include_directories(${AE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
