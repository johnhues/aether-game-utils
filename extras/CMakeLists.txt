cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_library(ae_extras STATIC "")
set_target_properties(ae_extras PROPERTIES DEBUG_POSTFIX "d")
target_link_libraries(ae_extras ae)
if(UNIX AND NOT APPLE)
	target_link_libraries(ae_extras pthread)
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
	include/ae/aeSignal.h
	include/ae/aeSparseGrid.h
	include/ae/aeTerrain.h
	include/ae/Editor.h
	include/ae/Entity.h
	include/ae/SpriteRenderer.h
	aeCommonRender.cpp
	aeCompactingAllocator.cpp
	aeHotSpot.cpp
	aeImage.cpp
	aeTerrain.cpp
	aeTerrainSDF.cpp
	ctpl_stl.h
	# Editor.cpp
	# Entity.cpp
	SpriteRenderer.cpp
	sse2neon.h
)
