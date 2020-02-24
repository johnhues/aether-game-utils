cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# @TODO: Fix install directory. This installs to /usr/local on linux, not /usr/local/lib
set(AE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${AE_LIBRARY_NAME})

# library
add_library(${AE_LIBRARY_NAME} STATIC "")

target_include_directories(${AE_LIBRARY_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/src
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # for headers when building
		$<INSTALL_INTERFACE:${AE_INSTALL_DIR}/include> # for headers when installing
)

# configure header file
configure_file(
	${PROJECT_SOURCE_DIR}/cmake/config.h.in
	${PROJECT_BINARY_DIR}/config.h
)

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

set(AE_HEADER_FILES
	${PROJECT_BINARY_DIR}/config.h
	aeAlloc.h
	aeArray.h
	aeBinaryStream.h
	aeClock.h
	aeCommand.h
	aeCommandLineArgs.h
	aeDict.h
	aeInitializer.h
	aeInput.h
	aeInventoryGrid.h
	aeList.h
	aeLog.h
	aeMap.h
	aeMath.h
	aeMesh.h
	aeMeta.h
	aeNet.h
	aeObjectPool.h
	aePlatform.h
	aeRef.h
	aeRender.h
	aeRingBuffer.h
	aeSignal.h
	aeSparseGrid.h
	aeString.h
	aeUuid.h
	aeVfs.h
	aeWindow.h
)

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
)

# ae lib installation
install(TARGETS ${AE_LIBRARY_NAME} EXPORT ${AE_LIBRARY_NAME} DESTINATION "${AE_INSTALL_DIR}")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION "${AE_INSTALL_DIR}/include")
install(
	FILES ${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
	RENAME ${AE_LIBRARY_NAME}Config.cmake
	DESTINATION "${AE_INSTALL_DIR}"
)
install(EXPORT ${AE_LIBRARY_NAME} DESTINATION "${AE_INSTALL_DIR}")
