cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# ae_loaders
add_library(ae_loaders STATIC "")
set_target_properties(ae_loaders PROPERTIES DEBUG_POSTFIX "d")
target_link_libraries(ae_loaders ae)
target_include_directories(ae_loaders PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(ae_loaders PRIVATE include/ae/loaders.h)

# stb
target_sources(ae_loaders PRIVATE stb.cpp stb_image.h)
# GCC flags stb_image after inlining, past the pragmas in stb.cpp
set_source_files_properties(stb.cpp PROPERTIES COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-overflow>")

# openfbx
include(FetchContent)
FetchContent_Declare(
	openfbx
	GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
	GIT_TAG 9974840
	SOURCE_SUBDIR "NO-SUBDIRECTORY" # Use a non-existent subdirectory name to skip add_subdirectory()
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(openfbx)
FetchContent_GetProperties(openfbx SOURCE_DIR OPENFBX_SOURCE_DIR)
if(WASI)
	# long is 32 bits on wasm32, which OpenFBX only knows about for Emscripten
	file(READ "${OPENFBX_SOURCE_DIR}/src/ofbx.h" _ae_ofbx_h)
	if(NOT _ae_ofbx_h MATCHES "__wasm32__")
		string(REPLACE "defined(__EMSCRIPTEN__)" "defined(__EMSCRIPTEN__) || defined(__wasm32__)" _ae_ofbx_h "${_ae_ofbx_h}")
		file(WRITE "${OPENFBX_SOURCE_DIR}/src/ofbx.h" "${_ae_ofbx_h}")
	endif()
endif()
file(GLOB_RECURSE openfbx_SOURCES CONFIGURE_DEPENDS "${openfbx_SOURCE_DIR}/src/*")
add_library(openfbx ${openfbx_SOURCES})
target_include_directories(openfbx INTERFACE "${openfbx_SOURCE_DIR}/src")
target_compile_options(openfbx PRIVATE
	-Wno-switch
	-Wno-\#pragma-messages
)

target_link_libraries(ae_loaders openfbx)
target_sources(ae_loaders PRIVATE ofbx.cpp)
