cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# project name and supported languages
project(aether-game-utils LANGUAGES CXX VERSION 0.1.0)
set(AE_LIBRARY_NAME ae)
set(AE_LIBRARIES ${AE_LIBRARY_NAME})

# guard against in-source builds and bad build-type strings
include(cmake/safeguards.cmake)

# detect operating system and host processor
message(STATUS "We are on a ${CMAKE_SYSTEM_NAME} system")
message(STATUS "The host processor is ${CMAKE_HOST_SYSTEM_PROCESSOR}")

# get git hash
include(cmake/git_revision.cmake)

# configure header file
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/config.h.in
  ${PROJECT_BINARY_DIR}/config.h
)

include(ExternalProject)

IF (WIN32)

	set(AE_GLEW_SRC ${CMAKE_CURRENT_SOURCE_DIR}/build/thirdparty/glew)
	set(AE_ZLIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/build/thirdparty/zlib)
	set(AE_LIBPNG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/build/thirdparty/libpng)
	set(AE_SDL2_SRC ${CMAKE_CURRENT_SOURCE_DIR}/build/thirdparty/SDL2)
	set(AE_ASSIMP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/build/thirdparty/assimp)
	set(AE_ENET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/build/thirdparty/enet)

	set(AE_GLEW_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/build/glew)
	set(AE_ZLIB_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/build/zlib)
	set(AE_LIBPNG_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/build/libpng)
	set(AE_SDL2_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/build/SDL2)
	set(AE_ASSIMP_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/build/assimp)
	set(AE_ENET_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/build/enet)

	set(WIN_LIBRARIES version Winmm Imm32 Setupapi Ws2_32)

	ExternalProject_Add(glew
		PREFIX glew
		SOURCE_DIR ${AE_GLEW_SRC}
		URL https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.tgz
		SOURCE_SUBDIR build/cmake
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_GLEW_INSTALL}
	)
	set(GLEW_INCLUDE_DIRS ${AE_GLEW_INSTALL}/include)
	set(GLEW_LIBRARY_DIRS ${AE_GLEW_INSTALL}/lib)
	set(GLEW_LIBRARIES libglew32d)

	ExternalProject_Add(zlib
		PREFIX zlib
		SOURCE_DIR ${AE_ZLIB_SRC}
		GIT_REPOSITORY https://github.com/madler/zlib.git
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_ZLIB_INSTALL}
	)
	set(ZLIB_INCLUDE_DIRS ${AE_ZLIB_INSTALL}/include)
	set(ZLIB_LIBRARY_DIRS ${AE_ZLIB_INSTALL}/lib)
	set(ZLIB_LIBRARIES zlibstaticd)

	ExternalProject_Add(libpng
		PREFIX libpng
		SOURCE_DIR ${AE_LIBPNG_SRC}
		GIT_REPOSITORY git://git.code.sf.net/p/libpng/code
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_LIBPNG_INSTALL} -DZLIB_ROOT=${AE_ZLIB_INSTALL}
	)
	set(PNG_INCLUDE_DIRS ${AE_LIBPNG_INSTALL}/include)
	set(PNG_LIBRARY_DIRS ${AE_LIBPNG_INSTALL}/lib)
	set(PNG_LIBRARIES libpng16_staticd)

	ExternalProject_Add(SDL2
		PREFIX SDL2
		SOURCE_DIR ${AE_SDL2_SRC}
		URL https://www.libsdl.org/release/SDL2-2.0.10.tar.gz
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_SDL2_INSTALL} -DZLIB_ROOT=${AE_ZLIB_INSTALL}
	)
	set(SDL2_INCLUDE_DIR ${AE_SDL2_INSTALL}/include/SDL2)
	set(SDL2_LIBRARY_DIRS ${AE_SDL2_INSTALL}/lib)
	set(SDL2_LIBRARY SDL2-staticd)

	ExternalProject_Add(assimp
		PREFIX assimp
		SOURCE_DIR ${AE_ASSIMP_SRC}
		GIT_REPOSITORY https://github.com/assimp/assimp.git
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_ASSIMP_INSTALL} -DZLIB_ROOT=${AE_ZLIB_INSTALL}
	)
	set(ASSIMP_INCLUDE_DIRS ${AE_ASSIMP_INSTALL}/include)
	set(ASSIMP_LIBRARY_DIRS ${AE_ASSIMP_INSTALL}/lib)
	set(ASSIMP_LIBRARIES assimp-vc142-mtd)

	ExternalProject_Add(Enet
		PREFIX Enet
		SOURCE_DIR ${AE_ENET_SRC}
		URL http://enet.bespin.org/download/enet-1.3.14.tar.gz
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	set(ENET_INCLUDE_DIR ${AE_ENET_SRC}/include)
	set(ENET_LIBRARY_DIRS ${AE_ENET_SRC})
	set(ENET_LIBRARY ${AE_ENET_SRC}/enet64.lib) # Use path so 'lib' is not prepended

	# dependency library directories
	link_directories(${GLEW_LIBRARY_DIRS})
	link_directories(${GLEW_LIBRARY_DIRS})
	link_directories(${ZLIB_LIBRARY_DIRS})
	link_directories(${PNG_LIBRARY_DIRS})
	link_directories(${SDL2_LIBRARY_DIRS})
	link_directories(${ASSIMP_LIBRARY_DIRS})
	link_directories(${ENET_LIBRARY_DIRS})
ENDIF()

set(AE_CATCH_SRC ${CMAKE_CURRENT_SOURCE_DIR}/build/thirdparty/Catch2)
set(AE_CATCH_INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/build/Catch2)
ExternalProject_Add(Catch2
	PREFIX Catch2
	SOURCE_DIR ${AE_CATCH_SRC}
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_CATCH_INSTALL}
)
set(CATCH_INCLUDE_DIRS ${AE_CATCH_INSTALL}/include)

# library and tests
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(example)
