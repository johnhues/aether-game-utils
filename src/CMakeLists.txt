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
# dependency linking
target_link_libraries(${AE_LIBRARY_NAME} ${AE_DEPS_LIBARARIES})

# build dependencies
if(AE_BUILD_DEPS)
	if(WIN32 AND NOT CYGWIN)
		add_dependencies(${AE_LIBRARY_NAME} glew)
		add_dependencies(${AE_LIBRARY_NAME} openal)
	endif()
	add_dependencies(${AE_LIBRARY_NAME}
		zlib
		assimp
		Enet
	)
	if(NOT EMSCRIPTEN)
		add_dependencies(${AE_LIBRARY_NAME} SDL2)
	endif()
endif()

# source files
set(AE_HEADER_FILES
	${PROJECT_BINARY_DIR}/config.h
	../include/aeAlloc.h
	../include/aeArray.h
	../include/aeAudio.h
	../include/aeBinaryStream.h
	../include/aeClock.h
	../include/aeCommand.h
	../include/aeCommandLineArgs.h
	../include/aeDict.h
	../include/aeInitializer.h
	../include/aeInput.h
	../include/aeInventoryGrid.h
	../include/aeList.h
	../include/aeLog.h
	../include/aeLog.hpp
	../include/aeMap.h
	../include/aeMath.h
	../include/aeMesh.h
	../include/aeMeta.h
	../include/aeNet.h
	../include/aeObjectPool.h
	../include/aePlatform.h
	../include/aeRef.h
	../include/aeRender.h
	../include/aeRingBuffer.h
	../include/aeSignal.h
	../include/aeSparseGrid.h
	../include/aeString.h
	../include/aeUuid.h
	../include/aeVfs.h
	../include/aeWindow.h
)

# @HACK: Include header files as private source so they are added to generated projects
target_sources(${AE_LIBRARY_NAME}
	PRIVATE
		${AE_HEADER_FILES}
		aeAlloc.cpp
		aeAudio.cpp
		aeBinaryStream.cpp
		aeClock.cpp
		aeCommandLineArgs.cpp
		aeCommonRender.cpp
		aeCommonRender.h
		aeDict.cpp
		aeInput.cpp
		aeLog.cpp
		aeMath.cpp
		aeMesh.cpp
		aeMeta.cpp
		aeNet.cpp
		aeNetClient.cpp
		aeNetServer.cpp
		aeOpenGLRender.cpp
		aePlatform.cpp
		aeUuid.cpp
		aeVfs.cpp
		aeWindow.cpp
		stb_image.h
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
