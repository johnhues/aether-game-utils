cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# library
add_library(${AE_LIBRARY_NAME} STATIC "")
set_target_properties(${AE_LIBRARY_NAME} PROPERTIES DEBUG_POSTFIX "d")

# dependencies
target_link_libraries(${AE_LIBRARY_NAME} "${AE_DEPS}")

# header files
target_include_directories(${AE_LIBRARY_NAME}
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
		"$<BUILD_INTERFACE:${AE_DEPS_LIBRARY_INCLUDE_DIRS}>" # expose glew etc to targets dependent on ae. Must be in quotes for BUILD_INTERFACE to work
)
set(AE_HEADER_FILES ../aether.h)
if(AE_EXTRAS)
	target_include_directories(${AE_LIBRARY_NAME}
		PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/misc>
		PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/misc>
	)
	list(APPEND AE_HEADER_FILES
		ae/aeCompactingAllocator.h
		ae/aeHotSpot.h
		ae/aeImage.h
		ae/aeImGui.h
		ae/aeInitializer.h
		ae/aeInventoryGrid.h
		ae/aeRef.h
		ae/aeRender.h
		ae/aeRingBuffer.h
		ae/aeSignal.h
		ae/aeSparseGrid.h
		ae/aeTerrain.h
		ae/aeTesting.h
	)
endif()
if(AE_NET)
	list(APPEND AE_HEADER_FILES
		ae/aeNet.h
		ae/EmSocket.h
		ae/aeUuid.h
	)
endif()

# source files
target_sources(${AE_LIBRARY_NAME} PRIVATE ${AE_HEADER_FILES}) # @NOTE: Include header files as private source so they are added to generated IDE projects
if(APPLE)
	target_sources(${AE_LIBRARY_NAME} PRIVATE aether.mm)
else()
	target_sources(${AE_LIBRARY_NAME} PRIVATE aether.cpp)
endif()
if(AE_EXTRAS)
	target_sources(${AE_LIBRARY_NAME} PRIVATE
		aeCommonRender.cpp
		aeCompactingAllocator.cpp
		aeHotSpot.cpp
		aeImage.cpp
		aeTerrain.cpp
		aeTerrainSDF.cpp
		aeTesting.cpp
		ctpl_stl.h
	)
endif()

# enet
if(AE_NET)
	add_dependencies(${AE_LIBRARY_NAME} enet)
	target_include_directories(${AE_LIBRARY_NAME} PRIVATE ${enet_SOURCE_DIR}/include)
	target_sources(${AE_LIBRARY_NAME} PRIVATE
		aeNetClient.cpp
		aeNetServer.cpp
		aeUuid.cpp
		EmSocket.cpp
		${enet_SOURCE_DIR}/callbacks.c
		${enet_SOURCE_DIR}/compress.c
		${enet_SOURCE_DIR}/host.c
		${enet_SOURCE_DIR}/list.c
		${enet_SOURCE_DIR}/packet.c
		${enet_SOURCE_DIR}/peer.c
		${enet_SOURCE_DIR}/protocol.c
		${enet_SOURCE_DIR}/unix.c
		${enet_SOURCE_DIR}/win32.c
	)
endif()
