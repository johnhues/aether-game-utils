//------------------------------------------------------------------------------
//! @file aether.h
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
#include <map> // @TODO: Remove. For meta system.
#include <vector> // @TODO: Remove. For meta system.

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
  #define aeAssert() throw
#else
  #define aeAssert() asm( "int $3" )
#endif

#if _AE_WINDOWS_
  #define aeCompilationWarning( _msg ) _Pragma( message _msg )
#else
  #define aeCompilationWarning( _msg ) _Pragma( "warning #_msg" )
#endif

#if _AE_LINUX_ || _AE_APPLE_
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

namespace ae {

//------------------------------------------------------------------------------
//! \defgroup Platform
//! @{
//------------------------------------------------------------------------------
uint32_t GetPID(); //!< Returns the process ID on Windows, OSX, and Linux. Returns 0 with Emscripten builds.
uint32_t GetMaxConcurrentThreads(); //!< Returns the number of virtual cores available.
bool IsDebuggerAttached(); //!< Returns true if attached to Visual Studio or Xcode.
template < typename T > const char* GetTypeName(); //!< Returns the name of the given class or basic type.
double GetTime(); //!< Returns a monotonically increasing time in seconds, useful for calculating high precision deltas. Time '0' is undefined.
//! @}

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
#define AE_ALLOC_TAG_FIXME ae::Tag( "aeFixMe" )
#define AE_ALLOC_TAG_FILE ae::Tag( "aeFile" )

//------------------------------------------------------------------------------
//! \defgroup Allocation
//! Allocation utilities.
//! By default aether-game-utils uses system allocations (malloc / free), which may
//! be fine for your use case. If not, it's advised that you implement your own
//! ae::Allocator with dlmalloc or similar and then call ae::SetGlobalAllocator()
//! with your allocator at program start. All allocations are tagged, (@TODO)
//! they can be inspected through the current ae::Allocator with ae::GetGlobalAllocator().
//! @{
//------------------------------------------------------------------------------
class Allocator
{
public:
  virtual ~Allocator() {}
  //! Should return 'bytes' with minimum alignment of 'alignment'. Optionally, a
  //! tag should be used to select a pool of memory, or for diagnostics/debugging.
  virtual void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) = 0;
  //! Should attempt to expand or contract allocations made with Allocate() to
  //! match size 'bytes'. On failure this function should return nullptr.
  virtual void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) = 0;
  //! Free memory allocated with ae::Allocator::Allocate() or ae::Allocator::Reallocate().
  virtual void Free( void* data ) = 0;
};
//! The given ae::Allocator is used for all memory allocations. You must call
//! ae::SetGlobalAllocator() before any allocations are made or else a default
//! allocator which uses malloc / free will be used. The set value can be retrieved
//! with ae::GetGlobalAllocator().
void SetGlobalAllocator( Allocator* alloc );
//! Get the custom allocator set with ae::SetGlobalAllocator(). If no custom
//! allocator is set before the first allocation is made, this will return a
//! default ae::Allocator which uses malloc / free. If ae::SetGlobalAllocator() has
//! never been called and no allocations have been made, this will return nullptr.
Allocator* GetGlobalAllocator();
//! Allocate and constructs an array of 'count' elements of type T. an ae::Tag
//! must be specifed and should represent the allocation type. Type T must have a
//! default constructor. All arrays allocated with this function should be freed with
//! ae::Delete(). Uses ae::GetGlobalAllocator() and ae::Allocator::Allocate() internally.
template < typename T > T* NewArray( ae::Tag tag, uint32_t count );
//! Allocates and constructs a single element of type T. an ae::Tag must be specified
//! and should represent the allocation type. All 'args' are passed to the constructor
//! of T. All allocations should be freed with ae::Delete(). Uses ae::GetGlobalAllocator()
//! and ae::Allocator::Allocate() internally.
template < typename T, typename ... Args > T* New( ae::Tag tag, Args ... args );
//! Should be called to destruct and free all allocations made with ae::New()
//! and ae::NewArray(). Uses ae::GetGlobalAllocator() and ae::Allocator::Free()
//! internally.
template < typename T > void Delete( T* obj );
// C style allocations
void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment );
void* Reallocate( void* data, uint32_t bytes, uint32_t alignment );
void Free( void* data );
//! @}

//------------------------------------------------------------------------------
// ae::Scratch< T > class
//! This class is useful for scoped allocations. (@TODO) In the future it
//! should allocate from a stack. This should allow big allocations to happen
//! cheaply each frame without creating any fragmentation.
//------------------------------------------------------------------------------
template < typename T >
class Scratch
{
public:
  Scratch( ae::Tag tag, uint32_t count );
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
//! \defgroup Math
//! @{
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
struct AE_ALIGN( 16 ) VecT
{
  VecT() {}
  VecT( bool ) = delete;

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
struct Vec2 : public VecT< Vec2 >
{
  Vec2() {} // Empty default constructor for performance of vertex arrays etc
  Vec2( const Vec2& ) = default;
  explicit Vec2( float v );
  Vec2( float x, float y );
  explicit Vec2( const float* v2 );
  explicit Vec2( struct Int2 i2 );
  static Vec2 FromAngle( float angle );
  struct Int2 NearestCopy() const;
  struct Int2 FloorCopy() const;
  struct Int2 CeilCopy() const;
  Vec2 RotateCopy( float rotation ) const;
  float GetAngle() const;
  Vec2 Reflect( Vec2 v, Vec2 n ) const;
  union
  {
    struct
    {
      float x;
      float y;
    };
    float data[ 2 ];
  };
};

//------------------------------------------------------------------------------
// ae::Vec3 struct
//------------------------------------------------------------------------------
struct Vec3 : public VecT< Vec3 >
{
  Vec3() {} // Empty default constructor for performance of vertex arrays etc
  explicit Vec3( float v );
  Vec3( float x, float y, float z );
  explicit Vec3( const float* v3 );
  explicit Vec3( struct Int3 i3 );
  Vec3( Vec2 xy, float z ); // @TODO: Support Y up
  explicit Vec3( Vec2 xy );
  explicit operator Vec2() const;
  Vec2 GetXY() const;
  Vec2 GetXZ() const;
  struct Int3 NearestCopy() const;
  struct Int3 FloorCopy() const;
  struct Int3 CeilCopy() const;
  
  float GetAngleBetween( const Vec3& v, float epsilon = 0.0001f ) const;
  void AddRotationXY( float rotation ); // @TODO: Support Y up
  Vec3 RotateCopy( Vec3 axis, float angle ) const;
  Vec3 Lerp( const Vec3& end, float t ) const;
  Vec3 Slerp( const Vec3& end, float t, float epsilon = 0.0001f ) const;
  
  static Vec3 Cross( const Vec3& v0, const Vec3& v1 );
  Vec3 Cross( const Vec3& v ) const;
  void ZeroAxis( Vec3 axis ); // Zero component along arbitrary axis (ie vec dot axis == 0)
  void ZeroDirection( Vec3 direction ); // Zero component along positive half of axis (ie vec dot dir > 0)
  Vec3 ZeroAxisCopy( Vec3 axis ) const; // Zero component along arbitrary axis (ie vec dot axis == 0)
  Vec3 ZeroDirectionCopy( Vec3 direction ) const; // Zero component along positive half of axis (ie vec dot dir > 0)

  static Vec3 ProjectPoint( const class Matrix4& projection, Vec3 p );
  
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
struct Vec4 : public VecT< Vec4 >
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
// ae::Matrix4 struct
//------------------------------------------------------------------------------
class Matrix4
{
public:
  float data[ 16 ];

  Matrix4() = default;
  Matrix4( const Matrix4& ) = default;

  // Constructor helpers
  static Matrix4 Identity();
  static Matrix4 Translation( const Vec3& p );
  static Matrix4 Rotation( Vec3 forward0, Vec3 up0, Vec3 forward1, Vec3 up1 );
  static Matrix4 RotationX( float angle );
  static Matrix4 RotationY( float angle );
  static Matrix4 RotationZ( float angle );
  static Matrix4 Scaling( const Vec3& s );
  static Matrix4 Scaling( float sx, float sy, float sz );
  static Matrix4 WorldToView( Vec3 position, Vec3 forward, Vec3 up );
  static Matrix4 ViewToProjection( float fov, float aspectRatio, float nearPlane, float farPlane );

  bool operator==( const Matrix4& o ) const { return memcmp( o.data, data, sizeof(data) ) == 0; }
  bool operator!=( const Matrix4& o ) const { return !operator== ( o ); }
  Vec4 operator*( const Vec4& v ) const;
  Matrix4 operator*( const Matrix4& m ) const;
  void operator*=( const Matrix4& m );

  void SetTranslation( float x, float y, float z );
  void SetTranslation( const Vec3& t );
  void SetScale( const Vec3& s );
  void SetRotation( const class Quaternion& r );
  Vec3 GetTranslation() const;
  Vec3 GetPosition() const { return GetTranslation(); } // HACK Remove
  Vec3 GetScale() const;
  class Quaternion GetRotation() const;

  void SetTranspose();
  void SetInverse();
  Matrix4 GetTranspose() const;
  Matrix4 GetInverse() const;
  Matrix4 GetNormalMatrix() const;
  Matrix4 GetScaleRemoved() const;

  void SetAxis( uint32_t column, const Vec3& v );
  void SetRow( uint32_t row, const Vec3& v );
  void SetRow( uint32_t row, const Vec4& v );
  Vec3 GetAxis( uint32_t column ) const;
  Vec4 GetRow( uint32_t row ) const;
};

inline std::ostream& operator << ( std::ostream& os, const Matrix4& mat )
{
  os << mat.data[ 0 ] << " " << mat.data[ 1 ] << " " << mat.data[ 2 ] << " " << mat.data[ 3 ]
    << " " << mat.data[ 4 ] << " " << mat.data[ 5 ] << " " << mat.data[ 6 ] << " " << mat.data[ 7 ]
    << " " << mat.data[ 8 ] << " " << mat.data[ 9 ] << " " << mat.data[ 10 ] << " " << mat.data[ 11 ]
    << " " << mat.data[ 12 ] << " " << mat.data[ 13 ] << " " << mat.data[ 14 ] << " " << mat.data[ 15 ];
  return os;
}

//------------------------------------------------------------------------------
// ae::Quaternion class
//------------------------------------------------------------------------------
class Quaternion
{
public:
  union
  {
    struct
    {
      float i;
      float j;
      float k;
      float r;
    };
    float data[ 4 ];
  };
  
  Quaternion() = default;
  Quaternion( const Quaternion& ) = default;

  Quaternion( const float i, const float j, const float k, const float r ) : i(i), j(j), k(k), r(r) {}
  explicit Quaternion( Vec3 v ) : i(v.x), j(v.y), k(v.z), r(0.0f) {}
  Quaternion( Vec3 forward, Vec3 up, bool prioritizeUp = true );
  Quaternion( Vec3 axis, float angle );
  static Quaternion Identity() { return Quaternion( 0.0f, 0.0f, 0.0f, 1.0f ); }

  void Normalize();
  bool operator==( const Quaternion& q ) const;
  bool operator!=( const Quaternion& q ) const;
  Quaternion& operator*= ( const Quaternion& q );
  Quaternion operator* ( const Quaternion& q ) const;
  float Dot( const Quaternion& q ) const;
  Quaternion const operator* ( float s ) const;
  void AddScaledVector( const Vec3& v, float s );
  void RotateByVector( const Vec3& v );
  void SetDirectionXY( const Vec3& v );
  Vec3 GetDirectionXY() const;
  void ZeroXY();
  void GetAxisAngle( Vec3* axis, float* angle ) const;
  void AddRotationXY( float rotation);
  Quaternion Nlerp( Quaternion end, float t ) const;
  Matrix4 GetTransformMatrix() const;
  Quaternion  GetInverse() const;
  Quaternion& SetInverse();
  Vec3 Rotate( Vec3 v ) const;
};

//------------------------------------------------------------------------------
// ae::Int2 shared member functions
// ae::Int3 shared member functions
//------------------------------------------------------------------------------
// @NOTE: Int2 and Int3 share these functions
template < typename T >
struct IntT
{
  IntT() {}
  IntT( bool ) = delete;
  bool operator==( const T& v ) const;
  bool operator!=( const T& v ) const;
  int32_t operator[]( uint32_t idx ) const;
  int32_t& operator[]( uint32_t idx );
  T operator-() const;
  T operator+( const T& v ) const;
  T operator-( const T& v ) const;
  T operator*( const T& v ) const;
  T operator/( const T& v ) const;
  void operator+=( const T& v );
  void operator-=( const T& v );
  void operator*=( const T& v );
  void operator/=( const T& v );
  T operator*( int32_t s ) const;
  T operator/( int32_t s ) const;
  void operator*=( int32_t s );
  void operator/=( int32_t s );
};
template < typename T >
inline std::ostream& operator<<( std::ostream& os, const IntT< T >& v );

//------------------------------------------------------------------------------
// ae::Int2 class
//------------------------------------------------------------------------------
struct Int2 : public IntT< Int2 >
{
  union
  {
    struct
    {
      int32_t x;
      int32_t y;
    };
    int32_t data[ 2 ];
  };

  Int2() = default;
  Int2( const Int2& ) = default;
  explicit Int2( int32_t _v );
  explicit Int2( const struct Int3& v );
  Int2( int32_t _x, int32_t _y );
  // @NOTE: No automatic conversion from ae::Vec2 because rounding type should be explicit!
};

//------------------------------------------------------------------------------
// ae::Int3 class
//------------------------------------------------------------------------------
struct Int3 : public IntT< Int3 >
{
  union
  {
    struct
    {
      int32_t x;
      int32_t y;
      int32_t z;
    };
    int32_t data[ 3 ];
  };
  int32_t pad;

  Int3() = default;
  Int3( const Int3& ) = default;
  explicit Int3( int32_t _v );
  Int3( int32_t _x, int32_t _y, int32_t _z );
  Int3( Int2 xy, int32_t _z );
  Int3( const int32_t( &v )[ 3 ] );
  Int3( const int32_t( &v )[ 4 ] );
  explicit Int3( int32_t*& v );
  explicit Int3( const int32_t*& v );
  // @NOTE: No conversion from ae::Vec3 because rounding type should be explicit!
  Int2 GetXY() const;
};

//! @} End Math group

//------------------------------------------------------------------------------
// ae::Color struct
//------------------------------------------------------------------------------
struct Color
{
  Color() {} //!< Empty default constructor for performance of vertex arrays etc
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
  void SetDt( float sec ); //!< Useful for handling frames with high delta time, eg: timeStep.SetDt( timeStep.GetTimeStep() )

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
//! A fixed length string class. The templated value is the total size of
//! the string in memory.
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
  Array( uint32_t length, const T& val ); //!< Appends 'length' number of 'val's
  // Dynamic array (N == 0)
  Array( ae::Tag tag );
  Array( ae::Tag tag, uint32_t size ); //!< Reserve size (with length of 0)
  Array( ae::Tag tag, uint32_t length, const T& val ); //!< Reserves 'length' and appends 'length' number of 'val's
  void Reserve( uint32_t total );
  // Static and dynamic arrays
  Array( const Array< T, N >& other );
  Array( Array< T, N >&& other ) noexcept; //!< Move operators fallback to regular operators if ae::Tags don't match
  void operator =( const Array< T, N >& other );
  void operator =( Array< T, N >&& other ) noexcept;
  ~Array();
  
  // Add elements
  T& Append( const T& value );
  void Append( const T* values, uint32_t count );
  T& Insert( uint32_t index, const T& value );

  // Find elements
  template < typename U > int32_t Find( const U& value ) const; //!< Returns -1 when not found
  template < typename Fn > int32_t FindFn( Fn testFn ) const; //!< Returns -1 when not found

  // Remove elements
  template < typename U > uint32_t RemoveAll( const U& value );
  template < typename Fn > uint32_t RemoveAllFn( Fn testFn );
  void Remove( uint32_t index );
  void Clear();

  // Access elements
  const T& operator[]( int32_t index ) const; //!< Performs bounds checking in debug mode. Use 'Begin()' to get raw array.
  T& operator[]( int32_t index );
  T* Begin() { return m_array; } //!< These functions can return null when array length is zero
  T* End() { return m_array + m_length; }
  const T* Begin() const { return m_array; }
  const T* End() const { return m_array + m_length; }

  // Array info
  uint32_t Length() const { return m_length; }
  uint32_t Size() const { return m_size; }
  
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
  Map(); //!< Constructor for a map with static allocated storage (N > 0)
  Map( ae::Tag pool ); //!< Constructor for a map with dynamically allocated storage (N == 0)
  void Reserve( uint32_t count ); //!< Expands the map storage if necessary so a @count number of key/value pairs can be added without any allocations. Asserts if using static storage and @count is less than N.
  
  // Access elements by key
  V& Set( const K& key, const V& value ); //!< Add or replace a key/value pair in the map. Can be retrieved with ae::Map::Get(). It's not safe to keep a pointer to the value across non-const operations.
  V& Get( const K& key ); //!< Returns a modifiable reference to the value set with @key. Asserts when key/value pair is missing.
  const V& Get( const K& key ) const; //!< Returns the value set with @key. Asserts when key/value pair is missing.
  const V& Get( const K& key, const V& defaultValue ) const; //!< Returns the value set with @key.Returns @defaultValue otherwise when the key/value pair is missing.
  V* TryGet( const K& key ); //!< Returns a pointer to the value set with @key. Returns null otherwise when the key/value pair is missing.
  const V* TryGet( const K& key ) const; //!< Returns a pointer to the value set with @key. Returns null otherwise when the key/value pair is missing.
  bool TryGet( const K& key, V* valueOut ); //!< Returns true when @key matches an existing key/value pair. A copy of the value is set to @valueOut.
  bool TryGet( const K& key, V* valueOut ) const; //!< Returns true when @key matches an existing key/value pair. A copy of the value is set to @valueOut.
  
  // Remove elements
  bool Remove( const K& key ); //!< Find and remove a key/value pair from the map. Returns true.
  bool Remove( const K& key, V* valueOut ); //!< Find and remove a key/value pair from the map. Returns true on success and a copy of the value is set to @valueOut.
  void Clear(); //!< Remove all key/value pairs from the map. ae::Map::Length() will return 0 after calling this.

  // Access elements by index
  const K& GetKey( uint32_t index ) const; //!< Returns the nth key in the map
  const V& GetValue( uint32_t index ) const; //!< Returns the nth value in the map
  V& GetValue( uint32_t index ); //!< Returns a modifiable reference to the nth value in the map
  uint32_t Length() const; //!< Returns the number of key/value pairs in the map

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
// ae::Dict class
//------------------------------------------------------------------------------
class Dict
{
public:
  Dict( ae::Tag tag );
  void SetString( const char* key, const char* value );
  void SetInt( const char* key, int32_t value );
  void SetFloat( const char* key, float value );
  void SetBool( const char* key, bool value );
  void SetVec2( const char* key, ae::Vec2 value );
  void SetVec3( const char* key, ae::Vec3 value );
  void SetVec4( const char* key, ae::Vec4 value );
  void SetInt2( const char* key, ae::Int2 value );
  void Clear();

  const char* GetString( const char* key, const char* defaultValue ) const;
  int32_t GetInt( const char* key, int32_t defaultValue ) const;
  float GetFloat( const char* key, float defaultValue ) const;
  bool GetBool( const char* key, bool defaultValue ) const;
  ae::Vec2 GetVec2( const char* key, ae::Vec2 defaultValue ) const;
  ae::Vec3 GetVec3( const char* key, ae::Vec3 defaultValue ) const;
  ae::Vec4 GetVec4( const char* key, ae::Vec4 defaultValue ) const;
  ae::Int2 GetInt2( const char* key, ae::Int2 defaultValue ) const;
  ae::Color GetColor( const char* key, ae::Color defaultValue ) const;
  bool Has( const char* key ) const;

  const char* GetKey( uint32_t idx ) const;
  const char* GetValue( uint32_t idx ) const;
  uint32_t Length() const { return m_entries.Length(); }
  
  // Supported automatic conversions which would otherwise be deleted below
  void SetString( const char* key, char* value ) { SetString( key, (const char*)value ); }
  void SetInt( const char* key, uint32_t value ) { SetInt( key, (int32_t)value ); }
  void SetFloat( const char* key, double value ) { SetFloat( key, (float)value ); }

private:
  Dict() = delete;
  // Prevent the above functions from being called accidentally through automatic conversions
  template < typename T > void SetString( const char*, T ) = delete;
  template < typename T > void SetInt( const char*, T ) = delete;
  template < typename T > void SetFloat( const char*, T ) = delete;
  template < typename T > void SetBool( const char*, T ) = delete;
  template < typename T > void SetVec2( const char*, T ) = delete;
  template < typename T > void SetVec3( const char*, T ) = delete;
  template < typename T > void SetVec4( const char*, T ) = delete;
  template < typename T > void SetInt2( const char*, T ) = delete;
  
  ae::Map< ae::Str128, ae::Str128 > m_entries; // @TODO: Should support static allocation
};

inline std::ostream& operator<<( std::ostream& os, const ae::Dict& dict );

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

//------------------------------------------------------------------------------
// ae::RectInt class
//------------------------------------------------------------------------------
struct RectInt
{
  RectInt() = default;
  RectInt( const RectInt& ) = default;
  RectInt( int32_t x, int32_t y, int32_t w, int32_t h ) : x(x), y(y), w(w), h(h) {}

  ae::Int2 GetPos() const { return ae::Int2( x, y ); }
  ae::Int2 GetSize() const { return ae::Int2( w, h ); }
  bool Contains( ae::Int2 pos ) const;
  bool Intersects( RectInt other ) const;
  void Expand( ae::Int2 pos ); //!< Zero size rect is expanded to 1x1 grid square by Expand()
  
  int32_t x, y, w, h;
};
inline std::ostream& operator<<( std::ostream& os, RectInt r )
{
  return os << r.x << " " << r.y << " " << r.w << " " << r.h;
}

//------------------------------------------------------------------------------
// ae::Hash class (fnv1a)
//! A FNV1a hash utility class. Empty strings and zero-length data buffers do not
//! hash to zero.
//------------------------------------------------------------------------------
class Hash
{
public:
  Hash() = default;
  explicit Hash( uint32_t initialValue );
  
  bool operator == ( Hash o ) const { return m_hash == o.m_hash; }
  bool operator != ( Hash o ) const { return m_hash != o.m_hash; }

  Hash& HashString( const char* str );
  Hash& HashData( const void* data, uint32_t length );
  template < typename T > Hash& HashBasicType( const T& v ) { return HashData( &v, sizeof(v) ); }
  Hash& HashFloat( float f );
  template < uint32_t N > Hash& HashFloatArray( const float (&f)[ N ] );

  void Set( uint32_t hash );
  uint32_t Get() const;

private:
  uint32_t m_hash = 0x811c9dc5;
};

template < uint32_t N >
Hash& Hash::HashFloatArray( const float (&f)[ N ] )
{
  for ( uint32_t i = 0; i < N; i++ )
  {
    HashFloat( f[ i ] );
  }
  return *this;
}

} // ae end

//------------------------------------------------------------------------------
// Logging functions
//------------------------------------------------------------------------------
#define AE_LOG(...) ae::LogInternal( _AE_LOG_INFO_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_TRACE(...) ae::LogInternal( _AE_LOG_TRACE_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_DEBUG(...) ae::LogInternal( _AE_LOG_DEBUG_, __FILE__, __LINE__, "", __VA_ARGS__ )
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

namespace ae {

//------------------------------------------------------------------------------
// ae::Window class
//! Window size is specified in virtual DPI units. Actual window content width and height are subject to the
//! displays scale factor. Passing a width and height of 1280x720 on a display with a scale factor of 2 will result
//! in a virtual window size of 1280x720 and a backbuffer size of 2560x1440. The windows scale factor can be
//! checked with ae::Window::GetScaleFactor().
//------------------------------------------------------------------------------
class Window
{
public:
  Window();
  //! Window size is specified in virtual DPI units, content size is subject to the displays scale factor
  bool Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor );
  //! Window size is specified in virtual DPI units, content size is subject to the displays scale factor
  bool Initialize( Int2 pos, uint32_t width, uint32_t height, bool showCursor );
  void Terminate();

  void SetTitle( const char* title );
  void SetFullScreen( bool fullScreen );
  void SetPosition( Int2 pos );
  void SetSize( uint32_t width, uint32_t height ); //!< Window size is specified in virtual DPI units, content size is subject to the displays scale factor
  void SetMaximized( bool maximized );

  const char* GetTitle() const { return m_windowTitle.c_str(); }
  bool GetFullScreen() const { return m_fullScreen; }
  bool GetMaximized() const { return m_maximized; }
  bool GetFocused() const { return m_focused; } //!< True if the user is currently working with this window
  Int2 GetPosition() const { return m_pos; }
  int32_t GetWidth() const; //!< Virtual window width (unscaled by display scale factor)
  int32_t GetHeight() const; //!< Virtual window height (unscaled by display scale factor)
  float GetScaleFactor() const { return m_scaleFactor; } //!<  Window content scale factor

private:
  void m_Initialize();
  Int2 m_pos;
  int32_t m_width;
  int32_t m_height;
  bool m_fullScreen;
  bool m_maximized;
  bool m_focused;
  float m_scaleFactor;
  Str256 m_windowTitle;
public:
  // Internal
  void m_UpdatePos( Int2 pos ) { m_pos = pos; }
  void m_UpdateSize( int32_t width, int32_t height, float scaleFactor );
  void m_UpdateMaximized( bool maximized ) { m_maximized = maximized; }
  void m_UpdateFocused( bool focused ) { m_focused = focused; }
  void* window;
  class GraphicsDevice* graphicsDevice;
  class Input* input;
};

//------------------------------------------------------------------------------
// ae::Key enum
//------------------------------------------------------------------------------
enum class Key : uint8_t
{
  Unknown = 0,

  A = 4,
  B = 5,
  C = 6,
  D = 7,
  E = 8,
  F = 9,
  G = 10,
  H = 11,
  I = 12,
  J = 13,
  K = 14,
  L = 15,
  M = 16,
  N = 17,
  O = 18,
  P = 19,
  Q = 20,
  R = 21,
  S = 22,
  T = 23,
  U = 24,
  V = 25,
  W = 26,
  X = 27,
  Y = 28,
  Z = 29,

  Num1 = 30,
  Num2 = 31,
  Num3 = 32,
  Num4 = 33,
  Num5 = 34,
  Num6 = 35,
  Num7 = 36,
  Num8 = 37,
  Num9 = 38,
  Num0 = 39,

  Enter = 40,
  Escape = 41,
  Backspace = 42,
  Tab = 43,
  Space = 44,

  Minus = 45,
  Equals = 46,
  LeftBracket = 47,
  RightBracket = 48,
  Backslash = 49,

  Semicolon = 51,
  Apostrophe = 52,
  Tilde = 53,
  Comma = 54,
  Period = 55,
  Slash = 56,
  CapsLock = 57,

  F1 = 58,
  F2 = 59,
  F3 = 60,
  F4 = 61,
  F5 = 62,
  F6 = 63,
  F7 = 64,
  F8 = 65,
  F9 = 66,
  F10 = 67,
  F11 = 68,
  F12 = 69,

  PrintScreen = 70,
  ScrollLock = 71,
  Pause = 72,

  Insert = 73,
  Home = 74,
  PageUp = 75,
  Delete = 76,
  End = 77,
  PageDown = 78,

  Right = 79,
  Left = 80,
  Down = 81,
  Up = 82,

  NumLock = 84,
  NumPadDivide = 84,
  NumPadMultiply = 85,
  NumPadMinus = 86,
  NumPadPlus = 87,
  NumPadEnter = 88,
  NumPad1 = 89,
  NumPad2 = 90,
  NumPad3 = 91,
  NumPad4 = 92,
  NumPad5 = 93,
  NumPad6 = 94,
  NumPad7 = 95,
  NumPad8 = 96,
  NumPad9 = 97,
  NumPad0 = 98,
  NumPadPeriod = 99,
  NumPadEquals = 103,

  LeftControl = 224,
  LeftShift = 225,
  LeftAlt = 226,
  LeftSuper = 227,
  RightControl = 228,
  RightShift = 229,
  RightAlt = 230,
  RightSuper = 231,
  LeftMeta = 254, // Command on Apple, Control on others
  RightMeta = 255, // Command on Apple, Control on others
};

//------------------------------------------------------------------------------
// ae::MouseState class
//------------------------------------------------------------------------------
struct MouseState
{
  bool leftButton = false;
  bool middleButton = false;
  bool rightButton = false;
  ae::Int2 position = ae::Int2( 0 );
  ae::Int2 movement = ae::Int2( 0 );
  ae::Vec2 scroll = ae::Vec2( 0.0f );
  bool usingTouch = false;
};

struct GamepadState
{
  ae::Vec2 leftAnalog = Vec2( 0.0f );
  ae::Vec2 rightAnalog = Vec2( 0.0f );
  ae::Int2 dpad = ae::Int2( 0 );
  
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;

  bool start = false;
  bool select = false;

  bool a = false;
  bool b = false;
  bool x = false;
  bool y = false;
  
  float leftTrigger = 0.0f;
  float rightTrigger = 0.0f;
  bool l = false;
  bool r = false;
  
  enum class BatteryLevel
  {
    None,
    Empty,
    Low,
    Medium,
    Full,
    Wired
  };
  BatteryLevel batteryLevel = BatteryLevel::None;
};

//------------------------------------------------------------------------------
// ae::Input class
//------------------------------------------------------------------------------
class Input
{
public:
  void Initialize( Window* window );
  void Terminate();
  void Pump();
  
  void SetMouseCaptured( bool enable );
  bool GetMouseCaptured() const { return m_captureMouse; }
  
  void SetTextMode( bool enabled );
  bool GetTextMode() const { return m_textMode; }
  void SetText( const char* text ) { m_text = text; }
  const char* GetText() const { return m_text.c_str(); }
  const char* GetTextInput() const { return m_textInput.c_str(); }
  
  bool Get( ae::Key key ) const;
  bool GetPrev( ae::Key key ) const;
  MouseState mouse;
  MouseState mousePrev;
  GamepadState gamepad;
  GamepadState gamepadPrev;
  bool quit = false;

// private:
  void m_SetMousePos( ae::Int2 pos );
  ae::Window* m_window = nullptr;
  bool m_captureMouse = false;
  bool m_positionSet = false;
  bool m_keys[ 256 ];
  bool m_keysPrev[ 256 ];
  bool m_textMode = false;
  void* m_textInputHandler = nullptr;
  std::string m_text;
  std::string m_textInput;
};

//------------------------------------------------------------------------------
// ae::FileFilter for ae::FileDialogParams
//------------------------------------------------------------------------------
struct FileFilter
{
  FileFilter() = default;
  FileFilter( const char* desc, const char* ext ) : description( desc ) { extensions[ 0 ] = ext; }
  FileFilter( const char* desc, const char** ext, uint32_t extensionCount );
  const char* description = ""; // "JPEG Image"
  // Only alphanumeric extension strings are supported (with the exception of "*")
  const char* extensions[ 8 ] = { 0 }; // { "jpg", "jpeg", "jpe" }
};

//------------------------------------------------------------------------------
// ae::FileDialogParams for both ae::FileSystem::OpenDialog/SaveDialog
//------------------------------------------------------------------------------
struct FileDialogParams
{
  const char* windowTitle = ""; //!< Title of the dialog window
  ae::Array< FileFilter, 8 > filters; //!< Leave empty for { ae::FileFilter( "All Files", "*" ) }
  Window* window = nullptr; //!< Recommended. Setting this will create a modal dialog.
  const char* defaultPath = ""; //!< The path that the dialog will default to.
  //! Only used with OpenDialog. If true, the dialog will allow multiple files to be selected.
  //! The files names will be returned in an ae::Array. If false the ae::Array will have 1 or
  //! 0 elements.
  bool allowMultiselect = false;
};

//------------------------------------------------------------------------------
// ae::FileSystem class
//! \brief Used to read and write files or create save and open dialogs.
//------------------------------------------------------------------------------
class FileSystem
{
public:
  //! Represents directories that the FileSystem class can load/save from.
  enum class Root
  {
    Data, //!< A given existing directory
    User, //!< A directory for storing preferences and savedata
    Cache, //!< A directory for storing expensive to generate data (computed, downloaded, etc)
    UserShared, //!< Same as above but shared accross the 'organization name'
    CacheShared //!< Same as above but shared accross the 'organization name'
  };
  
  //! Passing an empty string to dataDir is equivalent to using
  //! the applications working directory. Organization name should be your name
  //! or your companies name and should be consistent across apps. Application
  //! name should be the name of this application. Initialize() creates missing
  //! folders for Root::User and Root::Cache.
  void Initialize( const char* dataDir, const char* organizationName, const char* applicationName );

  // Member functions for use of Root directories
  bool GetRootDir( Root root, Str256* outDir ) const;
  uint32_t GetSize( Root root, const char* filePath ) const;
  uint32_t Read( Root root, const char* filePath, void* buffer, uint32_t bufferSize ) const;
  uint32_t Write( Root root, const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs ) const;
  bool CreateFolder( Root root, const char* folderPath ) const;
  void ShowFolder( Root root, const char* folderPath ) const;

  // Static member functions intended to be used when not creating a  instance
  static uint32_t GetSize( const char* filePath );
  static uint32_t Read( const char* filePath, void* buffer, uint32_t bufferSize );
  static uint32_t Write( const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs );
  static bool CreateFolder( const char* folderPath );
  static void ShowFolder( const char* folderPath );
  
  // Static helpers
  static Str256 GetAbsolutePath( const char* filePath );
  static const char* GetFileNameFromPath( const char* filePath );
  static const char* GetFileExtFromPath( const char* filePath );
  static Str256 GetDirectoryFromPath( const char* filePath );
  static void AppendToPath( Str256* path, const char* str );

  // File dialogs
  ae::Array< std::string > OpenDialog( const FileDialogParams& params );
  std::string SaveDialog( const FileDialogParams& params );

private:
  void m_SetDataDir( const char* dataDir );
  void m_SetUserDir( const char* organizationName, const char* applicationName );
  void m_SetCacheDir( const char* organizationName, const char* applicationName );
  void m_SetUserSharedDir( const char* organizationName );
  void m_SetCacheSharedDir( const char* organizationName );
  Str256 m_dataDir;
  Str256 m_userDir;
  Str256 m_cacheDir;
  Str256 m_userSharedDir;
  Str256 m_cacheSharedDir;
};

//------------------------------------------------------------------------------
// @TODO: Graphics globals. Should be parameters to modules that need them.
//------------------------------------------------------------------------------
extern uint32_t GLMajorVersion;
extern uint32_t GLMinorVersion;
// Caller enables this externally.  The renderer, Shader, math aren't tied to one another
// enough to pass this locally.  glClipControl is also not accessible in ES or GL 4.1, so
// doing this just to write the shaders for reverseZ.  In GL, this won't improve precision.
// http://www.reedbeta.com/blog/depth-precision-visualized/
extern bool ReverseZ;

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
  ae::Hash GetHash() const { return m_hash; }

private:
  ae::Map< Str32, Value > m_uniforms = AE_ALLOC_TAG_RENDER;
  ae::Hash m_hash;
};

//------------------------------------------------------------------------------
// ae::Shader class
// @NOTE: Some special built in functions and defines are automatically included
//        for portability reasons (e.g. for OpenGL ES). There are also some
//        convenient helper functions to convert between linear and srgb color
//        spaces. It's not necessary to use any of these helpers and basic valid
//        GLSL can be provided instead.
// Example vertex shader:
/*
AE_UNIFORM_HIGHP mat4 u_worldToScreen;

AE_IN_HIGHP vec3 a_position;
AE_IN_HIGHP vec2 a_uv;
AE_IN_HIGHP vec4 a_color;

AE_OUT_HIGHP vec2 v_uv;
AE_OUT_HIGHP vec4 v_color;

void main()
{
  v_uv = a_uv;
  v_color = a_color;
  gl_Position = u_worldToScreen * vec4( a_position, 1.0 );
}
*/
// Example fragment shader:
/*
AE_UNIFORM sampler2D u_tex;

AE_IN_HIGHP vec2 v_uv;
AE_IN_HIGHP vec4 v_color;

void main()
{
  AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;
}
*/
//------------------------------------------------------------------------------
const uint32_t _kMaxShaderAttributeCount = 16;
const uint32_t _kMaxShaderAttributeNameLength = 16;
const uint32_t _kMaxShaderDefines = 4;

class Shader
{
public:
  // Constants
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
  void Terminate();
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
  // Constants
  enum class Usage { Dynamic, Static };
  enum class Type { UInt8, UInt16, UInt32, NormalizedUInt8, NormalizedUInt16, NormalizedUInt32, Float };
  enum class Primitive { Point, Line, Triangle };
  
  // Initialization
  VertexData() = default;
  ~VertexData();
  void Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, VertexData::Primitive primitive, VertexData::Usage vertexUsage, VertexData::Usage indexUsage );
  void AddAttribute( const char *name, uint32_t componentCount, VertexData::Type type, uint32_t offset );
  void Terminate();
  
  //! Sets current vertex data. Equivalent to calling Clear() then Append(). After calling this function you must call Upload() once before calling Render().
  void SetVertices( const void* vertices, uint32_t count );
  //! Sets current index data. Equivalent to calling Clear() then Append(). After calling this function you must call Upload() once before calling Render().
  void SetIndices( const void* indices, uint32_t count );
  //! Add vertices to end of existing array. After calling this function you must call Upload() once before calling Render().
  void AppendVertices( const void* vertices, uint32_t count );
  //! Add indices to end of existing array. Given indices are each offset based on @indexOffset. It could be useful to use GetVertexCount() as a parameter to @indexOffset
  //! before appending new vertices. After calling this function you must call Upload() once before calling Render().
  void AppendIndices( const void* indices, uint32_t count, uint32_t indexOffset );
  //! Sets dynamic vertex count to 0. Has no effect if vertices are using VertexData::Usage::Static.
  void ClearVertices();
  //! Sets dynamic index count to 0. Has no effect if indices are using VertexData::Usage::Static.
  void ClearIndices();
  
  //! Prepares buffers for rendering. Call after Setting/Appending vertices and indices, but before Render(). This will result in a no-op if no changes have been made.
  void Upload();
  //! Renders all vertex data
  void Render( const Shader* shader, const UniformList& uniforms ) const;
  //! Renders vertex data range
  void Render( const Shader* shader, const UniformList& uniforms, uint32_t primitiveStart, uint32_t primitiveCount ) const;
  
  // Get info
  const void* GetVertices() const;
  const void* GetIndices() const;
  uint32_t GetVertexSize() const { return m_vertexSize; }
  uint32_t GetIndexSize() const { return m_indexSize; }
  uint32_t GetVertexCount() const { return m_vertexCount; }
  uint32_t GetIndexCount() const { return m_indexCount; }
  uint32_t GetMaxVertexCount() const { return m_maxVertexCount; }
  uint32_t GetMaxIndexCount() const { return m_maxIndexCount; }
  uint32_t GetAttributeCount() const { return m_attributes.Length(); }
  VertexData::Primitive GetPrimitiveType() const { return m_primitive; }
  
  // Implementation
private:
  struct Attribute
  {
    char name[ _kMaxShaderAttributeNameLength ];
    uint32_t componentCount;
    uint32_t type; // GL_BYTE, GL_SHORT, GL_FLOAT...
    uint32_t offset;
    bool normalized;
  };
  VertexData( const VertexData& ) = delete;
  VertexData( VertexData&& ) = delete;
  void operator=( const VertexData& ) = delete;
  void operator=( VertexData&& ) = delete;
  void m_SetVertices( const void* vertices, uint32_t count );
  void m_SetIndices( const void* indices, uint32_t count );
  const Attribute* m_GetAttributeByName( const char* name ) const;
  // Params
  uint32_t m_vertexSize = 0;
  uint32_t m_indexSize = 0;
  VertexData::Primitive m_primitive = (VertexData::Primitive)-1;
  VertexData::Usage m_vertexUsage = (VertexData::Usage)-1;
  VertexData::Usage m_indexUsage = (VertexData::Usage)-1;
  ae::Array< Attribute, _kMaxShaderAttributeCount > m_attributes;
  uint32_t m_maxVertexCount = 0;
  uint32_t m_maxIndexCount = 0;
  // Dynamic state
  uint32_t m_vertexCount = 0;
  uint32_t m_indexCount = 0;
  void* m_vertexReadable = nullptr;
  void* m_indexReadable = nullptr;
  bool m_vertexDirty = false;
  bool m_indexDirty = false;
  // System resources
  uint32_t m_array = 0;
  uint32_t m_vertices = ~0;
  uint32_t m_indices = ~0;
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
    Depth16,
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
  virtual void Terminate();
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
  void Initialize( const void* data, uint32_t width, uint32_t height, Format format, Type type, Filter filter, Wrap wrap, bool autoGenerateMipmaps );
  void Terminate() override;

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
  void Terminate();

  void Activate();
  void Clear( Color color );
  void Render( const Shader* shader, const UniformList& uniforms );
  void Render2D( uint32_t textureIndex, Rect ndc, float z );

  const Texture2D* GetTexture( uint32_t index ) const;
  const Texture2D* GetDepth() const;
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

  //! Get ndc space rect of this target within another target (fill but maintain aspect ratio)
  //! GetNDCFillRectForTarget( GraphicsDevice::GetWindow()::GetWidth(),  GraphicsDevice::GetWindow()::Height() )
  //! GetNDCFillRectForTarget( GraphicsDeviceTarget()::GetWidth(),  GraphicsDeviceTarget()::Height() )
  Rect GetNDCFillRectForTarget( uint32_t otherWidth, uint32_t otherHeight ) const;

  //! Other target to local transform (pixels->pixels)
  //! Useful for transforming window/mouse pixel coordinates to local pixels
  //! GetTargetPixelsToLocalTransform( GraphicsDevice::GetWindow()::GetWidth(),  GraphicsDevice::GetWindow()::Height(), GetNDCFillRectForTarget( ... ) )
  Matrix4 GetTargetPixelsToLocalTransform( uint32_t otherPixelWidth, uint32_t otherPixelHeight, Rect ndc ) const;

  //! Mouse/window pixel coordinates to world space
  //! GetTargetPixelsToWorld( GetTargetPixelsToLocalTransform( ... ), TODO )
  Matrix4 GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const;

  //! Creates a transform matrix from aeQuad vertex positions to ndc space
  //! GraphicsDeviceTarget uses aeQuad vertices internally
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
  ~GraphicsDevice();
  void Initialize( class Window* window );
  void Terminate();

  void Activate();
  void Clear( Color color );
  void Present();
  void AddTextureBarrier(); //!< Must call to readback from active render target (GL only)

  class Window* GetWindow() { return m_window; }
  RenderTarget* GetCanvas() { return &m_canvas; }
  uint32_t GetWidth() const { return m_canvas.GetWidth(); }
  uint32_t GetHeight() const { return m_canvas.GetHeight(); }
  float GetAspectRatio() const;

//private:
  friend class ae::Window;
  void m_HandleResize( uint32_t width, uint32_t height );
  Window* m_window = nullptr;
  RenderTarget m_canvas;
#if _AE_EMSCRIPTEN_
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE m_context = 0;
#else
  void* m_context = nullptr;
#endif
  int32_t m_defaultFbo = 0;
};

//------------------------------------------------------------------------------
// ae::DebugLines class
//------------------------------------------------------------------------------
class DebugLines
{
public:
  void Initialize( uint32_t maxObjects );
  void Terminate();
  void Render( const Matrix4& worldToNdc ); //!< Also calls Clear() so AddLine() etc should be called every frame
  void SetXRayEnabled( bool enabled ) { m_xray = enabled; } //!< Draw desaturated lines on failed depth test

  bool AddLine( Vec3 p0, Vec3 p1, Color color );
  bool AddDistanceCheck( Vec3 p0, Vec3 p1, float distance );
  bool AddRect( Vec3 pos, Vec3 up, Vec3 normal, Vec2 size, Color color );
  bool AddCircle( Vec3 pos, Vec3 normal, float radius, Color color, uint32_t pointCount );
  bool AddAABB( Vec3 pos, Vec3 halfSize, Color color );
  bool AddOBB( Matrix4 transform, Color color );
  bool AddSphere( Vec3 pos, float radius, Color color, uint32_t pointCount );
  void Clear();

private:
  struct DebugVertex
  {
    Vec3 pos;
    Color color;
  };
  Array< DebugVertex > m_verts = AE_ALLOC_TAG_RENDER;
  VertexData m_vertexData;
  Shader m_shader;
  bool m_xray = true;
  enum class DebugType
  {
    Line,
    Rect,
    Circle,
    Sphere,
    AABB,
    Cube,
  };
  // @TODO: Should just store verts, Init() should take a max vert count
  struct DebugObject
  {
    DebugType type;
    Vec3 pos;
    Vec3 end;
    Quaternion rotation;
    Vec3 size;
    float radius;
    Color color;
    uint32_t pointCount; // circle only
    Matrix4 transform;
  };
  Array< DebugObject > m_objs = AE_ALLOC_TAG_RENDER;
};

//------------------------------------------------------------------------------
// ae::AudioData class
//------------------------------------------------------------------------------
class AudioData
{
public:
  AudioData();
  void Initialize( const char* filePath );
  void Terminate();

  ae::Str64 name;
  uint32_t buffer;
  float length;
};

//------------------------------------------------------------------------------
// ae::Audio class
//------------------------------------------------------------------------------
class Audio
{
public:
  void Initialize( uint32_t musicChannels, uint32_t sfxChannels );
  void Terminate();

  void SetVolume( float volume );
  void PlayMusic( const AudioData* audioFile, float volume, uint32_t channel );
  void PlaySfx( const AudioData* audioFile, float volume, int32_t priority ); //!< Lower priority values interrupt sfx with higher values
  void StopMusic( uint32_t channel );
  void StopAllSfx();

  uint32_t GetMusicChannelCount() const;
  uint32_t GetSfxChannelCount() const;
  void Log();

private:
  struct Channel
  {
    Channel();
    uint32_t source;
    int32_t priority;
    const AudioData* resource;
  };
  ae::Array< Channel > m_musicChannels = AE_ALLOC_TAG_AUDIO;
  ae::Array< Channel > m_sfxChannels = AE_ALLOC_TAG_AUDIO;
};

//------------------------------------------------------------------------------
// ae::BinaryStream class
//------------------------------------------------------------------------------
class BinaryStream
{
public:
  static BinaryStream Writer( uint8_t* data, uint32_t length );
  static BinaryStream Writer( Array< uint8_t >* array );
  static BinaryStream Writer();
  static BinaryStream Reader( const uint8_t* data, uint32_t length );
  static BinaryStream Reader( const Array< uint8_t >& data );

  void SerializeUint8( uint8_t& v );
  void SerializeUint8( const uint8_t& v );
  void SerializeUint16( uint16_t& v );
  void SerializeUint16( const uint16_t& v );
  void SerializeUint32( uint32_t& v );
  void SerializeUint32( const uint32_t& v );
  void SerializeUint64( uint64_t& v );
  void SerializeUint64( const uint64_t& v );

  void SerializeInt8( int8_t& v );
  void SerializeInt8( const int8_t& v );
  void SerializeInt16( int16_t& v );
  void SerializeInt16( const int16_t& v );
  void SerializeInt32( int32_t& v );
  void SerializeInt32( const int32_t& v );
  void SerializeInt64( int64_t& v );
  void SerializeInt64( const int64_t& v );

  void SerializeFloat( float& v );
  void SerializeFloat( const float& v );
  void SerializeDouble( double& v );
  void SerializeDouble( const double& v );

  void SerializeBool( bool& v );
  void SerializeBool( const bool& v );
  
  template < uint32_t N >
  void SerializeString( Str< N >& str );
  template < uint32_t N >
  void SerializeString( const Str< N >& str );

  template< typename T >
  void SerializeObject( T& v );
  template< typename T >
  void SerializeObject( const T& v );
  // Use SerializeObjectConditional() when an object may not be available for serialization when writing or reading. This function correctly updates read/write offsets when skipping serialization. Sends slightly more data than SerializeObject().
  template < typename T >
  void SerializeObjectConditional( T* obj );

  template< uint32_t N >
  void SerializeArray( char (&str)[ N ] );
  template< uint32_t N >
  void SerializeArray( const char (&str)[ N ] );
  void SerializeArray( Array< uint8_t>& array, uint32_t maxLength = 65535 );
  void SerializeArray( const Array< uint8_t>& array, uint32_t maxLength = 65535 );

  // @NOTE: Be careful when using SerializeRaw() functions, different platforms
  // will have different struct packing and alignment schemes.
  template< typename T >
  void SerializeRaw( T& v );
  template< typename T >
  void SerializeRaw( const T& v );
  void SerializeRaw( void* data, uint32_t length );
  void SerializeRaw( const void* data, uint32_t length );
  void SerializeRaw( Array< uint8_t >& array );
  void SerializeRaw( const Array< uint8_t >& array );

  // Once the stream is invalid serialization calls will result in silent no-ops
  void Invalidate() { m_isValid = false; }
  bool IsValid() const { return m_isValid; }
  
  // Get mode
  bool IsWriter() const { return m_mode == Mode::WriteBuffer; }
  bool IsReader() const { return m_mode == Mode::ReadBuffer; }

  // Get data buffer
  const uint8_t* GetData() const { return ( m_data || m_GetArray().Length() == 0 ) ? m_data : &m_GetArray()[ 0 ]; }
  uint32_t GetOffset() const { return m_offset; }
  uint32_t GetLength() const { return m_length; }

  // Get data past the current read head
  const uint8_t* PeekData() const { return GetData() + m_offset; }
  uint32_t GetRemaining() const { return m_length - m_offset; }
  void Discard( uint32_t length );

  // Internal
  enum class Mode
  {
    None,
    ReadBuffer,
    WriteBuffer,
  };
private:
  Array< uint8_t >& m_GetArray() { return m_extArray ? *m_extArray : m_array; }
  const Array< uint8_t >& m_GetArray() const { return m_extArray ? *m_extArray : m_array; }
  void m_SerializeArrayLength( uint32_t& length, uint32_t maxLength );
  Mode m_mode = Mode::None;
  bool m_isValid = false;
  uint8_t* m_data = nullptr;
  uint32_t m_length = 0;
  uint32_t m_offset = 0;
  Array< uint8_t >* m_extArray = nullptr;
  Array< uint8_t > m_array = AE_ALLOC_TAG_NET;

public:
  BinaryStream() = default;
  BinaryStream( Mode mode, uint8_t * data, uint32_t length );
  BinaryStream( Mode mode, const uint8_t * data, uint32_t length );
  BinaryStream( Mode mode );
  BinaryStream( Array< uint8_t >*array );

  // Prevent Serialize functions from being called accidentally through automatic conversions
  template < typename T > void SerializeUint8( T ) = delete;
  template < typename T > void SerializeUint16( T ) = delete;
  template < typename T > void SerializeUint32( T ) = delete;
  template < typename T > void SerializeUint64( T ) = delete;
  template < typename T > void SerializeInt8( T ) = delete;
  template < typename T > void SerializeInt16( T ) = delete;
  template < typename T > void SerializeInt32( T ) = delete;
  template < typename T > void SerializeInt64( T ) = delete;
  template < typename T > void SerializeFloat( T ) = delete;
  template < typename T > void SerializeDouble( T ) = delete;
  template < typename T > void SerializeBool( T ) = delete;
  template < typename T > void SerializeString( T ) = delete;
};

//------------------------------------------------------------------------------
// ae::NetId struct
//------------------------------------------------------------------------------
struct NetId
{
  NetId() = default;
  NetId( const NetId& ) = default;
  explicit NetId( uint32_t id ) : m_id( id ) {}
  bool operator==( const NetId& o ) const { return o.m_id == m_id; }
  bool operator!=( const NetId& o ) const { return o.m_id != m_id; }
  explicit operator bool () const { return m_id != 0; }
  uint32_t GetInternalId() const { return m_id; }
  void Serialize( BinaryStream* s ) { s->SerializeUint32( m_id ); }
  void Serialize( BinaryStream* s ) const { s->SerializeUint32( m_id ); }
private:
  uint32_t m_id = 0;
};
using RemoteId = NetId;

//------------------------------------------------------------------------------
// ae::NetObject class
//------------------------------------------------------------------------------
class NetObject
{
public:
  struct Msg
  {
    const uint8_t* data;
    uint32_t length;
  };

  //------------------------------------------------------------------------------
  // General
  //------------------------------------------------------------------------------
  NetId GetId() const { return m_id; }
  bool IsAuthority() const { return m_local; }
  
  //------------------------------------------------------------------------------
  // Server
  // @NOTE: All server data will be sent with the next NetObjectServer::UpdateSendData()
  //------------------------------------------------------------------------------
  // True until SetInitData() is called
  bool IsPendingInit() const;
  // Call once after NetObjectServer::CreateNetObject(), will trigger Create event
  // on clients. You can pass 'nullptr' and '0' as params, but you still must call
  // before the object will be created remotely. Clients can call NetObject::GetInitData()
  // to get the data set here.
  void SetInitData( const void* initData, uint32_t initDataLength );
  // Call SetSyncData each frame to update that state of the clients NetObject.
  // Only the most recent data is sent. Data is only sent when changed.
  void SetSyncData( const void* data, uint32_t length );
  // Call as many times as necessary each tick
  void SendMessage( const void* data, uint32_t length );

  //------------------------------------------------------------------------------
  // Client
  //------------------------------------------------------------------------------
  // Use GetInitData() after receiving a new NetObject from NetObjectClient::PumpCreate()
  // to construct the object. Retrieves the data set by NetObject::SetInitData() on
  // the server.
  const uint8_t* GetInitData() const;
  // Retrieves the length of the data set by NetObject::SetInitData() on the server.
  // Use in conjunction with NetObject::GetInitData().
  uint32_t InitDataLength() const;

  // Only the latest sync data is ever available, so there's no need to read this
  // data as if it was a stream.
  const uint8_t* GetSyncData() const;
  // Check for new data from server
  uint32_t SyncDataLength() const;
  // (Optional) Call to clear SyncDataLength() until new data is received
  void ClearSyncData();

  // Get messages sent from the server. Call repeatedly until false is returned
  bool PumpMessages( Msg* msgOut );

  // True once the NetObject has been deleted on the server.
  // Call NetObjectClient::Destroy() when you're done with it.
  bool IsPendingDestroy() const;

  //------------------------------------------------------------------------------
  // Internal
  //------------------------------------------------------------------------------
private:
  friend class NetObjectClient;
  friend class NetObjectConnection;
  friend class NetObjectServer;

  void m_SetLocal() { m_local = true; }
  void m_SetClientData( const uint8_t* data, uint32_t length );
  void m_ReceiveMessages( const uint8_t* data, uint32_t length );
  void m_FlagForDestruction() { m_isPendingDestroy = true; }
  void m_UpdateHash();
  bool m_Changed() const { return m_hash != m_prevHash; }

  NetId m_id;
  bool m_local = false;
  ae::Array< uint8_t > m_initData = AE_ALLOC_TAG_NET;
  ae::Array< uint8_t > m_data = AE_ALLOC_TAG_NET;
  ae::Array< uint8_t > m_messageDataOut = AE_ALLOC_TAG_NET;
  ae::Array< uint8_t > m_messageDataIn = AE_ALLOC_TAG_NET;
  uint32_t m_messageDataInOffset = 0;
  uint32_t m_hash = 0;
  uint32_t m_prevHash = 0;
  bool m_isPendingInit = true;
  bool m_isPendingDestroy = false;
};

//------------------------------------------------------------------------------
// ae::NetObjectClient class
//------------------------------------------------------------------------------
class NetObjectClient
{
public:
  // The following sequence should be performed each frame
  void ReceiveData( const uint8_t* data, uint32_t length ); // 1) Handle raw data from server (call once when new data arrives)
  NetObject* PumpCreate(); // 2) Get new objects (call this repeatedly until no new NetObjects are returned)
  // 3) Handle new sync data with NetObject::GetSyncData() and process incoming messages with NetObject::PumpMessages()
  void Destroy( NetObject* pendingDestroy ); // 4) Call this on ae::NetObjects once NetObject::IsPendingDestroy() returns true
  
  NetId GetLocalId( RemoteId remoteId ) const { return m_remoteToLocalIdMap.Get( remoteId, {} ); }
  RemoteId GetRemoteId( NetId localId ) const { return m_localToRemoteIdMap.Get( localId, {} ); }

private:
  NetObject* m_CreateNetObject( BinaryStream* rStream, bool allowResolve );
  void m_StartNetObjectDestruction( NetObject* netObject );
  uint32_t m_serverSignature = 0;
  uint32_t m_lastNetId = 0;
  bool m_delayCreationForDestroy = false;
  ae::Map< NetId, NetObject* > m_netObjects = AE_ALLOC_TAG_NET;
  ae::Map< RemoteId, NetId > m_remoteToLocalIdMap = AE_ALLOC_TAG_NET;
  ae::Map< NetId, RemoteId > m_localToRemoteIdMap = AE_ALLOC_TAG_NET;
  ae::Array< NetObject* > m_created = AE_ALLOC_TAG_NET;
};

//------------------------------------------------------------------------------
// ae::NetObjectConnection class
//------------------------------------------------------------------------------
class NetObjectConnection
{
public:
  const uint8_t* GetSendData() const; // Call NetObjectServer::UpdateSendData() first
  uint32_t GetSendLength() const; // Call NetObjectServer::UpdateSendData() first

public:
  void m_UpdateSendData();

  bool m_first = true;
  class NetObjectServer* m_replicaDB = nullptr;
  bool m_pendingClear = false;
  ae::Array< uint8_t > m_sendData = AE_ALLOC_TAG_NET;
  // Internal
  enum class EventType : uint8_t
  {
    Connect,
    Create,
    Destroy,
    Update,
    Messages
  };
};

//------------------------------------------------------------------------------
// ae::NetObjectServer class
//------------------------------------------------------------------------------
class NetObjectServer
{
public:
  NetObjectServer();
  // Create a server authoritative NetObject which will be replicated to clients through NetObjectConnection/NetObjectClient
  NetObject* CreateNetObject();
  void DestroyNetObject( NetObject* netObject );

  // Allocate one ae::NetObjectConnection per client
  NetObjectConnection* CreateConnection();
  void DestroyConnection( NetObjectConnection* connection );

  // Call each frame before NetObjectConnection::GetSendData()
  void UpdateSendData();

private:
  uint32_t m_signature = 0;
  uint32_t m_lastNetId = 0;
  ae::Array< NetObject* > m_pendingCreate = AE_ALLOC_TAG_NET;
  ae::Map< NetId, NetObject* > m_netObjects = AE_ALLOC_TAG_NET;
  ae::Array< NetObjectConnection* > m_servers = AE_ALLOC_TAG_NET;
public:
  // Internal
  NetObject* GetNetObject( uint32_t index ) { return m_netObjects.GetValue( index ); }
  uint32_t GetNetObjectCount() const { return m_netObjects.Length(); }
};

//! \defgroup Meta
//! @{

//------------------------------------------------------------------------------
// External macros to force module linking
//------------------------------------------------------------------------------
#define AE_FORCE_LINK_CLASS( x ) \
  extern int force_link_##x; \
  struct ForceLink_##x { ForceLink_##x() { force_link_##x = 1; } }; \
  ForceLink_##x forceLink_##x;

//------------------------------------------------------------------------------
// External meta class registerer
//------------------------------------------------------------------------------
#define AE_REGISTER_CLASS( x ) \
  int force_link_##x = 0; \
  template <> const char* ae::_TypeName< x >::Get() { return #x; } \
  template <> void ae::_DefineType< x >( ae::Type *type, uint32_t index ) { type->Init< x >( #x, index ); } \
  static ae::_TypeCreator< x > ae_type_creator_##x( #x );

//------------------------------------------------------------------------------
// External meta var registerer
//------------------------------------------------------------------------------
#define AE_REGISTER_CLASS_VAR( c, v ) \
  static ae::_VarCreator< c, decltype(c::v), offsetof( c, v ) > ae_var_creator_##c##_##v( #c, #v );
//------------------------------------------------------------------------------
// External meta property registerer
//------------------------------------------------------------------------------
#define AE_REGISTER_CLASS_PROPERTY( c, p ) \
  static ae::_PropCreator< c > ae_prop_creator_##c##_##p( #c, #p, "" );

#define AE_REGISTER_CLASS_PROPERTY_VALUE( c, p, v ) \
static ae::_PropCreator< c > ae_prop_creator_##c##_##p_##v( #c, #p, #v );

//------------------------------------------------------------------------------
// External enum definer and registerer
//------------------------------------------------------------------------------
//! Define a new enum (must register with AE_REGISTER_ENUM_CLASS)
#define AE_DEFINE_ENUM_CLASS( E, T, ... ) \
  enum class E : T { \
    __VA_ARGS__ \
  }; \
  template <> \
  struct ae::_VarType< E > { \
    static ae::Var::Type GetType() { return ae::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
  }; \
  struct AE_ENUM_##E { AE_ENUM_##E( const char* name = #E, const char* def = #__VA_ARGS__ ); };\
  static std::ostream &operator << ( std::ostream &os, E e ) { \
    os << ae::GetEnum( #E )->GetNameByValue( (int32_t)e ); \
    return os; \
  }

//! Register an enum defined with AE_DEFINE_ENUM_CLASS
#define AE_REGISTER_ENUM_CLASS( E ) \
  AE_ENUM_##E::AE_ENUM_##E( const char* name, const char* def ) { ae::_EnumCreator< E > ec( name, def ); } \
  AE_ENUM_##E ae_enum_creator_##E; \
  template <> const ae::Enum* ae::GetEnum< E >() { static const ae::Enum* e = GetEnum( #E ); return e; }

//------------------------------------------------------------------------------
// External c-style enum registerer
//------------------------------------------------------------------------------
//! Register an already defined c-style enum type
#define AE_REGISTER_ENUM( E ) \
  template <> \
  struct ae::_VarType< E > { \
    static ae::Var::Type GetType() { return ae::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
    static const char* GetPrefix() { return ""; } \
  }; \
  ae::_EnumCreator2< E > ae_enum_creator_##E( #E ); \
  template <> const ae::Enum* ae::GetEnum< E >() { static const ae::Enum* e = GetEnum( #E ); return e; }

//! Register an already defined c-style enum type where each value has a prefix
#define AE_REGISTER_ENUM_PREFIX( E, PREFIX ) \
  template <> \
  struct ae::_VarType< E > { \
    static ae::Var::Type GetType() { return ae::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
    static const char* GetPrefix() { return #PREFIX; } \
  }; \
  ae::_EnumCreator2< E > ae_enum_creator_##E( #E ); \
  template <> const ae::Enum* ae::GetEnum< E >() { static const ae::Enum* e = GetEnum( #E ); return e; }

//! Register c-style enum value
#define AE_REGISTER_ENUM_VALUE( E, V ) \
  ae::_EnumCreator2< E > ae_enum_creator_##E##_##V( #V, V );

//! Register c-style enum value with a manually specified name
#define AE_REGISTER_ENUM_VALUE_NAME( E, V, N ) \
ae::_EnumCreator2< E > ae_enum_creator_##E##_##V( #N, V );

//------------------------------------------------------------------------------
// External enum class registerer
//------------------------------------------------------------------------------
//! Register an already defined enum class type
#define AE_REGISTER_ENUM_CLASS2( E ) \
  template <> \
  struct ae::_VarType< E > { \
    static ae::Var::Type GetType() { return ae::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
    static const char* GetPrefix() { return ""; } \
  }; \
  namespace aeEnums::_##E { ae::_EnumCreator2< E > ae_enum_creator( #E ); } \
  template <> const ae::Enum* ae::GetEnum< E >() { static const ae::Enum* e = GetEnum( #E ); return e; }
  // @NOTE: Nested namespace declaration requires C++17

//! Register enum class value
#define AE_REGISTER_ENUM_CLASS2_VALUE( E, V ) \
  namespace aeEnums::_##E { ae::_EnumCreator2< E > ae_enum_creator_##V( #V, E::V ); }

//------------------------------------------------------------------------------
// Meta constants
//------------------------------------------------------------------------------
using TypeId = uint32_t;
const ae::TypeId kInvalidTypeId = 0;
const uint32_t kMaxMetaTypes = 64;
const uint32_t kMaxMetaProps = 8;
class Type;

//------------------------------------------------------------------------------
// External base meta object
//! Base class for all meta registered objects. Inherit from this using
//! ae::Inheritor and register your classes with AE_REGISTER_CLASS.
//------------------------------------------------------------------------------
class Object
{
public:
  virtual ~Object() {}
  static const char* GetParentTypeName() { return ""; }
  static const ae::Type* GetParentType() { return nullptr; }
  ae::TypeId GetTypeId() const { return _metaTypeId; }
  ae::TypeId _metaTypeId = ae::kInvalidTypeId;
  ae::Str32 _typeName;
};

//------------------------------------------------------------------------------
// External inheritor meta object
//------------------------------------------------------------------------------
template < typename Parent, typename This >
class Inheritor : public Parent
{
public:
  Inheritor();
  typedef Parent aeBaseType;
  static const char* GetParentTypeName();
  static const ae::Type* GetParentType();
};

//------------------------------------------------------------------------------
// External meta functions
//------------------------------------------------------------------------------
uint32_t GetTypeCount(); //!< Get the number of registered ae::Type's
const Type* GetTypeByIndex( uint32_t i ); //!< Get a registered ae::Type by index
const Type* GetTypeById( ae::TypeId id ); //!< Get a registered ae::Type by id. Same as ae::Type::GetId()
const Type* GetTypeByName( const char* typeName ); //!< Get a registered ae::Type from a type name
const Type* GetTypeFromObject( const ae::Object& obj ); //!< Get a registered ae::Type from an ae::Object
const Type* GetTypeFromObject( const ae::Object* obj ); //!< Get a registered ae::Type from a pointer to an ae::Object
template < typename T > const Type* GetType(); //!< Get a registered ae::Type directly from a type
const class Enum* GetEnum( const char* enumName ); //!< Get a registered ae::Enum by name
ae::TypeId GetObjectTypeId( const ae::Object* obj ); //!< Get a registered ae::TypeId from an ae::Object
ae::TypeId GetTypeIdFromName( const char* name ); //!< Get a registered ae::TypeId from a type name
  
//------------------------------------------------------------------------------
// Enum class
//------------------------------------------------------------------------------
class Enum
{
public:
  const char* GetName() const { return m_name.c_str(); }
  uint32_t TypeSize() const { return m_size; }
  bool TypeIsSigned() const { return m_isSigned; }
    
  template < typename T > std::string GetNameByValue( T value ) const;
  template < typename T > bool GetValueFromString( const char* str, T* valueOut ) const;
  template < typename T > bool HasValue( T value ) const;
    
  int32_t GetValueByIndex( int32_t index ) const;
  std::string GetNameByIndex( int32_t index ) const;
  uint32_t Length() const;
    
  template < typename T > static std::string GetNameFromValue( T value );
  template < typename T > static T GetValueFromString( const char* str, T defaultValue );
  
  //------------------------------------------------------------------------------
  // Internal
  //------------------------------------------------------------------------------
private:
  ae::Str32 m_name;
  uint32_t m_size;
  bool m_isSigned;
  ae::Map< int32_t, std::string > m_enumValueToName = AE_ALLOC_TAG_META;
  ae::Map< std::string, int32_t > m_enumNameToValue = AE_ALLOC_TAG_META;
public: // Internal
  Enum( const char* name, uint32_t size, bool isSigned );
  void m_AddValue( const char* name, int32_t value );
  static Enum* s_Get( const char* enumName, bool create, uint32_t size, bool isSigned );
};
  
//------------------------------------------------------------------------------
// Var class
//------------------------------------------------------------------------------
class Var
{
public:
  enum Type
  {
    String,
    UInt8,
    UInt16,
    UInt32,
    Int8,
    Int16,
    Int32,
    Bool,
    Float,
    Matrix4,
    Enum,
    Ref
  };

  // Info
  const char* GetName() const;
  Var::Type GetType() const;
  const char* GetTypeName() const;
  uint32_t GetOffset() const;
  uint32_t GetSize() const;
    
  // Value
  std::string GetObjectValueAsString( const ae::Object* obj, std::function< std::string( const ae::Object* ) > getStringFromObjectPointer = nullptr ) const;
  bool SetObjectValueFromString( ae::Object* obj, const char* value, std::function< bool( const ae::Type*, const char*, ae::Object** ) > getObjectPointerFromString = nullptr ) const;
  bool SetObjectValue( ae::Object* obj, const ae::Object* value ) const;
  template < typename T > bool SetObjectValue( ae::Object* obj, const T& value ) const;
    
  // Types
  const class Enum* GetEnum() const;
  const ae::Type* GetRefType() const;

  //------------------------------------------------------------------------------
  // Internal
  //------------------------------------------------------------------------------
  const ae::Type* m_owner = nullptr;
  ae::Str32 m_name = "";
  Var::Type m_type;
  ae::Str32 m_typeName = "";
  uint32_t m_offset = 0;
  uint32_t m_size = 0;
  ae::TypeId m_refTypeId = ae::kInvalidTypeId; // @TODO: Need to use an id here in case type has not been registered yet
  mutable const class Enum* m_enum = nullptr;
};

//------------------------------------------------------------------------------
// External MetaType class
//------------------------------------------------------------------------------
class Type
{
public:
  ae::TypeId GetId() const;
    
  // Properties
  bool HasProperty( const char* prop ) const;
  uint32_t GetPropertyCount() const;
  const char* GetPropertyName( uint32_t propIndex ) const;
  uint32_t GetPropertyValueCount( uint32_t propIndex ) const;
  uint32_t GetPropertyValueCount( const char* propName ) const;
  const char* GetPropertyValue( uint32_t propIndex, uint32_t valueIndex ) const;
  const char* GetPropertyValue( const char* propName, uint32_t valueIndex ) const;
    
  // Vars
  uint32_t GetVarCount() const;
  const ae::Var* GetVarByIndex( uint32_t i ) const;
  const ae::Var* GetVarByName( const char* name ) const;

  // C++ type info
  template < typename T = ae::Object > T* New( void* obj ) const;
  uint32_t GetSize() const;
  uint32_t GetAlignment() const;
  const char* GetName() const;
  bool HasNew() const;
  bool IsAbstract() const;
  bool IsPolymorphic() const;
  bool IsDefaultConstructible() const;

  // Inheritance info
  const char* GetParentTypeName() const;
  const Type* GetParentType() const;
  bool IsType( const Type* otherType ) const;
  template < typename T > bool IsType() const;
    
  //------------------------------------------------------------------------------
  // Internal
  //------------------------------------------------------------------------------
  template < typename T >
  typename std::enable_if< !std::is_abstract< T >::value && std::is_default_constructible< T >::value, void >::type
  Init( const char* name, uint32_t index );
  template < typename T >
  typename std::enable_if< std::is_abstract< T >::value || !std::is_default_constructible< T >::value, void >::type
  Init( const char* name, uint32_t index );
  void m_AddProp( const char* prop, const char* value );
  void m_AddVar( const Var& var );
private:
  ae::Object* ( *m_placementNew )( ae::Object* ) = nullptr;
  ae::Str32 m_name;
  ae::TypeId m_id = ae::kInvalidTypeId;
  uint32_t m_size = 0;
  uint32_t m_align = 0;
  ae::Map< ae::Str32, ae::Array< ae::Str32 >, kMaxMetaProps > m_props;
  ae::Array< Var > m_vars = AE_ALLOC_TAG_META;
  ae::Str32 m_parent;
  bool m_isAbstract = false;
  bool m_isPolymorphic = false;
  bool m_isDefaultConstructible = false;
};

//------------------------------------------------------------------------------
// ae::Cast
//------------------------------------------------------------------------------
template< typename T, typename C > const T* Cast( const C* obj );
template< typename T, typename C > T* Cast( C* obj );

//! @}

} // ae end

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
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
namespace ae {

template < typename T >
const char* GetTypeName()
{
  const char* typeName = typeid( typename std::decay< T >::type ).name();
#ifdef _MSC_VER
  // @TODO: Support pointers to types
  if ( strncmp( typeName, "class ", 6 ) == 0 )
  {
    typeName += 6;
  }
  else if ( strncmp( typeName, "struct ", 7 ) == 0 )
  {
    typeName += 7;
  }
#else
  // @NOTE: Demangle calls realloc on given buffer
  int status = 1;
  static size_t s_length = 32;
  static char* s_buffer = (char*)malloc( s_length );
  s_buffer = abi::__cxa_demangle( typeName, s_buffer, &s_length, &status );
  if ( status == 0 )
  {
    int32_t len = strlen( s_buffer );
    while ( s_buffer[ len - 1 ] == '*' )
    {
      s_buffer[ len - 1 ] = 0;
      len--;
    }
    return s_buffer;
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
void LogInternal( std::stringstream& os, const char* message );
void LogFormat( std::stringstream& os, uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format );
template < typename T, typename... Args >
void LogInternal( std::stringstream& os, const char* format, T value, Args... args );
template < typename... Args >
void LogInternal( uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format, Args... args );
// extern const char* LogLevelNames[ 6 ];

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
  os << std::boolalpha;
  LogFormat( os, severity, filePath, line, assertInfo, format );
  LogInternal( os, format, args... );
}

//------------------------------------------------------------------------------
// C++ style allocation functions
//------------------------------------------------------------------------------
#if _AE_EMSCRIPTEN_
// @NOTE: Max alignment is 8 bytes, sizeof(long double) https://github.com/emscripten-core/emscripten/issues/10072
const uint32_t _kDefaultAlignment = 8;
#else
const uint32_t _kDefaultAlignment = 16;
#endif
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
  AE_ASSERT_MSG( (intptr_t)base % _kDefaultAlignment == 0, "Alignment off by # bytes", (intptr_t)base % _kDefaultAlignment );
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
  AE_ASSERT_MSG( (intptr_t)base % _kDefaultAlignment == 0, "Alignment off by # bytes", (intptr_t)base % _kDefaultAlignment );
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
Scratch< T >::Scratch( ae::Tag tag, uint32_t count )
{
  m_count = count;
  m_data = ae::NewArray< T >( tag, count );
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

inline ae::Vec3 Min( ae::Vec3 v0, ae::Vec3 v1 )
{
  return ae::Vec3(
    Min( v0.x, v1.x ),
    Min( v0.y, v1.y ),
    Min( v0.z, v1.z )
  );
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

inline ae::Vec3 Max( ae::Vec3 v0, ae::Vec3 v1 )
{
  return ae::Vec3(
    Max( v0.x, v1.x ),
    Max( v0.y, v1.y ),
    Max( v0.z, v1.z )
  );
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
  if ( snappiness == 0.0f )
  {
    return value;
  }
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
    // @TODO: Needed for ae::Color, support types without lerp
    return start.Lerp( end, t ); //return start + ( ( end - start ) * t );
  }
}

static bool _HACK_randInit = false;

inline int32_t Random( int32_t min, int32_t max )
{
  if ( !_HACK_randInit )
  {
    srand( (uint32_t)time( 0 ) );
    _HACK_randInit = true;
  }

  if ( min >= max )
  {
    return min;
  }
  return min + ( rand() % ( max - min ) );
}

inline float Random( float min, float max )
{
  if ( !_HACK_randInit )
  {
    srand( (uint32_t)time( 0 ) );
    _HACK_randInit = true;
  }

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
bool VecT< T >::operator==( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) == 0;
}

template < typename T >
bool VecT< T >::operator!=( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) != 0;
}

template < typename T >
float VecT< T >::operator[]( uint32_t idx ) const
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
float& VecT< T >::operator[]( uint32_t idx )
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
T VecT< T >::operator+( const T& v ) const
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
void VecT< T >::operator+=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(T::data); i++ )
  {
    self.data[ i ] += v.data[ i ];
  }
}

template < typename T >
T VecT< T >::operator-() const
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
T VecT< T >::operator-( const T& v ) const
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
void VecT< T >::operator-=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] -= v.data[ i ];
  }
}

template < typename T >
T VecT< T >::operator*( const T& v ) const
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
T VecT< T >::operator/( const T& v ) const
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
void VecT< T >::operator*=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] *= v.data[ i ];
  }
}

template < typename T >
void VecT< T >::operator/=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] /= v.data[ i ];
  }
}

template < typename T >
T VecT< T >::operator*( float s ) const
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
void VecT< T >::operator*=( float s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] *= s;
  }
}

template < typename T >
T VecT< T >::operator/( float s ) const
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
void VecT< T >::operator/=( float s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] /= s;
  }
}

template < typename T >
float VecT< T >::Dot( const T& v0, const T& v1 )
{
  float result = 0.0f;
  for ( uint32_t i = 0; i < countof(v0.data); i++ )
  {
    result += v0.data[ i ] * v1.data[ i ];
  }
  return result;
}

template < typename T >
float VecT< T >::Dot( const T& v ) const
{
  return Dot( *(T*)this, v );
}

template < typename T >
float VecT< T >::Length() const
{
  return sqrt( LengthSquared() );
}

template < typename T >
float VecT< T >::LengthSquared() const
{
  return Dot( *(T*)this );
}

template < typename T >
float VecT< T >::Normalize()
{
  float length = Length();
  *(T*)this /= length;
  return length;
}

template < typename T >
float VecT< T >::SafeNormalize( float epsilon )
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
T VecT< T >::NormalizeCopy() const
{
  T result = *(T*)this;
  result.Normalize();
  return result;
}

template < typename T >
T VecT< T >::SafeNormalizeCopy( float epsilon ) const
{
  T result = *(T*)this;
  result.SafeNormalize( epsilon );
  return result;
}

template < typename T >
float VecT< T >::Trim( float trimLength )
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
T operator*( float f, const VecT< T >& v )
{
  return v * f;
}

template < typename T >
inline std::ostream& operator<<( std::ostream& os, const VecT< T >& v )
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
inline Vec2::Vec2( struct Int2 i2 ) : x( (float)i2.x ), y( (float)i2.y ) {}
inline Vec2 Vec2::FromAngle( float angle ) { return Vec2( ae::Cos( angle ), ae::Sin( angle ) ); }
inline Int2 Vec2::NearestCopy() const { return Int2( (int32_t)(x + 0.5f), (int32_t)(y + 0.5f) ); }
inline Int2 Vec2::FloorCopy() const { return Int2( (int32_t)floorf( x ), (int32_t)floorf( y ) ); }
inline Int2 Vec2::CeilCopy() const { return Int2( (int32_t)ceilf( x ), (int32_t)ceilf( y ) ); }
inline Vec2 Vec2::RotateCopy( float rotation ) const
{
  float sinTheta = std::sin( rotation );
  float cosTheta = std::cos( rotation );
  return Vec2( x * cosTheta - y * sinTheta, x * sinTheta + y * cosTheta );
}
inline float Vec2::GetAngle() const
{
  if ( LengthSquared() < 0.0001f )
  {
    return 0.0f;
  }
  return ae::Atan2( y, x );
}
inline Vec2 Vec2::Reflect( Vec2 v, Vec2 n ) const
{
  return n * ( 2.0f * v.Dot( n ) / n.LengthSquared() ) - v;
}

//------------------------------------------------------------------------------
// ae::Vec3 member functions
//------------------------------------------------------------------------------
inline Vec3::Vec3( float v ) : x( v ), y( v ), z( v ), pad( 0.0f ) {}
inline Vec3::Vec3( float x, float y, float z ) : x( x ), y( y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( const float* v3 ) : x( v3[ 0 ] ), y( v3[ 1 ] ), z( v3[ 2 ] ), pad( 0.0f ) {}
inline Vec3::Vec3( struct Int3 i3 ) : x( (float)i3.x ), y( (float)i3.y ), z( (float)i3.z ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy, float z ) : x( xy.x ), y( xy.y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy ) : x( xy.x ), y( xy.y ), z( 0.0f ), pad( 0.0f ) {}
inline Vec3::operator Vec2() const { return Vec2( x, y ); }
inline Vec2 Vec3::GetXY() const { return Vec2( x, y ); }
inline Vec2 Vec3::GetXZ() const { return Vec2( x, z ); }
inline Int3 Vec3::NearestCopy() const { return Int3( (int32_t)(x + 0.5f), (int32_t)(y + 0.5f), (int32_t)(z + 0.5f) ); }
inline Int3 Vec3::FloorCopy() const { return Int3( (int32_t)floorf( x ), (int32_t)floorf( y ), (int32_t)floorf( z ) ); }
inline Int3 Vec3::CeilCopy() const { return Int3( (int32_t)ceilf( x ), (int32_t)ceilf( y ), (int32_t)ceilf( z ) ); }
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
inline Vec3 Vec3::ZeroAxisCopy( Vec3 axis ) const
{
  Vec3 r = *this;
  r.ZeroAxis( axis );
  return r;
}
inline Vec3 Vec3::ZeroDirectionCopy( Vec3 direction ) const
{
  Vec3 r = *this;
  r.ZeroDirection( direction );
  return r;
}
inline Vec3 Vec3::ProjectPoint( const Matrix4& projection, Vec3 p )
{
  Vec4 projected = projection * Vec4( p, 1.0f );
  return projected.GetXYZ() / projected.w;
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
// ae::Int2 shared member functions
// ae::Int3 shared member functions
//------------------------------------------------------------------------------
template < typename T >
bool IntT< T >::operator==( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) == 0;
}

template < typename T >
bool IntT< T >::operator!=( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) != 0;
}

template < typename T >
int32_t IntT< T >::operator[]( uint32_t idx ) const
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
int32_t& IntT< T >::operator[]( uint32_t idx )
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
T IntT< T >::operator+( const T& v ) const
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
void IntT< T >::operator+=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(T::data); i++ )
  {
    self.data[ i ] += v.data[ i ];
  }
}

template < typename T >
T IntT< T >::operator-() const
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
T IntT< T >::operator-( const T& v ) const
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
void IntT< T >::operator-=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] -= v.data[ i ];
  }
}

template < typename T >
T IntT< T >::operator*( const T& v ) const
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
T IntT< T >::operator/( const T& v ) const
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
void IntT< T >::operator*=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] *= v.data[ i ];
  }
}

template < typename T >
void IntT< T >::operator/=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] /= v.data[ i ];
  }
}

template < typename T >
T IntT< T >::operator*( int32_t s ) const
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
void IntT< T >::operator*=( int32_t s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] *= s;
  }
}

template < typename T >
T IntT< T >::operator/( int32_t s ) const
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
void IntT< T >::operator/=( int32_t s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] /= s;
  }
}

template < typename T >
inline std::ostream& operator<<( std::ostream& os, const IntT< T >& v )
{
  constexpr uint32_t count = countof( T::data );
  for ( uint32_t i = 0; i < count - 1; i++ )
  {
    os << v[ i ] << " ";
  }
  return os << v[ count - 1 ];
}

//------------------------------------------------------------------------------
// ae::Int2 member functions
//------------------------------------------------------------------------------
inline Int2::Int2( int32_t _v ) : x( _v ), y( _v ) {}
inline Int2::Int2( const struct Int3& v ) : x( v.x ), y( v.y ) {}
inline Int2::Int2( int32_t _x, int32_t _y ) : x( _x ), y( _y ) {}

//------------------------------------------------------------------------------
// ae::Int3 member functions
//------------------------------------------------------------------------------
inline Int3::Int3( int32_t _v ) : x( _v ), y( _v ), z( _v ), pad( 0 ) {}
inline Int3::Int3( int32_t _x, int32_t _y, int32_t _z ) : x( _x ), y( _y ), z( _z ), pad( 0 ) {}
inline Int3::Int3( Int2 xy, int32_t _z ) : x( xy.x ), y( xy.y ), z( _z ), pad( 0 ) {}
inline Int3::Int3( const int32_t( &v )[ 3 ] ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
inline Int3::Int3( const int32_t( &v )[ 4 ] ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
inline Int3::Int3( int32_t*& v ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
inline Int3::Int3( const int32_t*& v ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
inline Int2 Int3::GetXY() const { return Int2( x, y ); }

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
inline float Color::SRGBToRGB( float x ) { return powf( x , 2.2f ); }
inline float Color::RGBToSRGB( float x ) { return powf( x, 1.0f / 2.2f ); }

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
// ae::Array functions
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
#if _AE_DEBUG_
    AE_ASSERT_MSG( m_array == (T*)&m_storage, "Static array reference has been overwritten" );
#endif
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
uint32_t Array< T, N >::m_GetNextSize() const
{
  if ( m_size == 0 )
  {
    return ae::Max( 1u, 32u / (uint32_t)sizeof(T) ); // @NOTE: Initially allocate 32 bytes (rounded down) of type
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
void Map< K, V, N >::Reserve( uint32_t count )
{
  m_entries.Reserve( count );
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

//------------------------------------------------------------------------------
// ae::BinaryStream member functions
//------------------------------------------------------------------------------
template < uint32_t N >
void BinaryStream::SerializeString( Str< N >& str )
{
  if ( IsWriter() )
  {
    const uint16_t len = str.Length();
    SerializeUint16( len );
    SerializeRaw( str.c_str(), len );
  }
  else if ( IsReader() )
  {
    uint16_t len = 0;
    SerializeUint16( len );
    if ( !IsValid() )
    {
      return;
    }

    if ( len > Str< N >::MaxLength() || GetRemaining() < len )
    {
      Invalidate();
    }
    else
    {
      str = Str< N >( len, (const char*)PeekData() );
      Discard( len );
    }
  }
}

template < uint32_t N >
void BinaryStream::SerializeString( const Str< N >& str )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  const uint16_t len = str.Length();
  SerializeUint16( len );
  SerializeRaw( str.c_str(), len );
}

template <typename C> static constexpr std::true_type _ae_serialize_test( decltype( std::declval<C&&>().Serialize( nullptr ) )* ) noexcept;
template <typename C> static constexpr std::false_type _ae_serialize_test( ... ) noexcept;
template <typename C> constexpr decltype( _ae_serialize_test<C>( nullptr ) ) _ae_has_Serialize{};

template< typename T >
typename std::enable_if<_ae_has_Serialize<T>>::type
BinaryStream_SerializeObjectInternal( BinaryStream* stream, T& v )
{
  v.Serialize( stream );
}

template< typename T >
typename std::enable_if<_ae_has_Serialize<T>>::type
BinaryStream_SerializeObjectInternalConst( BinaryStream* stream, const T& v )
{
  v.Serialize( stream );
}

template< typename T >
typename std::enable_if<!_ae_has_Serialize<T>>::type
BinaryStream_SerializeObjectInternal( BinaryStream* stream, T& v, ... )
{
  Serialize( stream, &v );
}

template< typename T >
typename std::enable_if<!_ae_has_Serialize<T>>::type
BinaryStream_SerializeObjectInternalConst( BinaryStream* stream, const T& v, ... )
{
  Serialize( stream, &v );
}

template< typename T >
void BinaryStream::SerializeObject( T& v )
{
  BinaryStream_SerializeObjectInternal( this, v );
}

template< typename T >
void BinaryStream::SerializeObject( const T& v )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const type." );
  BinaryStream_SerializeObjectInternalConst( this, v );
}

template< typename T >
void BinaryStream::SerializeRaw( T& v )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    AE_ASSERT( m_offset + sizeof(T) <= m_length );
    memcpy( &v, m_data + m_offset, sizeof(T) );
    m_offset += sizeof(T);
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    if ( m_data )
    {
      AE_ASSERT( sizeof(T) <= m_length - m_offset );
      memcpy( m_data + m_offset, &v, sizeof(T) );
      m_offset += sizeof(T);
    }
    else
    {
      Array< uint8_t >& array = m_GetArray();
      array.Append( (uint8_t*)&v, sizeof(T) );
      m_offset = array.Length();
      m_length = array.Size();
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with ae::BinaryStream::Writer or ae::BinaryStream::Reader static functions." );
  }
}

template< typename T >
void BinaryStream::SerializeRaw( const T& v )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const type." );
  SerializeRaw( *const_cast< T* >( &v ) );
}

template< uint32_t N >
void BinaryStream::SerializeArray( char (&str)[ N ] )
{
  // @TODO: Cleanup and use m_SerializeArrayLength()
  uint16_t len = 0;
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    AE_ASSERT( m_offset + sizeof(len) <= m_length ); // @TODO: Remove this and invalidate stream instead
    memcpy( &len, m_data + m_offset, sizeof(len) );
    m_offset += sizeof(len);

    AE_ASSERT( m_offset + len + 1 <= m_length ); // @TODO: Remove this and invalidate stream instead
    memcpy( str, m_data + m_offset, len );
    str[ len ] = 0;
    m_offset += len;
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    len = strlen( str );

    if ( m_data )
    {
      AE_ASSERT( sizeof(len) <= m_length - m_offset ); // @TODO: Remove this and invalidate stream instead
      memcpy( m_data + m_offset, &len, sizeof(len) );
      m_offset += sizeof(len);

      AE_ASSERT( len <= m_length - m_offset ); // @TODO: Remove this and invalidate stream instead
      memcpy( m_data + m_offset, str, len );
      m_offset += len;
    }
    else
    {
      Array< uint8_t >& array = m_GetArray();
      array.Append( (uint8_t*)&len, sizeof(len) );
      array.Append( (uint8_t*)&str, len );
      m_offset = array.Length();
      m_length = array.Size();
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with ae::BinaryStream::Writer or ae::BinaryStream::Reader static functions." );
  }
}

template< uint32_t N >
void BinaryStream::SerializeArray( const char (&str)[ N ] )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  SerializeArray( const_cast< char[ N ] >( str ) );
}

template < typename T >
void BinaryStream::SerializeObjectConditional( T* obj )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    uint16_t length = 0;
    SerializeRaw( &length, sizeof( length ) );

    if ( length )
    {
      if ( obj )
      {
        // Read object
        uint32_t prevOffset = m_offset;
        SerializeObject( *obj );

        // Object should always read everything it wrote
        if ( prevOffset + length != m_offset )
        {
          Invalidate();
        }
      }
      else
      {
        Discard( length );
      }
    }
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    if ( obj )
    {
      // Reserve length
      uint32_t lengthOffset = m_offset;
      uint16_t lengthFake = 0xCDCD;
      SerializeRaw( &lengthFake, sizeof( lengthFake ) ); // Raw to avoid compression

      // Write object
      uint32_t prevOffset = m_offset;
      SerializeObject( *obj );

      // Rewrite previously serialized value
      uint32_t writeLength = m_offset - prevOffset;
      if ( writeLength > MaxValue< uint16_t >() )
      {
        Invalidate(); // Object is too large to serialize
      }
      else if ( IsValid() ) // Can become invalid while writing by running out of memory
      {
        // @NOTE: Use length offset from above (and not a pointer into the data buffer) because the data buffer may not yet bet allocated or may be reallocated while serializing
        AE_ASSERT( GetData() );
        uint16_t* length = (uint16_t*)( GetData() + lengthOffset );
        AE_ASSERT( *length == 0xCDCD );
        *length = writeLength;
      }
    }
    else
    {
      uint16_t length = 0;
      SerializeRaw( &length, sizeof( length ) ); // Raw to avoid compression
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with ae::BinaryStream::Writer or ae::BinaryStream::Reader static functions." );
  }
}

//------------------------------------------------------------------------------
// Internal meta state
//------------------------------------------------------------------------------
std::map< ae::Str32, class Type* >& _GetTypeNameMap();
std::map< ae::TypeId, class Type* >& _GetTypeIdMap();
std::vector< class Type* >& _GetTypes();
template< typename T > ae::Object* _PlacementNew( ae::Object* d ) { return new( d ) T(); }

//------------------------------------------------------------------------------
// External meta initialization helpers
//------------------------------------------------------------------------------
template < typename T >
struct _TypeName
{
  static const char* Get();
};

template < typename T >
struct _VarType
{
  static Var::Type GetType();
  static const char* GetName();
};

template < typename Parent, typename This >
Inheritor< Parent, This >::Inheritor()
{
  const ae::Type* t = ae::GetTypeByName( ae::_TypeName< This >::Get() );
  AE_ASSERT_MSG( t, "No inheritor type" );
  ae::Object::_metaTypeId = t->GetId();
  ae::Object::_typeName = ae::_TypeName< This >::Get();
}

template < typename Parent, typename This >
const char* Inheritor< Parent, This >::GetParentTypeName()
{
  return ae::_TypeName< Parent >::Get();
}

template < typename Parent, typename This >
const ae::Type* Inheritor< Parent, This >::GetParentType()
{
  return ae::GetType( ae::_TypeName< Parent >::Get() );
}

//------------------------------------------------------------------------------
// Internal meta initialization functions
//------------------------------------------------------------------------------
template< typename T >
void _DefineType( Type* type, uint32_t index );

template < typename T >
struct _TypeCreator
{
  _TypeCreator( const char *typeName )
  {
    static Type type;
    // ae::TypeId id = m_GetNextTypeId();
    _DefineType< T >( &type, 0 );
    _GetTypeNameMap()[ typeName ] = &type;
    _GetTypeIdMap()[ type.GetId() ] = &type; // @TODO: Should check for hash collision
    _GetTypes().push_back( &type );
  }
};

template< typename C, typename V, uint32_t Offset >
struct _VarCreator
{
  _VarCreator( const char* typeName, const char* varName )
  {
    ae::Type* type = _GetTypeNameMap().find( typeName )->second;
    AE_ASSERT( type );
      
    Var var;
    var.m_owner = type;
    var.m_name = varName;
    var.m_type = ae::_VarType< V >::GetType();
    var.m_typeName = ae::_VarType< V >::GetName();
    var.m_refTypeId = GetTypeIdFromName( ae::_VarType< V >::GetRefTypeName() );
#if !_AE_WINDOWS_
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
    var.m_offset = Offset; // @TODO: Verify var is not member of base class
#if !_AE_WINDOWS_
  #pragma clang diagnostic pop
#endif
    var.m_size = sizeof(V);

    type->m_AddVar( var );
  }
};
  
template< typename C >
struct _PropCreator
{
  _PropCreator( const char* typeName, const char* propName, const char* propValue )
  {
    ae::Type* type = _GetTypeNameMap().find( typeName )->second;
    type->m_AddProp( propName, propValue );
  }
};
  
// @NOTE: Internal. Non-specialized GetEnum() has no implementation so templated GetEnum() calls (defined
// with AE_DEFINE_ENUM_CLASS, AE_META_ENUM, and AE_META_ENUM_PREFIX) will call the specialized function.
template < typename T >
const Enum* GetEnum();
  
template < typename E, typename T = typename std::underlying_type< E >::type >
struct _EnumCreator
{
  _EnumCreator( const char* typeName, std::string strMap )
  {
    ae::Enum* enumType = ae::Enum::s_Get( typeName, true, sizeof( T ), std::is_signed< T >::value );
      
    strMap.erase( std::remove( strMap.begin(), strMap.end(), ' ' ), strMap.end() );
    strMap.erase( std::remove( strMap.begin(), strMap.end(), '(' ), strMap.end() );
    std::vector< std::string > enumTokens( m_SplitString( strMap, ',' ) );

    T currentValue = 0;
    for ( auto iter = enumTokens.begin(); iter != enumTokens.end(); ++iter )
    {
      std::string enumName;
      if ( iter->find( '=' ) == std::string::npos )
      {
        enumName = *iter;
      }
      else
      {
        std::vector<std::string> enumNameValue( m_SplitString( *iter, '=' ) );
        enumName = enumNameValue[ 0 ];
        if ( std::is_unsigned< T >::value )
        {
          currentValue = static_cast< T >( std::stoull( enumNameValue[ 1 ], 0, 0 ) );
        }
        else
        {
          currentValue = static_cast< T >( std::stoll( enumNameValue[ 1 ], 0, 0 ) );
        }
      }
        
      enumType->m_AddValue( enumName.c_str(), currentValue );
      currentValue++;
    }
  }
    
private:
  static std::vector< std::string > m_SplitString( std::string str, char separator )
  {
    std::vector< std::string > result;

    std::string item;
    std::stringstream stringStream( str );
    while ( std::getline( stringStream, item, separator ) )
    {
      result.push_back( item );
    }

    return result;
  }
};
  
template < typename T >
class _EnumCreator2
{
public:
  _EnumCreator2( const char* typeName )
  {
    ae::Enum::s_Get( typeName, true, sizeof( T ), std::is_signed< T >::value );
  }
    
  _EnumCreator2( const char* valueName, T value )
  {
    const char* prefix = ae::_VarType< T >::GetPrefix();
    uint32_t prefixLen = (uint32_t)strlen( prefix );
    AE_ASSERT( prefixLen < strlen( valueName ) );
    AE_ASSERT( memcmp( prefix, valueName, prefixLen ) == 0 );
      
    ae::Enum* enumType = const_cast< ae::Enum* >( ae::GetEnum< T >() );
    AE_ASSERT_MSG( enumType, "Could not register enum value '#'. No registered Enum.", valueName );
    enumType->m_AddValue( valueName + prefixLen, (int32_t)value );
  }
};

} // ae end

//------------------------------------------------------------------------------
// Internal meta var registration
//------------------------------------------------------------------------------
#define _ae_DefineMetaVarType( t, e ) \
template <> \
struct ae::_VarType< t > { \
static ae::Var::Type GetType() { return ae::Var::e; } \
static const char* GetName() { return #t; } \
static const char* GetRefTypeName() { return ""; } \
};

_ae_DefineMetaVarType( uint8_t, UInt8 );
_ae_DefineMetaVarType( uint16_t, UInt16 );
_ae_DefineMetaVarType( uint32_t, UInt32 );
_ae_DefineMetaVarType( int8_t, Int8 );
_ae_DefineMetaVarType( int16_t, Int16 );
_ae_DefineMetaVarType( int32_t, Int32 );
_ae_DefineMetaVarType( bool, Bool );
_ae_DefineMetaVarType( float, Float );
_ae_DefineMetaVarType( ae::Matrix4, Matrix4 );

template < uint32_t N >
struct ae::_VarType< ae::Str<N> >
{
  static ae::Var::Type GetType() { return ae::Var::String; }
  static const char* GetName() { return "String"; }
  static const char* GetRefTypeName() { return ""; }
};

template < typename T >
struct ae::_VarType< T* >
{
  static ae::Var::Type GetType()
  {
    static_assert( std::is_base_of< ae::Object, T >::value, "AE_REGISTER_CLASS_VAR refs must have base type ae::Object" );
    return ae::Var::Ref;
  }
  static const char* GetName() { return "Ref"; }
  static const char* GetRefTypeName() { return ae::GetTypeName< T >(); }
};

template < typename T >
bool ae::Type::IsType() const
{
  const Type* type = GetType< T >();
  AE_ASSERT( type );
  return IsType( type );
}

template < typename T >
const ae::Type* ae::GetType()
{
  const char* typeName = ae::GetTypeName< T >();
  auto it = _GetTypeNameMap().find( typeName );
  if ( it != _GetTypeNameMap().end() )
  {
    return it->second;
  }
  else
  {
    AE_ASSERT_MSG( false, "No meta info for type name: #", typeName );
    return nullptr;
  }
}

template < typename T >
std::string ae::Enum::GetNameFromValue( T value )
{
  const Enum* enumType = GetEnum< T >();
  AE_ASSERT( enumType );
  return enumType->m_enumValueToName.Get( (int32_t)value, "" );
}

template < typename T >
T ae::Enum::GetValueFromString( const char* str, T defaultValue )
{
  const Enum* enumType = GetEnum< T >();
  AE_ASSERT_MSG( enumType, "Value '#' has no Enum #", str, typeid(T).name() ); // TODO: Pretty print
  enumType->GetValueFromString( str, &defaultValue );
  return defaultValue;
}

template < typename T >
std::string ae::Enum::GetNameByValue( T value ) const
{
  return m_enumValueToName.Get( (int32_t)value, "" );
}

template < typename T >
bool ae::Enum::GetValueFromString( const char* str, T* valueOut ) const
{
  int32_t value = 0;
  if ( m_enumNameToValue.TryGet( str, &value ) ) // Set object var with named enum value
  {
    *valueOut = (T)value;
    return true;
  }
  else if ( isdigit( str[ 0 ] ) || str[ 0 ] == '-' ) // Set object var with a numerical enum value
  {
    value = atoi( str );
    if ( HasValue( value ) )
    {
      *valueOut = (T)value;
      return true;
    }
  }
  
  return false;
}

template < typename T >
bool ae::Enum::HasValue( T value ) const
{
  return m_enumValueToName.TryGet( value );
}

template < typename T >
T* ae::Type::New( void* obj ) const
{
  AE_ASSERT( obj );
  AE_ASSERT_MSG( !m_isAbstract, "Placement new not available for abstract type: #", m_name.c_str() );
  AE_ASSERT_MSG( m_isDefaultConstructible, "Placement new not available for type without default constructor: #", m_name.c_str() );
  AE_ASSERT( m_placementNew );
  AE_ASSERT( IsType< T >() );
  AE_ASSERT( (uint64_t)obj % GetAlignment() == 0 );
  return (T*)m_placementNew( (T*)obj );
}

template < typename T >
typename std::enable_if< !std::is_abstract< T >::value && std::is_default_constructible< T >::value, void >::type
ae::Type::Init( const char* name, uint32_t index )
{
  m_placementNew = &( _PlacementNew< T > );
  m_name = name;
  m_id = GetTypeIdFromName( name );
  m_size = sizeof( T );
  m_align = alignof( T );
  m_parent = T::GetParentTypeName();
  m_isAbstract = false;
  m_isPolymorphic = std::is_polymorphic< T >::value;
  m_isDefaultConstructible = true;
}
template < typename T >
typename std::enable_if< std::is_abstract< T >::value || !std::is_default_constructible< T >::value, void >::type
ae::Type::Init( const char* name, uint32_t index )
{
  m_placementNew = nullptr;
  m_name = name;
  m_id = GetTypeIdFromName( name );
  m_size = sizeof( T );
  m_align = 0;
  m_parent = T::GetParentTypeName();
  m_isAbstract = std::is_abstract< T >::value;
  m_isPolymorphic = std::is_polymorphic< T >::value;
  m_isDefaultConstructible = std::is_default_constructible< T >::value;
}

template < typename T >
bool ae::Var::SetObjectValue( ae::Object* obj, const T& value ) const
{
  AE_ASSERT( m_type != Ref );

  if ( !obj )
  {
    return false;
  }

  const ae::Type* objType = ae::GetTypeFromObject( obj );
  AE_ASSERT( objType );
  AE_ASSERT_MSG( objType->IsType( m_owner ), "Attempting to set var on '#' with unrelated type '#'", objType->GetName(), m_owner->GetName() );

  Var::Type typeCheck = ae::_VarType< T >::GetType();
  AE_ASSERT( typeCheck == m_type );
  AE_ASSERT( m_size == sizeof( T ) );

  T* varData = reinterpret_cast<T*>( (uint8_t*)obj + m_offset );
  *varData = value;

  return true;
}

template< typename T, typename C >
const T* ae::Cast( const C* obj )
{
  static_assert( std::is_base_of< C, T >::value || std::is_base_of< T, C >::value, "Unrelated types" );
  return dynamic_cast<const T*>( obj );
}

template< typename T, typename C >
T* ae::Cast( C* obj )
{
  static_assert( std::is_base_of< C, T >::value || std::is_base_of< T, C >::value, "Unrelated types" );
  return dynamic_cast<T*>( obj );
}

#endif // AE_AETHER_H

//------------------------------------------------------------------------------
// The following should be compiled into a single module and linked with the
// application. It's worth putting this in it's own module to limit the
// number of dependencies brought into your own code. For instance 'Windows.h'
// is included and this can easily cause naming conflicts with gameplay/engine
// code.
// Usage inside a cpp/mm file is:
//
// // ae.cpp/ae.mm EXAMPLE START
//
// #define AE_MAIN
// #define AE_USE_MODULES // C++ Modules are optional
// #include "aether.h"
//
// // ae.cpp/ae.mm EXAMPLE END
//------------------------------------------------------------------------------
#if defined(AE_MAIN) && !defined(AE_MAIN_ALREADY)
#define AE_MAIN_ALREADY
#if _AE_APPLE_ && !defined(__OBJC__)
#error "AE_MAIN must be defined in an Objective-C file on Apple platforms"
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
#elif _AE_APPLE_
  #define GL_SILENCE_DEPRECATION
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

//------------------------------------------------------------------------------
// Platform includes, required for logging, windowing, file io
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #define WIN32_LEAN_AND_MEAN 1
  #include <Windows.h>
  #include <shellapi.h>
  #include <Shlobj_core.h>
  #include <commdlg.h>
  #include "processthreadsapi.h" // For GetCurrentProcessId()
  #include <filesystem> // @HACK: Shouldn't need this just for Windows
#elif _AE_APPLE_
  #include <sys/sysctl.h>
  #include <unistd.h>
  #ifdef AE_USE_MODULES
    @import AppKit;
    @import Carbon;
    @import Cocoa;
    @import CoreFoundation;
    @import OpenGL;
    @import OpenAL;
  #else
    #include <Cocoa/Cocoa.h>
    #include <Carbon/Carbon.h>
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
  #endif
#elif _AE_LINUX_
  #include <unistd.h>
  #include <pwd.h>
  #include <sys/stat.h>
#endif
#include <thread>
#include <random>

//------------------------------------------------------------------------------
// Platform functions internal implementation
//------------------------------------------------------------------------------
namespace ae {

uint32_t GetPID()
{
#if _AE_WINDOWS_
  return GetCurrentProcessId();
#elif _AE_EMSCRIPTEN_
  return 0;
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
// ae::Matrix4 member functions
//------------------------------------------------------------------------------
Matrix4 Matrix4::Identity()
{
  Matrix4 r;
  r.data[ 0 ] = 1; r.data[ 4 ] = 0; r.data[ 8 ] = 0;  r.data[ 12 ] = 0;
  r.data[ 1 ] = 0; r.data[ 5 ] = 1; r.data[ 9 ] = 0;  r.data[ 13 ] = 0;
  r.data[ 2 ] = 0; r.data[ 6 ] = 0; r.data[ 10 ] = 1; r.data[ 14 ] = 0;
  r.data[ 3 ] = 0; r.data[ 7 ] = 0; r.data[ 11 ] = 0; r.data[ 15 ] = 1;
  return r;
}

Matrix4 Matrix4::Translation( const Vec3& t )
{
  Matrix4 r;
  r.data[ 0 ] = 1.0f; r.data[ 4 ] = 0.0f; r.data[ 8 ] = 0.0f;  r.data[ 12 ] = t.x;
  r.data[ 1 ] = 0.0f; r.data[ 5 ] = 1.0f; r.data[ 9 ] = 0.0f;  r.data[ 13 ] = t.y;
  r.data[ 2 ] = 0.0f; r.data[ 6 ] = 0.0f; r.data[ 10 ] = 1.0f; r.data[ 14 ] = t.z;
  r.data[ 3 ] = 0.0f; r.data[ 7 ] = 0.0f; r.data[ 11 ] = 0.0f; r.data[ 15 ] = 1.0f;
  return r;
}

Matrix4 Matrix4::Rotation( Vec3 forward0, Vec3 up0, Vec3 forward1, Vec3 up1 )
{
  // Remove rotation
  forward0.Normalize();
  up0.Normalize();

  Vec3 right0 = forward0.Cross( up0 );
  right0.Normalize();
  up0 = right0.Cross( forward0 );

  Matrix4 removeRotation;
  memset( &removeRotation, 0, sizeof( removeRotation ) );
  removeRotation.SetRow( 0, right0 ); // right -> ( 1, 0, 0 )
  removeRotation.SetRow( 1, forward0 ); // forward -> ( 0, 1, 0 )
  removeRotation.SetRow( 2, up0 ); // up -> ( 0, 0, 1 )
  removeRotation.data[ 15 ] = 1;

  // Rotate
  forward1.Normalize();
  up1.Normalize();

  Vec3 right1 = forward1.Cross( up1 );
  right1.Normalize();
  up1 = right1.Cross( forward1 );

  Matrix4 newRotation;
  memset( &newRotation, 0, sizeof( newRotation ) );
  // Set axis vector to invert (transpose)
  newRotation.SetAxis( 0, right1 ); // ( 1, 0, 0 ) -> right
  newRotation.SetAxis( 1, forward1 ); // ( 0, 1, 0 ) -> forward
  newRotation.SetAxis( 2, up1 ); // ( 0, 0, 1 ) -> up
  newRotation.data[ 15 ] = 1;

  return newRotation * removeRotation;
}

Matrix4 Matrix4::RotationX( float angle )
{
  Matrix4 r;
  r.data[ 0 ] = 1.0f; r.data[ 4 ] = 0.0f;          r.data[ 8 ] = 0.0f;           r.data[ 12 ] = 0.0f;
  r.data[ 1 ] = 0.0f; r.data[ 5 ] = cosf( angle ); r.data[ 9 ] = -sinf( angle ); r.data[ 13 ] = 0.0f;
  r.data[ 2 ] = 0.0f; r.data[ 6 ] = sinf( angle ); r.data[ 10 ] = cosf( angle ); r.data[ 14 ] = 0.0f;
  r.data[ 3 ] = 0.0f; r.data[ 7 ] = 0.0f;          r.data[ 11 ] = 0.0f;          r.data[ 15 ] = 1.0f;
  return r;
}

Matrix4 Matrix4::RotationY( float angle )
{
  Matrix4 r;
  r.data[ 0 ] = cosf( angle );  r.data[ 4 ] = 0.0f; r.data[ 8 ] = sinf( angle );  r.data[ 12 ] = 0.0f;
  r.data[ 1 ] = 0.0f;           r.data[ 5 ] = 1.0f; r.data[ 9 ] = 0.0f;           r.data[ 13 ] = 0.0f;
  r.data[ 2 ] = -sinf( angle ); r.data[ 6 ] = 0.0f; r.data[ 10 ] = cosf( angle ); r.data[ 14 ] = 0.0f;
  r.data[ 3 ] = 0.0f;           r.data[ 7 ] = 0.0f; r.data[ 11 ] = 0.0f;          r.data[ 15 ] = 1.0f;
  return r;
}

Matrix4 Matrix4::RotationZ( float angle )
{
  Matrix4 r;
  r.data[ 0 ] = cosf( angle ); r.data[ 4 ] = -sinf( angle ); r.data[ 8 ] = 0.0f;  r.data[ 12 ] = 0.0f;
  r.data[ 1 ] = sinf( angle ); r.data[ 5 ] = cosf( angle );  r.data[ 9 ] = 0.0f;  r.data[ 13 ] = 0.0f;
  r.data[ 2 ] = 0.0f;          r.data[ 6 ] = 0.0f;           r.data[ 10 ] = 1.0f; r.data[ 14 ] = 0.0f;
  r.data[ 3 ] = 0.0f;          r.data[ 7 ] = 0.0f;           r.data[ 11 ] = 0.0f; r.data[ 15 ] = 1.0f;
  return r;
}

Matrix4 Matrix4::Scaling( const Vec3& s )
{
  return Scaling( s.x, s.y, s.z );
}

Matrix4 Matrix4::Scaling( float sx, float sy, float sz )
{
  Matrix4 r;
  r.data[ 0 ] = sx;   r.data[ 4 ] = 0.0f; r.data[ 8 ] = 0.0f;  r.data[ 12 ] = 0.0f;
  r.data[ 1 ] = 0.0f; r.data[ 5 ] = sy;   r.data[ 9 ] = 0.0f;  r.data[ 13 ] = 0.0f;
  r.data[ 2 ] = 0.0f; r.data[ 6 ] = 0.0f; r.data[ 10 ] = sz;   r.data[ 14 ] = 0.0f;
  r.data[ 3 ] = 0.0f; r.data[ 7 ] = 0.0f; r.data[ 11 ] = 0.0f; r.data[ 15 ] = 1.0f;
  return r;
}

Matrix4 Matrix4::WorldToView( Vec3 position, Vec3 forward, Vec3 up )
{
  //xaxis.x  xaxis.y  xaxis.z  dot(xaxis, -eye)
  //yaxis.x  yaxis.y  yaxis.z  dot(yaxis, -eye)
  //zaxis.x  zaxis.y  zaxis.z  dot(zaxis, -eye)
  //0        0        0        1

  position = -position;
  forward.Normalize();
  up.Normalize();

  Vec3 right = forward.Cross( up );
  right.Normalize();
  up = right.Cross( forward );

  Matrix4 result;
  memset( &result, 0, sizeof( result ) );
  result.SetRow( 0, right );
  result.SetRow( 1, up );
  result.SetRow( 2, -forward ); // @TODO: Seems a little sketch to flip handedness here
  result.SetAxis( 3, Vec3( position.Dot( right ), position.Dot( up ), position.Dot( -forward ) ) );
  result.data[ 15 ] = 1;
  return result;
}

Matrix4 Matrix4::ViewToProjection( float fov, float aspectRatio, float nearPlane, float farPlane )
{
  // a  0  0  0
  // 0  b  0  0
  // 0  0  A  B
  // 0  0 -1  0

  // this is assuming a symmetric frustum, in this case nearPlane cancels out
  
  float halfAngleTangent = tanf( fov * 0.5f);
  // Multiply by near plane so fov is consistent regardless of near plane distance
  float r = aspectRatio * halfAngleTangent * nearPlane; // scaled by view aspect ratio
  float t = halfAngleTangent * nearPlane; // tan of half angle fit vertically

  float a = nearPlane / r;
  float b = nearPlane / t;
   
  float A;
  float B;
  if ( ReverseZ )
  {
    A = 0;
    B = nearPlane;
  }
  else
  {
    A = -( farPlane + nearPlane ) / ( farPlane - nearPlane );
    B = ( -2.0f * farPlane * nearPlane ) / ( farPlane - nearPlane );
  }
  
  Matrix4 result;
  memset( &result, 0, sizeof( result ) );
  result.data[ 0 ] = a;
  result.data[ 5 ] = b;
  result.data[ 10 ] = A;
  result.data[ 14 ] = B;
  result.data[ 11 ] = -1;
  return result;
}

Vec4 Matrix4::operator*(const Vec4& v) const
{
  return Vec4(
    v.x*data[0]  + v.y*data[4]  + v.z*data[8]  + v.w*data[12],
    v.x*data[1]  + v.y*data[5]  + v.z*data[9]  + v.w*data[13],
    v.x*data[2]  + v.y*data[6]  + v.z*data[10] + v.w*data[14],
    v.x*data[3] + v.y*data[7] + v.z*data[11] + v.w*data[15]);
}

Matrix4 Matrix4::operator*(const Matrix4& m) const
{
  Matrix4 r;
  r.data[0]=(m.data[0]*data[0])+(m.data[1]*data[4])+(m.data[2]*data[8])+(m.data[3]*data[12]);
  r.data[1]=(m.data[0]*data[1])+(m.data[1]*data[5])+(m.data[2]*data[9])+(m.data[3]*data[13]);
  r.data[2]=(m.data[0]*data[2])+(m.data[1]*data[6])+(m.data[2]*data[10])+(m.data[3]*data[14]);
  r.data[3]=(m.data[0]*data[3])+(m.data[1]*data[7])+(m.data[2]*data[11])+(m.data[3]*data[15]);
  r.data[4]=(m.data[4]*data[0])+(m.data[5]*data[4])+(m.data[6]*data[8])+(m.data[7]*data[12]);
  r.data[5]=(m.data[4]*data[1])+(m.data[5]*data[5])+(m.data[6]*data[9])+(m.data[7]*data[13]);
  r.data[6]=(m.data[4]*data[2])+(m.data[5]*data[6])+(m.data[6]*data[10])+(m.data[7]*data[14]);
  r.data[7]=(m.data[4]*data[3])+(m.data[5]*data[7])+(m.data[6]*data[11])+(m.data[7]*data[15]);
  r.data[8]=(m.data[8]*data[0])+(m.data[9]*data[4])+(m.data[10]*data[8])+(m.data[11]*data[12]);
  r.data[9]=(m.data[8]*data[1])+(m.data[9]*data[5])+(m.data[10]*data[9])+(m.data[11]*data[13]);
  r.data[10]=(m.data[8]*data[2])+(m.data[9]*data[6])+(m.data[10]*data[10])+(m.data[11]*data[14]);
  r.data[11]=(m.data[8]*data[3])+(m.data[9]*data[7])+(m.data[10]*data[11])+(m.data[11]*data[15]);
  r.data[12]=(m.data[12]*data[0])+(m.data[13]*data[4])+(m.data[14]*data[8])+(m.data[15]*data[12]);
  r.data[13]=(m.data[12]*data[1])+(m.data[13]*data[5])+(m.data[14]*data[9])+(m.data[15]*data[13]);
  r.data[14]=(m.data[12]*data[2])+(m.data[13]*data[6])+(m.data[14]*data[10])+(m.data[15]*data[14]);
  r.data[15]=(m.data[12]*data[3])+(m.data[13]*data[7])+(m.data[14]*data[11])+(m.data[15]*data[15]);
  return r;
}

void Matrix4::operator*=(const Matrix4& m)
{
  *this = (*this) * m;
}

void Matrix4::SetInverse()
{
  *this = GetInverse();
}

Matrix4 Matrix4::GetInverse() const
{
  Matrix4 r;

  r.data[0] = data[5]  * data[10] * data[15] -
    data[5]  * data[11] * data[14] -
    data[9]  * data[6]  * data[15] +
    data[9]  * data[7]  * data[14] +
    data[13] * data[6]  * data[11] -
    data[13] * data[7]  * data[10];

  r.data[4] = -data[4]  * data[10] * data[15] +
    data[4]  * data[11] * data[14] +
    data[8]  * data[6]  * data[15] -
    data[8]  * data[7]  * data[14] -
    data[12] * data[6]  * data[11] +
    data[12] * data[7]  * data[10];

  r.data[8] = data[4]  * data[9] * data[15] -
    data[4]  * data[11] * data[13] -
    data[8]  * data[5] * data[15] +
    data[8]  * data[7] * data[13] +
    data[12] * data[5] * data[11] -
    data[12] * data[7] * data[9];

  r.data[12] = -data[4]  * data[9] * data[14] +
    data[4]  * data[10] * data[13] +
    data[8]  * data[5] * data[14] -
    data[8]  * data[6] * data[13] -
    data[12] * data[5] * data[10] +
    data[12] * data[6] * data[9];

  r.data[1] = -data[1]  * data[10] * data[15] +
    data[1]  * data[11] * data[14] +
    data[9]  * data[2] * data[15] -
    data[9]  * data[3] * data[14] -
    data[13] * data[2] * data[11] +
    data[13] * data[3] * data[10];

  r.data[5] = data[0]  * data[10] * data[15] -
    data[0]  * data[11] * data[14] -
    data[8]  * data[2] * data[15] +
    data[8]  * data[3] * data[14] +
    data[12] * data[2] * data[11] -
    data[12] * data[3] * data[10];

  r.data[9] = -data[0]  * data[9] * data[15] +
    data[0]  * data[11] * data[13] +
    data[8]  * data[1] * data[15] -
    data[8]  * data[3] * data[13] -
    data[12] * data[1] * data[11] +
    data[12] * data[3] * data[9];

  r.data[13] = data[0]  * data[9] * data[14] -
    data[0]  * data[10] * data[13] -
    data[8]  * data[1] * data[14] +
    data[8]  * data[2] * data[13] +
    data[12] * data[1] * data[10] -
    data[12] * data[2] * data[9];

  r.data[2] = data[1]  * data[6] * data[15] -
    data[1]  * data[7] * data[14] -
    data[5]  * data[2] * data[15] +
    data[5]  * data[3] * data[14] +
    data[13] * data[2] * data[7] -
    data[13] * data[3] * data[6];

  r.data[6] = -data[0]  * data[6] * data[15] +
    data[0]  * data[7] * data[14] +
    data[4]  * data[2] * data[15] -
    data[4]  * data[3] * data[14] -
    data[12] * data[2] * data[7] +
    data[12] * data[3] * data[6];

  r.data[10] = data[0]  * data[5] * data[15] -
    data[0]  * data[7] * data[13] -
    data[4]  * data[1] * data[15] +
    data[4]  * data[3] * data[13] +
    data[12] * data[1] * data[7] -
    data[12] * data[3] * data[5];

  r.data[14] = -data[0]  * data[5] * data[14] +
    data[0]  * data[6] * data[13] +
    data[4]  * data[1] * data[14] -
    data[4]  * data[2] * data[13] -
    data[12] * data[1] * data[6] +
    data[12] * data[2] * data[5];

  r.data[3] = -data[1] * data[6] * data[11] +
    data[1] * data[7] * data[10] +
    data[5] * data[2] * data[11] -
    data[5] * data[3] * data[10] -
    data[9] * data[2] * data[7] +
    data[9] * data[3] * data[6];

  r.data[7] = data[0] * data[6] * data[11] -
    data[0] * data[7] * data[10] -
    data[4] * data[2] * data[11] +
    data[4] * data[3] * data[10] +
    data[8] * data[2] * data[7] -
    data[8] * data[3] * data[6];

  r.data[11] = -data[0] * data[5] * data[11] +
    data[0] * data[7] * data[9] +
    data[4] * data[1] * data[11] -
    data[4] * data[3] * data[9] -
    data[8] * data[1] * data[7] +
    data[8] * data[3] * data[5];

  r.data[15] = data[0] * data[5] * data[10] -
    data[0] * data[6] * data[9] -
    data[4] * data[1] * data[10] +
    data[4] * data[2] * data[9] +
    data[8] * data[1] * data[6] -
    data[8] * data[2] * data[5];

  float det = data[0] * r.data[0] + data[1] * r.data[4] + data[2] * r.data[8] + data[3] * r.data[12];
  det = 1.0f / det;
  for ( uint32_t i = 0; i < 16; i++ )
  {
    r.data[ i ] *= det;
  }
  return r;
}

void Matrix4::SetRotation( const Quaternion& q2 )
{
  Quaternion q = q2.GetInverse();
  data[0] = 1 - (2*q.j*q.j + 2*q.k*q.k);
  data[4] = 2*q.i*q.j + 2*q.k*q.r;
  data[8] = 2*q.i*q.k - 2*q.j*q.r;
  data[1] = 2*q.i*q.j - 2*q.k*q.r;
  data[5] = 1 - (2*q.i*q.i  + 2*q.k*q.k);
  data[9] = 2*q.j*q.k + 2*q.i*q.r;
  data[2] = 2*q.i*q.k + 2*q.j*q.r;
  data[6] = 2*q.j*q.k - 2*q.i*q.r;
  data[10] = 1 - (2*q.i*q.i  + 2*q.j*q.j);
}

Quaternion Matrix4::GetRotation() const
{
  // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

  Matrix4 t = *this;
  t.SetScale( Vec3( 1.0f ) );

  #define m00 t.data[ 0 ]
  #define m01 t.data[ 4 ]
  #define m02 t.data[ 8 ]
  #define m10 t.data[ 1 ]
  #define m11 t.data[ 5 ]
  #define m12 t.data[ 9 ]
  #define m20 t.data[ 2 ]
  #define m21 t.data[ 6 ]
  #define m22 t.data[ 10 ]

  float trace = m00 + m11 + m22;
  if ( trace > 0.0f )
  {
    float s = sqrt( trace + 1.0f ) * 2.0f;
    return Quaternion(
      ( m21 - m12 ) / s,
      ( m02 - m20 ) / s,
      ( m10 - m01 ) / s,
      0.25f * s
    );
  }
  else if ( ( m00 > m11 ) && ( m00 > m22 ) )
  {
    float s = sqrt( 1.0f + m00 - m11 - m22 ) * 2.0f;
    return Quaternion(
      0.25f * s,
      ( m01 + m10 ) / s,
      ( m02 + m20 ) / s,
      ( m21 - m12 ) / s
    );
  }
  else if ( m11 > m22 )
  {
    float s = sqrt( 1.0f + m11 - m00 - m22 ) * 2.0f;
    return Quaternion(
      ( m01 + m10 ) / s,
      0.25f * s,
      ( m12 + m21 ) / s,
      ( m02 - m20 ) / s
    );
  }
  else
  {
    float s = sqrt( 1.0f + m22 - m00 - m11 ) * 2.0f;
    return Quaternion(
      ( m02 + m20 ) / s,
      ( m12 + m21 ) / s,
      0.25f * s,
      ( m10 - m01 ) / s
    );
  }

  #undef m00
  #undef m01
  #undef m02
  #undef m10
  #undef m11
  #undef m12
  #undef m20
  #undef m21
  #undef m22
}

Vec3 Matrix4::GetAxis( uint32_t col ) const
{
    return Vec3( data[ col * 4 ], data[ col * 4 + 1 ], data[ col * 4 + 2 ] );
}

void Matrix4::SetAxis( uint32_t col, const Vec3& v )
{
  data[ col * 4 ] = v.x;
  data[ col * 4 + 1 ] = v.y;
  data[ col * 4 + 2 ] = v.z;
}

Vec4 Matrix4::GetRow( uint32_t row ) const
{
  return Vec4( data[ row ], data[ row + 4 ], data[ row + 8 ], data[ row + 12 ] );
}

void Matrix4::SetRow( uint32_t row, const Vec3 &v )
{
  data[ row ] = v.x;
  data[ row + 4 ] = v.y;
  data[ row + 8 ] = v.z;
}

void Matrix4::SetRow( uint32_t row, const Vec4 &v)
{
  data[ row ] = v.x;
  data[ row + 4 ] = v.y;
  data[ row + 8 ] = v.z;
  data[ row + 12 ] = v.w;
}

void Matrix4::SetTranslation( float x, float y, float z )
{
  data[ 12 ] = x;
  data[ 13 ] = y;
  data[ 14 ] = z;
}

void Matrix4::SetTranslation( const Vec3& translation )
{
  data[ 12 ] = translation.x;
  data[ 13 ] = translation.y;
  data[ 14 ] = translation.z;
}

Vec3 Matrix4::GetTranslation() const
{
  return Vec3( data[ 12 ], data[ 13 ], data[ 14 ] );
}

Vec3 Matrix4::GetScale() const
{
  return Vec3(
    Vec3( data[ 0 ], data[ 1 ], data[ 2 ] ).Length(),
    Vec3( data[ 4 ], data[ 5 ], data[ 6 ] ).Length(),
    Vec3( data[ 8 ], data[ 9 ], data[ 10 ] ).Length()
  );
}

void Matrix4::SetScale( const Vec3& s )
{
  for( uint32_t i = 0; i < 3; i++ )
  {
    SetAxis( i, GetAxis( i ).NormalizeCopy() * s[ i ] );
  }
}

void Matrix4::SetTranspose( void )
{
  for( uint32_t i = 0; i < 4; i++ )
  {
    for( uint32_t j = i + 1; j < 4; j++ )
    {
      std::swap( data[ i * 4 + j ], data[ j * 4 + i ] );
    }
  }
}

Matrix4 Matrix4::GetTranspose() const
{
  Matrix4 r = *this;
  r.SetTranspose();
  return r;
}

Matrix4 Matrix4::GetNormalMatrix() const
{
  return GetInverse().GetTranspose();
}

Matrix4 Matrix4::GetScaleRemoved() const
{
  Matrix4 r = *this;
  r.SetAxis( 0, r.GetAxis( 0 ).NormalizeCopy() );
  r.SetAxis( 1, r.GetAxis( 1 ).NormalizeCopy() );
  r.SetAxis( 2, r.GetAxis( 2 ).NormalizeCopy() );
  return r;
}

//------------------------------------------------------------------------------
// ae::Quaternion member functions
//------------------------------------------------------------------------------
Quaternion::Quaternion( Vec3 forward, Vec3 up, bool prioritizeUp )
{
  forward.Normalize();
  up.Normalize();

  Vec3 right = forward.Cross( up );
  right.Normalize();
  if ( prioritizeUp )
  {
    up = right.Cross( forward );
  }
  else
  {
    forward = up.Cross( right );
  }

#define m0 right
#define m1 forward
#define m2 up
#define m00 m0.x
#define m01 m1.x
#define m02 m2.x
#define m10 m0.y
#define m11 m1.y
#define m12 m2.y
#define m20 m0.z
#define m21 m1.z
#define m22 m2.z

  //http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
  float trace = m00 + m11 + m22;
  if ( trace > 0.0f )
  {
    float S = sqrtf( trace + 1.0f ) * 2;
    r = 0.25f * S;
    i = ( m21 - m12 ) / S;
    j = ( m02 - m20 ) / S;
    k = ( m10 - m01 ) / S;
  }
  else if ( ( m00 > m11 ) & ( m00 > m22 ) )
  {
    float S = sqrtf( 1.0f + m00 - m11 - m22 ) * 2;
    r = ( m21 - m12 ) / S;
    i = 0.25f * S;
    j = ( m01 + m10 ) / S;
    k = ( m02 + m20 ) / S;
  }
  else if ( m11 > m22 )
  {
    float S = sqrtf( 1.0f + m11 - m00 - m22 ) * 2;
    r = ( m02 - m20 ) / S;
    i = ( m01 + m10 ) / S;
    j = 0.25f * S;
    k = ( m12 + m21 ) / S;
  }
  else
  {
    float S = sqrtf( 1.0f + m22 - m00 - m11 ) * 2;
    r = ( m10 - m01 ) / S;
    i = ( m02 + m20 ) / S;
    j = ( m12 + m21 ) / S;
    k = 0.25f * S;
  }

#undef m0
#undef m1
#undef m2
#undef m00
#undef m01
#undef m02
#undef m10
#undef m11
#undef m12
#undef m20
#undef m21
#undef m22
}

Quaternion::Quaternion( Vec3 axis, float angle )
{
  axis.Normalize();
  float sinAngleDiv2 = sinf( angle / 2.0f );
  i = axis.x * sinAngleDiv2;
  j = axis.y * sinAngleDiv2;
  k = axis.z * sinAngleDiv2;
  r = cosf( angle / 2.0f );
}

void Quaternion::Normalize()
{
  float invMagnitude = r * r + i * i + j * j + k * k;

  if ( invMagnitude == 0.0f )
  {
    r = 1;
    return;
  }

  invMagnitude = 1.0f / std::sqrt( invMagnitude );
  r *= invMagnitude;
  i *= invMagnitude;
  j *= invMagnitude;
  k *= invMagnitude;
}

bool Quaternion::operator==( const Quaternion& q ) const
{
  return ( i == q.i ) && ( j == q.j ) && ( k == q.k ) && ( r == q.r );
}

bool Quaternion::operator!=( const Quaternion& q ) const
{
  return !operator==( q );
}

Quaternion& Quaternion::operator*= ( const Quaternion& q )
{
  //http://www.mathworks.com/help/aeroblks/quaternionmultiplication.html
  Quaternion copy = *this;
  r = copy.r * q.r - copy.i * q.i - copy.j * q.j - copy.k * q.k;
  i = copy.r * q.i + copy.i * q.r + copy.j * q.k - copy.k * q.j;
  j = copy.r * q.j + copy.j * q.r + copy.k * q.i - copy.i * q.k;
  k = copy.r * q.k + copy.k * q.r + copy.i * q.j - copy.j * q.i;
  return *this;
}

Quaternion Quaternion::operator* ( const Quaternion& q ) const
{
  return Quaternion( *this ) *= q;
}

Quaternion const Quaternion::operator*( float s ) const
{
  return Quaternion( s * i, s * j, s * k, s * r );
}

void Quaternion::AddScaledVector( const Vec3& v, float t )
{
  Quaternion q( v.x * t, v.y * t, v.z * t, 0.0f );
  q *= *this;

  r += q.r * 0.5f;
  i += q.i * 0.5f;
  j += q.j * 0.5f;
  k += q.k * 0.5f;
}

void Quaternion::RotateByVector( const Vec3& v )
{
  Quaternion q = Quaternion::Identity();

  float s = v.Length();
  //ASSERT_MSG(s > 0.001f, "Can't rotate by a zero vector!");
  q.r = cosf( s * 0.5f );

  Vec3 n = v.NormalizeCopy() * sinf( s * 0.5f );
  q.i = n.x;
  q.j = n.y;
  q.k = n.z;

  ( *this ) *= q;

}

void Quaternion::SetDirectionXY( const Vec3& v )
{
  float theta = std::atan( v.y / v.x );
  if ( v.x < 0 && v.y >= 0 )
    theta += ae::PI;
  else if ( v.x < 0 && v.y < 0 )
    theta -= ae::PI;

  r = std::cos( theta / 2.0f );
  i = 0.0f;
  j = 0.0f;
  k = std::sin( theta / 2.0f );
}

Vec3 Quaternion::GetDirectionXY() const
{
  float theta;
  if ( k >= 0.0f )
    theta = 2.0f * std::acos( r );
  else
    theta = -2.0f * std::acos( r );

  return Vec3( std::cos( theta ), std::sin( theta ), 0.0f );
}

void Quaternion::ZeroXY()
{
  i = 0.0f;
  j = 0.0f;
}

void Quaternion::GetAxisAngle( Vec3* axis, float* angle ) const
{
  *angle = 2 * acos( r );
  axis->x = i / sqrt( 1 - r * r );
  axis->y = j / sqrt( 1 - r * r );
  axis->z = k / sqrt( 1 - r * r );
}

void Quaternion::AddRotationXY( float rotation )
{
  float sinThetaOver2 = std::sin( rotation / 2.0f );
  float cosThetaOver2 = std::cos( rotation / 2.0f );

  // create a quaternion representing the amount to rotate
  Quaternion change( 0.0f, 0.0f, sinThetaOver2, cosThetaOver2 );
  change.Normalize();

  // apply the change in rotation
  ( *this ) *= change;
}

Quaternion Quaternion::Nlerp( Quaternion d, float t ) const
{
  float epsilon = this->Dot( d );
  Quaternion end = d;

  if ( epsilon < 0.0f )
  {
    epsilon = -epsilon;

    end = Quaternion( -d.i, -d.j, -d.k, -d.r );
  }

  Quaternion result = ( *this ) * ( 1.0f - t );
  end = end * t;

  result.i += end.i;
  result.j += end.j;
  result.k += end.k;
  result.r += end.r;
  result.Normalize();

  return result;
}

Matrix4 Quaternion::GetTransformMatrix( void ) const
{
  Quaternion n = *this;
  n.Normalize();

  Matrix4 matrix = Matrix4::Identity();

  matrix.data[ 0 ] = 1.0f - 2.0f * n.j * n.j - 2.0f * n.k * n.k;
  matrix.data[ 4 ] = 2.0f * n.i * n.j - 2.0f * n.r * n.k;
  matrix.data[ 8 ] = 2.0f * n.i * n.k + 2.0f * n.r * n.j;

  matrix.data[ 1 ] = 2.0f * n.i * n.j + 2.0f * n.r * n.k;
  matrix.data[ 5 ] = 1.0f - 2.0f * n.i * n.i - 2.0f * n.k * n.k;
  matrix.data[ 9 ] = 2.0f * n.j * n.k - 2.0f * n.r * n.i;

  matrix.data[ 2 ] = 2.0f * n.i * n.k - 2.0f * n.r * n.j;
  matrix.data[ 6 ] = 2.0f * n.j * n.k + 2.0f * n.r * n.i;
  matrix.data[ 10 ] = 1.0f - 2.0f * n.i * n.i - 2.0f * n.j * n.j;

  return matrix;
}

Quaternion Quaternion::GetInverse( void ) const
{
  return Quaternion( *this ).SetInverse();
}

Quaternion& Quaternion::SetInverse( void )
{
  //http://www.mathworks.com/help/aeroblks/quaternioninverse.html
  float d = r * r + i * i + j * j + k * k;
  r /= d;
  i /= -d;
  j /= -d;
  k /= -d;

  return *this;
}

Vec3 Quaternion::Rotate( Vec3 v ) const
{
  //http://www.mathworks.com/help/aeroblks/quaternionrotation.html
  Quaternion q = ( *this ) * Quaternion( v ) * this->GetInverse();
  return Vec3( q.i, q.j, q.k );
}

float Quaternion::Dot( const Quaternion& q ) const
{
  return ( q.r * r ) + ( q.i * i ) + ( q.j * j ) + ( q.k * k );
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
#elif _AE_OSX_
    // @HACK: macosx clang c++11 does not have aligned alloc
    return malloc( bytes );
#else
    return aligned_alloc( alignment, bytes );
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
// ae::Dict members
//------------------------------------------------------------------------------
Dict::Dict( ae::Tag tag ) :
  m_entries( tag )
{}

void Dict::SetString( const char* key, const char* value )
{
  m_entries.Set( key, value );
}

void Dict::SetInt( const char* key, int32_t value )
{
  char buf[ 128 ];
  sprintf( buf, "%d", value );
  SetString( key, buf );
}

void Dict::SetFloat( const char* key, float value )
{
  char buf[ 128 ];
  sprintf( buf, "%f", value );
  SetString( key, buf );
}

void Dict::SetBool( const char* key, bool value )
{
  SetString( key, value ? "true" : "false" );
}

void Dict::SetVec2( const char* key, ae::Vec2 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f", value.x, value.y );
  SetString( key, buf );
}

void Dict::SetVec3( const char* key, ae::Vec3 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f %.2f", value.x, value.y, value.z );
  SetString( key, buf );
}

void Dict::SetVec4( const char* key, ae::Vec4 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f %.2f %.2f", value.x, value.y, value.z, value.w );
  SetString( key, buf );
}

void Dict::SetInt2( const char* key, ae::Int2 value )
{
  char buf[ 128 ];
  sprintf( buf, "%d %d", value.x, value.y );
  SetString( key, buf );
}

void Dict::Clear()
{
  m_entries.Clear();
}

const char* Dict::GetString( const char* key, const char* defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    return value->c_str();
  }
  return defaultValue;
}

int32_t Dict::GetInt( const char* key, int32_t defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    return atoi( value->c_str() );
  }
  return defaultValue;
}

float Dict::GetFloat( const char* key, float defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    return (float)atof( value->c_str() );
  }
  return defaultValue;
}

bool Dict::GetBool( const char* key, bool defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    if ( *value == "true" )
    {
      return true;
    }
    else if ( *value == "false" )
    {
      return false;
    }
  }
  return defaultValue;
}

ae::Vec2 Dict::GetVec2( const char* key, ae::Vec2 defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    ae::Vec2 result( 0.0f );
    sscanf( value->c_str(), "%f %f", &result.x, &result.y );
    return result;
  }
  return defaultValue;
}

ae::Vec3 Dict::GetVec3( const char* key, ae::Vec3 defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    ae::Vec3 result( 0.0f );
    sscanf( value->c_str(), "%f %f %f", &result.x, &result.y, &result.z );
    return result;
  }
  return defaultValue;
}

ae::Vec4 Dict::GetVec4( const char* key, ae::Vec4 defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    ae::Vec4 result( 0.0f );
    sscanf( value->c_str(), "%f %f %f %f", &result.x, &result.y, &result.z, &result.w );
    return result;
  }
  return defaultValue;
}

ae::Int2 Dict::GetInt2( const char* key, ae::Int2 defaultValue ) const
{
  if ( const ae::Str128* value = m_entries.TryGet( key ) )
  {
    ae::Int2 result( 0.0f );
    sscanf( value->c_str(), "%d %d", &result.x, &result.y );
    return result;
  }
  return defaultValue;
}

ae::Color Dict::GetColor( const char* key, ae::Color defaultValue ) const
{
  // uint8_t c[ 4 ];
  // const KeyValue* kv = m_GetValue( key );
  // if ( kv && kv->value.Length() == 9 && sscanf( kv->value.c_str(), "#%2hhx%2hhx%2hhx%2hhx", &c[ 0 ], &c[ 1 ], &c[ 2 ], &c[ 3 ] ) == 4 )
  // {
  //   return (Color)((c[ 0 ] << 24) | (c[ 1 ] << 16) | (c[ 2 ] << 8) | c[ 3 ]);
  // }
  return defaultValue;
}

bool Dict::Has( const char* key ) const
{
  return m_entries.TryGet( key ) != nullptr;
}

const char* Dict::GetKey( uint32_t idx ) const
{
  return m_entries.GetKey( idx ).c_str();
}

const char* Dict::GetValue( uint32_t idx ) const
{
  return m_entries.GetValue( idx ).c_str();
}

std::ostream& operator<<( std::ostream& os, const Dict& dict )
{
  os << "[";
  for ( uint32_t i = 0; i < dict.Length(); i++ )
  {
    if ( i )
    {
      os << ",";
    }
    os << "<'" << dict.GetKey( i ) << "','" << dict.GetValue( i ) << "'>";
  }
  return os << "]";
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
// ae::RectInt member functions
//------------------------------------------------------------------------------
bool RectInt::Contains( ae::Int2 pos ) const
{
  return !( pos.x < x || pos.x >= ( x + w ) || pos.y < y || pos.y >= ( y + h ) );
}

bool RectInt::Intersects( RectInt o ) const
{
  return !( o.x + o.w <= x || x + w <= o.x // No horizontal intersection
    || o.y + o.h <= y || y + h <= o.y ); // No vertical intersection
}

void RectInt::Expand( ae::Int2 pos )
{
  if ( w == 0 )
  {
    x = pos.x;
    w = 1;
  }
  else
  {
    // @NOTE: One past input value to expand width by one column
    int x1 = ae::Max( x + w, pos.x + 1 );
    x = ae::Min( x, pos.x );
    w = x1 - x;
  }

  if ( h == 0 )
  {
    y = pos.y;
    h = 1;
  }
  else
  {
    // @NOTE: One past input value to expand width by one row
    int y1 = ae::Max( y + h, pos.y + 1 );
    y = ae::Min( y, pos.y );
    h = y1 - y;
  }
}

//------------------------------------------------------------------------------
// ae::Hash member functions
//------------------------------------------------------------------------------
Hash::Hash( uint32_t initialValue )
{
  m_hash = initialValue;
}

Hash& Hash::HashString( const char* str )
{
  while ( *str )
  {
    m_hash = m_hash ^ str[ 0 ];
    m_hash *= 0x1000193;
    str++;
  }

  return *this;
}

Hash& Hash::HashData( const void* _data, uint32_t length )
{
  const uint8_t* data = (const uint8_t*)_data;
  for ( uint32_t i = 0; i < length; i++ )
  {
    m_hash = m_hash ^ data[ i ];
    m_hash *= 0x1000193;
  }

  return *this;
}

Hash& Hash::HashFloat( float f )
{
  uint32_t ui;
  memcpy( &ui, &f, sizeof( float ) );
  ui &= 0xfffff000;
  return HashBasicType( ui );
}

void Hash::Set( uint32_t hash )
{
  m_hash = hash;
}

uint32_t Hash::Get() const
{
  return m_hash;
}

//------------------------------------------------------------------------------
// ae::Window MSVC/Windows event callback
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
        window->m_UpdateSize( width, height, 1.0f ); // @TODO: Scale factor
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

//------------------------------------------------------------------------------
// ae::Window Objective-C aeApplicationDelegate class
//------------------------------------------------------------------------------
#if _AE_OSX_
} // ae end
@interface aeApplicationDelegate : NSObject< NSApplicationDelegate >
@property ae::Window* aewindow;
@end
@implementation aeApplicationDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
  [NSApp stop:nil]; // Prevents app run from blocking
}
@end

//------------------------------------------------------------------------------
// ae::Window Objective-C aeWindowDelegate class
//------------------------------------------------------------------------------
@interface aeWindowDelegate : NSObject< NSWindowDelegate >
@property ae::Window* aewindow;
@end
@implementation aeWindowDelegate
- (BOOL)windowShouldClose:(NSWindow *)sender
{
  return true; // @TODO: Allow user to prevent window from closing
}
- (void)windowWillClose:(NSNotification *)notification
{
    AE_ASSERT( _aewindow );
    AE_ASSERT( _aewindow->input );
    _aewindow->input->quit = true;
}
- (void)windowDidResize:(NSWindow*)sender
{
  AE_ASSERT( _aewindow );
  NSWindow* window = (NSWindow*)_aewindow->window;
  AE_ASSERT( window );
  
  NSRect contentScreenRect = [window convertRectToScreen:[window contentLayoutRect]];
  _aewindow->m_UpdatePos( ae::Int2( contentScreenRect.origin.x, contentScreenRect.origin.y ) );
  _aewindow->m_UpdateSize( contentScreenRect.size.width, contentScreenRect.size.height, [window backingScaleFactor] );
  
  NSPoint mouseScreenPos = [NSEvent mouseLocation];
  _aewindow->input->m_SetMousePos( ae::Int2( mouseScreenPos.x, mouseScreenPos.y ) );
}
- (void)windowDidMove:(NSNotification *)notification
{
  AE_ASSERT( _aewindow );
  NSWindow* window = (NSWindow*)_aewindow->window;
  AE_ASSERT( window );
  
  NSRect contentScreenRect = [window convertRectToScreen:[window contentLayoutRect]];
  _aewindow->m_UpdatePos( ae::Int2( contentScreenRect.origin.x, contentScreenRect.origin.y ) );
  
  NSPoint mouseScreenPos = [NSEvent mouseLocation];
  _aewindow->input->m_SetMousePos( ae::Int2( mouseScreenPos.x, mouseScreenPos.y ) );
}
- (void)windowDidBecomeKey:(NSNotification *)notification
{
  AE_ASSERT( _aewindow );
  _aewindow->m_UpdateFocused( true );
}
- (void)windowDidResignKey:(NSNotification *)notification
{
  AE_ASSERT( _aewindow );
  _aewindow->m_UpdateFocused( false );
}
@end
namespace ae {
#endif

//------------------------------------------------------------------------------
// ae::Window member functions
//------------------------------------------------------------------------------
Window::Window()
{
  window = nullptr;
  graphicsDevice = nullptr;
  input = nullptr;
  m_pos = Int2( 0 );
  m_width = 0;
  m_height = 0;
  m_fullScreen = false;
  m_maximized = false;
  m_focused = false;
  m_scaleFactor = 0.0f;
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

void Window::m_UpdateSize( int32_t width, int32_t height, float scaleFactor )
{
  m_width = width;
  m_height = height;
  m_scaleFactor = scaleFactor;
}

void Window::m_Initialize()
{
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
  
  // @TODO: Get real scale factor
  m_scaleFactor = 1.0f;
#elif _AE_OSX_
  // Autorelease Pool
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  
  // Application
  [NSApplication sharedApplication];
  aeApplicationDelegate* applicationDelegate = [[aeApplicationDelegate alloc] init];
  applicationDelegate.aewindow = this;
  [NSApp setDelegate:applicationDelegate];

  // Main window
  aeWindowDelegate* windowDelegate = [[aeWindowDelegate alloc] init];
  windowDelegate.aewindow = this;
  NSWindow* nsWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, m_width, m_height )
    styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
    backing:NSBackingStoreBuffered
    defer:YES
  ];
  nsWindow.delegate = windowDelegate;
  this->window = nsWindow;
  
  NSOpenGLPixelFormatAttribute nsPixelAttribs[] =
  {
    NSOpenGLPFAAccelerated,
    NSOpenGLPFAClosestPolicy,
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core, // @TODO: Use ae GL version
    //NSOpenGLPFADoubleBuffer,
    //NSOpenGLPFASampleBuffers, 1,
    //NSOpenGLPFASamples, samples,
    0
  };
  NSRect frame = [nsWindow contentRectForFrameRect:[nsWindow frame]];
  NSOpenGLPixelFormat* nsPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:nsPixelAttribs];
  AE_ASSERT_MSG( nsPixelFormat, "Could not determine a valid pixel format" );
  
  NSOpenGLView* glView = [[NSOpenGLView alloc] initWithFrame:frame pixelFormat:nsPixelFormat];
  AE_ASSERT_MSG( glView, "Could not create view with specified pixel format" );
  [glView setWantsBestResolutionOpenGLSurface:true]; // @TODO: Retina. Does this do anything?
  [glView.openGLContext makeCurrentContext];
  
  [nsPixelFormat release];
  [nsWindow setContentView:glView];
  [nsWindow makeFirstResponder:glView];
  [nsWindow setOpaque:YES];
  [nsWindow setContentMinSize:NSMakeSize(150.0, 100.0)];
  // @TODO: Create menus (especially Quit!)
  
  NSRect contentScreenRect = [nsWindow convertRectToScreen:[nsWindow contentLayoutRect]];
  m_pos = ae::Int2( contentScreenRect.origin.x, contentScreenRect.origin.y );
  m_width = contentScreenRect.size.width;
  m_height = contentScreenRect.size.height;
  m_scaleFactor = nsWindow.backingScaleFactor;
  
  if (![[NSRunningApplication currentApplication] isFinishedLaunching]) // Make sure run is only called once
  {
    [NSApp run];
  }
#elif _AE_EMSCRIPTEN_
  m_width = 0;
  m_height = 0;
  // double dpr = emscripten_get_device_pixel_ratio();
  // emscripten_set_element_css_size("canvas", GetWidth() / dpr, GetHeight() / dpr);
  emscripten_set_canvas_element_size( "canvas", GetWidth(), GetHeight() );
  EM_ASM({
    var canvas = document.getElementsByTagName('canvas')[0];
    canvas.style.position = "absolute";
    canvas.style.top = "0px";
    canvas.style.left = "0px";
    canvas.style.width = "100%";
    canvas.style.height = "100%";
  });
  m_scaleFactor = 1.0f;
#endif
}

void Window::Terminate()
{
  //SDL_DestroyWindow( (SDL_Window*)window );
}

#if _AE_EMSCRIPTEN_
int32_t Window::GetWidth() const
{
  return EM_ASM_INT({ return window.innerWidth; });
}

int32_t Window::GetHeight() const
{
  return EM_ASM_INT({ return window.innerHeight; });
}
#else
int32_t Window::GetWidth() const
{
  return m_width;
}

int32_t Window::GetHeight() const
{
  return m_height;
}
#endif

void Window::SetTitle( const char* title )
{
  if ( window && m_windowTitle != title )
  {
#if _AE_WINDOWS_
    SetWindowTextA( (HWND)window, title );
#endif
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
// ae::Input Objective-C aeTextInputDelegate and aeKeyInput classes
//------------------------------------------------------------------------------
#if _AE_OSX_
} // ae end
@interface aeTextInputDelegate : NSView< NSTextInputClient >
@property ae::Input* aeinput;
@end

@interface aeKeyInput : NSObject< NSStandardKeyBindingResponding >
@property (retain) aeTextInputDelegate* input;
@end

//------------------------------------------------------------------------------
// ae::Input Objective-C aeKeyInput member functions
//------------------------------------------------------------------------------
@implementation aeKeyInput
- (void)deleteBackward:(id)sender
{
  if ( !_input.aeinput->m_text.empty() )
  {
    _input.aeinput->m_text.pop_back();
  }
}
- (void)insertNewline:(id)sender
{
  _input.aeinput->m_text.append( 1, '\n' );
}
@end

//------------------------------------------------------------------------------
// ae::Input Objective-C aeTextInputDelegate member functions
//------------------------------------------------------------------------------
@implementation aeTextInputDelegate {
  aeKeyInput* _keyInput;
}
- (instancetype)initWithFrame:(NSRect)frameRect
{
  self = [super initWithFrame:frameRect];
  if ( self != nil )
  {
    _keyInput = [[aeKeyInput alloc] init];
    _keyInput.input = self;
  }
  return self;
}
// Handling Marked Text
- (BOOL)hasMarkedText
{
  // Returns a Boolean value indicating whether the receiver has marked text.
  return false;
}
- (NSRange)markedRange
{
  // Returns the range of the marked text.
  return NSMakeRange(0, 0);
}
- (NSRange)selectedRange
{
  // Returns the range of selected text.
  return NSMakeRange(0, 0);
}
- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
  // Replaces a specified range in the receiver’s text storage with the given string and sets the selection.
}
- (void)unmarkText
{
  // Unmarks the marked text.
}
- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText
{
  // Returns an array of attribute names recognized by the receiver.
  return [NSArray array];
}
// Storing Text
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
  // Returns an attributed string derived from the given range in the receiver's text storage.
  return nil;
}
- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
  AE_ASSERT( _aeinput );
  // Inserts the given string into the receiver, replacing the specified content.
  const char* str = [string isKindOfClass: [NSAttributedString class]] ? [[string string] UTF8String] : [string UTF8String];
  _aeinput->m_text += str;
  _aeinput->m_textInput += str;
}
// Getting Character Coordinates
- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
  // Returns the index of the character whose bounding rectangle includes the given point.
  return 0;
}
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
  // Returns the first logical boundary rectangle for characters in the given range.
  return NSMakeRect(0, 0, 0, 0);
}
  // Binding Keystrokes
- (void)doCommandBySelector:(SEL)selector
{
  // Invokes the action specified by the given selector.
  if ([_keyInput respondsToSelector:selector])
  {
    [_keyInput performSelector:selector];
  }
}
@end
namespace ae {
#endif

//------------------------------------------------------------------------------
// ae::Input member functions
//------------------------------------------------------------------------------
#if _AE_EMSCRIPTEN_
EM_BOOL ae_em_handle_key( int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData )
{
  if ( !keyEvent->repeat )
  {
    AE_ASSERT( userData );
    Input* input = (Input*)userData;
    // Use 'code' instead of 'key' so value is not affected by modifiers/layout
    // const char* type = EMSCRIPTEN_EVENT_KEYUP == eventType ? "up" : "down";
    // AE_LOG( "# #", keyEvent->code, type );
    bool pressed = EMSCRIPTEN_EVENT_KEYUP != eventType;
    if ( strcmp( keyEvent->code, "ArrowRight" ) == 0 ) { input->m_keys[ (int)Key::Right ] = pressed; }
    if ( strcmp( keyEvent->code, "ArrowLeft" ) == 0 ) { input->m_keys[ (int)Key::Left ] = pressed; }
    if ( strcmp( keyEvent->code, "ArrowUp" ) == 0 ) { input->m_keys[ (int)Key::Up ] = pressed; }
    if ( strcmp( keyEvent->code, "ArrowDown" ) == 0 ) { input->m_keys[ (int)Key::Down ] = pressed; }
  }
  return true;
}
#endif

void Input::Initialize( Window* window )
{
  m_window = window;
  if ( window )
  {
    window->input = this;
  }
  memset( m_keys, 0, sizeof(m_keys) );
  memset( m_keysPrev, 0, sizeof(m_keysPrev) );

#if _AE_EMSCRIPTEN_
  emscripten_set_keydown_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &ae_em_handle_key );
  emscripten_set_keyup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &ae_em_handle_key );
#endif
  
#if _AE_OSX_
  aeTextInputDelegate* textInput = [[aeTextInputDelegate alloc] initWithFrame: NSMakeRect(0.0, 0.0, 0.0, 0.0)];
  textInput.aeinput = this;
  m_textInputHandler = textInput;
  NSWindow* nsWindow = (NSWindow*)m_window->window;
  NSView* nsWindowContent = [nsWindow contentView];
  [nsWindowContent addSubview:textInput];
  
  // Do this here so input is ready to handle events
  [nsWindow makeKeyAndOrderFront:nil]; // nil sender
  [nsWindow orderFrontRegardless];
#endif
}

void Input::Terminate()
{}

void Input::Pump()
{
  memcpy( m_keysPrev, m_keys, sizeof( m_keys ) );
  mousePrev = mouse;
  mouse.movement = ae::Int2( 0 );
  mouse.scroll = ae::Vec2( 0.0f );
  m_textInput = ""; // Clear last frames text input

  // Handle system events
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
#elif _AE_OSX_
  @autoreleasepool
  {
    while ( 1 )
    {
      NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
        untilDate:[NSDate distantPast]
        inMode:NSDefaultRunLoopMode
        dequeue:YES];
      if (event == nil)
      {
        break;
      }
      
      switch ( event.type )
      {
          // Mouse
        case NSEventTypeMouseMoved: // @NOTE: Move events are not sent if any mouse button is clicked
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        {
          NSPoint p = [NSEvent mouseLocation];
          m_SetMousePos( ae::Int2( p.x, p.y ) );
          mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
          break;
        }
        case NSEventTypeLeftMouseDown:
        {
          NSPoint p = [NSEvent mouseLocation];
          m_SetMousePos( ae::Int2( p.x, p.y ) );
          // @TODO: Can these values be calculated somehow?
          if ( mouse.position.x > 2
            && mouse.position.y > 2
            && mouse.position.x < m_window->GetWidth() - 3
            && mouse.position.y < m_window->GetHeight() ) // No border because of title bar
          {
            mouse.leftButton = true;
            mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
          }
          break;
        }
        case NSEventTypeLeftMouseUp:
          mouse.leftButton = false;
          mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
          break;
        case NSEventTypeRightMouseDown:
          mouse.rightButton = true;
          mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
          break;
        case NSEventTypeRightMouseUp:
          mouse.rightButton = false;
          mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
          break;
        case NSEventTypeOtherMouseDown:
          mouse.middleButton = true;
          mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
          break;
        case NSEventTypeOtherMouseUp:
          mouse.middleButton = false;
          mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
          break;
        case NSEventTypeScrollWheel:
          mouse.scroll.x += event.deltaX;
          mouse.scroll.y += event.deltaY;
          // Scroll is never NSEventSubtypeTouch
          break;
        case NSEventTypeKeyDown:
          if ( m_textMode )
          {
            [(aeTextInputDelegate*)m_textInputHandler interpretKeyEvents:[NSArray arrayWithObject:event]];
          }
          continue; // Don't propagate keyboard events or OSX will make the clicking error sound
        case NSEventTypeKeyUp:
          continue; // Don't propagate keyboard events or OSX will make the clicking error sound
        default:
          break;
      }
      [NSApp sendEvent:event];
    }
    
    if ( m_captureMouse && m_window )
    {
      NSWindow* nsWindow = (NSWindow*)m_window->window;
      ae::Int2 posWindow( m_window->GetWidth() / 2, m_window->GetHeight() / 2 );
      NSPoint posScreen = [nsWindow convertPointToScreen:NSMakePoint( posWindow.x, posWindow.y )];
      // @NOTE: Quartz coordinate space has (0,0) at the top left, Cocoa uses bottom left
      posScreen.y = NSMaxY( NSScreen.screens[ 0 ].frame ) - posScreen.y;
      CGWarpMouseCursorPosition( CGPointMake( posScreen.x, posScreen.y ) );
      CGAssociateMouseAndMouseCursorPosition( true );
      
      if ( m_positionSet )
      {
        mouse.movement = mouse.position - posWindow;
      }
      mouse.position = posWindow;
      m_positionSet = true;
    }
  }
#endif

#if _AE_WINDOWS_
#define AE_UPDATE_KEY( _aek, _vk ) m_keys[ (int)ae::Key::_aek ] = keyStates[ _vk ] & ( 1 << 7 )
  uint8_t keyStates[ 256 ];
  if ( GetKeyboardState( keyStates ) )
  {
    // @TODO: ae::Key::NumPadEnter is currently not handled
    AE_UPDATE_KEY( Backspace, VK_BACK );
    AE_UPDATE_KEY( Tab, VK_TAB );
    // AE_UPDATE_KEY( ?, VK_CLEAR );
    AE_UPDATE_KEY( Enter, VK_RETURN );
    // AE_UPDATE_KEY( ?, VK_SHIFT );
    // AE_UPDATE_KEY( ?, VK_CONTROL );
    // AE_UPDATE_KEY( ?, VK_MENU );
    AE_UPDATE_KEY( Pause, VK_PAUSE );
    AE_UPDATE_KEY( CapsLock, VK_CAPITAL );
    // AE_UPDATE_KEY( ?, VK_KANA );
    // AE_UPDATE_KEY( ?, VK_IME_ON );
    // AE_UPDATE_KEY( ?, VK_JUNJA );
    // AE_UPDATE_KEY( ?, VK_FINAL );
    // AE_UPDATE_KEY( ?, VK_KANJI );
    // AE_UPDATE_KEY( ?, VK_IME_OFF );
    AE_UPDATE_KEY( Escape, VK_ESCAPE );
    // AE_UPDATE_KEY( ?, VK_CONVERT );
    // AE_UPDATE_KEY( ?, VK_NONCONVERT );
    // AE_UPDATE_KEY( ?, VK_ACCEPT );
    // AE_UPDATE_KEY( ?, VK_MODECHANGE );
    AE_UPDATE_KEY( Space, VK_SPACE );
    AE_UPDATE_KEY( PageUp, VK_PRIOR );
    AE_UPDATE_KEY( PageDown, VK_NEXT );
    AE_UPDATE_KEY( End, VK_END );
    AE_UPDATE_KEY( Home, VK_HOME );
    AE_UPDATE_KEY( Left, VK_LEFT );
    AE_UPDATE_KEY( Up, VK_UP );
    AE_UPDATE_KEY( Right, VK_RIGHT );
    AE_UPDATE_KEY( Down, VK_DOWN );
    // AE_UPDATE_KEY( ?, VK_SELECT );
    //AE_UPDATE_KEY( ?, VK_PRINT );
    // AE_UPDATE_KEY( ?, VK_EXECUTE );
    AE_UPDATE_KEY( PrintScreen, VK_SNAPSHOT );
    AE_UPDATE_KEY( Insert, VK_INSERT );
    AE_UPDATE_KEY( Delete, VK_DELETE );
    // AE_UPDATE_KEY( ?, VK_HELP );
    for ( uint32_t i = 0; i <= ('9' - '1'); i++ )
    {
      AE_UPDATE_KEY( Num1 + i, '1' + i );
    }
    AE_UPDATE_KEY( Num0, '0' );
    for ( uint32_t i = 0; i <= ('Z' - 'A'); i++ )
    {
      AE_UPDATE_KEY( A + i, 'A' + i );
    }
    AE_UPDATE_KEY( LeftSuper, VK_LWIN );
    AE_UPDATE_KEY( RightSuper, VK_RWIN );
    // AE_UPDATE_KEY( ?, VK_APPS );
    // AE_UPDATE_KEY( ?, VK_SLEEP );
    for ( uint32_t i = 0; i <= (VK_NUMPAD9 - VK_NUMPAD1); i++ )
    {
      AE_UPDATE_KEY( NumPad1 + i, VK_NUMPAD1 + i );
    }
    AE_UPDATE_KEY( NumPad0, VK_NUMPAD0 );
    AE_UPDATE_KEY( NumPadMultiply, VK_MULTIPLY );
    AE_UPDATE_KEY( NumPadPlus, VK_ADD );
    // AE_UPDATE_KEY( ?, VK_SEPARATOR );
    AE_UPDATE_KEY( NumPadMinus, VK_SUBTRACT );
    AE_UPDATE_KEY( NumPadPeriod, VK_DECIMAL );
    AE_UPDATE_KEY( NumPadDivide, VK_DIVIDE );
    for ( uint32_t i = 0; i <= (VK_F12 - VK_F1); i++ )
    {
      AE_UPDATE_KEY( F1 + i, VK_F1 + i );
    }
    // AE_UPDATE_KEY( ?, VK_F13 );
    // AE_UPDATE_KEY( ?, VK_F14 );
    // AE_UPDATE_KEY( ?, VK_F15 );
    // AE_UPDATE_KEY( ?, VK_F16 );
    // AE_UPDATE_KEY( ?, VK_F17 );
    // AE_UPDATE_KEY( ?, VK_F18 );
    // AE_UPDATE_KEY( ?, VK_F19 );
    // AE_UPDATE_KEY( ?, VK_F20 );
    // AE_UPDATE_KEY( ?, VK_F21 );
    // AE_UPDATE_KEY( ?, VK_F22 );
    // AE_UPDATE_KEY( ?, VK_F23 );
    // AE_UPDATE_KEY( ?, VK_F24 );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_VIEW );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_MENU );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_UP );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_DOWN );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_LEFT );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_RIGHT );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_ACCEPT );
    // AE_UPDATE_KEY( ?, VK_NAVIGATION_CANCEL );
    AE_UPDATE_KEY( NumLock, VK_NUMLOCK );
    AE_UPDATE_KEY( ScrollLock, VK_SCROLL );
    AE_UPDATE_KEY( NumPadEquals, VK_OEM_NEC_EQUAL ); // '=' key on numpad
    // AE_UPDATE_KEY( ?, VK_OEM_FJ_JISHO ); // 'Dictionary' key
    // AE_UPDATE_KEY( ?, VK_OEM_FJ_MASSHOU ); // 'Unregister word' key
    // AE_UPDATE_KEY( ?, VK_OEM_FJ_TOUROKU ); // 'Register word' key
    // AE_UPDATE_KEY( ?, VK_OEM_FJ_LOYA ); // 'Left OYAYUBI' key
    // AE_UPDATE_KEY( ?, VK_OEM_FJ_ROYA ); // 'Right OYAYUBI' key
    AE_UPDATE_KEY( LeftShift, VK_LSHIFT );
    AE_UPDATE_KEY( RightShift, VK_RSHIFT );
    AE_UPDATE_KEY( LeftControl, VK_LCONTROL );
    AE_UPDATE_KEY( RightControl, VK_RCONTROL );
    AE_UPDATE_KEY( LeftAlt, VK_LMENU );
    AE_UPDATE_KEY( RightAlt, VK_RMENU );
    // AE_UPDATE_KEY( ?, VK_BROWSER_BACK );
    // AE_UPDATE_KEY( ?, VK_BROWSER_FORWARD );
    // AE_UPDATE_KEY( ?, VK_BROWSER_REFRESH );
    // AE_UPDATE_KEY( ?, VK_BROWSER_STOP );
    // AE_UPDATE_KEY( ?, VK_BROWSER_SEARCH );
    // AE_UPDATE_KEY( ?, VK_BROWSER_FAVORITES );
    // AE_UPDATE_KEY( ?, VK_BROWSER_HOME );
    // AE_UPDATE_KEY( ?, VK_VOLUME_MUTE );
    // AE_UPDATE_KEY( ?, VK_VOLUME_DOWN );
    // AE_UPDATE_KEY( ?, VK_VOLUME_UP );
    // AE_UPDATE_KEY( ?, VK_MEDIA_NEXT_TRACK );
    // AE_UPDATE_KEY( ?, VK_MEDIA_PREV_TRACK );
    // AE_UPDATE_KEY( ?, VK_MEDIA_STOP );
    // AE_UPDATE_KEY( ?, VK_MEDIA_PLAY_PAUSE );
    // AE_UPDATE_KEY( ?, VK_LAUNCH_MAIL );
    // AE_UPDATE_KEY( ?, VK_LAUNCH_MEDIA_SELECT );
    // AE_UPDATE_KEY( ?, VK_LAUNCH_APP1 );
    // AE_UPDATE_KEY( ?, VK_LAUNCH_APP2 );
    AE_UPDATE_KEY( Semicolon, VK_OEM_1 ); // ';:' for US
    AE_UPDATE_KEY( Equals, VK_OEM_PLUS ); // '+' any country
    AE_UPDATE_KEY( Comma, VK_OEM_COMMA ); // ',' any country
    AE_UPDATE_KEY( Minus, VK_OEM_MINUS ); // '-' any country
    AE_UPDATE_KEY( Period, VK_OEM_PERIOD ); // '.' any country
    AE_UPDATE_KEY( Slash, VK_OEM_2 ); // '/?' for US
    AE_UPDATE_KEY( Tilde, VK_OEM_3 ); // '`~' for US
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_A );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_B );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_X );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_Y );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_RIGHT_SHOULDER );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_LEFT_SHOULDER );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_LEFT_TRIGGER );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_RIGHT_TRIGGER );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_DPAD_UP );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_DPAD_DOWN );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_DPAD_LEFT );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_DPAD_RIGHT );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_MENU );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_VIEW );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_LEFT_THUMBSTICK_UP );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_LEFT_THUMBSTICK_DOWN );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_LEFT_THUMBSTICK_LEFT );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_RIGHT_THUMBSTICK_UP );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT );
    // AE_UPDATE_KEY( ?, VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT );
    AE_UPDATE_KEY( LeftBracket, VK_OEM_4 ); //  '[{' for US
    AE_UPDATE_KEY( Backslash, VK_OEM_5 ); //  '\|' for US
    AE_UPDATE_KEY( RightBracket, VK_OEM_6 ); //  ']}' for US
    AE_UPDATE_KEY( Apostrophe, VK_OEM_7 ); //  ''"' for US
    // AE_UPDATE_KEY( ?, VK_OEM_8 );
    // AE_UPDATE_KEY( ?, VK_OEM_AX ); //  'AX' key on Japanese AX kbd
    // AE_UPDATE_KEY( ?, VK_OEM_10 ); //  "<>" or "\|" on RT 102-key kbd.
    // AE_UPDATE_KEY( ?, VK_ICO_HELP ); //  Help key on ICO
    // AE_UPDATE_KEY( ?, VK_ICO_00 ); //  00 key on ICO
    // AE_UPDATE_KEY( ?, VK_PROCESSKEY );
    // AE_UPDATE_KEY( ?, VK_ICO_CLEAR );
    // AE_UPDATE_KEY( ?, VK_PACKET );
    // AE_UPDATE_KEY( ?, VK_OEM_RESET );
    // AE_UPDATE_KEY( ?, VK_OEM_JUMP );
    // AE_UPDATE_KEY( ?, VK_OEM_PA1 );
    // AE_UPDATE_KEY( ?, VK_OEM_PA2 );
    // AE_UPDATE_KEY( ?, VK_OEM_PA3 );
    // AE_UPDATE_KEY( ?, VK_OEM_WSCTRL );
    // AE_UPDATE_KEY( ?, VK_OEM_CUSEL );
    // AE_UPDATE_KEY( ?, VK_OEM_ATTN );
    // AE_UPDATE_KEY( ?, VK_OEM_FINISH );
    // AE_UPDATE_KEY( ?, VK_OEM_COPY );
    // AE_UPDATE_KEY( ?, VK_OEM_AUTO );
    // AE_UPDATE_KEY( ?, VK_OEM_ENLW );
    // AE_UPDATE_KEY( ?, VK_OEM_BACKTAB );
    // AE_UPDATE_KEY( ?, VK_ATTN );
    // AE_UPDATE_KEY( ?, VK_CRSEL );
    // AE_UPDATE_KEY( ?, VK_EXSEL );
    // AE_UPDATE_KEY( ?, VK_EREOF );
    // AE_UPDATE_KEY( ?, VK_PLAY );
    // AE_UPDATE_KEY( ?, VK_ZOOM );
    // AE_UPDATE_KEY( ?, VK_NONAME );
    // AE_UPDATE_KEY( ?, VK_PA1 );
    // AE_UPDATE_KEY( ?, VK_OEM_CLEAR );
  }
#undef AE_UPDATE_KEY
#elif _AE_OSX_
#define AE_UPDATE_KEY( _aek, _vk ) m_keys[ (int)ae::Key::_aek ] = keyStates[ _vk / 32 ] & ( 1 << ( _vk % 32 ) )
  KeyMap _keyStates;
  GetKeys(_keyStates);
  uint32_t* keyStates = (uint32_t*)_keyStates;
  AE_UPDATE_KEY( A, kVK_ANSI_A );
  AE_UPDATE_KEY( S, kVK_ANSI_S );
  AE_UPDATE_KEY( D, kVK_ANSI_D );
  AE_UPDATE_KEY( F, kVK_ANSI_F );
  AE_UPDATE_KEY( H, kVK_ANSI_H );
  AE_UPDATE_KEY( G, kVK_ANSI_G );
  AE_UPDATE_KEY( Z, kVK_ANSI_Z );
  AE_UPDATE_KEY( X, kVK_ANSI_X );
  AE_UPDATE_KEY( C, kVK_ANSI_C );
  AE_UPDATE_KEY( V, kVK_ANSI_V );
  AE_UPDATE_KEY( B, kVK_ANSI_B );
  AE_UPDATE_KEY( Q, kVK_ANSI_Q );
  AE_UPDATE_KEY( W, kVK_ANSI_W );
  AE_UPDATE_KEY( E, kVK_ANSI_E );
  AE_UPDATE_KEY( R, kVK_ANSI_R );
  AE_UPDATE_KEY( Y, kVK_ANSI_Y );
  AE_UPDATE_KEY( T, kVK_ANSI_T );
  AE_UPDATE_KEY( Num1, kVK_ANSI_1 );
  AE_UPDATE_KEY( Num2, kVK_ANSI_2 );
  AE_UPDATE_KEY( Num3, kVK_ANSI_3 );
  AE_UPDATE_KEY( Num4, kVK_ANSI_4 );
  AE_UPDATE_KEY( Num6, kVK_ANSI_6 );
  AE_UPDATE_KEY( Num5, kVK_ANSI_5 );
  AE_UPDATE_KEY( Equals, kVK_ANSI_Equal );
  AE_UPDATE_KEY( Num9, kVK_ANSI_9 );
  AE_UPDATE_KEY( Num7, kVK_ANSI_7 );
  AE_UPDATE_KEY( Minus, kVK_ANSI_Minus );
  AE_UPDATE_KEY( Num8, kVK_ANSI_8 );
  AE_UPDATE_KEY( Num0, kVK_ANSI_0 );
  AE_UPDATE_KEY( RightBracket, kVK_ANSI_RightBracket );
  AE_UPDATE_KEY( O, kVK_ANSI_O );
  AE_UPDATE_KEY( U, kVK_ANSI_U );
  AE_UPDATE_KEY( LeftBracket, kVK_ANSI_LeftBracket );
  AE_UPDATE_KEY( I, kVK_ANSI_I );
  AE_UPDATE_KEY( P, kVK_ANSI_P );
  AE_UPDATE_KEY( L, kVK_ANSI_L );
  AE_UPDATE_KEY( J, kVK_ANSI_J );
  AE_UPDATE_KEY( Apostrophe, kVK_ANSI_Quote );
  AE_UPDATE_KEY( K, kVK_ANSI_K );
  AE_UPDATE_KEY( Semicolon, kVK_ANSI_Semicolon );
  AE_UPDATE_KEY( Backslash, kVK_ANSI_Backslash );
  AE_UPDATE_KEY( Comma, kVK_ANSI_Comma );
  AE_UPDATE_KEY( Slash, kVK_ANSI_Slash );
  AE_UPDATE_KEY( N, kVK_ANSI_N );
  AE_UPDATE_KEY( M, kVK_ANSI_M );
  AE_UPDATE_KEY( Period, kVK_ANSI_Period );
  AE_UPDATE_KEY( Tilde, kVK_ANSI_Grave );
  AE_UPDATE_KEY( NumPadPeriod, kVK_ANSI_KeypadDecimal );
  AE_UPDATE_KEY( NumPadMultiply, kVK_ANSI_KeypadMultiply );
  AE_UPDATE_KEY( NumPadPlus, kVK_ANSI_KeypadPlus );
  //AE_UPDATE_KEY( NumPadClear, kVK_ANSI_KeypadClear );
  AE_UPDATE_KEY( NumPadDivide, kVK_ANSI_KeypadDivide );
  AE_UPDATE_KEY( NumPadEnter, kVK_ANSI_KeypadEnter );
  AE_UPDATE_KEY( NumPadMinus, kVK_ANSI_KeypadMinus );
  AE_UPDATE_KEY( NumPadEquals, kVK_ANSI_KeypadEquals );
  AE_UPDATE_KEY( NumPad0, kVK_ANSI_Keypad0 );
  AE_UPDATE_KEY( NumPad1, kVK_ANSI_Keypad1 );
  AE_UPDATE_KEY( NumPad2, kVK_ANSI_Keypad2 );
  AE_UPDATE_KEY( NumPad3, kVK_ANSI_Keypad3 );
  AE_UPDATE_KEY( NumPad4, kVK_ANSI_Keypad4 );
  AE_UPDATE_KEY( NumPad5, kVK_ANSI_Keypad5 );
  AE_UPDATE_KEY( NumPad6, kVK_ANSI_Keypad6 );
  AE_UPDATE_KEY( NumPad7, kVK_ANSI_Keypad7 );
  AE_UPDATE_KEY( NumPad8, kVK_ANSI_Keypad8 );
  AE_UPDATE_KEY( NumPad9, kVK_ANSI_Keypad9 );
  AE_UPDATE_KEY( Enter, kVK_Return );
  AE_UPDATE_KEY( Tab, kVK_Tab );
  AE_UPDATE_KEY( Space, kVK_Space );
  AE_UPDATE_KEY( Backspace, kVK_Delete );
  AE_UPDATE_KEY( Escape, kVK_Escape );
  AE_UPDATE_KEY( LeftSuper, kVK_Command );
  AE_UPDATE_KEY( LeftShift, kVK_Shift );
  AE_UPDATE_KEY( CapsLock, kVK_CapsLock );
  AE_UPDATE_KEY( LeftAlt, kVK_Option );
  AE_UPDATE_KEY( LeftControl, kVK_Control );
  AE_UPDATE_KEY( RightSuper, kVK_RightCommand );
  AE_UPDATE_KEY( RightShift, kVK_RightShift );
  AE_UPDATE_KEY( RightAlt, kVK_RightOption );
  AE_UPDATE_KEY( RightControl, kVK_RightControl );
  //AE_UPDATE_KEY( Function, kVK_Function );
  //AE_UPDATE_KEY( F17, kVK_F17 );
  //AE_UPDATE_KEY( VolumeUp, kVK_VolumeUp );
  //AE_UPDATE_KEY( VolumeDown, kVK_VolumeDown );
  //AE_UPDATE_KEY( Mute, kVK_Mute );
  //AE_UPDATE_KEY( F18, kVK_F18 );
  //AE_UPDATE_KEY( F19, kVK_F19 );
  //AE_UPDATE_KEY( F20, kVK_F20 );
  AE_UPDATE_KEY( F5, kVK_F5 );
  AE_UPDATE_KEY( F6, kVK_F6 );
  AE_UPDATE_KEY( F7, kVK_F7 );
  AE_UPDATE_KEY( F3, kVK_F3 );
  AE_UPDATE_KEY( F8, kVK_F8 );
  AE_UPDATE_KEY( F9, kVK_F9 );
  AE_UPDATE_KEY( F11, kVK_F11 );
  //AE_UPDATE_KEY( F13, kVK_F13 );
  //AE_UPDATE_KEY( F16, kVK_F16 );
  //AE_UPDATE_KEY( F14, kVK_F14 );
  AE_UPDATE_KEY( F10, kVK_F10 );
  AE_UPDATE_KEY( F12, kVK_F12 );
  //AE_UPDATE_KEY( F15, kVK_F15 );
  //AE_UPDATE_KEY( Help, kVK_Help );
  AE_UPDATE_KEY( Home, kVK_Home );
  AE_UPDATE_KEY( PageUp, kVK_PageUp );
  AE_UPDATE_KEY( Delete, kVK_ForwardDelete );
  AE_UPDATE_KEY( F4, kVK_F4 );
  AE_UPDATE_KEY( End, kVK_End );
  AE_UPDATE_KEY( F2, kVK_F2 );
  AE_UPDATE_KEY( PageDown, kVK_PageDown );
  AE_UPDATE_KEY( F1, kVK_F1 );
  AE_UPDATE_KEY( Left, kVK_LeftArrow );
  AE_UPDATE_KEY( Right, kVK_RightArrow );
  AE_UPDATE_KEY( Down, kVK_DownArrow );
  AE_UPDATE_KEY( Up, kVK_UpArrow );
#undef AE_UPDATE_KEY
#endif

  // Update meta key
#if _AE_APPLE_
  m_keys[ (int)ae::Key::LeftMeta ] = m_keys[ (int)ae::Key::LeftSuper ];
  m_keys[ (int)ae::Key::RightMeta ] = m_keys[ (int)ae::Key::RightSuper ];
#else
  m_keys[ (int)ae::Key::LeftMeta ] = m_keys[ (int)ae::Key::LeftControl ];
  m_keys[ (int)ae::Key::RightMeta ] = m_keys[ (int)ae::Key::RightControl ];
#endif
}

void Input::SetMouseCaptured( bool enable )
{
  if ( m_captureMouse && !enable )
  {
    CGDisplayShowCursor( kCGDirectMainDisplay );
  }
  else if ( !m_captureMouse && enable )
  {
    CGDisplayHideCursor( kCGDirectMainDisplay );
    m_positionSet = false;
  }
  m_captureMouse = enable;
}

void Input::SetTextMode( bool enabled )
{
  if ( m_textMode != enabled )
  {
    m_textMode = enabled;
    
    NSWindow* nsWindow = (NSWindow*)m_window->window;
    if ( m_textMode )
    {
      aeTextInputDelegate* textInput = (aeTextInputDelegate*)m_textInputHandler;
      [nsWindow makeFirstResponder:textInput];
    }
    else
    {
      NSOpenGLView* glView = [nsWindow contentView];
      [nsWindow makeFirstResponder:glView];
    }
  }
}

bool Input::Get( ae::Key key ) const
{
  return m_keys[ static_cast< int >( key ) ];
}

bool Input::GetPrev( ae::Key key ) const
{
  return m_keysPrev[ static_cast< int >( key ) ];
}

void Input::m_SetMousePos( ae::Int2 pos )
{
  AE_ASSERT( m_window );
  pos -= m_window->GetPosition();
  if ( m_positionSet )
  {
    mouse.movement = pos - mouse.position;
  }
  mouse.position = pos;
  m_positionSet = true;
}

//------------------------------------------------------------------------------
// ae::FileFilter member functions
//------------------------------------------------------------------------------
FileFilter::FileFilter( const char* desc, const char** ext, uint32_t extensionCount )
{
  extensionCount = ae::Min( extensionCount, countof( extensions ) );
  description = desc;
  for ( uint32_t i = 0; i < extensionCount; i++ )
  {
    extensions[ i ] = ext[ i ];
  }
}

//------------------------------------------------------------------------------
// ae::FileSystem member functions
//------------------------------------------------------------------------------
// @TODO: Remove separator define when cleaning up path functions
#if _AE_WINDOWS_
  #define AE_PATH_SEPARATOR '\\'
#else
  #define AE_PATH_SEPARATOR '/'
#endif

#if _AE_APPLE_
bool FileSystem_GetUserDir( Str256* outDir )
{
  // Something like /Users/someone/Library/Application Support
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
  NSString* prefsPath = [paths lastObject];
  if ( [prefsPath length] )
  {
    *outDir = [prefsPath UTF8String];
    return true;
  }
  return false;
}
bool FileSystem_GetCacheDir( Str256* outDir )
{
  // Something like /User/someone/Library/Caches
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
  NSString* cachesPath = [paths lastObject];
  if ( [cachesPath length] )
  {
    *outDir = [cachesPath UTF8String];
    return true;
  }
  return false;
}
#elif _AE_LINUX_
bool FileSystem_GetUserDir( Str256* outDir )
{
  return false;
}
bool FileSystem_GetCacheDir( Str256* outDir )
{
  // Something like /users/someone/.cache
  const char* cacheFolderName = ".cache";
  const char* homeDir = getenv( "HOME" );
  if ( homeDir && homeDir[ 0 ] )
  {
    *outDir = homeDir;
    FileSystem::AppendToPath( outDir, cacheFolderName );
    return true;
  }
  else
  {
    const passwd* pw = getpwuid( getuid() );
    if ( pw )
    {
      const char* homeDir = pw->pw_dir;
      if ( homeDir && homeDir[ 0 ] )
      {
        *outDir = homeDir;
        FileSystem::AppendToPath( outDir, cacheFolderName );
        return true;
      }
    }
  }
  return false;
}
#elif _AE_WINDOWS_
bool FileSystem_GetDir( KNOWNFOLDERID folderId, Str256* outDir )
{
  bool result = false;
  PWSTR wpath = nullptr;
  // SHGetKnownFolderPath does not include trailing backslash
  HRESULT pathResult = SHGetKnownFolderPath( folderId, 0, nullptr, &wpath );
  if ( pathResult == S_OK )
  {
    char path[ outDir->MaxLength() + 1 ];
    int32_t pathLen = (int32_t)wcstombs( path, wpath, outDir->MaxLength() );
    if ( pathLen > 0 )
    {
      path[ pathLen ] = 0;

      *outDir = path;
      result = true;
    }
  }
  CoTaskMemFree( wpath ); // Always free even on failure
  return result;
}
bool FileSystem_GetUserDir( Str256* outDir )
{
  // Something like C:\Users\someone\AppData\Local\Company\Game
  return FileSystem_GetDir( FOLDERID_RoamingAppData, outDir );
}
bool FileSystem_GetCacheDir( Str256* outDir )
{
  // Something like C:\Users\someone\AppData\Local\Company\Game
  return FileSystem_GetDir( FOLDERID_LocalAppData, outDir );
}
#elif _AE_EMSCRIPTEN_
bool FileSystem_GetUserDir( Str256* outDir )
{
  return false;
}
bool FileSystem_GetCacheDir( Str256* outDir )
{
  return false;
}
#endif

void FileSystem::Initialize( const char* dataDir, const char* organizationName, const char* applicationName )
{
  AE_ASSERT_MSG( organizationName && organizationName[ 0 ], "Organization name must not be empty" );
  AE_ASSERT_MSG( applicationName && applicationName[ 0 ], "Application name must not be empty" );

  const char* validateOrgName = organizationName;
  while ( *validateOrgName )
  {
    AE_ASSERT_MSG( isalnum( *validateOrgName ) || ( *validateOrgName == '_' )  || ( *validateOrgName == '-' ), "Invalid organization name '#'. Only alphanumeric characters and undersrcores are supported.", organizationName );
    validateOrgName++;
  }
  const char* validateAppName = applicationName;
  while ( *validateAppName )
  {
    AE_ASSERT_MSG( isalnum( *validateAppName ) || ( *validateAppName == '_' ) || ( *validateAppName == '-' ), "Invalid application name '#'. Only alphanumeric characters and undersrcores are supported.", applicationName );
    validateAppName++;
  }

  m_SetDataDir( dataDir ? dataDir : "" );
  m_SetUserDir( organizationName, applicationName );
  m_SetCacheDir( organizationName, applicationName );
  m_SetUserSharedDir( organizationName );
  m_SetCacheSharedDir( organizationName );
}

void FileSystem::m_SetDataDir( const char* dataDir )
{
  m_dataDir = GetAbsolutePath( dataDir );
  // Append slash if not empty and is currently missing
  if ( m_dataDir.Length() )
  {
    char sepatator[ 2 ] = { AE_PATH_SEPARATOR, 0 };
    AppendToPath( &m_dataDir, sepatator );
  }
}

void FileSystem::m_SetUserDir( const char* organizationName, const char* applicationName )
{
  const Str16 pathChar( 1, AE_PATH_SEPARATOR );
  m_userDir = "";
  if ( FileSystem_GetUserDir( &m_userDir ) )
  {
    AE_ASSERT( m_userDir.Length() );
    m_userDir += pathChar;
    m_userDir += organizationName;
    m_userDir += pathChar;
    m_userDir += applicationName;
    m_userDir += pathChar;
    if ( !CreateFolder( m_userDir.c_str() ) )
    {
      m_userDir = "";
    }
  }
}

void FileSystem::m_SetCacheDir( const char* organizationName, const char* applicationName )
{
  const Str16 pathChar( 1, AE_PATH_SEPARATOR );
  m_cacheDir = "";
  if ( FileSystem_GetCacheDir( &m_cacheDir ) )
  {
    AE_ASSERT( m_cacheDir.Length() );
    m_cacheDir += pathChar;
    m_cacheDir += organizationName;
    m_cacheDir += pathChar;
    m_cacheDir += applicationName;
    m_cacheDir += pathChar;
    if ( !CreateFolder( m_cacheDir.c_str() ) )
    {
      m_cacheDir = "";
    }
  }
}

void FileSystem::m_SetUserSharedDir( const char* organizationName )
{
  const Str16 pathChar( 1, AE_PATH_SEPARATOR );
  m_userSharedDir = "";
  if ( FileSystem_GetUserDir( &m_userSharedDir ) )
  {
    AE_ASSERT( m_userSharedDir.Length() );
    m_userSharedDir += pathChar;
    m_userSharedDir += organizationName;
    m_userSharedDir += pathChar;
    m_userSharedDir += "shared";
    m_userSharedDir += pathChar;
    if ( !CreateFolder( m_userSharedDir.c_str() ) )
    {
      m_userSharedDir = "";
    }
  }
}

void FileSystem::m_SetCacheSharedDir( const char* organizationName )
{
  const Str16 pathChar( 1, AE_PATH_SEPARATOR );
  m_cacheSharedDir = "";
  if ( FileSystem_GetCacheDir( &m_cacheSharedDir ) )
  {
    AE_ASSERT( m_cacheSharedDir.Length() );
    m_cacheSharedDir += pathChar;
    m_cacheSharedDir += organizationName;
    m_cacheSharedDir += pathChar;
    m_cacheSharedDir += "shared";
    m_cacheSharedDir += pathChar;
    if ( !CreateFolder( m_cacheSharedDir.c_str() ) )
    {
      m_cacheSharedDir = "";
    }
  }
}

uint32_t FileSystem::GetSize( Root root, const char* filePath ) const
{
  Str256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += filePath;
    return GetSize( fullName.c_str() );
  }
  return 0;
}

uint32_t FileSystem::Read( Root root, const char* filePath, void* buffer, uint32_t bufferSize ) const
{
  Str256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += filePath;
    return Read( fullName.c_str(), buffer, bufferSize );
  }
  return 0;
}

uint32_t FileSystem::Write( Root root, const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs ) const
{
  Str256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += filePath;
    return Write( fullName.c_str(), buffer, bufferSize, createIntermediateDirs );
  }
  return 0;
}

bool FileSystem::CreateFolder( Root root, const char* folderPath ) const
{
  Str256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += folderPath;
    return CreateFolder( fullName.c_str() );
  }
  return false;
}

void FileSystem::ShowFolder( Root root, const char* folderPath ) const
{
  Str256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += folderPath;
    ShowFolder( fullName.c_str() );
  }
}

bool FileSystem::GetRootDir( Root root, Str256* outDir ) const
{
  switch ( root )
  {
    case Root::Data:
      if ( m_dataDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_dataDir;
        }
        return true;
      }
      break;
    case Root::User:
      if ( m_userDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_userDir;
        }
        return true;
      }
      break;
    case Root::Cache:
      if ( m_cacheDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_cacheDir;
        }
        return true;
      }
      break;
    case Root::UserShared:
      if ( m_userSharedDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_userSharedDir;
        }
        return true;
      }
      break;
    case Root::CacheShared:
      if ( m_cacheSharedDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_cacheSharedDir;
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

uint32_t FileSystem::GetSize( const char* filePath )
{
#if _AE_APPLE_
  CFStringRef filePathIn = CFStringCreateWithCString( kCFAllocatorDefault, filePath, kCFStringEncodingUTF8 );
  CFURLRef appUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), filePathIn, nullptr, nullptr );
  CFStringRef bundlePath = nullptr;
  if ( appUrl )
  {
    bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
    filePath = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
  }
#endif
  
  uint32_t fileSize = 0;
  if ( FILE* file = fopen( filePath, "rb" ) )
  {
    fseek( file, 0, SEEK_END );
    fileSize = (uint32_t)ftell( file );
    fclose( file );
  }
  
#if _AE_APPLE_
  if ( bundlePath ) { CFRelease( bundlePath ); }
  if ( appUrl ) { CFRelease( appUrl ); }
  CFRelease( filePathIn );
#endif
  
  return fileSize;
}

uint32_t FileSystem::Read( const char* filePath, void* buffer, uint32_t bufferSize )
{
#if _AE_APPLE_
  CFStringRef filePathIn = CFStringCreateWithCString( kCFAllocatorDefault, filePath, kCFStringEncodingUTF8 );
  CFURLRef appUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), filePathIn, nullptr, nullptr );
  CFStringRef bundlePath = nullptr;
  if ( appUrl )
  {
    CFStringRef bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
    filePath = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
  }
#endif

  uint32_t resultLen = 0;
  
  if ( FILE* file = fopen( filePath, "rb" ) )
  {
    fseek( file, 0, SEEK_END );
    resultLen = (uint32_t)ftell( file );
    fseek( file, 0, SEEK_SET );

    if ( resultLen <= bufferSize )
    {
      size_t readLen = fread( buffer, sizeof(uint8_t), resultLen, file );
      AE_ASSERT( readLen == resultLen );
    }
    else
    {
      resultLen = 0;
    }

    fclose( file );
  }
  
#if _AE_APPLE_
  if ( bundlePath ) { CFRelease( bundlePath ); }
  if ( appUrl ) { CFRelease( appUrl ); }
  CFRelease( filePathIn );
#endif

  return resultLen;
}

uint32_t FileSystem::Write( const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs )
{
  if ( createIntermediateDirs )
  {
    auto dir = GetDirectoryFromPath( filePath );
    if ( dir.Length() && !FileSystem::CreateFolder( dir.c_str() ) )
    {
      return 0;
    }
  }
  
  FILE* file = fopen( filePath, "wb" );
  if ( !file )
  {
    return 0;
  }

  fwrite( buffer, sizeof(uint8_t), bufferSize, file );
  fclose( file );

  return bufferSize;
}

bool FileSystem::CreateFolder( const char* folderPath )
{
#if _AE_APPLE_
  NSString* path = [NSString stringWithUTF8String:folderPath];
  NSError* error = nil;
  BOOL success = [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:&error];
  return success && !error;
#elif _AE_WINDOWS_
  switch ( SHCreateDirectoryExA( nullptr, folderPath, nullptr ) )
  {
    case ERROR_SUCCESS:
    case ERROR_ALREADY_EXISTS:
      return true;
    default:
      return false;
  }
#elif _AE_LINUX_
  // @TODO: Recursive https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
  int result = mkdir( folderPath, 0777 ) == 0;
  return ( result == 0 ) || errno == EEXIST;
#endif
  return false;
}

void FileSystem::ShowFolder( const char* folderPath )
{
#if _AE_WINDOWS_
  ShellExecuteA( NULL, "explore", folderPath, NULL, NULL, SW_SHOWDEFAULT );
#elif _AE_OSX_
  NSString* path = [NSString stringWithUTF8String:folderPath];
  [[NSWorkspace sharedWorkspace] selectFile:path inFileViewerRootedAtPath:@""];
#endif
  // @TODO: Linux
}

Str256 FileSystem::GetAbsolutePath( const char* filePath )
{
#if _AE_APPLE_
  NSString* currentPath = [[NSFileManager defaultManager] currentDirectoryPath];
  if ( [currentPath isEqualToString:@"/"] )
  {
    // Current path is root. This is Xcode's new default as of 7/21
    // @TODO: When inside an App fallback to the resource directory instead
    return "";
  }
  NSString* path = [NSString stringWithUTF8String:filePath];
  AE_ASSERT( [currentPath characterAtIndex:0] != '~' );
  NSURL* currentPathUrl = [NSURL fileURLWithPath:currentPath];
  NSURL* absoluteUrl = [NSURL URLWithString:path relativeToURL:currentPathUrl];
  return [absoluteUrl.path UTF8String];
#elif _AE_WINDOWS_
  char result[ ae::Str256::MaxLength() ];
  if ( _fullpath( result, filePath, countof(result) ) )
  {
    return result;
  }
  else
  {
    return "";
  }
#endif
  // @TODO: Linux
  return filePath;
}

const char* FileSystem::GetFileNameFromPath( const char* filePath )
{
  const char* s0 = strrchr( filePath, '/' );
  const char* s1 = strrchr( filePath, '\\' );
  
  if ( s1 && s0 )
  {
    return ( ( s1 > s0 ) ? s1 : s0 ) + 1;
  }
  else if ( s0 )
  {
    return s0 + 1;
  }
  else if ( s1 )
  {
    return s1 + 1;
  }
  else
  {
    return filePath;
  }
}

const char* FileSystem::GetFileExtFromPath( const char* filePath )
{
  const char* fileName = GetFileNameFromPath( filePath );
  const char* s = strchr( fileName, '.' );
  if ( s )
  {
    return s + 1;
  }
  else
  {
    // @NOTE: Return end of given string in case pointer arithmetic is being done by user
    uint32_t len = (uint32_t)strlen( fileName );
    return fileName + len;
  }
}

Str256 FileSystem::GetDirectoryFromPath( const char* filePath )
{
  const char* fileName = GetFileNameFromPath( filePath );
  return Str256( fileName - filePath, filePath );
}

void FileSystem::AppendToPath( Str256* path, const char* str )
{
  if ( !path )
  {
    return;
  }
  
  // @TODO: Handle paths that already have a file name and extension
  
  // @TODO: Handle one or more path separators at end of path
  if ( uint32_t pathLen = path->Length() )
  {
    char lastChar = path->operator[]( pathLen - 1 );
    if ( lastChar != '/' && lastChar != '\\' )
    {
      path->Append( Str16( 1, AE_PATH_SEPARATOR ) );

      if ( ( str[ 0 ] == '/' || str[ 0 ] == '\\' ) && !str[ 1 ] )
      {
        // @HACK: Append single separator when given separator only string
        return;
      }
    }
  }
  
  // @TODO: Handle one or more path separators at front of str
  *path += str;
}

#if _AE_WINDOWS_

void FixPathExtension( const char* extension, std::filesystem::path* pathOut )
{
  // Set if path has no extension
  if ( !pathOut->has_extension() )
  {
    pathOut->replace_extension( extension );
    return;
  }

  // Set if extension chars are just periods
  std::string pathExt = pathOut->extension().string();
  if ( pathExt[ pathExt.length() - 1 ] == '.' )
  {
    pathOut->concat( std::string( "." ) + extension );
    return;
  }
}

ae::Array< char > CreateFilterString( const Array< FileFilter, 8 >& filters )
{
  ae::Array< char > result( AE_ALLOC_TAG_FILE );
  if ( !filters.Length() )
  {
    return result;
  }

  ae::Array< char > tempFilterStr( AE_ALLOC_TAG_FILE );
  for ( uint32_t i = 0; i < filters.Length(); i++ )
  {
    const FileFilter& filter = filters[ i ];
    tempFilterStr.Clear();

    uint32_t extCount = 0;
    for ( uint32_t j = 0; j < countof( FileFilter::extensions ); j++ )
    {
      const char* ext = filter.extensions[ j ];
      if ( ext == nullptr )
      {
        continue;
      }

      // Validate extension
      if ( strcmp( "*", ext ) != 0 )
      {
        for ( const char* extCheck = ext; *extCheck; extCheck++ )
        {
          if ( !std::isalnum( *extCheck ) )
          {
            AE_FAIL_MSG( "File extensions must only contain alphanumeric characters or '*': #", ext );
            result.Clear();
            return result;
          }
        }
      }

      if ( extCount == 0 )
      {
        tempFilterStr.Append( "*.", 2 );
      }
      else
      {
        tempFilterStr.Append( ";*.", 3 );
      }

      tempFilterStr.Append( ext, (uint32_t)strlen( ext ) );
      extCount++;
    }

    if ( extCount == 0 )
    {
      tempFilterStr.Append( "*.*", 3 );
    }

    // Description
    result.Append( filter.description, (uint32_t)strlen( filter.description ) );
    result.Append( " (", 2 );
    result.Append( &tempFilterStr[ 0 ], tempFilterStr.Length() );
    result.Append( ")", 2 ); // Every description must be null terminated

    result.Append( &tempFilterStr[ 0 ], tempFilterStr.Length() );
    result.Append( "", 1 ); // Every filter must be null terminated
  }

  // Append final null terminator because GetOpenFileName requires double termination at end of string
  result.Append( "", 1 );

  return result;
}

ae::Array< std::string > FileSystem::OpenDialog( const FileDialogParams& params )
{
  ae::Array< char > filterStr = CreateFilterString( params.filters );

  char fileNameBuf[ 2048 ]; // Not just MAX_PATH
  fileNameBuf[ 0 ] = 0;

  // Set parameters for Windows function call
  OPENFILENAMEA winParams;
  ZeroMemory( &winParams, sizeof( winParams ) );
  winParams.lStructSize = sizeof( winParams );
  winParams.hwndOwner = params.window ? (HWND)params.window->window : nullptr;
  if ( params.windowTitle && params.windowTitle[ 0 ] )
  {
    winParams.lpstrTitle = params.windowTitle;
  }
  winParams.lpstrFile = fileNameBuf;
  winParams.nMaxFile = sizeof( fileNameBuf );
  winParams.lpstrFilter = filterStr.Length() ? &filterStr[ 0 ] : "All Files (*.*)\0*.*\0";
  winParams.nFilterIndex = 1;
  winParams.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  if ( params.allowMultiselect )
  {
    winParams.Flags |= OFN_ALLOWMULTISELECT;
  }

  // Open window
  if ( GetOpenFileNameA( &winParams ) )
  {
    uint32_t offset = (uint32_t)strlen( winParams.lpstrFile ) + 1; // Double null terminated
    if ( winParams.lpstrFile[ offset ] == 0 )
    {
      return ae::Array< std::string >( AE_ALLOC_TAG_FILE, 1, winParams.lpstrFile );
    }
    else // Multiple results
    {
      const char* head = winParams.lpstrFile;
      const char* directory = head;
      head += offset; // Null separated
      ae::Array< std::string > result = AE_ALLOC_TAG_FILE;
      while ( *head )
      {
        auto&& r = result.Append( directory );
        r += AE_PATH_SEPARATOR;
        r += head;

        offset = (uint32_t)strlen( head ) + 1; // Double null terminated
        head += offset; // Null separated
      }
      return result;
    }
  }

  return ae::Array< std::string >( AE_ALLOC_TAG_FILE );
}

std::string FileSystem::SaveDialog( const FileDialogParams& params )
{
  ae::Array< char > filterStr = CreateFilterString( params.filters );

  char fileNameBuf[ MAX_PATH ];
  fileNameBuf[ 0 ] = 0;

  // Set parameters for Windows function call
  OPENFILENAMEA winParams;
  ZeroMemory( &winParams, sizeof( winParams ) );
  winParams.lStructSize = sizeof( winParams );
  winParams.hwndOwner = params.window ? (HWND)params.window->window : nullptr;
  if ( params.windowTitle && params.windowTitle[ 0 ] )
  {
    winParams.lpstrTitle = params.windowTitle;
  }
  winParams.lpstrFile = fileNameBuf;
  winParams.nMaxFile = sizeof( fileNameBuf );
  winParams.lpstrFilter = filterStr.Length() ? &filterStr[ 0 ] : "All Files (*.*)\0*.*\0";
  winParams.nFilterIndex = 1;
  winParams.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

  if ( GetSaveFileNameA( &winParams ) )
  {
    std::filesystem::path result = winParams.lpstrFile;
    if ( winParams.nFilterIndex >= 1 )
    {
      winParams.nFilterIndex--;
      const char* ext = params.filters[ winParams.nFilterIndex ].extensions[ 0 ];
      
      FixPathExtension( ext, &result );
    }
    return result.string();
  }

  return "";
}

#elif _AE_APPLE_

//------------------------------------------------------------------------------
// OpenDialog not implemented
//------------------------------------------------------------------------------
ae::Array< std::string > FileSystem::OpenDialog( const FileDialogParams& params )
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSWindow* window = (NSWindow*)( params.window ? params.window->window : nullptr );
  NSOpenPanel* dialog = [NSOpenPanel openPanel];
  dialog.canChooseFiles = YES;
  dialog.canChooseDirectories = NO;
  dialog.allowsMultipleSelection = params.allowMultiselect;
  if ( params.windowTitle && params.windowTitle[ 0 ] )
  {
    dialog.message = [NSString stringWithUTF8String:params.windowTitle];
  }
  if ( params.defaultPath && params.defaultPath[ 0 ] )
  {
    ae::Str256 dir = "file://";
    dir += params.defaultPath;
    dialog.directoryURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:dir.c_str()]];
  }
  
  bool allowAny = false;
  NSMutableArray* filters = [NSMutableArray arrayWithCapacity:params.filters.Length()];
  for ( const FileFilter& filter : params.filters )
  {
    for ( const char* ext : filter.extensions )
    {
      if ( ext )
      {
        if ( strcmp( ext, "*" ) == 0 )
        {
          allowAny = true;
        }
        [filters addObject:[NSString stringWithUTF8String:ext]];
      }
    }
  }
  if ( !allowAny )
  {
    [dialog setAllowedFileTypes:filters];
  }
  
  __block bool finished = false;
  __block bool success = false;
  ae::Array< std::string > result = AE_ALLOC_TAG_FILE;
  // Show
  if ( window )
  {
    AE_ASSERT_MSG( params.window->input, "Must initialize ae::Input with ae::Window before creating a file dialog" );
    [dialog beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode)
    {
      success = ( returnCode == NSFileHandlingPanelOKButton );
      finished = true;
    }];
    // Block here until panel returns
    while ( !finished )
    {
      params.window->input->Pump();
      sleep( 0 );
    }
  }
  else
  {
    success = ( [dialog runModal] == NSModalResponseOK );
  }
  // Result
  if ( success )
  {
    if ( dialog.URLs.count )
    {
      for (NSURL* url in dialog.URLs)
      {
        result.Append( url.fileSystemRepresentation );
      }
    }
    else if ( dialog.URL )
    {
      result.Append( dialog.URL.fileSystemRepresentation );
    }
  }
  
  [pool release];
  return result;
}

//------------------------------------------------------------------------------
// SaveDialog not implemented
//------------------------------------------------------------------------------
std::string FileSystem::SaveDialog( const FileDialogParams& params )
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSWindow* window = (NSWindow*)( params.window ? params.window->window : nullptr );
  NSSavePanel* dialog = [NSSavePanel savePanel];
  if ( params.windowTitle && params.windowTitle[ 0 ] )
  {
    dialog.message = [NSString stringWithUTF8String:params.windowTitle];
  }
  if ( params.defaultPath && params.defaultPath[ 0 ] )
  {
    ae::Str256 dir = "file://";
    dir += params.defaultPath;
    dialog.directoryURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:dir.c_str()]];
  }
  
  bool allowAny = false;
  NSMutableArray* filters = [NSMutableArray arrayWithCapacity:params.filters.Length()];
  for ( const FileFilter& filter : params.filters )
  {
    for ( const char* ext : filter.extensions )
    {
      if ( ext )
      {
        if ( strcmp( ext, "*" ) == 0 )
        {
          allowAny = true;
        }
        [filters addObject:[NSString stringWithUTF8String:ext]];
      }
    }
  }
  if ( !allowAny )
  {
    [dialog setAllowedFileTypes:filters];
  }
  
  __block bool finished = false;
  __block bool success = false;
  std::string result;
  // Show
  if ( window )
  {
    AE_ASSERT_MSG( params.window->input, "Must initialize ae::Input with ae::Window before creating a file dialog" );
    [dialog beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode)
    {
      success = ( returnCode == NSFileHandlingPanelOKButton );
      finished = true;
    }];
    // Block here until panel returns
    while ( !finished )
    {
      params.window->input->Pump();
      sleep( 0 );
    }
  }
  else
  {
    success = ( [dialog runModal] == NSModalResponseOK );
  }
  // Result
  if ( success && dialog.URL )
  {
    result = dialog.URL.fileSystemRepresentation;
  }
  
  [pool release];
  return result;
}

#else

//------------------------------------------------------------------------------
// OpenDialog not implemented
//------------------------------------------------------------------------------
Array< std::string > FileSystem::OpenDialog( const FileDialogParams& params )
{
  return { AE_ALLOC_TAG_FILE };
}

//------------------------------------------------------------------------------
// SaveDialog not implemented
//------------------------------------------------------------------------------
std::string FileSystem::SaveDialog( const FileDialogParams& params )
{
  return "";
}

#endif

}  // ae end

//------------------------------------------------------------------------------
// OpenGL includes
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #pragma comment (lib, "opengl32.lib")
  #pragma comment (lib, "glu32.lib")
  #include <gl/GL.h>
  #include <gl/GLU.h>
#elif _AE_EMSCRIPTEN_
  #include <GLES3/gl3.h>
#elif _AE_LINUX_
  #include <GL/gl.h>
  #include <GLES3/gl3.h>
#elif _AE_IOS_
  #include <OpenGLES/ES3/gl.h>
#else
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
  #include <OpenGL/gl3.h>
  #include <OpenGL/gl3ext.h>
#endif

namespace ae
{
#if _AE_IOS_ || _AE_EMSCRIPTEN_
  uint32_t GLMajorVersion = 3;
  uint32_t GLMinorVersion = 0;
#else
  uint32_t GLMajorVersion = 4;
  uint32_t GLMinorVersion = 1;
#endif
bool ReverseZ = false;
}  // ae end

#if _AE_WINDOWS_
// OpenGL function pointers
typedef char GLchar;
typedef intptr_t GLsizeiptr;
typedef intptr_t GLintptr;

// GL_VERSION_1_2
#define GL_TEXTURE_3D                     0x806F
#define GL_CLAMP_TO_EDGE                  0x812F
// GL_VERSION_1_3
#define GL_TEXTURE0                       0x84C0
// GL_VERSION_1_4
#define GL_DEPTH_COMPONENT16              0x81A5
// GL_VERSION_1_5
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
// GL_VERSION_2_0
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
// GL_VERSION_2_1
#define GL_SRGB8                          0x8C41
#define GL_SRGB8_ALPHA8                   0x8C43
// GL_VERSION_3_0
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_FRAMEBUFFER                    0x8D40
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_HALF_FLOAT                     0x140B
#define GL_R8                             0x8229
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_R16UI                          0x8234
// GL_VERSION_3_2
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
// GL_VERSION_4_3
typedef void ( *GLDEBUGPROC )(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
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
void ( *glTextureBarrierNV ) () = nullptr;
// OpenGL Vertex Functions
void ( *glGenVertexArrays ) (GLsizei n, GLuint *arrays ) = nullptr;
void ( *glBindVertexArray ) ( GLuint array ) = nullptr;
void ( *glDeleteVertexArrays ) ( GLsizei n, const GLuint *arrays ) = nullptr;
void ( *glDeleteBuffers ) ( GLsizei n, const GLuint *buffers ) = nullptr;
void ( *glBindBuffer ) ( GLenum target, GLuint buffer ) = nullptr;
void ( *glGenBuffers ) ( GLsizei n, GLuint *buffers ) = nullptr;
void ( *glBufferData ) ( GLenum target, GLsizeiptr size, const void *data, GLenum usage ) = nullptr;
void ( *glBufferSubData ) ( GLenum target, GLintptr offset, GLsizeiptr size, const void *data ) = nullptr;
void ( *glEnableVertexAttribArray ) ( GLuint index ) = nullptr;
void ( *glVertexAttribPointer ) ( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer ) = nullptr;
void ( *glDebugMessageCallback ) ( GLDEBUGPROC callback, const void *userParam ) = nullptr;
#endif

#if _AE_EMSCRIPTEN_
#define glClearDepth glClearDepthf
#endif

// Helpers
#if _AE_DEBUG_
  #define AE_CHECK_GL_ERROR() do { if ( GLenum err = glGetError() ) { AE_FAIL_MSG( "GL Error: #", err ); } } while ( 0 )
#else
  #define AE_CHECK_GL_ERROR() do {} while ( 0 )
#endif

namespace ae {

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
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
#endif
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

#if _AE_DEBUG_ && !_AE_APPLE_
  // Apple platforms only support OpenGL 4.1 and lower
  #define AE_GL_DEBUG_MODE 1
#endif

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

GLenum VertexDataTypeToGL( VertexData::Type type )
{
  switch ( type )
  {
    case VertexData::Type::UInt8:
      return GL_UNSIGNED_BYTE;
    case VertexData::Type::UInt16:
      return GL_UNSIGNED_SHORT;
    case VertexData::Type::UInt32:
      return GL_UNSIGNED_INT;
    case VertexData::Type::NormalizedUInt8:
      return GL_UNSIGNED_BYTE;
    case VertexData::Type::NormalizedUInt16:
      return GL_UNSIGNED_SHORT;
    case VertexData::Type::NormalizedUInt32:
      return GL_UNSIGNED_INT;
    case VertexData::Type::Float:
      return GL_FLOAT;
    default:
      AE_FAIL();
      return 0;
  }
}

typedef uint32_t aeQuadIndex;
const uint32_t aeQuadVertCount = 4;
const uint32_t aeQuadIndexCount = 6;
extern const Vec3 aeQuadVertPos[ aeQuadVertCount ];
extern const Vec2 aeQuadVertUvs[ aeQuadVertCount ];
extern const aeQuadIndex aeQuadIndices[ aeQuadIndexCount ];

const Vec3 aeQuadVertPos[ aeQuadVertCount ] = {
  Vec3( -0.5f, -0.5f, 0.0f ),
  Vec3( 0.5f, -0.5f, 0.0f ),
  Vec3( 0.5f, 0.5f, 0.0f ),
  Vec3( -0.5f, 0.5f, 0.0f )
};

const Vec2 aeQuadVertUvs[ aeQuadVertCount ] = {
  Vec2( 0.0f, 0.0f ),
  Vec2( 1.0f, 0.0f ),
  Vec2( 1.0f, 1.0f ),
  Vec2( 0.0f, 1.0f )
};

const aeQuadIndex aeQuadIndices[ aeQuadIndexCount ] = {
  3, 0, 1,
  3, 1, 2
};

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
  m_hash.HashString( name );
  m_hash.HashFloat( value );
}

void UniformList::Set( const char* name, Vec2 value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 2;
  uniform.value.data[ 0 ] = value.x;
  uniform.value.data[ 1 ] = value.y;
  m_hash.HashString( name );
  m_hash.HashFloatArray( value.data );
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
  m_hash.HashString( name );
  m_hash.HashFloatArray( value.data );
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
  m_hash.HashString( name );
  m_hash.HashFloatArray( value.data );
}

void UniformList::Set( const char* name, const Matrix4& value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 16;
  uniform.value = value;
  m_hash.HashString( name );
  m_hash.HashFloatArray( value.data );
}

void UniformList::Set( const char* name, const Texture* tex )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.sampler = tex->GetTexture();
  uniform.target = tex->GetTarget();
  m_hash.HashString( name );
  m_hash.HashBasicType( tex->GetTexture() );
  m_hash.HashBasicType( tex->GetTarget() );
}

const UniformList::Value* UniformList::Get( const char* name ) const
{
  return m_uniforms.TryGet( name );
}

//------------------------------------------------------------------------------
// ae::Shader member functions
//------------------------------------------------------------------------------
ae::Hash s_shaderHash;
ae::Hash s_uniformHash;

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
  Terminate();
}

void Shader::Initialize( const char* vertexStr, const char* fragStr, const char* const* defines, int32_t defineCount )
{
  Terminate();
  AE_ASSERT( !m_program );
  
  m_program = glCreateProgram();

  m_vertexShader = m_LoadShader( vertexStr, Type::Vertex, defines, defineCount );
  m_fragmentShader = m_LoadShader( fragStr, Type::Fragment, defines, defineCount );

  if ( !m_vertexShader )
  {
    AE_ERR( "Failed to load vertex shader! #", vertexStr );
  }
  if ( !m_fragmentShader )
  {
    AE_ERR( "Failed to load fragment shader! #", fragStr );
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
    Terminate();
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

void Shader::Terminate()
{
  AE_CHECK_GL_ERROR();

  m_attributes.Clear();
  m_uniforms.Clear();

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
  ae::Hash shaderHash;
  shaderHash.HashBasicType( this );
  shaderHash.HashBasicType( m_blending );
  shaderHash.HashBasicType( m_blendingPremul );
  shaderHash.HashBasicType( m_depthWrite );
  shaderHash.HashBasicType( m_depthTest );
  shaderHash.HashBasicType( m_culling );
  shaderHash.HashBasicType( m_wireframe );
  bool shaderDirty = ( s_shaderHash != shaderHash );
  if ( shaderDirty )
  {
    s_shaderHash = shaderHash;
    
    AE_CHECK_GL_ERROR();

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
      // This is really context state shadow, and that should be able to override
      // so reverseZ for example can be set without the shader knowing about that.
      glDepthFunc( ReverseZ ? GL_GEQUAL : GL_LEQUAL );
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
#if _AE_IOS_ || _AE_EMSCRIPTEN_
    AE_ASSERT_MSG( !m_wireframe, "Wireframe mode not supported on this platform" );
#else
    glPolygonMode( GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL );
#endif

    // Now setup the shader
    glUseProgram( m_program );
  }
  
  // Always update uniforms after a shader change
  if ( !shaderDirty && s_uniformHash == uniforms.GetHash() )
  {
    return;
  }
  s_uniformHash = uniforms.GetHash();
  
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
      glUniformMatrix4fv( uniformVar->location, 1, GL_FALSE, uniformValue->value.data );
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
  ae::Str32 glVersionStr = "#version ";
#if _AE_IOS_ || _AE_EMSCRIPTEN_
  glVersionStr += ae::Str16::Format( "##0 es", ae::GLMajorVersion, ae::GLMinorVersion );
#else
  glVersionStr += ae::Str16::Format( "##0 core", ae::GLMajorVersion, ae::GLMinorVersion );
#endif
  glVersionStr += "\n";
  if ( glVersionStr.Length() )
  {
    shaderSource[ sourceCount++ ] = glVersionStr.c_str();
  }

  // Precision
#if _AE_IOS_ || _AE_EMSCRIPTEN_
  shaderSource[ sourceCount++ ] = "precision highp float;\n";
#else
  // No default precision specified
#endif

  // Input/output
// #if _AE_EMSCRIPTEN_
//   shaderSource[ sourceCount++ ] = "#define AE_COLOR gl_FragColor\n";
//   shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture2d\n";
//   shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
//   if ( type == Type::Vertex )
//   {
//     shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP attribute highp\n";
//     shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP varying highp\n";
//   }
//   else if ( type == Type::Fragment )
//   {
//     shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP varying highp\n";
//     shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
//   }
// #else
  shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM uniform\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform\n";
  shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP in\n";
  shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP out\n";
  if ( type == Type::Fragment )
  {
    shaderSource[ sourceCount++ ] = "out vec4 AE_COLOR;\n";
  }
// #endif

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

    const char* typeStr = ( type == Type::Vertex ? "vertex" : "fragment" );
    if ( logLength > 0 )
    {
      unsigned char* log = new unsigned char[ logLength ];
      glGetShaderInfoLog( shader, logLength, NULL, (GLchar*)log );
      AE_ERR( "Error compiling # shader #", typeStr, log );
      delete[] log;
    }
    else
    {
      AE_ERR( "Error compiling # shader: unknown issue", typeStr );
    }

    AE_CHECK_GL_ERROR();
    return 0;
  }

  AE_CHECK_GL_ERROR();
  return shader;
}

//------------------------------------------------------------------------------
// ae::VertexData member functions
//------------------------------------------------------------------------------
VertexData::~VertexData()
{
  Terminate();
}

void VertexData::Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, VertexData::Primitive primitive, VertexData::Usage vertexUsage, VertexData::Usage indexUsage )
{
  Terminate();

  AE_ASSERT( m_vertexSize == 0 );
  AE_ASSERT( vertexSize );
  AE_ASSERT( m_indexSize == 0 );
  AE_ASSERT( indexSize == sizeof(uint8_t) || indexSize == sizeof(uint16_t) || indexSize == sizeof(uint32_t) );

  m_maxVertexCount = maxVertexCount;
  m_maxIndexCount = maxIndexCount;
  m_primitive = primitive;
  m_vertexUsage = vertexUsage;
  m_indexUsage = indexUsage;
  m_vertexSize = vertexSize;
  m_indexSize = indexSize;
  
  glGenVertexArrays( 1, &m_array );
  glBindVertexArray( m_array );
}

void VertexData::Terminate()
{
  if ( m_vertexReadable )
  {
    ae::Delete( (uint8_t*)m_vertexReadable );
  }
  if ( m_indexReadable )
  {
    ae::Delete( (uint8_t*)m_indexReadable );
  }
  
  if ( m_array )
  {
    glDeleteVertexArrays( 1, &m_array );
  }
  if ( m_vertices != ~0 )
  {
    glDeleteBuffers( 1, &m_vertices );
  }
  if ( m_indices != ~0 )
  {
    glDeleteBuffers( 1, &m_indices );
  }
  
  m_attributes.Clear();
  
  // Params
  m_vertexSize = 0;
  m_indexSize = 0;
  m_primitive = (VertexData::Primitive)-1;
  m_vertexUsage = (VertexData::Usage)-1;
  m_indexUsage = (VertexData::Usage)-1;
  m_maxVertexCount = 0;
  m_maxIndexCount = 0;
  // Dynamic state
  m_vertexCount = 0;
  m_indexCount = 0;
  m_vertexReadable = nullptr;
  m_indexReadable = nullptr;
  m_vertexDirty = false;
  m_indexDirty = false;
  // System resources
  m_array = 0;
  m_vertices = ~0;
  m_indices = ~0;
}

void VertexData::AddAttribute( const char *name, uint32_t componentCount, VertexData::Type type, uint32_t offset )
{
  AE_ASSERT( m_vertices == ~0 && m_indices == ~0 );
  
  Attribute* attribute = &m_attributes.Append( Attribute() );
  
  size_t length = strlen( name );
  AE_ASSERT( length < _kMaxShaderAttributeNameLength );
  strcpy( attribute->name, name );
  attribute->componentCount = componentCount;
  attribute->type = VertexDataTypeToGL( type );
  attribute->offset = offset;
  attribute->normalized =
    type == VertexData::Type::NormalizedUInt8 ||
    type == VertexData::Type::NormalizedUInt16 ||
    type == VertexData::Type::NormalizedUInt32;
}

void VertexData::m_SetVertices( const void* vertices, uint32_t count )
{
  AE_ASSERT( m_vertexSize );
  AE_ASSERT_MSG( count <= m_maxVertexCount, "# #", count, m_maxVertexCount );

  if ( m_indices != ~0 )
  {
    AE_ASSERT( m_indexSize != 0 );
  }
  if ( m_indexSize )
  {
    AE_ASSERT_MSG( count <= (uint64_t)1 << ( m_indexSize * 8 ), "Vertex count (#) too high for index of size #", count, m_indexSize );
  }
  
  m_vertexDirty = false;
  
  if( m_vertexUsage == Usage::Static )
  {
    AE_ASSERT( count );
    AE_ASSERT( m_vertices == ~0 );

    glGenBuffers( 1, &m_vertices );
    glBindVertexArray( m_array );
    glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
    glBufferData( GL_ARRAY_BUFFER, count * m_vertexSize, vertices, GL_STATIC_DRAW );
    return;
  }
  
  if( m_vertexUsage == Usage::Dynamic )
  {
    if ( !m_vertexCount )
    {
      return;
    }
    
    if( m_vertices == ~0 )
    {
      glGenBuffers( 1, &m_vertices );
      glBindVertexArray( m_array );
      glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
      glBufferData( GL_ARRAY_BUFFER, m_vertexSize * m_maxVertexCount, nullptr, GL_DYNAMIC_DRAW );
    }
    
    glBindVertexArray( m_array );
    glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
    glBufferSubData( GL_ARRAY_BUFFER, 0, count * m_vertexSize, vertices );
    return;
  }
  
  AE_FAIL();
}

void VertexData::m_SetIndices( const void* indices, uint32_t count )
{
  AE_ASSERT( m_indexSize );
  AE_ASSERT( count % 3 == 0 );
  AE_ASSERT( count <= m_maxIndexCount );
  
  m_indexDirty = false;
  
  if( m_indexUsage == Usage::Static )
  {
    AE_ASSERT( count );
    AE_ASSERT( m_indices == ~0 );

    glGenBuffers( 1, &m_indices );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indexCount * m_indexSize, indices, GL_STATIC_DRAW );
    return;
  }
  
  if( m_indexUsage == Usage::Dynamic )
  {
    if ( !m_indexCount )
    {
      return;
    }
    
    if( m_indices == ~0 )
    {
      glGenBuffers( 1, &m_indices );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indexSize * m_maxIndexCount, nullptr, GL_DYNAMIC_DRAW );
    }
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, m_indexCount * m_indexSize, indices );
    return;
  }
  
  AE_FAIL();
}

void VertexData::SetVertices( const void* vertices, uint32_t count )
{
  // State validation
  AE_ASSERT( m_vertexSize );
  if ( !m_vertexCount && !count )
  {
    return;
  }
  
  // Set vertices
  if ( count )
  {
    if ( m_vertexUsage == VertexData::Usage::Static )
    {
      AE_ASSERT_MSG( !m_vertexCount, "Cannot re-set vertices, buffer was created as static!" );
    }
    
    if ( !m_vertexReadable )
    {
      // @TODO: Realloc or use array
      m_vertexReadable = ae::NewArray< uint8_t >( AE_ALLOC_TAG_RENDER, m_maxVertexCount * m_vertexSize );
    }
    memcpy( m_vertexReadable, vertices, count * m_vertexSize );
  }
  m_vertexCount = count;
  m_vertexDirty = true;
}

void VertexData::SetIndices( const void* indices, uint32_t count )
{
  // State validation
  AE_ASSERT( m_indexSize );
  if ( !m_indexCount && !count )
  {
    return;
  }

  // Validate indices
  if ( count && _AE_DEBUG_ )
  {
    int32_t badIndex = -1;
    
    if ( m_indexSize == 1 )
    {
      uint8_t* indicesCheck = (uint8_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        if ( indicesCheck[ i ] >= m_maxVertexCount )
        {
          badIndex = indicesCheck[ i ];
          break;
        }
      }
    }
    else if ( m_indexSize == 2 )
    {
      uint16_t* indicesCheck = (uint16_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        if ( indicesCheck[ i ] >= m_maxVertexCount )
        {
          badIndex = indicesCheck[ i ];
          break;
        }
      }
    }
    else if ( m_indexSize == 4 )
    {
      uint32_t* indicesCheck = (uint32_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        if ( indicesCheck[ i ] >= m_maxVertexCount )
        {
          badIndex = indicesCheck[ i ];
          break;
        }
      }
    }

    if ( badIndex >= 0 )
    {
      AE_FAIL_MSG( "Out of range index detected #", badIndex );
    }
  }
  
  // Set indices
  if ( count )
  {
    if ( m_indexUsage == VertexData::Usage::Static )
    {
      AE_ASSERT_MSG( !m_indexCount, "Cannot re-set indices, buffer was created as static!" );
    }
    
    if ( !m_indexReadable )
    {
      // @TODO: Realloc or use array
      m_indexReadable = ae::NewArray< uint8_t >( AE_ALLOC_TAG_RENDER, m_maxIndexCount * m_indexSize );
    }
    memcpy( m_indexReadable, indices, count * m_indexSize );
  }
  m_indexCount = count;
  m_indexDirty = true;
}

void VertexData::AppendVertices( const void* vertices, uint32_t count )
{
  // State validation
  AE_ASSERT( m_vertexSize );
  if ( m_vertexUsage == VertexData::Usage::Static )
  {
    AE_ASSERT_MSG( !m_vertexCount, "Cannot re-set vertices, buffer was created as static!" );
  }
  AE_ASSERT_MSG( m_vertexCount + count <= m_maxVertexCount, "Vertex limit exceeded #/#", m_vertexCount + count, m_maxVertexCount );
  
  if ( !count )
  {
    return;
  }

  if ( !m_vertexReadable )
  {
    // @TODO: Realloc or use array
    m_vertexReadable = ae::NewArray< uint8_t >( AE_ALLOC_TAG_RENDER, m_maxVertexCount * m_vertexSize );
  }
  
  // Append vertices
  memcpy( (uint8_t*)m_vertexReadable + ( m_vertexCount * m_vertexSize ), vertices, count * m_vertexSize );

  m_vertexCount += count;
  m_vertexDirty = true;
}

void VertexData::AppendIndices( const void* indices, uint32_t count, uint32_t _indexOffset )
{
  // State validation
  AE_ASSERT( m_indexSize );
  if ( m_indexUsage == VertexData::Usage::Static )
  {
    AE_ASSERT_MSG( !m_indexCount, "Cannot re-set indices, buffer was created as static!" );
  }
  AE_ASSERT_MSG( m_indexCount + count <= m_maxIndexCount, "Index limit exceeded #/#", m_indexCount + count, m_maxIndexCount );
  
  if ( !count )
  {
    return;
  }

  if ( !m_indexReadable )
  {
    // @TODO: Realloc or use array
    m_indexReadable = ae::NewArray< uint8_t >( AE_ALLOC_TAG_RENDER, m_maxIndexCount * m_indexSize );
  }
  
  // Append indices
  switch ( m_indexSize )
  {
    case 1:
    {
      uint8_t indexOffset = _indexOffset;
      uint8_t* target = (uint8_t*)m_indexReadable + m_indexCount;
      uint8_t* source = (uint8_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        target[ i ] = indexOffset + source[ i ];
      }
      break;
    }
    case 2:
    {
      uint16_t indexOffset = _indexOffset;
      uint16_t* target = (uint16_t*)m_indexReadable + m_indexCount;
      uint16_t* source = (uint16_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        target[ i ] = indexOffset + source[ i ];
      }
      break;
    }
    case 4:
    {
      uint32_t indexOffset = _indexOffset;
      uint32_t* target = (uint32_t*)m_indexReadable + m_indexCount;
      uint32_t* source = (uint32_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        target[ i ] = indexOffset + source[ i ];
      }
      break;
    }
    default:
      AE_FAIL();
  }
  
  m_indexCount += count;
  m_indexDirty = true;
}

void VertexData::ClearVertices()
{
  if ( m_vertexCount && m_vertexUsage == VertexData::Usage::Dynamic ) // @TODO: Should this be an assert?
  {
    m_vertexCount = 0;
    m_vertexDirty = true;
  }
}

void VertexData::ClearIndices()
{
  if ( m_indexCount && m_indexUsage == VertexData::Usage::Dynamic ) // @TODO: Should this be an assert?
  {
    m_indexCount = 0;
    m_indexDirty = true;
  }
}

const void* VertexData::GetVertices() const
{
  AE_ASSERT( m_vertexReadable != nullptr );
  return m_vertexReadable;
}

const void* VertexData::GetIndices() const
{
  AE_ASSERT( m_indexReadable != nullptr );
  return m_indexReadable;
}

void VertexData::Upload()
{
  if ( m_vertexDirty )
  {
    m_SetVertices( m_vertexReadable, m_vertexCount );
  }
  if ( m_indexDirty )
  {
    m_SetIndices( m_indexReadable, m_indexCount );
  }
}

void VertexData::Render( const Shader* shader, const UniformList& uniforms ) const
{
  Render( shader, uniforms, 0, 0 ); // Draw all
}

void VertexData::Render( const Shader* shader, const UniformList& uniforms, uint32_t primitiveStart, uint32_t primitiveCount ) const
{
  AE_ASSERT( shader );
  AE_ASSERT_MSG( m_vertexSize && m_indexSize, "Must call Initialize() before Render()" );
  AE_ASSERT_MSG( !m_vertexDirty && !m_indexDirty, "Must call Upload() after modifying vertices or indices before calling Render()" );
  
  if ( m_vertices == ~0 || !m_vertexCount || ( m_indices != ~0 && !m_indexCount ) )
  {
    return;
  }

  shader->Activate( uniforms );

  glBindVertexArray( m_array );
  AE_CHECK_GL_ERROR();

  glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
  AE_CHECK_GL_ERROR();

  if ( m_indexCount && m_primitive != Primitive::Point )
  {
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    AE_CHECK_GL_ERROR();
  }

  for ( uint32_t i = 0; i < shader->GetAttributeCount(); i++ )
  {
    const Shader::Attribute* shaderAttribute = shader->GetAttributeByIndex( i );
    const Attribute* vertexAttribute = m_GetAttributeByName( shaderAttribute->name );

    AE_ASSERT_MSG( vertexAttribute, "No vertex attribute named '#'", shaderAttribute->name );
    // @TODO: Verify attribute type and size match

    GLint location = shaderAttribute->location;
    AE_ASSERT( location != -1 );
    glEnableVertexAttribArray( location );
    AE_CHECK_GL_ERROR();

    uint32_t componentCount = vertexAttribute->componentCount;
    uint64_t attribOffset = vertexAttribute->offset;
    glVertexAttribPointer( location, componentCount, vertexAttribute->type, vertexAttribute->normalized, m_vertexSize, (void*)attribOffset );
    AE_CHECK_GL_ERROR();
  }

  GLenum mode = 0;
  uint32_t primitiveSize = 0;
  const char* primitiveTypeName = "";
  if ( m_primitive == Primitive::Triangle ) { mode = GL_TRIANGLES; primitiveSize = 3; primitiveTypeName = "Triangle"; }
  else if ( m_primitive == Primitive::Line ) { mode = GL_LINES; primitiveSize = 2; primitiveTypeName = "Line"; }
  else if ( m_primitive == Primitive::Point ) { mode = GL_POINTS; primitiveSize = 1; primitiveTypeName = "Point"; }
  else { AE_FAIL(); return; }
  
  if ( m_indexCount && mode != GL_POINTS )
  {
    AE_ASSERT( primitiveStart + primitiveCount <= m_indexCount / primitiveSize );
    int64_t start = primitiveStart * primitiveSize * m_indexSize; // Byte offset into index buffer
    int32_t count = primitiveCount ? primitiveCount * primitiveSize : m_indexCount; // Number of indices to render
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    AE_CHECK_GL_ERROR();
    
    GLenum type = 0;
    if ( m_indexSize == sizeof(uint8_t) ) { type = GL_UNSIGNED_BYTE; }
    else if ( m_indexSize == sizeof(uint16_t) ) { type = GL_UNSIGNED_SHORT; }
    else if ( m_indexSize == sizeof(uint32_t) ) { type = GL_UNSIGNED_INT; }
    glDrawElements( mode, count, type, (void*)start );
    AE_CHECK_GL_ERROR();
  }
  else
  {
    AE_ASSERT( ( primitiveStart + primitiveCount ) * primitiveSize <= m_vertexCount );
    GLint start = primitiveStart * primitiveSize;
    GLsizei count = primitiveCount ? primitiveCount * primitiveSize : m_vertexCount;
    AE_ASSERT_MSG( count % primitiveSize == 0, "Vertex count must be a multiple of # when rendering #s without indices", primitiveSize, primitiveTypeName );
    glDrawArrays( mode, start, count );
    AE_CHECK_GL_ERROR();
  }
}

const VertexData::Attribute* VertexData::m_GetAttributeByName( const char* name ) const
{
  for ( uint32_t i = 0; i < m_attributes.Length(); i++ )
  {
    if ( strcmp( m_attributes[ i ].name, name ) == 0 )
    {
      return &m_attributes[ i ];
    }
  }

  return nullptr;
}

//------------------------------------------------------------------------------
// ae::Texture member functions
//------------------------------------------------------------------------------
Texture::~Texture()
{
  // @NOTE: Only ae::Texture should call it's virtual Destroy() so it only runs once
  Terminate();
}

void Texture::Initialize( uint32_t target )
{
  // @NOTE: To avoid undoing any initialization logic only ae::Texture should
  //        call Destroy() on initialize, and inherited Initialize()'s should
  //        always call Base::Initialize() before any other logic.
  Terminate();

  m_target = target;

  glGenTextures( 1, &m_texture );
  AE_ASSERT( m_texture );
}

void Texture::Terminate()
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
    case Format::Depth16:
      glInternalFormat = GL_DEPTH_COMPONENT16;
      glFormat = GL_DEPTH_COMPONENT;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
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

void Texture2D::Terminate()
{
  m_width = 0;
  m_height = 0;
  m_hasAlpha = false;

  Texture::Terminate();
}

//------------------------------------------------------------------------------
// ae::RenderTarget member functions
//------------------------------------------------------------------------------
RenderTarget::~RenderTarget()
{
  Terminate();
}

void RenderTarget::Initialize( uint32_t width, uint32_t height )
{
  Terminate();

  AE_ASSERT( m_fbo == 0 );

  if ( width * height == 0 )
  {
    m_width = 0;
    m_height = 0;
    return;
  }

  m_width = width;
  m_height = height;

  glGenFramebuffers( 1, &m_fbo );
  AE_CHECK_GL_ERROR();
  AE_ASSERT( m_fbo );
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  AE_CHECK_GL_ERROR();

  Vertex quadVerts[] =
  {
    { aeQuadVertPos[ 0 ], aeQuadVertUvs[ 0 ] },
    { aeQuadVertPos[ 1 ], aeQuadVertUvs[ 1 ] },
    { aeQuadVertPos[ 2 ], aeQuadVertUvs[ 2 ] },
    { aeQuadVertPos[ 3 ], aeQuadVertUvs[ 3 ] }
  };
  AE_STATIC_ASSERT( countof( quadVerts ) == aeQuadVertCount );
  m_quad.Initialize( sizeof( Vertex ), sizeof( aeQuadIndex ), aeQuadVertCount, aeQuadIndexCount, VertexData::Primitive::Triangle, VertexData::Usage::Static, VertexData::Usage::Static );
  m_quad.AddAttribute( "a_position", 3, VertexData::Type::Float, offsetof( Vertex, pos ) );
  m_quad.AddAttribute( "a_uv", 2, VertexData::Type::Float, offsetof( Vertex, uv ) );
  m_quad.SetVertices( quadVerts, aeQuadVertCount );
  m_quad.SetIndices( aeQuadIndices, aeQuadIndexCount );
  m_quad.Upload();
  AE_CHECK_GL_ERROR();

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
      vec4 color = AE_TEXTURE2D( u_tex, v_uv );"
#if _AE_EMSCRIPTEN_
      // It seems like WebGL requires a manual conversion to sRGB, since there is no way to specify a framebuffer format
      "AE_COLOR.rgb = pow( color.rgb, vec3( 1.0/2.2 ) );"
      "AE_COLOR.a = color.a;"
#else
      "AE_COLOR = color;"
#endif
    "}";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );

  AE_CHECK_GL_ERROR();
}

void RenderTarget::Terminate()
{
  m_shader.Terminate();
  m_quad.Terminate();

  for ( uint32_t i = 0; i < m_targets.Length(); i++ )
  {
    m_targets[ i ]->Terminate();
    ae::Delete( m_targets[ i ] );
  }
  m_targets.Clear();

  m_depth.Terminate();

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
  if ( m_width * m_height == 0 )
  {
    return;
  }

#if _AE_EMSCRIPTEN_
  Texture::Format format = Texture::Format::RGBA8;
  Texture::Type type = Texture::Type::Uint8;
#else
  Texture::Format format = Texture::Format::RGBA16F;
  Texture::Type type = Texture::Type::HalfFloat;
#endif
  Texture2D* tex = ae::New< Texture2D >( AE_ALLOC_TAG_RENDER );
  tex->Initialize( nullptr, m_width, m_height, format, type, filter, wrap, false );

  GLenum attachement = GL_COLOR_ATTACHMENT0 + m_targets.Length();
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  glFramebufferTexture2D( GL_FRAMEBUFFER, attachement, tex->GetTarget(), tex->GetTexture(), 0 );

  m_targets.Append( tex );

  AE_CHECK_GL_ERROR();
}

void RenderTarget::AddDepth( Texture::Filter filter, Texture::Wrap wrap )
{
  AE_ASSERT_MSG( m_depth.GetTexture() == 0, "Render target already has a depth texture" );
  if ( m_width * m_height == 0 )
  {
    return;
  }

#if _AE_EMSCRIPTEN_
  Texture::Format format = Texture::Format::Depth16;
  Texture::Type type = Texture::Type::Uint16;
#else
  Texture::Format format = Texture::Format::Depth32F;
  Texture::Type type = Texture::Type::Float;
#endif
  m_depth.Initialize( nullptr, m_width, m_height, format, type, filter, wrap, false );
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
  glClearDepth( ReverseZ ? 0.0f : 1.0f );

  glDepthMask( GL_TRUE );
  glDisable( GL_DEPTH_TEST );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  AE_CHECK_GL_ERROR();
}

void RenderTarget::Render( const Shader* shader, const UniformList& uniforms )
{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
  m_quad.Render( shader, uniforms );
}

void RenderTarget::Render2D( uint32_t textureIndex, Rect ndc, float z )
{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );

  UniformList uniforms;
  uniforms.Set( "u_localToNdc", RenderTarget::GetQuadToNDCTransform( ndc, z ) );
  uniforms.Set( "u_tex", GetTexture( textureIndex ) );
  m_quad.Render( &m_shader, uniforms );
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
  Matrix4 windowToNDC = Matrix4::Translation( Vec3( -1.0f, -1.0f, 0.0f ) );
  windowToNDC *= Matrix4::Scaling( Vec3( 2.0f / otherPixelWidth, 2.0f / otherPixelHeight, 1.0f ) );

  Matrix4 ndcToQuad = RenderTarget::GetQuadToNDCTransform( ndc, 0.0f );
  ndcToQuad.SetInverse();

  Matrix4 quadToRender = Matrix4::Scaling( Vec3( m_width, m_height, 1.0f ) );
  quadToRender *= Matrix4::Translation( Vec3( 0.5f, 0.5f, 0.0f ) );

  return ( quadToRender * ndcToQuad * windowToNDC );
}

Rect RenderTarget::GetNDCFillRectForTarget( uint32_t otherWidth, uint32_t otherHeight ) const
{
  float canvasAspect = m_width / (float)m_height;
  float targetAspect = otherWidth / (float)otherHeight;
  if ( canvasAspect >= targetAspect )
  {
    // Fit width
    float height = targetAspect / canvasAspect;
    return Rect( -1.0f, -height, 2.0f, height * 2.0f );
  }
  else
  {
    // Fit height
    float width = canvasAspect / targetAspect;
    return Rect( -width, -1.0f, width * 2.0f, 2.0f );
  }
}

Matrix4 RenderTarget::GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const
{
  Matrix4 canvasToNdc = Matrix4::Translation( Vec3( -1.0f, -1.0f, 0.0f ) ) * Matrix4::Scaling( Vec3( 2.0f / GetWidth(), 2.0f / GetHeight(), 1.0f ) );
  return ( worldToNdc.GetInverse() * canvasToNdc * otherTargetToLocal );
}

Matrix4 RenderTarget::GetQuadToNDCTransform( Rect ndc, float z )
{
  Matrix4 localToNdc = Matrix4::Translation( Vec3( ndc.x, ndc.y, z ) );
  localToNdc *= Matrix4::Scaling( Vec3( ndc.w, ndc.h, 1.0f ) );
  localToNdc *= Matrix4::Translation( Vec3( 0.5f, 0.5f, 0.0f ) );
  return localToNdc;
}

//------------------------------------------------------------------------------
// GraphicsDevice member functions
//------------------------------------------------------------------------------
GraphicsDevice::~GraphicsDevice()
{
  Terminate();
}

#if _AE_WINDOWS_
#define LOAD_OPENGL_FN( _glfn )\
_glfn = (decltype(_glfn))wglGetProcAddress( #_glfn );\
AE_ASSERT_MSG( _glfn, "Failed to load OpenGL function '" #_glfn "'" );
#endif

void GraphicsDevice::Initialize( class Window* window )
{
  AE_ASSERT_MSG( !m_context, "GraphicsDevice already initialized" );

  AE_ASSERT( window );
  m_window = window;
  window->graphicsDevice = this;

#if !_AE_EMSCRIPTEN_
  AE_ASSERT_MSG( window->window, "Window must be initialized prior to GraphicsDevice initialization." );
#endif

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
#elif _AE_APPLE_
  m_context = ((NSOpenGLView*)((NSWindow*)window->window).contentView).openGLContext;
#elif _AE_EMSCRIPTEN_
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes( &attrs );
  attrs.alpha = 0;
  attrs.majorVersion = ae::GLMajorVersion;
  attrs.minorVersion = ae::GLMinorVersion;
  m_context = emscripten_webgl_create_context( "canvas", &attrs );
  AE_ASSERT( m_context );
  emscripten_webgl_make_context_current( m_context );
#endif
  
  AE_CHECK_GL_ERROR();

#if _AE_WINDOWS_
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
  LOAD_OPENGL_FN( glTextureBarrierNV );
  // Vertex functions
  LOAD_OPENGL_FN( glGenVertexArrays );
  LOAD_OPENGL_FN( glBindVertexArray );
  LOAD_OPENGL_FN( glDeleteVertexArrays );
  LOAD_OPENGL_FN( glDeleteBuffers );
  LOAD_OPENGL_FN( glBindBuffer );
  LOAD_OPENGL_FN( glGenBuffers );
  LOAD_OPENGL_FN( glBufferData );
  LOAD_OPENGL_FN( glBufferSubData );
  LOAD_OPENGL_FN( glEnableVertexAttribArray );
  LOAD_OPENGL_FN( glVertexAttribPointer );
  // Debug functions
  LOAD_OPENGL_FN( glDebugMessageCallback );
  AE_CHECK_GL_ERROR();
#endif

#if AE_GL_DEBUG_MODE
  glDebugMessageCallback( ae::OpenGLDebugCallback, nullptr );
#endif

  glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFbo );
  AE_CHECK_GL_ERROR();

  float scaleFactor = m_window->GetScaleFactor();
  int32_t contentWidth = m_window->GetWidth() * scaleFactor;
  int32_t contentHeight = m_window->GetHeight() * scaleFactor;
  m_HandleResize( contentWidth, contentHeight );
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

  float scaleFactor = m_window->GetScaleFactor();
  int32_t contentWidth = m_window->GetWidth() * scaleFactor;
  int32_t contentHeight = m_window->GetHeight() * scaleFactor;
  if ( contentWidth != m_canvas.GetWidth() || contentHeight != m_canvas.GetHeight() )
  {
#if _AE_EMSCRIPTEN_
    emscripten_set_canvas_element_size( "canvas", contentWidth, contentHeight );
#else
     m_HandleResize( contentWidth, contentHeight );
#endif
  }

  if ( m_canvas.GetWidth() * m_canvas.GetHeight() == 0 )
  {
    return;
  }

  m_canvas.Activate();
#if !_AE_IOS_ && !_AE_EMSCRIPTEN_
  // This is automatically enabled on opengl es3 and can't be turned off
  glEnable( GL_FRAMEBUFFER_SRGB );
#endif
}

void GraphicsDevice::Clear( Color color )
{
  if ( m_canvas.GetWidth() * m_canvas.GetHeight() == 0 )
  {
    return;
  }
  Activate();
  m_canvas.Clear( color );
}

void GraphicsDevice::Present()
{
  if ( m_canvas.GetWidth() * m_canvas.GetHeight() == 0 )
  {
    return;
  }

  AE_ASSERT( m_context );
  AE_CHECK_GL_ERROR();

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_defaultFbo );
  glViewport( 0, 0, m_canvas.GetWidth(), m_canvas.GetHeight() );

  // Clear window target in case canvas doesn't fit exactly
  glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
  glClearDepth( 1.0f );

  glDepthMask( GL_TRUE );

  glDisable( GL_DEPTH_TEST );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  AE_CHECK_GL_ERROR();

  m_canvas.Render2D( 0, Rect( Vec2( -1.0f ), Vec2( 1.0f ) ), 0.5f );
  
  glFlush(); // @TODO: Needed on osx with single buffering. Use double buffering or leave this to be safe?

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
  if ( m_canvas.GetWidth() * m_canvas.GetHeight() == 0 )
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
  // only GL has texture barrier for reading from previously written textures
  // There are less draconian ways in desktop ES, and nothing in WebGL.
#if _AE_WINDOWS_ || _AE_OSX_
  glTextureBarrierNV();
#endif
}

void GraphicsDevice::m_HandleResize( uint32_t width, uint32_t height )
{
  // @TODO: Also resize actual canvas element with emscripten?
  // emscripten_set_canvas_element_size( "canvas", m_window->GetWidth(), m_window->GetHeight() );
  // emscripten_set_canvas_size( m_window->GetWidth(), m_window->GetHeight() );
  // @TODO: Allow user to pass in a canvas scale factor / aspect ratio parameter
  m_canvas.Initialize( width, height );
  m_canvas.AddTexture( Texture::Filter::Nearest, Texture::Wrap::Clamp );
  m_canvas.AddDepth( Texture::Filter::Nearest, Texture::Wrap::Clamp );
  
  // Force refresh uniforms for new canvas
  s_shaderHash = ae::Hash();
  s_uniformHash = ae::Hash();
}

//------------------------------------------------------------------------------
// ae::DebugLines member functions
//------------------------------------------------------------------------------
const uint32_t kDebugVertexCountPerObject = 32;

void DebugLines::Initialize( uint32_t maxObjects )
{
  m_objs = ae::Array< DebugObject >( AE_ALLOC_TAG_RENDER, maxObjects );

  // @HACK: Should handle vert count in a safer way
  m_vertexData.Initialize( sizeof(DebugVertex), sizeof(uint16_t), m_objs.Size() * kDebugVertexCountPerObject, 0, VertexData::Primitive::Line, VertexData::Usage::Dynamic, VertexData::Usage::Static );
  m_vertexData.AddAttribute( "a_position", 3, VertexData::Type::Float, offsetof(DebugVertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, VertexData::Type::Float, offsetof(DebugVertex, color) );

  // Load shader
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_worldToNdc;\
    AE_UNIFORM float u_saturation;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      float bw = (min(a_color.r, min(a_color.g, a_color.b)) + max(a_color.r, max(a_color.g, a_color.b))) * 0.5;\
      v_color = vec4(mix(vec3(bw), a_color.rgb, u_saturation), 1.0);\
      gl_Position = u_worldToNdc * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      AE_COLOR = v_color;\
    }";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );
  m_shader.SetBlending( true );
  m_shader.SetDepthTest( true );
}

void DebugLines::Terminate()
{
  m_shader.Terminate();
  m_vertexData.Terminate();
}

void DebugLines::Render( const Matrix4& worldToNdc )
{
  if ( !m_objs.Length() )
  {
    return;
  }

  const uint16_t kQuadIndices[] = {
    3, 1, 0,
    3, 1, 2
  };

  m_verts.Clear();
  m_verts.Reserve( m_objs.Size() * kDebugVertexCountPerObject );

  for ( uint32_t i = 0; i < m_objs.Length(); i++ )
  {
    DebugObject obj = m_objs[ i ];
    if ( obj.type == DebugType::Rect )
    {
      Vec3 halfSize = obj.size * 0.5f;

      DebugVertex verts[ 4 ];
      
      verts[ 0 ].pos = obj.pos + obj.rotation.Rotate( Vec3( -halfSize.x, 0.0f, -halfSize.y ) ); // Bottom Left
      verts[ 1 ].pos = obj.pos + obj.rotation.Rotate( Vec3( halfSize.x, 0.0f, -halfSize.y ) ); // Bottom Right
      verts[ 2 ].pos = obj.pos + obj.rotation.Rotate( Vec3( halfSize.x, 0.0f, halfSize.y ) ); // Top Right
      verts[ 3 ].pos = obj.pos + obj.rotation.Rotate( Vec3( -halfSize.x, 0.0f, halfSize.y ) ); // Top Left

      verts[ 0 ].color = obj.color;
      verts[ 1 ].color = obj.color;
      verts[ 2 ].color = obj.color;
      verts[ 3 ].color = obj.color;

      m_verts.Append( verts[ 0 ] );
      m_verts.Append( verts[ 1 ] );
      m_verts.Append( verts[ 1 ] );
      m_verts.Append( verts[ 2 ] );
      m_verts.Append( verts[ 2 ] );
      m_verts.Append( verts[ 3 ] );
      m_verts.Append( verts[ 3 ] );
      m_verts.Append( verts[ 0 ] );
    }
    else if ( obj.type == DebugType::Circle )
    {
      float angleInc = ae::PI * 2.0f / obj.pointCount;
      for ( uint32_t i = 0; i < obj.pointCount; i++ )
      {
        float angle0 = angleInc * i;
        float angle1 = angleInc * ( i + 1 );

        DebugVertex verts[ 2 ];

        verts[ 0 ].pos = Vec3( cosf( angle0 ) * obj.radius, 0.0f, sinf( angle0 ) * obj.radius );
        verts[ 1 ].pos = Vec3( cosf( angle1 ) * obj.radius, 0.0f, sinf( angle1 ) * obj.radius );
        verts[ 0 ].pos = obj.rotation.Rotate( verts[ 0 ].pos );
        verts[ 1 ].pos = obj.rotation.Rotate( verts[ 1 ].pos );
        verts[ 0 ].pos += obj.pos;
        verts[ 1 ].pos += obj.pos;

        verts[ 0 ].color = obj.color;
        verts[ 1 ].color = obj.color;

        m_verts.Append( verts, countof( verts ) );
      }
    }
    else if ( obj.type == DebugType::Line )
    {
      DebugVertex verts[ 2 ];
      verts[ 0 ].pos = obj.pos;
      verts[ 0 ].color = obj.color;
      verts[ 1 ].pos = obj.end;
      verts[ 1 ].color = obj.color;

      m_verts.Append( verts, countof( verts ) );
    }
    else if ( obj.type == DebugType::AABB )
    {
      Vec3 s = obj.size;
      Vec3 c[] =
      {
        obj.pos + Vec3( -s.x, s.y, s.z ),
        obj.pos + s,
        obj.pos + Vec3( s.x, -s.y, s.z ),
        obj.pos + Vec3( -s.x, -s.y, s.z ),
        obj.pos + Vec3( -s.x, s.y, -s.z ),
        obj.pos + Vec3( s.x, s.y, -s.z ),
        obj.pos + Vec3( s.x, -s.y, -s.z ),
        obj.pos + Vec3( -s.x, -s.y, -s.z )
      };
      AE_STATIC_ASSERT( countof( c ) == 8 );

      DebugVertex verts[] =
      {
        // Top
        { c[ 0 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 0 ], obj.color },
        // Sides
        { c[ 0 ], obj.color },
        { c[ 4 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 7 ], obj.color },
        //Bottom
        { c[ 4 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 4 ], obj.color },
      };
      AE_STATIC_ASSERT( countof( c ) * 3 == countof( verts ) );
      
      m_verts.Append( verts, countof( verts ) );
    }
    else if ( obj.type == DebugType::Cube )
    {
      Vec3 c[] =
      {
        ( obj.transform * Vec4( -0.5f, 0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * Vec4( 0.5f, 0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * Vec4( 0.5f, -0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * Vec4( -0.5f, -0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * Vec4( -0.5f, 0.5f, -0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * Vec4( 0.5f, 0.5f, -0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * Vec4( 0.5f, -0.5f, -0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * Vec4( -0.5f, -0.5f, -0.5f, 1.0f ) ).GetXYZ()
      };
      AE_STATIC_ASSERT( countof( c ) == 8 );

      DebugVertex verts[] =
      {
        // Top
        { c[ 0 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 0 ], obj.color },
        // Sides
        { c[ 0 ], obj.color },
        { c[ 4 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 7 ], obj.color },
        //Bottom
        { c[ 4 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 4 ], obj.color },
      };
      AE_STATIC_ASSERT( countof( c ) * 3 == countof( verts ) );

      m_verts.Append( verts, countof( verts ) );
    }
  }

  if ( m_verts.Length() )
  {
    m_vertexData.SetVertices( &m_verts[ 0 ], ae::Min( m_verts.Length(), m_vertexData.GetMaxVertexCount() ) );
    m_vertexData.Upload();

    UniformList uniforms;
    uniforms.Set( "u_worldToNdc", worldToNdc );

    if ( m_xray )
    {
      m_shader.SetDepthTest( false );
      m_shader.SetDepthWrite( false );
      uniforms.Set( "u_saturation", 0.1f );
      m_vertexData.Render( &m_shader, uniforms );
    }

    m_shader.SetDepthTest( true );
    m_shader.SetDepthWrite( true );
    uniforms.Set( "u_saturation", 1.0f );
    m_vertexData.Render( &m_shader, uniforms );
  }

  m_objs.Clear();
}

void DebugLines::Clear()
{
  m_objs.Clear();
}

bool DebugLines::AddLine( Vec3 p0, Vec3 p1, Color color )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Line;
    obj->pos = p0;
    obj->end = p1;
    obj->color = color;
    return true;
  }
  return false;
}

bool DebugLines::AddDistanceCheck( Vec3 p0, Vec3 p1, float distance )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Line;
    obj->pos = p0;
    obj->end = p1;
    obj->color = ( ( p1 - p0 ).Length() <= distance ) ? Color::Green() : Color::Red();
    return true;
  }
  return false;
}

bool DebugLines::AddRect( Vec3 pos, Vec3 up, Vec3 normal, Vec2 size, Color color )
{
  if ( m_objs.Length() < m_objs.Size()
    && up.LengthSquared() > 0.001f
    && normal.LengthSquared() > 0.001f )
  {
    up.SafeNormalize();
    normal.SafeNormalize();
    if ( normal.Dot( up ) < 0.999f )
    {
      DebugObject* obj = &m_objs.Append( DebugObject() );
      obj->type = DebugType::Rect;
      obj->pos = pos;
      obj->rotation = Quaternion( normal, up );
      obj->size = Vec3( size );
      obj->color = color;
      obj->pointCount = 0;
      return true;
    }
  }
  return false;
}

bool DebugLines::AddCircle( Vec3 pos, Vec3 normal, float radius, Color color, uint32_t pointCount )
{
  if ( m_objs.Length() < m_objs.Size() && normal.LengthSquared() > 0.001f )
  {
    normal.SafeNormalize();
    float dot = normal.Dot( Vec3(0,0,1) );
    
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Circle;
    obj->pos = pos;
    obj->rotation = Quaternion( normal, ( dot < 0.99f && dot > -0.99f ) ? Vec3(0,0,1) : Vec3(1,0,0) );
    obj->radius = radius;
    obj->color = color;
    obj->pointCount = pointCount;
    return true;
  }
  return false;
}

bool DebugLines::AddAABB( Vec3 pos, Vec3 halfSize, Color color )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::AABB;
    obj->pos = pos;
    obj->rotation = Quaternion::Identity();
    obj->size = halfSize;
    obj->color = color;
    obj->pointCount = 0;
    return true;
  }
  return false;
}

bool DebugLines::AddOBB( Matrix4 transform, Color color )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Cube;
    obj->transform = transform;
    obj->color = color;
    return true;
  }
  return false;
}

bool DebugLines::AddSphere( Vec3 pos, float radius, Color color, uint32_t pointCount )
{
  if ( m_objs.Length() + 3 <= m_objs.Size() )
  if ( AddCircle( pos, Vec3(1,0,0), radius, color, pointCount ) )
  if ( AddCircle( pos, Vec3(0,1,0), radius, color, pointCount ) )
  if ( AddCircle( pos, Vec3(0,0,1), radius, color, pointCount ) )
  {
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
void _CheckALError()
{
  const char* errStr = "UNKNOWN_ERROR";
  switch ( alGetError() )
  {
  case AL_NO_ERROR: return;
  case AL_INVALID_NAME: errStr = "AL_INVALID_NAME"; break;
  case AL_INVALID_ENUM: errStr = "AL_INVALID_ENUM"; break;
  case AL_INVALID_VALUE: errStr = "AL_INVALID_VALUE"; break;
  case AL_INVALID_OPERATION: errStr = "AL_INVALID_OPERATION"; break;
  case AL_OUT_OF_MEMORY: errStr = "AL_OUT_OF_MEMORY"; break;
  default: break;
  }
  AE_LOG( "OpenAL Error: #", errStr );
  AE_FAIL();
}

void _LoadWavFile( const char* fileName, ALuint* buffer, float* length )
{
  struct ChunkHeader
  {
    char chunkId[ 4 ];
    uint32_t chunkSize;
  };

  struct FormatChunk
  {
    uint16_t formatCode;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint16_t dwChannelMask;
    uint8_t subformat[ 16 ];
  };

  struct RiffChunk
  {
    char waveId[ 4 ];
  };

  FormatChunk wave_format;
  bool hasReadFormat = false;
  uint32_t dataSize = 0;

  uint32_t fileSize = ae::FileSystem::GetSize( fileName );
  AE_ASSERT_MSG( fileSize, "Could not open wav file: #", fileName );

  ae::Scratch< uint8_t > fileScratch( AE_ALLOC_TAG_AUDIO, fileSize );
  uint8_t* fileBuffer = fileScratch.Data();
  ae::FileSystem::Read( fileName, fileBuffer, fileSize );

  ChunkHeader header;

  uint32_t fileOffset = 0;
  memcpy( &header, fileBuffer + fileOffset, sizeof(header) );
  fileOffset += sizeof(header);
  while ( fileOffset < fileSize )
  {
    if ( memcmp( header.chunkId, "RIFF", 4 ) == 0 )
    {
      RiffChunk riff;
      memcpy( &riff, fileBuffer + fileOffset, sizeof(riff) );
      fileOffset += sizeof(riff);
      AE_ASSERT( memcmp( riff.waveId, "WAVE", 4 ) == 0 );
    }
    else if ( memcmp( header.chunkId, "fmt ", 4 ) == 0 )
    {
      memcpy( &wave_format, fileBuffer + fileOffset, header.chunkSize );
      fileOffset += header.chunkSize;
      hasReadFormat = true;
    }
    else if ( memcmp( header.chunkId, "data", 4 ) == 0 )
    {
      AE_ASSERT( hasReadFormat );
      AE_ASSERT_MSG( dataSize == 0, "Combining WAV data chunks is currently not supported" );

      uint8_t* data = new uint8_t[ header.chunkSize ];
      memcpy( data, fileBuffer + fileOffset, header.chunkSize );
      fileOffset += header.chunkSize;

      ALsizei size = header.chunkSize;
      ALsizei frequency = wave_format.sampleRate;
      dataSize = size;

      ALenum format;
      if ( wave_format.numChannels == 1 )
      {
        if ( wave_format.bitsPerSample == 8 ) { format = AL_FORMAT_MONO8; }
        else if ( wave_format.bitsPerSample == 16 ) { format = AL_FORMAT_MONO16; }
      }
      else if ( wave_format.numChannels == 2 )
      {
        if ( wave_format.bitsPerSample == 8 ) { format = AL_FORMAT_STEREO8; }
        else if ( wave_format.bitsPerSample == 16 ) { format = AL_FORMAT_STEREO16; }
      }
      alGenBuffers( 1, buffer );
      alBufferData( *buffer, format, (void*)data, size, frequency );
      delete[] data;
    }
    else
    {
      fileOffset += header.chunkSize;
    }

    memcpy( &header, fileBuffer + fileOffset, sizeof(header) );
    fileOffset += sizeof(header);
  }

  _CheckALError();

  AE_ASSERT( hasReadFormat );
  AE_ASSERT( dataSize );

  *length = dataSize / ( wave_format.sampleRate * wave_format.numChannels * wave_format.bitsPerSample / 8.0f );
}

//------------------------------------------------------------------------------
// ae::AudioData member functions
//------------------------------------------------------------------------------
AudioData::AudioData()
{
  name = "";
  buffer = 0;
  length = 0.0f;
}

void AudioData::Initialize( const char* filePath )
{
  AE_ASSERT( !buffer );
  this->name = filePath; // @TODO: Should just be file name or file hash
  _LoadWavFile( filePath, &this->buffer, &this->length );
  AE_ASSERT( buffer );
}

void AudioData::Terminate()
{
  AE_ASSERT( buffer );

  alDeleteBuffers( 1, &this->buffer );
  _CheckALError();

  *this = AudioData();
}

//------------------------------------------------------------------------------
// ae::Audio::Channel member functions
//------------------------------------------------------------------------------
Audio::Channel::Channel()
{
  source = 0;
  priority = ae::MaxValue< int32_t >();
  resource = nullptr;
}

//------------------------------------------------------------------------------
// ae::Audio member functions
//------------------------------------------------------------------------------
void Audio::Initialize( uint32_t musicChannels, uint32_t sfxChannels )
{
  ALCdevice* device = alcOpenDevice( nullptr );
  AE_ASSERT( device );
  ALCcontext* ctx = alcCreateContext( device, nullptr );
  alcMakeContextCurrent( ctx );
  AE_ASSERT( ctx );
  _CheckALError();
  
  ae::Array< ALuint > sources( AE_ALLOC_TAG_AUDIO, musicChannels + sfxChannels, 0 );
  alGenSources( (ALuint)sources.Length(), sources.Begin() );

  m_musicChannels.Reserve( musicChannels );
  for ( uint32_t i = 0; i < musicChannels; i++ )
  {
    Channel* channel = &m_musicChannels.Append( Channel() );
    channel->source = sources[ i ];
    alGenSources( (ALuint)1, &channel->source );
    alSourcef( channel->source, AL_PITCH, 1 );
    alSourcef( channel->source, AL_GAIN, 1.0f );
    alSource3f( channel->source, AL_POSITION, 0, 0, 0 );
    alSourcei( channel->source, AL_LOOPING, AL_TRUE );
  }

  m_sfxChannels.Reserve( sfxChannels );
  for ( uint32_t i = 0; i < sfxChannels; i++ )
  {
    Channel* channel = &m_sfxChannels.Append( Channel() );
    channel->source = sources[ musicChannels + i ];
    alSourcef( channel->source, AL_PITCH, 1 );
    alSourcef( channel->source, AL_GAIN, 1.0f );
    alSource3f( channel->source, AL_POSITION, 0, 0, 0 );
    alSourcei( channel->source, AL_LOOPING, AL_FALSE );
  }

  ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
  alListener3f( AL_POSITION, 0, 0, 1.0f );
  alListenerfv( AL_ORIENTATION, listenerOri );
  alListenerf( AL_GAIN, 1.0f );

  _CheckALError();
}

void Audio::Terminate()
{
  for ( uint32_t i = 0; i < m_musicChannels.Length(); i++ )
  {
    Channel* channel = &m_musicChannels[ i ];
    alDeleteSources( 1, &channel->source );
    channel->source = -1;
  }

  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    Channel* channel = &m_sfxChannels[ i ];
    alDeleteSources( 1, &channel->source );
    channel->source = -1;
  }

  ALCcontext* ctx = alcGetCurrentContext();
  ALCdevice* device = alcGetContextsDevice( ctx );
  alcMakeContextCurrent( nullptr );
  alcDestroyContext( ctx );
  alcCloseDevice( device );
}

void Audio::SetVolume( float volume )
{
  volume = ae::Clip01( volume );
  alListenerf( AL_GAIN, volume );
}

void Audio::PlayMusic( const AudioData* audioFile, float volume, uint32_t channel )
{
  AE_ASSERT( audioFile );
  if ( channel >= m_musicChannels.Length() )
  {
    return;
  }

  Channel* musicChannel = &m_musicChannels[ channel ];

  ALint state;
  alGetSourcei( musicChannel->source, AL_SOURCE_STATE, &state );
  if ( ( audioFile == musicChannel->resource ) && state == AL_PLAYING )
  {
    return;
  }

  if ( state == AL_PLAYING )
  {
    alSourceStop( musicChannel->source );
  }

  alSourcei( musicChannel->source, AL_BUFFER, audioFile->buffer );
  alSourcef( musicChannel->source, AL_GAIN, volume );
  alSourcePlay( musicChannel->source );
  _CheckALError();
}

void Audio::PlaySfx( const AudioData* audioFile, float volume, int32_t priority )
{
  ALint state;
  AE_ASSERT( audioFile );

  Channel* leastPriorityChannel = nullptr;
  Channel* unusedChannel = nullptr;
  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    Channel* sfxChannel = &m_sfxChannels[ i ];
    if ( !leastPriorityChannel || sfxChannel->priority >= leastPriorityChannel->priority )
    {
      leastPriorityChannel = sfxChannel;
    }

    if ( !unusedChannel )
    {
      alGetSourcei( sfxChannel->source, AL_SOURCE_STATE, &state );
      if ( state != AL_PLAYING )
      {
        unusedChannel = sfxChannel;
      }
    }
  }
  AE_ASSERT( leastPriorityChannel );

  Channel* currentChannel = nullptr;
  if ( unusedChannel )
  {
    currentChannel = unusedChannel;
  }
  else if ( !leastPriorityChannel || leastPriorityChannel->priority < priority )
  {
    return;
  }
  else
  {
    currentChannel = leastPriorityChannel;
  }
  AE_ASSERT( currentChannel );

  alSourceStop( currentChannel->source );
  alGetSourcei( currentChannel->source, AL_SOURCE_STATE, &state );
  AE_ASSERT( state != AL_PLAYING );

  currentChannel->resource = audioFile;

  alSourcei( currentChannel->source, AL_BUFFER, audioFile->buffer );
  alSourcef( currentChannel->source, AL_GAIN, volume );
  alSourcePlay( currentChannel->source );
  _CheckALError();
  currentChannel->priority = priority;

  alGetSourcei( currentChannel->source, AL_SOURCE_STATE, &state );
}

void Audio::StopMusic( uint32_t channel )
{
  if ( channel < m_musicChannels.Length() )
  {
    alSourceStop( m_musicChannels[ channel ].source );
  }
}

void Audio::StopAllSfx()
{
  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    alSourceStop( m_sfxChannels[ i ].source );
  }
}

uint32_t Audio::GetMusicChannelCount() const
{
  return m_musicChannels.Length();
}

uint32_t Audio::GetSfxChannelCount() const
{
  return m_sfxChannels.Length();
}

// @TODO: Should return a string with current state of audio channels
void Audio::Log()
{
  for ( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
  {
    ALint state = 0;
    const Channel* channel = &m_sfxChannels[ i ];
    alGetSourcei( channel->source, AL_SOURCE_STATE, &state );

    if ( state == AL_PLAYING )
    {
      AE_ASSERT( channel->resource );

      float playOffset = 0.0f;
      alGetSourcef( channel->source, AL_SEC_OFFSET, &playOffset );

      float playLength = channel->resource->length;

      const char* soundName = strrchr( channel->resource->name.c_str(), '/' );
      soundName = soundName ? soundName + 1 : channel->resource->name.c_str();
      const char* soundNameEnd = strrchr( channel->resource->name.c_str(), '.' );
      soundNameEnd = soundNameEnd ? soundNameEnd : soundName + strlen( soundName );
      uint32_t soundNameLen = (uint32_t)(soundNameEnd - soundName);

      char buffer[ 512 ];
      sprintf( buffer, "channel:%u name:%.*s offset:%.2fs length:%.2fs", i, soundNameLen, soundName, playOffset, playLength );
      AE_LOG( buffer );
    }
  }

  _CheckALError();
}

//------------------------------------------------------------------------------
// ae::BinaryStream member functions
//------------------------------------------------------------------------------
BinaryStream::BinaryStream( Mode mode, uint8_t* data, uint32_t length )
{
  m_mode = mode;
  m_data = data;
  m_length = length;
  m_isValid = m_data && m_length;
}

BinaryStream::BinaryStream( Mode mode, const uint8_t* data, uint32_t length )
{
  AE_ASSERT_MSG( mode == Mode::ReadBuffer, "Only read mode can be used with a constant data buffer." );
  m_mode = mode;
  m_data = const_cast< uint8_t* >( data );
  m_length = length;
  m_isValid = m_data && m_length;
}

BinaryStream::BinaryStream( Mode mode )
{
  AE_ASSERT_MSG( mode == Mode::WriteBuffer, "Only write mode can be used when a data buffer is not provided." );
  m_mode = mode;
  m_isValid = true;
}

BinaryStream::BinaryStream( Array< uint8_t >* array )
{
  m_mode = Mode::WriteBuffer;
  if ( array )
  {
    m_extArray = array;
    m_offset = m_extArray->Length();
    m_length = m_extArray->Size();
    m_isValid = true;
  }
}

BinaryStream BinaryStream::Writer( uint8_t* data, uint32_t length )
{
  return BinaryStream( Mode::WriteBuffer, data, length );
}

BinaryStream BinaryStream::Writer( Array< uint8_t >* array )
{
  return BinaryStream( array );
}

BinaryStream BinaryStream::Writer()
{
  return BinaryStream( Mode::WriteBuffer );
}

BinaryStream BinaryStream::Reader( const uint8_t* data, uint32_t length )
{
  return BinaryStream( Mode::ReadBuffer, const_cast< uint8_t* >( data ), length );
}

BinaryStream BinaryStream::Reader( const Array< uint8_t >& data )
{
  if ( !data.Length() )
  {
    return BinaryStream::Reader( nullptr, 0 );
  }
  return BinaryStream( Mode::ReadBuffer, &data[ 0 ], data.Length() );
}

void BinaryStream::SerializeUint8( uint8_t& v )
{
  SerializeRaw( &v, sizeof(v) );
}

void BinaryStream::SerializeUint8( const uint8_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( &v, sizeof(v) );
}

void BinaryStream::SerializeUint16( uint16_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeUint16( const uint16_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeUint32( uint32_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeUint32( const uint32_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeUint64( uint64_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeUint64( const uint64_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt8( int8_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt8( const int8_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt16( int16_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt16( const int16_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt32( int32_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt32( const int32_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt64( int64_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt64( const int64_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeFloat( float& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeFloat( const float& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeDouble( double& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeDouble( const double& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeBool( bool& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeBool( const bool& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::m_SerializeArrayLength( uint32_t& length, uint32_t maxLength )
{
  if ( maxLength <= ae::MaxValue< uint8_t >() )
  {
    uint8_t len = (uint8_t)length;
    SerializeUint8( len );
    length = len;
  }
  else if ( maxLength <= ae::MaxValue< uint16_t >() )
  {
    uint16_t len = (uint16_t)length;
    SerializeUint16( len );
    length = len;
  }
  else
  {
    uint32_t len = length;
    SerializeUint32( len );
    length = len;
  }
}

void BinaryStream::SerializeArray( Array< uint8_t >& array, uint32_t maxLength )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    uint32_t length = 0;
    m_SerializeArrayLength( length, maxLength );
    if ( !m_isValid || length == 0 )
    {
      return;
    }
    else if ( GetRemaining() < length )
    {
      Invalidate();
      return;
    }
    
    array.Append( PeekData(), length );
    Discard( length );
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    uint32_t length = array.Length();
    m_SerializeArrayLength( length, maxLength );
    if ( length )
    {
      SerializeRaw( &array[ 0 ], length );
    }
  }
}

void BinaryStream::SerializeArray( const Array< uint8_t >& array, uint32_t maxLength )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  
  uint32_t length = array.Length();
  m_SerializeArrayLength( length, maxLength );
  if ( length )
  {
    SerializeRaw( &array[ 0 ], length );
  }
}

void BinaryStream::SerializeRaw( void* data, uint32_t length )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    if ( m_offset + length <= m_length )
    {
      memcpy( data, m_data + m_offset, length );
      m_offset += length;
    }
    else
    {
      Invalidate();
    }
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    if ( m_data )
    {
      if ( length <= m_length - m_offset )
      {
        memcpy( m_data + m_offset, data, length );
        m_offset += length;
      }
      else
      {
        Invalidate();
      }
    }
    else
    {
      Array< uint8_t >& array = m_GetArray();
      array.Append( (uint8_t*)data, length );
      m_offset = array.Length();
      m_length = array.Size();
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with ae::BinaryStream::Writer or ae::BinaryStream::Reader static functions." );
  }
}

void BinaryStream::SerializeRaw( const void* data, uint32_t length )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  SerializeRaw( (void*)data, length );
}

void BinaryStream::SerializeRaw( Array< uint8_t>& array )
{
  AE_FAIL_MSG( "Not implemented" );
}

void BinaryStream::SerializeRaw( const Array< uint8_t>& array )
{
  AE_FAIL_MSG( "Not implemented" );
}

void BinaryStream::Discard( uint32_t length )
{
  if ( !length )
  {
    return;
  }
  else if ( GetRemaining() < length )
  {
    Invalidate();
  }
  else
  {
    m_offset += length;
  }
}

//------------------------------------------------------------------------------
// ae::NetObject member functions
//------------------------------------------------------------------------------
void NetObject::SetSyncData( const void* data, uint32_t length )
{
  AE_ASSERT_MSG( IsAuthority(), "Cannot set net data from client. The NetObjectConnection has exclusive ownership." );
  m_data.Clear();
  m_data.Append( (const uint8_t*)data, length );
}

// @HACK: Should rearrange file so windows.h is included with as little logic as possible after it
#ifdef SendMessage
#undef SendMessage
#endif
void NetObject::SendMessage( const void* data, uint32_t length )
{
  uint16_t lengthU16 = length;
  m_messageDataOut.Reserve( m_messageDataOut.Length() + sizeof( lengthU16 ) + length );
  m_messageDataOut.Append( (uint8_t*)&lengthU16, sizeof( lengthU16 ) );
  m_messageDataOut.Append( (const uint8_t*)data, length );
}

void NetObject::SetInitData( const void* initData, uint32_t initDataLength )
{
  m_initData.Clear();
  m_initData.Append( (uint8_t*)initData, initDataLength );
  m_isPendingInit = false;
}

const uint8_t* NetObject::GetInitData() const
{
  return m_initData.Length() ? &m_initData[ 0 ] : nullptr;
}

uint32_t NetObject::InitDataLength() const
{
  return m_initData.Length();
}

const uint8_t* NetObject::GetSyncData() const
{
  return m_data.Length() ? &m_data[ 0 ] : nullptr;
}

uint32_t NetObject::SyncDataLength() const
{
  return m_data.Length();
}

void NetObject::ClearSyncData()
{
  m_data.Clear();
}

bool NetObject::PumpMessages( Msg* msgOut )
{
  if ( m_messageDataInOffset >= m_messageDataIn.Length() )
  {
    AE_ASSERT( m_messageDataInOffset == m_messageDataIn.Length() );
    return false;
  }
  else if ( !msgOut )
  {
    // Early out
    return true;
  }

  // Write out incoming message data
  msgOut->length = *(uint16_t*)&m_messageDataIn[ m_messageDataInOffset ];
  m_messageDataInOffset += sizeof( uint16_t );

  msgOut->data = &m_messageDataIn[ m_messageDataInOffset ];
  m_messageDataInOffset += msgOut->length;

  if ( m_messageDataInOffset >= m_messageDataIn.Length() )
  {
    AE_ASSERT( m_messageDataInOffset == m_messageDataIn.Length() );

    // Clear messages once they've all been read
    m_messageDataInOffset = 0;
    m_messageDataIn.Clear();
  }

  return true;
}

bool NetObject::IsPendingInit() const
{
  return m_isPendingInit;
}

bool NetObject::IsPendingDestroy() const
{
  return m_isPendingDestroy;
}

void NetObject::m_SetClientData( const uint8_t* data, uint32_t length )
{
  m_data.Clear();
  m_data.Append( data, length );
}

void NetObject::m_ReceiveMessages( const uint8_t* data, uint32_t length )
{
  m_messageDataIn.Append( data, length );
}

void NetObject::m_UpdateHash()
{
  if ( m_data.Length() )
  {
    m_hash = ae::Hash().HashData( &m_data[ 0 ], m_data.Length() ).Get();
  }
  else
  {
    m_hash = 0;
  }
}

//------------------------------------------------------------------------------
// ae::NetObjectClient member functions
//------------------------------------------------------------------------------
void NetObjectClient::ReceiveData( const uint8_t* data, uint32_t length )
{
  BinaryStream rStream = BinaryStream::Reader( data, length );
  while ( rStream.GetOffset() < rStream.GetLength() )
  {
    NetObjectConnection::EventType type;
    rStream.SerializeRaw( type );
    if ( !rStream.IsValid() )
    {
      break;
    }
    switch ( type )
    {
      case NetObjectConnection::EventType::Connect:
      {
        uint32_t signature = 0;
        rStream.SerializeUint32( signature );
        AE_ASSERT( signature );

        ae::Map< NetObject*, int > toDestroy = AE_ALLOC_TAG_NET;
        bool allowResolve = ( m_serverSignature == signature );
        if ( m_serverSignature )
        {
          if ( allowResolve )
          {
            for ( uint32_t i = 0; i < m_netObjects.Length(); i++ )
            {
              toDestroy.Set( m_netObjects.GetValue( i ), 0 );
            }
          }
          else
          {
            m_delayCreationForDestroy = true; // This prevents new server objects and old server objects overlapping for a frame
            m_created.Clear(); // Don't call delete, are pointers to m_netObjects
            for ( uint32_t i = 0; i < m_netObjects.Length(); i++ )
            {
              m_StartNetObjectDestruction( m_netObjects.GetValue( i ) );
            }
            AE_ASSERT( !m_remoteToLocalIdMap.Length() );
            AE_ASSERT( !m_localToRemoteIdMap.Length() );
          }
        }
        
        uint32_t length = 0;
        rStream.SerializeUint32( length );
        for ( uint32_t i = 0; i < length && rStream.IsValid(); i++ )
        {
          NetObject* created = m_CreateNetObject( &rStream, allowResolve );
          toDestroy.Remove( created );
        }
        for ( uint32_t i = 0; i < toDestroy.Length(); i++ )
        {
          NetObject* netObject = toDestroy.GetKey( i );
          m_StartNetObjectDestruction( netObject );
        }

        m_serverSignature = signature;
        break;
      }
      case NetObjectConnection::EventType::Create:
      {
        m_CreateNetObject( &rStream, false );
        break;
      }
      case NetObjectConnection::EventType::Destroy:
      {
        RemoteId remoteId;
        rStream.SerializeObject( remoteId );
        
        NetId localId;
        NetObject* netObject = nullptr;
        // Try to find object, may have been deleted locally
        if ( m_remoteToLocalIdMap.TryGet( remoteId, &localId )
            && m_netObjects.TryGet( localId, &netObject ) )
        {
          m_StartNetObjectDestruction( netObject );
        }
        break;
      }
      case NetObjectConnection::EventType::Update:
      {
        uint32_t netObjectCount = 0;
        rStream.SerializeUint32( netObjectCount );
        for ( uint32_t i = 0; i < netObjectCount; i++ )
        {
          RemoteId remoteId;
          uint32_t dataLen = 0;
          rStream.SerializeObject( remoteId );
          rStream.SerializeUint32( dataLen );

          NetId localId;
          NetObject* netObject = nullptr;
          if ( dataLen
            && m_remoteToLocalIdMap.TryGet( remoteId, &localId )
            && m_netObjects.TryGet( localId, &netObject ) )
          {
            if ( rStream.GetRemaining() >= dataLen )
            {
              netObject->m_SetClientData( rStream.PeekData(), dataLen );
            }
            else
            {
              rStream.Invalidate();
            }
          }

          rStream.Discard( dataLen );
        }
        break;
      }
      case NetObjectConnection::EventType::Messages:
      {
        uint32_t netObjectCount = 0;
        rStream.SerializeUint32( netObjectCount );
        for ( uint32_t i = 0; i < netObjectCount; i++ )
        {
          RemoteId remoteId;
          uint32_t dataLen = 0;
          rStream.SerializeObject( remoteId );
          rStream.SerializeUint32( dataLen );

          NetId localId;
          NetObject* netObject = nullptr;
          if ( dataLen
            && m_remoteToLocalIdMap.TryGet( remoteId, &localId )
            && m_netObjects.TryGet( localId, &netObject ) )
          {
            if ( rStream.GetRemaining() >= dataLen )
            {
              netObject->m_ReceiveMessages( rStream.PeekData(), dataLen );
            }
            else
            {
              rStream.Invalidate();
            }
          }

          rStream.Discard( dataLen );
        }
        break;
      }
    }
  }
}

NetObject* NetObjectClient::PumpCreate()
{
  if ( !m_created.Length() )
  {
    return nullptr;
  }

  if ( m_delayCreationForDestroy )
  {
    for ( uint32_t i = 0; i < m_netObjects.Length(); i++ )
    {
      if ( m_netObjects.GetValue( i )->IsPendingDestroy() )
      {
        return nullptr;
      }
    }
    m_delayCreationForDestroy = false;
  }
  
  NetObject* created = m_created[ 0 ];
  AE_ASSERT( created );
  m_created.Remove( 0 );
  return created;
}

void NetObjectClient::Destroy( NetObject* pendingDestroy )
{
  if ( !pendingDestroy )
  {
    return;
  }
  bool removed = m_netObjects.Remove( pendingDestroy->GetId() );
  AE_ASSERT_MSG( removed, "ae::NetObject can't be destroyed. It's' not managed by this ae::NetObjectClient." );
  
  if ( pendingDestroy->IsPendingDestroy() )
  {
    m_StartNetObjectDestruction( pendingDestroy );
  }
  ae::Delete( pendingDestroy );
}

NetObject* NetObjectClient::m_CreateNetObject( BinaryStream* rStream, bool allowResolve )
{
  AE_ASSERT( rStream->IsReader() );

  RemoteId remoteId;
  rStream->SerializeObject( remoteId );

  NetObject* netObject = nullptr;
  if ( allowResolve )
  {
    NetId localId = m_remoteToLocalIdMap.Get( remoteId, {} );
    if ( localId )
    {
      netObject = m_netObjects.Get( localId );
    }
  }

  if ( !netObject )
  {
    NetId localId( ++m_lastNetId );
    netObject = ae::New< NetObject >( AE_ALLOC_TAG_NET );
    netObject->m_id = localId;

    m_netObjects.Set( localId, netObject );
    m_remoteToLocalIdMap.Set( remoteId, localId );
    m_localToRemoteIdMap.Set( localId, remoteId );
    m_created.Append( netObject );
  }
  
  rStream->SerializeArray( netObject->m_initData );

  return netObject;
}

void NetObjectClient::m_StartNetObjectDestruction( NetObject* netObject )
{
  AE_ASSERT( netObject );
  if ( netObject->IsPendingDestroy() )
  {
    return;
  }
  
  RemoteId remoteId;
  bool found = m_localToRemoteIdMap.Remove( netObject->GetId(), &remoteId );
  AE_ASSERT( found );
  found = m_remoteToLocalIdMap.Remove( remoteId );
  AE_ASSERT( found );
  netObject->m_FlagForDestruction();
}

//------------------------------------------------------------------------------
// ae::NetObjectConnection member functions
//------------------------------------------------------------------------------
void NetObjectConnection::m_UpdateSendData()
{
  AE_ASSERT( m_replicaDB );

  ae::Array< NetObject* > toSync = AE_ALLOC_TAG_NET;
  uint32_t netObjectMessageCount = 0;
  for ( uint32_t i = 0; i < m_replicaDB->GetNetObjectCount(); i++ )
  {
    NetObject* netObject = m_replicaDB->GetNetObject( i );
    if ( m_first || netObject->m_Changed() )
    {
      toSync.Append( netObject );
    }

    if ( netObject->m_messageDataOut.Length() )
    {
      netObjectMessageCount++;
    }
  }

  BinaryStream wStream = BinaryStream::Writer( &m_sendData );

  if ( toSync.Length() )
  {
    wStream.SerializeRaw( NetObjectConnection::EventType::Update );
    wStream.SerializeUint32( toSync.Length() );
    for ( uint32_t i = 0; i < toSync.Length(); i++ )
    {
      NetObject* netObject = toSync[ i ];
      wStream.SerializeObject( netObject->GetId() );
      wStream.SerializeUint32( netObject->SyncDataLength() );
      wStream.SerializeRaw( netObject->GetSyncData(), netObject->SyncDataLength() );
    }
  }

  if ( netObjectMessageCount )
  {
    wStream.SerializeRaw( NetObjectConnection::EventType::Messages );
    wStream.SerializeUint32( netObjectMessageCount );
    for ( uint32_t i = 0; i < m_replicaDB->GetNetObjectCount(); i++ )
    {
      NetObject* netObject = m_replicaDB->GetNetObject( i );
      if ( netObject->m_messageDataOut.Length() )
      {
        wStream.SerializeObject( netObject->GetId() );
        wStream.SerializeUint32( netObject->m_messageDataOut.Length() );
        wStream.SerializeRaw( &netObject->m_messageDataOut[ 0 ], netObject->m_messageDataOut.Length() );
      }
    }
  }

  m_pendingClear = true;
  m_first = false;
}

const uint8_t* NetObjectConnection::GetSendData() const
{
  return m_sendData.Length() ? &m_sendData[ 0 ] : nullptr;
}

uint32_t NetObjectConnection::GetSendLength() const
{
  return m_sendData.Length();
}

//------------------------------------------------------------------------------
// ae::NetObjectServer member functions
//------------------------------------------------------------------------------
NetObjectServer::NetObjectServer()
{
  std::random_device random_device;
  std::mt19937 random_engine( random_device() );
  std::uniform_int_distribution< uint32_t > dist( 1, ae::MaxValue< uint32_t >() );
  m_signature = dist( random_engine );
}

NetObject* NetObjectServer::CreateNetObject()
{
  NetObject* netObject = ae::New< NetObject >( AE_ALLOC_TAG_NET );
  netObject->m_SetLocal();
  netObject->m_id = NetId( ++m_lastNetId );
  m_pendingCreate.Append( netObject );
  return netObject;
}

void NetObjectServer::DestroyNetObject( NetObject* netObject )
{
  if ( !netObject )
  {
    return;
  }
  
  int32_t pendingIdx = m_pendingCreate.Find( netObject );
  if ( pendingIdx >= 0 )
  {
    // Early out, no need to send Destroy message because Create has not been queued
    m_pendingCreate.Remove( pendingIdx );
    ae::Delete( netObject );
    return;
  }

  NetId id = netObject->GetId();
  bool removed = m_netObjects.Remove( id );
  AE_ASSERT_MSG( removed, "NetObject was not found." );

  for ( uint32_t i = 0; i < m_servers.Length(); i++ )
  {
    NetObjectConnection* server = m_servers[ i ];
    if ( server->m_pendingClear )
    {
      server->m_sendData.Clear();
      server->m_pendingClear = false;
    }

    BinaryStream wStream = BinaryStream::Writer( &server->m_sendData );
    wStream.SerializeRaw( NetObjectConnection::EventType::Destroy );
    wStream.SerializeObject( id );
  }

  ae::Delete( netObject );
}

NetObjectConnection* NetObjectServer::CreateConnection()
{
  NetObjectConnection* server = m_servers.Append( ae::New< NetObjectConnection >( AE_ALLOC_TAG_NET ) );
  AE_ASSERT( !server->m_pendingClear );
  server->m_replicaDB = this;

  // Send initial net datas
  BinaryStream wStream = BinaryStream::Writer( &server->m_sendData );
  wStream.SerializeRaw( NetObjectConnection::EventType::Connect );
  wStream.SerializeUint32( m_signature );
  wStream.SerializeUint32( m_netObjects.Length() );
  for ( uint32_t i = 0; i < m_netObjects.Length(); i++ )
  {
    const NetObject* netObject = m_netObjects.GetValue( i );
    wStream.SerializeObject( netObject->GetId() );
    wStream.SerializeArray( netObject->m_initData );
  }

  return server;
}

void NetObjectServer::DestroyConnection( NetObjectConnection* server )
{
  if ( !server )
  {
    return;
  }

  int32_t index = m_servers.Find( server );
  if ( index >= 0 )
  {
    m_servers.Remove( index );
    ae::Delete( server );
  }
}

void NetObjectServer::UpdateSendData()
{
  // Clear old send data before writing new
  for ( uint32_t i = 0; i < m_servers.Length(); i++ )
  {
    NetObjectConnection* server = m_servers[ i ];
    if ( server->m_pendingClear )
    {
      server->m_sendData.Clear();
      server->m_pendingClear = false;
    }
  }
  
  // Send info about new objects (delayed until Update in case objects initData need to reference each other)
  for ( NetObject* netObject : m_pendingCreate )
  {
    if ( !netObject->IsPendingInit() )
    {
      // Add net data to list, remove all initialized net datas from m_pendingCreate at once below
      m_netObjects.Set( netObject->GetId(), netObject );
      
      // Send create messages on existing server connections
      for ( uint32_t i = 0; i < m_servers.Length(); i++ )
      {
        NetObjectConnection* server = m_servers[ i ];
        BinaryStream wStream = BinaryStream::Writer( &server->m_sendData );
        wStream.SerializeRaw( NetObjectConnection::EventType::Create );
        wStream.SerializeObject( netObject->GetId() );
        wStream.SerializeArray( netObject->m_initData );
      }
    }
  }
  // Remove all pending net datas that were just initialized
  m_pendingCreate.RemoveAllFn( []( const NetObject* netObject ){ return !netObject->IsPendingInit(); } );
  
  for ( uint32_t i = 0; i < m_netObjects.Length(); i++ )
  {
    m_netObjects.GetValue( i )->m_UpdateHash();
  }

  for ( uint32_t i = 0; i < m_servers.Length(); i++ )
  {
    m_servers[ i ]->m_UpdateSendData();
  }

  for ( uint32_t i = 0; i < m_netObjects.Length(); i++ )
  {
    NetObject* netObject = m_netObjects.GetValue( i );
    netObject->m_prevHash = netObject->m_hash;
    netObject->m_messageDataOut.Clear();
  }
}

} // ae end

//------------------------------------------------------------------------------
// Meta register base object
//------------------------------------------------------------------------------
// @TODO: Support registering classes in namespaces
//AE_REGISTER_CLASS( ae::Object );
int force_link_aeObject = 0;
template <> const char* ae::_TypeName< ae::Object >::Get() { return "ae::Object"; }
template <> void ae::_DefineType< ae::Object >( ae::Type *type, uint32_t index ) { type->Init< ae::Object >( "ae::Object", index ); }
static ae::_TypeCreator< ae::Object > ae_type_creator_aeObject( "ae::Object" );

uint32_t ae::GetTypeCount()
{
  return (uint32_t)_GetTypes().size();
}

const ae::Type* ae::GetTypeByIndex( uint32_t i )
{
  return _GetTypes()[ i ];
}

const ae::Type* ae::GetTypeById( ae::TypeId id )
{
  return _GetTypeIdMap()[ id ];
}

const ae::Type* ae::GetTypeByName( const char* typeName )
{
  auto it = _GetTypeNameMap().find( typeName );
  if ( it != _GetTypeNameMap().end() ) { return it->second; }
  else { return nullptr; }
}

const ae::Type* ae::GetTypeFromObject( const ae::Object& obj )
{
  return GetTypeFromObject( &obj );
}

const ae::Enum* ae::GetEnum( const char* enumName )
{
  return Enum::s_Get( enumName, false, 0 , false );
}

const ae::Type* ae::GetTypeFromObject( const ae::Object* obj )
{
  if ( !obj )
  {
    return nullptr;
  }
  
  ae::TypeId id = GetObjectTypeId( obj );
  auto it = _GetTypeIdMap().find( id );
  if ( it != _GetTypeIdMap().end() )
  {
    return it->second;
  }
  else
  {
    AE_ASSERT_MSG( false, "No meta info for object '#' type id: #", obj, (uint32_t)id );
    return nullptr;
  }
}

const char* ae::Var::GetName() const { return m_name.c_str(); }
ae::Var::Type ae::Var::GetType() const { return m_type; }
const char* ae::Var::GetTypeName() const { return m_typeName.c_str(); }
uint32_t ae::Var::GetOffset() const { return m_offset; }
uint32_t ae::Var::GetSize() const { return m_size; }

bool ae::Var::SetObjectValueFromString( ae::Object* obj, const char* value, std::function< bool( const ae::Type*, const char*, ae::Object** ) > getObjectPointerFromString ) const
{
  if ( !obj )
  {
    return false;
  }
  
  // Safety check to make sure 'this' Var belongs to 'obj' ae::Type
  const ae::Type* objType = ae::GetTypeFromObject( obj );
  AE_ASSERT( objType );
  AE_ASSERT_MSG( objType == m_owner, "Attempting to modify object '#' with var '#::#'", objType->GetName(), m_owner->GetName(), GetName() );
  
  void* varData = (uint8_t*)obj + m_offset;
  
  switch ( m_type )
  {
    case Var::String:
    {
      switch ( m_size )
      {
        case 16:
        {
          *(ae::Str16*)varData = value;
          return true;
        }
        case 32:
        {
          *(ae::Str32*)varData = value;
          return true;
        }
        case 64:
        {
          ae::Str64* str = (ae::Str64*)varData;
          *str = value;
          return true;
        }
        case 128:
        {
          ae::Str128* str = (ae::Str128*)varData;
          *str = value;
          return true;
        }
        case 256:
        {
          ae::Str256* str = (ae::Str256*)varData;
          *str = value;
          return true;
        }
        case 512:
        {
          ae::Str512* str = (ae::Str512*)varData;
          *str = value;
          return true;
        }
        default:
        {
          AE_ASSERT_MSG( false, "Invalid string size '#'", m_size );
          return false;
        }
      }
    }
    case Var::UInt8:
    {
      AE_ASSERT(m_size == sizeof(uint8_t) );
      uint8_t* u8 = (uint8_t*)varData;
      sscanf( value, "%hhu", u8 );
      return true;
    }
    case Var::UInt16:
    {
      AE_ASSERT(m_size == sizeof(uint16_t) );
      uint16_t* u16 = (uint16_t*)varData;
      sscanf( value, "%hu", u16 );
      return true;
    }
    case Var::UInt32:
    {
      AE_ASSERT(m_size == sizeof(uint32_t) );
      uint32_t* u32 = (uint32_t*)varData;
      sscanf( value, "%u", u32 );
      return true;
    }
    case Var::Int8:
    {
      AE_ASSERT(m_size == sizeof(int8_t) );
      int8_t* i8 = (int8_t*)varData;
      sscanf( value, "%hhd", i8 );
      return true;
    }
    case Var::Int16:
    {
      AE_ASSERT(m_size == sizeof(int16_t) );
      int16_t* i16 = (int16_t*)varData;
      sscanf( value, "%hd", i16 );
      return true;
    }
    case Var::Int32:
    {
      AE_ASSERT(m_size == sizeof(int32_t) );
      int32_t* i32 = (int32_t*)varData;
      sscanf( value, "%d", i32 );
      return true;
    }
    case Var::Bool:
    {
      // @TODO: Clean this up. Should check for both `true` and `false`, and return false if neither match
      const char* trueStr = "true";
      bool b = value[ 0 ];
      if ( b )
      {
        for ( uint32_t i = 0; ( value[ i ] && trueStr[ i ] ); i++ )
        {
          if ( trueStr[ i ] != tolower( value[ i ] ) )
          {
            b = false;
          }
        }
      }
      *(bool*)varData = b;
      return true;
    }
    case Var::Float:
    {
      AE_ASSERT( m_size == sizeof(float) );
      float* f = (float*)varData;
      sscanf( value, "%f", f );
      return true;
    }
    case Var::Matrix4:
    {
      AE_ASSERT( m_size == sizeof(ae::Matrix4) );
      ae::Matrix4* v = (ae::Matrix4*)varData;
      // @TODO: Should match GetObjectValueAsString() which uses ae::Str::Format
      sscanf( value, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
             v->data, v->data + 1, v->data + 2, v->data + 3,
             v->data + 4, v->data + 5, v->data + 6, v->data + 7,
             v->data + 8, v->data + 9, v->data + 10, v->data + 11,
             v->data + 12, v->data + 13, v->data + 14, v->data + 15 );
      return true;
    }
    case Var::Enum:
    {
      if ( !value[ 0 ] )
      {
        return false;
      }
      
      const class Enum* enumType = GetEnum();
      
      if ( enumType->TypeIsSigned() )
      {
        switch ( enumType->TypeSize() )
        {
        case 1:
          return enumType->GetValueFromString( value, reinterpret_cast< int8_t* >( varData ) );
        case 2:
          return enumType->GetValueFromString( value, reinterpret_cast< int16_t* >( varData ) );
        case 4:
          return enumType->GetValueFromString( value, reinterpret_cast< int32_t* >( varData ) );
        case 8:
          return enumType->GetValueFromString( value, reinterpret_cast< int64_t* >( varData ) );
        default:
          AE_FAIL();
          return false;
        }
      }
      else
      {
        switch ( enumType->TypeSize() )
        {
        case 1:
          return enumType->GetValueFromString( value, reinterpret_cast< uint8_t* >( varData ) );
        case 2:
          return enumType->GetValueFromString( value, reinterpret_cast< uint16_t* >( varData ) );
        case 4:
          return enumType->GetValueFromString( value, reinterpret_cast< uint32_t* >( varData ) );
        case 8:
          return enumType->GetValueFromString( value, reinterpret_cast< uint64_t* >( varData ) );
        default:
          AE_FAIL();
          return false;
        }
      }
      return false;
    }
    case Var::Ref:
    {
      const ae::Type* refType = GetRefType();
      AE_ASSERT_MSG( getObjectPointerFromString, "Must provide mapping function for reference types when calling SetObjectValueFromString" );
      
      class ae::Object* obj = nullptr;
      if ( getObjectPointerFromString( refType, value, &obj ) )
      {
        if ( obj )
        {
          const ae::Type* objType = ae::GetTypeFromObject( obj );
          AE_ASSERT( objType );
          AE_ASSERT_MSG( objType->IsType( refType ), "SetObjectValueFromString for var '#::#' returned object with wrong type '#'", m_owner->GetName(), GetName(), objType->GetName() );
        }
        class ae::Object** varPtr = reinterpret_cast< class ae::Object** >( varData );
        *varPtr = obj;
        return true;
      }
      return false;
    }
  }
  
  return false;
}

bool ae::Var::SetObjectValue( ae::Object* obj, const ae::Object* value ) const
{
  AE_ASSERT( m_type == Ref );
  
  if ( !obj )
  {
    return false;
  }
  
  const ae::Type* objType = ae::GetTypeFromObject( obj );
  AE_ASSERT( objType );
  AE_ASSERT_MSG( objType->IsType( m_owner ), "Attempting to set var on '#' with unrelated type '#'", objType->GetName(), m_owner->GetName() );
  
  if ( !value )
  {
    memset( (uint8_t*)obj + m_offset, 0, m_size );
    return true;
  }
  
  const ae::Type* refType = GetRefType();
  const ae::Type* valueType = ae::GetTypeFromObject( value );
  AE_ASSERT( valueType );
  AE_ASSERT_MSG( valueType->IsType( refType ), "Attempting to set ref type '#' with unrelated type '#'", refType->GetName(), valueType->GetName() );
  
  uint8_t* target = (uint8_t*)obj + m_offset;
  const ae::Object*& varData = *reinterpret_cast< const ae::Object** >( target );
  varData = value;
  
  return true;
}

const ae::Var* ae::Type::GetVarByName( const char* name ) const
{
  int32_t i = m_vars.FindFn( [name]( auto&& v )
  {
    return v.m_name == name;
  } );
  return ( i >= 0 ) ? &m_vars[ i ] : nullptr;
}

const ae::Type* ae::Type::GetParentType() const
{
  return GetTypeByName( m_parent.c_str() );
}

bool ae::Type::IsType( const Type* otherType ) const
{
  AE_ASSERT( otherType );
  for ( const ae::Type* baseType = this; baseType; baseType = baseType->GetParentType() )
  {
    if ( baseType == otherType )
    {
      return true;
    }
  }
  return false;
}

const class ae::Enum* ae::Var::GetEnum() const
{
  if ( !m_enum )
  {
    if ( m_type != Var::Enum )
    {
      return nullptr;
    }
    m_enum = ae::GetEnum( m_typeName.c_str() );
  }
  return m_enum;
}

const ae::Type* ae::Var::GetRefType() const
{
  if ( m_refTypeId == ae::kInvalidTypeId )
  {
    return nullptr;
  }
  const ae::Type* type = GetTypeById( m_refTypeId );
  AE_ASSERT( type );
  return type;
}

//------------------------------------------------------------------------------
// Internal ae::Object functions
//------------------------------------------------------------------------------
ae::TypeId ae::GetObjectTypeId( const ae::Object* obj )
{
  return obj ? obj->_metaTypeId : ae::kInvalidTypeId;
}

ae::TypeId ae::GetTypeIdFromName( const char* name )
{
  // @TODO: Look into https://en.cppreference.com/w/cpp/types/type_info/hash_code
  return name[ 0 ] ? ae::Hash().HashString( name ).Get() : ae::kInvalidTypeId;
}

std::map< ae::Str32, ae::Type* >& ae::_GetTypeNameMap()
{
  static std::map< ae::Str32, Type* > s_map;
  return s_map;
}

std::map< ae::TypeId, ae::Type* >& ae::_GetTypeIdMap()
{
  static std::map< ae::TypeId, Type* > s_map;
  return s_map;
}

std::vector< ae::Type* >& ae::_GetTypes()
{
  static std::vector< ae::Type* > s_vec;
  return s_vec;
}

int32_t ae::Enum::GetValueByIndex( int32_t index ) const { return m_enumValueToName.GetKey( index ); }
std::string ae::Enum::GetNameByIndex( int32_t index ) const { return m_enumValueToName.GetValue( index ); }
uint32_t ae::Enum::Length() const { return m_enumValueToName.Length(); }

ae::Enum::Enum( const char* name, uint32_t size, bool isSigned ) :
  m_name( name ),
  m_size( size ),
  m_isSigned( isSigned )
{}

void ae::Enum::m_AddValue( const char* name, int32_t value )
{
  m_enumValueToName.Set( value, name );
  m_enumNameToValue.Set( name, value );
}

ae::Enum* ae::Enum::s_Get( const char* enumName, bool create, uint32_t size, bool isSigned )
{
  static ae::Map< std::string, Enum* > enums = AE_ALLOC_TAG_META;
  if ( create )
  {
    AE_ASSERT( !enums.TryGet( enumName ) );
    return enums.Set( enumName, ae::New< Enum >( AE_ALLOC_TAG_META, enumName, size, isSigned ) );
  }
  else
  {
    Enum* metaEnum = enums.Get( enumName, nullptr );
    AE_ASSERT_MSG( metaEnum, "Could not find meta registered Enum named '#'", enumName );
    return metaEnum;
  }
}

// @TODO: Replace return type with a dynamic ae::Str
std::string ae::Var::GetObjectValueAsString( const ae::Object* obj, std::function< std::string( const ae::Object* ) > getStringFromObjectPointer ) const
{
  if ( !obj )
  {
    return "";
  }
  
  // @TODO: Add debug safety check to make sure 'this' Var belongs to 'obj' ae::Type
  
  const void* varData = reinterpret_cast< const uint8_t* >( obj ) + m_offset;
  
  switch ( m_type )
  {
    case Var::String:
      switch ( m_size )
      {
        case 16:
          return reinterpret_cast< const ae::Str16* >( varData )->c_str();
        case 32:
          return reinterpret_cast< const ae::Str32* >( varData )->c_str();
        case 64:
          return reinterpret_cast< const ae::Str64* >( varData )->c_str();
        case 128:
          return reinterpret_cast< const ae::Str128* >( varData )->c_str();
        case 256:
          return reinterpret_cast< const ae::Str256* >( varData )->c_str();
        case 512:
          return reinterpret_cast< const ae::Str512* >( varData )->c_str();
        default:
          AE_FAIL_MSG( "Invalid string size '#'", m_size );
          return "";
      }
    case Var::UInt8:
      // Prevent char formatting
      return ae::Str32::Format( "#", (uint32_t)*reinterpret_cast< const uint8_t* >( varData ) ).c_str();
    case Var::UInt16:
      return ae::Str32::Format( "#", *reinterpret_cast< const uint16_t* >( varData ) ).c_str();
    case Var::UInt32:
      return ae::Str32::Format( "#", *reinterpret_cast< const uint32_t* >( varData ) ).c_str();
    case Var::Int8:
      // Prevent char formatting
      return ae::Str32::Format( "#", (int32_t)*reinterpret_cast< const int8_t* >( varData ) ).c_str();
    case Var::Int16:
      return ae::Str32::Format( "#", *reinterpret_cast< const int16_t* >( varData ) ).c_str();
    case Var::Int32:
      return ae::Str32::Format( "#", *reinterpret_cast< const int32_t* >( varData ) ).c_str();
    case Var::Bool:
      return ae::Str32::Format( "#", *reinterpret_cast< const bool* >( varData ) ).c_str();
    case Var::Float:
      return ae::Str32::Format( "#", *reinterpret_cast< const float* >( varData ) ).c_str();
    case Var::Matrix4:
      return ae::Str256::Format( "#", *reinterpret_cast< const ae::Matrix4* >( varData ) ).c_str();
    case Var::Enum:
    {
      const class Enum* enumType = GetEnum();
      int32_t value = 0;
      switch ( enumType->TypeSize() )
      {
        case 1: value = *reinterpret_cast< const int8_t* >( varData ); break;
        case 2: value = *reinterpret_cast< const int16_t* >( varData ); break;
        case 4: value = *reinterpret_cast< const int32_t* >( varData ); break;
        case 8: value = *reinterpret_cast< const int64_t* >( varData ); break;
        default: AE_FAIL();
      }
      return enumType->GetNameByValue( value );
    }
    case Var::Ref:
    {
      AE_ASSERT_MSG( getStringFromObjectPointer, "Must provide mapping function for reference types when calling GetObjectValueAsString" );
      const ae::Object* obj = *reinterpret_cast< const ae::Object* const * >( varData );
      return getStringFromObjectPointer( obj ).c_str();
    }
  }
  
  return "";
}

ae::TypeId ae::Type::GetId() const { return m_id; }
bool ae::Type::HasProperty( const char* prop ) const { return m_props.TryGet( prop ) != nullptr; }
uint32_t ae::Type::GetPropertyCount() const { return m_props.Length(); }
const char* ae::Type::GetPropertyName( uint32_t propIndex ) const { return m_props.GetKey( propIndex ).c_str(); }
uint32_t ae::Type::GetPropertyValueCount( uint32_t propIndex ) const { return m_props.GetValue( propIndex ).Length(); }
uint32_t ae::Type::GetPropertyValueCount( const char* propName ) const { auto* props = m_props.TryGet( propName ); return props ? props->Length() : 0; }
const char* ae::Type::GetPropertyValue( uint32_t propIndex, uint32_t valueIndex ) const { return m_props.GetValue( propIndex )[ valueIndex ].c_str(); }
const char* ae::Type::GetPropertyValue( const char* propName, uint32_t valueIndex ) const { return m_props.Get( propName )[ valueIndex ].c_str(); }
uint32_t ae::Type::GetVarCount() const { return m_vars.Length(); }
const ae::Var* ae::Type::GetVarByIndex( uint32_t i ) const { return &m_vars[ i ]; }
uint32_t ae::Type::GetSize() const { return m_size; }
uint32_t ae::Type::GetAlignment() const { return m_align; }
const char* ae::Type::GetName() const { return m_name.c_str(); }
bool ae::Type::HasNew() const { return m_placementNew; }
bool ae::Type::IsAbstract() const { return m_isAbstract; }
bool ae::Type::IsPolymorphic() const { return m_isPolymorphic; }
bool ae::Type::IsDefaultConstructible() const { return m_isDefaultConstructible; }
const char* ae::Type::GetParentTypeName() const { return m_parent.c_str(); }

void ae::Type::m_AddProp( const char* prop, const char* value )
{
  auto* props = m_props.TryGet( prop );
  if ( !props )
  {
    props = &m_props.Set( prop, AE_ALLOC_TAG_META );
  }
  if ( value && value[ 0 ] ) // 'm_props' will have an empty array for properties with no value specified
  {
    props->Append( value );
  }
}

void ae::Type::m_AddVar( const Var& var )
{
  m_vars.Append( var );
  std::sort( m_vars.Begin(), m_vars.End(), []( const auto& a, const auto& b )
  {
    return a.GetOffset() < b.GetOffset();
  } );
}

//------------------------------------------------------------------------------
// Warnings
//------------------------------------------------------------------------------
#if _AE_APPLE_
  // Pop deprecated OpenGL function warning disable
  #pragma clang diagnostic pop
#endif

#endif // AE_MAIN
