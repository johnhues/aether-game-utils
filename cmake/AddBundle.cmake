# Bundle helper
function(add_bundle _AE_BUNDLE_NAME _AE_EXECUTABLE_NAME _AE_BUNDLE_ID _AE_BUNDLE_VERSION _AE_ICNS_FILE  _AE_SRC_FILES _AE_RESOURCES _AE_LIBS _AE_INCLUDE_DIRS)
	message(STATUS "_AE_BUNDLE_NAME ${_AE_BUNDLE_NAME}")
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
		# Use main instead of WinMain
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
	endif()

	add_executable(${_AE_EXECUTABLE_NAME} ${_AE_EXE_TYPE} ${_AE_SRC_FILES})
	if (_AE_LIBS)
		target_link_libraries(${_AE_EXECUTABLE_NAME} ${_AE_LIBS})
	endif()
	if (_AE_INCLUDE_DIRS)
		target_include_directories(${_AE_EXECUTABLE_NAME} PRIVATE ${_AE_INCLUDE_DIRS})
	endif()

	if(WIN32)
		# Set the working directory while debugging in visual studio to the location of this targets CMakeLists.txt
		set_property(TARGET ${_AE_EXECUTABLE_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
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
	endif()
endfunction()
