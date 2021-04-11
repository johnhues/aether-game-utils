//------------------------------------------------------------------------------
// aePlatform.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aePlatform.h"
#if _AE_WINDOWS_
  #define WIN32_LEAN_AND_MEAN
  #include "Windows.h"
  #include "processthreadsapi.h"
#elif _AE_APPLE_
  #include <sys/sysctl.h>
  #include <unistd.h>
#else
  #include <unistd.h>
#endif
#include <thread>

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
#if _AE_OSX_
  template <>
  const char* aeGetTypeName< float16_t >() { return "float16_t"; }
#endif

//------------------------------------------------------------------------------
// Utilities
//------------------------------------------------------------------------------
uint32_t aeGetPID()
{
#if _AE_WINDOWS_
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}

uint32_t aeGetMaxConcurrentThreads()
{
	return std::thread::hardware_concurrency();
}

#if _AE_APPLE_
bool aeIsDebuggerAttached()
{
	int                 junk;
	int                 mib[ 4 ];
	struct kinfo_proc   info;
	size_t              size;

	// Initialize the flags so that, if sysctl fails for some bizarre
	// reason, we get a predictable result.

	info.kp_proc.p_flag = 0;

	// Initialize mib, which tells sysctl the info we want, in this case
	// we're looking for information about a specific process ID.

	mib[ 0 ] = CTL_KERN;
	mib[ 1 ] = KERN_PROC;
	mib[ 2 ] = KERN_PROC_PID;
	mib[ 3 ] = getpid();

	// Call sysctl
	size = sizeof( info );
	junk = sysctl( mib, sizeof( mib ) / sizeof( *mib ), &info, &size, NULL, 0 );
	assert( junk == 0 );

	// We're being debugged if the P_TRACED flag is set.

	return ( ( info.kp_proc.p_flag & P_TRACED ) != 0 );
}
#elif _AE_WINDOWS_
bool aeIsDebuggerAttached()
{
	return IsDebuggerPresent();
}
#else
bool aeIsDebuggerAttached()
{
	return false;
}
#endif
