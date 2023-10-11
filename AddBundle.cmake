cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

function(ae_add_shared_library _AE_NAME _AE_MAJOR_MINOR_PATCH_VERSION _AE_SRC_FILES _AE_HEADER_FILES _AE_LIBS _AE_PUBLIC_INCLUDE_DIRS _AE_PRIVATE_INCLUDE_DIRS)
	message(STATUS "_AE_NAME \"${_AE_NAME}\" \(${CMAKE_BUILD_TYPE}\)")
	message(STATUS "_AE_MAJOR_MINOR_PATCH_VERSION \"${_AE_MAJOR_MINOR_PATCH_VERSION}\"")
	message(STATUS "_AE_SRC_FILES \"${_AE_SRC_FILES}\"")
	message(STATUS "_AE_HEADER_FILES \"${_AE_HEADER_FILES}\"")
	message(STATUS "_AE_LIBS \"${_AE_LIBS}\"")
	message(STATUS "_AE_PUBLIC_INCLUDE_DIRS \"${_AE_PUBLIC_INCLUDE_DIRS}\"")
	message(STATUS "_AE_PRIVATE_INCLUDE_DIRS \"${_AE_PRIVATE_INCLUDE_DIRS}\"")
	message(STATUS "")

	add_library(${_AE_NAME} SHARED ${_AE_SRC_FILES})
	if(_AE_LIBS)
		target_link_libraries(${_AE_NAME} ${_AE_LIBS})
	endif()
	if(_AE_INCLUDE_DIRS)
		target_include_directories(${_AE_NAME} PRIVATE ${_AE_INCLUDE_DIRS})
	endif()
	if (("${CMAKE_GENERATOR}" STREQUAL "Xcode"))
		set(_AE_APPLE_SKIP_INSTALL "YES") # See Professional CMake 24.7. Creating And Exporting Archives
	endif()
	if(APPLE)
		set_target_properties(${_AE_NAME} PROPERTIES
			FRAMEWORK TRUE
			FRAMEWORK_VERSION "${_AE_MAJOR_MINOR_PATCH_VERSION}"
			PUBLIC_HEADER "${_AE_HEADER_FILES}"
			XCODE_ATTRIBUTE_SKIP_INSTALL "${_AE_APPLE_SKIP_INSTALL}" # See Professional CMake 24.7. Creating And Exporting Archives
		)
	endif()
endfunction()

# Bundle helper
function(ae_add_bundle _AE_BUNDLE_NAME _AE_EXECUTABLE_NAME _AE_BUNDLE_ID _AE_APPLE_DEVELOPMENT_TEAM _AE_MAJOR_MINOR_PATCH_VERSION _AE_ICNS_FILE _AE_SRC_FILES _AE_RESOURCES _AE_LIBS _AE_PACKAGE_LIBS _AE_INCLUDE_DIRS)
	message(STATUS "_AE_BUNDLE_NAME \"${_AE_BUNDLE_NAME}\" \(${CMAKE_BUILD_TYPE})")
	message(STATUS "_AE_EXECUTABLE_NAME \"${_AE_EXECUTABLE_NAME}\"")
	message(STATUS "_AE_BUNDLE_ID \"${_AE_BUNDLE_ID}\"")
	message(STATUS "_AE_APPLE_DEVELOPMENT_TEAM \"${_AE_APPLE_DEVELOPMENT_TEAM}\"")
	message(STATUS "_AE_MAJOR_MINOR_PATCH_VERSION \"${_AE_MAJOR_MINOR_PATCH_VERSION}\"")
	message(STATUS "_AE_ICNS_FILE \"${_AE_ICNS_FILE}\"")
	message(STATUS "_AE_SRC_FILES \"${_AE_SRC_FILES}\"")
	message(STATUS "_AE_RESOURCES \"${_AE_RESOURCES}\"")
	message(STATUS "_AE_LIBS \"${_AE_LIBS}\"")
	message(STATUS "_AE_PACKAGE_LIBS \"${_AE_PACKAGE_LIBS}\"")
	message(STATUS "_AE_INCLUDE_DIRS \"${_AE_INCLUDE_DIRS}\"")
	message(STATUS "")

	if(WIN32)
		# Create a regular windowed application instead of the default console subsystem target
		set(_AE_EXE_TYPE WIN32)
	elseif(APPLE)
		set(_AE_EXE_TYPE MACOSX_BUNDLE)
	endif()

	add_executable(${_AE_EXECUTABLE_NAME} ${_AE_EXE_TYPE} ${_AE_SRC_FILES})
	if(_AE_LIBS)
		target_link_libraries(${_AE_EXECUTABLE_NAME} ${_AE_LIBS})
	endif()
	if(_AE_PACKAGE_LIBS)
		target_link_libraries(${_AE_EXECUTABLE_NAME} ${_AE_PACKAGE_LIBS})
	endif()
	if(_AE_INCLUDE_DIRS)
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
		foreach(resource ${_AE_RESOURCES})
			get_filename_component(resource_path ${resource} ABSOLUTE) # First get absolute path to resource
			file(RELATIVE_PATH resource_path ${CMAKE_CURRENT_SOURCE_DIR} ${resource_path}) # Get the relative path to the resource from the root of the resource
			cmake_path(GET resource_path PARENT_PATH resource_path) # Remove file name from path
			set_source_files_properties(${resource} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${resource_path}")
		endforeach()

		if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
			if (_AE_APPLE_DEVELOPMENT_TEAM)
				set(_AE_APPLE_CODE_SIGN_IDENTITY "Apple Development") # See Professional CMake 24.6.1. Signing Identity And Development Team
			endif()
			set(_AE_APPLE_INSTALL_PATH "$(LOCAL_APPS_DIR)") # See Professional CMake 24.7. Creating And Exporting Archives
			set(_AE_APPLE_SKIP_INSTALL "NO") # See Professional CMake 24.7. Creating And Exporting Archives
		endif()

		set_target_properties(${_AE_EXECUTABLE_NAME} PROPERTIES
			XCODE_ATTRIBUTE_PRODUCT_NAME "${_AE_BUNDLE_NAME}" # CFBundleName
			XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${_AE_BUNDLE_ID}" # CFBundleIdentifier
			XCODE_ATTRIBUTE_MARKETING_VERSION "${_AE_MAJOR_MINOR_PATCH_VERSION}"
			XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "${_AE_MAJOR_MINOR_PATCH_VERSION}"
			XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE YES
			XCODE_ATTRIBUTE_INFOPLIST_FILE ""

			XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName "${_AE_BUNDLE_NAME}"
			XCODE_ATTRIBUTE_INFOPLIST_KEY_LSApplicationCategoryType "public.app-category.games"
			# XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleIconFile "${_AE_ICNS_FILE}"
			XCODE_ATTRIBUTE_INFOPLIST_KEY_NSPrincipalClass "NSApplication"
			# XCODE_ATTRIBUTE_INFOPLIST_KEY_NSHumanReadable @TODO

			XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${_AE_APPLE_DEVELOPMENT_TEAM}"
			XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${_AE_APPLE_CODE_SIGN_IDENTITY}"

			XCODE_ATTRIBUTE_INSTALL_PATH "${_AE_APPLE_INSTALL_PATH}" # See Professional CMake 24.7. Creating And Exporting Archives
			XCODE_ATTRIBUTE_SKIP_INSTALL "${_AE_APPLE_SKIP_INSTALL}" # See Professional CMake 24.7. Creating And Exporting Archives

			XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES"
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "No"

			MACOSX_BUNDLE_ICON_FILE "${_AE_ICNS_FILE}" # CFBundleIconFile (*.icns file path)

			OUTPUT_NAME "${_AE_BUNDLE_NAME}"

			MACOSX_RPATH TRUE
			INSTALL_RPATH @executable_path/../Frameworks
		)

		if(_AE_PACKAGE_LIBS)
			set_target_properties(${_AE_EXECUTABLE_NAME} PROPERTIES XCODE_EMBED_FRAMEWORKS "${_AE_PACKAGE_LIBS}") # 24.10. Embedding Frameworks, Plugins And Extensions
		endif()
	elseif(EMSCRIPTEN)
		set(_AE_EM_LINKER_FLAGS
			"-lopenal"
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
			# "-s FILESYSTEM=0" # Reduce code size: We do not need native POSIX filesystem emulation support (Emscripten FS/MEMFS) @TODO: Filesystem is required for sockets
			# Choose the oldest browser versions that should be supported. The higher minimum bar you choose, the less emulation code may be present for old browser quirks.
			"-s MIN_FIREFOX_VERSION=70"
			"-s MIN_SAFARI_VERSION=130000"
			"-s MIN_IE_VERSION=0x7FFFFFFF" # Do not support Internet Explorer at all (this is the Emscripten default, shown here for posterity)
			"-s MIN_EDGE_VERSION=79" # Require Chromium-based Edge browser
			"-s MIN_CHROME_VERSION=80"
		)
		string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_tolower)
		if (cmake_build_type_tolower STREQUAL "debug")
			list(APPEND _AE_EM_LINKER_FLAGS
				"-s SAFE_HEAP=1" # Enable safe heap mode
				"-s ASSERTIONS=1" # Enable assertions
				"-s DEMANGLE_SUPPORT=1"
				"-s STACK_OVERFLOW_CHECK=1"
				"-O0"
				"-frtti"
				"-fsanitize=undefined"
				"-g" # Debug symbols (DWARF https://developer.chrome.com/blog/wasm-debugging-2020/)
			)
		else()
			list(APPEND _AE_EM_LINKER_FLAGS
				"--closure=1" # Enable Closure compiler for aggressive JS size minification
				"-O3"
			)
		endif()
		string (REPLACE ";" " " _AE_EM_LINKER_FLAGS "${_AE_EM_LINKER_FLAGS}")
		
		set(_AE_EM_OUT_SUFFIX ".html")
		string(FIND "${_AE_RESOURCES}" ".html" _AE_FOUND_HTML)
		if(NOT _AE_FOUND_HTML EQUAL -1)
			set(_AE_EM_OUT_SUFFIX ".js") # Export js if an html file is specified
		endif()

		set_target_properties(${_AE_EXECUTABLE_NAME} PROPERTIES
			LINK_FLAGS "${_AE_EM_LINKER_FLAGS}"
			OUTPUT_NAME "index"
			SUFFIX ${_AE_EM_OUT_SUFFIX}
		)
	endif()
	
	if(NOT APPLE)
		foreach(RESOURCE ${_AE_RESOURCES})
		get_filename_component(RESOURCE ${RESOURCE} ABSOLUTE)
		get_filename_component(RESOURCE_NAME ${RESOURCE} NAME)
		get_property(DESTINATION SOURCE ${RESOURCE} PROPERTY DESTINATION)
		add_custom_command(TARGET ${_AE_EXECUTABLE_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${RESOURCE}
				$<TARGET_FILE_DIR:${_AE_EXECUTABLE_NAME}>/${DESTINATION}/${RESOURCE_NAME}
		)
		endforeach()
	endif()
	
endfunction()
