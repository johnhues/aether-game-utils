cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# Meta property info for targets
if(NOT AE_CMAKE_PROPERTY_LIST)
	execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE AE_CMAKE_PROPERTY_LIST)
	
	# Convert command output into a CMake list
	string(REGEX REPLACE ";" "\\\\;" AE_CMAKE_PROPERTY_LIST "${AE_CMAKE_PROPERTY_LIST}")
	string(REGEX REPLACE "\n" ";" AE_CMAKE_PROPERTY_LIST "${AE_CMAKE_PROPERTY_LIST}")
	list(REMOVE_DUPLICATES AE_CMAKE_PROPERTY_LIST)
endif()

function(ae_print_target_properties target)
	cmake_parse_arguments(AEPTP "" "PREFIX" "" ${ARGN})
	if(NOT TARGET ${target})
		message("There is no target named '${target}'")
		return()
	endif()

	foreach(property ${AE_CMAKE_PROPERTY_LIST})
		string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" property ${property})

		# Fix https://stackoverflow.com/questions/32197663/how-can-i-remove-the-the-location-property-may-not-be-read-from-target-error-i
		if(property STREQUAL "LOCATION" OR property MATCHES "^LOCATION_" OR property MATCHES "_LOCATION$")
			continue()
		endif()

		get_property(was_set TARGET ${target} PROPERTY ${property} SET)
		if(was_set)
			get_target_property(value ${target} ${property})
			message("${AEPTP_PREFIX}${property} = ${value}")
		endif()
	endforeach()
endfunction()

function(ae_add_shared_library)
	cmake_parse_arguments(
		ADD_SHARED_LIB
		""
		"NAME;APPLE_DEVELOPMENT_TEAM;MAJOR_MINOR_PATCH_VERSION"
		"SRC_FILES;HEADER_FILES;LIBS;PUBLIC_INCLUDE_DIRS;PRIVATE_INCLUDE_DIRS"
		${ARGN}
	)
	message("  Added shared library \"${ADD_SHARED_LIB_NAME}\"")
	message("    NAME \"${ADD_SHARED_LIB_NAME}\" \(${CMAKE_BUILD_TYPE}\)")
	message("    APPLE_DEVELOPMENT_TEAM \"${ADD_SHARED_LIB_APPLE_DEVELOPMENT_TEAM}\"")
	message("    MAJOR_MINOR_PATCH_VERSION \"${ADD_SHARED_LIB_MAJOR_MINOR_PATCH_VERSION}\"")
	message("    SRC_FILES \"${ADD_SHARED_LIB_SRC_FILES}\"")
	message("    HEADER_FILES \"${ADD_SHARED_LIB_HEADER_FILES}\"")
	message("    LIBS \"${ADD_SHARED_LIB_LIBS}\"")
	message("    PUBLIC_INCLUDE_DIRS \"${ADD_SHARED_LIB_PUBLIC_INCLUDE_DIRS}\"")
	message("    PRIVATE_INCLUDE_DIRS \"${ADD_SHARED_LIB_PRIVATE_INCLUDE_DIRS}\"")
	message("")

	add_library(${ADD_SHARED_LIB_NAME} SHARED ${ADD_SHARED_LIB_SRC_FILES})
	if(ADD_SHARED_LIB_LIBS)
		target_link_libraries(${ADD_SHARED_LIB_NAME} ${ADD_SHARED_LIB_LIBS})
	endif()
	if(ADD_SHARED_LIB_INCLUDE_DIRS)
		target_include_directories(${ADD_SHARED_LIB_NAME} PRIVATE ${ADD_SHARED_LIB_INCLUDE_DIRS})
	endif()
	if(MSVC)
		set_target_properties(${ADD_SHARED_LIB_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS YES)
	elseif(APPLE)
		if (("${CMAKE_GENERATOR}" STREQUAL "Xcode"))
			if (ADD_SHARED_LIB_APPLE_DEVELOPMENT_TEAM)
				set(ADD_SHARED_LIB_APPLE_CODE_SIGN_IDENTITY "Apple Development") # See Professional CMake 24.6.1. Signing Identity And Development Team
			endif()
			set(ADD_SHARED_LIB_APPLE_SKIP_INSTALL YES) # See Professional CMake 24.7. Creating And Exporting Archives
		endif()
		set_target_properties(${ADD_SHARED_LIB_NAME} PROPERTIES
			FRAMEWORK TRUE
			FRAMEWORK_VERSION "${ADD_SHARED_LIB_MAJOR_MINOR_PATCH_VERSION}"
			PUBLIC_HEADER "${ADD_SHARED_LIB_HEADER_FILES}"

			XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${ADD_SHARED_LIB_APPLE_DEVELOPMENT_TEAM}"
			XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${ADD_SHARED_LIB_APPLE_CODE_SIGN_IDENTITY}"

			XCODE_ATTRIBUTE_SKIP_INSTALL "${ADD_SHARED_LIB_APPLE_SKIP_INSTALL}" # See Professional CMake 24.7. Creating And Exporting Archives
			BUILD_RPATH @loader_path/../../..
			INSTALL_RPATH @loader_path/../../..
		)
	endif()

	message("  ${ADD_SHARED_LIB_NAME} properties:")
	ae_print_target_properties(${ADD_SHARED_LIB_NAME} PREFIX "    ")
	message("")
endfunction()

# Bundle helper
function(ae_add_bundle BUNDLE_NAME)
	cmake_parse_arguments(
		AEAB
		""
		"BUNDLE_NAME;TARGET_NAME;BUNDLE_ID;APPLE_DEVELOPMENT_TEAM;MAJOR_MINOR_PATCH_VERSION;ICON_FILE"
		"SRC_FILES;RESOURCES;LIBS;PACKAGE_LIBS;INCLUDE_DIRS"
		${ARGN}
	)
	set(AEAB_BUNDLE_NAME "${BUNDLE_NAME}")
	if(NOT AEAB_TARGET_NAME)
		set(AEAB_TARGET_NAME "${BUNDLE_NAME}")
	endif()
	if(NOT AEAB_MAJOR_MINOR_PATCH_VERSION)
		set(AEAB_MAJOR_MINOR_PATCH_VERSION "0.0.0")
	endif()
	if(CMAKE_CONFIGURATION_TYPES)
		set(AEAB_BUILD_TYPE "Multi-Config")
	else()
		set(AEAB_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
	endif()

	message("Added ${AEAB_TARGET_NAME} \(${AEAB_BUILD_TYPE})")
	
	message("  ${AEAB_TARGET_NAME} parameters:")
	message("    BUNDLE_NAME = ${AEAB_BUNDLE_NAME}")
	message("    TARGET_NAME = ${AEAB_TARGET_NAME}")
	message("    BUNDLE_ID = ${AEAB_BUNDLE_ID}")
	message("    APPLE_DEVELOPMENT_TEAM = ${AEAB_APPLE_DEVELOPMENT_TEAM}")
	message("    MAJOR_MINOR_PATCH_VERSION = ${AEAB_MAJOR_MINOR_PATCH_VERSION}")
	message("    ICON_FILE = ${AEAB_ICON_FILE}")
	message("    SRC_FILES = ${AEAB_SRC_FILES}")
	message("    RESOURCES = ${AEAB_RESOURCES}")
	message("    LIBS = ${AEAB_LIBS}")
	message("    PACKAGE_LIBS = ${AEAB_PACKAGE_LIBS}")
	message("    INCLUDE_DIRS = ${AEAB_INCLUDE_DIRS}")

	if(WIN32)
		# Create a regular windowed application instead of the default console subsystem target
		set(AEAB_EXE_TYPE WIN32)
	elseif(APPLE)
		set(AEAB_EXE_TYPE MACOSX_BUNDLE)
	endif()

	add_executable(${AEAB_TARGET_NAME} ${AEAB_EXE_TYPE} "${AEAB_SRC_FILES}")
	target_link_libraries(${AEAB_TARGET_NAME} PRIVATE "${AEAB_LIBS};${AEAB_PACKAGE_LIBS}")
	target_include_directories(${AEAB_TARGET_NAME} PRIVATE "${AEAB_INCLUDE_DIRS}")

	if(WIN32)
		set_target_properties(${AEAB_TARGET_NAME} PROPERTIES
			LINK_FLAGS "/ENTRY:mainCRTStartup" # Use main instead of WinMain
		)
	elseif(APPLE)
		# Only add resource files to Apple bundles
		# Adding resources on Windows causes an issue where files are copied only once on configure
		if(AEAB_RESOURCES)
			target_sources(${AEAB_TARGET_NAME} PRIVATE "${AEAB_RESOURCES}")
		endif()
		set_source_files_properties(${AEAB_RESOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
		foreach(resource ${AEAB_RESOURCES})
			get_filename_component(resource_path ${resource} ABSOLUTE) # First get absolute path to resource
			file(RELATIVE_PATH resource_path ${CMAKE_CURRENT_SOURCE_DIR} ${resource_path}) # Get the relative path to the resource from the root of the resource
			cmake_path(GET resource_path PARENT_PATH resource_path) # Remove file name from path
			set_source_files_properties(${resource} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${resource_path}")
		endforeach()

		if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
			if (AEAB_APPLE_DEVELOPMENT_TEAM)
				set(AEAB_APPLE_CODE_SIGN_IDENTITY "Apple Development") # See Professional CMake 24.6.1. Signing Identity And Development Team
			endif()
			set(AEAB_APPLE_INSTALL_PATH "$(LOCAL_APPS_DIR)") # See Professional CMake 24.7. Creating And Exporting Archives
			set(AEAB_APPLE_SKIP_INSTALL "NO") # See Professional CMake 24.7. Creating And Exporting Archives
		endif()

		set_target_properties(${AEAB_TARGET_NAME} PROPERTIES
			# Xcode only
			XCODE_ATTRIBUTE_PRODUCT_NAME "${AEAB_BUNDLE_NAME}" # CFBundleName
			XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${AEAB_BUNDLE_ID}" # CFBundleIdentifier
			XCODE_ATTRIBUTE_MARKETING_VERSION "${AEAB_MAJOR_MINOR_PATCH_VERSION}"
			XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "${AEAB_MAJOR_MINOR_PATCH_VERSION}"
			XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE YES
			XCODE_ATTRIBUTE_INFOPLIST_FILE ""

			XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName "${AEAB_BUNDLE_NAME}"
			XCODE_ATTRIBUTE_INFOPLIST_KEY_LSApplicationCategoryType "public.app-category.games"
			# XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleIconFile "${AEAB_ICON_FILE}"
			XCODE_ATTRIBUTE_INFOPLIST_KEY_NSPrincipalClass "NSApplication"
			# XCODE_ATTRIBUTE_INFOPLIST_KEY_NSHumanReadable @TODO

			XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${AEAB_APPLE_DEVELOPMENT_TEAM}"
			XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${AEAB_APPLE_CODE_SIGN_IDENTITY}"

			XCODE_ATTRIBUTE_INSTALL_PATH "${AEAB_APPLE_INSTALL_PATH}" # See Professional CMake 24.7. Creating And Exporting Archives
			XCODE_ATTRIBUTE_SKIP_INSTALL "${AEAB_APPLE_SKIP_INSTALL}" # See Professional CMake 24.7. Creating And Exporting Archives

			XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO

			# All builds
			MACOSX_BUNDLE TRUE
			MACOSX_BUNDLE_GUI_IDENTIFIER "${AEAB_BUNDLE_ID}" # CFBundleIdentifier needed for NSOpenPanel etc
			MACOSX_BUNDLE_ICON_FILE "${AEAB_ICON_FILE}" # CFBundleIconFile (*.icns file path)

			OUTPUT_NAME "${AEAB_BUNDLE_NAME}"

			MACOSX_RPATH TRUE
			BUILD_RPATH @executable_path/../Frameworks
			INSTALL_RPATH @executable_path/../Frameworks
		)

		if(AEAB_PACKAGE_LIBS)
			set_target_properties(${AEAB_TARGET_NAME} PROPERTIES XCODE_EMBED_FRAMEWORKS "${AEAB_PACKAGE_LIBS}") # 24.10. Embedding Frameworks, Plugins And Extensions

			install(TARGETS ${AEAB_PACKAGE_LIBS} ${AEAB_TARGET_NAME}
				BUNDLE DESTINATION .
				FRAMEWORK DESTINATION "${AEAB_BUNDLE_NAME}.app/Contents/Frameworks"
			)
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
			# "-s FILESYSTEM=0" # Reduce code size: We do not need native POSIX filesystem emulation support (Emscripten FS/MEMFS) @TODO: Filesystem is required for sockets
			# Choose the oldest browser versions that should be supported. The higher minimum bar you choose, the less emulation code may be present for old browser quirks.
			"-s MIN_FIREFOX_VERSION=70"
			"-s MIN_SAFARI_VERSION=130000"
			"-s MIN_CHROME_VERSION=80"
			"-s ALLOW_MEMORY_GROWTH"
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
				# "-Wl,-u,htonl" # Workaround for -flto issue
				# "-flto"
				# "-fno-exceptions"
			)
			# list(APPEND _AE_EM_COMPILER_FLAGS
			# 	"-flto"
			# 	"-fno-exceptions"
			# )
		endif()
		string (REPLACE ";" " " _AE_EM_LINKER_FLAGS "${_AE_EM_LINKER_FLAGS}")
		string (REPLACE ";" " " _AE_EM_COMPILER_FLAGS "${_AE_EM_COMPILER_FLAGS}")

		set_target_properties(${AEAB_TARGET_NAME} PROPERTIES
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${AEAB_TARGET_NAME}"
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${AEAB_TARGET_NAME}"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${AEAB_TARGET_NAME}"
		)
		
		set(_AE_EM_OUT_SUFFIX ".html")
		string(FIND "${AEAB_RESOURCES}" ".html" _AE_FOUND_HTML)
		if(NOT _AE_FOUND_HTML EQUAL -1)
			set(_AE_EM_OUT_SUFFIX ".js") # Export js if an html file is specified
		endif()

		set_target_properties(${AEAB_TARGET_NAME} PROPERTIES
			LINK_FLAGS "${_AE_EM_LINKER_FLAGS}"
			COMPILE_FLAGS "${_AE_EM_COMPILER_FLAGS}"
			OUTPUT_NAME "index"
			SUFFIX ${_AE_EM_OUT_SUFFIX}
		)
	endif()
	
	if(NOT APPLE)
		foreach(RESOURCE ${AEAB_RESOURCES})
			get_filename_component(RESOURCE_PATH ${RESOURCE} ABSOLUTE)
			cmake_path(RELATIVE_PATH RESOURCE_PATH OUTPUT_VARIABLE RESOURCE_RELATIVE)
			add_custom_command(TARGET ${AEAB_TARGET_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${RESOURCE_PATH}
					$<TARGET_FILE_DIR:${AEAB_TARGET_NAME}>/${RESOURCE_RELATIVE}
			)
		endforeach()
	endif()
	
	message("  ${AEAB_TARGET_NAME} properties:")
	ae_print_target_properties(${AEAB_TARGET_NAME} PREFIX "    ")
	message("")
endfunction()
