cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# project name and supported languages
project(aether-game-utils LANGUAGES CXX VERSION 0.1.0)
set(AE_LIBRARY_NAME ae)
set(AE_LIBRARIES ${AE_LIBRARY_NAME})

# guard against in-source builds and bad build-type strings
include(cmake/safeguards.cmake)
# import external projects module
include(ExternalProject)

# get git hash
include(cmake/git_revision.cmake)

# configure header file
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/config.h.in
  ${PROJECT_BINARY_DIR}/config.h
)

# configure project to download dependencies instead of searching for them
if(WIN32 OR EMSCRIPTEN)
	set(AE_DOWNLOAD_DEPS ON CACHE BOOL "Download dependencies instead of searching system directories")
else()
	set(AE_DOWNLOAD_DEPS OFF CACHE BOOL "Download dependencies instead of searching system directories")
endif()
# codesigning configuration
if(APPLE)
	set(AE_CODESIGN_IDENTITY "" CACHE STRING "Codesigning identity to use for apple builds")
endif()

# find dependencies
include(FindOpenGL)
if (NOT AE_DOWNLOAD_DEPS)
	include(FindPNG)
	include(FindSDL2)
	include(Findassimp)
	include(FindEnet)
	include(Finduuid)

	# @HACK: This should be found by the above, or the following variable names should be fixed
	set(ASSIMP_LIBRARY_DIRS /usr/local/Cellar/assimp/5.0.1/lib)
	set(PNG_LIBRARY_DIRS /usr/local/Cellar/libpng/1.6.37/lib)
	set(ENET_LIBRARY_DIRS /usr/local/Cellar/enet/1.3.14/lib)
	set(UUID_LIBRARY_DIRS /usr/local/Cellar/ossp-uuid/1.6.2_2/lib)
else()
	set(AE_GLEW_SRC ${CMAKE_BINARY_DIR}/thirdparty/glew)
	set(AE_ZLIB_SRC ${CMAKE_BINARY_DIR}/thirdparty/zlib)
	set(AE_LIBPNG_SRC ${CMAKE_BINARY_DIR}/thirdparty/libpng)
	set(AE_SDL2_SRC ${CMAKE_BINARY_DIR}/thirdparty/SDL2)
	set(AE_ASSIMP_SRC ${CMAKE_BINARY_DIR}/thirdparty/assimp)
	set(AE_ENET_SRC ${CMAKE_BINARY_DIR}/thirdparty/enet)

	set(AE_GLEW_INSTALL ${CMAKE_BINARY_DIR}/glew)
	set(AE_ZLIB_INSTALL ${CMAKE_BINARY_DIR}/zlib)
	set(AE_LIBPNG_INSTALL ${CMAKE_BINARY_DIR}/libpng)
	set(AE_SDL2_INSTALL ${CMAKE_BINARY_DIR}/SDL2)
	set(AE_ASSIMP_INSTALL ${CMAKE_BINARY_DIR}/assimp)
	set(AE_ENET_INSTALL ${CMAKE_BINARY_DIR}/enet)

	if(WIN32)
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
	endif()

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

	if(NOT EMSCRIPTEN)
		ExternalProject_Add(SDL2
			PREFIX SDL2
			SOURCE_DIR ${AE_SDL2_SRC}
			URL https://www.libsdl.org/release/SDL2-2.0.10.tar.gz
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AE_SDL2_INSTALL} -DZLIB_ROOT=${AE_ZLIB_INSTALL}
		)
		set(SDL2_INCLUDE_DIR ${AE_SDL2_INSTALL}/include/SDL2)
		set(SDL2_LIBRARY_DIRS ${AE_SDL2_INSTALL}/lib)
		set(SDL2_LIBRARY SDL2-staticd)
	endif()

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
endif()

# dependency include directories
set(AE_DEPS_LIBRARY_INCLUDE_DIRS
	${GLEW_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${ASSIMP_INCLUDE_DIRS}
	${ENET_INCLUDE_DIR}
	${UUID_INCLUDE_DIR}
)
if(NOT EMSCRIPTEN)
	list(APPEND AE_DEPS_LIBRARY_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
endif()

# dependency library directories
set(AE_DEPS_LIBRARY_DIRS
	${GLEW_LIBRARY_DIRS}
	${ZLIB_LIBRARY_DIRS}
	${PNG_LIBRARY_DIRS}
	${SDL2_LIBRARY_DIRS}
	${ASSIMP_LIBRARY_DIRS}
	${ENET_LIBRARY_DIRS}
	${UUID_LIBRARY_DIRS}
)

# dependency libraries
set(AE_DEPS_LIBARARIES ${OPENGL_LIBRARIES})
if(APPLE)
	list(APPEND AE_DEPS_LIBARARIES "-framework Cocoa")
elseif(WIN32)
	list(APPEND AE_DEPS_LIBARARIES
		version
		Winmm
		Imm32
		Setupapi
		Ws2_32
	)
endif()
list(APPEND AE_DEPS_LIBARARIES
	${GLEW_LIBRARIES}
	${ZLIB_LIBRARIES}
	${PNG_LIBRARIES}
	${ASSIMP_LIBRARIES}
	${ENET_LIBRARY}
	${UUID_LIBRARY}
)
if(NOT EMSCRIPTEN)
	list(APPEND AE_DEPS_LIBARARIES ${SDL2_LIBRARY})
endif()

# use c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# use unicode
add_definitions(-D_UNICODE -DUNICODE)

# library, tests, and examples
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(example)
