//------------------------------------------------------------------------------
// aether.h
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
#ifndef AE_AETHER_H
#define AE_AETHER_H

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
// Debug define
//------------------------------------------------------------------------------
#if defined(_DEBUG) || defined(DEBUG) || ( _AE_APPLE_ && !defined(NDEBUG) )
  #define _AE_DEBUG_ 1
#else
  #define _AE_DEBUG_ 0
#endif

//------------------------------------------------------------------------------
// Warnings
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
  #endif
  #pragma warning( disable : 4244 )
  #pragma warning( disable : 4800 )
#endif

#if _AE_APPLE_
  #define GL_SILENCE_DEPRECATION
#endif

//------------------------------------------------------------------------------
// ae Namespace
//------------------------------------------------------------------------------
#define AE_NAMESPACE ae
// @TODO: Remove aeMath
#define aeMath ae

//------------------------------------------------------------------------------
// System Headers
//------------------------------------------------------------------------------
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <functional>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <utility>

//------------------------------------------------------------------------------
// SIMD headers
//------------------------------------------------------------------------------
#if _AE_APPLE_
  #ifdef __aarch64__
    #include <arm_neon.h>
  #else
    #include <x86intrin.h>
  #endif
#elif _AE_WINDOWS_
  #include <intrin.h>
#endif

//------------------------------------------------------------------------------
// Platform Utils
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #define aeAssert() __debugbreak()
#elif _AE_APPLE_
  #define aeAssert() __builtin_trap()
#elif _AE_EMSCRIPTEN_
  // @TODO: Handle asserts with emscripten builds
  #define aeAssert()
#else
  #define aeAssert() asm( "int $3" )
#endif

#if _AE_WINDOWS_
  #define aeCompilationWarning( _msg ) _Pragma( message _msg )
#else
  #define aeCompilationWarning( _msg ) _Pragma( "warning #_msg" )
#endif

#if _AE_LINUX_
  #define AE_ALIGN( _x ) __attribute__ ((aligned(_x)))
//#elif _AE_WINDOWS_
  // @TODO: Windows doesn't support aligned function parameters
  //#define AE_ALIGN( _x ) __declspec(align(_x))
#else
  #define AE_ALIGN( _x )
#endif

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
template < typename T, int N > char( &countof_helper( T(&)[ N ] ) )[ N ];
#define countof( _x ) ( (uint32_t)sizeof( countof_helper( _x ) ) )

#define AE_CALL_CONST( _tx, _x, _tfn, _fn ) const_cast< _tfn* >( const_cast< const _tx* >( _x )->_fn() );

namespace AE_NAMESPACE {

//------------------------------------------------------------------------------
// System functions
//------------------------------------------------------------------------------
uint32_t GetPID();
uint32_t GetMaxConcurrentThreads();
bool IsDebuggerAttached();
template < typename T > const char* GetTypeName();

//------------------------------------------------------------------------------
// Allocator
//------------------------------------------------------------------------------
using Tag = std::string;
#define AE_ALLOC_TAG_RENDER ae::Tag( "aeRender" )
#define AE_ALLOC_TAG_AUDIO ae::Tag( "aeAudio" )
#define AE_ALLOC_TAG_META ae::Tag( "aeMeta" )
#define AE_ALLOC_TAG_TERRAIN ae::Tag( "aeTerrain" )
#define AE_ALLOC_TAG_NET ae::Tag( "aeNet" )
#define AE_ALLOC_TAG_HOTSPOT ae::Tag( "aeHotSpot" )
#define AE_ALLOC_TAG_MESH ae::Tag( "aeMesh" )
#define AE_ALLOC_TAG_FIXME ae::Tag( "aeFixMe" )

// @NOTE: Use your own custom allocator by inheriting from this class and calling ae::SetGlobalAllocator()
class Allocator
{
public:
  virtual ~Allocator() {}
  virtual void* Allocate( uint32_t bytes, uint32_t alignment, Tag tag ) = 0;
  virtual void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) = 0;
  virtual void Free( void* data ) = 0;
};

void SetGlobalAllocator( Allocator* alloc );
Allocator* GetGlobalAllocator();

//------------------------------------------------------------------------------
// Allocation functions
//------------------------------------------------------------------------------
// @TODO: Remove and only use Allocator
void* AlignedAlloc( uint32_t size, uint32_t boundary );
void AlignedFree( void* p );
void* Realloc( void* p, uint32_t size, uint32_t boundary );
template < typename T > T* Allocate();
template < typename T > T* AllocateArray( uint32_t count );
template < typename T, typename ... Args > T* Allocate( Args ... args );
uint8_t* AllocateRaw( uint32_t typeSize, uint32_t typeAlignment, uint32_t count );
template < typename T > void Release( T* obj );

//------------------------------------------------------------------------------
// ae::Scratch< T > class
//------------------------------------------------------------------------------
template < typename T >
class Scratch
{
public:
  Scratch( uint32_t count );
  ~Scratch();
  
  T* Data();
  uint32_t Length() const;

  T& operator[] ( int32_t index );
  const T& operator[] ( int32_t index ) const;
  T& GetSafe( int32_t index );
  const T& GetSafe( int32_t index ) const;

private:
  T* m_data;
  uint32_t m_count;
};

//------------------------------------------------------------------------------
// Math functions
//------------------------------------------------------------------------------
const float PI = 3.14159265358979323846f;
const float TWO_PI = 2.0f * PI;
const float HALF_PI = 0.5f * PI;
const float QUARTER_PI = 0.25f * PI;

template< typename T >
T&& Min( T&& v )
{
  return std::forward< T >( v );
}
template< typename T0, typename T1, typename... Tn >
auto Min( T0&& v0, T1&& v1, Tn&&... vn )
{
  return ( v0 < v1 ) ? Min( v0, std::forward< Tn >( vn )... ) : Min( v1, std::forward< Tn >( vn )... );
}

template< typename T >
T&& Max( T&& v )
{
  return std::forward< T >( v );
}
template< typename T0, typename T1, typename... Tn >
auto Max( T0&& v0, T1&& v1, Tn&&... vn )
{
  return ( v0 > v1 ) ? Max( v0, std::forward< Tn >( vn )... ) : Max( v1, std::forward< Tn >( vn )... );
}

template<typename T>
inline T Abs(const T &x)
{
  if(x < static_cast<T>(0))
  {
    return x * static_cast<T>(-1);
  }
  return x;
}

template < typename T >
inline T Clip( T x, T min, T max )
{
  return Min( Max( x, min ), max );
}

inline float Clip01( float x )
{
  return Clip( x, 0.0f, 1.0f );
}

inline float DegToRad( float degrees )
{
  return degrees * PI / 180.0f;
}

inline float RadToDeg( float radians )
{
  return radians * 180.0f / PI;
}

inline int32_t Ceil( float f )
{
  bool positive = f >= 0.0f;
  if(positive)
  {
    int i = static_cast<int>(f);
    if( f > static_cast<float>(i) ) return i + 1;
    else return i;
  }
  else return static_cast<int>(f);
}

inline int32_t Floor( float f )
{
  bool negative = f < 0.0f;
  if(negative)
  {
    int i = static_cast<int>(f);
    if( f < static_cast<float>(i) ) return i - 1;
    else return i;
  }
  else return static_cast<int>(f);
}

inline int32_t Round( float f )
{
  if( f >= 0.0f ) return (int32_t)( f + 0.5f );
  else return (int32_t)( f - 0.5f );
}

inline uint32_t Mod( uint32_t i, uint32_t n )
{
  return i % n;
}

inline int Mod( int32_t i, int32_t n )
{
  if( i < 0 )
  {
    return ( ( i % n ) + n ) % n;
  }
  else
  {
    return i % n;
  }
}

inline float Mod( float f, float n )
{
  return fmodf( fmodf( f, n ) + n, n );
}

inline float Pow( float x, float e )
{
  return powf( x, e );
}

inline float Cos( float x )
{
  return cosf( x );
}

inline float Sin( float x )
{
  return sinf( x );
}

inline float Atan2( float y, float x )
{
  return atan2( y, x );
}

template< typename T >
constexpr T MaxValue()
{
  return std::numeric_limits< T >::max();
}

template< typename T >
constexpr T MinValue()
{
  return std::numeric_limits< T >::min();
}

template<>
constexpr float MaxValue< float >()
{
  return std::numeric_limits< float >::infinity();
}

template<>
constexpr float MinValue< float >()
{
  return -1 * std::numeric_limits< float >::infinity();
}

template<>
constexpr double MaxValue< double >()
{
  return std::numeric_limits< double >::infinity();
}

template<>
constexpr double MinValue< double >()
{
  return -1 * std::numeric_limits< double >::infinity();
}

template< typename T >
T Lerp( T start, T end, float t )
{
  return start + ( end - start ) * t;
}

inline float Delerp( float start, float end, float value )
{
  return ( value - start ) / ( end - start );
}

inline float Delerp01( float start, float end, float value )
{
  return Clip01( ( value - start ) / ( end - start ) );
}

template< typename T >
T DtLerp( T value, float snappiness, float dt, T target )
{
  return ae::Lerp( target, value, exp2( -exp2( snappiness ) * dt ) );
}

inline float DtLerpAngle( float value, float snappiness, float dt, float target )
{
  target = ae::Mod( target, ae::TWO_PI );
  float innerDist = ae::Abs( target - value );
  float preDist = ae::Abs( ( target - ae::TWO_PI ) - value );
  float postDist = ae::Abs( ( target + ae::TWO_PI ) - value );
  if ( innerDist >= preDist || innerDist >= postDist )
  {
    if ( preDist < postDist )
    {
      target -= ae::TWO_PI;
    }
    else
    {
      target += ae::TWO_PI;
    }
  }
  value = ae::DtLerp( value, snappiness, dt, target );
  return ae::Mod( value, ae::TWO_PI );
}

template< typename T >
T CosineInterpolate( T start, T end, float t )
{
  float angle = ( t * PI ) + PI;
  t = cosf(angle);
  t = ( t + 1 ) / 2.0f;
  return start + ( ( end - start ) * t );
}

inline int32_t Random( int32_t min, int32_t max )
{
  if ( min >= max )
  {
    return min;
  }
  return min + ( rand() % ( max - min ) );
}

inline float Random( float min, float max )
{
  if ( min >= max )
  {
    return min;
  }
  return min + ( ( rand() / (float)RAND_MAX ) * ( max - min ) );
}

inline bool RandomBool()
{
  return Random( 0, 2 );
}

template < typename T >
class RandomValue
{
public:
  RandomValue() {}
  RandomValue( T min, T max );
  RandomValue( T value );
  
  void SetMin( T min );
  void SetMax( T max );
  
  T GetMin() const;
  T GetMax() const;
  
  T Get() const;
  operator T() const;
  
private:
  T m_min;
  T m_max;
};

namespace Interpolation
{
  template< typename T >
  T Linear( T start, T end, float t )
  {
    return start + ( ( end - start ) * t );
  }

  template< typename T >
  T Cosine( T start, T end, float t )
  {
    float angle = ( t * ae::PI );// + ae::PI;
    t = ( 1.0f - ae::Cos( angle ) ) / 2;
    // @TODO: Needed for aeColor, support types without lerp
    return start.Lerp( end, t ); //return start + ( ( end - start ) * t );
  }
}

} // AE_NAMESPACE end

//------------------------------------------------------------------------------
// Logging functions
//------------------------------------------------------------------------------
#define AE_TRACE(...) ae::LogInternal( _AE_LOG_TRACE_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_DEBUG(...) ae::LogInternal( _AE_LOG_DEBUG_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_LOG(...) ae::LogInternal( _AE_LOG_INFO_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_INFO(...) ae::LogInternal( _AE_LOG_INFO_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_WARN(...) ae::LogInternal( _AE_LOG_WARN_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_ERR(...) ae::LogInternal( _AE_LOG_ERROR_, __FILE__, __LINE__, "", __VA_ARGS__ )

//------------------------------------------------------------------------------
// Assertion functions
//------------------------------------------------------------------------------
// @TODO: Use __analysis_assume( x ); on windows to prevent warning C6011 (Dereferencing NULL pointer)
#define AE_ASSERT( _x ) do { if ( !(_x) ) { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "AE_ASSERT( " #_x " )", "" ); aeAssert(); } } while (0)
#define AE_ASSERT_MSG( _x, ... ) do { if ( !(_x) ) { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "AE_ASSERT( " #_x " )", __VA_ARGS__ ); aeAssert(); } } while (0)
#define AE_FAIL() do { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "", "" ); aeAssert(); } while (0)
#define AE_FAIL_MSG( ... ) do { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "", __VA_ARGS__ ); aeAssert(); } while (0)

//------------------------------------------------------------------------------
// Static assertion functions
//------------------------------------------------------------------------------
#define AE_STATIC_ASSERT( _x ) static_assert( _x, "static assert" )
#define AE_STATIC_ASSERT_MSG( _x, _m ) static_assert( _x, _m )
#define AE_STATIC_FAIL( _m ) static_assert( 0, _m )

//------------------------------------------------------------------------------
// Handle missing 'standard' C functions
//------------------------------------------------------------------------------
#ifndef HAVE_STRLCAT
inline size_t strlcat( char* dst, const char* src, size_t size )
{
  size_t dstlen = strlen( dst );
  size -= dstlen + 1;

  if ( !size )
  {
    return dstlen;
  }

  size_t srclen = strlen( src );
  if ( srclen > size )
  {
    srclen = size;
  }

  memcpy( dst + dstlen, src, srclen );
  dst[ dstlen + srclen ] = '\0';

  return ( dstlen + srclen );
}
#endif

#ifndef HAVE_STRLCPY
inline size_t strlcpy( char* dst, const char* src, size_t size )
{
  size--;

  size_t srclen = strlen( src );
  if ( srclen > size )
  {
    srclen = size;
  }

  memcpy( dst, src, srclen );
  dst[ srclen ] = '\0';

  return srclen;
}
#endif

#ifdef _MSC_VER
# define strtok_r strtok_s
#endif

//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
//
//
//
//
// Internal implementation beyond this point
//
//
//
//
//------------------------------------------------------------------------------
// System internal implementation
//------------------------------------------------------------------------------
namespace AE_NAMESPACE {

inline void* AlignedAlloc( uint32_t size, uint32_t boundary )
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

inline void AlignedFree( void* p )
{
#if _AE_WINDOWS_
  _aligned_free( p );
#elif _AE_LINUX_
  free( p );
#else
  free( p );
#endif
}

inline void* Realloc( void* p, uint32_t size, uint32_t boundary )
{
#if _AE_WINDOWS_
  return _aligned_realloc( p, size, boundary );
#else
  aeCompilationWarning( "Aligned realloc() not determined on this platform" )
    return nullptr;
#endif
}

template < typename T >
const char* GetTypeName()
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

#if defined(__aarch64__) && _AE_OSX_
  // @NOTE: Typeinfo appears to be missing for float16_t
  template <> const char* GetTypeName< float16_t >();
#endif

//------------------------------------------------------------------------------
// Log levels internal implementation
//------------------------------------------------------------------------------
#define _AE_LOG_TRACE_ 0
#define _AE_LOG_DEBUG_ 1
#define _AE_LOG_INFO_ 2
#define _AE_LOG_WARN_ 3
#define _AE_LOG_ERROR_ 4
#define _AE_LOG_FATAL_ 5
extern const char* LogLevelNames[ 6 ];

//------------------------------------------------------------------------------
// Log colors internal implementation
//------------------------------------------------------------------------------
#if _AE_WINDOWS_ || _AE_APPLE_
#define _AE_LOG_COLORS_ false
#else
#define _AE_LOG_COLORS_ true
extern const char* LogLevelColors[ 6 ];
#endif

//------------------------------------------------------------------------------
// Internal Logging functions internal implementation
//------------------------------------------------------------------------------
void LogInternal( std::stringstream& os, const char* message );
void LogFormat( std::stringstream& os, uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format );

template < typename T, typename... Args >
void LogInternal( std::stringstream& os, const char* format, T value, Args... args )
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

  LogInternal( os, head, args... );
}

template < typename... Args >
void LogInternal( uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format, Args... args )
{
  std::stringstream os;
  LogFormat( os, severity, filePath, line, assertInfo, format );
  LogInternal( os, format, args... );
}

//------------------------------------------------------------------------------
// Allocation functions
//------------------------------------------------------------------------------
const uint32_t kDefaultAlignment = 16;
const uint32_t kHeaderSize = 16;
struct Header
{
  uint32_t check;
  uint32_t count;
  uint32_t size;
  uint32_t typeSize;
};

template < typename T >
T* Allocate()
{
  return AllocateArray< T >( 1 );
}

template < typename T >
T* AllocateArray( uint32_t count )
{
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );
  AE_STATIC_ASSERT( sizeof( T ) % alignof( T ) == 0 ); // All elements in array should have correct alignment

  uint32_t size = kHeaderSize + sizeof( T ) * count;
  uint8_t* base = (uint8_t*)ae::AlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  AE_STATIC_ASSERT( sizeof( Header ) <= kHeaderSize );
  AE_STATIC_ASSERT( kHeaderSize % kDefaultAlignment == 0 );

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = count;
  header->size = size;
  header->typeSize = sizeof( T );

  T* result = (T*)( base + kHeaderSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    new( &result[ i ] ) T();
  }

  return result;
}

template < typename T, typename ... Args >
T* Allocate( Args ... args )
{
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );

  uint32_t size = kHeaderSize + sizeof( T );
  uint8_t* base = (uint8_t*)ae::AlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = 1;
  header->size = size;
  header->typeSize = sizeof( T );

  return new( (T*)( base + kHeaderSize ) ) T( args ... );
}

template < typename T >
void Release( T* obj )
{
  if ( !obj )
  {
    return;
  }

  AE_ASSERT( (intptr_t)obj % kDefaultAlignment == 0 );
  uint8_t* base = (uint8_t*)obj - kHeaderSize;

  Header* header = (Header*)( base );
  AE_ASSERT( header->check == 0xABCD );

  uint32_t count = header->count;
  AE_ASSERT_MSG( sizeof( T ) <= header->typeSize, "Released type T '#' does not match allocated type of size #", ae::GetTypeName< T >(), header->typeSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    T* o = (T*)( (uint8_t*)obj + header->typeSize * i );
    o->~T();
  }

#if _AE_DEBUG_
  memset( (void*)base, 0xDD, header->size );
#endif

  ae::AlignedFree( base );
}

//------------------------------------------------------------------------------
// ae::Scratch< T > member functions
//------------------------------------------------------------------------------
template < typename T >
Scratch< T >::Scratch( uint32_t count )
{
  m_count = count;
  m_data = AllocateArray< T >( count );
}

template < typename T >
Scratch< T >::~Scratch()
{
  Release( m_data );
}

template < typename T >
T* Scratch< T >::Data()
{
  return m_data;
}

template < typename T >
uint32_t Scratch< T >::Length() const
{
  return m_count;
}

template < typename T >
T& Scratch< T >::operator[] ( int32_t index )
{
  return m_data[ index ];
}

template < typename T >
const T& Scratch< T >::operator[] ( int32_t index ) const
{
  return m_data[ index ];
}

template < typename T >
T& Scratch< T >::GetSafe( int32_t index )
{
  AE_ASSERT( index < (int32_t)m_count );
  return m_data[ index ];
}

template < typename T >
const T& Scratch< T >::GetSafe( int32_t index ) const
{
  AE_ASSERT( index < (int32_t)m_count );
  return m_data[ index ];
}

//------------------------------------------------------------------------------
// RandomValue member functions
//------------------------------------------------------------------------------
template < typename T >
inline aeMath::RandomValue< T >::RandomValue( T min, T max ) : m_min(min), m_max(max) {}

template < typename T >
inline aeMath::RandomValue< T >::RandomValue( T value ) : m_min(value), m_max(value) {}

template < typename T >
inline void aeMath::RandomValue< T >::SetMin( T min )
{
  m_min = min;
}

template < typename T >
inline void aeMath::RandomValue< T >::SetMax( T max )
{
  m_max = max;
}

template < typename T >
inline T aeMath::RandomValue< T >::GetMin() const
{
  return m_min;
}

template < typename T >
inline T aeMath::RandomValue< T >::GetMax() const
{
  return m_max;
}

template < typename T >
inline T aeMath::RandomValue< T >::Get() const
{
  return Random( m_min, m_max );
}

template < typename T >
inline aeMath::RandomValue< T >::operator T() const
{
  return Get();
}

} // AE_NAMESPACE end

//------------------------------------------------------------------------------
// The following should be compiled into a single module and linked with the
// application. It's worth putting this in it's own module to limit the
// number of dependencies brought into your own code. For instance 'Windows.h'
// is included and this can easily cause naming conflicts with gameplay/engine
// code.
// Usage inside a cpp/mm file is:
//
// // ae.cpp/mm EXAMPLE START
//
// #define AE_MAIN
// #include "aether.h"
//
// // ae.cpp/mm EXAMPLE END
//------------------------------------------------------------------------------
#ifdef AE_MAIN

//------------------------------------------------------------------------------
// Platform includes, required for logging, windowing, file io
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #define WIN32_LEAN_AND_MEAN
  #include "Windows.h"
  #include "processthreadsapi.h" // For GetCurrentProcessId()
#elif _AE_APPLE_
  #include <sys/sysctl.h>
  #include <unistd.h>
#else
  #include <unistd.h>
#endif
#include <thread>

//------------------------------------------------------------------------------
// System functions internal implementation
//------------------------------------------------------------------------------
namespace AE_NAMESPACE {

uint32_t GetPID()
{
#if _AE_WINDOWS_
  return GetCurrentProcessId();
#else
  return getpid();
#endif
}

uint32_t GetMaxConcurrentThreads()
{
  return std::thread::hardware_concurrency();
}

#if _AE_APPLE_
bool IsDebuggerAttached()
{
  struct kinfo_proc info;
  info.kp_proc.p_flag = 0;

  // Initialize mib, which tells sysctl the info we want, in this case
  // we're looking for information about a specific process ID.
  int mib[ 4 ];
  mib[ 0 ] = CTL_KERN;
  mib[ 1 ] = KERN_PROC;
  mib[ 2 ] = KERN_PROC_PID;
  mib[ 3 ] = getpid();

  // Call sysctl
  size_t size = sizeof( info );
  int result = sysctl( mib, sizeof( mib ) / sizeof( *mib ), &info, &size, NULL, 0 );
  AE_ASSERT( result == 0 );

  // Application is being debugged if the P_TRACED flag is set
  return ( ( info.kp_proc.p_flag & P_TRACED ) != 0 );
}
#elif _AE_WINDOWS_
bool IsDebuggerAttached()
{
  return IsDebuggerPresent();
}
#else
bool IsDebuggerAttached()
{
  return false;
}
#endif

//------------------------------------------------------------------------------
// Log levels internal implementation
//------------------------------------------------------------------------------
const char* LogLevelNames[] =
{
  "TRACE",
  "DEBUG",
  "INFO ",
  "WARN ",
  "ERROR",
  "FATAL"
};

//------------------------------------------------------------------------------
// Log colors internal implementation
//------------------------------------------------------------------------------
#if _AE_LOG_COLORS_
const char* LogLevelColors[] =
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
// Logging functions internal implementation
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
void LogInternal( std::stringstream& os, const char* message )
{
  os << message << std::endl;
  printf( os.str().c_str() ); // std out
  OutputDebugStringA( os.str().c_str() ); // visual studio debug output
}
#else
void LogInternal( std::stringstream& os, const char* message )
{
  std::cout << os.str() << message << std::endl;
}
#endif

void LogFormat( std::stringstream& os, uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format )
{
  char timeBuf[ 16 ];
  time_t t = time( nullptr );
  tm* lt = localtime( &t );
  timeBuf[ strftime( timeBuf, sizeof( timeBuf ), "%H:%M:%S", lt ) ] = '\0';

  const char* fileName = strrchr( filePath, '/' );
  if ( fileName )
  {
    fileName++; // Remove end forward slash
  }
  else if ( ( fileName = strrchr( filePath, '\\' ) ) )
  {
    fileName++; // Remove end backslash
  }
  else
  {
    fileName = filePath;
  }

#if _AE_LOG_COLORS_
  os << "\x1b[90m" << timeBuf;
  os << " [" << ae::GetPID() << "] ";
  os << LogLevelColors[ severity ] << LogLevelNames[ severity ];
  os << " \x1b[90m" << fileName << ":" << line;
#else
  os << timeBuf;
  os << " [" << ae::GetPID() << "] ";
  os << LogLevelNames[ severity ];
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

//------------------------------------------------------------------------------
// DefaultAllocator class
//------------------------------------------------------------------------------
class DefaultAllocator final : public Allocator
{
public:
  void* Allocate( uint32_t bytes, uint32_t alignment, Tag tag ) override
  {
    return ae::AlignedAlloc( bytes, alignment );
  }

  void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) override
  {
    return ae::Realloc( (uint8_t*)data, bytes, alignment );
  }

  void Free( void* data ) override
  {
    free( (uint8_t*)data );
  //  ae::Release( (uint8_t*)data );
  }
};

//------------------------------------------------------------------------------
// Allocator functions
//------------------------------------------------------------------------------
static Allocator* g_allocator = nullptr;

void SetGlobalAllocator( Allocator* alloc )
{
  AE_ASSERT_MSG( alloc, "No allocator provided to ae::SetGlobalAllocator()" );
  AE_ASSERT_MSG( !g_allocator, "Call ae::SetGlobalAllocator() before making any allocations to use your own allocator" );
  g_allocator = alloc;
}

Allocator* GetGlobalAllocator()
{
  if ( !g_allocator )
  {
    g_allocator = ae::Allocate< DefaultAllocator >();
  }
  return g_allocator;
}

uint8_t* AllocateRaw( uint32_t typeSize, uint32_t typeAlignment, uint32_t count )
{
#if _AE_ALLOC_DISABLE
  return AllocateArray< uint8_t >( count );
#else
  AE_ASSERT( typeAlignment );
  AE_ASSERT( typeAlignment <= kDefaultAlignment );
  AE_ASSERT( typeSize % typeAlignment == 0 ); // All elements in array should have correct alignment

  uint32_t size = kHeaderSize + typeSize * count;
  uint8_t* base = (uint8_t*)ae::AlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = count;
  header->size = size;
  header->typeSize = typeSize;

  return base + kHeaderSize;
#endif
}

} // AE_NAMESPACE end

#endif // AE_MAIN
#endif // AE_AETHER_H
