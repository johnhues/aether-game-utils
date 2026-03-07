cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_library(ae_extras STATIC "")
set_target_properties(ae_extras PROPERTIES DEBUG_POSTFIX "d")
target_link_libraries(ae_extras PRIVATE ae)
if(UNIX AND NOT APPLE)
	target_link_libraries(ae_extras PRIVATE pthread)
endif()
target_include_directories(ae_extras PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(ae_extras PRIVATE
	include/ae/aeCompactingAllocator.h
	include/ae/aeHotSpot.h
	include/ae/aeImGui.h
	include/ae/aeImage.h
	include/ae/aeInitializer.h
	include/ae/aeInventoryGrid.h
	include/ae/aeRef.h
	include/ae/aeRender.h
	include/ae/aeSparseGrid.h
	include/ae/aeTerrain.h
	include/ae/Editor.h
	include/ae/Entity.h
	include/ae/sse2neon.h
	aeCommonRender.cpp
	aeCompactingAllocator.cpp
	aeHotSpot.cpp
	aeImage.cpp
	aeTerrain.cpp
	aeTerrainSDF.cpp
	ctpl_stl.h
	# Editor.cpp
	# Entity.cpp
)
