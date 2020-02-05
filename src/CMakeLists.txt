cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# use c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# use unicode
add_definitions(-D_UNICODE -DUNICODE)

INCLUDE(FindOpenGL)

IF (NOT WIN32)
	INCLUDE(FindPNG)
	INCLUDE(FindSDL2)
	INCLUDE(Findassimp)
	INCLUDE(FindEnet)
	INCLUDE(Finduuid)
ENDIF()

message(STATUS "Found OpenGL in ${OPENGL_INCLUDE_DIR}")
message(STATUS "Found libpng in ${PNG_INCLUDE_DIRS}")
message(STATUS "Found SDL2 in ${SDL2_INCLUDE_DIR}")
message(STATUS "Found assimp in ${ASSIMP_INCLUDE_DIRS}")
message(STATUS "Found enet in ${ENET_INCLUDE_DIR}")
message(STATUS "Found uuid in ${UUID_INCLUDE_DIR}")
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIR})
include_directories(${ASSIMP_INCLUDE_DIRS})
include_directories(${ENET_INCLUDE_DIR})
include_directories(${UUID_INCLUDE_DIR})

# library
add_library(${AE_LIBRARY_NAME} STATIC "")

IF (WIN32)
	add_dependencies(${AE_LIBRARY_NAME}
		glew
		zlib
		libpng
		SDL2
		assimp
		Enet
	)
ENDIF()

target_link_libraries(${AE_LIBRARY_NAME}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${ZLIB_LIBRARIES}
	${PNG_LIBRARIES}
	${SDL2_LIBRARY}
	${ASSIMP_LIBRARIES}
	${ENET_LIBRARY}
	${UUID_LIBRARY}
	${WIN_LIBRARIES}
)

target_sources(${AE_LIBRARY_NAME}
	PRIVATE
		aeAlloc.cpp
		aeBinaryStream.cpp
		aeClock.cpp
		aeCommandLineArgs.cpp
		aeDict.cpp
		aeInput.cpp
		aeLog.cpp
		aeMath.cpp
		aeMesh.cpp
		aeMeta.cpp
		aeNet.cpp
		aeNetClient.cpp
		aeNetServer.cpp
		aeRender.cpp
		aeUuid.cpp
		aeVfs.cpp
		aeWindow.cpp
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/aeAlloc.h
		${CMAKE_CURRENT_LIST_DIR}/aeArray.h
		${CMAKE_CURRENT_LIST_DIR}/aeBinaryStream.h
		${CMAKE_CURRENT_LIST_DIR}/aeClock.h
		${CMAKE_CURRENT_LIST_DIR}/aeCommand.h
		${CMAKE_CURRENT_LIST_DIR}/aeCommandLineArgs.h
		${CMAKE_CURRENT_LIST_DIR}/aeDict.h
		${CMAKE_CURRENT_LIST_DIR}/aeInitializer.h
		${CMAKE_CURRENT_LIST_DIR}/aeInput.h
		${CMAKE_CURRENT_LIST_DIR}/aeInventoryGrid.h
		${CMAKE_CURRENT_LIST_DIR}/aeList.h
		${CMAKE_CURRENT_LIST_DIR}/aeLog.h
		${CMAKE_CURRENT_LIST_DIR}/aeMap.h
		${CMAKE_CURRENT_LIST_DIR}/aeMath.h
		${CMAKE_CURRENT_LIST_DIR}/aeMesh.h
		${CMAKE_CURRENT_LIST_DIR}/aeMeta.h
		${CMAKE_CURRENT_LIST_DIR}/aeNet.h
		${CMAKE_CURRENT_LIST_DIR}/aeObjectPool.h
		${CMAKE_CURRENT_LIST_DIR}/aePlatform.h
		${CMAKE_CURRENT_LIST_DIR}/aeRef.h
		${CMAKE_CURRENT_LIST_DIR}/aeRender.h
		${CMAKE_CURRENT_LIST_DIR}/aeRingBuffer.h
		${CMAKE_CURRENT_LIST_DIR}/aeSignal.h
		${CMAKE_CURRENT_LIST_DIR}/aeSparseGrid.h
		${CMAKE_CURRENT_LIST_DIR}/aeString.h
		${CMAKE_CURRENT_LIST_DIR}/aeUuid.h
		${CMAKE_CURRENT_LIST_DIR}/aeVfs.h
		${CMAKE_CURRENT_LIST_DIR}/aeWindow.h
)

target_include_directories(${AE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
