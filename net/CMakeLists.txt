cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(FetchContent)

# enet dependency
if(EMSCRIPTEN)
	add_compile_definitions(HAS_SOCKLEN_T=1) # Prevent enet from redefining socklen_t
endif()
FetchContent_Declare(
	enet
	GIT_REPOSITORY "https://github.com/lsalzman/enet.git"
	GIT_TAG cf735e639e5c9e3d2d84d71f1dbf789e8c2f3fd0
)
FetchContent_Populate(enet) # Download but don't build
add_library(enet STATIC "")
set_target_properties(enet PROPERTIES DEBUG_POSTFIX "d")
target_include_directories(enet PUBLIC ${enet_SOURCE_DIR}/include)
target_sources(enet PRIVATE
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

# ae_net
add_library(ae_net STATIC "")
set_target_properties(ae_net PROPERTIES DEBUG_POSTFIX "d")
target_link_libraries(ae_net ae enet)
if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
	target_link_libraries(ae_net uuid)
endif()
target_include_directories(ae_net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(ae_net PRIVATE
	include/ae/aeNet.h
	include/ae/EmSocket.h
	include/ae/aeUuid.h
	aeNetClient.cpp
	aeNetServer.cpp
	aeUuid.cpp
	EmSocket.cpp
)
