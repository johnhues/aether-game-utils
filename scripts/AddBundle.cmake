cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
if(POLICY CMP0156)
	cmake_policy(SET CMP0156 NEW) # Link each static library once when the linker allows, the Apple linker warns about repeats
endif()

# aether.h and aether.js live one directory up
set(_AE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

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
	# if(NOT ${AE_VERBOSE})
		return()
	# endif()

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
	if(WIN32)
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
		"TARGET_NAME;BUNDLE_ID;APPLE_DEVELOPMENT_TEAM;MAJOR_MINOR_PATCH_VERSION;ICON_FILE"
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
	
	if(${AE_VERBOSE})
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
	endif()

	if(WIN32)
		# Create a regular windowed application instead of the default console subsystem target
		set(AEAB_EXE_TYPE WIN32)
	elseif(APPLE)
		set(AEAB_EXE_TYPE MACOSX_BUNDLE)
	endif()

	add_executable(${AEAB_TARGET_NAME} ${AEAB_EXE_TYPE} "${AEAB_SRC_FILES}")
	target_link_libraries(${AEAB_TARGET_NAME} PRIVATE "${AEAB_LIBS};${AEAB_PACKAGE_LIBS}")
	target_include_directories(${AEAB_TARGET_NAME} PRIVATE "${AEAB_INCLUDE_DIRS}")

	if(MSVC)
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
			# Without a team the project is configured unsigned so any team-less
			# xcodebuild invocation (eg. cmake-tools' F5 auto-build) succeeds;
			# scripts/ios_build.py overrides DEVELOPMENT_TEAM + sign-enable
			# settings at xcodebuild runtime for the on-device build.
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
			# XCODE_ATTRIBUTE_INFOPLIST_KEY_NSHumanReadable @TODO

			XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${AEAB_APPLE_DEVELOPMENT_TEAM}"
			XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${AEAB_APPLE_CODE_SIGN_IDENTITY}"

			XCODE_ATTRIBUTE_INSTALL_PATH "${AEAB_APPLE_INSTALL_PATH}" # See Professional CMake 24.7. Creating And Exporting Archives
			XCODE_ATTRIBUTE_SKIP_INSTALL "${AEAB_APPLE_SKIP_INSTALL}" # See Professional CMake 24.7. Creating And Exporting Archives

			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO

			# All builds
			MACOSX_BUNDLE TRUE
			MACOSX_BUNDLE_GUI_IDENTIFIER "${AEAB_BUNDLE_ID}" # CFBundleIdentifier needed for NSOpenPanel etc
			MACOSX_BUNDLE_ICON_FILE "${AEAB_ICON_FILE}" # CFBundleIconFile (*.icns file path)
			MACOSX_BUNDLE_BUNDLE_NAME "${AEAB_BUNDLE_NAME}" # CFBundleName; XCODE_ATTRIBUTE_PRODUCT_NAME above only applies to the Xcode generator
			MACOSX_BUNDLE_SHORT_VERSION_STRING "${AEAB_MAJOR_MINOR_PATCH_VERSION}" # CFBundleShortVersionString
			MACOSX_BUNDLE_BUNDLE_VERSION "${AEAB_MAJOR_MINOR_PATCH_VERSION}" # CFBundleVersion

			OUTPUT_NAME "${AEAB_BUNDLE_NAME}"
		)

		# Reconfigures when a bundle Info.plist goes missing. CMake writes it while
		# generating, so a build alone can never restore it. Result is unused.
		file(GLOB AEAB_INFO_PLIST CONFIGURE_DEPENDS
			"${CMAKE_CURRENT_BINARY_DIR}/${AEAB_BUNDLE_NAME}.app/Contents/Info.plist"
			"${CMAKE_CURRENT_BINARY_DIR}/*/${AEAB_BUNDLE_NAME}.app/Contents/Info.plist"
		)

		if(IOS)
			if(AEAB_APPLE_DEVELOPMENT_TEAM)
				set(AEAB_IOS_CODE_SIGNING "YES")
			else()
				set(AEAB_IOS_CODE_SIGNING "NO")
			endif()
			set_target_properties(${AEAB_TARGET_NAME} PROPERTIES
				XCODE_ATTRIBUTE_INFOPLIST_KEY_NSPrincipalClass "UIApplication"
				XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME NO
				XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
				XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
				XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "${AEAB_IOS_CODE_SIGNING}"
				XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "${AEAB_IOS_CODE_SIGNING}"
				XCODE_ATTRIBUTE_INFOPLIST_KEY_UILaunchScreen_Generation YES
				XCODE_ATTRIBUTE_INFOPLIST_KEY_UISupportedInterfaceOrientations
					"UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown"
				XCODE_ATTRIBUTE_INFOPLIST_KEY_UISupportedInterfaceOrientations_iPad
					"UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown"
				XCODE_ATTRIBUTE_INFOPLIST_KEY_ITSAppUsesNonExemptEncryption NO
				XCODE_ATTRIBUTE_INFOPLIST_KEY_UIApplicationSupportsIndirectInputEvents YES
			)
			target_link_libraries(${AEAB_TARGET_NAME} PRIVATE
				"-framework UIKit" "-framework Foundation" "-framework QuartzCore"
				"-framework GameController" "-framework AudioToolbox" "-framework AVFoundation"
			)
		else()
			set_target_properties(${AEAB_TARGET_NAME} PROPERTIES
				XCODE_ATTRIBUTE_INFOPLIST_KEY_NSPrincipalClass "NSApplication"
				XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
				MACOSX_RPATH TRUE
				BUILD_RPATH @executable_path/../Frameworks
				INSTALL_RPATH @executable_path/../Frameworks
			)
		endif()

		if(AEAB_PACKAGE_LIBS)
			set_target_properties(${AEAB_TARGET_NAME} PROPERTIES XCODE_EMBED_FRAMEWORKS "${AEAB_PACKAGE_LIBS}") # 24.10. Embedding Frameworks, Plugins And Extensions

			install(TARGETS ${AEAB_PACKAGE_LIBS} ${AEAB_TARGET_NAME}
				BUNDLE DESTINATION .
				FRAMEWORK DESTINATION "${AEAB_BUNDLE_NAME}.app/Contents/Frameworks"
			)
		endif()
	elseif(WASI)
		# A reactor, so main() returns into aether.js through ae::Application()
		target_link_options(${AEAB_TARGET_NAME} PRIVATE
			"-mexec-model=reactor"
			"-Wl,-z,stack-size=1048576" # Matches the smallest desktop main thread stack (Windows)
			"-Wl,--initial-memory=16777216" # Reserved up front by every instance, mobile browsers can refuse large memories
			"-Wl,--max-memory=268435456" # Upper limit of growth for each instance
		)
		# $<CONFIG> is explicit so multi-config generators do not append it after
		# the target directory. Matches the <config>/<bundle> layout of the other
		# platforms.
		set_target_properties(${AEAB_TARGET_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${AEAB_TARGET_NAME}"
			OUTPUT_NAME "index"
			SUFFIX ".wasm"
		)
		add_custom_command(TARGET ${AEAB_TARGET_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${_AE_ROOT_DIR}/aether.js"
				$<TARGET_FILE_DIR:${AEAB_TARGET_NAME}>/aether.js
		)
		set_property(TARGET ${AEAB_TARGET_NAME} APPEND PROPERTY LINK_DEPENDS "${_AE_ROOT_DIR}/aether.js")
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
			# Relinks when the resource changes so the copy above runs (Ninja and Makefile generators)
			set_property(TARGET ${AEAB_TARGET_NAME} APPEND PROPERTY LINK_DEPENDS ${RESOURCE_PATH})
		endforeach()
	endif()
	
	message("  ${AEAB_TARGET_NAME} properties:")
	ae_print_target_properties(${AEAB_TARGET_NAME} PREFIX "    ")
	message("")
endfunction()
