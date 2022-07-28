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
	ae/aeCompactingAllocator.h
	ae/aeHotSpot.h
	ae/aeImage.h
	ae/aeImGui.h
	ae/aeInitializer.h
	ae/aeInventoryGrid.h
	ae/aeNet.h
	ae/aeRef.h
	ae/aeRender.h
	ae/aeRingBuffer.h
	ae/aeSignal.h
	ae/aeSparseGrid.h
	ae/aeTerrain.h
	ae/aeTesting.h
	ae/aeUuid.h
	ae/EmSocket.h
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
