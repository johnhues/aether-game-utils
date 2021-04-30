cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(GNUInstallDirs)

# library
add_library(${AE_LIBRARY_NAME} STATIC "")
set_target_properties(${AE_LIBRARY_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(${AE_LIBRARY_NAME}
	PRIVATE
		${AE_DEPS_LIBRARY_INCLUDE_DIRS}
		${PROJECT_SOURCE_DIR}/src
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${AE_LIBRARY_NAME}> # for headers when building
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # for headers when building
		$<INSTALL_INTERFACE:include> # <install prefix>/include
)

# build dependencies
if(AE_BUILD_DEPS)
	if(WIN32 AND NOT CYGWIN)
		add_dependencies(${AE_LIBRARY_NAME} glew)
		add_dependencies(${AE_LIBRARY_NAME} openal)
	endif()
	add_dependencies(${AE_LIBRARY_NAME}
		zlib
		Enet
	)
	if(NOT EMSCRIPTEN)
		add_dependencies(${AE_LIBRARY_NAME} SDL2)
	endif()
endif()
target_link_libraries(${AE_LIBRARY_NAME}
	assimp
)

# dependency linking
# @TODO: Static libraries are always marked public so they can be linked directly
#        by the final executable. This means that any downloaded dependent libraries
#        should also be installed.
target_link_libraries(${AE_LIBRARY_NAME} ${AE_DEPS_LIBARARIES})

# source files
set(AE_HEADER_FILES
	../include/ae/aether.h
	../include/ae/aeAlloc.h
	../include/ae/aeArray.h
	../include/ae/aeAudio.h
	../include/ae/aeBinaryStream.h
	../include/ae/aeClock.h
	../include/ae/aeCommand.h
	../include/ae/aeCompactingAllocator.h
	../include/ae/aeCommandLineArgs.h
	../include/ae/aeCompactingAllocator.h
	../include/ae/aeDict.h
	../include/ae/aeEditorCamera.h
	../include/ae/aeFileDialog.h
	../include/ae/aeHotSpot.h
	../include/ae/aeImage.h
	../include/ae/aeImGui.h
	../include/ae/aeInitializer.h
	../include/ae/aeInput.h
	../include/ae/aeInventoryGrid.h
	../include/ae/aeList.h
	../include/ae/aeLog.h
	../include/ae/aeLog.hpp
	../include/ae/aeMap.h
	../include/ae/aeMath.h
	../include/ae/aeMesh.h
	../include/ae/aeMeta.h
	../include/ae/aeNet.h
	../include/ae/aeObjectPool.h
	../include/ae/aePlatform.h
	../include/ae/aeRef.h
	../include/ae/aeRender.h
	../include/ae/aeRingBuffer.h
	../include/ae/aeSignal.h
	../include/ae/aeSparseGrid.h
	../include/ae/aeSpline.h
	../include/ae/aeString.h
	../include/ae/aeTerrain.h
	../include/ae/aeTesting.h
	../include/ae/aeUuid.h
	../include/ae/aeVfs.h
	../include/ae/aeWindow.h
)

# @NOTE: Include header files as private source so they are added to generated IDE projects
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
		aeCompactingAllocator.cpp
		aeDict.cpp
		aeEditorCamera.cpp
		aeFileDialog.cpp
		aeHotSpot.cpp
		aeImage.cpp
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
		aeSpline.cpp
		aeTerrain.cpp
		aeTerrainSDF.cpp
		aeTesting.cpp
		aeUuid.cpp
		aeVfs.cpp
		aeWindow.cpp
		ctpl_stl.h
		stb_image.h
)

if(APPLE)
	target_sources(${AE_LIBRARY_NAME}
		PRIVATE
			aeVfs.mm
	)
endif()

# ae lib installation
message(STATUS "ae installation path: ${CMAKE_INSTALL_PREFIX}")
install(TARGETS ${AE_LIBRARY_NAME} EXPORT ${AE_LIBRARY_NAME}Config
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(EXPORT ${AE_LIBRARY_NAME}Config DESTINATION lib/cmake/${AE_LIBRARY_NAME})
