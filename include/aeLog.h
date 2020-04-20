//------------------------------------------------------------------------------
// aeLog.h
// Logging functionality
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
#ifndef AELOG_H
#define AELOG_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aePlatform.h"

//------------------------------------------------------------------------------
// Log functions
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
void AE_LOG_INTERNAL( std::stringstream& os, const char* format );
#else
inline void AE_LOG_INTERNAL( std::stringstream& os, const char* format )
{
  std::cout << os.str() << format << std::endl;
}
#endif

template < typename T, typename... Args >
inline void AE_LOG_INTERNAL( std::stringstream& os, const char* format, T value, Args... args )
{
  if ( !*format )
  {
    os << std::endl;
    return;
  }
  
  const char* head = format;
  while ( *head && *head != '#' )
  {
    head++;
  }
  if ( head > format )
  {
    os.write( format, head - format );
  }

  if ( *head == '#' )
  {
    os << value;
    head++;
  }
  AE_LOG_INTERNAL( os, head, args... );
}

#define AE_LOG(...) \
do { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << _file << ":" << __LINE__ << " Info: "; \
AE_LOG_INTERNAL( os, __VA_ARGS__ ); \
} while (0)

#if _AE_DEBUG_
#define AE_LOGD(...) \
do { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << _file << ":" << __LINE__ << " Debug: "; \
AE_LOG_INTERNAL( os, __VA_ARGS__ ); \
} while (0)
#else
#define AE_LOGD(...)
#endif

#define AE_LOGI(...) \
do { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << _file << ":" << __LINE__ << " Info: "; \
AE_LOG_INTERNAL( os, __VA_ARGS__ ); \
} while (0)

#define AE_LOGW(...) \
do { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << "" << _file << ":" << __LINE__ << " \033[1;33mWarning:\033[0m "; \
AE_LOG_INTERNAL( os, __VA_ARGS__ ); \
} while (0)

#if _AE_WINDOWS_
inline void AE_ASSERT_INTERNAL()
{
  __debugbreak();
  // @TODO: Use __analysis_assume( someBool ); on windows to prevent warning C6011 (Dereferencing NULL pointer)
}
#elif _AE_EMSCRIPTEN_
inline void AE_ASSERT_INTERNAL()
{
  // @TODO: Handle asserts with emscripten builds
}
#else
inline void AE_ASSERT_INTERNAL()
{
  asm( "int $3" );
}
#endif

#define AE_ASSERT( x ) \
do { if ( !(x) ) { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << _file << ":" << __LINE__ << " Assert( " << #x << " )" << std::endl; \
AE_LOG_INTERNAL( os, "" ); \
AE_ASSERT_INTERNAL(); \
}} while (0)

#define AE_ASSERT_MSG( x, ... ) \
do { if ( !(x) ) { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << _file << ":" << __LINE__ << " Assert( " << #x << " ): "; \
AE_LOG_INTERNAL( os, __VA_ARGS__ ); \
AE_ASSERT_INTERNAL(); \
}} while (0)

#define AE_FAIL() \
do { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << _file << ":" << __LINE__ << " Error" << std::endl; \
AE_LOG_INTERNAL( os, "" ); \
AE_ASSERT_INTERNAL(); \
} while (0)

#define AE_FAIL_MSG( ... ) \
do { \
std::stringstream os; \
const char* _file = strrchr( __FILE__, '/' ); \
_file = _file ? _file + 1 : __FILE__; \
os << _file << ":" << __LINE__ << " Error: "; \
AE_LOG_INTERNAL( os, __VA_ARGS__ ); \
AE_ASSERT_INTERNAL(); \
} while (0)

#define AE_STATIC_ASSERT( _x ) static_assert( _x, "static assert" )
#define AE_STATIC_ASSERT_MSG( _x, _m ) static_assert( _x, _m )
#define AE_STATIC_FAIL( _m ) static_assert( 0, _m )

#if _AE_WINDOWS_
#define AE_WARNING( _m )
#elif _AE_APPLE_
#define AE_WARNING( _m )
#elif _AE_LINUX_
#define AE_WARNING( _m )
#elif _AE_EMSCRIPTEN_
#define AE_WARNING( _m )
#else
#define AE_WARNING( _m ) #warning _m
#endif

#endif
