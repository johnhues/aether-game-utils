cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Default to release build if not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(14_Emscripten)

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")

	set(SOEMTHING
		"-lGL" # Link in to WebGL/GLES system library
		"--closure=1" # Enable Closure compiler for aggressive JS size minification
		"-s TEXTDECODER=2" # When marshalling C UTF-8 strings across the JS<->Wasm language boundary, favor smallest generated code size rather than performance
		"-s MINIMAL_RUNTIME=2" # Enable aggressive MINIMAL_RUNTIME mode.
		"-s MIN_WEBGL_VERSION=3 -s MAX_WEBGL_VERSION=3" # Require WebGL 3 support in target browser, for smallest generated code size. (pass -s MIN_WEBGL_VERSION=1 to dual-target WebGL 1 and WebGL 2)
		"-s ENVIRONMENT=web" # The generated build output is only to be expected to be run in a web browser, never in a native CLI shell, or in a web worker.
		"-s ABORTING_MALLOC=0" # Fine tuning for code size: do not generate code to abort program execution on malloc() failures, that will not be interesting here.
		"-s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0" # Reduce WebGL code size: We do not need GLES2 emulation for automatic GL extension enabling
		"-s GL_EXTENSIONS_IN_PREFIXED_FORMAT=0" # Reduce WebGL code size: We do not need GLES2 emulation for GL extension names
		"-s GL_EMULATE_GLES_VERSION_STRING_FORMAT=0" # Reduce WebGL code size: No need to specify the GL_VENDOR/GL_RENDERER etc. fields in format required by GLES2 spec.
		"-s GL_POOL_TEMP_BUFFERS=0" # Reduce WebGL code size at the expense of performance (this only has an effect in WebGL 1, practically a no-op here)
		"-s GL_TRACK_ERRORS=0" # Reduce WebGL code size: WebGL bindings layer should not keep track of certain WebGL errors that are only meaningful for C/C++ applications. (good to enable for release when glGetError() is not used, but disable in debug)
		"-s GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0" # Reduce WebGL code size: do not emit code for extensions that we might not need.
		"-s SUPPORT_ERRNO=0" # Reduce code size: We do not need libc errno field support in our build output.
		"-s FILESYSTEM=0" # Reduce code size: We do not need native POSIX filesystem emulation support (Emscripten FS/MEMFS)
		# Choose the oldest browser versions that should be supported. The higher minimum bar you choose, the less emulation code may be present for old browser quirks.
		"-s MIN_FIREFOX_VERSION=70"
		"-s MIN_SAFARI_VERSION=130000"
		"-s MIN_IE_VERSION=0x7FFFFFFF" # Do not support Internet Explorer at all (this is the Emscripten default, shown here for posterity)
		"-s MIN_EDGE_VERSION=79" # Require Chromium-based Edge browser
		"-s MIN_CHROME_VERSION=80"
		# Optimization mode
		# "-Oz" # Optimization flag to optimize aggressively for size. (other options -Os, -O3, -O2, -O1, -O0)
		"-g" # Debug
	)
	string (REPLACE ";" " " SOEMTHING "${SOEMTHING}")
	message(STATUS "Using Emscripten flags: ${SOEMTHING}")
endif()

file(GLOB_RECURSE sources *.cpp *.c *.h)
add_executable(14_Emscripten ${sources})
target_include_directories(14_Emscripten PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")
set_property(TARGET 14_Emscripten PROPERTY LINK_FLAGS "${SOEMTHING}")

file(GLOB_RECURSE assets *.png)
list(APPEND assets test.txt)
foreach(asset ${assets})
	message("Adding asset ${asset}")
	file(COPY "${asset}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
endforeach()
