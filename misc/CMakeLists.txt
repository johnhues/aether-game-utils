cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(GNUInstallDirs)

# library
add_library(${AE_LIBRARY_NAME} STATIC "")
set_target_properties(${AE_LIBRARY_NAME} PROPERTIES DEBUG_POSTFIX "d")

if (AE_DEPS_TARGETS)
	add_dependencies(${AE_LIBRARY_NAME} ${AE_DEPS_TARGETS})
endif()

target_include_directories(${AE_LIBRARY_NAME}
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/misc>
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/misc>
		"$<BUILD_INTERFACE:${AE_DEPS_LIBRARY_INCLUDE_DIRS}>" # expose glew etc to targets dependent on ae. Must be in quotes for BUILD_INTERFACE to work
)

# dependency linking
# @TODO: Static libraries are always marked public so they can be linked directly
#        by the final executable. This means that any downloaded dependent libraries
#        should also be installed.
target_link_libraries(${AE_LIBRARY_NAME} ${AE_DEPS_LIBARARIES})

# source files
set(AE_HEADER_FILES
	../aether.h
	aeCompactingAllocator.h
	aeHotSpot.h
	aeImage.h
	aeImGui.h
	aeInitializer.h
	aeInventoryGrid.h
	aeNet.h
	aeRef.h
	aeRender.h
	aeRingBuffer.h
	aeSignal.h
	aeSparseGrid.h
	aeTerrain.h
	aeTesting.h
	aeUuid.h
	EmSocket.h
)

# @NOTE: Include header files as private source so they are added to generated IDE projects
target_sources(${AE_LIBRARY_NAME}
	PRIVATE
		${AE_HEADER_FILES}
		aeCommonRender.cpp
		aeCompactingAllocator.cpp
		aeHotSpot.cpp
		aeImage.cpp
		aeNetClient.cpp
		aeNetServer.cpp
		aeTerrain.cpp
		aeTerrainSDF.cpp
		aeTesting.cpp
		aether.cpp
		aeUuid.cpp
		ctpl_stl.h
		EmSocket.cpp
)

if(APPLE)
	target_sources(${AE_LIBRARY_NAME} PRIVATE aether.mm)
endif()
