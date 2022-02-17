# Bundle helper
function(add_bundle _AE_BUNDLE_NAME _AE_EXECUTABLE_NAME _AE_BUNDLE_ID _AE_BUNDLE_VERSION _AE_ICNS_FILE  _AE_SRC_FILES _AE_RESOURCES _AE_LIBS _AE_INCLUDE_DIRS)
	message(STATUS "_AE_BUNDLE_NAME ${_AE_BUNDLE_NAME} (${CMAKE_BUILD_TYPE})")
	message(STATUS "_AE_EXECUTABLE_NAME ${_AE_EXECUTABLE_NAME}")
	message(STATUS "_AE_BUNDLE_ID ${_AE_BUNDLE_ID}")
	message(STATUS "_AE_BUNDLE_VERSION ${_AE_BUNDLE_VERSION}")
	message(STATUS "_AE_ICNS_FILE ${_AE_ICNS_FILE}")
	message(STATUS "_AE_SRC_FILES ${_AE_SRC_FILES}")
	message(STATUS "_AE_RESOURCES ${_AE_RESOURCES}")
	message(STATUS "_AE_LIBS ${_AE_LIBS}")
	message(STATUS "_AE_INCLUDE_DIRS ${_AE_INCLUDE_DIRS}")
	message(STATUS "")

	if(WIN32)
		# Create a regular windowed application instead of the default console subsystem target
		set(_AE_EXE_TYPE WIN32)
	endif()

	add_executable(${_AE_EXECUTABLE_NAME} ${_AE_EXE_TYPE} ${_AE_SRC_FILES})
	if (_AE_LIBS)
		target_link_libraries(${_AE_EXECUTABLE_NAME} ${_AE_LIBS})
	endif()
	if (_AE_INCLUDE_DIRS)
		target_include_directories(${_AE_EXECUTABLE_NAME} PRIVATE ${_AE_INCLUDE_DIRS})
	endif()

	if(WIN32)
		set_target_properties(${_AE_EXECUTABLE_NAME} PROPERTIES
			LINK_FLAGS "/ENTRY:mainCRTStartup" # Use main instead of WinMain
		)
	elseif(APPLE)
		# Only add resource files to Apple bundles
		# Adding resources on Windows causes an issue where files are copied only once on configure
		target_sources(${_AE_EXECUTABLE_NAME} PRIVATE "${_AE_RESOURCES}")
		set_source_files_properties(${_AE_RESOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

		set(CMAKE_OSX_SYSROOT macosx)
		set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

		set_target_properties(${_AE_EXECUTABLE_NAME} PROPERTIES
			MACOSX_BUNDLE ON
			OUTPUT_NAME "${_AE_BUNDLE_NAME}"
			RESOURCE "${_AE_RESOURCES}"
			XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES"
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "No"
			MACOSX_BUNDLE_BUNDLE_NAME "${_AE_BUNDLE_NAME}" # CFBundleName
			MACOSX_BUNDLE_BUNDLE_VERSION "${_AE_BUNDLE_VERSION}" # CFBundleVersion
			MACOSX_BUNDLE_GUI_IDENTIFIER "${_AE_BUNDLE_ID}" # CFBundleIdentifier
			MACOSX_BUNDLE_ICON_FILE "${_AE_ICNS_FILE}" # CFBundleIconFile (*.icns file name without extension)
			MACOSX_BUNDLE_LONG_VERSION_STRING "${_AE_BUNDLE_VERSION}" # CFBundleLongVersionString
			MACOSX_BUNDLE_SHORT_VERSION_STRING "${_AE_BUNDLE_VERSION}" # CFBundleShortVersionString
			XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
			XCODE_ATTRIBUTE_SKIP_INSTALL "No"
		)

		set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
		if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
			# CMAKE_CURRENT_BINARY_DIR does not include CONFIG path with Xcode builds
			set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE})
		endif()
		install(CODE "
			include(BundleUtilities)
			set(BU_CHMOD_BUNDLE_ITEMS TRUE)
			fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/${_AE_BUNDLE_NAME}.app\" \"\" \"${_AE_DEPS_LIBRARY_DIRS}\")
		" COMPONENT Runtime)
	elseif(EMSCRIPTEN)
		set(_AE_EM_LINKER_FLAGS
			"-s TEXTDECODER=2" # When marshalling C UTF-8 strings across the JS<->Wasm language boundary, favor smallest generated code size rather than performance
			# "-s MINIMAL_RUNTIME=2" # Enable aggressive MINIMAL_RUNTIME mode.
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
		)
		if (AE_IS_DEBUG)
			list(APPEND _AE_EM_LINKER_FLAGS
				"-s SAFE_HEAP=1" # Enable safe heap mode
				"-s ASSERTIONS=1" # Enable assertions
				"-s DEMANGLE_SUPPORT=1"
				"-s STACK_OVERFLOW_CHECK=1"
				"-s SOCKET_DEBUG=1" # Print socket debug information automatically
				"-O0"
				"-frtti"
				"-fsanitize=undefined"
				"-g" # Debug
				# "-gsource-map" # Debug mode with mappings to c/c++ source files
				# "--source-map-base http://localhost:8000/embuild/example/"
			)
		else()
			list(APPEND _AE_EM_LINKER_FLAGS
				"--closure=1" # Enable Closure compiler for aggressive JS size minification
				"-Oz" # Optimization flag to optimize aggressively for size. (other options -Os, -O3, -O2, -O1, -O0)
			)
		endif()
		string (REPLACE ";" " " _AE_EM_LINKER_FLAGS "${_AE_EM_LINKER_FLAGS}")
		set_target_properties(${_AE_EXECUTABLE_NAME} PROPERTIES
			LINK_FLAGS "${_AE_EM_LINKER_FLAGS}"
			SUFFIX ".html"
		)
	endif()
	
	if(NOT APPLE)
		foreach(resource ${_AE_RESOURCES})
			add_custom_command(TARGET ${_AE_EXECUTABLE_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${CMAKE_CURRENT_SOURCE_DIR}/${resource}
					$<TARGET_FILE_DIR:${_AE_EXECUTABLE_NAME}>/${resource}
			)
		endforeach()
	endif()
	
endfunction()
