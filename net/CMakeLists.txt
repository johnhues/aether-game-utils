cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(FetchContent)

message("")
message("aether-game-utils net")
message("")

# ae_net
add_library(ae_net STATIC "")
set_target_properties(ae_net PROPERTIES DEBUG_POSTFIX "d")
target_include_directories(ae_net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(ae_net PRIVATE
	include/ae/aeNet.h
	aeNetClient.cpp
)
target_link_libraries(ae_net ae)
if(WASI)
	return() # @TODO: A web transport. Sockets and enet are unavailable.
endif()

# enet dependency
FetchContent_Declare(
	enet
	GIT_REPOSITORY "https://github.com/lsalzman/enet.git"
	GIT_TAG cf735e639e5c9e3d2d84d71f1dbf789e8c2f3fd0
	SOURCE_SUBDIR "NO-SUBDIRECTORY" # Use a non-existent subdirectory name to skip add_subdirectory()
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(enet)
add_library(enet STATIC "")
set_target_properties(enet PROPERTIES DEBUG_POSTFIX "d")
target_include_directories(enet PUBLIC ${enet_SOURCE_DIR}/include)
if(WIN32)
	target_link_libraries(enet PUBLIC ws2_32 winmm)
	target_sources(enet PRIVATE ${enet_SOURCE_DIR}/win32.c)
	if(MSVC)
		target_compile_options(enet PRIVATE /wd4005 /wd4996) # Third party, enet redefines _CRT_SECURE_NO_WARNINGS and uses deprecated winsock calls
	endif()
else()
	target_sources(enet PRIVATE ${enet_SOURCE_DIR}/unix.c)
endif()
set_target_properties(enet PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION NO) # Third party, enet narrows sizes throughout
target_sources(enet PRIVATE
	${enet_SOURCE_DIR}/callbacks.c
	${enet_SOURCE_DIR}/compress.c
	${enet_SOURCE_DIR}/host.c
	${enet_SOURCE_DIR}/list.c
	${enet_SOURCE_DIR}/packet.c
	${enet_SOURCE_DIR}/peer.c
	${enet_SOURCE_DIR}/protocol.c
)
target_sources(ae_net PRIVATE aeNetServer.cpp)
target_link_libraries(ae_net enet)
