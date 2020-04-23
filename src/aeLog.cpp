//------------------------------------------------------------------------------
// aeLog.cpp
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
#include "aeLog.h"
#include <sstream>
#if _AE_WINDOWS_
  #define WIN32_LEAN_AND_MEAN
  #include "Windows.h"
  #include "processthreadsapi.h"
#endif

//------------------------------------------------------------------------------
// Log levels
//------------------------------------------------------------------------------
const char* aeLogLevelNames[] =
{
  "TRACE",
  "DEBUG",
  "INFO ",
  "WARN ",
  "ERROR",
  "FATAL"
};

//------------------------------------------------------------------------------
// Log colors
//------------------------------------------------------------------------------
#if _AE_LOG_COLORS_
const char* aeLogLevelColors[] =
{
  "\x1b[94m",
  "\x1b[36m",
  "\x1b[32m",
  "\x1b[33m",
  "\x1b[31m",
  "\x1b[35m"
};
#endif

//------------------------------------------------------------------------------
// Platform specific logging
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
void aeLogHost( std::stringstream& os )
{
  // @TODO: Also print process name https://stackoverflow.com/questions/4570174/how-to-get-the-process-name-in-c
  os << " [" << GetCurrentProcessId() << "] ";
}

void aeLogInternal( std::stringstream& os, const char* message )
{
  os << message << std::endl;
  OutputDebugStringA( os.str().c_str() );
}
#else
void aeLogInternal( std::stringstream& os, const char* message )
{
  std::cout << os.str() << message << std::endl;
}

void aeLogHost( std::stringstream& os )
{
  os << " ";
}
#endif

//------------------------------------------------------------------------------
// Internal Logging functions
//------------------------------------------------------------------------------
void aeLogFormat( std::stringstream& os, uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format )
{
  char timeBuf[ 16 ];
  time_t t = time( nullptr );
  tm* lt = localtime( &t );
  timeBuf[ strftime( timeBuf, sizeof( timeBuf ), "%H:%M:%S", lt ) ] = '\0';

  const char* fileName = strrchr( filePath, '/' );
  fileName = fileName ? fileName + 1 : strrchr( filePath, '\\' );
  if ( !fileName )
  {
    fileName = filePath;
  }

#if _AE_LOG_COLORS_
  os << "\x1b[90m" << timeBuf;
  aeLogHost( os );
  os << aeLogLevelColors[ severity ] << aeLogLevelNames[ severity ];
  os << " \x1b[90m" << fileName << ":" << line;
#else
  os << timeBuf;
  aeLogHost( os );
  os << aeLogLevelNames[ severity ];
  os << " " << fileName << ":" << line;
#endif

  bool hasAssertInfo = ( assertInfo && assertInfo[ 0 ] );
  bool hasFormat = ( format && format[ 0 ] );
  if ( hasAssertInfo || hasFormat )
  {
    os << ": ";
  }
#if _AE_LOG_COLORS_
  os << "\x1b[0m";
#endif
  if ( hasAssertInfo )
  {
    os << assertInfo;
    if ( hasFormat )
    {
      os << " ";
    }
  }
}
