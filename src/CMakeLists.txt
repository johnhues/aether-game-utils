cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# library
add_library(${AE_LIBRARY_NAME} STATIC "")
target_include_directories(${AE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# dependency include directories
include_directories(${AE_DEPS_LIBRARY_INCLUDE_DIRS})
# dependency library directories
if (AE_DOWNLOAD_DEPS)
	link_directories(${AE_DEPS_LIBRARY_DIRS})
endif()
# dependency linking
target_link_libraries(${AE_LIBRARY_NAME} ${AE_DEPS_LIBARARIES})

# dependency building
if(AE_DOWNLOAD_DEPS)
	add_dependencies(${AE_LIBRARY_NAME}
		glew
		zlib
		libpng
		assimp
		Enet
	)
	if(NOT EMSCRIPTEN)
		add_dependencies(${AE_LIBRARY_NAME} SDL2)
	endif()
endif()

# source files
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
