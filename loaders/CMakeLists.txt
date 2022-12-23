cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# ae_loaders
if(AE_LOADERS_STB OR AE_LOADERS_OFBX)
	add_library(ae_loaders STATIC "")
	set_target_properties(ae_loaders PROPERTIES DEBUG_POSTFIX "d")
	target_link_libraries(ae_loaders ae)
	target_include_directories(ae_loaders PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_sources(ae_loaders PRIVATE include/ae/loaders.h)
endif()

# stb
if(AE_LOADERS_STB)
	target_sources(ae_loaders PRIVATE stb.cpp stb_image.h)
endif()

# openfbx
if(AE_LOADERS_OFBX)
	FetchContent_Declare(
		openfbx
		GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
		GIT_TAG b2e720c8f6d5979fc4ed346262e6270084b15836
	)
	FetchContent_Populate(openfbx) # Download but don't build
	FetchContent_GetProperties(openfbx SOURCE_DIR OPENFBX_SOURCE_DIR)
	file(GLOB_RECURSE openfbx_SOURCES CONFIGURE_DEPENDS "${openfbx_SOURCE_DIR}/src/*")
	add_library(openfbx ${openfbx_SOURCES})
	target_include_directories(openfbx INTERFACE "${openfbx_SOURCE_DIR}/src")

	target_link_libraries(ae_loaders openfbx)
	target_sources(ae_loaders PRIVATE ofbx.cpp)
endif()
