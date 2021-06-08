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
// Platform functions
//------------------------------------------------------------------------------
uint32_t GetPID();
uint32_t GetMaxConcurrentThreads();
bool IsDebuggerAttached();
template < typename T > const char* GetTypeName();
double GetTime();

//------------------------------------------------------------------------------
// Tags
//------------------------------------------------------------------------------
using Tag = std::string; // @TODO: Fixed length string
#define AE_ALLOC_TAG_RENDER ae::Tag( "aeGraphics" )
#define AE_ALLOC_TAG_AUDIO ae::Tag( "aeAudio" )
#define AE_ALLOC_TAG_META ae::Tag( "aeMeta" )
#define AE_ALLOC_TAG_TERRAIN ae::Tag( "aeTerrain" )
#define AE_ALLOC_TAG_NET ae::Tag( "aeNet" )
#define AE_ALLOC_TAG_HOTSPOT ae::Tag( "aeHotSpot" )
#define AE_ALLOC_TAG_MESH ae::Tag( "aeMesh" )
#define AE_ALLOC_TAG_SCRATCH ae::Tag( "aeScratch" )
#define AE_ALLOC_TAG_FIXME ae::Tag( "aeFixMe" )
#define AE_ALLOC_TAG_FILE ae::Tag( "aeFile" )

//------------------------------------------------------------------------------
// Allocator interface
// @NOTE: By default aether-game-utils uses system allocations, which may be fine
// for your use case. If not it's advised that you implement this class with
// dlmalloc or similar and then call ae::SetGlobalAllocator() with your own
// allocator when your program starts.
//------------------------------------------------------------------------------
class Allocator
{
public:
  virtual ~Allocator() {}
  virtual void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) = 0;
  virtual void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) = 0;
  virtual void Free( void* data ) = 0;
};
//------------------------------------------------------------------------------
// @NOTE: Call ae::SetGlobalAllocator() before making any allocations or else a
// default allocator will be used. You must call ae::SetGlobalAllocator() before
// any allocations are made.
//------------------------------------------------------------------------------
void SetGlobalAllocator( Allocator* alloc );
Allocator* GetGlobalAllocator();
//------------------------------------------------------------------------------
// Allocation functions
// @NOTE: All allocations are tagged, (@TODO) they can be inspected through the
// current ae::Allocator with ae::GetGlobalAllocator()
//------------------------------------------------------------------------------
// C++ style allocations
template < typename T > T* NewArray( ae::Tag tag, uint32_t count );
template < typename T, typename ... Args > T* New( ae::Tag tag, Args ... args );
template < typename T > void Delete( T* obj );
// C style allocations
void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment );
void* Reallocate( void* data, uint32_t bytes, uint32_t alignment );
void Free( void* data );

//------------------------------------------------------------------------------
// ae::Scratch< T > class
//------------------------------------------------------------------------------
// @NOTE: This class is useful for scoped allocations. (@TODO) In the future it
// should allocate from a stack. This should allow big allocations to happen
// cheaply each frame without creating any fragmentation.
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
// Math defines
//------------------------------------------------------------------------------
const float PI = 3.14159265358979323846f;
const float TWO_PI = 2.0f * PI;
const float HALF_PI = 0.5f * PI;
const float QUARTER_PI = 0.25f * PI;

//------------------------------------------------------------------------------
// Standard math operations
//------------------------------------------------------------------------------
inline float Pow( float x, float e );
inline float Cos( float x );
inline float Sin( float x );
inline float Atan2( float y, float x );

inline uint32_t Mod( uint32_t i, uint32_t n );
inline int Mod( int32_t i, int32_t n );
inline float Mod( float f, float n );

inline int32_t Ceil( float f );
inline int32_t Floor( float f );
inline int32_t Round( float f );

//------------------------------------------------------------------------------
// Range functions
//------------------------------------------------------------------------------
template< typename T0, typename T1, typename... Tn > auto Min( T0&& v0, T1&& v1, Tn&&... vn );
template< typename T0, typename T1, typename... Tn > auto Max( T0&& v0, T1&& v1, Tn&&... vn );
template< typename T > inline T Abs( const T &x );

template < typename T > inline T Clip( T x, T min, T max );
inline float Clip01( float x );

//------------------------------------------------------------------------------
// Interpolation
//------------------------------------------------------------------------------
template< typename T > T Lerp( T start, T end, float t );
inline float Delerp( float start, float end, float value );
inline float Delerp01( float start, float end, float value );
template< typename T > T DtLerp( T value, float snappiness, float dt, T target );
inline float DtLerpAngle( float value, float snappiness, float dt, float target );
// @TODO: Cleanup duplicate interpolation functions
template< typename T > T CosineInterpolate( T start, T end, float t );
namespace Interpolation
{
  template< typename T > T Linear( T start, T end, float t );
  template< typename T > T Cosine( T start, T end, float t );
}

//------------------------------------------------------------------------------
// Angle functions
//------------------------------------------------------------------------------
inline float DegToRad( float degrees );
inline float RadToDeg( float radians );

//------------------------------------------------------------------------------
// Type specific limits
//------------------------------------------------------------------------------
template< typename T > constexpr T MaxValue();
template< typename T > constexpr T MinValue();

//------------------------------------------------------------------------------
// ae::Vec2 shared member functions
// ae::Vec3 shared member functions
// ae::Vec4 shared member functions
//------------------------------------------------------------------------------
// @NOTE: Vec2 Vec3 and Vec4 share these functions. They act on each component
// of the vector, so in the case of Vec4 a dot product is implemented as
// (a.x*b.x)+(a.y*b.y)+(a.z*b.z)+(a.w*b.w).
template < typename T >
struct _VecT
{
  _VecT() {}
  _VecT( bool ) = delete;

  bool operator==( const T& v ) const;
  bool operator!=( const T& v ) const;
  
  float operator[]( uint32_t idx ) const;
  float& operator[]( uint32_t idx );
  
  T operator-() const;
  T operator*( float s ) const;
  T operator/( float s ) const;
  T operator+( const T& v ) const;
  T operator-( const T& v ) const;
  T operator*( const T& v ) const;
  T operator/( const T& v ) const;
  void operator*=( float s );
  void operator/=( float s );
  void operator+=( const T& v );
  void operator-=( const T& v );
  void operator*=( const T& v );
  void operator/=( const T& v );
  
  static float Dot( const T& v0, const T& v1 );
  float Dot( const T& v ) const;
  float Length() const;
  float LengthSquared() const;
  
  float Normalize();
  float SafeNormalize( float epsilon = 0.000001f );
  T NormalizeCopy() const;
  T SafeNormalizeCopy( float epsilon = 0.000001f ) const;
  float Trim( float length );
};

#pragma warning(disable:26495) // Vecs are left uninitialized for performance

//------------------------------------------------------------------------------
// ae::Vec2 struct
//------------------------------------------------------------------------------
struct Vec2 : public _VecT< Vec2 >
{
  Vec2() {} // Empty default constructor for performance of vertex arrays etc
  Vec2( const Vec2& ) = default;
  explicit Vec2( float v );
  Vec2( float x, float y );
  explicit Vec2( const float* v2 );
  static Vec2 FromAngle( float angle );
  struct
  {
    float x;
    float y;
  };
  float data[ 2 ];
};
// @HACK: For Window
using Int2 = Vec2;

//------------------------------------------------------------------------------
// ae::Vec3 struct
//------------------------------------------------------------------------------
struct Vec3 : public _VecT< Vec3 >
{
  Vec3() {} // Empty default constructor for performance of vertex arrays etc
  explicit Vec3( float v );
  Vec3( float x, float y, float z );
  explicit Vec3( const float* v3 );
  Vec3( Vec2 xy, float z ); // @TODO: Support Y up
  explicit Vec3( Vec2 xy );
  explicit operator Vec2() const;
  Vec2 GetXY() const;
  Vec2 GetXZ() const;
  // Int3 NearestCopy() const; // @TODO
  // Int3 FloorCopy() const; // @TODO
  // Int3 CeilCopy() const; // @TODO
  
  float GetAngleBetween( const Vec3& v, float epsilon = 0.0001f ) const;
  void AddRotationXY( float rotation ); // @TODO: Support Y up
  Vec3 RotateCopy( Vec3 axis, float angle ) const;
  Vec3 Lerp( const Vec3& end, float t ) const;
  Vec3 Slerp( const Vec3& end, float t, float epsilon = 0.0001f ) const;
  
  static Vec3 Cross( const Vec3& v0, const Vec3& v1 );
  Vec3 Cross( const Vec3& v ) const;
  void ZeroAxis( Vec3 axis ); // Zero component along arbitrary axis (ie vec dot axis == 0)
  void ZeroDirection( Vec3 direction ); // Zero component along positive half of axis (ie vec dot dir > 0)

  // static Vec3 ProjectPoint( const class Matrix4& projection, Vec3 p ); // @TODO
  // static Vec3 ProjectVector( const class Matrix4& projection, Vec3 p ); // @TODO
  
  union
  {
    struct
    {
      float x;
      float y;
      float z;
    };
    float data[ 3 ];
  };
  float pad;
};

//------------------------------------------------------------------------------
// ae::Vec4 struct
//------------------------------------------------------------------------------
struct Vec4 : public _VecT< Vec4 >
{
  Vec4() {} // Empty default constructor for performance of vertex arrays etc
  Vec4( const Vec4& ) = default;
  explicit Vec4( float f );
  explicit Vec4( float* v );
  explicit Vec4( float xyz, float w );
  Vec4( float x, float y, float z, float w );
  Vec4( Vec3 xyz, float w );
  Vec4( Vec2 xy, float z, float w );
  Vec4( Vec2 xy, Vec2 zw );
  explicit operator Vec2() const;
  explicit operator Vec3() const;
  Vec4( const float* v3, float w );
  explicit Vec4( const float* v4 );
  Vec2 GetXY() const;
  Vec2 GetZW() const;
  Vec3 GetXYZ() const;
  union
  {
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
    float data[ 4 ];
  };
};

//------------------------------------------------------------------------------
// ae::Color struct
//------------------------------------------------------------------------------
struct Color
{
  Color() {} // Empty default constructor for performance of vertex arrays etc
  Color( const Color& ) = default;
  Color( float rgb );
  Color( float r, float g, float b );
  Color( float r, float g, float b, float a );
  Color( Color c, float a );
  static Color R( float r );
  static Color RG( float r, float g );
  static Color RGB( float r, float g, float b );
  static Color RGBA( float r, float g, float b, float a );
  static Color RGBA( const float* v );
  static Color SRGB( float r, float g, float b );
  static Color SRGBA( float r, float g, float b, float a );
  static Color R8( uint8_t r );
  static Color RG8( uint8_t r, uint8_t g );
  static Color RGB8( uint8_t r, uint8_t g, uint8_t b );
  static Color RGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
  static Color SRGB8( uint8_t r, uint8_t g, uint8_t b );
  static Color SRGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a );

  Vec3 GetLinearRGB() const;
  Vec4 GetLinearRGBA() const;
  Vec3 GetSRGB() const;
  Vec4 GetSRGBA() const;

  Color Lerp( const Color& end, float t ) const;
  Color DtLerp( float snappiness, float dt, const Color& target ) const;
  Color ScaleRGB( float s ) const;
  Color ScaleA( float s ) const;
  Color SetA( float alpha ) const;

  static float SRGBToRGB( float x );
  static float RGBToSRGB( float x );

  // Grayscale
  static Color White();
  static Color Gray();
  static Color Black();
  // Rainbow
  static Color Red();
  static Color Orange();
  static Color Yellow();
  static Color Green();
  static Color Blue();
  static Color Indigo();
  static Color Violet();
  // Pico
  static Color PicoBlack();
  static Color PicoDarkBlue();
  static Color PicoDarkPurple();
  static Color PicoDarkGreen();
  static Color PicoBrown();
  static Color PicoDarkGray();
  static Color PicoLightGray();
  static Color PicoWhite();
  static Color PicoRed();
  static Color PicoOrange();
  static Color PicoYellow();
  static Color PicoGreen();
  static Color PicoBlue();
  static Color PicoIndigo();
  static Color PicoPink();
  static Color PicoPeach();
  // Misc
  static Color Magenta();

  float r;
  float g;
  float b;
  float a;

private:
  // Delete implicit conversions to try to catch color space issues
  template < typename T > Color R( T r ) = delete;
  template < typename T > Color RG( T r, T g ) = delete;
  template < typename T > Color RGB( T r, T g, T b ) = delete;
  template < typename T > Color RGBA( T r, T g, T b, T a ) = delete;
  template < typename T > Color RGBA( const T* v ) = delete;
  template < typename T > Color SRGB( T r, T g, T b ) = delete;
  template < typename T > Color SRGBA( T r, T g, T b, T a ) = delete;
};

#pragma warning(default:26495) // Re-enable uninitialized variable warning

//------------------------------------------------------------------------------
// Random values
//------------------------------------------------------------------------------
inline int32_t Random( int32_t min, int32_t max );
inline float Random( float min, float max );
inline bool RandomBool();

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
  T m_min = T();
  T m_max = T();
};

//------------------------------------------------------------------------------
// ae::TimeStep
//------------------------------------------------------------------------------
class TimeStep
{
public:
  TimeStep();

  void SetTimeStep( float timeStep );
  float GetTimeStep() const;
  uint32_t GetStepCount() const;

  float GetDt() const;
  void SetDt( float sec ); // Useful for handling frames with high delta time, eg: timeStep.SetDt( timeStep.GetTimeStep() )

  void Wait();

private:
  uint32_t m_stepCount = 0;
  float m_timeStepSec = 0.0f;
  float m_timeStep = 0.0f;
  int64_t m_frameExcess = 0;
  float m_prevFrameTime = 0.0f;
  float m_prevFrameTimeSec = 0.0f;
  std::chrono::steady_clock::time_point m_frameStart;
};

//------------------------------------------------------------------------------
// ae::Str class
// @NOTE: A fixed length string class. The templated value is the total size of
// the string in memory.
//------------------------------------------------------------------------------
template < uint32_t N >
class Str
{
public:
  Str();
  template < uint32_t N2 > Str( const Str<N2>& str );
  Str( const char* str );
  Str( uint32_t length, const char* str );
  Str( uint32_t length, char c );
  template < typename... Args > Str( const char* format, Args... args );
  template < typename... Args > static Str< N > Format( const char* format, Args... args );
  explicit operator const char*() const;
  
  template < uint32_t N2 > void operator =( const Str<N2>& str );
  template < uint32_t N2 > Str<N> operator +( const Str<N2>& str ) const;
  template < uint32_t N2 > void operator +=( const Str<N2>& str );
  template < uint32_t N2 > bool operator ==( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator !=( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator <( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator >( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator <=( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator >=( const Str<N2>& str ) const;
  Str<N> operator +( const char* str ) const;
  void operator +=( const char* str );
  bool operator ==( const char* str ) const;
  bool operator !=( const char* str ) const;
  bool operator <( const char* str ) const;
  bool operator >( const char* str ) const;
  bool operator <=( const char* str ) const;
  bool operator >=( const char* str ) const;

  char& operator[]( uint32_t i );
  const char operator[]( uint32_t i ) const;
  const char* c_str() const;

  template < uint32_t N2 >
  void Append( const Str<N2>& str );
  void Append( const char* str );
  void Trim( uint32_t len );

  uint32_t Length() const;
  uint32_t Size() const;
  bool Empty() const;
  static constexpr uint32_t MaxLength() { return N - 3u; } // Leave room for length var and null terminator

private:
  template < uint32_t N2 > friend class Str;
  template < uint32_t N2 > friend bool operator ==( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator !=( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator <( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator >( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator <=( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator >=( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend std::istream& operator>>( std::istream&, Str< N2 >& );
  void m_Format( const char* format );
  template < typename T, typename... Args >
  void m_Format( const char* format, T value, Args... args );
  uint16_t m_length;
  char m_str[ MaxLength() + 1u ];
};
// Predefined lengths
using Str16 = Str< 16 >;
using Str32 = Str< 32 >;
using Str64 = Str< 64 >;
using Str128 = Str< 128 >;
using Str256 = Str< 256 >;
using Str512 = Str< 512 >;

//------------------------------------------------------------------------------
// ae::Array class
//------------------------------------------------------------------------------
template < typename T, uint32_t N = 0 >
class Array
{
public:
  // Static array (N > 0)
  Array();
  Array( uint32_t length, const T& val ); // Appends 'length' number of 'val's
  // Dynamic array (N == 0)
  Array( ae::Tag tag );
  Array( ae::Tag tag, uint32_t size ); // Reserve size (with length of 0)
  Array( ae::Tag tag, uint32_t length, const T& val ); // Reserves 'length' and appends 'length' number of 'val's
  void Reserve( uint32_t total );
  // Static and dynamic arrays
  Array( const Array< T, N >& other );
  Array( Array< T, N >&& other ) noexcept; // Move operators fallback to regular operators if ae::Tags don't match
  void operator =( const Array< T, N >& other );
  void operator =( Array< T, N >&& other ) noexcept;
  ~Array();
  
  // Add elements
  T& Append( const T& value );
  void Append( const T* values, uint32_t count );
  T& Insert( uint32_t index, const T& value );

  // Find elements
  template < typename U > int32_t Find( const U& value ) const; // Returns -1 when not found
  template < typename Fn > int32_t FindFn( Fn testFn ) const; // Returns -1 when not found

  // Remove elements
  template < typename U > uint32_t RemoveAll( const U& value );
  template < typename Fn > uint32_t RemoveAllFn( Fn testFn );
  void Remove( uint32_t index );
  void Clear();

  // Access elements
  const T& operator[]( int32_t index ) const; // Performs bounds checking in debug mode. Use 'Begin()' to get raw array.
  T& operator[]( int32_t index );
  T* Begin() { return m_array; } // These functions can return null when array length is zero
  T* End() { return m_array + m_length; }
  const T* Begin() const { return m_array; }
  const T* End() const { return m_array + m_length; }

  // Array info
  uint32_t Length() const;
  uint32_t Size() const;
  
private:
  uint32_t m_GetNextSize() const;
  uint32_t m_length;
  uint32_t m_size;
  T* m_array;
  ae::Tag m_tag;
  typedef typename std::aligned_storage< sizeof(T), alignof(T) >::type AlignedStorageT;
#if _AE_LINUX_
  struct Storage { AlignedStorageT data[ N ]; };
  Storage m_storage;
#else
  template < uint32_t > struct Storage { AlignedStorageT data[ N ]; };
  template <> struct Storage< 0 > {};
  Storage< N > m_storage;
#endif
public:
  // @NOTE: Ranged-based loop. Lowercase to match c++ standard ('-.-)
  T* begin() { return m_array; }
  T* end() { return m_array + m_length; }
  const T* begin() const { return m_array; }
  const T* end() const { return m_array + m_length; }
};

//------------------------------------------------------------------------------
// ae::Map class
//------------------------------------------------------------------------------
template < typename K, typename V, uint32_t N = 0 >
class Map
{
public:
  Map(); // Static map only (N > 0)
  Map( ae::Tag pool ); // Dynamic map only (N == 0)
  
  V& Set( const K& key, const V& value );
  V& Get( const K& key );
  const V& Get( const K& key ) const;
  const V& Get( const K& key, const V& defaultValue ) const;
  
  V* TryGet( const K& key );
  const V* TryGet( const K& key ) const;

  bool TryGet( const K& key, V* valueOut );
  bool TryGet( const K& key, V* valueOut ) const;
  
  bool Remove( const K& key );
  bool Remove( const K& key, V* valueOut );

  void Reserve( uint32_t total );
  void Clear();

  const K& GetKey( uint32_t index ) const;
  const V& GetValue( uint32_t index ) const;
  V& GetValue( uint32_t index );
  uint32_t Length() const;

private:
  template < typename K2, typename V2, uint32_t N2 >
  friend std::ostream& operator<<( std::ostream&, const Map< K2, V2, N2 >& );

  struct Entry
  {
    Entry() = default;
    Entry( const K& k, const V& v );

    K key;
    V value;
  };

  int32_t m_FindIndex( const K& key ) const;

  Array< Entry, N > m_entries;
};

//------------------------------------------------------------------------------
// ae::Rect class
//------------------------------------------------------------------------------
struct Rect
{
  Rect() = default;
  Rect( const Rect& ) = default;
  Rect( float x, float y, float w, float h ) : x(x), y(y), w(w), h(h) {}
  Rect( Vec2 p0, Vec2 p1 );
  explicit operator Vec4() const { return Vec4( x, y, w, h ); }
  
  Vec2 GetMin() const { return Vec2( x, y ); }
  Vec2 GetMax() const { return Vec2( x + w, y + h ); }
  Vec2 GetSize() const { return Vec2( w, h ); }
  bool Contains( Vec2 pos ) const;
  void Expand( Vec2 pos ); // @NOTE: Zero size rect is maintained by Expand()
  bool GetIntersection( const Rect& other, Rect* intersectionOut ) const;
  
  float x, y, w, h;
};
inline std::ostream& operator<<( std::ostream& os, Rect r )
{
  return os << r.x << " " << r.y << " " << r.w << " " << r.h;
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

namespace AE_NAMESPACE {

// @HACK:
struct Matrix4
{
  float data[ 16 ];
};

//------------------------------------------------------------------------------
// ae::Window class
//------------------------------------------------------------------------------
class Window
{
public:
  Window();
  bool Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor );
  bool Initialize( Int2 pos, uint32_t width, uint32_t height, bool showCursor );
  void Terminate();

  void SetTitle( const char* title );
  void SetFullScreen( bool fullScreen );
  void SetPosition( Int2 pos );
  void SetSize( uint32_t width, uint32_t height );
  void SetMaximized( bool maximized );

  const char* GetTitle() const { return m_windowTitle.c_str(); }
  Int2 GetPosition() const { return m_pos; }
  int32_t GetWidth() const { return m_width; }
  int32_t GetHeight() const { return m_height; }
  bool GetFullScreen() const { return m_fullScreen; }
  bool GetMaximized() const { return m_maximized; }

private:
  void m_Initialize();
    
  Int2 m_pos;
  int32_t m_width;
  int32_t m_height;
  bool m_fullScreen;
  bool m_maximized;
  Str256 m_windowTitle;

public:
  // Internal
  void m_UpdatePos( Int2 pos ) { m_pos = pos; }
  void m_UpdateWidthHeight( int32_t width, int32_t height ) { m_width = width; m_height = height; }
  void m_UpdateMaximized( bool maximized ) { m_maximized = maximized; }

  void* window;
  class GraphicsDevice* graphicsDevice;
};

//------------------------------------------------------------------------------
// ae::Input class
//------------------------------------------------------------------------------
class Input
{
public:
  void Pump();
  bool quit = false;
};

//------------------------------------------------------------------------------
// ae::UniformList class
//------------------------------------------------------------------------------
class UniformList
{
public:
  struct Value
  {
    uint32_t sampler = 0;
    uint32_t target = 0;
    int32_t size = 0;
    Matrix4 value;
  };

  void Set( const char* name, float value );
  void Set( const char* name, Vec2 value );
  void Set( const char* name, Vec3 value );
  void Set( const char* name, Vec4 value );
  void Set( const char* name, const Matrix4& value );
  void Set( const char* name, const class Texture* tex );

  const Value* Get( const char* name ) const;

private:
  ae::Map< Str32, Value > m_uniforms = AE_ALLOC_TAG_RENDER;
};

//------------------------------------------------------------------------------
// ae::Shader class
//------------------------------------------------------------------------------
const uint32_t _kMaxShaderAttributeCount = 16;
const uint32_t _kMaxShaderAttributeNameLength = 16;
const uint32_t _kMaxShaderDefines = 4;

class Shader
{
public:
  enum class Type
  {
    Vertex,
    Fragment
  };
  enum class Culling
  {
    None,
    ClockwiseFront,
    CounterclockwiseFront,
  };
  struct Attribute
  {
    char name[ _kMaxShaderAttributeNameLength ];
    uint32_t type; // GL_FLOAT, GL_FLOAT_VEC4, GL_FLOAT_MAT4...
    int32_t location;
  };
  struct Uniform
  {
    Str32 name;
    uint32_t type;
    int32_t location;
  };
  
  // Interface
  Shader();
  ~Shader();
  void Initialize( const char* vertexStr, const char* fragStr, const char* const* defines, int32_t defineCount );
  void Destroy();
  void SetBlending( bool enabled ) { m_blending = enabled; }
  void SetDepthTest( bool enabled ) { m_depthTest = enabled; }
  void SetDepthWrite( bool enabled ) { m_depthWrite = enabled; }
  void SetCulling( Culling culling ) { m_culling = culling; }
  void SetWireframe( bool enabled ) { m_wireframe = enabled; }
  void SetBlendingPremul( bool enabled ) { m_blendingPremul = enabled; }

  // Internal
private:
  int m_LoadShader( const char* shaderStr, Type type, const char* const* defines, int32_t defineCount );
  uint32_t m_fragmentShader;
  uint32_t m_vertexShader;
  uint32_t m_program;
  bool m_blending;
  bool m_blendingPremul;
  bool m_depthTest;
  bool m_depthWrite;
  Culling m_culling;
  bool m_wireframe;
  ae::Array< Attribute, _kMaxShaderAttributeCount > m_attributes;
  ae::Map< Str32, Uniform > m_uniforms = AE_ALLOC_TAG_RENDER;
public:
  void Activate( const UniformList& uniforms ) const;
  const Attribute* GetAttributeByIndex( uint32_t index ) const;
  uint32_t GetAttributeCount() const { return m_attributes.Length(); }
};

//------------------------------------------------------------------------------
// ae::VertexData class
//------------------------------------------------------------------------------
class VertexData
{
public:
};

//------------------------------------------------------------------------------
// ae::Texture class
//------------------------------------------------------------------------------
class Texture
{
public:
  // Constants
  enum class Filter
  {
    Linear,
    Nearest
  };
  enum class Wrap
  {
    Repeat,
    Clamp
  };
  enum class Format
  {
    Depth32F,
    R8, // unorm
    R16_UNORM, // for height fields
    R16F,
    R32F,
    RG8, // unorm
    RG16F,
    RG32F,
    RGB8, // unorm
    RGB8_SRGB,
    RGB16F,
    RGB32F,
    RGBA8, // unorm
    RGBA8_SRGB,
    RGBA16F,
    RGBA32F,
    // non-specific formats, prefer specific types above
    R = RGBA8,
    RG = RG8,
    RGB = RGB8,
    RGBA = RGBA8,
    Depth = Depth32F,
    SRGB = RGB8_SRGB,
    SRGBA = RGBA8_SRGB,
  };
  enum class Type
  {
    Uint8,
    Uint16,
    HalfFloat,
    Float
  };

  // Interface
  Texture() = default;
  virtual ~Texture();
  void Initialize( uint32_t target );
  virtual void Destroy();
  uint32_t GetTexture() const { return m_texture; }
  uint32_t GetTarget() const { return m_target; }

private:
  Texture( const Texture& ) = delete;
  Texture( Texture&& ) = delete;
  void operator=( const Texture& ) = delete;
  void operator=( Texture&& ) = delete;
  uint32_t m_texture = 0;
  uint32_t m_target = 0;
};

//------------------------------------------------------------------------------
// ae::Texture2D class
//------------------------------------------------------------------------------
class Texture2D : public Texture
{
public:
  void Initialize( const void* data, uint32_t width, uint32_t height, Format format, Type type, Filter filter, Wrap wrap, bool autoGenerateMipmaps = false );
  void Initialize( const char* file, Filter filter, Wrap wrap, bool autoGenerateMipmaps = false,
    bool isSRGB = false );
  void Destroy() override;

  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }

private:
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  bool m_hasAlpha = false;
};

//------------------------------------------------------------------------------
// ae::RenderTarget class
//------------------------------------------------------------------------------
class RenderTarget
{
public:
  ~RenderTarget();
  void Initialize( uint32_t width, uint32_t height );
  void AddTexture( Texture::Filter filter, Texture::Wrap wrap );
  void AddDepth( Texture::Filter filter, Texture::Wrap wrap );
  void Destroy();

  void Activate();
  void Clear( Color color );
  void Render( const Shader* shader, const UniformList& uniforms );
  void Render2D( uint32_t textureIndex, Rect ndc, float z );

  const Texture2D* GetTexture( uint32_t index ) const;
  const Texture2D* GetDepth() const;
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

  // @NOTE: Get ndc space rect of this target within another target (fill but maintain aspect ratio)
  // GetNDCFillRectForTarget( GraphicsDevice::GetWindow()::GetWidth(),  GraphicsDevice::GetWindow()::Height() )
  // GetNDCFillRectForTarget( GraphicsDeviceTarget()::GetWidth(),  GraphicsDeviceTarget()::Height() )
  Rect GetNDCFillRectForTarget( uint32_t otherWidth, uint32_t otherHeight ) const;

  // @NOTE: Other target to local transform (pixels->pixels)
  // Useful for transforming window/mouse pixel coordinates to local pixels
  // GetTargetPixelsToLocalTransform( GraphicsDevice::GetWindow()::GetWidth(),  GraphicsDevice::GetWindow()::Height(), GetNDCFillRectForTarget( ... ) )
  Matrix4 GetTargetPixelsToLocalTransform( uint32_t otherPixelWidth, uint32_t otherPixelHeight, Rect ndc ) const;

  // @NOTE: Mouse/window pixel coordinates to world space
  // GetTargetPixelsToWorld( GetTargetPixelsToLocalTransform( ... ), TODO )
  Matrix4 GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const;

  // @NOTE: Creates a transform matrix from aeQuad vertex positions to ndc space
  // GraphicsDeviceTarget uses aeQuad vertices internally
  static Matrix4 GetQuadToNDCTransform( Rect ndc, float z );

private:
  struct Vertex
  {
    Vec3 pos;
    Vec2 uv;
  };
  uint32_t m_fbo = 0;
  Array< Texture2D*, 4 > m_targets;
  Texture2D m_depth;
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  VertexData m_quad;
  Shader m_shader;
};

//------------------------------------------------------------------------------
// ae::GraphicsDevice class
//------------------------------------------------------------------------------
class GraphicsDevice
{
public:
  GraphicsDevice();
  ~GraphicsDevice();

  void Initialize( class Window* window );
  void Terminate();

  void Activate();
  void Clear( Color color );
  void Present();

  class Window* GetWindow() { return m_window; }
  RenderTarget* GetCanvas() { return &m_canvas; }

  uint32_t GetWidth() const { return m_canvas.GetWidth(); }
  uint32_t GetHeight() const { return m_canvas.GetHeight(); }
  float GetAspectRatio() const;

  // have to inject a barrier to readback from active render target (GL only)
  void AddTextureBarrier();

//private:
  friend class ae::Window;
  void m_HandleResize( uint32_t width, uint32_t height );

  Window* m_window;
  RenderTarget m_canvas;

  // OpenGL
  void* m_context;
  int32_t m_defaultFbo;
};

} // AE_NAMESPACE end

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
// Platform internal implementation
//------------------------------------------------------------------------------
namespace AE_NAMESPACE {

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
// C++ style allocation functions
//------------------------------------------------------------------------------
const uint32_t _kDefaultAlignment = 16;
const uint32_t _kHeaderSize = 16;
struct _Header
{
  uint32_t check;
  uint32_t count;
  uint32_t size;
  uint32_t typeSize;
};

template < typename T >
T* NewArray( ae::Tag tag, uint32_t count )
{
  AE_STATIC_ASSERT( alignof( T ) <= _kDefaultAlignment );
  AE_STATIC_ASSERT( sizeof( T ) % alignof( T ) == 0 ); // All elements in array should have correct alignment

  uint32_t size = _kHeaderSize + sizeof( T ) * count;
  uint8_t* base = (uint8_t*)ae::Allocate( tag, size, _kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % _kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  AE_STATIC_ASSERT( sizeof( _Header ) <= _kHeaderSize );
  AE_STATIC_ASSERT( _kHeaderSize % _kDefaultAlignment == 0 );

  _Header* header = (_Header*)base;
  header->check = 0xABCD;
  header->count = count;
  header->size = size;
  header->typeSize = sizeof( T );

  T* result = (T*)( base + _kHeaderSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    new( &result[ i ] ) T();
  }

  return result;
}

template < typename T, typename ... Args >
T* New( ae::Tag tag, Args ... args )
{
  AE_STATIC_ASSERT( alignof( T ) <= _kDefaultAlignment );

  uint32_t size = _kHeaderSize + sizeof( T );
  uint8_t* base = (uint8_t*)ae::Allocate( tag, size, _kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % _kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  _Header* header = (_Header*)base;
  header->check = 0xABCD;
  header->count = 1;
  header->size = size;
  header->typeSize = sizeof( T );

  return new( (T*)( base + _kHeaderSize ) ) T( args ... );
}

template < typename T >
void Delete( T* obj )
{
  if ( !obj )
  {
    return;
  }

  AE_ASSERT( (intptr_t)obj % _kDefaultAlignment == 0 );
  uint8_t* base = (uint8_t*)obj - _kHeaderSize;

  _Header* header = (_Header*)( base );
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

  ae::Free( base );
}

//------------------------------------------------------------------------------
// C style allocations
//------------------------------------------------------------------------------
inline void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment )
{
  return ae::GetGlobalAllocator()->Allocate( tag, bytes, alignment );
}

inline void* Reallocate( void* data, uint32_t bytes, uint32_t alignment )
{
  return ae::GetGlobalAllocator()->Reallocate( data, bytes, alignment );
}

inline void Free( void* data )
{
  ae::GetGlobalAllocator()->Free( data );
}

//------------------------------------------------------------------------------
// ae::Scratch< T > member functions
//------------------------------------------------------------------------------
template < typename T >
Scratch< T >::Scratch( uint32_t count )
{
  m_count = count;
  m_data = ae::NewArray< T >( AE_ALLOC_TAG_SCRATCH, count );
}

template < typename T >
Scratch< T >::~Scratch()
{
  ae::Delete( m_data );
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
// Math function implementations
//------------------------------------------------------------------------------
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
    // @TODO: Needed for Color, support types without lerp
    return start.Lerp( end, t ); //return start + ( ( end - start ) * t );
  }
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

//------------------------------------------------------------------------------
// RandomValue member functions
//------------------------------------------------------------------------------
template < typename T >
inline ae::RandomValue< T >::RandomValue( T min, T max ) : m_min(min), m_max(max) {}

template < typename T >
inline ae::RandomValue< T >::RandomValue( T value ) : m_min(value), m_max(value) {}

template < typename T >
inline void ae::RandomValue< T >::SetMin( T min )
{
  m_min = min;
}

template < typename T >
inline void ae::RandomValue< T >::SetMax( T max )
{
  m_max = max;
}

template < typename T >
inline T ae::RandomValue< T >::GetMin() const
{
  return m_min;
}

template < typename T >
inline T ae::RandomValue< T >::GetMax() const
{
  return m_max;
}

template < typename T >
inline T ae::RandomValue< T >::Get() const
{
  return Random( m_min, m_max );
}

template < typename T >
inline ae::RandomValue< T >::operator T() const
{
  return Get();
}

//------------------------------------------------------------------------------
// ae::Vec2 shared member functions
// ae::Vec3 shared member functions
// ae::Vec4 shared member functions
//------------------------------------------------------------------------------
template < typename T >
bool _VecT< T >::operator==( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) == 0;
}

template < typename T >
bool _VecT< T >::operator!=( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) != 0;
}

template < typename T >
float _VecT< T >::operator[]( uint32_t idx ) const
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
float& _VecT< T >::operator[]( uint32_t idx )
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
T _VecT< T >::operator+( const T& v ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(T::data); i++ )
  {
    result.data[ i ] = self.data[ i ] + v.data[ i ];
  }
  return result;
}

template < typename T >
void _VecT< T >::operator+=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(T::data); i++ )
  {
    self.data[ i ] += v.data[ i ];
  }
}

template < typename T >
T _VecT< T >::operator-() const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = -self.data[ i ];
  }
  return result;
}

template < typename T >
T _VecT< T >::operator-( const T& v ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] - v.data[ i ];
  }
  return result;
}

template < typename T >
void _VecT< T >::operator-=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] -= v.data[ i ];
  }
}

template < typename T >
T _VecT< T >::operator*( const T& v ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] * v.data[ i ];
  }
  return result;
}

template < typename T >
T _VecT< T >::operator/( const T& v ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] / v.data[ i ];
  }
  return result;
}

template < typename T >
void _VecT< T >::operator*=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] *= v.data[ i ];
  }
}

template < typename T >
void _VecT< T >::operator/=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] /= v.data[ i ];
  }
}

template < typename T >
T _VecT< T >::operator*( float s ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] * s;
  }
  return result;
}

template < typename T >
void _VecT< T >::operator*=( float s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] *= s;
  }
}

template < typename T >
T _VecT< T >::operator/( float s ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] / s;
  }
  return result;
}

template < typename T >
void _VecT< T >::operator/=( float s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] /= s;
  }
}

template < typename T >
float _VecT< T >::Dot( const T& v0, const T& v1 )
{
  float result = 0.0f;
  for ( uint32_t i = 0; i < countof(v0.data); i++ )
  {
    result += v0.data[ i ] * v1.data[ i ];
  }
  return result;
}

template < typename T >
float _VecT< T >::Dot( const T& v ) const
{
  return Dot( *(T*)this, v );
}

template < typename T >
float _VecT< T >::Length() const
{
  return sqrt( LengthSquared() );
}

template < typename T >
float _VecT< T >::LengthSquared() const
{
  return Dot( *(T*)this );
}

template < typename T >
float _VecT< T >::Normalize()
{
  float length = Length();
  *(T*)this /= length;
  return length;
}

template < typename T >
float _VecT< T >::SafeNormalize( float epsilon )
{
  auto&& self = *(T*)this;
  float length = Length();
  if ( length < epsilon )
  {
    self = T( 0.0f );
    return 0.0f;
  }
  self /= length;
  return length;
}

template < typename T >
T _VecT< T >::NormalizeCopy() const
{
  T result = *(T*)this;
  result.Normalize();
  return result;
}

template < typename T >
T _VecT< T >::SafeNormalizeCopy( float epsilon ) const
{
  T result = *(T*)this;
  result.SafeNormalize( epsilon );
  return result;
}

template < typename T >
float _VecT< T >::Trim( float trimLength )
{
  float length = Length();
  if ( trimLength < length )
  {
    *(T*)this *= ( trimLength / length );
    return trimLength;
  }
  return length;
}

template < typename T >
inline std::ostream& operator<<( std::ostream& os, const _VecT< T >& v )
{
  constexpr uint32_t count = countof( T::data );
  for ( uint32_t i = 0; i < count - 1; i++ )
  {
    os << v[ i ] << " ";
  }
  return os << v[ count - 1 ];
}

#pragma warning(disable:26495) // Hide incorrect Vec2 initialization warning due to union

//------------------------------------------------------------------------------
// ae::Vec2 member functions
//------------------------------------------------------------------------------
inline Vec2::Vec2( float v ) : x( v ), y( v ) {}
inline Vec2::Vec2( float x, float y ) : x( x ), y( y ) {}
inline Vec2::Vec2( const float* v2 ) : x( v2[ 0 ] ), y( v2[ 1 ] ) {}
inline Vec2 Vec2::FromAngle( float angle ) { return Vec2( ae::Cos( angle ), ae::Sin( angle ) ); }

//------------------------------------------------------------------------------
// ae::Vec3 member functions
//------------------------------------------------------------------------------
inline Vec3::Vec3( float v ) : x( v ), y( v ), z( v ), pad( 0.0f ) {}
inline Vec3::Vec3( float x, float y, float z ) : x( x ), y( y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( const float* v3 ) : x( v3[ 0 ] ), y( v3[ 1 ] ), z( v3[ 2 ] ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy, float z ) : x( xy.x ), y( xy.y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy ) : x( xy.x ), y( xy.y ), z( 0.0f ), pad( 0.0f ) {}
inline Vec3::operator Vec2() const { return Vec2( x, y ); }
inline Vec2 Vec3::GetXY() const { return Vec2( x, y ); }
inline Vec2 Vec3::GetXZ() const { return Vec2( x, z ); }
inline Vec3 Vec3::Lerp( const Vec3& end, float t ) const
{
  t = ae::Clip01( t );
  float minT = ( 1.0f - t );
  return Vec3( x * minT + end.x * t, y * minT + end.y * t, z * minT + end.z * t );
}
inline Vec3 Vec3::Cross( const Vec3& v0, const Vec3& v1 )
{
  return Vec3( v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x );
}
inline Vec3 Vec3::Cross( const Vec3& v ) const { return Cross( *this, v ); }
inline void Vec3::ZeroAxis( Vec3 axis )
{
  axis.SafeNormalize();
  *this -= axis * Dot( axis );
}
inline void Vec3::ZeroDirection( Vec3 direction )
{
  float d = Dot( direction );
  if ( d > 0.0f )
  {
    direction.SafeNormalize();
    *this -= direction * d;
  }
}

//------------------------------------------------------------------------------
// ae::Vec4 member functions
//------------------------------------------------------------------------------
inline Vec4::Vec4( float f ) : x( f ), y( f ), z( f ), w( f ) {}
inline Vec4::Vec4( float xyz, float w ) : x( xyz ), y( xyz ), z( xyz ), w( w ) {}
inline Vec4::Vec4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}
inline Vec4::Vec4( Vec3 xyz, float w ) : x( xyz.x ), y( xyz.y ), z( xyz.z ), w( w ) {}
inline Vec4::Vec4( Vec2 xy, float z, float w ) : x( xy.x ), y( xy.y ), z( z ), w( w ) {}
inline Vec4::Vec4( Vec2 xy, Vec2 zw ) : x( xy.x ), y( xy.y ), z( zw.x ), w( zw.y ) {}
inline Vec4::operator Vec2() const { return Vec2( x, y ); }
inline Vec4::operator Vec3() const { return Vec3( x, y, z ); }
inline Vec4::Vec4( const float* v3, float w ) : x( v3[ 0 ] ), y( v3[ 1 ] ), z( v3[ 2 ] ), w( w ) {}
inline Vec4::Vec4( const float* v4 ) : x( v4[ 0 ] ), y( v4[ 1 ] ), z( v4[ 2 ] ), w( v4[ 3 ] ) {}
inline Vec2 Vec4::GetXY() const { return Vec2( x, y ); }
inline Vec2 Vec4::GetZW() const { return Vec2( z, w ); }
inline Vec3 Vec4::GetXYZ() const { return Vec3( x, y, z ); }

//------------------------------------------------------------------------------
// ae::Colors
// It's expensive to do the srgb conversion everytime these are constructed so
// do it once and then return a copy each time static Color functions are called.
//------------------------------------------------------------------------------
// Grayscale
inline Color Color::White() { static Color c = Color::SRGB8( 255, 255, 255 ); return c; }
inline Color Color::Gray() { static Color c = Color::SRGB8( 127, 127, 127 ); return c; }
inline Color Color::Black() { static Color c = Color::SRGB8( 0, 0, 0 ); return c; }
// Rainbow
inline Color Color::Red() { static Color c = Color::SRGB8( 255, 0, 0 ); return c; }
inline Color Color::Orange() { static Color c = Color::SRGB8( 255, 127, 0 ); return c; }
inline Color Color::Yellow() { static Color c = Color::SRGB8( 255, 255, 0 ); return c; }
inline Color Color::Green() { static Color c = Color::SRGB8( 0, 255, 0 ); return c; }
inline Color Color::Blue() { static Color c = Color::SRGB8( 0, 0, 255 ); return c; }
inline Color Color::Indigo() { static Color c = Color::SRGB8( 75, 0, 130 ); return c; }
inline Color Color::Violet() { static Color c = Color::SRGB8( 148, 0, 211 ); return c; }
// Pico
inline Color Color::PicoBlack() { static Color c = Color::SRGB8( 0, 0, 0 ); return c; }
inline Color Color::PicoDarkBlue() { static Color c = Color::SRGB8( 29, 43, 83 ); return c; }
inline Color Color::PicoDarkPurple() { static Color c = Color::SRGB8( 126, 37, 83 ); return c; }
inline Color Color::PicoDarkGreen() { static Color c = Color::SRGB8( 0, 135, 81 ); return c; }
inline Color Color::PicoBrown() { static Color c = Color::SRGB8( 171, 82, 54 ); return c; }
inline Color Color::PicoDarkGray() { static Color c = Color::SRGB8( 95, 87, 79 ); return c; }
inline Color Color::PicoLightGray() { static Color c = Color::SRGB8( 194, 195, 199 ); return c; }
inline Color Color::PicoWhite() { static Color c = Color::SRGB8( 255, 241, 232 ); return c; }
inline Color Color::PicoRed() { static Color c = Color::SRGB8( 255, 0, 77 ); return c; }
inline Color Color::PicoOrange() { static Color c = Color::SRGB8( 255, 163, 0 ); return c; }
inline Color Color::PicoYellow() { static Color c = Color::SRGB8( 255, 236, 39 ); return c; }
inline Color Color::PicoGreen() { static Color c = Color::SRGB8( 0, 228, 54 ); return c; }
inline Color Color::PicoBlue() { static Color c = Color::SRGB8( 41, 173, 255 ); return c; }
inline Color Color::PicoIndigo() { static Color c = Color::SRGB8( 131, 118, 156 ); return c; }
inline Color Color::PicoPink() { static Color c = Color::SRGB8( 255, 119, 168 ); return c; }
inline Color Color::PicoPeach() { static Color c = Color::SRGB8( 255, 204, 170 ); return c; }
// Misc
inline Color Color::Magenta() { return Color( 1.0f, 0.0f, 1.0f ); }

//------------------------------------------------------------------------------
// ae::Color functions
//------------------------------------------------------------------------------
inline std::ostream& operator<<( std::ostream& os, Color c )
{
  return os << "<" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ">";
}
inline Color::Color( float rgb ) : r( rgb ), g( rgb ), b( rgb ), a( 1.0f ) {}
inline Color::Color( float r, float g, float b ) : r( r ), g( g ), b( b ), a( 1.0f ) {}
inline Color::Color( float r, float g, float b, float a )
  : r( r ), g( g ), b( b ), a( a )
{}
inline Color::Color( Color c, float a ) : r( c.r ), g( c.g ), b( c.b ), a( a ) {}
inline Color Color::R( float r ) { return Color( r, 0.0f, 0.0f, 1.0f ); }
inline Color Color::RG( float r, float g ) { return Color( r, g, 0.0f, 1.0f ); }
inline Color Color::RGB( float r, float g, float b ) { return Color( r, g, b, 1.0f ); }
inline Color Color::RGBA( float r, float g, float b, float a ) { return Color( r, g, b, a ); }
inline Color Color::RGBA( const float* v ) { return Color( v[ 0 ], v[ 1 ], v[ 2 ], v[ 3 ] ); }
inline Color Color::SRGB( float r, float g, float b ) { return Color( SRGBToRGB( r ), SRGBToRGB( g ), SRGBToRGB( b ), 1.0f ); }
inline Color Color::SRGBA( float r, float g, float b, float a ) { return Color( SRGBToRGB( r ), SRGBToRGB( g ), SRGBToRGB( b ), a ); }
inline Color Color::R8( uint8_t r ) { return Color( r / 255.0f, 0.0f, 0.0f, 1.0f ); }
inline Color Color::RG8( uint8_t r, uint8_t g ) { return Color( r / 255.0f, g / 255.0f, 0.0f, 1.0f ); }
inline Color Color::RGB8( uint8_t r, uint8_t g, uint8_t b ) { return Color( r / 255.0f, g / 255.0f, b / 255.0f, 1.0f ); }
inline Color Color::RGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
  return Color( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );
}
inline Color Color::SRGB8( uint8_t r, uint8_t g, uint8_t b )
{
  return Color( SRGBToRGB( r / 255.0f ), SRGBToRGB( g / 255.0f ), SRGBToRGB( b / 255.0f ), 1.0f );
}
inline Color Color::SRGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
  return Color( SRGBToRGB( r / 255.0f ), SRGBToRGB( g / 255.0f ), SRGBToRGB( b / 255.0f ), a / 255.0f );
}
inline Vec3 Color::GetLinearRGB() const { return Vec3( r, g, b ); }
inline Vec4 Color::GetLinearRGBA() const { return Vec4( r, g, b, a ); }
inline Vec3 Color::GetSRGB() const { return Vec3( RGBToSRGB( r ), RGBToSRGB( g ), RGBToSRGB( b ) ); }
inline Vec4 Color::GetSRGBA() const { return Vec4( GetSRGB(), a ); }
inline Color Color::Lerp( const Color& end, float t ) const
{
  return Color(
    ae::Lerp( r, end.r, t ),
    ae::Lerp( g, end.g, t ),
    ae::Lerp( b, end.b, t ),
    ae::Lerp( a, end.a, t )
  );
}
inline Color Color::DtLerp( float snappiness, float dt, const Color& target ) const
{
  return Lerp( target, exp2( -exp2( snappiness ) * dt ) );
}
inline Color Color::ScaleRGB( float s ) const { return Color( r * s, g * s, b * s, a ); }
inline Color Color::ScaleA( float s ) const { return Color( r, g, b, a * s ); }
inline Color Color::SetA( float alpha ) const { return Color( r, g, b, alpha ); }
inline float Color::SRGBToRGB( float x ) { return pow( x , 2.2 ); }
inline float Color::RGBToSRGB( float x ) { return pow( x, 1.0 / 2.2 ); }

#pragma warning(default:26495) // Re-enable uninitialized variable warning

//------------------------------------------------------------------------------
// ae::Str functions
//------------------------------------------------------------------------------
template < uint32_t N >
std::ostream& operator<<( std::ostream& out, const Str< N >& str )
{
  return out << str.c_str();
}

template < uint32_t N >
std::istream& operator>>( std::istream& in, Str< N >& str )
{
  in.getline( str.m_str, Str< N >::MaxLength() );
  str.m_length = in.gcount();
  str.m_str[ str.m_length ] = 0;
  return in;
}

template < uint32_t N >
const Str< N >& ToString( const Str< N >& value )
{
  return value;
}

inline const char* ToString( const char* value )
{
  return value;
}

inline Str16 ToString( int32_t value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%d", value );
  return Str16( length, str );
}

inline Str16 ToString( uint32_t value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%u", value );
  return Str16( length, str );
}

inline Str16 ToString( float value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%.2f", value );
  return Str16( length, str );
}

inline Str16 ToString( double value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%.2f", value );
  return Str16( length, str );
}

template < typename T >
Str64 ToString( const T& v )
{
  std::stringstream os;
  os << v;
  return os.str().c_str();
}

template < uint32_t N >
Str< N >::Str()
{
  AE_STATIC_ASSERT_MSG( sizeof( *this ) == N, "Incorrect Str size" );
  m_length = 0;
  m_str[ 0 ] = 0;
}

template < uint32_t N >
template < uint32_t N2 >
Str< N >::Str( const Str<N2>& str )
{
  AE_ASSERT( str.m_length <= (uint16_t)MaxLength() );
  m_length = str.m_length;
  memcpy( m_str, str.m_str, m_length + 1u );
}

template < uint32_t N >
Str< N >::Str( const char* str )
{
  m_length = (uint16_t)strlen( str );
  AE_ASSERT_MSG( m_length <= (uint16_t)MaxLength(), "Length:# Max:#", m_length, MaxLength() );
  memcpy( m_str, str, m_length + 1u );
}

template < uint32_t N >
Str< N >::Str( uint32_t length, const char* str )
{
  AE_ASSERT( length <= (uint16_t)MaxLength() );
  m_length = length;
  memcpy( m_str, str, m_length );
  m_str[ length ] = 0;
}

template < uint32_t N >
Str< N >::Str( uint32_t length, char c )
{
  AE_ASSERT( length <= (uint16_t)MaxLength() );
  m_length = length;
  memset( m_str, c, m_length );
  m_str[ length ] = 0;
}

template < uint32_t N >
template < typename... Args >
Str< N >::Str( const char* format, Args... args )
{
  m_length = 0;
  m_str[ 0 ] = 0;
  m_Format( format, args... );
}

template < uint32_t N >
Str< N >::operator const char*() const
{
  return m_str;
}

template < uint32_t N >
const char* Str< N >::c_str() const
{
  return m_str;
}

template < uint32_t N >
template < uint32_t N2 >
void Str< N >::operator =( const Str<N2>& str )
{
  AE_ASSERT( str.m_length <= (uint16_t)MaxLength() );
  m_length = str.m_length;
  memcpy( m_str, str.m_str, str.m_length + 1u );
}

template < uint32_t N >
Str<N> Str< N >::operator +( const char* str ) const
{
  Str<N> out( *this );
  out += str;
  return out;
}

template < uint32_t N >
template < uint32_t N2 >
Str<N> Str< N >::operator +( const Str<N2>& str ) const
{
  Str<N> out( *this );
  out += str;
  return out;
}

template < uint32_t N >
void Str< N >::operator +=( const char* str )
{
  uint32_t len = (uint32_t)strlen( str );
  AE_ASSERT( m_length + len <= (uint16_t)MaxLength() );
  memcpy( m_str + m_length, str, len + 1u );
  m_length += len;
}

template < uint32_t N >
template < uint32_t N2 >
void Str< N >::operator +=( const Str<N2>& str )
{
  AE_ASSERT( m_length + str.m_length <= (uint16_t)MaxLength() );
  memcpy( m_str + m_length, str.c_str(), str.m_length + 1u );
  m_length += str.m_length;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator ==( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) == 0;
}

template < uint32_t N >
bool Str< N >::operator ==( const char* str ) const
{
  return strcmp( m_str, str ) == 0;
}

template < uint32_t N >
bool operator ==( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) == 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator !=( const Str<N2>& str ) const
{
  return !operator==( str );
}

template < uint32_t N >
bool Str< N >::operator !=( const char* str ) const
{
  return !operator==( str );
}

template < uint32_t N >
bool operator !=( const char* str0, const Str<N>& str1 )
{
  return !operator==( str0, str1 );
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator <( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) < 0;
}

template < uint32_t N >
bool Str< N >::operator <( const char* str ) const
{
  return strcmp( m_str, str ) < 0;
}

template < uint32_t N >
bool operator <( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) < 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator >( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) > 0;
}

template < uint32_t N >
bool Str< N >::operator >( const char* str ) const
{
  return strcmp( m_str, str ) > 0;
}

template < uint32_t N >
bool operator >( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) > 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator <=( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) <= 0;
}

template < uint32_t N >
bool Str< N >::operator <=( const char* str ) const
{
  return strcmp( m_str, str ) <= 0;
}

template < uint32_t N >
bool operator <=( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) <= 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator >=( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) >= 0;
}

template < uint32_t N >
bool Str< N >::operator >=( const char* str ) const
{
  return strcmp( m_str, str ) >= 0;
}

template < uint32_t N >
bool operator >=( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) >= 0;
}

template < uint32_t N >
char& Str< N >::operator[]( uint32_t i )
{
  AE_ASSERT( i <= m_length ); return m_str[ i ]; // @NOTE: Allow indexing null, one past length
}

template < uint32_t N >
const char Str< N >::operator[]( uint32_t i ) const
{
  AE_ASSERT( i <= m_length ); return m_str[ i ]; // @NOTE: Allow indexing null, one past length
}

template < uint32_t N >
uint32_t Str< N >::Length() const
{
  return m_length;
}

template < uint32_t N >
uint32_t Str< N >::Size() const
{
  return MaxLength();
}

template < uint32_t N >
bool Str< N >::Empty() const
{
  return m_length == 0;
}

template < uint32_t N >
template < uint32_t N2 >
void Str< N >::Append( const Str<N2>& str )
{
  *this += str;
}

template < uint32_t N >
void Str< N >::Append( const char* str )
{
  *this += str;
}

template < uint32_t N >
void Str< N >::Trim( uint32_t len )
{
  if ( len == m_length )
  {
    return;
  }

  AE_ASSERT( len < m_length );
  m_length = len;
  m_str[ m_length ] = 0;
}

template < uint32_t N >
template < typename... Args >
Str< N > Str< N >::Format( const char* format, Args... args )
{
  Str< N > result( "" );
  result.m_Format( format, args... );
  return result;
}

template < uint32_t N >
void Str< N >::m_Format( const char* format )
{
  *this += format;
}

template < uint32_t N >
template < typename T, typename... Args >
void Str< N >::m_Format( const char* format, T value, Args... args )
{
  if ( !*format )
  {
    return;
  }

  const char* head = format;
  while ( *head && *head != '#' )
  {
    head++;
  }
  if ( head > format )
  {
    *this += Str< N >( head - format, format );
  }

  if ( *head == '#' )
  {
    // @TODO: Replace with ToString()?
    std::ostringstream stream;
    stream << value;
    *this += stream.str().c_str();
    head++;
  }
  m_Format( head, args... );
}

//------------------------------------------------------------------------------
// Array functions
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
inline std::ostream& operator<<( std::ostream& os, const Array< T, N >& array )
{
  os << "<";
  for ( uint32_t i = 0; i < array.Length(); i++ )
  {
    os << array[ i ];
    if ( i != array.Length() - 1 )
    {
      os << ", ";
    }
  }
  return os << ">";
}

template < typename T, uint32_t N >
Array< T, N >::Array()
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
  
  m_length = 0;
  m_size = N;
  m_array = (T*)&m_storage;
}

template < typename T, uint32_t N >
Array< T, N >::Array( uint32_t length, const T& value )
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
  
  m_length = length;
  m_size = N;
  m_array = (T*)&m_storage;
  for ( uint32_t i = 0; i < length; i++ )
  {
    new ( &m_array[ i ] ) T ( value );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  AE_ASSERT( tag != ae::Tag() );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, uint32_t size )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;

  Reserve( size );
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, uint32_t length, const T& value )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;

  Reserve( length );

  m_length = length;
  for ( uint32_t i = 0; i < length; i++ )
  {
    new ( &m_array[ i ] ) T ( value );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( const Array< T, N >& other )
{
  m_length = 0;
  m_size = N;
  m_array = N ? (T*)&m_storage : nullptr;
  m_tag = other.m_tag;
  
  // Array must be initialized above before calling Reserve
  Reserve( other.m_length );

  m_length = other.m_length;
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &m_array[ i ] ) T ( other.m_array[ i ] );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( Array< T, N >&& other ) noexcept
{
  m_tag = other.m_tag;
  if ( N )
  {
    m_length = 0;
    m_size = N;
    m_array = N ? (T*)&m_storage : nullptr;
    *this = other; // Regular assignment (without std::move)
  }
  else
  {
    m_length = other.m_length;
    m_size = other.m_size;
    m_array = other.m_array;
    
    other.m_length = 0;
    other.m_size = 0;
    other.m_array = nullptr;
    // @NOTE: Don't reset tag. 'other' must remain in a valid state.
  }
}

template < typename T, uint32_t N >
Array< T, N >::~Array()
{
  Clear();
  
  if ( N == 0 )
  {
    ae::Free( m_array );
  }
  m_size = 0;
  m_array = nullptr;
}

template < typename T, uint32_t N >
void Array< T, N >::operator =( const Array< T, N >& other )
{
  if ( m_array == other.m_array )
  {
    return;
  }
  
  Clear();
  
  if ( m_size < other.m_length )
  {
    Reserve( other.m_length );
  }

  m_length = other.m_length;
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &m_array[ i ] ) T ( other.m_array[ i ] );
  }
}

template < typename T, uint32_t N >
void Array< T, N >::operator =( Array< T, N >&& other ) noexcept
{
  if ( N || m_tag != other.m_tag )
  {
    *this = other; // Regular assignment (without std::move)
  }
  else
  {
    if ( m_array )
    {
      Clear();
      ae::Free( m_array );
    }
    
    m_length = other.m_length;
    m_size = other.m_size;
    m_array = other.m_array;
    
    other.m_length = 0;
    other.m_size = 0;
    other.m_array = nullptr;
  }
}

template < typename T, uint32_t N >
T& Array< T, N >::Append( const T& value )
{
  if ( m_length == m_size )
  {
    Reserve( m_GetNextSize() );
  }

  new ( &m_array[ m_length ] ) T ( value );
  m_length++;

  return m_array[ m_length - 1 ];
}

template < typename T, uint32_t N >
void Array< T, N >::Append( const T* values, uint32_t count )
{
  Reserve( m_length + count );

#if _AE_DEBUG_
  AE_ASSERT( m_size >= m_length + count );
#endif
  for ( uint32_t i = 0; i < count; i++ )
  {
    new ( &m_array[ m_length ] ) T ( values[ i ] );
    m_length++;
  }
}

template < typename T, uint32_t N >
T& Array< T, N >::Insert( uint32_t index, const T& value )
{
#if _AE_DEBUG_
  AE_ASSERT( index <= m_length );
#endif

  if ( m_length == m_size )
  {
    Reserve( m_GetNextSize() );
  }

  if ( index == m_length )
  {
    new ( &m_array[ index ] ) T ( value );
  }
  else
  {
    new ( &m_array[ m_length ] ) T ( std::move( m_array[ m_length - 1 ] ) );
    for ( int32_t i = m_length - 1; i > index; i-- )
    {
      m_array[ i ] = std::move( m_array[ i - 1 ] );
    }
    m_array[ index ] = value;
  }
  
  m_length++;

  return m_array[ index ];
}

template < typename T, uint32_t N >
void Array< T, N >::Remove( uint32_t index )
{
#if _AE_DEBUG_
  AE_ASSERT( index < m_length );
#endif

  m_length--;
  for ( uint32_t i = index; i < m_length; i++ )
  {
    m_array[ i ] = std::move( m_array[ i + 1 ] );
  }
  m_array[ m_length ].~T();
}

template < typename T, uint32_t N >
template < typename U >
uint32_t Array< T, N >::RemoveAll( const U& value )
{
  uint32_t count = 0;
  int32_t index = 0;
  while ( ( index = Find( value ) ) >= 0 )
  {
    // @TODO: Update this to be single loop, so array is only compacted once
    Remove( index );
    count++;
  }
  return count;
}

template < typename T, uint32_t N >
template < typename Fn >
uint32_t Array< T, N >::RemoveAllFn( Fn testFn )
{
  uint32_t count = 0;
  int32_t index = 0;
  while ( ( index = FindFn( testFn ) ) >= 0 )
  {
    // @TODO: Update this to be single loop, so array is only compacted once
    Remove( index );
    count++;
  }
  return count;
}

template < typename T, uint32_t N >
template < typename U >
int32_t Array< T, N >::Find( const U& value ) const
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    if ( m_array[ i ] == value )
    {
      return i;
    }
  }
  return -1;
}

template < typename T, uint32_t N >
template < typename Fn >
int32_t Array< T, N >::FindFn( Fn testFn ) const
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    if ( testFn( m_array[ i ] ) )
    {
      return i;
    }
  }
  return -1;
}

template < typename T, uint32_t N >
void Array< T, N >::Reserve( uint32_t size )
{
  if ( N > 0 )
  {
    AE_ASSERT( N >= size );
    return;
  }
  else if ( size <= m_size )
  {
    return;
  }
  
#if _AE_DEBUG_
  AE_ASSERT( m_tag != ae::Tag() );
#endif
  
  // Next power of two
  size--;
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  size++;
  
#if _AE_DEBUG_
  AE_ASSERT( size );
#endif
  m_size = size;
  
  T* arr = (T*)ae::Allocate( m_tag, m_size * sizeof(T), alignof(T) );
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &arr[ i ] ) T ( std::move( m_array[ i ] ) );
    m_array[ i ].~T();
  }
  
  ae::Free( m_array );
  m_array = arr;
}

template < typename T, uint32_t N >
void Array< T, N >::Clear()
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    m_array[ i ].~T();
  }
  m_length = 0;
}

template < typename T, uint32_t N >
const T& Array< T, N >::operator[]( int32_t index ) const
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT( index < (int32_t)m_length );
#endif
  return m_array[ index ];
}

template < typename T, uint32_t N >
T& Array< T, N >::operator[]( int32_t index )
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT_MSG( index < (int32_t)m_length, "index: # length: #", index, m_length );
#endif
  return m_array[ index ];
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::Length() const
{
  return m_length;
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::Size() const
{
  return m_size;
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::m_GetNextSize() const
{
  if ( m_size == 0 )
  {
    return ae::Max( 1, 32 / sizeof(T) ); // @NOTE: Initially allocate 32 bytes (rounded down) of type
  }
  else
  {
    return m_size * 2;
  }
}

//------------------------------------------------------------------------------
// ae::Map functions
//------------------------------------------------------------------------------
template < typename K >
bool Map_IsEqual( const K& k0, const K& k1 );

template <>
inline bool Map_IsEqual( const char* const & k0, const char* const & k1 )
{
  return strcmp( k0, k1 ) == 0;
}

template < typename K >
bool Map_IsEqual( const K& k0, const K& k1 )
{
  return k0 == k1;
}

template < typename K, typename V, uint32_t N >
Map< K, V, N >::Map()
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static maps" );
}

template < typename K, typename V, uint32_t N >
Map< K, V, N >::Map( ae::Tag pool ) :
  m_entries( pool )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static maps" );
}

template < typename K, typename V, uint32_t N >
Map< K, V, N >::Entry::Entry( const K& k, const V& v ) :
  key( k ),
  value( v )
{}

template < typename K, typename V, uint32_t N >
int32_t Map< K, V, N >::m_FindIndex( const K& key ) const
{
  for ( uint32_t i = 0; i < m_entries.Length(); i++ )
  {
    if ( Map_IsEqual( m_entries[ i ].key, key ) )
    {
      return i;
    }
  }

  return -1;
}

template < typename K, typename V, uint32_t N >
V& Map< K, V, N >::Set( const K& key, const V& value )
{
  int32_t index = m_FindIndex( key );
  Entry* entry = ( index >= 0 ) ? &m_entries[ index ] : nullptr;
  if ( entry )
  {
    entry->value = value;
    return entry->value;
  }
  else
  {
    return m_entries.Append( Entry( key, value ) ).value;
  }
}

template < typename K, typename V, uint32_t N >
V& Map< K, V, N >::Get( const K& key )
{
  return m_entries[ m_FindIndex( key ) ].value;
}

template < typename K, typename V, uint32_t N >
const V& Map< K, V, N >::Get( const K& key ) const
{
  return m_entries[ m_FindIndex( key ) ].value;
}

template < typename K, typename V, uint32_t N >
const V& Map< K, V, N >::Get( const K& key, const V& defaultValue ) const
{
  int32_t index = m_FindIndex( key );
  return ( index >= 0 ) ? m_entries[ index ].value : defaultValue;
}

template < typename K, typename V, uint32_t N >
V* Map< K, V, N >::TryGet( const K& key )
{
  return const_cast< V* >( const_cast< const Map< K, V, N >* >( this )->TryGet( key ) );
}

template < typename K, typename V, uint32_t N >
const V* Map< K, V, N >::TryGet( const K& key ) const
{
  int32_t index = m_FindIndex( key );
  if ( index >= 0 )
  {
    return &m_entries[ index ].value;
  }
  else
  {
    return nullptr;
  }
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::TryGet( const K& key, V* valueOut )
{
  return const_cast< const Map< K, V, N >* >( this )->TryGet( key, valueOut );
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::TryGet( const K& key, V* valueOut ) const
{
  const V* val = TryGet( key );
  if ( val )
  {
    if ( valueOut )
    {
      *valueOut = *val;
    }
    return true;
  }
  return false;
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::Remove( const K& key )
{
  return Remove( key, nullptr );
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::Remove( const K& key, V* valueOut )
{
  int32_t index = m_FindIndex( key );
  if ( index >= 0 )
  {
    if ( valueOut )
    {
      *valueOut = m_entries[ index ].value;
    }
    m_entries.Remove( index );
    return true;
  }
  else
  {
    return false;
  }
}

template < typename K, typename V, uint32_t N >
void Map< K, V, N >::Reserve( uint32_t total )
{
  m_entries.Reserve( total );
}

template < typename K, typename V, uint32_t N >
void Map< K, V, N >::Clear()
{
  m_entries.Clear();
}

template < typename K, typename V, uint32_t N >
const K& Map< K, V, N >::GetKey( uint32_t index ) const
{
  return m_entries[ index ].key;
}

template < typename K, typename V, uint32_t N >
V& Map< K, V, N >::GetValue( uint32_t index )
{
  return m_entries[ index ].value;
}

template < typename K, typename V, uint32_t N >
const V& Map< K, V, N >::GetValue( uint32_t index ) const
{
  return m_entries[ index ].value;
}

template < typename K, typename V, uint32_t N >
uint32_t Map< K, V, N >::Length() const
{
  return m_entries.Length();
}

template < typename K, typename V, uint32_t N >
std::ostream& operator<<( std::ostream& os, const Map< K, V, N >& map )
{
  os << "{";
  for ( uint32_t i = 0; i < map.m_entries.Length(); i++ )
  {
    os << "(" << map.m_entries[ i ].key << ", " << map.m_entries[ i ].value << ")";
    if ( i != map.m_entries.Length() - 1 )
    {
      os << ", ";
    }
  }
  return os << "}";
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
  #define WIN32_LEAN_AND_MEAN 1
  #include "Windows.h"
  #include "processthreadsapi.h" // For GetCurrentProcessId()
#elif _AE_APPLE_
  #include <sys/sysctl.h>
  #include <unistd.h>
  #import <Cocoa/Cocoa.h>
#else
  #include <unistd.h>
#endif
#include <thread>

//------------------------------------------------------------------------------
// Platform functions internal implementation
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

double GetTime()
{
#if _AE_WINDOWS_
  static LARGE_INTEGER counterFrequency = { 0 };
  if ( !counterFrequency.QuadPart )
  {
    bool success = QueryPerformanceFrequency( &counterFrequency ) != 0;
    AE_ASSERT( success );
  }

  LARGE_INTEGER performanceCount = { 0 };
  bool success = QueryPerformanceCounter( &performanceCount ) != 0;
  AE_ASSERT( success );
  return performanceCount.QuadPart / (double)counterFrequency.QuadPart;
#else
  return std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::steady_clock::now().time_since_epoch() ).count() / 1000000.0;
#endif
}

//------------------------------------------------------------------------------
// ae::Vec3 functions
//------------------------------------------------------------------------------
float Vec3::GetAngleBetween( const Vec3& v, float epsilon ) const
{
  const Vec3 crossProduct = Cross( v );
  const float dotProduct = Dot( v );
  if ( crossProduct.LengthSquared() < epsilon && dotProduct > 0.0f )
  {
    return 0.0f;
  }
  else if ( crossProduct.LengthSquared() < epsilon && dotProduct < 0.0f )
  {
    return ae::PI;
  }
  float angle = dotProduct;
  angle /= Length() * v.Length();
  angle = std::acos( angle );
  angle = std::abs( angle );
  return angle;
}

void Vec3::AddRotationXY( float rotation )
{
  float sinTheta = std::sin( rotation );
  float cosTheta = std::cos( rotation );
  float newX = x * cosTheta - y * sinTheta;
  float newY = x * sinTheta + y * cosTheta;
  x = newX;
  y = newY;
}

Vec3 Vec3::RotateCopy( Vec3 axis, float angle ) const
{
  // http://stackoverflow.com/questions/6721544/circular-rotation-around-an-arbitrary-axis
  axis.Normalize();
  float cosA = cosf( angle );
  float mCosA = 1.0f - cosA;
  float sinA = sinf( angle );
  Vec3 r0(
    cosA + axis.x * axis.x * mCosA,
    axis.x * axis.y * mCosA - axis.z * sinA,
    axis.x * axis.z * mCosA + axis.y * sinA );
  Vec3 r1(
    axis.y * axis.x * mCosA + axis.z * sinA,
    cosA + axis.y * axis.y * mCosA,
    axis.y * axis.z * mCosA - axis.x * sinA );
  Vec3 r2(
    axis.z * axis.x * mCosA - axis.y * sinA,
    axis.z * axis.y * mCosA + axis.x * sinA,
    cosA + axis.z * axis.z * mCosA );
  return Vec3( r0.Dot( *this ), r1.Dot( *this ), r2.Dot( *this ) );
}

Vec3 Vec3::Slerp( const Vec3& end, float t, float epsilon ) const
{
  if ( Length() < epsilon || end.Length() < epsilon )
  {
    return Vec3( 0.0f );
  }
  Vec3 v0 = NormalizeCopy();
  Vec3 v1 = end.NormalizeCopy();
  float d = ae::Clip( v0.Dot( v1 ), -1.0f, 1.0f );
  if ( d > ( 1.0f - epsilon ) )
  {
    return v1;
  }
  if ( d < -( 1.0f - epsilon ) )
  {
    return v0;
  }
  float angle = std::acos( d ) * t;
  Vec3 v2 = v1 - v0 * d;
  v2.Normalize();
  return ( ( v0 * std::cos( angle ) ) + ( v2 * std::sin( angle ) ) );
}

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
// _DefaultAllocator class
//------------------------------------------------------------------------------
class _DefaultAllocator final : public Allocator
{
public:
  void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) override
  {
#if _AE_WINDOWS_
    return _aligned_malloc( bytes, alignment );
#elif _AE_LINUX_
    return aligned_alloc( alignment, bytes );
#else
    // @HACK: macosx clang c++11 does not have aligned alloc
    return malloc( bytes );
#endif
  }

  void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) override
  {
#if _AE_WINDOWS_
    return _aligned_realloc( data, bytes, alignment );
#else
    aeCompilationWarning( "Aligned realloc() not determined on this platform" )
      return nullptr;
#endif
  }

  void Free( void* data ) override
  {
#if _AE_WINDOWS_
    _aligned_free( data );
#elif _AE_LINUX_
    free( data );
#else
    free( data );
#endif
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
    // @TODO: Allocating this statically here won't work for hotloading
    static _DefaultAllocator s_allocator;
	  g_allocator = &s_allocator;
  }
  return g_allocator;
}

//------------------------------------------------------------------------------
// ae::TimeStep member functions
//------------------------------------------------------------------------------
TimeStep::TimeStep()
{
  m_stepCount = 0;
  m_timeStep = 0.0f;
  m_frameExcess = 0;
  m_prevFrameTime = 0.0f;

  SetTimeStep( 1.0f / 60.0f );
}

void TimeStep::SetTimeStep( float timeStep )
{
  m_timeStepSec = timeStep; m_timeStep = timeStep * 1000000.0f;
}

float TimeStep::GetTimeStep() const
{
  return m_timeStepSec;
}

uint32_t TimeStep::GetStepCount() const
{
  return m_stepCount;
}

float TimeStep::GetDt() const
{
  return m_prevFrameTimeSec;
}

void TimeStep::SetDt( float sec )
{
  m_prevFrameTimeSec = sec;
}

void TimeStep::Wait()
{
  if ( m_timeStep == 0.0f )
  {
    return;
  }
  
  // @TODO: Maybe this should use the same time source as GetTime()
  
  if ( m_stepCount == 0 )
  {
    m_prevFrameTime = m_timeStep;
    m_frameStart = std::chrono::steady_clock::now();
  }
  else
  {
    std::chrono::steady_clock::time_point execFinish = std::chrono::steady_clock::now();
    std::chrono::microseconds execDuration = std::chrono::duration_cast< std::chrono::microseconds >( execFinish - m_frameStart );
    
    int64_t prevFrameExcess = m_prevFrameTime - m_timeStep;
    m_frameExcess = ( m_frameExcess * 0.5f + prevFrameExcess * 0.5f ) + 0.5f;

    int64_t wait = m_timeStep - execDuration.count();
    wait -= ( m_frameExcess > 0 ) ? m_frameExcess : 0;
    if ( 1000 < wait && wait < m_timeStep )
    {
      std::this_thread::sleep_for( std::chrono::microseconds( wait ) );
    }
    std::chrono::steady_clock::time_point frameFinish = std::chrono::steady_clock::now();
    std::chrono::microseconds frameDuration = std::chrono::duration_cast< std::chrono::microseconds >( frameFinish - m_frameStart );

    m_prevFrameTime = frameDuration.count();
    m_frameStart = std::chrono::steady_clock::now();
  }
  
  m_prevFrameTimeSec = m_prevFrameTime / 1000000.0f;
  
  m_stepCount++;
}

//------------------------------------------------------------------------------
// ae::Rect member functions
//------------------------------------------------------------------------------
Rect::Rect( Vec2 p0, Vec2 p1 )
{
  if ( p0.x < p1.x )
  {
    x = p0.x;
    w = p1.x - p0.x;
  }
  else
  {
    x = p1.x;
    w = p0.x - p1.x;
  }

  if ( p0.y < p1.y )
  {
    y = p0.y;
    h = p1.y - p0.y;
  }
  else
  {
    y = p1.y;
    h = p0.y - p1.y;
  }
}

bool Rect::Contains( Vec2 pos ) const
{
  return !( pos.x < x || pos.x >= ( x + w ) || pos.y < y || pos.y >= ( y + h ) );
}

void Rect::Expand( Vec2 pos )
{
  if ( w == 0.0f )
  {
    x = pos.x;
  }
  else
  {
    float x1 = ae::Max( x + w, pos.x );
    x = ae::Min( x, pos.x );
    w = x1 - x;
  }

  if ( h == 0.0f )
  {
    y = pos.y;
  }
  else
  {
    float y1 = ae::Max( y + h, pos.y );
    y = ae::Min( y, pos.y );
    h = y1 - y;
  }
}

bool Rect::GetIntersection( const Rect& other, Rect* intersectionOut ) const
{
  float x0 = ae::Max( x, other.x );
  float x1 = ae::Min( x + w, other.x + other.w );
  float y0 = ae::Max( y, other.y );
  float y1 = ae::Min( y + h, other.y + other.h );
  if ( x0 < x1 && y0 < y1 )
  {
    if ( intersectionOut )
    {
      *intersectionOut = Rect( Vec2( x0, y0 ), Vec2( x1, y1 ) );
    }
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
// Window member functions
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
// @TODO: Cleanup namespace
LRESULT CALLBACK WinProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  ae::Window* window = (ae::Window*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
  switch ( msg )
  {
    case WM_CREATE:
    {
      // Store ae window pointer in window state. Retrievable with GetWindowLongPtr()
      // https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-?redirectedfrom=MSDN
      // @TODO: Handle these error cases gracefully
      CREATESTRUCT* createMsg = (CREATESTRUCT*)lParam;
      AE_ASSERT( createMsg );
      ae::Window* window = (ae::Window*)createMsg->lpCreateParams;
      AE_ASSERT( window );
      SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)window );
      AE_ASSERT( window == (ae::Window*)GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
      window->window = hWnd;
      break;
    }
    case WM_SIZE:
    {
      if ( window->graphicsDevice )
      {
        uint32_t width = LOWORD( lParam );
        uint32_t height = HIWORD( lParam );
        window->m_UpdateWidthHeight( width, height );
        window->graphicsDevice->m_HandleResize( width, height );
      }
      break;
    }
    case WM_CLOSE:
    {
      PostQuitMessage( 0 );
      break;
    }
  }
  return DefWindowProc( hWnd, msg, wParam, lParam );
}
#endif

Window::Window()
{
  window = nullptr;
  graphicsDevice = nullptr;
  m_pos = Int2( 0.0f ); // @TODO: int
  m_width = 0;
  m_height = 0;
  m_fullScreen = false;
  m_maximized = false;
}

bool Window::Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor )
{
  AE_ASSERT( !window );

  //m_pos = Int2( fullScreen ? 0 : (int)SDL_WINDOWPOS_CENTERED );
  m_width = width;
  m_height = height;
  m_fullScreen = fullScreen;

  m_Initialize();

  //SDL_ShowCursor( showCursor ? SDL_ENABLE : SDL_DISABLE );
  //SDL_GetWindowPosition( (SDL_Window*)window, &m_pos.x, &m_pos.y );

  return false;
}

bool Window::Initialize( Int2 pos, uint32_t width, uint32_t height, bool showCursor )
{
  AE_ASSERT( !window );

  m_pos = pos;
  m_width = width;
  m_height = height;
  m_fullScreen = false;

  m_Initialize();

  //SDL_ShowCursor( showCursor ? SDL_ENABLE : SDL_DISABLE );

  return false;
}

void Window::m_Initialize()
{
//  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER ) < 0 )
//  {
//    AE_FAIL_MSG( "SDL could not initialize: #", SDL_GetError() );
//  }
//
//#if _AE_IOS_
//  m_pos = Int2( 0 );
//  m_fullScreen = true;
//
//  SDL_DisplayMode displayMode;
//  if ( SDL_GetDesktopDisplayMode( 0, &displayMode ) == 0 )
//  {
//    m_width = displayMode.w;
//    m_height = displayMode.h;
//  }
//
//  window = SDL_CreateWindow( "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, SDL_WINDOW_SHOWN );
//#else
//  Rect windowRect( m_pos.x, m_pos.y, m_width, m_height );
//  bool overlapsAny = false;
//  uint32_t displayCount = SDL_GetNumVideoDisplays();
//  for ( uint32_t i = 0; i < displayCount; i++ )
//  {
//    SDL_Rect rect;
//    int result = SDL_GetDisplayBounds( i, &rect );
//    if ( result == 0 )
//    {
//      Rect screenRect( rect.x, rect.y, rect.w, rect.h );
//      Rect intersection;
//      if ( windowRect.GetIntersection( screenRect, &intersection ) )
//      {
//        // Check how much window overlaps. This prevent windows that are barely overlapping from appearing offscreen.
//        float intersectionArea = intersection.w * intersection.h;
//        float screenArea = screenRect.w * screenRect.h;
//        float windowArea = windowRect.w * windowRect.h;
//        float screenOverlap = intersectionArea / screenArea;
//        float windowOverlap = intersectionArea / windowArea;
//        if ( screenOverlap > 0.1f || windowOverlap > 0.1f )
//        {
//          overlapsAny = true;
//          break;
//        }
//      }
//    }
//  }
//
//  if ( !overlapsAny && displayCount )
//  {
//    SDL_Rect screenRect;
//    if ( SDL_GetDisplayBounds( 0, &screenRect ) == 0 )
//    {
//      int32_t border = screenRect.w / 16;
//
//      m_width = screenRect.w - border * 2;
//      int32_t h0 = screenRect.h - border * 2;
//      int32_t h1 = m_width * ( 10.0f / 16.0f );
//      m_height = aeMath::Min( h0, h1 );
//
//      m_pos.x = border;
//      m_pos.y = ( screenRect.h - m_height ) / 2;
//      m_pos.x += screenRect.x;
//      m_pos.y += screenRect.y;
//
//      m_fullScreen = false;
//    }
//  }
//
//  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
//  flags |= m_fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
//  window = SDL_CreateWindow( "", m_pos.x, m_pos.y, m_width, m_height, flags );
//#endif
//  AE_ASSERT( window );
//
//  SDL_SetWindowTitle( (SDL_Window*)window, "" );
//  m_windowTitle = "";

#if _AE_WINDOWS_
#define WNDCLASSNAME L"wndclass"
  HINSTANCE hinstance = GetModuleHandle( NULL );

  WNDCLASSEX ex;
  memset( &ex, 0, sizeof( ex ) );
  ex.cbSize = sizeof( ex );
  ex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  ex.lpfnWndProc = WinProc;
  ex.hInstance = hinstance;
  ex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
  ex.hCursor = LoadCursor( NULL, IDC_ARROW );
  ex.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
  ex.lpszClassName = WNDCLASSNAME;
  if ( !RegisterClassEx( &ex ) ) // Create the window
  {
    AE_FAIL_MSG( "Failed to register window. Error: #", GetLastError() );
  }

  // WS_POPUP for full screen
  uint32_t windowStyle = WS_OVERLAPPEDWINDOW;
  windowStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  HWND hwnd = CreateWindowEx( NULL, WNDCLASSNAME, L"Window", WS_OVERLAPPEDWINDOW, 0, 0, GetWidth(), GetHeight(), NULL, NULL, hinstance, this );
  AE_ASSERT_MSG( hwnd, "Failed to create window. Error: #", GetLastError() );

  HDC hdc = GetDC( hwnd );
  AE_ASSERT_MSG( hdc, "Failed to Get the Window Device Context" );

  // Choose the best pixel format for the curent environment
  PIXELFORMATDESCRIPTOR pfd;
  memset( &pfd, 0, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int indexPixelFormat = ChoosePixelFormat( hdc, &pfd );
  AE_ASSERT_MSG( indexPixelFormat, "Failed to choose pixel format. Error: #", GetLastError() );
  if ( !DescribePixelFormat( hdc, indexPixelFormat, sizeof( pfd ), &pfd ) )
  {
    AE_FAIL_MSG( "Failed to read chosen pixel format. Error: #", GetLastError() );
  }
  AE_INFO( "Chosen Pixel format: #bit RGB #bit Depth",
    (int)pfd.cColorBits,
    (int)pfd.cDepthBits
  );
  if ( !SetPixelFormat( hdc, indexPixelFormat, &pfd ) )
  {
    AE_FAIL_MSG( "Could not set window pixel format. Error: #", GetLastError() );
  }

  // Finish window setup
  ShowWindow( hwnd, SW_SHOW );
  SetForegroundWindow( hwnd ); // Slightly Higher Priority
  SetFocus( hwnd ); // Sets Keyboard Focus To The Window
  if ( !UpdateWindow( hwnd ) )
  {
    AE_FAIL_MSG( "Failed on first window update. Error: #", GetLastError() );
  }
#elif _AE_OSX_
  // Autorelease Pool
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  // Create shared app instance
  [NSApplication sharedApplication];

  // Main window
  NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask;
  NSRect windowRect = NSMakeRect(100, 100, 400, 400);
//  NSWindow* window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];

  // Test content
//  NSTextView* textView = [[NSTextView alloc] initWithFrame:windowRect];
//  [window setContentView:textView];
//  NSOpenGLView* openGLView = [[NSOpenGLView alloc] init:windowRect pizelFormat:];
//  [window setContentView:openGLView];
  
  NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(100,100,400,300)
    styleMask:NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask
    backing:NSBackingStoreBuffered
    defer:YES
  ];
  NSRect frame = [w contentRectForFrameRect:[w frame]];
  // this is optional - request accelerated context
  unsigned int attrs[] = { NSOpenGLPFAAccelerated, 0 };
  NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:(NSOpenGLPixelFormatAttribute*)attrs];
  NSOpenGLView  *view = [[NSOpenGLView alloc] initWithFrame:frame pixelFormat:pixelFormat];
  [pixelFormat release];
  // manage properties of the window as you please ...
  [w setOpaque:YES];
  [w setContentView:view];
  [w makeFirstResponder:view];
  [w setContentMinSize:NSMakeSize(150.0, 100.0)];
  //[w makeKeyAndOrderFront: self];

  // Window controller
  NSWindowController* windowController = [[NSWindowController alloc] initWithWindow:w];

  // @todo Create app delegate
  // @todo Create menus (especially Quit!)

  // Show window and run event loop
  [w orderFrontRegardless];
  [NSApp run];
#endif
}

void Window::Terminate()
{
  //SDL_DestroyWindow( (SDL_Window*)window );
}

void Window::SetTitle( const char* title )
{
  if ( window && m_windowTitle != title )
  {
    //SDL_SetWindowTitle( (SDL_Window*)window, title );
    m_windowTitle = title;
  }
}

void Window::SetFullScreen( bool fullScreen )
{
  //if ( window )
  //{
  //  uint32_t oldFlags = SDL_GetWindowFlags( (SDL_Window*)window );

  //  uint32_t newFlags = oldFlags;
  //  if ( fullScreen )
  //  {
  //    newFlags |= SDL_WINDOW_FULLSCREEN;
  //  }
  //  else
  //  {
  //    newFlags &= ~SDL_WINDOW_FULLSCREEN;
  //  }

  //  if ( newFlags != oldFlags )
  //  {
  //    SDL_SetWindowFullscreen( (SDL_Window*)window, newFlags );
  //  }

  //  m_fullScreen = fullScreen;
  //}
}

void Window::SetPosition( Int2 pos )
{
  //if ( window )
  //{
  //  SDL_SetWindowPosition( (SDL_Window*)window, pos.x, pos.y );
  //  m_pos = pos;
  //}
}

void Window::SetSize( uint32_t width, uint32_t height )
{
  //if ( window )
  //{
  //  SDL_SetWindowSize( (SDL_Window*)window, width, height );
  //  m_width = width;
  //  m_height = height;
  //}
}

void Window::SetMaximized( bool maximized )
{
  //if ( maximized )
  //{
  //  SDL_MaximizeWindow( (SDL_Window*)window );
  //}
  //else
  //{
  //  SDL_RestoreWindow( (SDL_Window*)window );
  //}
  //m_maximized = maximized;
}

//------------------------------------------------------------------------------
// ae::Input member functions
//------------------------------------------------------------------------------
void Input::Pump()
{
#if _AE_WINDOWS_
  MSG msg;
  // Get messages for current thread
  while ( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
  {
    switch ( msg.message )
    {
      case WM_QUIT:
        quit = true;
        break;
    }
    TranslateMessage( &msg );
    DispatchMessage( &msg );
  }
#endif
}

}  // AE_NAMESPACE end

//------------------------------------------------------------------------------
// OpenGL includes
//------------------------------------------------------------------------------
// @TODO: Are these being included in the ae namespace? Fix if so
#if _AE_WINDOWS_
	#pragma comment (lib, "opengl32.lib")
	#pragma comment (lib, "glu32.lib")
	#include <gl/GL.h>
	#include <gl/GLU.h>
#elif _AE_EMSCRIPTEN_
  #include <GLES2/gl2.h>
#elif _AE_LINUX_
  #include <GL/gl.h>
  #include <GLES3/gl3.h>
#elif _AE_IOS_
  #include <OpenGLES/ES3/gl.h>
  //#include <OpenGLES/ES3/glext.h>
  //#define glClearDepth glClearDepthf
#else
  #include <OpenGL/gl.h>
#endif

// Caller enables this externally.  The renderer, AEShader, math aren't tied to one another
// enough to pass this locally.  glClipControl is also no accessible in ES or GL 4.1, so
// doing this just to write the shaders for reverseZ.  In GL, this won't improve precision.
// http://www.reedbeta.com/blog/depth-precision-visualized/
bool gReverseZ = false;

// turn this on to run at GL4.1 instead of GL3.3
bool gGL41 = true;

// OpenGL function pointers
typedef char GLchar;

// GL_VERSION_2_1
//#define GL_PIXEL_PACK_BUFFER              0x88EB
//#define GL_PIXEL_UNPACK_BUFFER            0x88EC
//#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
//#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
//#define GL_FLOAT_MAT2x3                   0x8B65
//#define GL_FLOAT_MAT2x4                   0x8B66
//#define GL_FLOAT_MAT3x2                   0x8B67
//#define GL_FLOAT_MAT3x4                   0x8B68
//#define GL_FLOAT_MAT4x2                   0x8B69
//#define GL_FLOAT_MAT4x3                   0x8B6A
//#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
//#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43
//#define GL_COMPRESSED_SRGB                0x8C48
//#define GL_COMPRESSED_SRGB_ALPHA          0x8C49
//#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
//#define GL_SLUMINANCE_ALPHA               0x8C44
//#define GL_SLUMINANCE8_ALPHA8             0x8C45
//#define GL_SLUMINANCE                     0x8C46
//#define GL_SLUMINANCE8                    0x8C47
//#define GL_COMPRESSED_SLUMINANCE          0x8C4A
//#define GL_COMPRESSED_SLUMINANCE_ALPHA    0x8C4B
// GL_VERSION_3_2
//#define GL_CONTEXT_CORE_PROFILE_BIT       0x00000001
//#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
//#define GL_LINES_ADJACENCY                0x000A
//#define GL_LINE_STRIP_ADJACENCY           0x000B
//#define GL_TRIANGLES_ADJACENCY            0x000C
//#define GL_TRIANGLE_STRIP_ADJACENCY       0x000D
//#define GL_PROGRAM_POINT_SIZE             0x8642
//#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
//#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
//#define GL_GEOMETRY_SHADER                0x8DD9
//#define GL_GEOMETRY_VERTICES_OUT          0x8916
//#define GL_GEOMETRY_INPUT_TYPE            0x8917
//#define GL_GEOMETRY_OUTPUT_TYPE           0x8918
//#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
//#define GL_MAX_GEOMETRY_OUTPUT_VERTICES   0x8DE0
//#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
//#define GL_MAX_VERTEX_OUTPUT_COMPONENTS   0x9122
//#define GL_MAX_GEOMETRY_INPUT_COMPONENTS  0x9123
//#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
//#define GL_MAX_FRAGMENT_INPUT_COMPONENTS  0x9125
//#define GL_CONTEXT_PROFILE_MASK           0x9126
//#define GL_DEPTH_CLAMP                    0x864F
//#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
//#define GL_FIRST_VERTEX_CONVENTION        0x8E4D
//#define GL_LAST_VERTEX_CONVENTION         0x8E4E
//#define GL_PROVOKING_VERTEX               0x8E4F
//#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F
//#define GL_MAX_SERVER_WAIT_TIMEOUT        0x9111
//#define GL_OBJECT_TYPE                    0x9112
//#define GL_SYNC_CONDITION                 0x9113
//#define GL_SYNC_STATUS                    0x9114
//#define GL_SYNC_FLAGS                     0x9115
//#define GL_SYNC_FENCE                     0x9116
//#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
//#define GL_UNSIGNALED                     0x9118
//#define GL_SIGNALED                       0x9119
//#define GL_ALREADY_SIGNALED               0x911A
//#define GL_TIMEOUT_EXPIRED                0x911B
//#define GL_CONDITION_SATISFIED            0x911C
//#define GL_WAIT_FAILED                    0x911D
//#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
//#define GL_SYNC_FLUSH_COMMANDS_BIT        0x00000001
//#define GL_SAMPLE_POSITION                0x8E50
//#define GL_SAMPLE_MASK                    0x8E51
//#define GL_SAMPLE_MASK_VALUE              0x8E52
//#define GL_MAX_SAMPLE_MASK_WORDS          0x8E59
//#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
//#define GL_PROXY_TEXTURE_2D_MULTISAMPLE   0x9101
//#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
//#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9103
//#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
//#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
//#define GL_TEXTURE_SAMPLES                0x9106
//#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
//#define GL_SAMPLER_2D_MULTISAMPLE         0x9108
//#define GL_INT_SAMPLER_2D_MULTISAMPLE     0x9109
//#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
//#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY   0x910B
//#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
//#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
//#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
//#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F
//#define GL_MAX_INTEGER_SAMPLES            0x9110

//#define GL_UNSIGNED_BYTE_3_3_2            0x8032
//#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
//#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
//#define GL_UNSIGNED_INT_8_8_8_8           0x8035
//#define GL_UNSIGNED_INT_10_10_10_2        0x8036
//#define GL_TEXTURE_BINDING_3D             0x806A
//#define GL_PACK_SKIP_IMAGES               0x806B
//#define GL_PACK_IMAGE_HEIGHT              0x806C
//#define GL_UNPACK_SKIP_IMAGES             0x806D
//#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
//#define GL_PROXY_TEXTURE_3D               0x8070
//#define GL_TEXTURE_DEPTH                  0x8071
//#define GL_TEXTURE_WRAP_R                 0x8072
//#define GL_MAX_3D_TEXTURE_SIZE            0x8073
//#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
//#define GL_UNSIGNED_SHORT_5_6_5           0x8363
//#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
//#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
//#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
//#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
//#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
//#define GL_BGR                            0x80E0
//#define GL_BGRA                           0x80E1
//#define GL_MAX_ELEMENTS_VERTICES          0x80E8
//#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_CLAMP_TO_EDGE                  0x812F
//#define GL_TEXTURE_MIN_LOD                0x813A
//#define GL_TEXTURE_MAX_LOD                0x813B
//#define GL_TEXTURE_BASE_LEVEL             0x813C
//#define GL_TEXTURE_MAX_LEVEL              0x813D
//#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
//#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
//#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
//#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
//#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
//#define GL_RESCALE_NORMAL                 0x803A
//#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
//#define GL_SINGLE_COLOR                   0x81F9
//#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
//#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_TEXTURE0                       0x84C0
//#define GL_TEXTURE1                       0x84C1
//#define GL_TEXTURE2                       0x84C2
//#define GL_TEXTURE3                       0x84C3
//#define GL_TEXTURE4                       0x84C4
//#define GL_TEXTURE5                       0x84C5
//#define GL_TEXTURE6                       0x84C6
//#define GL_TEXTURE7                       0x84C7
//#define GL_TEXTURE8                       0x84C8
//#define GL_TEXTURE9                       0x84C9
//#define GL_TEXTURE10                      0x84CA
//#define GL_TEXTURE11                      0x84CB
//#define GL_TEXTURE12                      0x84CC
//#define GL_TEXTURE13                      0x84CD
//#define GL_TEXTURE14                      0x84CE
//#define GL_TEXTURE15                      0x84CF
//#define GL_TEXTURE16                      0x84D0
//#define GL_TEXTURE17                      0x84D1
//#define GL_TEXTURE18                      0x84D2
//#define GL_TEXTURE19                      0x84D3
//#define GL_TEXTURE20                      0x84D4
//#define GL_TEXTURE21                      0x84D5
//#define GL_TEXTURE22                      0x84D6
//#define GL_TEXTURE23                      0x84D7
//#define GL_TEXTURE24                      0x84D8
//#define GL_TEXTURE25                      0x84D9
//#define GL_TEXTURE26                      0x84DA
//#define GL_TEXTURE27                      0x84DB
//#define GL_TEXTURE28                      0x84DC
//#define GL_TEXTURE29                      0x84DD
//#define GL_TEXTURE30                      0x84DE
//#define GL_TEXTURE31                      0x84DF
//#define GL_ACTIVE_TEXTURE                 0x84E0
//#define GL_MULTISAMPLE                    0x809D
//#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
//#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
//#define GL_SAMPLE_COVERAGE                0x80A0
//#define GL_SAMPLE_BUFFERS                 0x80A8
//#define GL_SAMPLES                        0x80A9
//#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
//#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
//#define GL_TEXTURE_CUBE_MAP               0x8513
//#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
//#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
//#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
//#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
//#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
//#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
//#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
//#define GL_PROXY_TEXTURE_CUBE_MAP         0x851B
//#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
//#define GL_COMPRESSED_RGB                 0x84ED
//#define GL_COMPRESSED_RGBA                0x84EE
//#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
//#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
//#define GL_TEXTURE_COMPRESSED             0x86A1
//#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
//#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3
//#define GL_CLAMP_TO_BORDER                0x812D
//#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1
//#define GL_MAX_TEXTURE_UNITS              0x84E2
//#define GL_TRANSPOSE_MODELVIEW_MATRIX     0x84E3
//#define GL_TRANSPOSE_PROJECTION_MATRIX    0x84E4
//#define GL_TRANSPOSE_TEXTURE_MATRIX       0x84E5
//#define GL_TRANSPOSE_COLOR_MATRIX         0x84E6
//#define GL_MULTISAMPLE_BIT                0x20000000
//#define GL_NORMAL_MAP                     0x8511
//#define GL_REFLECTION_MAP                 0x8512
//#define GL_COMPRESSED_ALPHA               0x84E9
//#define GL_COMPRESSED_LUMINANCE           0x84EA
//#define GL_COMPRESSED_LUMINANCE_ALPHA     0x84EB
//#define GL_COMPRESSED_INTENSITY           0x84EC
//#define GL_COMBINE                        0x8570
//#define GL_COMBINE_RGB                    0x8571
//#define GL_COMBINE_ALPHA                  0x8572
//#define GL_SOURCE0_RGB                    0x8580
//#define GL_SOURCE1_RGB                    0x8581
//#define GL_SOURCE2_RGB                    0x8582
//#define GL_SOURCE0_ALPHA                  0x8588
//#define GL_SOURCE1_ALPHA                  0x8589
//#define GL_SOURCE2_ALPHA                  0x858A
//#define GL_OPERAND0_RGB                   0x8590
//#define GL_OPERAND1_RGB                   0x8591
//#define GL_OPERAND2_RGB                   0x8592
//#define GL_OPERAND0_ALPHA                 0x8598
//#define GL_OPERAND1_ALPHA                 0x8599
//#define GL_OPERAND2_ALPHA                 0x859A
//#define GL_RGB_SCALE                      0x8573
//#define GL_ADD_SIGNED                     0x8574
//#define GL_INTERPOLATE                    0x8575
//#define GL_SUBTRACT                       0x84E7
//#define GL_CONSTANT                       0x8576
//#define GL_PRIMARY_COLOR                  0x8577
//#define GL_PREVIOUS                       0x8578
//#define GL_DOT3_RGB                       0x86AE
//#define GL_DOT3_RGBA                      0x86AF
//#define GL_BLEND_EQUATION_RGB             0x8009
//#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
//#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
//#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
//#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
//#define GL_CURRENT_VERTEX_ATTRIB          0x8626
//#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
//#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
//#define GL_STENCIL_BACK_FUNC              0x8800
//#define GL_STENCIL_BACK_FAIL              0x8801
//#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
//#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
//#define GL_MAX_DRAW_BUFFERS               0x8824
//#define GL_DRAW_BUFFER0                   0x8825
//#define GL_DRAW_BUFFER1                   0x8826
//#define GL_DRAW_BUFFER2                   0x8827
//#define GL_DRAW_BUFFER3                   0x8828
//#define GL_DRAW_BUFFER4                   0x8829
//#define GL_DRAW_BUFFER5                   0x882A
//#define GL_DRAW_BUFFER6                   0x882B
//#define GL_DRAW_BUFFER7                   0x882C
//#define GL_DRAW_BUFFER8                   0x882D
//#define GL_DRAW_BUFFER9                   0x882E
//#define GL_DRAW_BUFFER10                  0x882F
//#define GL_DRAW_BUFFER11                  0x8830
//#define GL_DRAW_BUFFER12                  0x8831
//#define GL_DRAW_BUFFER13                  0x8832
//#define GL_DRAW_BUFFER14                  0x8833
//#define GL_DRAW_BUFFER15                  0x8834
//#define GL_BLEND_EQUATION_ALPHA           0x883D
//#define GL_MAX_VERTEX_ATTRIBS             0x8869
//#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
//#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
//#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
//#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
//#define GL_MAX_VARYING_FLOATS             0x8B4B
//#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
//#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
//#define GL_SHADER_TYPE                    0x8B4F
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
//#define GL_INT_VEC2                       0x8B53
//#define GL_INT_VEC3                       0x8B54
//#define GL_INT_VEC4                       0x8B55
//#define GL_BOOL                           0x8B56
//#define GL_BOOL_VEC2                      0x8B57
//#define GL_BOOL_VEC3                      0x8B58
//#define GL_BOOL_VEC4                      0x8B59
//#define GL_FLOAT_MAT2                     0x8B5A
//#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
//#define GL_SAMPLER_1D                     0x8B5D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
//#define GL_SAMPLER_CUBE                   0x8B60
//#define GL_SAMPLER_1D_SHADOW              0x8B61
//#define GL_SAMPLER_2D_SHADOW              0x8B62
//#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
//#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
//#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
//#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
//#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
//#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
//#define GL_CURRENT_PROGRAM                0x8B8D
//#define GL_POINT_SPRITE_COORD_ORIGIN      0x8CA0
//#define GL_LOWER_LEFT                     0x8CA1
//#define GL_UPPER_LEFT                     0x8CA2
//#define GL_STENCIL_BACK_REF               0x8CA3
//#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
//#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
//#define GL_VERTEX_PROGRAM_TWO_SIDE        0x8643
//#define GL_POINT_SPRITE                   0x8861
//#define GL_COORD_REPLACE                  0x8862
//#define GL_MAX_TEXTURE_COORDS             0x8871
//#define GL_COMPARE_REF_TO_TEXTURE         0x884E
//#define GL_CLIP_DISTANCE0                 0x3000
//#define GL_CLIP_DISTANCE1                 0x3001
//#define GL_CLIP_DISTANCE2                 0x3002
//#define GL_CLIP_DISTANCE3                 0x3003
//#define GL_CLIP_DISTANCE4                 0x3004
//#define GL_CLIP_DISTANCE5                 0x3005
//#define GL_CLIP_DISTANCE6                 0x3006
//#define GL_CLIP_DISTANCE7                 0x3007
//#define GL_MAX_CLIP_DISTANCES             0x0D32
//#define GL_MAJOR_VERSION                  0x821B
//#define GL_MINOR_VERSION                  0x821C
//#define GL_NUM_EXTENSIONS                 0x821D
//#define GL_CONTEXT_FLAGS                  0x821E
//#define GL_COMPRESSED_RED                 0x8225
//#define GL_COMPRESSED_RG                  0x8226
//#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
//#define GL_VERTEX_ATTRIB_ARRAY_INTEGER    0x88FD
//#define GL_MAX_ARRAY_TEXTURE_LAYERS       0x88FF
//#define GL_MIN_PROGRAM_TEXEL_OFFSET       0x8904
//#define GL_MAX_PROGRAM_TEXEL_OFFSET       0x8905
//#define GL_CLAMP_READ_COLOR               0x891C
//#define GL_FIXED_ONLY                     0x891D
//#define GL_MAX_VARYING_COMPONENTS         0x8B4B
//#define GL_TEXTURE_1D_ARRAY               0x8C18
//#define GL_PROXY_TEXTURE_1D_ARRAY         0x8C19
//#define GL_TEXTURE_2D_ARRAY               0x8C1A
//#define GL_PROXY_TEXTURE_2D_ARRAY         0x8C1B
//#define GL_TEXTURE_BINDING_1D_ARRAY       0x8C1C
//#define GL_TEXTURE_BINDING_2D_ARRAY       0x8C1D
//#define GL_R11F_G11F_B10F                 0x8C3A
//#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
//#define GL_RGB9_E5                        0x8C3D
//#define GL_UNSIGNED_INT_5_9_9_9_REV       0x8C3E
//#define GL_TEXTURE_SHARED_SIZE            0x8C3F
//#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
//#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
//#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
//#define GL_TRANSFORM_FEEDBACK_VARYINGS    0x8C83
//#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
//#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
//#define GL_PRIMITIVES_GENERATED           0x8C87
//#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
//#define GL_RASTERIZER_DISCARD             0x8C89
//#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
//#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
//#define GL_INTERLEAVED_ATTRIBS            0x8C8C
//#define GL_SEPARATE_ATTRIBS               0x8C8D
//#define GL_TRANSFORM_FEEDBACK_BUFFER      0x8C8E
//#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
//#define GL_RGBA32UI                       0x8D70
//#define GL_RGB32UI                        0x8D71
//#define GL_RGBA16UI                       0x8D76
//#define GL_RGB16UI                        0x8D77
//#define GL_RGBA8UI                        0x8D7C
//#define GL_RGB8UI                         0x8D7D
//#define GL_RGBA32I                        0x8D82
//#define GL_RGB32I                         0x8D83
//#define GL_RGBA16I                        0x8D88
//#define GL_RGB16I                         0x8D89
//#define GL_RGBA8I                         0x8D8E
//#define GL_RGB8I                          0x8D8F
//#define GL_RED_INTEGER                    0x8D94
//#define GL_GREEN_INTEGER                  0x8D95
//#define GL_BLUE_INTEGER                   0x8D96
//#define GL_RGB_INTEGER                    0x8D98
//#define GL_RGBA_INTEGER                   0x8D99
//#define GL_BGR_INTEGER                    0x8D9A
//#define GL_BGRA_INTEGER                   0x8D9B
//#define GL_SAMPLER_1D_ARRAY               0x8DC0
//#define GL_SAMPLER_2D_ARRAY               0x8DC1
//#define GL_SAMPLER_1D_ARRAY_SHADOW        0x8DC3
//#define GL_SAMPLER_2D_ARRAY_SHADOW        0x8DC4
//#define GL_SAMPLER_CUBE_SHADOW            0x8DC5
//#define GL_UNSIGNED_INT_VEC2              0x8DC6
//#define GL_UNSIGNED_INT_VEC3              0x8DC7
//#define GL_UNSIGNED_INT_VEC4              0x8DC8
//#define GL_INT_SAMPLER_1D                 0x8DC9
//#define GL_INT_SAMPLER_2D                 0x8DCA
//#define GL_INT_SAMPLER_3D                 0x8DCB
//#define GL_INT_SAMPLER_CUBE               0x8DCC
//#define GL_INT_SAMPLER_1D_ARRAY           0x8DCE
//#define GL_INT_SAMPLER_2D_ARRAY           0x8DCF
//#define GL_UNSIGNED_INT_SAMPLER_1D        0x8DD1
//#define GL_UNSIGNED_INT_SAMPLER_2D        0x8DD2
//#define GL_UNSIGNED_INT_SAMPLER_3D        0x8DD3
//#define GL_UNSIGNED_INT_SAMPLER_CUBE      0x8DD4
//#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY  0x8DD6
//#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY  0x8DD7
//#define GL_QUERY_WAIT                     0x8E13
//#define GL_QUERY_NO_WAIT                  0x8E14
//#define GL_QUERY_BY_REGION_WAIT           0x8E15
//#define GL_QUERY_BY_REGION_NO_WAIT        0x8E16
//#define GL_BUFFER_ACCESS_FLAGS            0x911F
//#define GL_BUFFER_MAP_LENGTH              0x9120
//#define GL_BUFFER_MAP_OFFSET              0x9121
#define GL_DEPTH_COMPONENT32F             0x8CAC
//#define GL_DEPTH32F_STENCIL8              0x8CAD
//#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
//#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
//#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
//#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
//#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
//#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
//#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
//#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
//#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
//#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
//#define GL_FRAMEBUFFER_DEFAULT            0x8218
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
//#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
//#define GL_MAX_RENDERBUFFER_SIZE          0x84E8
//#define GL_DEPTH_STENCIL                  0x84F9
//#define GL_UNSIGNED_INT_24_8              0x84FA
//#define GL_DEPTH24_STENCIL8               0x88F0
//#define GL_TEXTURE_STENCIL_SIZE           0x88F1
//#define GL_TEXTURE_RED_TYPE               0x8C10
//#define GL_TEXTURE_GREEN_TYPE             0x8C11
//#define GL_TEXTURE_BLUE_TYPE              0x8C12
//#define GL_TEXTURE_ALPHA_TYPE             0x8C13
//#define GL_TEXTURE_DEPTH_TYPE             0x8C16
//#define GL_UNSIGNED_NORMALIZED            0x8C17
#define GL_FRAMEBUFFER_BINDING            0x8CA6
//#define GL_DRAW_FRAMEBUFFER_BINDING       0x8CA6
//#define GL_RENDERBUFFER_BINDING           0x8CA7
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
//#define GL_READ_FRAMEBUFFER_BINDING       0x8CAA
//#define GL_RENDERBUFFER_SAMPLES           0x8CAB
//#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
//#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
//#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
//#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
//#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
//#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
//#define GL_COLOR_ATTACHMENT1              0x8CE1
//#define GL_COLOR_ATTACHMENT2              0x8CE2
//#define GL_COLOR_ATTACHMENT3              0x8CE3
//#define GL_COLOR_ATTACHMENT4              0x8CE4
//#define GL_COLOR_ATTACHMENT5              0x8CE5
//#define GL_COLOR_ATTACHMENT6              0x8CE6
//#define GL_COLOR_ATTACHMENT7              0x8CE7
//#define GL_COLOR_ATTACHMENT8              0x8CE8
//#define GL_COLOR_ATTACHMENT9              0x8CE9
//#define GL_COLOR_ATTACHMENT10             0x8CEA
//#define GL_COLOR_ATTACHMENT11             0x8CEB
//#define GL_COLOR_ATTACHMENT12             0x8CEC
//#define GL_COLOR_ATTACHMENT13             0x8CED
//#define GL_COLOR_ATTACHMENT14             0x8CEE
//#define GL_COLOR_ATTACHMENT15             0x8CEF
//#define GL_COLOR_ATTACHMENT16             0x8CF0
//#define GL_COLOR_ATTACHMENT17             0x8CF1
//#define GL_COLOR_ATTACHMENT18             0x8CF2
//#define GL_COLOR_ATTACHMENT19             0x8CF3
//#define GL_COLOR_ATTACHMENT20             0x8CF4
//#define GL_COLOR_ATTACHMENT21             0x8CF5
//#define GL_COLOR_ATTACHMENT22             0x8CF6
//#define GL_COLOR_ATTACHMENT23             0x8CF7
//#define GL_COLOR_ATTACHMENT24             0x8CF8
//#define GL_COLOR_ATTACHMENT25             0x8CF9
//#define GL_COLOR_ATTACHMENT26             0x8CFA
//#define GL_COLOR_ATTACHMENT27             0x8CFB
//#define GL_COLOR_ATTACHMENT28             0x8CFC
//#define GL_COLOR_ATTACHMENT29             0x8CFD
//#define GL_COLOR_ATTACHMENT30             0x8CFE
//#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_DEPTH_ATTACHMENT               0x8D00
//#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER                    0x8D40
//#define GL_RENDERBUFFER                   0x8D41
//#define GL_RENDERBUFFER_WIDTH             0x8D42
//#define GL_RENDERBUFFER_HEIGHT            0x8D43
//#define GL_RENDERBUFFER_INTERNAL_FORMAT   0x8D44
//#define GL_STENCIL_INDEX1                 0x8D46
//#define GL_STENCIL_INDEX4                 0x8D47
//#define GL_STENCIL_INDEX8                 0x8D48
//#define GL_STENCIL_INDEX16                0x8D49
//#define GL_RENDERBUFFER_RED_SIZE          0x8D50
//#define GL_RENDERBUFFER_GREEN_SIZE        0x8D51
//#define GL_RENDERBUFFER_BLUE_SIZE         0x8D52
//#define GL_RENDERBUFFER_ALPHA_SIZE        0x8D53
//#define GL_RENDERBUFFER_DEPTH_SIZE        0x8D54
//#define GL_RENDERBUFFER_STENCIL_SIZE      0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
//#define GL_MAX_SAMPLES                    0x8D57
//#define GL_INDEX                          0x8222
//#define GL_TEXTURE_LUMINANCE_TYPE         0x8C14
//#define GL_TEXTURE_INTENSITY_TYPE         0x8C15
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_HALF_FLOAT                     0x140B
//#define GL_MAP_READ_BIT                   0x0001
//#define GL_MAP_WRITE_BIT                  0x0002
//#define GL_MAP_INVALIDATE_RANGE_BIT       0x0004
//#define GL_MAP_INVALIDATE_BUFFER_BIT      0x0008
//#define GL_MAP_FLUSH_EXPLICIT_BIT         0x0010
//#define GL_MAP_UNSYNCHRONIZED_BIT         0x0020
//#define GL_COMPRESSED_RED_RGTC1           0x8DBB
//#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
//#define GL_COMPRESSED_RG_RGTC2            0x8DBD
//#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE
//#define GL_RG                             0x8227
//#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
//#define GL_R16                            0x822A
//#define GL_RG8                            0x822B
//#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
//#define GL_RG16F                          0x822F
//#define GL_RG32F                          0x8230
//#define GL_R8I                            0x8231
//#define GL_R8UI                           0x8232
//#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
//#define GL_R32I                           0x8235
//#define GL_R32UI                          0x8236
//#define GL_RG8I                           0x8237
//#define GL_RG8UI                          0x8238
//#define GL_RG16I                          0x8239
//#define GL_RG16UI                         0x823A
//#define GL_RG32I                          0x823B
//#define GL_RG32UI                         0x823C
//#define GL_VERTEX_ARRAY_BINDING           0x85B5
//#define GL_CLAMP_VERTEX_COLOR             0x891A
//#define GL_CLAMP_FRAGMENT_COLOR           0x891B
//#define GL_ALPHA_INTEGER                  0x8D97
#if !_AE_APPLE_
// OpenGL Shader Functions
GLuint ( *glCreateProgram ) () = nullptr;
void ( *glAttachShader ) ( GLuint program, GLuint shader ) = nullptr;
void ( *glLinkProgram ) ( GLuint program ) = nullptr;
void ( *glGetProgramiv ) ( GLuint program, GLenum pname, GLint *params ) = nullptr;
void ( *glGetProgramInfoLog ) ( GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog ) = nullptr;
void ( *glGetActiveAttrib ) ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) = nullptr;
GLint (*glGetAttribLocation) ( GLuint program, const GLchar *name ) = nullptr;
void (*glGetActiveUniform) ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name );
GLint (*glGetUniformLocation) ( GLuint program, const GLchar *name ) = nullptr;
void (*glDeleteShader) ( GLuint shader ) = nullptr;
void ( *glDeleteProgram) ( GLuint program ) = nullptr;
void ( *glUseProgram) ( GLuint program ) = nullptr;
void ( *glBlendFuncSeparate ) ( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha ) = nullptr;
GLuint( *glCreateShader) ( GLenum type ) = nullptr;
void (*glShaderSource) ( GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length ) = nullptr;
void (*glCompileShader)( GLuint shader ) = nullptr;
void ( *glGetShaderiv)( GLuint shader, GLenum pname, GLint *params );
void ( *glGetShaderInfoLog)( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog ) = nullptr;
void ( *glActiveTexture) ( GLenum texture ) = nullptr;
void ( *glUniform1i ) ( GLint location, GLint v0 ) = nullptr;
void ( *glUniform1fv ) ( GLint location, GLsizei count, const GLfloat *value ) = nullptr;
void ( *glUniform2fv ) ( GLint location, GLsizei count, const GLfloat *value ) = nullptr;
void ( *glUniform3fv ) ( GLint location, GLsizei count, const GLfloat *value ) = nullptr;
void ( *glUniform4fv ) ( GLint location, GLsizei count, const GLfloat *value ) = nullptr;
void ( *glUniformMatrix4fv ) ( GLint location, GLsizei count, GLboolean transpose,  const GLfloat *value ) = nullptr;
// OpenGL Texture Functions
void ( *glGenerateMipmap ) ( GLenum target ) = nullptr;
void ( *glBindFramebuffer ) ( GLenum target, GLuint framebuffer ) = nullptr;
void ( *glFramebufferTexture2D ) ( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) = nullptr;
void ( *glGenFramebuffers ) ( GLsizei n, GLuint *framebuffers ) = nullptr;
void ( *glDeleteFramebuffers ) ( GLsizei n, const GLuint *framebuffers ) = nullptr;
GLenum ( *glCheckFramebufferStatus ) ( GLenum target ) = nullptr;
void ( *glDrawBuffers ) ( GLsizei n, const GLenum *bufs ) = nullptr;
#endif

// Helpers
#define AE_CHECK_GL_ERROR() do { if ( GLenum err = glGetError() ) { AE_FAIL_MSG( "GL Error: #", err ); } } while ( 0 )

namespace AE_NAMESPACE {

void CheckFramebufferComplete( GLuint framebuffer )
{
  GLenum fboStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
  if ( fboStatus != GL_FRAMEBUFFER_COMPLETE )
  {
    const char* errStr = "unknown";
    switch ( fboStatus )
    {
      case GL_FRAMEBUFFER_UNDEFINED:
        errStr = "GL_FRAMEBUFFER_UNDEFINED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
        errStr = "GL_FRAMEBUFFER_UNSUPPORTED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;
#endif
      default:
        break;
    }
    AE_FAIL_MSG( "GL FBO Error: (#) #", fboStatus, errStr );
  }
}

#if AE_GL_DEBUG_MODE
void OpenGLDebugCallback( GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam )
{
  //std::cout << "---------------------opengl-callback-start------------" << std::endl;
  //std::cout << "message: " << message << std::endl;
  //std::cout << "type: ";
  //switch ( type )
  //{
  //  case GL_DEBUG_TYPE_ERROR:
  //    std::cout << "ERROR";
  //    break;
  //  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
  //    std::cout << "DEPRECATED_BEHAVIOR";
  //    break;
  //  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
  //    std::cout << "UNDEFINED_BEHAVIOR";
  //    break;
  //  case GL_DEBUG_TYPE_PORTABILITY:
  //    std::cout << "PORTABILITY";
  //    break;
  //  case GL_DEBUG_TYPE_PERFORMANCE:
  //    std::cout << "PERFORMANCE";
  //    break;
  //  case GL_DEBUG_TYPE_OTHER:
  //    std::cout << "OTHER";
  //    break;
  //}
  //std::cout << std::endl;

  //std::cout << "id: " << id << std::endl;
  //std::cout << "severity: ";
  switch ( severity )
  {
    case GL_DEBUG_SEVERITY_LOW:
      //std::cout << "LOW";
      //AE_INFO( message );
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      //std::cout << "MEDIUM";
      AE_WARN( message );
      break;
    case GL_DEBUG_SEVERITY_HIGH:
      //std::cout << "HIGH";
      AE_ERR( message );
      break;
  }
  //std::cout << std::endl;
  //std::cout << "---------------------opengl-callback-end--------------" << std::endl;

  if ( severity == GL_DEBUG_SEVERITY_HIGH )
  {
    AE_FAIL();
  }
}
#endif

const uint32_t _kMaxFrameBufferAttachments = 16;

//------------------------------------------------------------------------------
// ae::UniformList member functions
//------------------------------------------------------------------------------
void UniformList::Set( const char* name, float value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 1;
  uniform.value.data[ 0 ] = value;
}

void UniformList::Set( const char* name, Vec2 value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 2;
  uniform.value.data[ 0 ] = value.x;
  uniform.value.data[ 1 ] = value.y;
}

void UniformList::Set( const char* name, Vec3 value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 3;
  uniform.value.data[ 0 ] = value.x;
  uniform.value.data[ 1 ] = value.y;
  uniform.value.data[ 2 ] = value.z;
}

void UniformList::Set( const char* name, Vec4 value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 4;
  uniform.value.data[ 0 ] = value.x;
  uniform.value.data[ 1 ] = value.y;
  uniform.value.data[ 2 ] = value.z;
  uniform.value.data[ 3 ] = value.w;
}

void UniformList::Set( const char* name, const Matrix4& value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 16;
  uniform.value = value;
}

void UniformList::Set( const char* name, const Texture* tex )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.sampler = tex->GetTexture();
  uniform.target = tex->GetTarget();
}

const UniformList::Value* UniformList::Get( const char* name ) const
{
  return m_uniforms.TryGet( name );
}

//------------------------------------------------------------------------------
// ae::Shader member functions
//------------------------------------------------------------------------------
Shader::Shader()
{
  m_fragmentShader = 0;
  m_vertexShader = 0;
  m_program = 0;

  m_blending = false;
  m_blendingPremul = false;
  m_depthTest = false;
  m_depthWrite = false;
  m_culling = Culling::None;
  m_wireframe = false;
}

Shader::~Shader()
{
  Destroy();
}

void Shader::Initialize( const char* vertexStr, const char* fragStr, const char* const* defines, int32_t defineCount )
{
  Destroy();
  AE_ASSERT( !m_program );

  m_program = glCreateProgram();

  m_vertexShader = m_LoadShader( vertexStr, Type::Vertex, defines, defineCount );
  m_fragmentShader = m_LoadShader( fragStr, Type::Fragment, defines, defineCount );

  if ( !m_vertexShader )
  {
    AE_LOG( "Failed to load vertex shader! #", vertexStr );
  }
  if ( !m_fragmentShader )
  {
    AE_LOG( "Failed to load fragment shader! #", fragStr );
  }

  if ( !m_vertexShader || !m_fragmentShader )
  {
    AE_FAIL();
  }

  glAttachShader( m_program, m_vertexShader );
  glAttachShader( m_program, m_fragmentShader );

  glLinkProgram( m_program );

  // immediate reflection of shader can be delayed by compiler and optimizer and can stll
  GLint status;
  glGetProgramiv( m_program, GL_LINK_STATUS, &status );
  if ( status == GL_FALSE )
  {
    GLint logLength = 0;
    glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &logLength );

    char* log = nullptr;
    if ( logLength > 0 )
    {
      log = new char[ logLength ];
      glGetProgramInfoLog( m_program, logLength, NULL, (GLchar*)log );
    }

    if ( log )
    {
      AE_FAIL_MSG( log );
      delete[] log;
    }
    else
    {
      AE_FAIL();
    }
    Destroy();
  }

  GLint attribCount = 0;
  glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTES, &attribCount );
  AE_ASSERT( 0 < attribCount && attribCount <= _kMaxShaderAttributeCount );
  GLint maxLen = 0;
  glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen );
  AE_ASSERT( 0 < maxLen && maxLen <= _kMaxShaderAttributeNameLength );
  for ( int32_t i = 0; i < attribCount; i++ )
  {
    Attribute* attribute = &m_attributes.Append( Attribute() );

    GLsizei length;
    GLint size;
    glGetActiveAttrib( m_program, i, _kMaxShaderAttributeNameLength, &length, &size, (GLenum*)&attribute->type, (GLchar*)attribute->name );

    attribute->location = glGetAttribLocation( m_program, attribute->name );
    AE_ASSERT( attribute->location != -1 );
  }

  GLint uniformCount = 0;
  maxLen = 0;
  glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &uniformCount );
  glGetProgramiv( m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );
  AE_ASSERT( maxLen <= (GLint)Str32::MaxLength() ); // @TODO: Read from Uniform

  for ( int32_t i = 0; i < uniformCount; i++ )
  {
    Uniform uniform;

    GLint size = 0;
    char name[ Str32::MaxLength() ]; // @TODO: Read from Uniform
    glGetActiveUniform( m_program, i, sizeof( name ), nullptr, &size, (GLenum*)&uniform.type, (GLchar*)name );
    AE_ASSERT( size == 1 );

    switch ( uniform.type )
    {
      case GL_SAMPLER_2D:
      case GL_SAMPLER_3D:
      case GL_FLOAT:
      case GL_FLOAT_VEC2:
      case GL_FLOAT_VEC3:
      case GL_FLOAT_VEC4:
      case GL_FLOAT_MAT4:
        break;
      default:
        AE_FAIL_MSG( "Unsupported uniform '#' type #", name, uniform.type );
        break;
    }

    uniform.name = name;
    uniform.location = glGetUniformLocation( m_program, name );
    AE_ASSERT( uniform.location != -1 );

    m_uniforms.Set( name, uniform );
  }

  AE_CHECK_GL_ERROR();
}

void Shader::Destroy()
{
  AE_CHECK_GL_ERROR();

  m_attributes.Clear();

  if ( m_fragmentShader != 0 )
  {
    glDeleteShader( m_fragmentShader );
    m_fragmentShader = 0;
  }

  if ( m_vertexShader != 0 )
  {
    glDeleteShader( m_vertexShader );
    m_vertexShader = 0;
  }

  if ( m_program != 0 )
  {
    glDeleteProgram( m_program );
    m_program = 0;
  }

  AE_CHECK_GL_ERROR();
}

void Shader::Activate( const UniformList& uniforms ) const
{
  AE_CHECK_GL_ERROR();

  // This is really context state shadow, and that should be able to override
  // so reverseZ for example can be set without the shader knowing about that.

  // Blending
  if ( m_blending || m_blendingPremul )
  {
    glEnable( GL_BLEND );

    // TODO: need other modes like Add, Min, Max - switch to enum then
    if ( m_blendingPremul )
    {
      // Colors coming out of shader already have alpha multiplied in.
      glBlendFuncSeparate( GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
                 GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    else
    {
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
  }
  else
  {
    glDisable( GL_BLEND );
  }

  // Depth write
  glDepthMask( m_depthWrite ? GL_TRUE : GL_FALSE );

  // Depth test
  if ( m_depthTest )
  {
    glDepthFunc( gReverseZ ? GL_GEQUAL : GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
  }
  else
  {
    glDisable( GL_DEPTH_TEST );
  }

  // Culling
  if ( m_culling == Culling::None )
  {
    glDisable( GL_CULL_FACE );
  }
  else
  {
    // TODO: det(modelToWorld) < 0, then CCW/CW flips from inversion in transform.
    glEnable( GL_CULL_FACE );
    glFrontFace( ( m_culling == Culling::ClockwiseFront ) ? GL_CW : GL_CCW );
  }

  // Wireframe
#if _AE_IOS_
  AE_ASSERT_MSG( !m_wireframe, "Wireframe mode not supported on iOS" );
#else
  glPolygonMode( GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL );
#endif

  // Now setup the shader
  glUseProgram( m_program );

  // Set shader uniforms
  bool missingUniforms = false;
  uint32_t textureIndex = 0;
  for ( uint32_t i = 0; i < m_uniforms.Length(); i++ )
  {
    const char* uniformVarName = m_uniforms.GetKey( i ).c_str();
    const Uniform* uniformVar = &m_uniforms.GetValue( i );
    const UniformList::Value* uniformValue = uniforms.Get( uniformVarName );

    // Start validation
    if ( !uniformValue )
    {
      AE_WARN( "Shader uniform '#' value is not set", uniformVarName );
      missingUniforms = true;
      continue;
    }
    uint32_t typeSize = 0;
    switch ( uniformVar->type )
    {
      case GL_SAMPLER_2D:
        typeSize = 0;
        break;
      case GL_SAMPLER_3D:
        typeSize = 0;
        break;
      case GL_FLOAT:
        typeSize = 1;
        break;
      case GL_FLOAT_VEC2:
        typeSize = 2;
        break;
      case GL_FLOAT_VEC3:
        typeSize = 3;
        break;
      case GL_FLOAT_VEC4:
        typeSize = 4;
        break;
      case GL_FLOAT_MAT4:
        typeSize = 16;
        break;
      default:
        AE_FAIL_MSG( "Unsupported uniform '#' type #", uniformVarName, uniformVar->type );
        break;
    }
    AE_ASSERT_MSG( uniformValue->size == typeSize, "Uniform size mismatch '#' type:# var:# param:#", uniformVarName, uniformVar->type, typeSize, uniformValue->size );
    // End validation

    if ( uniformVar->type == GL_SAMPLER_2D )
    {
      AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid #", uniformVarName, uniformValue->sampler );
      glActiveTexture( GL_TEXTURE0 + textureIndex );
      glBindTexture( uniformValue->target, uniformValue->sampler );
      glUniform1i( uniformVar->location, textureIndex );
      textureIndex++;
    }
    else if ( uniformVar->type == GL_SAMPLER_3D )
    {
      AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid #", uniformVarName, uniformValue->sampler );
      glActiveTexture( GL_TEXTURE0 + textureIndex );
      glBindTexture( GL_TEXTURE_3D, uniformValue->sampler );
      glUniform1i( uniformVar->location, textureIndex );
      textureIndex++;
    }
    else if ( uniformVar->type == GL_FLOAT )
    {
      glUniform1fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_VEC2 )
    {
      glUniform2fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_VEC3 )
    {
      glUniform3fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_VEC4 )
    {
      glUniform4fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_MAT4 )
    {
#if _AE_EMSCRIPTEN_
      // WebGL/Emscripten doesn't support glUniformMatrix4fv auto-transpose
      aeFloat4x4 transposedTransform = uniformValue->value.GetTransposeCopy();
      glUniformMatrix4fv( uniformVar->location, 1, GL_FALSE, transposedTransform.data );
#else
      glUniformMatrix4fv( uniformVar->location, 1, GL_TRUE, uniformValue->value.data );
#endif
    }
    else
    {
      AE_ASSERT_MSG( false, "Invalid uniform type '#': #", uniformVarName, uniformVar->type );
    }

    AE_CHECK_GL_ERROR();
  }

  AE_ASSERT_MSG( !missingUniforms, "Missing shader uniform parameters" );
}

const ae::Shader::Attribute* Shader::GetAttributeByIndex( uint32_t index ) const
{
  return &m_attributes[ index ];
}

int Shader::m_LoadShader( const char* shaderStr, Type type, const char* const* defines, int32_t defineCount )
{
  GLenum glType = -1;
  if ( type == Type::Vertex )
  {
    glType = GL_VERTEX_SHADER;
  }
  if ( type == Type::Fragment )
  {
    glType = GL_FRAGMENT_SHADER;
  }

  const uint32_t kPrependMax = 16;
  uint32_t sourceCount = 0;
  const char* shaderSource[ kPrependMax + _kMaxShaderDefines * 2 + 1 ]; // x2 max defines to make room for newlines. Plus one for actual shader.

  // Version
#if _AE_IOS_
  shaderSource[ sourceCount++ ] = "#version 300 es\n";
  shaderSource[ sourceCount++ ] = "precision highp float;\n";
#elif _AE_EMSCRIPTEN_
  // No version specified
  shaderSource[ sourceCount++ ] = "precision highp float;\n";
#else
  if ( gGL41 )
  {
    shaderSource[ sourceCount++ ] = "#version 410 core\n";
  }
  else
  {
    shaderSource[ sourceCount++ ] = "#version 330 core\n";
  }

  // No default precision specified
#endif

  // Input/output
#if _AE_EMSCRIPTEN_
  shaderSource[ sourceCount++ ] = "#define AE_COLOR gl_FragColor\n";
  shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture2d\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
  if ( type == Type::Vertex )
  {
    shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP attribute highp\n";
    shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP varying highp\n";
  }
  else if ( type == Type::Fragment )
  {
    shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP varying highp\n";
    shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
  }
#else
  shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM uniform\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform\n";
  shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP in\n";
  shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP out\n";
  if ( type == Type::Fragment )
  {
    shaderSource[ sourceCount++ ] = "out vec4 AE_COLOR;\n";
  }
#endif

  AE_ASSERT( sourceCount <= kPrependMax );

  for ( int32_t i = 0; i < defineCount; i++ )
  {
    shaderSource[ sourceCount ] = defines[ i ];
    sourceCount++;
    shaderSource[ sourceCount ] = "\n";
    sourceCount++;
  }

  shaderSource[ sourceCount ] = shaderStr;
  sourceCount++;

  GLuint shader = glCreateShader( glType );
  glShaderSource( shader, sourceCount, shaderSource, nullptr );
  glCompileShader( shader );

  GLint status;
  glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
  if ( status == GL_FALSE )
  {
    GLint logLength;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );

    if ( logLength > 0 )
    {
      unsigned char* log = new unsigned char[ logLength ];
      glGetShaderInfoLog( shader, logLength, NULL, (GLchar*)log );
      const char* typeStr = ( type == Type::Vertex ? "vertex" : "fragment" );
      AE_LOG( "Error compiling # shader #", typeStr, log );
      delete[] log;
    }

    return 0;
  }

  AE_CHECK_GL_ERROR();
  return shader;
}

//------------------------------------------------------------------------------
// ae::Texture member functions
//------------------------------------------------------------------------------
Texture::~Texture()
{
  // @NOTE: Only ae::Texture should call it's virtual Destroy() so it only runs once
  Destroy();
}

void Texture::Initialize( uint32_t target )
{
  // @NOTE: To avoid undoing any initialization logic only ae::Texture should
  //        call Destroy() on initialize, and inherited Initialize()'s should
  //        always call Base::Initialize() before any other logic.
  Destroy();

  m_target = target;

  glGenTextures( 1, &m_texture );
  AE_ASSERT( m_texture );
}

void Texture::Destroy()
{
  if ( m_texture )
  {
    glDeleteTextures( 1, &m_texture );
  }

  m_texture = 0;
  m_target = 0;
}

//------------------------------------------------------------------------------
// ae::Texture2D member functions
//------------------------------------------------------------------------------
void Texture2D::Initialize( const void* data, uint32_t width, uint32_t height, Texture::Format format, Texture::Type type, Texture::Filter filter, Wrap wrap, bool autoGenerateMipmaps )
{
  Texture::Initialize( GL_TEXTURE_2D );

  m_width = width;
  m_height = height;

  glBindTexture( GetTarget(), GetTexture() );

  if (autoGenerateMipmaps)
  {
	  glTexParameteri( GetTarget(), GL_TEXTURE_MIN_FILTER, ( filter == Filter::Nearest ) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR );
	  glTexParameteri( GetTarget(), GL_TEXTURE_MAG_FILTER, ( filter == Filter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  }
  else
  {
	  glTexParameteri( GetTarget(), GL_TEXTURE_MIN_FILTER, ( filter == Filter::Nearest ) ? GL_NEAREST : GL_LINEAR );
	  glTexParameteri( GetTarget(), GL_TEXTURE_MAG_FILTER, ( filter == Filter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  }
	
  glTexParameteri( GetTarget(), GL_TEXTURE_WRAP_S, ( wrap == Wrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  glTexParameteri( GetTarget(), GL_TEXTURE_WRAP_T, ( wrap == Wrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );

  // this is the type of data passed in, conflating with internal format type
  GLenum glType = 0;
  switch ( type )
  {
    case Type::Uint8:
      glType = GL_UNSIGNED_BYTE;
      break;
	  case Type::Uint16:
	    glType = GL_UNSIGNED_SHORT;
	    break;
    case Type::HalfFloat:
      glType = GL_HALF_FLOAT;
      break;
    case Type::Float:
      glType = GL_FLOAT;
      break;
    default:
      AE_FAIL_MSG( "Invalid texture type #", (int)type );
      return;
  }

  GLint glInternalFormat = 0;
  GLenum glFormat = 0;
  GLint unpackAlignment = 0;
  switch ( format )
  {
    // TODO: need D32F_S8 format
    case Format::Depth32F:
      glInternalFormat = GL_DEPTH_COMPONENT32F;
      glFormat = GL_DEPTH_COMPONENT;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;

    case Format::R8:
    case Format::R16_UNORM:
    case Format::R16F:
    case Format::R32F:
      switch(format)
      {
        case Format::R8:
          glInternalFormat = GL_R8;
          break;
        case Format::R16_UNORM:
          glInternalFormat = GL_R16UI;
          assert(glType == GL_UNSIGNED_SHORT);
          break; // only on macOS
        case Format::R16F:
          glInternalFormat = GL_R16F;
          break;
        case Format::R32F:
          glInternalFormat = GL_R32F;
          break;
        default: assert(false);
      }

      glFormat = GL_RED;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
		  
#if _AE_OSX_
	  // RedGreen, TODO: extend to other ES but WebGL1 left those constants out IIRC
	  case Format::RG8:
	  case Format::RG16F:
	  case Format::RG32F:
  		switch(format)
  		{
  			case Format::RG8: glInternalFormat = GL_RG8; break;
  			case Format::RG16F: glInternalFormat = GL_RG16F; break;
  			case Format::RG32F: glInternalFormat = GL_RG32F; break;
  			default: assert(false);
  		}
  			  
  		glFormat = GL_RG;
  		unpackAlignment = 1;
  		m_hasAlpha = false;
  		break;
#endif
  	case Format::RGB8:
  	case Format::RGB16F:
    case Format::RGB32F:
  	  switch(format)
  	  {
  	    case Format::RGB8: glInternalFormat = GL_RGB8; break;
  	    case Format::RGB16F: glInternalFormat = GL_RGB16F; break;
  	    case Format::RGB32F: glInternalFormat = GL_RGB32F; break;
  		  default: assert(false);
  	  }
      glFormat = GL_RGB;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;

    case Format::RGBA8:
	  case Format::RGBA16F:
	  case Format::RGBA32F:
  	  switch(format)
  	  {
      	case Format::RGBA8: glInternalFormat = GL_RGBA8; break;
      	case Format::RGBA16F: glInternalFormat = GL_RGBA16F; break;
      	case Format::RGBA32F: glInternalFormat = GL_RGBA32F; break;
      	default: assert(false);
  	  }
      glFormat = GL_RGBA;
      unpackAlignment = 1;
      m_hasAlpha = true;
      break;
		  
      // TODO: fix these constants, but they differ on ES2/3 and GL
      // WebGL1 they require loading an extension (if present) to get at the constants.    
    case Format::RGB8_SRGB:
	  // ignore type
      glInternalFormat = GL_SRGB8;
      glFormat = GL_RGB;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
    case Format::RGBA8_SRGB:
	  // ignore type
      glInternalFormat = GL_SRGB8_ALPHA8;
      glFormat = GL_RGBA;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
    default:
      AE_FAIL_MSG( "Invalid texture format #", (int)format );
      return;
  }

  if ( data )
  {
    glPixelStorei( GL_UNPACK_ALIGNMENT, unpackAlignment );
  }

    // count the mip levels
	int w = width;
	int h = height;
	
	int numberOfMipmaps = 1;
	if ( autoGenerateMipmaps )
	{
		while ( w > 1 || h > 1 )
		{
		  numberOfMipmaps++;
		  w = (w+1) / 2;
		  h = (h+1) / 2;
		}
	}
	
	// allocate mip levels
	// texStorage is GL4.2, so not on macOS.  ES emulates the call internaly.
#define USE_TEXSTORAGE 0
#if USE_TEXSTORAGE
	// TODO: enable glTexStorage on all platforms, this is in gl3ext.h for GL
	// It allocates a full mip chain all at once, and can handle formats glTexImage2D cannot
	// for compressed textures.
	glTexStorage2D( GetTarget(), numberOfMipmaps, glInternalFormat, width, height );
#else
	w = width;
	h = height;
	
	for ( int i = 0; i < numberOfMipmaps; ++i )
	{
	  glTexImage2D( GetTarget(), i, glInternalFormat, w, h, 0, glFormat, glType, NULL );
	  w = (w+1) / 2;
	  h = (h+1) / 2;
	}
#endif
	
  if ( data != nullptr )
  {
	  // upload the first mipmap
	  glTexSubImage2D( GetTarget(), 0, 0,0, width, height, glFormat, glType, data );

	  // autogen only works for uncompressed textures
	  // Also need to know if format is filterable on platform, or this will fail (f.e. R32F)
	  if ( numberOfMipmaps > 1 && autoGenerateMipmaps )
	  {
		  glGenerateMipmap( GetTarget() );
	  }
  }
	
  AE_CHECK_GL_ERROR();
}

void Texture2D::Initialize( const char* file, Filter filter, Wrap wrap, bool autoGenerateMipmaps, bool isSRGB )
{
#if STBI_INCLUDE_STB_IMAGE_H
  uint32_t fileSize = aeVfs::GetSize( file );
  AE_ASSERT_MSG( fileSize, "Could not load #", file );
  
  uint8_t* fileBuffer = (uint8_t*)malloc( fileSize );
  aeVfs::Read( file, fileBuffer, fileSize );

  int32_t width = 0;
  int32_t height = 0;
  int32_t channels = 0;
  stbi_set_flip_vertically_on_load( 1 );
#if _AE_IOS_
  stbi_convert_iphone_png_to_rgb( 1 );
#endif
  bool is16BitImage = stbi_is_16_bit_from_memory( fileBuffer, fileSize );

  uint8_t* image;
  if (is16BitImage)
  {
     image = (uint8_t*)stbi_load_16_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  }
  else
  {
    image = stbi_load_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  }
  AE_ASSERT( image );

  Format format;
  auto type = aeTextureType::Uint8;
  switch ( channels )
  {
    case STBI_grey:
  		format = Format::R8;
  		  
  		// for now only support R16Unorm
  		if (is16BitImage)
  		{
  			format = Format::R16_UNORM;
  			type = aeTextureType::Uint16;
  		}
  	  break;
    case STBI_grey_alpha:
      AE_FAIL();
      break;
    case STBI_rgb:
      format = isSRGB ? Format::RGB8_SRGB : Format::RGB8;
      break;
    case STBI_rgb_alpha:
      format = isSRGB ? Format::RGBA8_SRGB : Format::RGBA8;
      break;
  }
  
  Initialize( image, width, height, format, type, filter, wrap, autoGenerateMipmaps );
  
  stbi_image_free( image );
  free( fileBuffer );
#endif
}

void Texture2D::Destroy()
{
  m_width = 0;
  m_height = 0;
  m_hasAlpha = false;

  Texture::Destroy();
}

//------------------------------------------------------------------------------
// ae::RenderTarget member functions
//------------------------------------------------------------------------------
RenderTarget::~RenderTarget()
{
  Destroy();
}

void RenderTarget::Initialize( uint32_t width, uint32_t height )
{
  Destroy();

  AE_ASSERT( m_fbo == 0 );

  AE_ASSERT( width != 0 );
  AE_ASSERT( height != 0 );

  m_width = width;
  m_height = height;

  glGenFramebuffers( 1, &m_fbo );
  AE_ASSERT( m_fbo );
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  AE_CHECK_GL_ERROR();

  //Vertex quadVerts[] =
  //{
  //  { aeQuadVertPos[ 0 ], aeQuadVertUvs[ 0 ] },
  //  { aeQuadVertPos[ 1 ], aeQuadVertUvs[ 1 ] },
  //  { aeQuadVertPos[ 2 ], aeQuadVertUvs[ 2 ] },
  //  { aeQuadVertPos[ 3 ], aeQuadVertUvs[ 3 ] }
  //};
  //AE_STATIC_ASSERT( countof( quadVerts ) == aeQuadVertCount );
  //m_quad.Initialize( sizeof( Vertex ), sizeof( aeQuadIndex ), aeQuadVertCount, aeQuadIndexCount, aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
  //m_quad.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof( Vertex, pos ) );
  //m_quad.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof( Vertex, uv ) );
  //m_quad.SetVertices( quadVerts, aeQuadVertCount );
  //m_quad.SetIndices( aeQuadIndices, aeQuadIndexCount );
  //AE_CHECK_GL_ERROR();

  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_localToNdc;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_OUT_HIGHP vec2 v_uv;\
    void main()\
    {\
      v_uv = a_uv;\
      gl_Position = u_localToNdc * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    void main()\
    {\
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv );\
    }";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );

  AE_CHECK_GL_ERROR();
}

void RenderTarget::Destroy()
{
  m_shader.Destroy();
  //m_quad.Destroy(); // @TODO

  for ( uint32_t i = 0; i < m_targets.Length(); i++ )
  {
    m_targets[ i ]->Destroy();
    ae::Delete( m_targets[ i ] );
  }
  m_targets.Clear();

  m_depth.Destroy();

  if ( m_fbo )
  {
    glDeleteFramebuffers( 1, &m_fbo );
    m_fbo = 0;
  }

  m_width = 0;
  m_height = 0;
}

void RenderTarget::AddTexture( Texture::Filter filter, Texture::Wrap wrap )
{
  AE_ASSERT( m_targets.Length() < _kMaxFrameBufferAttachments );

  Texture2D* tex = ae::New< Texture2D >( AE_ALLOC_TAG_RENDER );
  tex->Initialize( nullptr, m_width, m_height, Texture::Format::RGBA16F, Texture::Type::HalfFloat, filter, wrap );

  GLenum attachement = GL_COLOR_ATTACHMENT0 + m_targets.Length();
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  glFramebufferTexture2D( GL_FRAMEBUFFER, attachement, tex->GetTarget(), tex->GetTexture(), 0 );

  m_targets.Append( tex );

  AE_CHECK_GL_ERROR();
}

void RenderTarget::AddDepth( Texture::Filter filter, Texture::Wrap wrap )
{
  AE_ASSERT_MSG( m_depth.GetTexture() == 0, "Render target already has a depth texture" );

  m_depth.Initialize( nullptr, m_width, m_height, Texture::Format::Depth32F, Texture::Type::Float, filter, wrap );
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth.GetTarget(), m_depth.GetTexture(), 0 );

  AE_CHECK_GL_ERROR();
}

void RenderTarget::Activate()
{
  CheckFramebufferComplete( m_fbo );
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_fbo );
  
  GLenum buffers[ _kMaxFrameBufferAttachments ];
  for ( uint32_t i = 0 ; i < countof(buffers); i++ )
  {
    buffers[ i ] = GL_COLOR_ATTACHMENT0 + i;
  }
  glDrawBuffers( m_targets.Length(), buffers );

  glViewport( 0, 0, GetWidth(), GetHeight() );
}

void RenderTarget::Clear( Color color )
{
  Activate();

  AE_CHECK_GL_ERROR();

  Vec3 clearColor = color.GetLinearRGB();
  glClearColor( clearColor.x, clearColor.y, clearColor.z, 1.0f );
  glClearDepth( gReverseZ ? 0.0f : 1.0f );

  glDepthMask( GL_TRUE );
  glDisable( GL_DEPTH_TEST );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  AE_CHECK_GL_ERROR();
}

void RenderTarget::Render( const Shader* shader, const UniformList& uniforms )
{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
  //m_quad.Render( shader, uniforms );
}

void RenderTarget::Render2D( uint32_t textureIndex, Rect ndc, float z )
{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );

  //aeUniformList uniforms;
  //uniforms.Set( "u_localToNdc", RenderTarget::GetQuadToNDCTransform( ndc, z ) );
  //uniforms.Set( "u_tex", GetTexture( textureIndex ) );
  //m_quad.Render( &m_shader, uniforms );
}

const Texture2D* RenderTarget::GetTexture( uint32_t index ) const
{
  return m_targets[ index ];
}

const Texture2D* RenderTarget::GetDepth() const
{
  return m_depth.GetTexture() ? &m_depth : nullptr;
}

uint32_t RenderTarget::GetWidth() const
{
  return m_width;
}

uint32_t RenderTarget::GetHeight() const
{
  return m_height;
}

Matrix4 RenderTarget::GetTargetPixelsToLocalTransform( uint32_t otherPixelWidth, uint32_t otherPixelHeight, Rect ndc ) const
{
  //Matrix4 windowToNDC = Matrix4::Translation( Vec3( -1.0f, -1.0f, 0.0f ) );
  //windowToNDC.Scale( Vec3( 2.0f / otherPixelWidth, 2.0f / otherPixelHeight, 1.0f ) );

  //Matrix4 ndcToQuad = RenderTarget::GetQuadToNDCTransform( ndc, 0.0f );
  //ndcToQuad.Invert();

  //Matrix4 quadToRender = Matrix4::Scaling( Vec3( m_width, m_height, 1.0f ) );
  //quadToRender.Translate( Vec3( 0.5f, 0.5f, 0.0f ) );

  //return ( quadToRender * ndcToQuad * windowToNDC );
  return {};
}

Rect RenderTarget::GetNDCFillRectForTarget( uint32_t otherWidth, uint32_t otherHeight ) const
{
  //float canvasAspect = m_width / (float)m_height;
  //float targetAspect = otherWidth / (float)otherHeight;
  //if ( canvasAspect >= targetAspect )
  //{
  //  // Fit width
  //  float height = targetAspect / canvasAspect;
  //  return Rect( -1.0f, -height, 2.0f, height * 2.0f );
  //}
  //else
  //{
  //  // Fit height
  //  float width = canvasAspect / targetAspect;
  //  return Rect( -width, -1.0f, width * 2.0f, 2.0f );
  //}
  return {};
}

Matrix4 RenderTarget::GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const
{
  //Matrix4 canvasToNdc = Matrix4::Translation( Vec3( -1.0f, -1.0f, 0.0f ) ) * Matrix4::Scaling( Vec3( 2.0f / GetWidth(), 2.0f / GetHeight(), 1.0f ) );
  //return ( worldToNdc.Inverse() * canvasToNdc * otherTargetToLocal );
  return {};
}

Matrix4 RenderTarget::GetQuadToNDCTransform( Rect ndc, float z )
{
  //Matrix4 localToNdc = Matrix4::Translation( Vec3( ndc.x, ndc.y, z ) );
  //localToNdc.Scale( Vec3( ndc.w, ndc.h, 1.0f ) );
  //localToNdc.Translate( Vec3( 0.5f, 0.5f, 0.0f ) );
  //return localToNdc;
  return {};
}

//------------------------------------------------------------------------------
// GraphicsDevice member functions
//------------------------------------------------------------------------------
GraphicsDevice::GraphicsDevice()
{
  m_window = nullptr;

  // OpenGL
  m_context = nullptr;
  m_defaultFbo = 0;
}

GraphicsDevice::~GraphicsDevice()
{
  Terminate();
}

#define LOAD_OPENGL_FN( _glfn )\
_glfn = (decltype(_glfn))wglGetProcAddress( #_glfn );\
AE_ASSERT_MSG( _glfn, "Failed to load OpenGL function '" #_glfn "'" );

void GraphicsDevice::Initialize( class Window* window )
{
  AE_ASSERT( window );
  AE_ASSERT_MSG( window->window, "Window must be initialized prior to GraphicsDevice initialization." );
  AE_ASSERT_MSG( !m_context, "GraphicsDevice already initialized" );

  m_window = window;
  window->graphicsDevice = this;

#if _AE_WINDOWS_
  // Create OpenGL context
  HWND hWnd = (HWND)m_window->window;
  AE_ASSERT_MSG( hWnd, "ae::Window must be initialized" );
  HDC hdc = GetDC( hWnd );
  AE_ASSERT_MSG( hdc, "Failed to Get the Window Device Context" );
  HGLRC hglrc = wglCreateContext( hdc );
  AE_ASSERT_MSG( hglrc, "Failed to create the OpenGL Rendering Context" );
  if ( !wglMakeCurrent( hdc, hglrc ) )
  {
    AE_FAIL_MSG( "Failed to make OpenGL Rendering Context current" );
  }
  m_context = hglrc;
#endif

  AE_CHECK_GL_ERROR();

#if !_AE_APPLE_
  // Shader functions
  LOAD_OPENGL_FN( glCreateProgram );
  LOAD_OPENGL_FN( glAttachShader );
  LOAD_OPENGL_FN( glLinkProgram );
  LOAD_OPENGL_FN( glGetProgramiv );
  LOAD_OPENGL_FN( glGetProgramInfoLog );
  LOAD_OPENGL_FN( glGetActiveAttrib );
  LOAD_OPENGL_FN( glGetAttribLocation );
  LOAD_OPENGL_FN( glGetActiveUniform );
  LOAD_OPENGL_FN( glGetUniformLocation );
  LOAD_OPENGL_FN( glDeleteShader );
  LOAD_OPENGL_FN( glDeleteProgram );
  LOAD_OPENGL_FN( glUseProgram );
  LOAD_OPENGL_FN( glBlendFuncSeparate );
  LOAD_OPENGL_FN( glCreateShader );
  LOAD_OPENGL_FN( glShaderSource );
  LOAD_OPENGL_FN( glCompileShader );
  LOAD_OPENGL_FN( glGetShaderiv );
  LOAD_OPENGL_FN( glGetShaderInfoLog );
  LOAD_OPENGL_FN( glActiveTexture );
  LOAD_OPENGL_FN( glUniform1i );
  LOAD_OPENGL_FN( glUniform1fv );
  LOAD_OPENGL_FN( glUniform2fv );
  LOAD_OPENGL_FN( glUniform3fv );
  LOAD_OPENGL_FN( glUniform4fv );
  LOAD_OPENGL_FN( glUniformMatrix4fv );
  // Texture functions
  LOAD_OPENGL_FN( glGenerateMipmap );
  LOAD_OPENGL_FN( glBindFramebuffer );
  LOAD_OPENGL_FN( glFramebufferTexture2D );
  LOAD_OPENGL_FN( glGenFramebuffers );
  LOAD_OPENGL_FN( glDeleteFramebuffers );
  LOAD_OPENGL_FN( glCheckFramebufferStatus );
  LOAD_OPENGL_FN( glDrawBuffers );
#endif

  AE_CHECK_GL_ERROR();

//  // TODO: needed on ES2/GL/WebGL1, but not on ES3/WebGL2
//#if !_AE_IOS_
//  AE_STATIC_ASSERT( GL_ARB_framebuffer_sRGB );
//#endif
//
//#if _AE_IOS_
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
//#else
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
//  if ( gGL41 )
//  {
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
//  }
//  else
//  {
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
//  }
//#endif
//
//  SDL_GL_SetAttribute( SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1 );
//
//#if AE_GL_DEBUG_MODE
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );
//#endif
//  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
//  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
//
//  m_context = SDL_GL_CreateContext( (SDL_Window*)m_window->window );
//  AE_ASSERT( m_context );
//  SDL_GL_MakeCurrent( (SDL_Window*)m_window->window, m_context );
//
//  SDL_GL_SetSwapInterval( 1 );
//
//#if _AE_WINDOWS_
//  glewExperimental = GL_TRUE;
//  GLenum err = glewInit();
//  glGetError(); // Glew currently has an issue which causes a GL_INVALID_ENUM on init
//  AE_ASSERT_MSG( err == GLEW_OK, "Could not initialize glew" );
//#endif

#if AE_GL_DEBUG_MODE
  glDebugMessageCallback( aeOpenGLDebugCallback, nullptr );
#endif

  glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFbo );

  AE_CHECK_GL_ERROR();

//#if _AE_WINDOWS_
//  // @TODO: Remove start
//  glShadeModel( GL_SMOOTH );							// Enable Smooth Shading
//  glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );				// Black Background
//  glClearDepth( 1.0f );									// Depth Buffer Setup
//  glEnable( GL_DEPTH_TEST );							// Enables Depth Testing
//  glDepthFunc( GL_LEQUAL );								// The Type Of Depth Testing To Do
//  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST ); // Really Nice Perspective Calculations
//  // @TODO: Remove end
//#endif

  m_HandleResize( m_window->GetWidth(), m_window->GetHeight() );

}

void GraphicsDevice::Terminate()
{
  if ( m_context )
  {
    //SDL_GL_DeleteContext( m_context );
    m_context = 0;
  }
}

void GraphicsDevice::Activate()
{
  AE_ASSERT( m_context );

  if ( m_window->GetWidth() != m_canvas.GetWidth() || m_window->GetHeight() != m_canvas.GetHeight() )
  {
    m_HandleResize( m_window->GetWidth(), m_window->GetHeight() );
  }
  m_canvas.Activate();

#if !_AE_IOS_
  // This is automatically enabled on opengl es3 and can't be turned off
  glEnable( GL_FRAMEBUFFER_SRGB );
#endif

//#if _AE_WINDOWS_
//  // @TODO: Remove start
//  glViewport( 0, 0, m_canvas.GetWidth(), m_canvas.GetHeight() ); // Reset The Current Viewport
//
//  glMatrixMode( GL_PROJECTION ); // Select The Projection Matrix
//  glLoadIdentity(); // Reset The Projection Matrix
//
//  // Calculate The Aspect Ratio Of The Window
//  float aspectRatio = m_canvas.GetWidth() / (float)m_canvas.GetHeight();
//  gluPerspective( 45.0f, aspectRatio, 0.1f, 100.0f );
//
//  glMatrixMode( GL_MODELVIEW ); // Select The Modelview Matrix
//  glLoadIdentity();
//
//  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// Clear Screen And Depth Buffer
//  glLoadIdentity();									// Reset The Current Modelview Matrix
//  glTranslatef( -1.5f, 0.0f, -6.0f );						// Move Left 1.5 Units And Into The Screen 6.0
//  glBegin( GL_TRIANGLES );								// Drawing Using Triangles
//  glVertex3f( 0.0f, 1.0f, 0.0f );					// Top
//  glVertex3f( -1.0f, -1.0f, 0.0f );					// Bottom Left
//  glVertex3f( 1.0f, -1.0f, 0.0f );					// Bottom Right
//  glEnd();											// Finished Drawing The Triangle
//  glTranslatef( 3.0f, 0.0f, 0.0f );						// Move Right 3 Units
//  glBegin( GL_QUADS );									// Draw A Quad
//  glVertex3f( -1.0f, 1.0f, 0.0f );					// Top Left
//  glVertex3f( 1.0f, 1.0f, 0.0f );					// Top Right
//  glVertex3f( 1.0f, -1.0f, 0.0f );					// Bottom Right
//  glVertex3f( -1.0f, -1.0f, 0.0f );					// Bottom Left
//  glEnd();											// Done Drawing The Quad
//  // @TODO: Remove end
//#endif
}

void GraphicsDevice::Clear( Color color )
{
  Activate();
  m_canvas.Clear( color );
}

void GraphicsDevice::Present()
{
  AE_ASSERT( m_context );
  AE_CHECK_GL_ERROR();

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_defaultFbo );
  glViewport( 0, 0, m_window->GetWidth(), m_window->GetHeight() );

  // Clear window target in case canvas doesn't fit exactly
  glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
  glClearDepth( 1.0f );

  glDepthMask( GL_TRUE );

  glDisable( GL_DEPTH_TEST );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  AE_CHECK_GL_ERROR();

  m_canvas.Render2D( 0, Rect( Vec2( -1.0f ), Vec2( 1.0f ) ), 0.5f );

  AE_CHECK_GL_ERROR();

#if _AE_WINDOWS_
  AE_ASSERT( m_window );
  HWND hWnd = (HWND)m_window->window;
  AE_ASSERT( hWnd );
  HDC hdc = GetDC( hWnd );
  SwapBuffers( hdc ); // Swap Buffers
#endif
}

float GraphicsDevice::GetAspectRatio() const
{
  if ( m_canvas.GetWidth() + m_canvas.GetHeight() == 0 )
  {
    return 0.0f;
  }
  else
  {
    return m_canvas.GetWidth() / (float)m_canvas.GetHeight();
  }
}

void GraphicsDevice::AddTextureBarrier()
{
//  // only GL has texture barrier for reading from previously written textures
//  // There are less draconian ways in desktop ES, and nothing in WebGL.
//#if _AE_WINDOWS_ || _AE_OSX_
//  glTextureBarrierNV();
//#endif
}

void GraphicsDevice::m_HandleResize( uint32_t width, uint32_t height )
{
  // @TODO: Allow user to pass in a canvas scaling factor / aspect ratio parameter
  m_canvas.Initialize( width, height );
  m_canvas.AddTexture( Texture::Filter::Nearest, Texture::Wrap::Clamp );
  m_canvas.AddDepth( Texture::Filter::Nearest, Texture::Wrap::Clamp );
}

} // AE_NAMESPACE end

#endif // AE_MAIN
#endif // AE_AETHER_H
