# Cross compiles for Windows with clang-cl and lld-link against the MSVC CRT and
# Windows SDK. The first configure downloads both into <build>/xwin with xwin,
# accepting the license at https://go.microsoft.com/fwlink/?LinkId=2086102.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

if(NOT AE_XWIN_DIR)
	set(AE_XWIN_DIR "${CMAKE_BINARY_DIR}/xwin")
endif()
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES AE_XWIN_DIR)
set(_ae_sysroot "${AE_XWIN_DIR}/splat")
if(NOT EXISTS "${_ae_sysroot}")
	find_program(AE_XWIN xwin REQUIRED)
	# Up to three attempts, each resuming from the download cache.
	foreach(_ae_attempt 1 2 3)
		file(REMOVE_RECURSE "${_ae_sysroot}.tmp")
		execute_process(
			COMMAND "${CMAKE_COMMAND}" -E env --unset=XWIN_ACCEPT_LICENSE
				"${AE_XWIN}" --accept-license --http-retry 3 --cache-dir "${AE_XWIN_DIR}/cache" --arch x86_64
				--crt-version 14.44.17.14 --sdk-version 10.0.26100
				splat --use-winsysroot-style --preserve-ms-arch-notation --output "${_ae_sysroot}.tmp"
			RESULT_VARIABLE _ae_xwin_result)
		if(_ae_xwin_result EQUAL 0)
			break()
		endif()
	endforeach()
	if(NOT _ae_xwin_result EQUAL 0)
		message(FATAL_ERROR "xwin failed to download the MSVC CRT and Windows SDK")
	endif()
	file(RENAME "${_ae_sysroot}.tmp" "${_ae_sysroot}")
	file(REMOVE_RECURSE "${AE_XWIN_DIR}/cache")
endif()

# Newest versioned LLVM first. MSVC STL 14.44 needs clang 19 or newer.
# scripts/coverage_build.sh searches in the same order.
file(GLOB _ae_llvm_versioned /opt/homebrew/opt/llvm@*/bin /usr/local/opt/llvm@*/bin /usr/lib/llvm-*/bin)
list(SORT _ae_llvm_versioned COMPARE NATURAL ORDER DESCENDING)
set(_ae_llvm_hints /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin ${_ae_llvm_versioned})
find_program(CMAKE_C_COMPILER clang-cl HINTS ${_ae_llvm_hints} REQUIRED)
find_program(CMAKE_CXX_COMPILER clang-cl HINTS ${_ae_llvm_hints} REQUIRED)
find_program(CMAKE_LINKER lld-link HINTS ${_ae_llvm_hints} REQUIRED)
find_program(CMAKE_AR llvm-lib HINTS ${_ae_llvm_hints} REQUIRED)
find_program(CMAKE_RC_COMPILER llvm-rc HINTS ${_ae_llvm_hints} REQUIRED)
find_program(CMAKE_MT llvm-mt HINTS ${_ae_llvm_hints} REQUIRED)

set(CMAKE_C_COMPILER_TARGET x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)
set(CMAKE_C_FLAGS_INIT "/winsysroot ${_ae_sysroot}")
set(CMAKE_CXX_FLAGS_INIT "/winsysroot ${_ae_sysroot}")
foreach(_ae_type EXE SHARED MODULE)
	set(CMAKE_${_ae_type}_LINKER_FLAGS_INIT "/winsysroot:${_ae_sysroot}")
endforeach()
# xwin leaves out the debug CRT, so every configuration links the release one.
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

set(CMAKE_FIND_ROOT_PATH "${_ae_sysroot}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
