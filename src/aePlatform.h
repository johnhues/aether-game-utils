//------------------------------------------------------------------------------
// aePlatform.h
// Platform specific defines
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
#ifndef AEPLATFORM_H
#define AEPLATFORM_H

//------------------------------------------------------------------------------
// Platform defines
//------------------------------------------------------------------------------
#define _AE_IOS_ 0
#define _AE_OSX_ 0
#define _AE_APPLE_ 0
#define _AE_WINDOWS_ 0
#define _AE_LINUX_ 0
#define _AE_EMSCRIPTEN_ 0

#if defined(__EMSCRIPTEN__)
  #undef _AE_EMSCRIPTEN_
  #define _AE_EMSCRIPTEN_ 1
#elif defined(__APPLE__)
  #include "TargetConditionals.h"
  #if TARGET_OS_IPHONE
    #undef _AE_IOS_
    #define _AE_IOS_ 1
  #elif TARGET_OS_MAC
    #undef _AE_OSX_
    #define _AE_OSX_ 1
  #else
    #error "Platform not supported"
  #endif
  #undef _AE_APPLE_
  #define _AE_APPLE_ 1
#elif defined(_MSC_VER)
  #undef _AE_WINDOWS_
  #define _AE_WINDOWS_ 1
#elif defined(__linux__)
  #undef _AE_LINUX_
  #define _AE_LINUX_ 1
#else
  #error "Platform not supported"
#endif

//------------------------------------------------------------------------------
// Debug defines
//------------------------------------------------------------------------------
#define _AE_DEBUG_ 0

#if ( defined(_DEBUG) || defined(DEBUG) ) && !defined(NDEBUG)
  #undef _AE_DEBUG_
  #define _AE_DEBUG_ 1
#endif

//------------------------------------------------------------------------------
// Warnings
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #define _CRT_SECURE_NO_WARNINGS
  #pragma warning( disable : 4244 )
  #pragma warning( disable : 4800 )
#endif

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <functional>
#include <ostream>
#include <type_traits>
#include <typeinfo>

#ifdef _AE_GRAPHICS_
  #if _AE_WINDOWS_
    #include <SDL.h>
    #define GLEW_STATIC 1
    #include <GL\glew.h>
  #elif _AE_EMSCRIPTEN_
    #include <SDL/SDL.h>
    #include <GLES2/gl2.h>
  #elif _AE_LINUX_
    #include <SDL.h>
    #include <GL/gl.h>
    #include <GLES3/gl3.h>
  #else
    #include <SDL.h>
    #include <OpenGL/gl3.h>
  #endif
#endif

//------------------------------------------------------------------------------
// Utils
//------------------------------------------------------------------------------
#if _AE_LINUX_
#define AE_ALIGN( _x ) __attribute__ ((aligned(_x)))
//#elif _AE_WINDOWS_
//#define AE_ALIGN( _x ) __declspec(align(_x)) // Windows doesn't support aligned function parameters
#else
#define AE_ALIGN( _x )
#endif

template < typename T, int N > char( &countof_helper( T(&)[ N ] ) )[ N ];
#define countof( _x ) ( (uint32_t)sizeof( countof_helper( _x ) ) )

inline void* aeAlignedAlloc( uint32_t size, uint32_t boundary )
{
#if _AE_WINDOWS_
  return _aligned_malloc( size, boundary );
#elif _AE_LINUX_
  return aligned_alloc( boundary, size );
#else
  // @HACK: macosx clang c++11 does not have aligned alloc
  return malloc( size );
#endif
}

inline void aeAlignedFree( void* p )
{
#if _AE_WINDOWS_
  _aligned_free( p );
#elif _AE_LINUX_
  free( p );
#else
  free( p );
#endif
}

template < typename T >
const char* aeGetTypeName()
{
  const char* typeName = typeid( T ).name();
#ifdef _MSC_VER
  if ( strncmp( typeName, "class ", 6 ) == 0 )
  {
    typeName += 6;
  }
  else if ( strncmp( typeName, "struct ", 7 ) == 0 )
  {
    typeName += 7;
  }
#else
  while ( *typeName && isdigit( typeName[ 0 ] ) )
  {
    typeName++;
  }
#endif
  return typeName;
}

#define AE_EXPORT extern "C"

// From: https://stackoverflow.com/questions/18547251/when-i-use-strlcpy-function-in-c-the-compilor-give-me-an-error
#ifndef HAVE_STRLCAT

inline
size_t                  /* O - Length of string */
strlcat(char       *dst,        /* O - Destination string */
              const char *src,      /* I - Source string */
          size_t     size)      /* I - Size of destination string buffer */
{
  size_t    srclen;         /* Length of source string */
  size_t    dstlen;         /* Length of destination string */

  dstlen = strlen(dst);
  size   -= dstlen + 1;

  if (!size)
    return (dstlen);        /* No room, return immediately... */

  srclen = strlen(src);

  if (srclen > size)
    srclen = size;

  memcpy(dst + dstlen, src, srclen);
  dst[dstlen + srclen] = '\0';

  return (dstlen + srclen);
}
#endif

#ifndef HAVE_STRLCPY

inline
size_t                  /* O - Length of string */
strlcpy(char       *dst,        /* O - Destination string */
              const char *src,      /* I - Source string */
          size_t      size)     /* I - Size of destination string buffer */
{
  size_t    srclen;         /* Length of source string */

  size --;

  srclen = strlen(src);

  if (srclen > size)
    srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}
#endif

#ifdef _MSC_VER
# define strtok_r strtok_s
#endif

#endif