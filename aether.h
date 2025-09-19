//------------------------------------------------------------------------------
//! aether.h
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and /or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Usage:
// Use this module by defining AE_MAIN once in your project above an
// included 'aether.h'. It's required that AE_MAIN is defined in an Objective-C
// '.mm' file on Apple platforms.
//
// Linking:
// Optionally you can define AE_USE_MODULES so linking system dependencies such
// as OpenGL will be handled for you.
//
// Recommendations:
// For bigger projects it's worth defining AE_MAIN in it's own module to limit
// the number of dependencies brought into your own code. For instance
// 'Windows.h' is included with AE_MAIN and this can easily cause naming
// conflicts with gameplay/engine code. The following example could be compiled
// into a single file/module and linked with the application.
// Usage inside of a cpp/mm file could be something like:
#if 0 // ae.cpp/ae.mm start

#define AE_MAIN
#define AE_USE_MODULES // C++ Modules (optional)
#include "aether.h"

#endif // ae.cpp/ae.mm end

//------------------------------------------------------------------------------
// AE_AETHER_H
//------------------------------------------------------------------------------
#ifndef AE_AETHER_H
#define AE_AETHER_H

//------------------------------------------------------------------------------
// AE_CONFIG_FILE define
//------------------------------------------------------------------------------
//! The path to a user defined configuration header file, something like
//! AE_CONFIG_FILE="aeConfig.h". Should contain defines such as
//! AE_VEC3_CLASS_CONFIG, AE_MAX_META_TYPES_CONFIG, etc (any AE_*_CONFIG's
//! below), which can be used to define custom conversion functions, or override
//! default limits. These defines MUST be consistent for all translation units
//! in a project. It's advised that an AE_CONFIG_FILE is provided globally as a
//! compiler definition to limit the opportunity for any configuration issues.
//! If AE_CONFIG_FILE is not defined, the default configuration will be used.
//! Be aware that no aether specific defines are provided in advance of
//! including this file, so it's not possible to check for _AE_DEBUG_ etc.
#ifdef AE_CONFIG_FILE
	#include AE_CONFIG_FILE
#endif

//------------------------------------------------------------------------------
// _AE_DEBUG_ define
//------------------------------------------------------------------------------
// Enables extra checks, asserts, and memory patterns for allocations, etc. It
// is automatically enabled based on platform specific flags. This is okay to
// manually enable for optimized builds by defining _AE_DEBUG_ in your
// AE_CONFIG_FILE. It is not recommended for public release builds.
//------------------------------------------------------------------------------
#ifndef _AE_DEBUG_
	#if defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__))
		#define _AE_DEBUG_ 1
	#else
		#define _AE_DEBUG_ 0
	#endif
#endif

//------------------------------------------------------------------------------
// AE_MEMORY_CHECKS define
//------------------------------------------------------------------------------
//! If you define AE_MEMORY_CHECKS=1, all allocations through aether will be
//! tracked and checked for leaks, double-frees, etc. If you use
//! ae::SetGlobalAllocator() the usefulness of AE_MEMORY_CHECKS will be very
//! limited, so it might be worth temporarily disabling your custom allocator to
//! get the full benefits of AE_MEMORY_CHECKS. AE_MEMORY_CHECKS is a heavy
//! diagnostic tool and may have a large performance impact. AE_MEMORY_CHECKS
//! must be defined for all files that include aether.h (using AE_CONFIG_FILE is
//! one way to do this).
//------------------------------------------------------------------------------
#ifndef AE_MEMORY_CHECKS
	#define AE_MEMORY_CHECKS 0
#endif

//------------------------------------------------------------------------------
// AE_ENABLE_SOURCE_INFO define
//------------------------------------------------------------------------------
//! Includes additional info about source files and line numbers in logs and
//! asserts. This could be useful to enable in dev builds to get extra
//! information about logged events. AE_ENABLE_SOURCE_INFO must be defined for
//! all files that include aether.h (using AE_CONFIG_FILE is one way to do
//! this).
#ifndef AE_ENABLE_SOURCE_INFO
	#define AE_ENABLE_SOURCE_INFO _AE_DEBUG_
#endif

//------------------------------------------------------------------------------
// AE_MAX_SCRATCH_BYTES_CONFIG define
//------------------------------------------------------------------------------
//! The cumulative maximum bytes of all currently allocated ae::ScratchBuffers.
//! Note that ae::ScratchBuffer memory is always released in the reverse order
//! that it was allocated, so this limit should accommodate the worst case.
//------------------------------------------------------------------------------
#ifndef AE_MAX_SCRATCH_BYTES_CONFIG
	#define AE_MAX_SCRATCH_BYTES_CONFIG ( 4 * 1024 *1024 )
#endif

//------------------------------------------------------------------------------
// AE_LOG_FUNCTION_CONFIG define
//------------------------------------------------------------------------------
//! Externally overridable for log redirection. This should be defined with the
//! name of a function whose signature matches ae::LogFn.
//------------------------------------------------------------------------------
#ifndef AE_LOG_FUNCTION_CONFIG
	#define AE_LOG_FUNCTION_CONFIG ae::_LogImpl
#endif

//------------------------------------------------------------------------------
// AE_FNV1A_*_CONFIG defines
//------------------------------------------------------------------------------
#ifndef AE_HASH32_FNV1A_OFFSET_BASIS_CONFIG
	#define AE_HASH32_FNV1A_OFFSET_BASIS_CONFIG 0x811c9dc5
#endif
#ifndef AE_HASH32_FNV1A_PRIME_CONFIG
	#define AE_HASH32_FNV1A_PRIME_CONFIG 0x1000193
#endif
#ifndef AE_HASH64_FNV1A_OFFSET_BASIS_CONFIG
	#define AE_HASH64_FNV1A_OFFSET_BASIS_CONFIG 0xCBF29CE484222325ull
#endif
#ifndef AE_HASH64_FNV1A_PRIME_CONFIG
	#define AE_HASH64_FNV1A_PRIME_CONFIG 0x100000001B3ull
#endif

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
// Warnings
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
	#define AE_POP_WARNINGS
	#pragma warning( push )
	#pragma warning( disable : 4018 ) // signed/unsigned mismatch
	#pragma warning( disable : 4244 ) // conversion from 'float' to 'int32_t'
	#pragma warning( disable : 4267 ) // conversion from 'size_t' to 'uint32_t'
	#pragma warning( disable : 4800 )
#elif _AE_APPLE_
	#define AE_POP_WARNINGS
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

//------------------------------------------------------------------------------
// Macro helpers
//------------------------------------------------------------------------------
//! Returns the number of arguments passed to this macro
#define AE_VA_ARGS_COUNT(...) AE_EVAL(AE_VA_ARGS_COUNT_IMPL(__VA_ARGS__,9,8,7,6,5,4,3,2,1,))
//! Combines each argument into a single token
#define AE_GLUE(...) AE_GLUE_IMPL(AE_GLUE_,AE_VA_ARGS_COUNT(__VA_ARGS__))(__VA_ARGS__)
//! Combines each argument into a single token, but arguments are separated by
//! '::' (double colons).
#define AE_GLUE_TYPE(...) AE_GLUE(AE_GLUE_TYPE_,AE_VA_ARGS_COUNT(__VA_ARGS__))(__VA_ARGS__)
//! Combines each argument into a single token with an underscore between each
//! argument. This is useful for creating unique names for variables, functions,
//! etc. from a list of arguments.
#define AE_GLUE_UNDERSCORE(...) AE_GLUE(AE_GLUE_UNDERSCORE_,AE_VA_ARGS_COUNT(__VA_ARGS__))(__VA_ARGS__)
//! Converts the given argument to a string. Useful for converting the result of
//! another macro invocation into a string.
#define AE_STRINGIFY(S) AE_STRINGIFY_IMPL(S)
//! Returns the Nth element of __VA_ARGS__
#define AE_GET_ELEM(N, ...) AE_GLUE(AE_GET_ELEM_, N)(__VA_ARGS__)
//! Returns the last argument passed to this macro
#define AE_GET_LAST(...) AE_GET_ELEM(AE_VA_ARGS_COUNT(__VA_ARGS__), _, __VA_ARGS__ ,,,,,,,,,,,) // Get last argument - placeholder decrements by one
//! Returns all of the arguments passed to this macro except the last one
#define AE_DROP_LAST(...) AE_GLUE(AE_DROP_LAST_,AE_VA_ARGS_COUNT(__VA_ARGS__))(__VA_ARGS__)

//------------------------------------------------------------------------------
// System Headers
//------------------------------------------------------------------------------
#include <algorithm>
#include <array> // @TODO: Remove. For _GetTypeName().
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <thread> // @TODO: Remove. For Globals::allocatorThread.
#include <type_traits>
#include <typeinfo>
#if AE_MEMORY_CHECKS
	#include <unordered_map>
#endif
#include <utility>
#include <vector> // @TODO: Remove. For _RegisterEnum.

//------------------------------------------------------------------------------
// Platform headers
//------------------------------------------------------------------------------
#if _AE_APPLE_
	#ifdef __aarch64__
		#include <arm_neon.h>
	#else
		#include <x86intrin.h>
	#endif
#elif _AE_WINDOWS_
	#include <intrin.h>
#elif _AE_EMSCRIPTEN_
	#include <emscripten.h>
	#include <emscripten/html5.h>
	#include <webgl/webgl1.h> // For Emscripten WebGL API headers (see also webgl/webgl1_ext.h and webgl/webgl2.h)
#endif
#if !_AE_WINDOWS_
	#include <cxxabi.h>
#endif

//------------------------------------------------------------------------------
// Platform Utils
//------------------------------------------------------------------------------
#ifndef AE_BREAK
	#if _AE_WINDOWS_
		#define AE_BREAK() __debugbreak()
	#elif _AE_APPLE_
		#define AE_BREAK() __builtin_trap()
	#elif _AE_EMSCRIPTEN_
		#define AE_BREAK() assert( 0 )
	#elif defined( __aarch64__ )
		#define AE_BREAK() asm( "brk #0" )
	#else
		#define AE_BREAK() asm( "int $3" )
	#endif
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

#if _AE_WINDOWS_
	#define AE_PACK( ... ) __pragma( pack(push, 1) ) __VA_ARGS__ __pragma( pack(pop))
#else
	#define AE_PACK( ... ) __VA_ARGS__ __attribute__((__packed__))
#endif

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
template< typename T, int N > char( &countof_helper( T(&)[ N ] ) )[ N ];
#define countof( _x ) ( (uint32_t)sizeof( countof_helper( _x ) ) ) // @TODO: AE_COUNT_OF
#define AE_CALL_CONST( _tx, _x, _tfn, _fn ) const_cast< _tfn* >( const_cast< const _tx* >( _x )->_fn() );
#define _AE_STATIC_STORAGE template< uint32_t NN = N, typename = std::enable_if_t< NN != 0 > >
#define _AE_DYNAMIC_STORAGE template< uint32_t NN = N, typename = std::enable_if_t< NN == 0 > >
#define _AE_FIXED_POOL template< bool P = Paged, typename = std::enable_if_t< !P > >
#define _AE_PAGED_POOL template< bool P = Paged, typename = std::enable_if_t< P > >
#if !_AE_WINDOWS_
	#define AE_DISABLE_INVALID_OFFSET_WARNING _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Winvalid-offsetof\"")
	#define AE_ENABLE_INVALID_OFFSET_WARNING _Pragma("GCC diagnostic pop")
#else
	#define AE_DISABLE_INVALID_OFFSET_WARNING
	#define AE_ENABLE_INVALID_OFFSET_WARNING
#endif
#define AE_DISABLE_COPY_ASSIGNMENT( _t ) _t( const _t& ) = delete; _t& operator=( const _t& ) = delete

namespace ae {

//------------------------------------------------------------------------------
//! \defgroup Platform
//! @{
//------------------------------------------------------------------------------
//! Returns the process ID on Windows, OSX, and Linux. Returns 0 with Emscripten builds.
uint32_t GetPID();
//! Returns the number of virtual cores available.
uint32_t GetMaxConcurrentThreads();
//! Returns true if attached to Visual Studio or Xcode.
bool IsDebuggerAttached();
//! Returns the name of the given class or basic type from an instance. Note
//! that this does not return the name of the derived class if the instance is
//! a base class (get the ae::ClassType of an ae::Object in that case).
template< typename T > const char* GetTypeName();
//! Returns the name of the given class or basic type from an instance. Note
//! that this does not return the name of the derived class if the instance is
//! a base class (get the ae::ClassType of an ae::Object in that case).
template< typename T > const char* GetTypeName( const T& );
//! Returns a monotonically increasing time in seconds, useful for calculating high precision deltas. Time '0' is undefined.
double GetTime();
//! Shows a generic message box
void ShowMessage( const char* msg );
//! Sets the systems clipboard text so it can be pasted into other applications.
void SetClipboardText( const char* text );
//! Gets the systems clipboard text.
std::string GetClipboardText();
//! @} End Platform defgroup

//------------------------------------------------------------------------------
// Tags @TODO: Remove this! All tags should be user specified
//------------------------------------------------------------------------------
using Tag = std::string; // @TODO: Fixed length string
#define AE_ALLOC_TAG_RENDER ae::Tag( "aeGraphics" )
#define AE_ALLOC_TAG_AUDIO ae::Tag( "aeAudio" )
#define AE_ALLOC_TAG_TERRAIN ae::Tag( "aeTerrain" )
#define AE_ALLOC_TAG_NET ae::Tag( "aeNet" )
#define AE_ALLOC_TAG_HOTSPOT ae::Tag( "aeHotSpot" )
#define AE_ALLOC_TAG_MESH ae::Tag( "aeMesh" )
#define AE_ALLOC_TAG_FIXME ae::Tag( "aeFixMe" )
#define AE_ALLOC_TAG_FILE ae::Tag( "aeFile" )

//------------------------------------------------------------------------------
//! \defgroup Allocation
//! Allocation utilities.
//! By default aether-game-utils uses system allocations (malloc / free). The
//! default allocator is thread safe. If this is not okay for your use case,
//! it's advised that you implement your own ae::Allocator with dlmalloc or
//! similar and then call ae::SetGlobalAllocator() with your allocator at
//! program start.
//! @{
//------------------------------------------------------------------------------
//! ae::Allocator base class
//! Inherit from this to manage how allocations are handled. ALL ALLOCATIONS are
//! made through the ae::Allocator interface. This allows for custom memory
//! management, such as pooling, tracking, etc. See ae::_DefaultAllocator for a
//! simple example of how to implement an allocator.
//------------------------------------------------------------------------------
class Allocator
{
public:
	virtual ~Allocator();
	//! Should return 'bytes' with minimum alignment of 'alignment'. Optionally, a
	//! tag should be used to select a pool of memory, or for diagnostics/debugging.
	virtual void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) = 0;
	//! Should attempt to expand or contract allocations made with Allocate() to
	//! match size 'bytes'. On failure this function should return nullptr.
	virtual void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) = 0;
	//! Free memory allocated with ae::Allocator::Allocate() or ae::Allocator::Reallocate().
	virtual void Free( void* data ) = 0;
	//! Used for safety checks.
	virtual bool IsThreadSafe() const = 0;
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
//! Allocates and constructs an array of 'count' elements of type T. An ae::Tag
//! must be specified and should represent the allocation type. All 'args' are
//! passed to the constructor of T. All arrays allocated with this function
//! should be freed with ae::Delete(). Uses ae::GetGlobalAllocator() and
//! ae::Allocator::Allocate() internally.
template< typename T, typename ... Args > T* NewArray( ae::Tag tag, uint32_t count, Args&& ... args );
//! Allocates and constructs a single element of type T. an ae::Tag must be specified
//! and should represent the allocation type. All 'args' are passed to the constructor
//! of T. All allocations should be freed with ae::Delete(). Uses ae::GetGlobalAllocator()
//! and ae::Allocator::Allocate() internally.
template< typename T, typename ... Args > T* New( ae::Tag tag, Args&& ... args );
//! Should be called to destruct and free all allocations made with ae::New()
//! and ae::NewArray(). Uses ae::GetGlobalAllocator() and ae::Allocator::Free()
//! internally.
template< typename T > void Delete( T* obj );
// C style allocations
void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment );
void* Reallocate( void* data, uint32_t bytes, uint32_t alignment );
void Free( void* data );
//! @} End Allocation defgroup

//------------------------------------------------------------------------------
// ae::Scratch< T > class
//! Can be used for scoped allocations within a single frame. Because this uses
//! a stack internally it can be used to make many cheap allocations, while
//! avoiding memory fragmentation. Up to kMaxScratchSize bytes may be allocated
//! at a time. Allocated objects will have their constructors and destructors
//! called in ae::Scratch() and ~ae::Scratch respectively.
//------------------------------------------------------------------------------
template< typename T >
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

	//! The max cumulative size of the internal scratch stack
	static const uint32_t kMaxScratchSize = AE_MAX_SCRATCH_BYTES_CONFIG;

private:
	T* m_data;
	uint32_t m_size;
	uint32_t m_prevOffsetCheck;
};

//------------------------------------------------------------------------------
//! \defgroup Math
//! @{
//------------------------------------------------------------------------------
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = 0.5f * PI;
constexpr float QUARTER_PI = 0.25f * PI;
constexpr float Pi = 3.14159265358979323846f;
constexpr float TwoPi = 2.0f * PI;
constexpr float HalfPi = 0.5f * PI;
constexpr float QuarterPi = 0.25f * PI;

enum class Axis { None, X, Y, Z, NegativeX, NegativeY, NegativeZ };

//------------------------------------------------------------------------------
// Standard math operations
//------------------------------------------------------------------------------
inline float Pow( float x, float e );
inline float Cos( float x );
inline float Sin( float x );
inline float Tan( float x );
inline float Acos( float x );
inline float Asin( float x );
inline float Atan( float x );
inline float Atan2( float y, float x );

inline float Sqrt( float x );

inline uint32_t Mod( uint32_t i, uint32_t n );
inline int Mod( int32_t i, int32_t n );
inline float Mod( float f, float n );

inline int32_t Ceil( float f );
inline int32_t Floor( float f );
inline int32_t Round( float f );

inline float Abs( float x );
inline int32_t Abs( int32_t x );

constexpr uint32_t NextPowerOfTwo( uint32_t x );

//------------------------------------------------------------------------------
// Range functions
//------------------------------------------------------------------------------
template< typename T0, typename T1, typename... TTT >
constexpr auto Min( const T0& v0, const T1& v1, const TTT&... tail );

template< typename T0, typename T1, typename... TTT >
constexpr auto Max( const T0& v0, const T1& v1, const TTT&... tail );

template< typename T > inline T Clip( T x, T min, T max );
inline float Clip01( float x );

//------------------------------------------------------------------------------
// Interpolation
//------------------------------------------------------------------------------
template< typename T0, typename T1 > T0 Lerp( T0 start, T0 end, T1 t );
inline float AngleDifference( float end, float start );
inline float LerpAngle( float start, float end, float t );
inline float Delerp( float start, float end, float value );
inline float Delerp01( float start, float end, float value );
template< typename T > T DtLerp( T start, float snappiness, float dt, T end );
template< typename T > T DtSlerp( T start, float snappiness, float dt, T end );
inline float DtLerpAngle( float start, float snappiness, float dt, float end );
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
// Forward declare to avoid https://stackoverflow.com/questions/7774188/explicit-specialization-after-instantiation
template<> constexpr float MaxValue< float >();
template<> constexpr float MinValue< float >();
template<> constexpr double MaxValue< double >();
template<> constexpr double MinValue< double >();

//------------------------------------------------------------------------------
// ae::Random functions
//------------------------------------------------------------------------------
extern thread_local uint64_t _randomSeed;
void RandomSeed();
inline float Random01( uint64_t* seed = &_randomSeed );
inline bool RandomBool( uint64_t* seed = &_randomSeed );
inline int32_t Random( int32_t minInclusive, int32_t maxExclusive, uint64_t* seed = &_randomSeed );
inline float Random( float min, float max, uint64_t* seed = &_randomSeed );

template< typename T >
class RandomValue
{
public:
	RandomValue( uint64_t* seed = &_randomSeed ) : m_seed( seed ) {}
	RandomValue( T min, T max, uint64_t* seed = &_randomSeed );
	RandomValue( T value, uint64_t* seed = &_randomSeed );
	
	void SetMin( T min );
	void SetMax( T max );
	
	T GetMin() const;
	T GetMax() const;
	
	T Get() const;
	operator T() const;
	
private:
	uint64_t* m_seed = nullptr;
	T m_min = T();
	T m_max = T();
};

//------------------------------------------------------------------------------
// Vector math utilities
//------------------------------------------------------------------------------
struct Vec2;
struct Vec3;
struct Vec4;
class Matrix4;
class Quaternion;

//------------------------------------------------------------------------------
// ae::Vec2 shared member functions
// ae::Vec3 shared member functions
// ae::Vec4 shared member functions
//------------------------------------------------------------------------------
// @NOTE: Vec2 Vec3 and Vec4 share these functions. They act on each component
// of the vector, so in the case of Vec4 a dot product is implemented as
// (a.x*b.x)+(a.y*b.y)+(a.z*b.z)+(a.w*b.w).
template< typename T >
struct VecT
{
	VecT() = default;
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
	T TrimCopy( float length ) const;

	bool IsNAN() const;
};

#if _AE_WINDOWS_
	#pragma warning(disable:26495) // Vecs are left uninitialized for performance
#endif

//------------------------------------------------------------------------------
// ae::Vec2 struct
//------------------------------------------------------------------------------
struct AE_ALIGN( 8 ) Vec2 : public VecT< Vec2 >
{
	Vec2() = default; //!< Trivial default constructor for performance of vertex arrays etc
	Vec2( const Vec2& ) = default;
	explicit Vec2( float v );
	Vec2( float x, float y );
	explicit Vec2( const float* xy );
	explicit Vec2( struct Int2 i2 );
	static Vec2 FromAngle( float angle );

	struct Int2 NearestCopy() const;
	struct Int2 FloorCopy() const;
	struct Int2 CeilCopy() const;
	
	Vec2 RotateCopy( float rotation ) const;
	float GetAngle() const;
	Vec2 DtSlerp( const Vec2& end, float snappiness, float dt, float epsilon = 0.0001f ) const;
	Vec2 Slerp( const Vec2& end, float t, float epsilon = 0.0001f ) const;
	static Vec2 Reflect( Vec2 v, Vec2 n );

	//! Define conversion functions etc for ae::Vec2. See AE_CONFIG_FILE for more info.
#ifdef AE_VEC2_CLASS_CONFIG
	AE_VEC2_CLASS_CONFIG
#endif

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
struct AE_ALIGN( 16 ) Vec3 : public VecT< Vec3 >
{
	Vec3() = default; //!< Trivial constructor for performance of vertex arrays etc
	explicit Vec3( float v );
	Vec3( float x, float y, float z );
	explicit Vec3( const float* xyz );
	explicit Vec3( struct Int3 i3 );
	Vec3( Vec2 xy, float z );
	explicit Vec3( Vec2 xy );
	explicit operator Vec2() const;
	static Vec3 XZY( Vec2 xz, float y );
	
	void SetXY( Vec2 xy );
	void SetXZ( Vec2 xz );
	Vec2 GetXY() const;
	Vec2 GetXZ() const;
	
	struct Int3 NearestCopy() const;
	struct Int3 FloorCopy() const;
	struct Int3 CeilCopy() const;
	
	void AddRotationXY( float rotation ); // @TODO: Support Y up
	Vec3 AddRotationXYCopy( float rotation ) const;
	float GetAngleBetween( const Vec3& v, float epsilon = 0.0001f ) const;
	Vec3 RotateCopy( Vec3 axis, float angle ) const;
	
	Vec3 Lerp( const Vec3& end, float t ) const;
	Vec3 DtSlerp( const Vec3& end, float snappiness, float dt, float epsilon = 0.0001f ) const;
	Vec3 Slerp( const Vec3& end, float t, float epsilon = 0.0001f ) const;
	
	static Vec3 Cross( const Vec3& v0, const Vec3& v1 );
	Vec3 Cross( const Vec3& v ) const;
	void ZeroAxis( Vec3 axis ); // Zero component along arbitrary axis (ie vec dot axis == 0)
	void ZeroDirection( Vec3 direction ); // Zero component along positive half of axis (ie vec dot dir > 0)
	Vec3 ZeroAxisCopy( Vec3 axis ) const; // Zero component along arbitrary axis (ie vec dot axis == 0)
	Vec3 ZeroDirectionCopy( Vec3 direction ) const; // Zero component along positive half of axis (ie vec dot dir > 0)

	static Vec3 ProjectPoint( const class Matrix4& projection, Vec3 p );

	//! Define conversion functions etc for ae::Vec3. See AE_CONFIG_FILE for more info.
#ifdef AE_VEC3_CLASS_CONFIG
	AE_VEC3_CLASS_CONFIG
#endif
	
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
struct AE_ALIGN( 16 ) Vec4 : public VecT< Vec4 >
{
	Vec4() = default; //!< Trivial default constructor for performance of vertex arrays etc
	Vec4( const Vec4& ) = default;
	explicit Vec4( float f );
	explicit Vec4( float xyz, float w );
	Vec4( float x, float y, float z, float w );
	Vec4( Vec3 xyz, float w );
	Vec4( Vec2 xy, float z, float w );
	Vec4( Vec2 xy, Vec2 zw );
	explicit operator Vec2() const;
	explicit operator Vec3() const;
	Vec4( const float* xyz, float w );
	explicit Vec4( const float* xyzw );
	
	void SetXY( Vec2 xy );
	void SetXZ( Vec2 xz );
	void SetZW( Vec2 zw );
	void SetXYZ( Vec3 xyz );
	Vec2 GetXY() const;
	Vec2 GetXZ() const;
	Vec2 GetZW() const;
	Vec3 GetXYZ() const;

	//! Define conversion functions etc for ae::Vec4. See AE_CONFIG_FILE for more info.
#ifdef AE_VEC4_CLASS_CONFIG
	AE_VEC4_CLASS_CONFIG
#endif

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
//! Stored column major, ie. the elements of the basis vectors are stored
//! contiguously in memory.
class AE_ALIGN( 16 ) Matrix4
{
public:
	Matrix4() = default; //!< Trivial default constructor for performance
	Matrix4( const ae::Matrix4& ) = default;

	// Construction helpers
	static ae::Matrix4 Identity();
	static ae::Matrix4 Translation( float tx, float ty, float tz );
	static ae::Matrix4 Translation( const ae::Vec3& p );
	static ae::Matrix4 Rotation( ae::Quaternion q );
	static ae::Matrix4 Rotation( ae::Vec3 forward0, ae::Vec3 up0, ae::Vec3 forward1, ae::Vec3 up1 );
	static ae::Matrix4 RotationX( float angle );
	static ae::Matrix4 RotationY( float angle );
	static ae::Matrix4 RotationZ( float angle );
	static ae::Matrix4 Scaling( float s );
	static ae::Matrix4 Scaling( const ae::Vec3& s );
	static ae::Matrix4 Scaling( float sx, float sy, float sz );
	static ae::Matrix4 LocalToWorld( ae::Vec3 position, ae::Quaternion rotation, ae::Vec3 scale );
	static ae::Matrix4 WorldToView( ae::Vec3 position, ae::Vec3 forward, ae::Vec3 up );
	static ae::Matrix4 ViewToProjection( float fov, float aspectRatio, float nearPlane, float farPlane );

	// Set transformation properties
	ae::Matrix4& SetTranslation( float x, float y, float z );
	ae::Matrix4& SetTranslation( const ae::Vec3& t );
	ae::Matrix4& SetRotation( const ae::Quaternion& r );
	ae::Matrix4& SetScale( const ae::Vec3& s );
	ae::Matrix4& SetScale( float s );
	ae::Matrix4& SetTranspose();
	ae::Matrix4& SetInverse();
	ae::Matrix4& SetAxis( uint32_t column, const ae::Vec3& v );
	ae::Matrix4& SetAxis( uint32_t column, const ae::Vec4& v );
	ae::Matrix4& SetRow( uint32_t row, const ae::Vec3& v );
	ae::Matrix4& SetRow( uint32_t row, const ae::Vec4& v );

	// Get transformation properties
	ae::Vec3 GetTranslation() const;
	ae::Quaternion GetRotation() const;
	ae::Vec3 GetScale() const;
	ae::Matrix4 GetTranspose() const;
	ae::Matrix4 GetInverse() const; // @TODO: Handle non-invertible matrices in API
	ae::Matrix4 GetNormalMatrix() const;
	ae::Matrix4 GetScaleRemoved() const;
	ae::Vec3 GetAxis( uint32_t column ) const;
	ae::Vec4 GetColumn( uint32_t column ) const;
	ae::Vec4 GetRow( uint32_t row ) const;

	//! Matrix multiplication
	ae::Matrix4 operator*( const ae::Matrix4& m ) const;
	//! Matrix multiplication
	void operator*=( const ae::Matrix4& m );

	//! Vector multiplication
	ae::Vec4 operator*( const ae::Vec4& v ) const;
	//! Transform the given vector as if it had a w component of 1. No perspective
	//! divide is done on the result, instead use operator*(ae::Vec4) for
	//! perspective transformations.
	ae::Vec3 TransformPoint3x4( ae::Vec3 v ) const;
	//! Transform the given vector as if it had a w component of 0. No perspective
	//! divide is done on the result, instead use operator*(ae::Vec4) for
	//! perspective transformations.
	ae::Vec3 TransformVector3x4( ae::Vec3 v ) const;

	// Comparison
	bool operator==( const ae::Matrix4& o ) const { return memcmp( o.data, data, sizeof(data) ) == 0; }
	bool operator!=( const ae::Matrix4& o ) const { return !operator== ( o ); }
	bool IsNAN() const;

	//! Define conversion functions etc for ae::Matrix4. See AE_CONFIG_FILE for more info.
#ifdef AE_MAT4_CLASS_CONFIG
	AE_MAT4_CLASS_CONFIG
#endif

	union
	{
		ae::Vec4 columns[ 4 ];
		float data[ 16 ];
	};
};
inline std::ostream& operator << ( std::ostream& os, const ae::Matrix4& mat );

//------------------------------------------------------------------------------
// ae::Quaternion class
//------------------------------------------------------------------------------
class AE_ALIGN( 16 ) Quaternion
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
	Quaternion NormalizeCopy() const;
	bool operator==( const Quaternion& q ) const;
	bool operator!=( const Quaternion& q ) const;
	Quaternion& operator*= ( const Quaternion& q );
	Quaternion operator* ( const Quaternion& q ) const;
	Quaternion operator- () const;
	float Dot( const Quaternion& q ) const;
	Quaternion operator* ( float s ) const;
	void AddScaledVector( const Vec3& v, float s );
	void RotateByVector( const Vec3& v );
	void SetDirectionXY( const Vec3& v );
	Vec3 GetDirectionXY() const;
	void ZeroXY();
	void GetAxisAngle( Vec3* axis, float* angle ) const;
	void AddRotationXY( float rotation );
	Quaternion Nlerp( Quaternion end, float t ) const;
	Matrix4 GetTransformMatrix() const;
	Quaternion GetInverse() const;
	Quaternion& SetInverse();
	Vec3 Rotate( Vec3 v ) const;
	//! Returns a quaternion which rotates this quaternion to the given quaternion.
	//! ie: this = reference * relative
	Quaternion RelativeCopy( const Quaternion& reference ) const;
	//! Returns the twist and swing components of this quaternion around the
	//! given \p axis. The \p twistOut component is the rotation around the axis, and
	//! the \p swingOut component is the rotation around the axis' orthogonal plane.
	void GetTwistSwing( Vec3 axis, Quaternion* twistOut, Quaternion* swingOut ) const;

	//! Define conversion functions etc for ae::Quaternion. See AE_CONFIG_FILE for more info.
#ifdef AE_QUAT_CLASS_CONFIG
	AE_QUAT_CLASS_CONFIG
#endif
};
inline std::ostream& operator << ( std::ostream& os, const Quaternion& quat );

//------------------------------------------------------------------------------
// ae::Int2 shared member functions
// ae::Int3 shared member functions
//------------------------------------------------------------------------------
// @NOTE: Int2 and Int3 share these functions
template< typename T >
struct IntT
{
	IntT() = default;
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
template< typename T >
inline std::ostream& operator<<( std::ostream& os, const IntT< T >& v );

//------------------------------------------------------------------------------
// ae::Int2 class
//------------------------------------------------------------------------------
struct AE_ALIGN( 8 ) Int2 : public IntT< Int2 >
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
struct AE_ALIGN( 16 ) Int3 : public IntT< Int3 >
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
	Int2 GetXZ() const;
	void SetXY( Int2 xy );
	void SetXZ( Int2 xz );
};

//------------------------------------------------------------------------------
// ae::Sphere class
//------------------------------------------------------------------------------
class Sphere
{
public:
	Sphere() = default;
	Sphere( ae::Vec3 center, float radius ) : center( center ), radius( radius ) {}
	explicit Sphere( const class OBB& obb );
	void Expand( ae::Vec3 p );

	// @TODO: IntersectLine() which should have hit0Out and hit1Out
	bool IntersectRay( ae::Vec3 origin, ae::Vec3 direction, ae::Vec3* pOut = nullptr, float* tOut = nullptr ) const;
	bool IntersectTriangle( ae::Vec3 t0, ae::Vec3 t1, ae::Vec3 t2, ae::Vec3* outNearestIntersectionPoint ) const;
	ae::Vec3 GetNearestPointOnSurface( ae::Vec3 p, float* signedDistOut = nullptr ) const;

	ae::Vec3 center = ae::Vec3( 0.0f );
	float radius = 0.5f;
};

//------------------------------------------------------------------------------
// ae::Plane class
//! A plane in the form of ax+by+cz+d=0. This means that n<x,y,z>*d equals the
//! closest point on the plane to the origin.
//------------------------------------------------------------------------------
class Plane
{
public:
	Plane() = default;
	Plane( ae::Vec3 point, ae::Vec3 normal );
	Plane( ae::Vec4 pointNormal ); // @TODO: Maybe this should be removed, it's very easy to provide a vector where the sign of the w component is incorrect
	Plane( ae::Vec3 p0, ae::Vec3 p1, ae::Vec3 p2 );
	explicit operator Vec4() const;
	
	ae::Vec3 GetNormal() const;
	ae::Vec3 GetClosestPointToOrigin() const;

	bool IntersectLine( ae::Vec3 p, ae::Vec3 d, ae::Vec3* hitOut = nullptr, float* tOut = nullptr ) const; // @TODO: Handle ray hitting normal reverse
	bool IntersectRay( ae::Vec3 source, ae::Vec3 ray, ae::Vec3* hitOut = nullptr, float* tOut = nullptr ) const; // @TODO: Handle ray hitting normal reverse
	ae::Vec3 GetClosestPoint( ae::Vec3 pos, float* distanceOut = nullptr ) const;
	float GetSignedDistance( ae::Vec3 pos ) const;

private:
	ae::Vec4 m_plane;
};

//------------------------------------------------------------------------------
// ae::Line class
//------------------------------------------------------------------------------
class Line
{
public:
	Line() = default;
	Line( ae::Vec3 p0, ae::Vec3 p1 );

	ae::Vec3 GetClosest( ae::Vec3 p, float* distance = nullptr ) const;
	float GetDistance( ae::Vec3 p, ae::Vec3* closestOut = nullptr ) const;

private:
	ae::Vec3 m_p;
	ae::Vec3 m_n;
};

//------------------------------------------------------------------------------
// ae::LineSegment class
//------------------------------------------------------------------------------
class LineSegment
{
public:
	LineSegment() = default;
	LineSegment( ae::Vec3 p0, ae::Vec3 p1 );

	ae::Vec3 GetClosest( ae::Vec3 p, float* distance = nullptr ) const;
	float GetDistance( ae::Vec3 p, ae::Vec3* closestOut = nullptr ) const;
	ae::Vec3 GetStart() const;
	ae::Vec3 GetEnd() const;
	float GetLength() const;

private:
	ae::Vec3 m_p0;
	ae::Vec3 m_p1;
};

//------------------------------------------------------------------------------
// ae::Circle class
//------------------------------------------------------------------------------
class Circle
{
public:
	Circle() = default;
	Circle( ae::Vec2 point, float radius );

	static float GetArea( float radius );

	ae::Vec2 GetCenter() const { return m_point; }
	float GetRadius() const { return m_radius; }
	void SetCenter( ae::Vec2 point ) { m_point = point; }
	void SetRadius( float radius ) { m_radius = radius; }

	bool Intersect( const Circle& other, ae::Vec2* out ) const;
	ae::Vec2 GetRandomPoint( uint64_t* seed = &_randomSeed ) const;

private:
	ae::Vec2 m_point;
	float m_radius;
};

//------------------------------------------------------------------------------
// ae::Frustum class
//------------------------------------------------------------------------------
class Frustum
{
public:
	enum class Plane
	{
		Near,
		Far,
		Left,
		Right,
		Top,
		Bottom
	};
	
	explicit Frustum( ae::Matrix4 worldToProjection );
	bool Intersects( const ae::Sphere& sphere ) const;
	bool Intersects( ae::Vec3 point ) const;
	ae::Plane GetPlane( ae::Frustum::Plane plane ) const;
	
private:
	ae::Plane m_planes[ 6 ];
};

//------------------------------------------------------------------------------
// ae::AABB class
//------------------------------------------------------------------------------
class AABB
{
public:
	AABB() = default;
	AABB( const AABB& ) = default;
	AABB( Vec3 p0, Vec3 p1 );
	explicit AABB( const Sphere& sphere );
	bool operator == ( const AABB& aabb ) const;
	bool operator != ( const AABB& aabb ) const;

	AABB& Expand( Vec3 p );
	AABB& Expand( AABB other );
	AABB& Expand( float boundary );

	Vec3 GetMin() const { return m_min; }
	Vec3 GetMax() const { return m_max; }
	Vec3 GetCenter() const { return ( m_min + m_max ) * 0.5f; }
	Vec3 GetHalfSize() const { return ( m_max - m_min ) * 0.5f; }
	Matrix4 GetTransform() const;

	bool Contains( Vec3 p ) const;
	bool Intersect( AABB other ) const;
	//! Returns the distance \p p is to the surface of the aabb. The returned value
	//! will be negative if \p p is inside the aabb.
	float GetSignedDistanceFromSurface( Vec3 p ) const;
	//! Returns the point on the aabbs surface  that is closest to the given point.
	//! If \p containsOut is provided it will be set to false if the point does not
	//! touch the aabb, and true otherwise.
	Vec3 GetClosestPointOnSurface( Vec3 p, bool* containsOut = nullptr ) const;
	//! Returns true if any point along the line \p p + \p d intersects the aabb. On
	//! intersection \p t0Out will be set so that \p p + \p d * \p t0Out = p0 (where p0
	//! is the first point along the line in the direction of \p d that is on the
	//! surface of the aabb). \p t1Out will be similarly set but for the last
	//! intersection point on the line. \p n0Out and \p n1Out will be set to the face
	//! normals of the aabb at \p t0Out and \p t1Out respectively.
	bool IntersectLine( Vec3 p, Vec3 d, float* t0Out = nullptr, float* t1Out = nullptr, ae::Vec3* n0Out = nullptr, ae::Vec3* n1Out = nullptr ) const;
	//! Returns true if the segment [\p source, \p source + \p ray] intersects the aabb
	//! (including when \p source is inside the aabb). On returning true: \p hitOut
	//! will be set to the first intersection point on the surface (or to \p source
	//! if the ray starts within the aabb). \p normOut will be set to the normal of
	//! the face of the contact point, or to the normal of the nearest face to
	//! \p source if it is inside the aabb. \p tOut will be set to a value so that
	//! \p source + \p ray * \p tOut = \p hitOut.
	bool IntersectRay( Vec3 source, Vec3 ray, Vec3* hitOut = nullptr, ae::Vec3* normOut = nullptr, float* tOut = nullptr ) const;

private:
	Vec3 m_min = Vec3( INFINITY );
	Vec3 m_max = Vec3( -INFINITY );
};
std::ostream& operator<<( std::ostream& os, AABB aabb );

//------------------------------------------------------------------------------
// ae::OBB class
//------------------------------------------------------------------------------
class OBB
{
public:
	OBB() = default;
	OBB( const OBB& ) = default;
	OBB( const Matrix4& transform );
	bool operator == ( const OBB& obb ) const;
	bool operator != ( const OBB& obb ) const;

	void SetTransform( const Matrix4& transform );
	Matrix4 GetTransform() const;

	//! Returns the distance \p p is to the surface of the obb. The returned value
	//! will be negative if \p p is inside the obb.
	float GetSignedDistanceFromSurface( Vec3 p ) const;
	//! Returns the point on the obbs surface  that is closest to the given point.
	//! If \p containsOut is provided it will be set to false if the point does not
	//! touch the obb, and true otherwise.
	Vec3 GetClosestPointOnSurface( Vec3 p, bool* containsOut = nullptr ) const;
	//! Returns true if any point along the line \p p + \p d intersects the obb. On
	//! intersection \p t0Out will be set so that \p p + \p d * \p t0Out = p0 (where p0
	//! is the first point along the line in the direction of \p d that is on the
	//! surface of the obb). \p t1Out will be similarly set but for the last
	//! intersection point on the line. \p n0Out and \p n1Out will be set to the face
	//! normals of the obb at \p t0Out and \p t1Out respectively.
	bool IntersectLine( Vec3 p, Vec3 d, float* t0Out = nullptr, float* t1Out = nullptr, ae::Vec3* n0Out = nullptr, ae::Vec3* n1Out = nullptr ) const;
	//! Returns true if the segment [\p source, \p source + \p ray] intersects the obb
	//! (including when \p source is inside the obb). On returning true: \p hitOut
	//! will be set to the first intersection point on the surface (or to \p source
	//! if the ray starts within the obb). \p normOut will be set to the normal of
	//! the face of the contact point, or to the normal of the nearest face to
	//! \p source if it is inside the obb. \p tOut will be set to a value so that
	//! \p source + \p ray * \p tOut = \p hitOut.
	bool IntersectRay( Vec3 source, Vec3 ray, Vec3* hitOut = nullptr, ae::Vec3* normOut = nullptr, float* tOut = nullptr ) const;
	//! Returns an AABB that tightly fits this obb.
	AABB GetAABB() const;
	
	ae::Vec3 GetCenter() const { return m_center; }
	ae::Vec3 GetAxis( uint32_t idx ) const { return m_axes[ idx ]; }
	ae::Vec3 GetHalfSize() const { return m_halfSize; }

private:
	Vec3 m_center;
	Vec3 m_axes[ 3 ];
	Vec3 m_halfSize;
};

//------------------------------------------------------------------------------
// Geometry helpers
//------------------------------------------------------------------------------
bool IntersectRayTriangle( ae::Vec3 p, ae::Vec3 ray, ae::Vec3 a, ae::Vec3 b, ae::Vec3 c, bool ccw, bool cw, ae::Vec3* pOut, ae::Vec3* nOut, float* tOut );
Vec3 ClosestPointOnTriangle( ae::Vec3 p, ae::Vec3 a, ae::Vec3 b, ae::Vec3 c );

//! @} End Math defgroup

//------------------------------------------------------------------------------
// ae::Color struct
//------------------------------------------------------------------------------
struct Color
{
	Color() : r( 0.0f ), g( 0.0f ), b( 0.0f ), a( 1.0f ) {} //!< Defaults to black
	Color( const Color& ) = default;
	Color( float rgb ); // @TODO: Delete. Color space should be explicit
	Color( float r, float g, float b ); // @TODO: Delete. Color space should be explicit
	Color( float r, float g, float b, float a ); // @TODO: Delete. Color space should be explicit
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
	//! hue: 0-1 saturation: 0-1 value: 0-1
	static Color HSV( float hue, float saturation, float value );

	bool operator == ( ae::Color o ) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
	bool operator != ( ae::Color o ) const { return !( operator == ( o ) ); }
	ae::Color operator * ( ae::Color o ) const { return RGBA( r * o.r, g * o.g, b * o.b, a * o.a ); }
	ae::Color& operator *= ( ae::Color o ) { r *= o.r; g *= o.g; b *= o.b; a *= o.a; return *this; }

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
	// CMYK
	static Color Cyan();
	static Color Magenta();
	// aether
	static Color AetherDarkRed();
	static Color AetherRed();
	static Color AetherOrange();
	static Color AetherYellow();
	static Color AetherGreen();
	static Color AetherTeal();
	static Color AetherBlue();
	static Color AetherPurple();
	static Color AetherWhite();
	static Color AetherGray();
	static Color AetherDarkGray();
	static Color AetherBlack();
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

	//! Define conversion functions etc for ae::Color. See AE_CONFIG_FILE for more info.
#ifdef AE_COLOR_CLASS_CONFIG
	AE_COLOR_CLASS_CONFIG
#endif

	union
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};
		float data[ 4 ];
	};

private:
	// Delete implicit conversions to try to catch color space issues
	template< typename T > Color R( T r ) = delete;
	template< typename T > Color RG( T r, T g ) = delete;
	template< typename T > Color RGB( T r, T g, T b ) = delete;
	template< typename T > Color RGBA( T r, T g, T b, T a ) = delete;
	template< typename T > Color RGBA( const T* v ) = delete;
	template< typename T > Color SRGB( T r, T g, T b ) = delete;
	template< typename T > Color SRGBA( T r, T g, T b, T a ) = delete;
};

#if _AE_WINDOWS_
	#pragma warning(default:26495) // Re-enable uninitialized variable warning
#endif

//------------------------------------------------------------------------------
// ae::TimeStep
//! A utility for measuring and controlling frame time. Create once at the
//! beginning of your game, and set a desired frame time with ae::TimeStep::SetTimeStep(),
//! then call ae::TimeStep::Tick() each frame after drawing your scene.
//------------------------------------------------------------------------------
class TimeStep
{
public:
	TimeStep();

	//! ae::TimeStep::Tick() will sleep to target the provided frame time.
	//! Otherwise provide 0.0 to disable sleeping in ae::TimeStep::Tick().
	void SetTimeStep( float frameTime );
	//! Returns the value set by ae::TimeStep::SetTimeStep(). Default is 0.0.
	float GetTimeStep() const;
	//! Returns the number of times ae::TimeStep::Tick() has been called.
	uint32_t GetStepCount() const;

	//! Returns the time between the last call to ae::TimeStep::Tick() in
	//! seconds. The return value is always 0.0 or greater.
	float GetDt() const;
	//! Useful for handling frames with 0.0 or high dt, eg: timeStep.SetDt( timeStep.GetTimeStep() )
	void SetDt( float sec );

	//! Call this every frame to update dt. If a non-zero frame rate is specified
	//! ae::TimeStep will attempt to keep a steady frame rate, but you should
	//! still use ae::TimeStep::GetDt() in case your frame takes too long.
	void Tick();

private:
	uint32_t m_stepCount = 0;
	double m_timeStep = 0.0;
	double m_sleepOverhead = 0.0;
	double m_prevFrameLength = 0.0;
	double m_frameStart = 0.0;
};

//! \defgroup DataStructures
//! @{

//------------------------------------------------------------------------------
// ae::Str class
//! A fixed length string class. The templated value is the total size of
//! the string in memory.
// @TODO: Fix usage in constexpr function: 'Str< N >' is not literal because it
// is not an aggregate and has no constexpr constructors other than copy or move
// constructors.
//------------------------------------------------------------------------------
template< uint32_t N >
class Str
{
public:
	Str();
	template< uint32_t N2 > Str( const Str<N2>& str );
	Str( const char* str );
	Str( uint32_t length, const char* str );
	//! Construct a string from a range of characters: [begining, end).
	Str( const char* begin, const char* end );
	Str( uint32_t length, char c );
	template< typename... Args > static Str< N > Format( const char* format, Args... args );
	explicit operator const char*() const;
	
	template< uint32_t N2 > void operator =( const Str<N2>& str );
	template< uint32_t N2 > Str<N> operator +( const Str<N2>& str ) const;
	template< uint32_t N2 > void operator +=( const Str<N2>& str );
	template< uint32_t N2 > bool operator ==( const Str<N2>& str ) const;
	template< uint32_t N2 > bool operator !=( const Str<N2>& str ) const;
	template< uint32_t N2 > bool operator <( const Str<N2>& str ) const;
	template< uint32_t N2 > bool operator >( const Str<N2>& str ) const;
	template< uint32_t N2 > bool operator <=( const Str<N2>& str ) const;
	template< uint32_t N2 > bool operator >=( const Str<N2>& str ) const;
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

	template< uint32_t N2 >
	void Append( const Str<N2>& str );
	void Append( const char* str );
	void Trim( uint32_t len );

	uint32_t Length() const;
	uint32_t Size() const;
	bool Empty() const;
	static constexpr uint32_t MaxLength() { return N - 3u; } // Leave room for length var and null terminator

private:
	template< uint32_t N2 > friend class Str;
	template< uint32_t N2 > friend bool operator ==( const char*, const Str< N2 >& );
	template< uint32_t N2 > friend bool operator !=( const char*, const Str< N2 >& );
	template< uint32_t N2 > friend bool operator <( const char*, const Str< N2 >& );
	template< uint32_t N2 > friend bool operator >( const char*, const Str< N2 >& );
	template< uint32_t N2 > friend bool operator <=( const char*, const Str< N2 >& );
	template< uint32_t N2 > friend bool operator >=( const char*, const Str< N2 >& );
	template< uint32_t N2 > friend std::istream& operator>>( std::istream&, Str< N2 >& );
	void m_Format( const char* format );
	template< typename T, typename... Args >
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
// ae::Pair class
//------------------------------------------------------------------------------
template< typename K, typename V >
struct Pair
{
	Pair( const K& k, const V& v ) : key( k ), value( v ) {}
	K key;
	V value;
};

//------------------------------------------------------------------------------
// ae::GetHash helper
//! Internally selects between T::GetHash{U}() and ae::GetHash{U}( const T& )
//! automatically based on availability. The member function is prioritized
//! if both are defined.
//------------------------------------------------------------------------------
template< typename U, typename T > U GetHash( const T& v );

//------------------------------------------------------------------------------
// ae::GetHash32 helper
//! Implement this helper for types that are used as ae::Map< Key, ...> or with
//! ae::Hash32.
//------------------------------------------------------------------------------
template< typename T > uint32_t GetHash32( const T& key );
template< uint32_t N > uint32_t GetHash32( const ae::Str< N >& key );
template< typename T > uint32_t GetHash32( T* const& key );
template<> uint32_t GetHash32( const char* const& key );
template<> uint32_t GetHash32( char* const& key );

//------------------------------------------------------------------------------
// ae::GetHash64 helper
//! Implement this helper for types that are used with ae::Hash64.
//------------------------------------------------------------------------------
template< typename T > uint64_t GetHash64( const T& key );
template< uint32_t N > uint64_t GetHash64( const ae::Str< N >& key );
template< typename T > uint64_t GetHash64( T* const& key );
template<> uint64_t GetHash64( const char* const& key );
template<> uint64_t GetHash64( char* const& key );

//------------------------------------------------------------------------------
// ae::Hash32 class
// ae::Hash64 class
//! A FNV1a hash utility class. Empty strings and zero-length data buffers do not
//! hash to zero.
//------------------------------------------------------------------------------
template< typename U >
class Hash
{
public:
	using UInt = U;

	Hash();
	explicit Hash( U initialValue );
	
	bool operator == ( Hash o ) const { return m_hash == o.m_hash; }
	bool operator != ( Hash o ) const { return m_hash != o.m_hash; }

	Hash& HashString( const char* str );
	Hash& HashData( const void* data, uint32_t length );
	template< typename T, uint32_t N > Hash& HashType( const T (&array)[ N ] );
	template< typename T > Hash& HashType( const T& v );
	
	void Set( U hash );
	U Get() const;
	
private:
	template< typename T > Hash( T initialValue ) = delete;
	template< typename T > void Set( T hash ) = delete;
	static constexpr U m_GetPrime();
	U m_hash;
};
using Hash32 = Hash< uint32_t >;
using Hash64 = Hash< uint64_t >;

//------------------------------------------------------------------------------
// ae::Optional class
//------------------------------------------------------------------------------
template< typename T >
class Optional
{
public:
	// @TODO: Handle assignment of {} when T is default constructible
	Optional() = default;
	Optional( const T& value );
	Optional( T&& value ) noexcept;
	Optional( const Optional< T >& other );
	Optional( Optional< T >&& other ) noexcept;
	T& operator =( const T& value );
	T& operator =( T&& value ) noexcept;
	void operator =( const Optional< T >& other );
	void operator =( Optional< T >&& other ) noexcept;
	~Optional();

	T* TryGet();
	const T* TryGet() const;
	T Get( T defaultValue ) const;
	void Clear();

private:
	bool m_hasValue = false;
	alignas(T) std::byte m_value[ sizeof(T) ];
};


//------------------------------------------------------------------------------
// ae::Array class
//------------------------------------------------------------------------------
template< typename T, uint32_t N = 0 >
class Array
{
public:
	//! Static array (N > 0) only. Constructs an empty array, where
	//! ae::Array::Length() == 0 and ae::Array::Size() == N.
	Array();
	//! Static array (N > 0) only. Appends 'length' number of 'val's, so that
	//! ae::Array::Length() == 'length' and ae::Array::Size() == N.
	Array( const T& val, uint32_t length );
	//! Static array (N > 0) only. Constructs from a standard initializer list,
	//! so that ae::Array::Length() == 'initList.size()' and ae::Array::Size() == N.
	Array( std::initializer_list< T > initList );

	//! Dynamic array (N == 0) only. Constructs an empty array, where
	//! ae::Array::Length() == 0 and ae::Array::Size() == N.
	Array( ae::Tag tag );
	//! Dynamic array (N == 0) only. Constructs an empty array, while reserving
	//! 'size' elements. ae::Array::Length() == 0 and ae::Array::Size() == 'size'.
	Array( ae::Tag tag, uint32_t size );
	//! Dynamic array (N == 0) only. Reserves 'length' and appends 'length'
	//! number of 'val's. ae::Array::Length() == 'length' and ae::Array::Size() >= 'length'.
	Array( ae::Tag tag, const T& val, uint32_t length );
	//! Dynamic array (N == 0) only. Expands the internal array storage to avoid
	//! copying data unnecessarily on Append(). This does not affect the number
	//! of elements returned by Length(). Retrieve the current storage limit
	//! with Size().
	void Reserve( uint32_t total );

	//! Copy constructor. The ae::Tag of \p other will be used for the newly
	//! constructed array if the array is dynamic (N == 0).
	Array( const Array< T, N >& other );
	//! Move constructor falls back to the regular copy constructor for static
	//! arrays (N > 0) or if the given ae::Tags don't match
	Array( Array< T, N >&& other ) noexcept;
	//! Assignment operator
	void operator =( const Array< T, N >& other );
	//! Move assignment operator falls back to the regular assignment operator
	//! for static arrays (N > 0) or if the given ae::Tags don't match
	void operator =( Array< T, N >&& other ) noexcept;
	~Array();

	//! Adds one copy of \p value to the end of the array. Can reallocate
	//! internal storage for dynamic arrays (N == 0), so take care when taking
	//! the address of any elements. Returns a reference to the added entry.
	T& Append( const T& value );
	//! Adds \p count copies of \p value to the end of the array. Can reallocate
	//! internal storage for dynamic arrays (N == 0), so take care when taking
	//! the address of any elements. Returns a pointer to the first new element
	//! added, or one past the end of the array if \p count is zero.
	T* Append( const T& value, uint32_t count );
	//! Adds \p count elements from \p values. Can reallocate internal storage
	//! for  dynamic arrays (N == 0), so take care when taking the address of
	//! any elements. Returns a pointer to the first new element added, or one
	//! past the end of the array if \p count is zero.
	T* AppendArray( const T* values, uint32_t count );

	//! Adds one copy of \p value at \p index. \p index must be less than or
	//! equal to Length(). Can reallocate internal storage for dynamic arrays
	//! (N == 0), so take care when taking the address of any elements. Returns
	//! a reference to the added entry.
	T& Insert( uint32_t index, const T& value );
	//! Adds \p count copies of \p value at \p index. \p index must be less than
	//! or equal to Length(). Can reallocate internal storage for dynamic arrays
	//! (N == 0), so take care when taking the address of any elements. Returns
	//! a pointer to the first new element added, or the address of the element
	//! at \p index if \p count is zero.
	T* Insert( uint32_t index, const T& value, uint32_t count );
	//! Adds \p count elements from \p values at \p index. \p index must be less
	//! than or equal to Length(). Can reallocate internal storage for  dynamic
	//! arrays (N == 0), so take care when taking the address of any elements.
	//! Returns a pointer to the first new element added, or the address of the
	//! element at \p index if \p count is zero.
	T* InsertArray( uint32_t index, const T* values, uint32_t count );

	//! Returns the index of the first matching element or -1 when not found.
	template< typename U > int32_t Find( const U& value ) const;
	//! Returns the index of the last matching element or -1 when not found.
	template< typename U > int32_t FindLast( const U& value ) const;
	//! Returns the index of the first matching element or -1 when not found.
	//! The function signature should match 'bool (*)( const T2& )' or
	//! '[...]( const T2& ) -> bool'. Return true from the predicate for a
	//! any matching element.
	template< typename Fn > int32_t FindFn( Fn testFn ) const;
	//! Returns the index of the last matching element or -1 when not found.
	//! The function signature should match 'bool (*)( const U& )' or
	//! '[...]( const T2& ) -> bool'. Return true from the predicate for any
	//! matching element.
	template< typename Fn > int32_t FindLastFn( Fn testFn ) const;

	//! Remove all elements that match \p value. Returns the number of elements
	//! that were removed.
	template< typename U > uint32_t RemoveAll( const U& value );
	//! Remove elements based on predicate \p testFn. The function signature
	//! should match 'bool (*)( const U& )' and '[]( const U& ) -> bool'. Return
	//! true from the predicate for removal of the given element. Returns the
	//! number of elements that were removed.
	template< typename Fn > uint32_t RemoveAllFn( Fn testFn );
	//! Removes \p count elements at \p index. \p index plus \p count must be
	//! less than or equal to Length().
	void Remove( uint32_t index, uint32_t count = 1 );
	//! Destructs all elements in the array and resets the array to length zero.
	//! Does not affect the size of the array.
	void Clear();

	//! Performs bounds checking in debug mode. Use 'GetData()' to get raw array.
	const T& operator[]( int32_t index ) const;
	//! Performs bounds checking in debug mode. Use 'GetData()' to get raw array.
	T& operator[]( int32_t index );

	//! Returns a pointer to the first element of the array, but can return null
	//! when the array length is zero
	T* Data() { return m_array; }
	//! Returns a pointer to the first element of the array, but can return null
	//! when the array length is zero
	const T* Data() const { return m_array; }
	//! Returns the number of elements currently in the array
	uint32_t Length() const { return m_length; }
	//! Returns the total size of a static array (N > 0)
	_AE_STATIC_STORAGE static constexpr uint32_t Size() { return N; }
	//! Returns the total size of a dynamic array (N == 0)
	_AE_DYNAMIC_STORAGE uint32_t Size(...) const { return m_size; }
	//! Returns the tag provided to the constructor for dynamic arrays (N == 0).
	//! Returns ae::Tag() for all static arrays (N > 0).
	ae::Tag Tag() const { return m_tag; }

private:
	uint32_t m_length;
	uint32_t m_size;
	T* m_array;
	ae::Tag m_tag;
	// clang-format off
	typedef typename std::aligned_storage< sizeof(T), alignof(T) >::type AlignedStorageT;
#if _AE_LINUX_
	struct Storage { AlignedStorageT data[ N ]; };
	Storage m_storage;
#else
	template< uint32_t > struct Storage { AlignedStorageT data[ N ]; };
	template<> struct Storage< 0 > {};
	Storage< N > m_storage;
#endif
	// clang-format on
public:
	//! For ranged-based looping. Returns a pointer to the first element of the
	//! array, but can return null when array length is zero. Lowercase to match
	//! the c++ standard.
	T* begin() { return m_array; }
	//! For ranged-based looping. Returns a pointer one past the last element of
	//! the array, but can return null when array length is zero. Lowercase to
	//! match the c++ standard.
	T* end() { return m_array + m_length; }
	//! For ranged-based looping. Returns a pointer to the first element of the
	//! array, but can return null when array length is zero. Lowercase to match
	//! the c++ standard.
	const T* begin() const { return m_array; }
	//! For ranged-based looping. Returns a pointer one past the last element of
	//! the array, but can return null when array length is zero. Lowercase to
	//! match the c++ standard.
	const T* end() const { return m_array + m_length; }
};

//------------------------------------------------------------------------------
// ae::HashMap class
//! Internally calls ae::GetHash< 32 or 64 >( const Key& ) depending on the
//! 'Hash' template parameter.
//------------------------------------------------------------------------------
template< typename Key, uint32_t N = 0, typename Hash = ae::Hash32 >
class HashMap
{
public:
	//! Constructor for a hash map with static allocated storage (N > 0).
	HashMap();
	//! Constructor for a hash map with dynamically allocated storage (N == 0).
	HashMap( ae::Tag pool );
	//! Expands the storage if necessary so a \p size number of key/index pairs
	//! can be added without any internal allocations. Asserts if using static
	//! storage and \p size is greater than N.
	void Reserve( uint32_t size );
	
	HashMap( const HashMap< Key, N, Hash >& other );
	void operator =( const HashMap< Key, N, Hash >& other );
	// @TODO: Move operators
	//! Releases allocated storage
	~HashMap();
	
	//! Adds an entry for lookup with ae::HashMap::Get(). If the key already
	//! exists the index will be updated. In both cases the return value will be
	//! true, and false otherwise.
	bool Set( Key key, uint32_t index );
	//! Removes the entry with \p key if it exists. Returns the index associated
	//! with the removed key on success, -1 otherwise.
	int32_t Remove( Key key );
	//! Decrements the existing index values supplied to ae::HashMap::Insert()
	//! of all entries greater than \p index. Useful when index values represent
	//! offsets into another array being compacted after the removal of an entry.
	void Decrement( uint32_t index );
	//! Returns the index associated with the given key, or -1 if the key is not found.
	int32_t Get( Key key ) const;
	//! Removes all entries.
	void Clear();

	//! Returns the number of entries.
	uint32_t Length() const;
	//! Returns the max number of entries.
	_AE_STATIC_STORAGE static constexpr uint32_t Size() { return N; }
	//! Returns the number of allocated entries.
	_AE_DYNAMIC_STORAGE uint32_t Size(...) const { return m_size; }

private:
	bool m_Insert( Key key, typename Hash::UInt hash, int32_t index );
	struct Entry
	{
		Key newKey;
		typename Hash::UInt hash;
		int32_t index = -1;
	};
	ae::Tag m_tag;
	Entry* m_entries;
	uint32_t m_size;
	uint32_t m_length;
	// clang-format off
#if _AE_LINUX_
	struct Storage { Entry data[ N ]; };
	Storage m_storage;
#else
	template< uint32_t > struct Storage { Entry data[ N ]; };
	template<> struct Storage< 0 > {};
	Storage< N > m_storage;
#endif
	// clang-format on
};

//! Set ae::Map to Fast mode to allow reording of elements. Stable to maintain
//! the order of inserted elements.
enum class MapMode { Fast, Stable };
//------------------------------------------------------------------------------
// ae::Map class
//! Internally calls ae::GetHash< 32 or 64 >( const Key& ) depending on the
//! 'Hash' template parameter.
//------------------------------------------------------------------------------
template< typename Key, typename Value, uint32_t N = 0, typename Hash = ae::Hash32, ae::MapMode Mode = ae::MapMode::Fast >
class Map
{
public:
	//! Constructor for a map with static allocated storage (N > 0)
	Map();
	//! Constructor for a map with dynamically allocated storage (N == 0)
	Map( ae::Tag pool );
	//! Expands the map storage if necessary so a \p count number of key/value
	//! pairs can be added without any internal allocations. Asserts if using
	//! static storage and \p count is less than N.
	void Reserve( uint32_t count );
	
	//! Add or replace a key/value pair in the map. Can be retrieved with
	//! ae::Map::Get(). The value is updated in place if the element is found,
	//! otherwise the new pair is appended. It's not safe to keep a pointer to
	//! the value across non-const operations.
	Value& Set( const Key& key, const Value& value );
	//! Returns a modifiable reference to the value set with \p key. Asserts
	//! when key/value pair is missing.
	Value& Get( const Key& key );
	//! Returns the value set with \p key. Asserts when key/value pair is missing.
	const Value& Get( const Key& key ) const;
	//! Returns the value set with \p key. Returns \p defaultValue otherwise
	//! when the key/value pair is missing.
	template< typename V = Value > Value Get( const Key& key, V&& defaultValue ) const&;
	//! Returns a pointer to the value set with \p key. Returns null otherwise
	//! when the key/value pair is missing.
	Value* TryGet( const Key& key );
	//! Returns a pointer to the value set with \p key. Returns null otherwise
	//! when the key/value pair is missing.
	const Value* TryGet( const Key& key ) const;
	//! Returns true when \p key matches an existing key/value pair. A copy of
	//! the value is set to \p valueOut.
	bool TryGet( const Key& key, Value* valueOut );
	//! Returns true when \p key matches an existing key/value pair. A copy of
	//! the value is set to \p valueOut.
	bool TryGet( const Key& key, Value* valueOut ) const;
	
	//! Performs a constant time removal of an element with \p key while
	//! potentially re-ordering elements with ae::MapMode::Fast. Performs a
	//! linear time removal of an element with \p key with ae::MapMode::Stable.
	//! Returns true on success, and a copy of the value is set to \p valueOut
	//! if it is not null.
	bool Remove( const Key& key, Value* valueOut = nullptr );
	//! Removes an element by index. See ae::Map::Remove() for more details.
	void RemoveIndex( uint32_t index, Value* valueOut = nullptr );
	//! Remove all key/value pairs from the map.
	void Clear();

	//! Access elements by index. Returns the nth key in the map.
	const Key& GetKey( int32_t index ) const;
	//! Access elements by index. Returns the nth value in the map.
	const Value& GetValue( int32_t index ) const;
	//! Access elements by index. Returns a modifiable reference to the nth
	//! value in the map.
	Value& GetValue( int32_t index );
	//! Returns the index of a key/value pair in the map. Returns -1 when
	//! key/value pair is missing.
	int32_t GetIndex( const Key& key ) const;
	//! Returns the number of key/value pairs in the map
	uint32_t Length() const;
	//! Returns the max number of entries.
	_AE_STATIC_STORAGE static constexpr uint32_t Size() { return N; }
	//! Returns the number of allocated entries.
	_AE_DYNAMIC_STORAGE uint32_t Size(...) const { return m_pairs.Size(); }

	// Ranged-based loop. Lowercase to match c++ standard
	ae::Pair< Key, Value >* begin() { return m_pairs.begin(); }
	ae::Pair< Key, Value >* end() { return m_pairs.end(); }
	const ae::Pair< Key, Value >* begin() const { return m_pairs.begin(); }
	const ae::Pair< Key, Value >* end() const { return m_pairs.end(); }

private:
	bool m_RemoveIndex( int32_t index, Value* valueOut );
	template< typename K2, typename V2, uint32_t N2, typename H2, ae::MapMode M2 >
	friend std::ostream& operator<<( std::ostream&, const Map< K2, V2, N2, H2, M2 >& );
	HashMap< Key, N, Hash > m_hashMap;
	Array< ae::Pair< Key, Value >, N > m_pairs;
};

//------------------------------------------------------------------------------
// ae::Dict class
//------------------------------------------------------------------------------
template< uint32_t N = 0 >
class Dict
{
public:
	Dict() = default;
	Dict( ae::Tag tag );
	void SetString( const char* key, const char* value );
	void SetString( const char* key, char* value ) { SetString( key, (const char*)value ); }
	void SetInt( const char* key, int64_t value );
	void SetUInt( const char* key, uint64_t value );
	void SetFloat( const char* key, float value );
	void SetBool( const char* key, bool value );
	void SetVec2( const char* key, ae::Vec2 value );
	void SetVec3( const char* key, ae::Vec3 value );
	void SetVec4( const char* key, ae::Vec4 value );
	void SetInt2( const char* key, ae::Int2 value );
	void SetMatrix4( const char* key, const ae::Matrix4& value );

	bool Remove( const char* key );
	void Clear();

	const char* GetString( const char* key, const char* defaultValue ) const;
	int64_t GetInt( const char* key, int64_t defaultValue ) const;
	uint64_t GetUInt( const char* key, uint64_t defaultValue ) const;
	float GetFloat( const char* key, float defaultValue ) const;
	bool GetBool( const char* key, bool defaultValue ) const;
	ae::Vec2 GetVec2( const char* key, ae::Vec2 defaultValue ) const;
	ae::Vec3 GetVec3( const char* key, ae::Vec3 defaultValue ) const;
	ae::Vec4 GetVec4( const char* key, ae::Vec4 defaultValue ) const;
	ae::Int2 GetInt2( const char* key, ae::Int2 defaultValue ) const;
	ae::Matrix4 GetMatrix4( const char* key, const ae::Matrix4& defaultValue ) const;
	bool Has( const char* key ) const;

	const char* GetKey( uint32_t idx ) const;
	const char* GetValue( uint32_t idx ) const;
	uint32_t Length() const { return m_entries.Length(); }
	//! Returns the max number of entries.
	_AE_STATIC_STORAGE static constexpr uint32_t Size() { return N; }
	//! Returns the max number of entries.
	_AE_DYNAMIC_STORAGE uint32_t Size(...) const { return m_entries.Size(); }
	
	// Ranged-based loop. Lowercase to match c++ standard
	ae::Pair< ae::Str128, ae::Str128 >* begin() { return m_entries.begin(); }
	ae::Pair< ae::Str128, ae::Str128 >* end() { return m_entries.end(); }
	const ae::Pair< ae::Str128, ae::Str128 >* begin() const { return m_entries.begin(); }
	const ae::Pair< ae::Str128, ae::Str128 >* end() const { return m_entries.end(); }

private:
	// Prevent the above functions from being called accidentally through automatic conversions
	template< typename T > void SetString( const char*, T ) = delete;
	template< typename T, typename = std::enable_if_t< !std::is_signed_v< T > > > void SetInt( const char*, T ) = delete;
	template< typename T, typename = std::enable_if_t< std::is_signed_v< T > > > void SetUInt( const char*, T ) = delete;
	template< typename T, typename = std::enable_if_t< !std::is_floating_point_v< T > > > void SetFloat( const char*, T ) = delete;
	template< typename T > void SetBool( const char*, T ) = delete;
	template< typename T > void SetVec2( const char*, T ) = delete;
	template< typename T > void SetVec3( const char*, T ) = delete;
	template< typename T > void SetVec4( const char*, T ) = delete;
	template< typename T > void SetInt2( const char*, T ) = delete;
	template< typename T > void SetMatrix4( const char*, T ) = delete;
	ae::Map< ae::Str128, ae::Str128, N, ae::Hash32, ae::MapMode::Stable > m_entries;
};

template< uint32_t N > std::ostream& operator<<( std::ostream& os, const ae::Dict< N >& dict );

//------------------------------------------------------------------------------
// ae::ListNode class
//------------------------------------------------------------------------------
template< typename T > class List; // ae::List forward declaration
template< typename T >
class ListNode
{
public:
	ListNode( T* owner );
	~ListNode();

	void Remove();

	T* GetFirst();
	T* GetNext();
	T* GetPrev();
	T* GetLast();

	const T* GetFirst() const;
	const T* GetNext() const;
	const T* GetPrev() const;
	const T* GetLast() const;

	List< T >* GetList();
	const List< T >* GetList() const;

private:
	friend class List< T >;
	
	// @NOTE: These operations don't make sense when either node is in a list,
	// to avoid a potentially hard to diagnose random assert, assignment is
	// disabled altogether
	ListNode( ListNode& ) = delete;
	void operator = ( ListNode& ) = delete;

	List< T >* m_root;
	ListNode* m_next;
	ListNode* m_prev;
	T* m_owner;
};

//------------------------------------------------------------------------------
// ae::List class
//------------------------------------------------------------------------------
template< typename T >
class List
{
public:
	List();
	~List();

	void Append( ListNode< T >& node );
	void Clear();

	T* GetFirst();
	T* GetLast();

	const T* GetFirst() const;
	const T* GetLast() const;

	template< typename U > T* Find( const U& value );
	template< typename Fn > T* FindFn( Fn predicateFn ); // @TODO: FindFn's parameter should be a reference
	template< typename U > const T* Find( const U& value ) const;
	template< typename Fn > const T* FindFn( Fn predicateFn ) const; // @TODO: FindFn's parameter should be a reference

	uint32_t Length() const;

private:
	friend class ListNode< T >;
	
	// @NOTE: Disable assignment. Assigning a list to another list technically makes sense,
	// but could result in unexpected orphaning of list nodes. Additionally disabling these
	// operations is consistent with list node.
	List( List& ) = delete;
	void operator = ( List& ) = delete;

	ListNode< T >* m_first;
};

//------------------------------------------------------------------------------
// ae::RingBuffer class
//------------------------------------------------------------------------------
template< typename T, uint32_t N = 0 >
class RingBuffer
{
public:
	//! Constructor for a ring buffer with static allocated storage (N > 0).
	RingBuffer();
	//! Constructor for a ring buffer with dynamically allocated storage (N == 0).
	RingBuffer( ae::Tag tag, uint32_t size );
	//! Appends an element to the current end of the ring buffer. It's safe to
	//! call this when the ring buffer is full, although in this case the
	//! element previously at index 0 to be destroyed. Does not affect
	//! ae::RingBuffer::Size().
	T& Append( const T& val );
	//! Resets ae::RingBuffer::Length() and ae::RingBuffer::GetOffset() to 0.
	//! Does not affect ae::RingBuffer::Size().
	void Clear();

	//! Returns the element at the given \p index, which must be less than
	//! ae::RingBuffer::Length(). The oldest appended element that has not been
	//! 'pushed out' of the buffer is at index 0, while the most recently
	//! appended element is at length - 1.
	T& Get( uint32_t index );
	//! Returns the element at the given \p index, which must be less than
	//! ae::RingBuffer::Length(). The oldest appended element that has not been
	//! 'pushed out' of the buffer is at index 0, while the most recently
	//! appended element is at length - 1.
	const T& Get( uint32_t index ) const;
	
	//! Returns a pointer to the internal buffer, but can return null when the
	//! length is zero. See ae::RingBuffer::GetOffset() for information on how
	//! to iterate over the returned buffer.
	T* Data() { return m_buffer.Data(); }
	//! Returns a pointer to the internal buffer, but can return null when the
	//! length is zero. See ae::RingBuffer::GetOffset() for information on how
	//! to iterate over the returned buffer.
	const T* Data() const { return m_buffer.Data(); }
	//! Returns the current index of the first element in the internal buffer
	//! (or 0 when the buffer is empty). Use this in conjunction with
	//! ae::RingBuffer::Data() to get a pointer to the the first element in the
	//! buffer, iterate 0 to ae::RingBuffer::Length() elements, and reduce the
	//! iteration index modulo ae::RingBuffer::Size() when indexing the buffer.
	//! This is a more complicated approach than using ae::RingBuffer::Get() but
	//! can be useful to avoid copying data for APIs that accept generic ring
	//! buffer data, such as ImGui.
	//! Eg.:
	//! \code
	//! for( uint32_t i = 0; i < ringBuffer.Length(); i++ )
	//! {
	//!     const uint32_t index = ( ringBuffer.GetOffset() + i ) % ringBuffer.Size();
	//!     const T& element = ringBuffer.Data()[ index ];
	//! }
	//! \endcode
	uint32_t GetOffset() const { return m_first; }

	//! Returns the number of appended entries up to ae::RingBuffer::Size().
	uint32_t Length() const { return m_buffer.Length(); }
	//! Returns the max number of entries.
	_AE_STATIC_STORAGE static constexpr uint32_t Size() { return N; }
	//! Returns the max number of entries.
	_AE_DYNAMIC_STORAGE uint32_t Size(...) const { return m_size; }

private:
	uint32_t m_first;
	uint32_t m_size;
	ae::Array< T, N > m_buffer;
};

//------------------------------------------------------------------------------
// ae::FreeList class
//! ae::FreeList can be used along side a separate data array to track allocated
//! elements. Given a size, ae::FreeList allows allocation and release of array
//! indices from 0 to size - 1.
//------------------------------------------------------------------------------
template< uint32_t N = 0 >
class FreeList
{
public:
	FreeList();
	FreeList( const ae::Tag& tag, uint32_t size );

	//! Returns (0 <= index < N) on success, and negative on failure.
	int32_t Allocate();
	//! Releases \p idx for future calls to ae::FreeList::Allocate(). \p idx must
	//! be an allocated index or negative (a result of ae::FreeList::Allocate() failure).
	void Free( int32_t idx );
	//! Frees all allocated indices.
	void FreeAll();

	//! Returns the index of the first allocated object. Returns a negative value
	//! if there are no allocated objects.
	int32_t GetFirst() const;
	//! Returns the index of the next allocated object after \p idx. Returns a
	//! negative value if there are no more allocated objects. \p idx must
	//! be an allocated index or negative. A negative value will be returned
	//! if \p idx is negative.
	int32_t GetNext( int32_t idx ) const;

	//! Returns true if the given \p idx is currently allocated. \p idx must be
	//! negative or less than N.
	bool IsAllocated( int32_t idx ) const;
	//! Returns true if the next Allocate() will succeed. Is constant time.
	bool HasFree() const;
	//! Returns the number of allocated elements. Is constant time.
	uint32_t Length() const;
	//! Returns the maximum length of the list (constxpr for static
	//! ae::FreeList's). Is constant time.
	_AE_STATIC_STORAGE static constexpr uint32_t Size() { return N; }
	//! Returns the maximum length of the list. Is constant time.
	_AE_DYNAMIC_STORAGE uint32_t Size(...) const { return m_pool.Length(); }

private:
	struct Entry { Entry* next; };
	uint32_t m_length;
	Entry* m_free;
	ae::Array< Entry, N > m_pool;
};

//------------------------------------------------------------------------------
// ae::ObjectPool class
//------------------------------------------------------------------------------
template< typename T, uint32_t N, bool Paged = false >
class ObjectPool
{
public:
	//! Constructor for static ae::ObjectPool's only.
	ObjectPool();
	//! Constructor for paged ae::ObjectPool's only.
	ObjectPool( const ae::Tag& tag );
	//! All objects allocated with ae::ObjectPool::New() must be destroyed before
	//! the ae::ObjectPool is destroyed.
	~ObjectPool();

	//! Returns a pointer to a freshly constructed object T or null if there
	//! are no free objects. Call ae::ObjectPool::Delete() to destroy the object.
	//! ae::ObjectPool::Delete() must be called on every object returned
	//! by ae::ObjectPool::New().
	template< typename ... Args > T* New( Args&& ... args );
	//! Destructs and releases the object \p obj for future use by ae::ObjectPool::New().
	//! It is safe for the \p obj parameter to be null.
	void Delete( T* obj );
	//! Destructs and releases all objects for future use by ae::ObjectPool::New().
	void DeleteAll();

	//! Returns the first allocated object in the pool or null if the pool is empty.
	const T* GetFirst() const;
	//! Returns the next allocated object after \p obj or null if there are no more objects.
	//! Null will be returned if \p obj is null.
	const T* GetNext( const T* obj ) const;
	//! Returns the first allocated object in the pool or null if the pool is empty.
	T* GetFirst();
	//! Returns the next allocated object after \p obj or null if there are no more objects.
	//! Null will be returned if \p obj is null.
	T* GetNext( T* obj );

	//! Returns true if the next ae::ObjectPool::New() will succeed. Is constant time.
	bool HasFree() const;
	//! Returns the number of allocated objects. Is constant time.
	uint32_t Length() const;
	//! Returns the total number of available objects in the pool. Is constant time.
	_AE_FIXED_POOL static constexpr uint32_t Size() { return N; }
	//! Returns INT32_MAX max, as paged pools can grow indefinitely. Is constant time.
	_AE_PAGED_POOL uint32_t Size(...) const { return INT32_MAX; }

private:
	struct Page; // Internal forward declaration
public:
	template< typename T2 > // Templated for T and const T
	class Iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T2;
		using const_value_type = const T2;
		using reference = T2&;
		using pointer = T2*;
		Iterator() = default;
		Iterator( Iterator& ) = default;
		Iterator( const ObjectPool* pool, const struct Page* page, pointer ptr );
		operator Iterator< const_value_type >() const { return Iterator< const_value_type >( m_pool, m_page, m_ptr ); } //!< Allow auto cast to const
		reference operator*() { return *m_ptr; }
		pointer operator->() { return m_ptr; }
		const T2& operator*() const { return *m_ptr; }
		const pointer operator->() const { return m_ptr; }
		friend bool operator== ( const Iterator& a, const Iterator& b ) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= ( const Iterator& a, const Iterator& b ) { return !( a == b ); };
		Iterator& operator++();
		Iterator operator++( int );
		Iterator begin();
		Iterator end();
	private:
		pointer m_ptr = nullptr;
		const struct Page* m_page = nullptr;
		const ObjectPool* m_pool = nullptr;
	};
	//! Returns an stl conformant ae::OpaquePool::Iterator pointing to the first element.
	Iterator< T > begin();
	//! Returns an stl conformant ae::OpaquePool::Iterator pointing to one beyond the end of the pool.
	Iterator< T > end();
	//! Returns an stl conformant const ae::OpaquePool::Iterator pointing to the first element.
	Iterator< const T > begin() const;
	//! Returns an stl conformant const ae::OpaquePool::Iterator pointing to one beyond the end of the pool.
	Iterator< const T > end() const;

private:
	ObjectPool( ObjectPool& other ) = delete;
	void operator=( ObjectPool& other ) = delete;
	typedef typename std::aligned_storage< sizeof(T), alignof(T) >::type AlignedStorageT;
	struct Page
	{
		Page() : node( this ) {}
		ae::ListNode< Page > node;
		ae::FreeList< N > freeList;
		AlignedStorageT objects[ N ];
	};
	
#if _AE_LINUX_
	template< bool Allocate > struct ConditionalPage {
		Page* Get() { return Allocate ? nullptr : page; }
		const Page* Get() const { return Allocate ? nullptr : page; }
		Page page[ Allocate ? 0 : 1 ];
	};
#else
	template< bool Allocate > struct ConditionalPage {
		Page* Get() { return nullptr; }
		const Page* Get() const { return nullptr; }
	};
	template<> struct ConditionalPage< false > {
		Page* Get() { return &page; }
		const Page* Get() const { return &page; }
		Page page;
	};
#endif
	ae::Tag m_tag;
	uint32_t m_length = 0;
	ae::List< Page > m_pages;
	ConditionalPage< Paged > m_firstPage;
};

//------------------------------------------------------------------------------
// ae::OpaquePool class
//------------------------------------------------------------------------------
//! ae::OpaquePool is useful for dynamically allocating memory for many object
//! instances when the object type is not known at compile time. It's particularly
//! useful in conjunction with ae::ClassType (see constructor for more info on this).
//! It's possible to iterate over an ae::OpaquePool by calling ae::OpaquePool::Iterate< T >
//! (where the template parameter is required because ae::OpaquePool is not
//! templated itself). Additionally, a static ae::Map of ae::OpaquePool's is a
//! great way to allocate game objects or components loaded from a level file.
//------------------------------------------------------------------------------
class OpaquePool
{
public:
	//! Constructs an ae::OpaquePool with dynamic internal storage. \p tag will
	//! be used for all internal allocations. All objects returned by the pool
	//! will have \p objectSize and \p objectAlignment. If the pool is \p paged
	//! it will allocate pages of \p size as necessary. If the pool is
	//! not \p paged, then \p size objects can be allocated at a time. It may be
	//! useful to use this in conjunction with registered ae::ClassType's, passing the
	//! results of ae::ClassType::GetSize() to \p objectSize and ae::ClassType::GetAlignment()
	//! to \p objectAlignment.
	OpaquePool( const ae::Tag& tag, uint32_t objectSize, uint32_t objectAlignment, uint32_t size, bool paged );
	//! All objects allocated with ae::OpaquePool::Allocate/New() must be destroyed before
	//! the ae::OpaquePool is destroyed.
	~OpaquePool();

	//! Returns a pointer to a freshly constructed object T. If the pool is not
	//! paged and there are no free objects null will be returned. Call
	//! ae::OpaquePool::Delete() to destroy the object. ae::OpaquePool::Delete()
	//! must be called on every object returned by ae::OpaquePool::New(), although
	//! it is safe to mix calls to ae::OpaquePool::Allocate/New() and
	//! ae::OpaquePool::Free/Delete() as long as constructors and destructors are
	//! called manually with ae::OpaquePool::Allocate() and ae::OpaquePool::Free().
	template< typename T, typename ... Args > T* New( Args&& ... args );
	//! Destructs and releases the object \p obj for future use. It is safe for \p obj to be null.
	template< typename T > void Delete( T* obj );
	//! Destructs and releases all objects for future use.
	template< typename T > void DeleteAll();

	//! Returns a pointer to an object. If the pool is not paged and there are no free
	//! objects null will be returned. The user is responsible for any constructor
	//! calls. ae::OpaquePool::Free() must be called on every object returned by
	//! ae::OpaquePool::Allocate(). It is safe to mix calls to ae::OpaquePool::Allocate/New()
	//! and ae::OpaquePool::Free/Delete() as long as constructors and destructors are
	//! called manually with ae::OpaquePool::Allocate() and ae::OpaquePool::Free().
	void* Allocate();
	//! Releases the object \p obj for future use. It is safe for \p obj to be null.
	void Free( void* obj );
	//! Releases all objects for future use by ae::OpaquePool::Allocate().
	//! THIS FUNCTION DOES NOT CALL THE OBJECTS DESTRUCTORS, so please use with caution!
	void FreeAll();

	//! Returns true if the next ae::OpaquePool::New() will succeed. Is constant time.
	bool HasFree() const;
	//! Returns the number of allocated objects. Is constant time.
	uint32_t Length() const { return m_length; }
	//! Returns the max number of objects that can be allocated, or INT32_MAX
	//! for paged pools as they can grow indefinitely. Is constant time.
	uint32_t Size() const { return m_paged ? INT32_MAX : m_pageSize; }
	//! Returns the maximum number of objects per page. Is constant time.
	uint32_t PageSize() const { return m_pageSize; }

private:
	struct Page; // Internal forward declaration
public:
	template< typename T >
	class Iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using reference = T&;
		using pointer = T*;
		Iterator() = default;
		Iterator( Iterator& ) = default;
		Iterator( const OpaquePool* pool, const struct Page* page, pointer ptr, uint32_t seq );
		reference operator*() { return *m_ptr; }
		pointer operator->() { return m_ptr; }
		const T& operator*() const { return *m_ptr; }
		const pointer operator->() const { return m_ptr; }
		friend bool operator== ( const Iterator& a, const Iterator& b ) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= ( const Iterator& a, const Iterator& b ) { return !( a == b ); };
		Iterator& operator++();
		Iterator operator++( int );
		Iterator begin();
		Iterator end();
	private:
		pointer m_ptr = nullptr;
		const struct Page* m_page = nullptr;
		const OpaquePool* m_pool = nullptr;
		uint32_t m_seq = 0;
	};
	//! Returns an ae::OpaquePool::Iterator which is stl conformant.
	template< typename T > Iterator< T > Iterate();
	//! Returns an ae::OpaquePool::Iterator which is stl conformant.
	template< typename T > Iterator< const T > Iterate() const;

private:
	template< typename T > friend class Iterator;
	OpaquePool( OpaquePool& other ) = delete;
	void operator=( OpaquePool& other ) = delete;
	struct Page
	{
		// Pages are deleted by the pool when empty, so it's safe to
		// assume pages always contain at least one object.
		Page( const ae::Tag& tag, uint32_t size ) : freeList( tag, size ) {}
		ae::ListNode< Page > node = this; // List node.
		ae::FreeList<> freeList; // Free object information.
		void* objects; // Pointer to array of objects in this page.
	};
	const void* m_GetFirst() const;
	const void* m_GetNext( const Page*& page, const void* obj, uint32_t seq ) const;
	ae::Tag m_tag;
	uint32_t m_pageSize; // Number of objects per page.
	bool m_paged; // If true, pool can be infinitely big.
	uint32_t m_objectSize; // Size of each object.
	uint32_t m_objectAlignment; // Alignment of each object.
	uint32_t m_length; // Number of actively allocated objects.
	ae::List< Page > m_pages;
	Page m_firstPage;
	uint32_t m_seq; // Tracks the number of pool operations, used for iterator safety.
};

//! @} End DataStructures defgroup

//------------------------------------------------------------------------------
// ae::Rect class
// @TODO: Move this up near Vec3 etc
//------------------------------------------------------------------------------
class Rect
{
public:
	Rect() = default;
	static Rect FromCenterAndSize( ae::Vec2 center, ae::Vec2 size );
	static Rect FromPoints( ae::Vec2 p0, ae::Vec2 p1 );

	Vec2 GetMin() const { return m_min; }
	Vec2 GetMax() const { return m_max; }
	Vec2 GetSize() const { return m_max - m_min; }
	Vec2 GetCenter() const { return ( m_min + m_max ) * 0.5f; }
	float GetWidth() const { return m_max.x - m_min.x; }
	float GetHeight() const { return m_max.y - m_min.y; }
	bool Contains( Vec2 pos ) const;
	Vec2 Clip( Vec2 pos ) const;
	void ExpandPoint( Vec2 pos );
	void ExpandEdge( Vec2 amount );
	bool GetIntersection( const Rect& other, Rect* intersectionOut = nullptr ) const;

	Rect operator*( float s ) const;
	Rect operator/( float s ) const;
	Rect operator+( const Vec2& v ) const;
	Rect operator-( const Vec2& v ) const;
	Rect operator*( const Vec2& v ) const;
	Rect operator/( const Vec2& v ) const;
	void operator*=( float s );
	void operator/=( float s );
	void operator+=( const Vec2& v );
	void operator-=( const Vec2& v );
	void operator*=( const Vec2& v );
	void operator/=( const Vec2& v );

	//! Define conversion functions etc for ae::Rect. See AE_CONFIG_FILE for more info.
#ifdef AE_RECT_CLASS_CONFIG
	AE_RECT_CLASS_CONFIG
#endif
	
private:
	friend std::ostream& operator<<( std::ostream& os, Rect r );
	ae::Vec2 m_min = ae::Vec2( INFINITY );
	ae::Vec2 m_max = ae::Vec2( -INFINITY );
};
inline std::ostream& operator<<( std::ostream& os, Rect r )
{
	return os << r.m_min.x << " " << r.m_min.y << " " << r.m_max.x << " " << r.m_max.y;
}

//------------------------------------------------------------------------------
// ae::RectInt class
// @TODO: Move this up near Vec3 etc
//------------------------------------------------------------------------------
struct RectInt
{
	//! Create an ae::RectInt from a corner position and size (non-inclusive).
	//! ie. ae::RectInt::Contains( pos + size ) will always return false.
	//! ae::RectInt::Contains( pos ) will return true only if size is non-zero.
	//! @TODO: Handle negative size.
	static RectInt FromPointAndSize( ae::Int2 pos, ae::Int2 size );
	//! Create an ae::RectInt from a corner position and size (non-inclusive).
	//! ie. ae::RectInt::Contains( pos + size ) will always return false.
	//! ae::RectInt::Contains( pos ) will return true only if size is non-zero.
	//! @TODO: Handle negative size.
	static RectInt FromPointAndSize( int32_t x, int32_t y, int32_t w, int32_t h );
	//! Create an ae::RectInt from points (inclusive) ie. ae::RectInt::Contains()
	//! will always return true for both points.
	static RectInt FromPoints( ae::Int2 p0, ae::Int2 p1 );
	//! Create ae::RectInt from points (inclusive) ie. ae::RectInt::Contains()
	//! will always return true for both points.
	static RectInt FromPoints( int32_t x0, int32_t y0, int32_t x1, int32_t y1 );
	// No FromCenterAndSize() function because the intended result is ambiguous
	// when width or height is odd.

	ae::Int2 GetPos() const { return ae::Int2( x, y ); }
	ae::Int2 GetSize() const { return ae::Int2( w, h ); }
	uint32_t GetWidth() const { return (uint32_t)w; }
	uint32_t GetHeight() const { return (uint32_t)h; }
	bool Contains( ae::Int2 pos ) const;
	bool Intersects( RectInt other ) const;
	//! Expand ae::RectInt by point (inclusive), ie. ae::RectInt::Contains()
	//! will return true. A zero size ae::RectInt is expanded so its width and
	//! height are 1.
	void Expand( ae::Int2 pos );

private:
	friend std::ostream& operator<<( std::ostream& os, RectInt r );
	// Private so w and h are never negative
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;
};
inline std::ostream& operator<<( std::ostream& os, RectInt r )
{
	return os << r.x << " " << r.y << " " << r.w << " " << r.h;
}

//------------------------------------------------------------------------------
// ae::BVHNode struct
//------------------------------------------------------------------------------
struct BVHNode
{
	ae::AABB aabb;
	int16_t parentIdx = -1;
	int16_t leftIdx = -1;
	int16_t rightIdx = -1;
	int16_t leafIdx = -1;
};

//------------------------------------------------------------------------------
// ae::BVHLeaf struct
//------------------------------------------------------------------------------
//! ae::BVHLeaf's store all user provided data. When automatically building the
//! bvh ae::BVHLeaf::count is automatically determined by the number of given
//! nodes, the more that are available the lower count will be. The tree bvh
//! represents the structure of the given data best when ae::BVHLeaf::count is
//! low.
//------------------------------------------------------------------------------
template< typename T >
struct BVHLeaf
{
	T* data;
	uint32_t count;
};

//------------------------------------------------------------------------------
// ae::BVH class
//------------------------------------------------------------------------------
template< typename T, uint32_t N = 0 >
class BVH
{
public:
	BVH(); //!< Static (N > 0)(constructor 1)
	BVH( const ae::Tag& allocTag ); //!< Dynamic (N == 0)(constructor 2)
	BVH( const ae::Tag& allocTag, uint32_t nodeLimit ); //!< Dynamic (N == 0)(constructor 3)
	BVH( const BVH& other );
	BVH& operator = ( const BVH& other );

	//! Builds an ae::BVH for the given \p data. \p data elements are not copied
	//! into the ae::BVH, so the given \p data lifetime must be greater than the
	//! lifetime of this ae::BVH. In addition to this \p data will be
	//! reorganized so it can be accessed contiguously with ae::BVHLeaf::data
	//! and ae::BVHLeaf::count. \p count should be the number of \p data
	//! elements to be processed. \p aabbFn should roughly have the signature
	//! ae::AABB()( const T& elem ), but it's valid to return anything that
	//! can be converted to an ae::AABB (like an ae::Sphere). \p targetLeafCount
	//! optionally specifies a stopping point to limit tree depth. It's possible
	//! ae::BVHLeaf::count will be less than \p targetLeafCount (but at least 1)
	//! if the data is unbalanced, or more if nodes are limited.
	template< typename AABBFn >
	void Build( T* data, uint32_t count, AABBFn aabbFn, uint32_t targetLeafCount = 0 );

	//! Add two child nodes to the given node at \p parentIdx. The index of the
	//! root node is 0. The given \p leftAABB and \p rightAABB will determine
	//! the aabb of their parent. The returned values are the node index of the
	//! left and right nodes respectively. It is not safe to use previous
	//! pointers to BVHNodes after calling this if using constructor 2.
	std::pair< int32_t, int32_t > AddNodes( int32_t parentIdx, const ae::AABB& leftAABB, const ae::AABB& rightAABB );
	//! Sets the leaf data of the node at \p nodeIdx
	void SetLeaf( int32_t nodeIdx, T* data, uint32_t count );
	//! Resets BVH to state directly after construction. Does not affect node limit.
	void Clear();
	
	//! Returns the aabb that contains all node aabbs
	ae::AABB GetAABB() const;
	//! Returns the root node or null if ae::BVH::AddNodes() has not been called yet.
	const BVHNode* GetRoot() const;
	//! Get the node at \p nodeIdx. Corresponds to ae::BVHNode::parentIdx,
	//! ae::BVHNode::leftIdx, and ae::BVHNode::rightIdx.
	const BVHNode* GetNode( int32_t nodeIdx ) const;
	//! Get the leaf at \p leafIdx. Corresponds to ae::BVHNode::leafIdx.
	const BVHLeaf< T >& GetLeaf( int32_t leafIdx ) const;
	//! Returns the leaf at \p leafIdx or null if it does not exist. Corresponds
	//! to ae::BVHNode::leafIdx.
	const BVHLeaf< T >* TryGetLeaf( int32_t leafIdx ) const;

	//! Returns the remaining number of nodes, or 0 if no limit was specified
	uint32_t GetAvailable() const { return m_limit ? m_limit - ae::Max( 1u, m_nodes.Length() ): 0; }
	//! Returns the max number of nodes, or 0 if no limit was specified
	uint32_t GetLimit() const { return m_limit; }

private:
	template< typename AABBFn >
	void m_Build( T* data, uint32_t count, AABBFn aabbFn, uint32_t targetLeafCount, int32_t bvhNodeIdx, uint32_t availableNodes );
	uint32_t m_limit = 0;
	ae::Array< BVHNode, N > m_nodes;
	ae::Array< BVHLeaf< T >, (N + 1)/2 > m_leaves;
};

//------------------------------------------------------------------------------
// Log utilities
//------------------------------------------------------------------------------
//! Enables output color codes in all logs. This is off by default as many debug
//! consoles render escaped characters instead of interpreting them, so this
//! should only be used when the output is known to support color codes.
void SetLogColorsEnabled( bool enabled );
//! Prepends the given message to all following logs and asserts on the current
//! thread until ae::PopLogTag() is called. Multiple calls to ae::PushLogTag() will
//! stack, so all messages will be prepended to the log messages in the order
//! they were pushed, separated by spaces. This is useful for grouping logs
//! together or tagging all logs in a specific scope. Take care to match every
//! call to ae::PushLogTag() with a call to ae::PopLogTag().
template< typename... Args > void PushLogTag( const char* format, Args... args );
//! Pops the last log prefix set by ae::PushLogTag() on the current thread. Take
//! care to match every call to ae::PushLogTag() with a call to ae::PopLogTag().
void PopLogTag();

//------------------------------------------------------------------------------
// Logging types
//------------------------------------------------------------------------------
enum class LogSeverity
{
	Fatal = 0,
	Error,
	Warning,
	Info,
	Debug,
	Trace,
};
typedef void (*LogFn)( ae::LogSeverity severity, const char* filePath, uint32_t line, const char* message );

} // ae end

//------------------------------------------------------------------------------
// Logging functions
//------------------------------------------------------------------------------
#define _AE_SRCCHK( _v, _d ) ( AE_ENABLE_SOURCE_INFO ? _v : _d ) // Internal usage
// clang-format off
#define AE_LOG(...) ae::_Log( ae::LogSeverity::Info, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )
#define AE_TRACE(...) ae::_Log( ae::LogSeverity::Trace, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )
#define AE_DEBUG(...) ae::_Log( ae::LogSeverity::Debug, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )
#define AE_INFO(...) ae::_Log( ae::LogSeverity::Info, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )
#define AE_WARN(...) ae::_Log( ae::LogSeverity::Warning, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )
#define AE_WARNING(...) ae::_Log( ae::LogSeverity::Warning, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )
#define AE_ERR(...) ae::_Log( ae::LogSeverity::Error, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )
#define AE_ERROR(...) ae::_Log( ae::LogSeverity::Error, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ )

//------------------------------------------------------------------------------
// Assertion functions
//------------------------------------------------------------------------------
#ifndef AE_ASSERT_IMPL
	#define AE_ASSERT_IMPL( msgStr ) { if( (msgStr)[ 0 ] && !ae::IsDebuggerAttached() ) { ae::ShowMessage( msgStr ); } AE_BREAK(); }
#endif
// @TODO: Use __analysis_assume( x ); on windows to prevent warning C6011 (Dereferencing NULL pointer)
#define AE_ASSERT( _x ) do { if( !(_x) ) { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "AE_ASSERT( " #_x " )", "" ); AE_ASSERT_IMPL( msgStr.c_str() ); } } while(0)
#define AE_ASSERT_MSG( _x, ... ) do { if( !(_x) ) { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "AE_ASSERT_MSG( " #_x " )", __VA_ARGS__ ); AE_ASSERT_IMPL( msgStr.c_str() ); } } while(0)
#if _AE_DEBUG_
	#define AE_DEBUG_ASSERT( _x ) do { if( !(_x) ) { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "AE_DEBUG_ASSERT( " #_x " )", "" ); AE_ASSERT_IMPL( msgStr.c_str() ); } } while(0)
	#define AE_DEBUG_ASSERT_MSG( _x, ... ) do { if( !(_x) ) { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "AE_DEBUG_ASSERT_MSG( " #_x " )", __VA_ARGS__ ); AE_ASSERT_IMPL( msgStr.c_str() ); } } while(0)
#else
	#define AE_DEBUG_ASSERT( _x ) do {} while(0)
	#define AE_DEBUG_ASSERT_MSG( _x, ... ) do {} while(0)
#endif
#define AE_FAIL() do { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", "" ); AE_ASSERT_IMPL( msgStr.c_str() ); } while(0)
#define AE_FAIL_MSG( ... ) do { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ ); AE_ASSERT_IMPL( msgStr.c_str() ); } while(0)
#if _AE_DEBUG_
	#define AE_DEBUG_FAIL() do { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", "" ); AE_ASSERT_IMPL( msgStr.c_str() ); } while(0)
	#define AE_DEBUG_FAIL_MSG( ... ) do { auto msgStr = ae::_Log( ae::LogSeverity::Fatal, _AE_SRCCHK(__FILE__,""), _AE_SRCCHK(__LINE__,0), "", __VA_ARGS__ ); AE_ASSERT_IMPL( msgStr.c_str() ); } while(0)
#else
	#define AE_DEBUG_FAIL() do {} while(0)
	#define AE_DEBUG_FAIL_MSG( ... ) do {} while(0)
#endif

//------------------------------------------------------------------------------
// Static assertion functions
//------------------------------------------------------------------------------
#define AE_STATIC_ASSERT( _x ) static_assert( _x, "static assert" )
#define AE_STATIC_ASSERT_MSG( _x, _m ) static_assert( _x, _m )
#define AE_STATIC_FAIL( _m ) static_assert( 0, _m )

//------------------------------------------------------------------------------
// AE_DEBUG_IF
//------------------------------------------------------------------------------
#if _AE_DEBUG_
	#define AE_DEBUG_IF( _expr ) if( _expr )
#else
	#define AE_DEBUG_IF( _expr ) if constexpr( false )
#endif
// clang-format on

//------------------------------------------------------------------------------
// Handle missing 'standard' C functions
//------------------------------------------------------------------------------
#ifndef HAVE_STRLCAT
inline size_t strlcat( char* dst, const char* src, size_t size )
{
	size_t dstlen = strlen( dst );
	size -= dstlen + 1;

	if( !size )
	{
		return dstlen;
	}

	size_t srclen = strlen( src );
	if( srclen > size )
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
	if( srclen > size )
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
// AE_EXPORT
//------------------------------------------------------------------------------
//! When building a hot loadable shared library for use with ae::HotLoader it is
//! recommended to use the the following compiler flags when building, and this
//! macro with any functions that will be called with ae::HotLoader::CallFn().
//! This will stop the compiler from unintentionally exporting functions that
//! could prevent the dynamic library from unloading.
//! clang++: -fvisibility=hidden
//! cl: @TODO
//! g++: @TODO
#if _AE_WINDOWS_
	#define AE_EXPORT
#else
	#define AE_EXPORT extern "C" __attribute__((visibility("default")))
#endif
// Shared library AE_EXPORT example:
#if 0
	AE_EXPORT bool Game_Update( Game* game )
	{
		// ...
	}
#endif
// Main executable ae::HotLoader::CallFn example:
#if 0
	while( hotLoader.CallFn< bool(*)( Game* ) >( "Game_Update", &game ) )
	{
		// ...
	}
#endif

//------------------------------------------------------------------------------
// ae::HotLoader class
//------------------------------------------------------------------------------
//! Used to dynamically reload a shared library. The shared library should
//! use AE_EXPORT to export any functions called with ae::HotLoader::CallFn().
class HotLoader
{
public:
	~HotLoader();
	void Initialize( const char* buildCmd, const char* postBuildCmd, const char* libPath );
	void Reload();
	void Close();
	bool IsLoaded() const { return m_dylib != nullptr; }

	template< typename Fn, typename... Args >
	decltype(auto) CallFn( const char* name, Args... args );

	// Static helpers
	static bool GetCMakeBuildCommand( ae::Str256* buildCmdOut, const char* cmakeBuildDir, const char* cmakeTargetName );
	static bool GetCopyCommand( ae::Str256* copyCmdOut, const char* dest, const char* src );

private:
	void* m_LoadFn( const char* name );
	void* m_dylib = nullptr;
	ae::Str256 m_buildCmd;
	ae::Str256 m_postBuildCmd;
	ae::Str256 m_libPath;
	ae::Map< ae::Str64, void*, 16 > m_fns;
};

//------------------------------------------------------------------------------
// ae::RunOnDestroy
// Usage:
/*
	FILE* f = fopen( "file.txt", "w" );
	ae::RunOnDestroy closeFile = [&]()
	{
		if( f )
		{
			fclose( f );
		}
	};
	// Do things with the file...
*/
//------------------------------------------------------------------------------
template< typename T >
struct RunOnDestroy
{
	RunOnDestroy( T&& func ) : m_func( std::forward< T >( func ) ) {}
	~RunOnDestroy() { m_func(); }
	T m_func;
};

//------------------------------------------------------------------------------
// ae::Screen
//------------------------------------------------------------------------------
//! Screen information. ae::Screen member values are in the same coordinate
//! space as ae::Window.
struct Screen
{
	ae::Int2 position;
	ae::Int2 size;
	bool isPrimary = false; //!< True for the display with the start menu, dock, etc
	bool isExternal = false; //!< True if the screen is removable from the device
	float scaleFactor = 1.0f; //!< Display scale factor
};
//! Returns an array of all available screens. If a system error is encountered
//! the returned array will be empty.
ae::Array< ae::Screen, 16 > GetScreens();

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
	bool Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor, bool rememberPosition );
	//! Window size is specified in virtual DPI units, content size is subject to the displays scale factor
	bool Initialize( Int2 pos, uint32_t width, uint32_t height, bool showCursor, bool rememberPosition );
	void Terminate();

	void SetTitle( const char* title );
	void SetFullScreen( bool fullScreen );
	void SetPosition( Int2 pos );
	//! Window size is specified in virtual DPI units, content size is subject to the displays scale factor
	void SetSize( uint32_t width, uint32_t height );
	void SetMaximized( bool maximized );
	void SetAlwaysOnTop( bool alwaysOnTop );
	// void SetMode( ae::WindowMode mode ); // @TODO: Replace SetFullScreen() and SetMaximized() with this

	const char* GetTitle() const { return m_windowTitle.c_str(); }
	bool GetFullScreen() const { return m_fullScreen; }
	bool GetMaximized() const { return m_maximized; }
	//! True if the user is currently working with this window
	bool GetFocused() const { return m_focused; }
	Int2 GetPosition() const { return m_pos; }
	//! Virtual window width (unscaled by display scale factor)
	int32_t GetWidth() const;
	//! Virtual window height (unscaled by display scale factor)
	int32_t GetHeight() const;
	//! Window content scale factor
	float GetScaleFactor() const { return m_scaleFactor; }

	//! Enable window events logging to console
	void SetLoggingEnabled( bool enable ) { m_debugLog = enable; }
	bool GetLoggingEnabled() const { return m_debugLog; }

private:
	Window( const Window& ) = delete;
	void m_Initialize( bool rememberPosition );
	Int2 m_pos = Int2( 0 );
	int32_t m_width = 0;
	int32_t m_height = 0;
	RectInt m_restoreRect; // Last pos/size before fullscreen
	bool m_fullScreen = false;
	bool m_maximized = false;
	bool m_focused = false;
	float m_scaleFactor = 1.0f;
	Str256 m_windowTitle;
	bool m_debugLog = false;
public:
	// Internal
	void m_UpdatePos( Int2 pos ) { m_pos = pos; }
	void m_UpdateSize( int32_t width, int32_t height, float scaleFactor );
	void m_UpdateMaximized( bool maximized ) { m_maximized = maximized; }
	void m_UpdateFullScreen( bool fullScreen ) { m_fullScreen = fullScreen; }
	void m_UpdateFocused( bool focused );
	ae::Int2 m_aeToNative( ae::Int2 pos, ae::Int2 size );
	ae::Int2 m_nativeToAe( ae::Int2 pos, ae::Int2 size );
	bool m_fixCanvasStyle = false;
	void* window = nullptr;
	class GraphicsDevice* graphicsDevice = nullptr;
	class Input* input = nullptr;
};

//------------------------------------------------------------------------------
// ae::Key enum
//------------------------------------------------------------------------------
enum class Key : uint8_t
{
	None = 0,

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

	Control = 249,
	Shift = 250,
	Alt = 251,
	Super = 252,
	Meta = 253,
	LeftMeta = 254, // Command on Apple, Control on others
	RightMeta = 255, // Command on Apple, Control on others
};
// @TODO: ae::Key ToString

//------------------------------------------------------------------------------
// ae::MouseState struct
//------------------------------------------------------------------------------
struct MouseState
{
	bool leftButton = false;
	bool middleButton = false;
	bool rightButton = false;
	//! Window space coordinates (ie. not affected by window scale factor). This
	//! value should be used for cursors etc. and not for calculating changes in
	//! position. In other words don't subtract mouse position from a previous
	//! frame. Use ae::MouseState::movement for changes in position.
	ae::Int2 position = ae::Int2( 0 );
	//! Window space coordinates (ie. not affected by window scale factor). This
	//! value should be used for detecting how much the mouse cursor is moved.
	//! Cursor jumps are filtered when the mouse is captured and when the window
	//! becomes active.
	ae::Int2 movement = ae::Int2( 0 );
	ae::Vec2 scroll = ae::Vec2( 0.0f );
	bool usingTouch = false;
};

//------------------------------------------------------------------------------
// ae::GamepadState struct
//------------------------------------------------------------------------------
// @TODO: Add or replace this with ae::Button/ae::Stick/ae::Trigger like ae::Key
struct GamepadState // @TODO: Rename Gamepad
{
	int32_t playerIndex = -1;
	bool connected = false;
	bool anyInput = false;
	bool anyButton = false;
	
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
	
	bool leftBumper = false;
	bool rightBumper = false;
	float leftTrigger = 0.0f;
	float rightTrigger = 0.0f;
	bool leftAnalogClick = false;
	bool rightAnalogClick = false;
	
	enum class BatteryState
	{
		None,
		InUse,
		Charging,
		Full,
		Wired
	};
	BatteryState batteryState = BatteryState::None;
	float batteryLevel = 0.0f;
};

//------------------------------------------------------------------------------
// ae::Touch struct
//------------------------------------------------------------------------------
struct Touch
{
	uint32_t id = 0;
	ae::Int2 startPosition = ae::Int2( 0.0f );
	ae::Int2 position = ae::Int2( 0.0f );
	ae::Int2 movement = ae::Int2( 0.0f );
};
const uint32_t kMaxTouches = 32; //!< Max number of touches supported by ae::Input
using TouchArray = ae::Array< ae::Touch, ae::kMaxTouches >;

//------------------------------------------------------------------------------
// ae::Input class
//------------------------------------------------------------------------------
class Input
{
#define AE_INPUT_PRESS( value, property ) value.property && !value##Prev.property
#define AE_INPUT_RELEASE( value, property ) !value.property && value##Prev.property
	
public:
	Input();
	void Initialize( Window* window );
	void Terminate();
	void Pump();
	
	//! Locks cursor to center of window if it is focused. Use mouse.movement to get input information. Mouse capture is automatically released when the window loses focus. This can be checked with Input::GetMouseCaptured(). Automatically hides the cursor.
	void SetMouseCaptured( bool enable );
	//! Returns true if the mouse is currently captured. Always returns false when the window does not have focus.
	bool GetMouseCaptured() const { return m_captureMouse; }
	//! Passing true enables gamepad input only when window is focused, otherwise gamepad input is always available. Default is true.
	void SetGamepadRequiresFocus( bool enable ) { m_gamepadRequiresFocus = enable; }
	//! Returns the current value of SetGamepadRequiresFocus().
	bool GetGamepadRequiresFocus() const { return m_gamepadRequiresFocus; }
	//! Hides the cursor
	void SetCursorHidden( bool hidden ) { m_hideCursor = hidden; }
	//! Returns true if the cursor is hidden
	bool GetCursorHidden() const { return m_hideCursor; }
	
	void SetTextMode( bool enabled );
	bool GetTextMode() const { return m_textMode; }
	void SetText( const char* text ) { m_text = text; }
	const char* GetText() const { return m_text.c_str(); }
	const char* GetTextInput() const { return m_textInput.c_str(); }
	
	void SetLeftAnalogThreshold( float threshold ) { m_leftAnalogThreshold = threshold; }
	void SetRightAnalogThreshold( float threshold ) { m_rightAnalogThreshold = threshold; }
	float GetLeftAnalogThreshold() { return m_leftAnalogThreshold; }
	float GetRightAnalogThreshold() { return m_rightAnalogThreshold; }
	
	bool Get( ae::Key key ) const;
	bool GetPrev( ae::Key key ) const;
	inline bool GetPress( ae::Key key ) const { return Get( key ) && !GetPrev( key ); }
	inline bool GetRelease( ae::Key key ) const { return !Get( key ) && GetPrev( key ); }
	
	inline bool GetMousePressLeft() const { return AE_INPUT_PRESS( mouse, leftButton ); }
	inline bool GetMousePressMid() const { return AE_INPUT_PRESS( mouse, middleButton ); }
	inline bool GetMousePressRight() const { return AE_INPUT_PRESS( mouse, rightButton ); }
	inline bool GetMouseReleaseLeft() const { return AE_INPUT_RELEASE( mouse, leftButton ); }
	inline bool GetMouseReleaseMid() const { return AE_INPUT_RELEASE( mouse, middleButton ); }
	inline bool GetMouseReleaseRight() const { return AE_INPUT_RELEASE( mouse, rightButton ); }
	
	inline bool GetGamepadPressA( uint32_t idx = 0 ) const { return gamepads[ idx ].a && !gamepadsPrev[ idx ].a; }
	inline bool GetGamepadPressB( uint32_t idx = 0 ) const { return gamepads[ idx ].b && !gamepadsPrev[ idx ].b; }
	inline bool GetGamepadPressX( uint32_t idx = 0 ) const { return gamepads[ idx ].x && !gamepadsPrev[ idx ].x; }
	inline bool GetGamepadPressY( uint32_t idx = 0 ) const { return gamepads[ idx ].y && !gamepadsPrev[ idx ].y; }
	inline bool GetGamepadPressStart( uint32_t idx = 0 ) const { return gamepads[ idx ].start && !gamepadsPrev[ idx ].start; }
	inline bool GetGamepadPressSelect( uint32_t idx = 0 ) const { return gamepads[ idx ].select && !gamepadsPrev[ idx ].select; }
	inline bool GetGamepadPressUp( uint32_t idx = 0 ) const { return gamepads[ idx ].up && !gamepadsPrev[ idx ].up; }
	inline bool GetGamepadPressDown( uint32_t idx = 0 ) const { return gamepads[ idx ].down && !gamepadsPrev[ idx ].down; }
	inline bool GetGamepadPressLeft( uint32_t idx = 0 ) const { return gamepads[ idx ].left && !gamepadsPrev[ idx ].left; }
	inline bool GetGamepadPressRight( uint32_t idx = 0 ) const { return gamepads[ idx ].right && !gamepadsPrev[ idx ].right; }

	//! Returns an active touch with the given \p id or nullptr if it does not
	//! exist
	const ae::Touch* GetTouchById( uint32_t id ) const;
	//! Returns a touch that was just released with the given \p id or nullptr
	//! if it does not exist
	const ae::Touch* GetFinishedTouchById( uint32_t id ) const;
	//! Returns all touches that have stated since the last ae::Input::Pump()
	//! call
	ae::TouchArray GetNewTouches() const;
	//! Returns all touches that have been released since the last
	//! ae::Input::Pump() call
	ae::TouchArray GetFinishedTouches() const;
	//! Returns all touches that are currently active
	const ae::TouchArray& GetTouches() const;
	//! Returns all touches that were active in the previous frame
	const ae::TouchArray& GetPreviousTouches() const;
	
	MouseState mouse;
	MouseState mousePrev;
	GamepadState gamepads[ 4 ];
	GamepadState gamepadsPrev[ 4 ];
	bool quit = false;

// private:
	Input( const Input& ) = delete;
	void m_SetMousePos( ae::Int2 pos );
	void m_SetMousePos( ae::Int2 pos, ae::Int2 movement );
	void m_SetCursorPos( ae::Int2 pos );
	void m_SetMouseCaptured( bool captured );
	void m_UpdateModifiers();
	ae::TimeStep m_timeStep;
	ae::Window* m_window = nullptr;
	bool m_captureMouse = false;
	ae::Int2 m_capturedMousePos = ae::Int2( INT_MAX );
	bool m_mousePosSet = false;
	bool m_hideCursor = false;
	bool m_keys[ 256 ];
	bool m_keysPrev[ 256 ];
	bool m_textMode = false;
	void* m_textInputHandler = nullptr;
	std::string m_text;
	std::string m_textInput;
	float m_leftAnalogThreshold = 0.1f;
	float m_rightAnalogThreshold = 0.1f;
	bool m_gamepadRequiresFocus = true;
	// Touch
	ae::TouchArray m_touches;
	ae::TouchArray m_touchesPrev;
	uint32_t m_touchIndex = 0; // 0 is invalid
	// Emscripten
	bool newFrame_HACK = false;
};

/* Internal */ } extern "C" { void _ae_FileSystem_ReadSuccess( void* arg, void* data, uint32_t length ); void _ae_FileSystem_ReadFail( void* arg, uint32_t code, bool timeout ); } namespace ae {
//------------------------------------------------------------------------------
// ae::File class
//! \brief Used to asynchronously load data from remote sources.
//------------------------------------------------------------------------------
class File
{
public:
	enum class Status
	{
		Success,
		Pending,
		NotFound,
		Timeout,
		Error
	};

	const char* GetUrl() const;
	Status GetStatus() const;
	//! Platform specific error code eg. 200, 404, etc. for http
	uint32_t GetCode() const;
	//! Null terminated for convenience
	const uint8_t* GetData() const;
	uint32_t GetLength() const;
	float GetElapsedTime() const;
	float GetTimeout() const;
	uint32_t GetRetryCount() const;

private:
	friend void ::_ae_FileSystem_ReadSuccess( void* arg, void* data, uint32_t length );
	friend void ::_ae_FileSystem_ReadFail( void* arg, uint32_t code, bool timeout );
	friend class FileSystem;
	ae::Str256 m_url;
	uint8_t* m_data = nullptr;
	uint32_t m_length = 0;
	Status m_status = Status::Pending;
	uint32_t m_code = 0;
	double m_startTime = 0.0;
	double m_finishTime = 0.0;
	float m_timeout = 0.0f;
	uint32_t m_retryCount = 0;
};

//------------------------------------------------------------------------------
// ae::FileFilter for ae::FileDialogParams
//------------------------------------------------------------------------------
struct FileFilter
{
	FileFilter() = default;
	FileFilter( const char* desc, const char* ext ) : description( desc ) { extensions[ 0 ] = ext; }
	FileFilter( const char* desc, const char** ext, uint32_t extensionCount );
	ae::Str64 description = ""; // "JPEG Image"
	// Only alphanumeric extension strings are supported (with the exception of "*")
	ae::Str16 extensions[ 8 ]; // { "jpg", "jpeg", "jpe" }
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
	//! ae::FileSystem::Destroy() or ae::FileSystem::DestroyAll() must be called
	//! to free all ae::File's before an ae::FileSystem is destroyed.
	~FileSystem();
	//! Represents directories that the FileSystem class can load/save from.
	enum class Root
	{
		Bundle, //!< The path to the app bundle on Apple platforms or the executable directory on other platforms
		Data, //!< A given existing directory
		User, //!< A directory for storing preferences and savedata
		Cache, //!< A directory for storing expensive to generate data (computed, downloaded, etc)
		UserShared, //!< Same as above but shared accross the 'organization name'
		CacheShared //!< Same as above but shared accross the 'organization name'
	};
	
	//! If \p dataDir is absolute no processing on the path will be done. Passing
	//! an empty string or relative path to \p dataDir will cause a platform
	//! specific directory to be chosen as the base path. For bundled Apple applications
	//! the base path will be the 'Resources' bundle folder. In all other cases the base
	//! path will be relative to the executable (ignoring the working directory).
	//! Organization name should be your name or your companies name and should be
	//! consistent across apps. Application name should be the name of this application.
	//! Initialize() creates missing folders for Root::User and Root::Cache.
	void Initialize( const char* dataDir, const char* organizationName, const char* applicationName );

	// Asynchronous file loading
	//! Loads a file asynchronously from disk or from the network (@TODO: currently
	//! only in emscripten builds). <b>Prefer this function over all other
	//! ae::FileSystem::Read...() methods as it will work the most consistently
	//! on all platforms.</b> Returns an ae::File object to be freed later
	//! with ae::FileSystem::Destroy(). A zero or negative \p timeoutSec value
	//! will disable the timeout.
	const ae::File* Read( Root root, const char* url, float timeoutSec );
	//! Loads a file asynchronously from disk or from the network (@TODO: currently
	//! only in emscripten builds). Returns an ae::File object to be freed
	//! later with ae::FileSystem::Destroy(). A zero or negative \p timeoutSec
	//! value will disable the timeout.
	const ae::File* Read( const char* url, float timeoutSec );
	//! Retry if reading or writing of the given \p file did not finish
	//! successfully. It's recommended (but not necessary) that you call this
	//! function only when a file has the status ae::File::Status::Timeout, and
	//! then you might want back off with a longer \p timeoutSec. Calling this
	//! function on an ae::File that is successfully loaded or pending will have
	//! no effect.
	void Retry( const ae::File* file, float timeoutSec );
	//! Destroys the given ae::File object returned by ae::FileSystem::Read().
	void Destroy( const ae::File* file );
	//! Frees all existing ae::File objects. It is not safe to access any
	//! ae::File objects returned earlier from ae::FileSystem::Read() after
	//! calling this.
	void DestroyAll();
	//! Get a file read created with ae::FileSystem::Read().
	const ae::File* GetFile( uint32_t idx ) const;
	//! Returns the number of file reads iterable with ae::FileSystem::GetFile().
	uint32_t GetFileCount() const;
	//! Returns the number of file reads with the given \p status.
	uint32_t GetFileStatusCount( ae::File::Status status ) const;

	// Member functions for use of Root directories
	bool GetAbsolutePath( Root root, const char* filePath, Str256* outPath ) const;
	bool GetRootDir( Root root, Str256* outDir ) const;
	uint32_t GetSize( Root root, const char* filePath ) const;
	uint32_t Read( Root root, const char* filePath, void* buffer, uint32_t bufferSize ) const;
	uint32_t Write( Root root, const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs ) const;
	bool CreateFolder( Root root, const char* folderPath ) const;
	void ShowFolder( Root root, const char* folderPath ) const;

	// Static member functions intended to be used when not creating a instance
	static uint32_t GetSize( const char* filePath );
	static uint32_t Read( const char* filePath, void* buffer, uint32_t bufferSize );
	static uint32_t Write( const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs );
	static bool CreateFolder( const char* folderPath );
	static void ShowFolder( const char* folderPath );
	
	// Static helpers
	//! If \p filePath is absolute no processing on the path will be done. Passing
	//! an empty string or relative path to \p filePath will cause a platform
	//! specific directory to be chosen as the base path. For bundled Apple applications
	//! the base path will be the 'Resources' bundle folder. In all other cases the base
	//! path will be relative to the executable (ignoring the working directory).
	static Str256 GetAbsolutePath( const char* filePath );
	static bool IsAbsolutePath( const char* filePath );
	static const char* GetFileNameFromPath( const char* filePath );
	//! Returns the entire file extension in the case of multiple dots. If
	//! \p includeDot is true the returned string will include the first dot,
	//! which can be useful for creating a substring of the file name.
	static const char* GetFileExtFromPath( const char* filePath, bool includeDot = false );
	//! Returns a range within \p filePath to the first section of a path. Eg.
	//! "/User/Documents/file.txt" will allow the extraction of the directory
	//! "User" by returning a pointer to both "User/Documents/file.txt" and
	//! /Documents/file.txt". No exceptions are made for file names (ie. files
	//! with a '.' in the name). The path can be separated by forward or
	//! backward slashes.
	static std::pair< const char*, const char* > TraversePath( const char* filePath );
	static Str256 GetDirectoryFromPath( const char* filePath );
	static void AppendToPath( Str256* path, const char* str );
	//! Replaces the extension of the given path with \p ext. If the given path
	//! does not have an extension then \p ext will be appended to the path.
	//! \p ext must be only alphanumeric characters.
	static bool SetExtension( Str256* path, const char* ext );
	//! Returns true if the given path is a directory. This function does not
	//! access the underlying filesystem to see if the directory exists.
	static bool IsDirectory( const char* path );
	//! Removes redundant slashes and up-level references from the given path.
	//! This does not access the underlying filesystem to resolve symbolic links
	//! or verify the path exists.
	static void NormalizePath( Str256* path );

	// File dialogs
	static ae::Array< std::string > OpenDialog( const FileDialogParams& params );
	static std::string SaveDialog( const FileDialogParams& params );

private:
	void m_SetBundleDir();
	void m_SetDataDir( const char* dataDir );
	void m_SetUserDir( const char* organizationName, const char* applicationName );
	void m_SetCacheDir( const char* organizationName, const char* applicationName );
	void m_SetUserSharedDir( const char* organizationName );
	void m_SetCacheSharedDir( const char* organizationName );
	void m_Read( ae::File* file, float timeoutSec ) const;
	ae::Array< ae::File* > m_files = AE_ALLOC_TAG_FILE;
	ae::Str256 m_bundleDir;
	ae::Str256 m_dataDir;
	ae::Str256 m_userDir;
	ae::Str256 m_cacheDir;
	ae::Str256 m_userSharedDir;
	ae::Str256 m_cacheSharedDir;
};

//------------------------------------------------------------------------------
// ae::Socket class
//------------------------------------------------------------------------------
class Socket
{
public:
	enum class Protocol { None, TCP, UDP };

	Socket( ae::Tag tag );
	~Socket();

	//! Attempts to connect to the \p address over the given \p protocol. Calling
	//! ae::Socket::Connect() clears all pending sent and received data. To avoid
	//! losing received data call ae::Socket::ReceiveData() or ae::Socket::ReceiveMsg()
	//! repeatedly until they return empty before calling ae::Socket::Connect().
	//! In this scenario all pending sent data will always be lost.
	bool Connect( ae::Socket::Protocol protocol, const char* address, uint16_t port );
	//! Closes ths connection established with ae::Socket::Connect().
	void Disconnect();
	//! Returns true if the connection established with ae::Socket::Connect() or
	//! ae::ListenerSocket::Accept() is still active. If this ae::Socket was
	//! returned from ae::ListenerSocket::Accept() then ae::ListenerSocket::Destroy()
	//! or ae::ListenerSocket::DestroyAll() should be called to clean it up.
	//! This can return false while received data is still waiting to be read,
	//! and so ae::Socket::ReceiveData() or ae::Socket::ReceiveMsg() can still
	//! be called.
	bool IsConnected() const;

	//! Queues \p length data to be sent with ae::Socket::SendAll(). Call
	//! ae::Socket::QueueData() multiple times to batch data sent with
	//! ae::Socket::SendAll(). Data sent with ae::Socket::QueueData() can be read
	//! by the receiver with ae::Socket::PeekData() and ae::Socket::ReceiveData().
	//! It's advised that you do not mix ae::Socket::QueueMsg() and ae::Socket::QueueData().
	bool QueueData( const void* data, uint32_t length );
	//! Returns true if \p length + \p offset bytes have been received. If
	//! \p dataOut is non-null and \p length + \p offset bytes have been received
	//! the data at \p offset will be written to \p dataOut. The read head will
	//! not move, so subsequent calls to ae::Socket::PeekData() will return the
	//! same result. It's useful to call ae::Socket::DiscardData() and pass it
	//! \p length after receiving data through ae::Socket::PeekData().
	bool PeekData( void* dataOut, uint16_t length, uint32_t offset );
	//! Returns true if \p length bytes have been received. If
	//! \p dataOut is also non-null, pending received data at 'offset' will be
	//! written to \p dataOut. In this case the read head will move forward
	//! \p length bytes. Calling ae::Socket::ReceiveData() with a null \p dataOut
	//! and calling ae::Socket::DiscardData() has the exact same effect.
	bool ReceiveData( void* dataOut, uint16_t length );
	//! Returns true if \p length bytes have been received. In this case the read
	//! head will move forward \p length bytes. Calling ae::Socket::DiscardData()
	//! and calling ae::Socket::ReceiveData() with a null \p dataOut has the exact
	//! same effect.
	bool DiscardData( uint16_t length );
	//! Returns the number of bytes available for reading. This is mostly intended
	//! for use with ae::Socket::ReceiveData() when it is not possible to know
	//! how much data will be received in advance. If you are using ae::Socket::QueueMsg()
	//! and ae::Socket::ReceiveMsg() the returned value will include all message headers.
	uint32_t ReceiveDataLength();

	//! Queues data for sending. A two byte (network order) message header is
	//! prepended to the given message. Ideally you should call ae::Socket::QueueMsg()
	//! for each logical chunk of data you need to send per 'network tick'.
	//! Finally call ae::Socket::SendAll() once starting a new network tick. It's
	//! unadvised to mix ae::Socket::QueueMsg() calls with ae::Socket::QueueData().
	bool QueueMsg( const void* data, uint16_t length );
	//! Can return a value greater than maxLength, in which case \p dataOut is not
	//! modified. Call ae::Socket::ReceiveMessage() again with a big enough buffer
	//! or skip the message by calling ae::Socket::DiscardMessage(). Uses a two
	//! byte (network order) message header. It's unadvised to mix ae::Socket::ReceiveMsg()
	//! calls with ae::Socket::ReceiveData().
	uint16_t ReceiveMsg( void* dataOut, uint16_t maxLength );
	//! Discards one received message sent with ae::Socket::QueueMsg(). Uses the
	//! two byte (network order) message header to determine discard data size.
	bool DiscardMsg();

	//! Returns the number of bytes sent. Sends all queued data from ae::Socket::QueueData()
	//! and ae::Socket::QueueMsg(). If the connection is lost all pending sent
	//! data will be discarded. See ae::Socket::Connect() for more information.
	//! For a real time multiplayer game this could be called 10 to 30 times
	//! per second, each time all data and messages have been queued.
	uint32_t SendAll();
	
	//! Returns the most recent remote address that this socket had or attempted
	//! a connection to.
	const char* GetAddress() const { return m_address.c_str(); }
	//! Returns the resolved remote address that this socket last successfully
	//! connected to, unless a connection is in progress in which case this will
	//! return a zero length string. This will either be an IPv4 or IPv6 address.
	//! If ae::Socket::Connect() was given an ip address (as opposed to a hostname)
	//! ae::Socket::GetAddress() will likely return the same address.
	const char* GetResolvedAddress() const { return m_resolvedAddress.c_str(); }
	//! Returns the protocol that this socket is currently connected with or
	//! ae::Socket::Protocol::None if not connected.
	ae::Socket::Protocol GetProtocol() const { return m_protocol; }
	//! Returns the remote port that this socket is currently connected to or 0
	//! if not connected.
	uint16_t GetPort() const { return m_port; }

private:
	// Params
	ae::Socket::Protocol m_protocol = ae::Socket::Protocol::None;
	ae::Str128 m_address;
	uint16_t m_port = 0;
	// Connection state
	int m_sock = -1;
	bool m_isConnected = false;
	void* m_addrInfo = nullptr;
	void* m_currAddrInfo = nullptr;
	ae::Str128 m_resolvedAddress;
	// Data buffers
	uint32_t m_readHead = 0;
	ae::Array< uint8_t > m_sendData;
	ae::Array< uint8_t > m_recvData;
public: // Internal
	Socket( ae::Tag tag, int s, Protocol proto, const char* addr, uint16_t port );
};

//------------------------------------------------------------------------------
// ae::ListenerSocket class
//! Used in conjunction with ae::Socket to send data over UDP/TCP. Supports both
//! IPv4 and IPv6. See ae::ListenerSocket::Listen() for more detailed information
//! on usage.
//------------------------------------------------------------------------------
class ListenerSocket
{
public:
	ListenerSocket( ae::Tag tag );
	~ListenerSocket();

	//! Starts listening on the given port. Will accept both incoming ipv4 and
	//! ipv6 connections. Does not affect existing ae::Sockets allocated with
	//! ae::ListenerSocket::Accept(). \p maxConnections specifies how many
	//! active sockets can be returned by ae::ListenerSocket::Accept() before
	//! new connections will be rejected. Existing ae::Sockets that are
	//! disconnected count towards the \p maxConnections total, and must be
	//! cleaned up with ae::ListenerSocket::Destroy() before ae::ListenerSocket::Accept()
	//! will allow new connections. Providing false for \p allowRemote will
	//! prevent connections from other devices, and will also prevent firewall
	//! popups on most platforms.
	bool Listen( ae::Socket::Protocol proto, bool allowRemote, uint16_t port, uint32_t maxConnections );
	//! ae::ListenerSocket will no longer accept new connections. Does not affect
	//! existing ae::Sockets allocated with ae::ListenerSocket::Accept().
	void StopListening();
	//! Returns true if listening for either ipv4 or ipv6 connections.
	bool IsListening() const;
	
	//! Returns a socket if a connection has been established. See
	//! ae::ListenerSocket::Listen() for more information.
	ae::Socket* Accept();
	//! Disconnects and releases an existing socket from ae::ListenerSocket::Accept().
	void Destroy( ae::Socket* sock );
	//! Disconnects and releases all existing sockets from Accept(). It is not
	//! safe to access released sockets obtained through ae::ListenerSocket::Accept()
	//! after calling this.
	void DestroyAll();
	
	//! Returns ae::Socket by index allocated through ae::ListenerSocket::Accept().
	ae::Socket* GetConnection( uint32_t idx );
	//! Returns the number of ae::Sockets currently allocated through
	//! ae::ListenerSocket::Accept().
	uint32_t GetConnectionCount() const;
	//! Returns the protocol that this socket is currently listening with or
	//! ae::Socket::Protocol::None if not listening.
	ae::Socket::Protocol GetProtocol() const { return m_protocol; }
	//! Returns the local port that this socket is currently listening on or 0
	//! if not listening.
	uint16_t GetPort() const { return m_port; }

private:
	ae::Tag m_tag;
	int m_sock4 = -1;
	int m_sock6 = -1;
	ae::Socket::Protocol m_protocol = ae::Socket::Protocol::None;
	uint16_t m_port = 0;
	uint32_t m_maxConnections = 0;
	ae::Array< ae::Socket* > m_connections;
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
	ae::Hash32 GetHash() const { return m_hash; }

private:
	ae::Map< Str32, Value, 64 > m_uniforms;
	ae::Hash32 m_hash;
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
AE_UNIFORM_HIGHP mat4 u_worldToProj;

AE_IN_HIGHP vec3 a_position;
AE_IN_HIGHP vec2 a_uv;
AE_IN_HIGHP vec4 a_color;

AE_OUT_HIGHP vec2 v_uv;
AE_OUT_HIGHP vec4 v_color;

void main()
{
	v_uv = a_uv;
	v_color = a_color;
	gl_Position = u_worldToProj * vec4( a_position, 1.0 );
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
const uint32_t _kMaxShaderDefines = 32;
class InstanceData;
enum class Culling { None, ClockwiseFront, CounterclockwiseFront };
template<> inline uint32_t GetHash32( const Culling& v ) { return (uint32_t)v; } 

class Shader
{
public:
	enum class Type { Vertex, Fragment };
	~Shader();

	void Initialize( const char* vertexStr, const char* fragStr, const char* const* defines = nullptr, int32_t defineCount = 0 );
	void Terminate();
	//! Enable alpha blending. Defaults to false.
	void SetBlending( bool enabled ) { m_blending = enabled; }
	//! Enable depth testing. Defaults to false.
	void SetDepthTest( bool enabled ) { m_depthTest = enabled; }
	//! Enable writing to depth buffer. Defaults to false.
	void SetDepthWrite( bool enabled ) { m_depthWrite = enabled; }
	//! Sets the backface culling mode. ClockwiseFront front specifies that
	//! triangles with clockwise winding should be visible. CounterclockwiseFront
	//! front specifies that triangles with counter-clockwise winding should be
	//! visible. Defaults to None.
	void SetCulling( Culling culling ) { m_culling = culling; }
	//! Enable wireframe rendering mode. Defaults to false.
	void SetWireframe( bool enabled ) { m_wireframe = enabled; }
	void SetBlendingPremul( bool enabled ) { m_blendingPremul = enabled; }

	// Internal
private:
	int m_LoadShader( const char* shaderStr, Type type, const char* const* defines, int32_t defineCount );
	uint32_t m_fragmentShader = 0;
	uint32_t m_vertexShader = 0;
	uint32_t m_program = 0;
	bool m_blending = false;
	bool m_blendingPremul = false;
	bool m_depthTest = false;
	bool m_depthWrite = false;
	Culling m_culling = Culling::None;
	bool m_wireframe = false;
public:
	struct _Attribute
	{
		char name[ _kMaxShaderAttributeNameLength ];
		uint32_t type; // GL_FLOAT, GL_FLOAT_VEC4, GL_FLOAT_MAT4...
		int32_t location;
	};
	struct _Uniform
	{
		Str32 name;
		uint32_t type;
		int32_t location;
	};
private:
	ae::Array< _Attribute, _kMaxShaderAttributeCount > m_attributes;
	ae::Map< Str32, _Uniform > m_uniforms = AE_ALLOC_TAG_RENDER;
public:
	void m_Activate( const UniformList& uniforms ) const;
	const _Attribute* m_GetAttributeByIndex( uint32_t index ) const;
	uint32_t m_GetAttributeCount() const { return m_attributes.Length(); }
};

//------------------------------------------------------------------------------
// ae::Vertex types
//------------------------------------------------------------------------------
namespace Vertex
{
	// Constants
	enum class Usage { Dynamic, Static };
	enum class Type { UInt8, UInt16, UInt32, NormalizedUInt8, NormalizedUInt16, NormalizedUInt32, Float };
	//! Don't forget to set gl_PointSize in your vertex shader when using ae::Vertex::Primitive::Point.
	enum class Primitive { Point, Line, Triangle };
}

//------------------------------------------------------------------------------
// ae::VertexBuffer class
//------------------------------------------------------------------------------
class VertexBuffer
{
public:
	// Initialization
	VertexBuffer() = default;
	~VertexBuffer();
	void Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, ae::Vertex::Primitive primitive, ae::Vertex::Usage vertexUsage, ae::Vertex::Usage indexUsage );
	void AddAttribute( const char *name, uint32_t componentCount, ae::Vertex::Type type, uint32_t offset );
	void Terminate();
	
	//! Sends vertex data to the gpu.
	void UploadVertices( uint32_t startIdx, const void* vertices, uint32_t count );
	//! Sends index data to the gpu.
	void UploadIndices( uint32_t startIdx, const void* indices, uint32_t count );
	//! Call once directly before all calls to ae::VertexBuffer::Draw().
	void Bind( const ae::Shader* shader, const ae::UniformList& uniforms, const ae::InstanceData** instanceDatas = nullptr, uint32_t instanceDataCount = 0 ) const;
	//! Renders the entire buffer without instancing.
	void Draw() const;
	//! Renders a range of primitives without instancing (ie. \p primitiveCount of 1 to render a triangle).
	void Draw( uint32_t primitiveStartIdx, uint32_t primitiveCount ) const;
	//! Renders a range of primitives (ie. \p primitiveCount of 1 to render a
	//! triangle). \p instanceCount specifies the number of times to render the
	//! range of primitives. Supply ae::InstanceData to ae::VertexBuffer::Bind()
	//! before calling this function.
	void DrawInstanced( uint32_t primitiveStartIdx, uint32_t primitiveCount, uint32_t instanceCount ) const;
	
	uint32_t GetVertexSize() const { return m_vertexSize; }
	uint32_t GetIndexSize() const { return m_indexSize; }
	uint32_t GetMaxVertexCount() const { return m_maxVertexCount; }
	uint32_t GetMaxIndexCount() const { return m_maxIndexCount; }
	uint32_t GetMaxPrimitiveCount() const;
	ae::Vertex::Primitive GetPrimitiveType() const { return m_primitive; }
	ae::Vertex::Usage GetVertexUsage() const { return m_vertexUsage; }
	ae::Vertex::Usage GetIndexUsage() const { return m_indexUsage; }
	bool IsIndexed() const { return m_indexSize != 0; }

private:
	VertexBuffer( const VertexBuffer& ) = delete;
	VertexBuffer( VertexBuffer&& ) = delete;
	void operator=( const VertexBuffer& ) = delete;
	void operator=( VertexBuffer&& ) = delete;
	void m_Draw( uint32_t primitiveStartIdx, uint32_t primitiveCount, int32_t instanceCount ) const;
	// Init params
	uint32_t m_vertexSize = 0;
	uint32_t m_indexSize = 0;
	ae::Vertex::Primitive m_primitive = (ae::Vertex::Primitive)-1;
	ae::Vertex::Usage m_vertexUsage = (ae::Vertex::Usage)-1;
	ae::Vertex::Usage m_indexUsage = (ae::Vertex::Usage)-1;
	uint32_t m_maxVertexCount = 0;
	uint32_t m_maxIndexCount = 0;
	// System resources
	uint32_t m_array = 0;
	uint32_t m_vertices = ~0;
	uint32_t m_indices = ~0;
public:
	struct _Attribute
	{
		char name[ _kMaxShaderAttributeNameLength ];
		uint32_t componentCount;
		uint32_t type; // GL_BYTE, GL_SHORT, GL_FLOAT...
		uint32_t offset;
		bool normalized;
	};
	ae::Array< _Attribute, _kMaxShaderAttributeCount > m_attributes;
	uint32_t _GetAttributeCount() const { return m_attributes.Length(); }
	bool m_HasUploadedVertices() const { return m_vertices != ~0; }
	bool m_HasUploadedIndices() const { return m_indices != ~0; }
};

//------------------------------------------------------------------------------
// ae::VertexArray class
//------------------------------------------------------------------------------
class VertexArray
{
public:
	// Initialization
	VertexArray( ae::Tag tag );
	~VertexArray();
	void Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, ae::Vertex::Primitive primitive, ae::Vertex::Usage vertexUsage, ae::Vertex::Usage indexUsage );
	void AddAttribute( const char *name, uint32_t componentCount, ae::Vertex::Type type, uint32_t offset );
	void Terminate();
	
	//! Sets current vertex data. Equivalent to calling ae::VertexArray::Clear()
	//! then ae::VertexArray::Append().
	void SetVertices( const void* vertices, uint32_t count );
	//! Sets current index data. Equivalent to calling ae::VertexArray::Clear()
	//! then ae::VertexArray::Append().
	void SetIndices( const void* indices, uint32_t count );
	//! Add vertices to end of existing array.
	void AppendVertices( const void* vertices, uint32_t count );
	//! Add indices to end of existing array. Given indices are each offset
	//! based on \p indexOffset. It could be useful to use GetVertexCount() as a
	//! parameter to \p indexOffset before appending new vertices.
	void AppendIndices( const void* indices, uint32_t count, uint32_t indexOffset );
	//! Sets dynamic vertex count to 0. Has no effect if vertices are using
	//! ae::Vertex::Usage::Static.
	void ClearVertices();
	//! Sets dynamic index count to 0. Has no effect if indices are using
	//! ae::Vertex::Usage::Static.
	void ClearIndices();
	
	//! Preemptively prepares buffers for rendering. Call after Setting/
	//! Appending vertices and indices, but before Render() to avoid waiting for
	//! upload when rendering. This will result in a no-op if no changes have
	//! been made.
	void Upload();
	//! Renders all vertex data. Automatically calls Upload() first.
	void Draw( const ae::Shader* shader, const ae::UniformList& uniforms ) const;
	//! Renders vertex data range. Automatically calls Upload() first.
	void Draw( const ae::Shader* shader, const ae::UniformList& uniforms, uint32_t primitiveStart, uint32_t primitiveCount ) const;
	
	template< typename T = void > const T* GetVertices() const;
	template< typename T = void > const T* GetIndices() const;
	uint32_t GetVertexSize() const { return m_buffer.GetVertexSize(); }
	uint32_t GetIndexSize() const { return m_buffer.GetIndexSize(); }
	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetIndexCount() const { return m_indexCount; }
	uint32_t GetMaxVertexCount() const { return m_buffer.GetMaxVertexCount(); }
	uint32_t GetMaxIndexCount() const { return m_buffer.GetMaxIndexCount(); }
	uint32_t GetMaxPrimitiveCount() const { return m_buffer.GetMaxPrimitiveCount(); }
	ae::Vertex::Primitive GetPrimitiveType() const { return m_buffer.GetPrimitiveType(); }
	bool IsIndexed() const { return m_buffer.IsIndexed(); }
	
private:
	const ae::Tag m_tag;
	ae::VertexBuffer m_buffer;
	uint32_t m_vertexCount = 0;
	uint32_t m_indexCount = 0;
	void* m_vertexReadable = nullptr;
	void* m_indexReadable = nullptr;
	bool m_vertexDirty = false;
	bool m_indexDirty = false;
};

//------------------------------------------------------------------------------
// ae::InstanceData class
//------------------------------------------------------------------------------
class InstanceData
{
public:
	InstanceData() = default;
	~InstanceData();

	void Initialize( uint32_t dataStride, uint32_t maxInstanceCount, ae::Vertex::Usage usage );
	void AddAttribute( const char *name, uint32_t componentCount, ae::Vertex::Type type, uint32_t offset );
	void Terminate();
	void UploadData( uint32_t startIdx, const void* data, uint32_t count );

	uint32_t GetStride() const { return m_dataStride; }
	uint32_t GetMaxInstanceCount() const { return m_maxInstanceCount; }

private:
	InstanceData( const InstanceData& ) = delete;
	InstanceData( InstanceData&& ) = delete;
	void operator=( const InstanceData& ) = delete;
	void operator=( InstanceData&& ) = delete;
	ae::Array< VertexBuffer::_Attribute, _kMaxShaderAttributeCount > m_attributes;
	uint32_t m_buffer = ~0;
	uint32_t m_dataStride = 0;
	uint32_t m_maxInstanceCount = 0;
	Vertex::Usage m_usage = (ae::Vertex::Usage)-1;
public:
	uint32_t _GetBuffer() const { return m_buffer; }
	const VertexBuffer::_Attribute* _GetAttribute( const char* n ) const;
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
		UInt8,
		UInt16,
		HalfFloat,
		Float
	};

	// Interface
	Texture() = default;
	virtual ~Texture();
	void Initialize( uint32_t target ); // GL_TEXTURE_2D etc
	virtual void Terminate();
	uint32_t GetTexture() const { return m_texture; }
	uint32_t GetTarget() const { return m_target; } // GL_TEXTURE_2D etc

// private:
	Texture( const Texture& ) = delete;
	Texture( Texture&& ) = delete;
	void operator=( const Texture& ) = delete;
	void operator=( Texture&& ) = delete;
	uint32_t m_texture = 0;
	uint32_t m_target = 0; // GL_TEXTURE_2D etc
};

//------------------------------------------------------------------------------
// ae::TextureParams class
//------------------------------------------------------------------------------
struct TextureParams
{
	const void* data = nullptr;
	bool bgrData = false;
	uint32_t width = 0;
	uint32_t height = 0;
	Texture::Format format = Texture::Format::RGBA8;
	Texture::Type type = Texture::Type::UInt8;
	Texture::Filter filter = Texture::Filter::Linear;
	Texture::Wrap wrap = Texture::Wrap::Repeat;
	bool autoGenerateMipmaps = true;
};

//------------------------------------------------------------------------------
// ae::Texture2D class
//! \brief A 2D texture primitive used as a parameter to ae::Shader/ae::UniformList. Use an sRGB format
//! if you are providing sRGB data. As long as you use the correct format you can assume shader texture reads
//! will return linear values.
//------------------------------------------------------------------------------
class Texture2D : public Texture
{
public:
	void Initialize( const TextureParams& params );
	void Initialize( const void* data, uint32_t width, uint32_t height, ae::Texture::Format format, ae::Texture::Type type, ae::Texture::Filter filter, ae::Texture::Wrap wrap, bool autoGenerateMipmaps );
	void Terminate() override;

	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }

// private:
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
	uint32_t GetTextureCount() const;
	const Texture2D* GetDepth() const;
	float GetAspectRatio() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;

	//! Get the ndc space rect of this target within another target (fill but
	//! maintain aspect ratio). Use this function by providing the width and
	//! height of the target that this texture will be written to. One use case
	//! is to call this function on the source ae::RenderTarget and provide the
	//! width and height of the ae::RenderTarget being written to.
	//! GetNDCFillRectForTarget( GraphicsDevice::GetWindow()::GetWidth(), GraphicsDevice::GetWindow()::Height() )
	//! GetNDCFillRectForTarget( GraphicsDeviceTarget()::GetWidth(), GraphicsDeviceTarget()::Height() )
	Rect GetNDCFillRectForTarget( uint32_t otherWidth, uint32_t otherHeight ) const;

	//! Other target to local transform (pixels->pixels). Useful for transforming
	//! window/mouse pixel coordinates to local pixels. Call this function on the
	//! 'inner' target (ie. viewport) and provide the width and height of the
	//! 'outermost' target (ie. window). The resulting matrix can be used to transform
	//! from the outer target to the inner target (ie. window to viewport).
	//! GetTargetPixelsToLocalTransform( GraphicsDevice::GetWindow()::GetWidth(),  GraphicsDevice::GetWindow()::Height(), GetNDCFillRectForTarget( ... ) )
	Matrix4 GetTargetPixelsToLocalTransform( uint32_t otherPixelWidth, uint32_t otherPixelHeight, Rect ndc ) const;

	//! Mouse/window pixel coordinates to world space
	//! GetTargetPixelsToWorld( GetTargetPixelsToLocalTransform( ... ), TODO )
	Matrix4 GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const;

	//! Creates a transform matrix from aeQuad vertex positions to ndc space
	//! GraphicsDeviceTarget uses aeQuad vertices internally
	static Matrix4 GetQuadToNDCTransform( Rect ndc, float z );

private:
	uint32_t m_fbo = 0;
	Array< Texture2D*, 4 > m_targets;
	Texture2D m_depth;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
};

//------------------------------------------------------------------------------
// ae::GraphicsDevice class
//! \brief Handles the final presentation of rendered graphics to the screen/window. The final phase of your rendering
//! should be to use Activate() and Clear(), then render your scene (with ae::VertexBuffer/Array::Render(), ae::RenderTarget::Render(), etc)
//! to the contained target, and then finally call Present(). The width and height of this target is automatically controlled by the
//! window size (multiplied by the scale factor for maximum resolution). This target is linear and so colors transferred
//! to it should also be linear. There is no need to to use sRGB at any point in your pipeline unless you have an explicit
//! need for it yourself.
//------------------------------------------------------------------------------
class GraphicsDevice
{
public:
	GraphicsDevice() = default;
	~GraphicsDevice();
	void Initialize( class Window* window );
	void Terminate();

	void SetVsyncEnbled( bool enabled );
	bool GetVsyncEnabled() const;

	void Activate();
	void Clear( Color color );
	void Present();
	//! Must call to readback from active render target (GL only)
	void AddTextureBarrier();

	class Window* GetWindow() { return m_window; }
	RenderTarget* GetCanvas() { return &m_canvas; }
	uint32_t GetWidth() const { return m_canvas.GetWidth(); }
	uint32_t GetHeight() const { return m_canvas.GetHeight(); }
	float GetAspectRatio() const;

//private:
	friend class ae::Window;
	GraphicsDevice( const GraphicsDevice& ) = delete;
	void m_HandleResize( uint32_t width, uint32_t height );
	Window* m_window = nullptr;
	RenderTarget m_canvas;
#if _AE_EMSCRIPTEN_
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE m_context = 0;
	double m_lastResize = 0.0;
	int32_t m_resizeWidthPrev = 0;
	int32_t m_resizeHeightPrev = 0;
#else
	void* m_context = nullptr;
#endif
	int32_t m_defaultFbo = -1;
	
	VertexBuffer m_renderQuad;
	Shader m_renderShaderRGB;
	Shader m_renderShaderSRGB;
	bool m_rgbToSrgb = false;
};

//------------------------------------------------------------------------------
// ae::TextRender class
//------------------------------------------------------------------------------
class TextRender
{
public:
	TextRender( const ae::Tag& tag );
	~TextRender();
	//! Initializes this TextRender. Must be called before other functions.
	//! @param maxStringCount the maximum number of strings that can
	//! be rendered per call to TextRender::Render()
	//! @param maxGlyphCount the maximum number of total characters that can
	//! be rendered per call to TextRender::Render()
	//! @param texture a square texture with ascii characters evenly spaced from
	//! top left to bottom right, the red channel of the texture can be used for
	//! transparency
	//! @param fontSize the width and height of each character in the texture
	//! @param spacing distance between each character, the given value is
	//! multiplied by \p fontSize
	void Initialize( uint32_t maxStringCount, uint32_t maxGlyphCount, const ae::Texture2D* texture, uint32_t fontSize, float spacing );
	void Terminate();
	void Render( const ae::Matrix4& uiToScreen );
	void Add( ae::Vec3 pos, ae::Vec2 size, const char* str, ae::Color color, uint32_t lineLength, uint32_t charLimit );
	uint32_t GetLineCount( const char* str, uint32_t lineLength, uint32_t charLimit ) const;
	uint32_t GetFontSize() const { return m_fontSize; }

private:
	uint32_t m_ParseText( const char* str, uint32_t lineLength, uint32_t charLimit, char** _outStr, uint32_t* lenOut ) const;
	struct Vertex
	{
		ae::Vec3 pos;
		ae::Vec2 uv;
		ae::Vec4 color;
	};
	struct TextRect
	{
		const char* str;
		ae::Vec3 pos;
		ae::Vec2 size;
		ae::Color color;
	};
	// Params
	const ae::Tag m_tag;
	uint32_t m_maxRectCount = 0;
	uint32_t m_maxGlyphCount = 0;
	const ae::Texture2D* m_texture = nullptr;
	uint32_t m_fontSize = 0;
	float m_spacing = 0.0f;
	// Data
	ae::VertexBuffer m_vertexData;
	ae::Shader m_shader;
	TextRect* m_strings = nullptr;
	char* m_stringData = nullptr;
	uint32_t m_allocatedStrings = 0;
	uint32_t m_allocatedChars = 0;
};

//------------------------------------------------------------------------------
// ae::DebugLines class
//------------------------------------------------------------------------------
class DebugLines
{
public:
	DebugLines( const ae::Tag& tag );
	~DebugLines();
	//! Call this before ae::DebugLines::Add...() and before calling ae::DebugLines::Render(). \p maxVerts
	//! are allocated when this function is called. All subsequent calls to ae::DebugLines::Add...() will return
	//! false once this limit has been reached until ae::DebugLines::Clear() or ae::DebugLines::Render()
	//! is called.
	void Initialize( uint32_t maxVerts );
	//! Deallocates vertices and frees GPU recources.
	void Terminate();
	//! Draws all debug lines submitted with ae::DebugLines::Add...() since the last call to ae::DebugLines::Clear()
	//! or ae::DebugLines::Render(). All debug lines must be resubmitted after calling this.
	void Render( const Matrix4& worldToNdc );
	//! Enable or disable drawing of desaturated lines on failed depth test.
	//! Enabled by default.
	void SetXRayEnabled( bool enabled ) { m_xray = enabled; }
	//! Resets the internal vertex buffer without uploading anything to the GPU. Use this if a call to
	//! ae::DebugLines::Render() is ever skipped.
	void Clear();

	//! Adds a line from \p p0 to \p p1 with \p color to be transformed and drawn with ae::DebugLines::Render().
	//! Returns false and the line is not added if ae::DebugLines::GetMaxVertexCount() would be exceeded.
	uint32_t AddLine( Vec3 p0, Vec3 p1, Color color );
	//! Adds a line from \p p0 to \p p1 to be transformed and drawn with ae::DebugLines::Render(). The
	//! color will be \p successColor if the distance between \p p0 and \p p1 is less than \p distance,
	//! otherwise the line color will be \p failColor. Returns false and the line is not added if
	//! ae::DebugLines::GetMaxVertexCount() would be exceeded.
	uint32_t AddDistanceCheck( Vec3 p0, Vec3 p1, float distance, ae::Color successColor, ae::Color failColor );
	//! Adds a \p color rectangle with center \p pos facing \p normal rotated so the top line is
	//! perpendicular to \p up to be transformed and drawn with ae::DebugLines::Render().
	//! Returns false and the rectangle is not added if ae::DebugLines::GetMaxVertexCount() would be exceeded.
	uint32_t AddRect( Vec3 pos, Vec3 up, Vec3 normal, Vec2 halfSize, float cornerRadius, uint32_t cornerPointCount, Color color );
	//! Adds a \p color circle with center \p pos facing \p normal to be transformed and drawn with
	//! ae::DebugLines::Render(). \p pointCount determines the number of points along the circumference.
	//! Returns false and the circle is not added if ae::DebugLines::GetMaxVertexCount() would be exceeded.
	uint32_t AddCircle( Vec3 pos, Vec3 normal, float radius, Color color, uint32_t pointCount );
	uint32_t AddAABB( Vec3 pos, Vec3 halfSize, Color color );
	uint32_t AddAABB( AABB aabb, Color color );
	uint32_t AddOBB( const Matrix4& transform, Color color );
	uint32_t AddOBB( const OBB& obb, Color color );
	uint32_t AddSphere( Vec3 pos, float radius, Color color, uint32_t pointCount );
	uint32_t AddMesh( const Vec3* vertices, uint32_t vertexStride, uint32_t count, Matrix4 transform, Color color );
	uint32_t AddMesh( const Vec3* vertices, uint32_t vertexStride, uint32_t vertexCount, const void* indices, uint32_t indexSize, uint32_t indexCount, Matrix4 transform, Color color );
	
	//! Returns the number of vertices submitted since the last call to ae::DebugLines::Clear() or ae::DebugLines::Render().
	uint32_t GetVertexCount() const;
	//! Returns the maximum number of vertices that can be submitted between calls to ae::DebugLines::Clear()
	//! and ae::DebugLines::Render(). This is the value provided to ae::DebugLines::Initialize().
	uint32_t GetMaxVertexCount() const;

private:
	struct DebugVertex
	{
		Vec4 pos;
		Color color;
	};
	VertexArray m_vertexArray;
	Shader m_shader;
	bool m_xray = true;
};

//------------------------------------------------------------------------------
// ae::SpriteFont class
//------------------------------------------------------------------------------
class SpriteFont
{
public:
	void SetGlyph( char c, ae::Rect quad, ae::Rect uvs, float advance );
	bool GetGlyph( char c, ae::Rect* quad, ae::Rect* uv, float* advance, float uiSize ) const;
	float GetTextWidth( const char* text, float uiSize ) const;

private:
	struct GlyphData
	{
		GlyphData();
		ae::Rect quad;
		ae::Rect uvs;
		float advance;
	};
	GlyphData m_glyphs[ 96 ];
};

//------------------------------------------------------------------------------
// ae::SpriteRenderer class
//------------------------------------------------------------------------------
//! Vertex attributes: a_position (4 floats), a_color (4 floats), a_uv
//! (2 floats) are all provided to the vertex shader. See example.
// Example vertex shader:
/*
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_color;
	AE_IN_HIGHP vec2 a_uv;

	AE_OUT_HIGHP vec4 v_color;
	AE_OUT_HIGHP vec2 v_uv;

	void main()
	{
		v_color = a_color;
		v_uv = a_uv;
		gl_Position = a_position;
	}
*/
// Example fragment shader:
/*
	AE_UNIFORM sampler2D u_tex;

	AE_IN_HIGHP vec4 v_color;
	AE_IN_HIGHP vec2 v_uv;

	void main()
	{
		AE_COLOR = v_color * AE_TEXTURE2D( u_tex, v_uv );
	}
*/
//------------------------------------------------------------------------------
class SpriteRenderer
{
public:
	SpriteRenderer( const ae::Tag& tag );
	void Initialize( uint32_t maxGroups, uint32_t maxCount );
	void Terminate();

	void AddSprite( uint32_t group, ae::Vec2 pos, ae::Vec2 size, ae::Rect uvs, ae::Color color );
	void AddSprite( uint32_t group, ae::Rect quad, ae::Rect uvs, ae::Color color );
	void AddSprite( uint32_t group, const ae::Matrix4& transform, ae::Rect uvs, ae::Color color );
	void AddText( uint32_t group, const char* text, const SpriteFont* font, ae::Rect region, float fontSize, float lineHeight, ae::Color color );

	void SetParams( uint32_t group, const ae::Shader* shader, const ae::UniformList& uniforms );
	void Render();
	void Clear();

private:
	struct GroupParams
	{
		const ae::Shader* shader = nullptr;
		ae::UniformList uniforms;
	};
	struct SpriteVertex
	{
		ae::Vec4 pos;
		ae::Vec4 color;
		ae::Vec2 uv;
	};
	using SpriteIndex = uint16_t;
	ae::Array< GroupParams > m_params;
	ae::Array< uint32_t > m_spriteGroups;
	ae::VertexArray m_vertexArray;
};

//------------------------------------------------------------------------------
// ae::DebugCamera class
//! A camera utility which provides basic functionality for mouse and keyboard
//! navigation. These controls mimic some popular CAD software, but are still
//! slightly modified to be useable with a trackpad. The controls are:
//! Orbit: (Alt+LMB)
//! Pan: (Alt+MMB) or (Alt+touch scroll)
//! Zoom: (Alt+RMB) or (Scroll)
//------------------------------------------------------------------------------
class DebugCamera
{
public:
	enum class Mode { None, Rotate, Pan, Zoom };
	
	DebugCamera( ae::Axis upAxis );
	//! Interrupts refocus. Does not affect in progress input.
	void Reset( ae::Vec3 pivot, ae::Vec3 pos );
	//! Prevents the position of the camera from being less than \p min distance from the pivot point and
	//! greater than \p max distance from the pivot point. May affect the current position of the camera.
	void SetDistanceLimits( float min, float max );
	//! Updates the cameras position. Does not affect in progress input or refocus.
	void SetDistanceFromFocus( float distance );
	//! Passing false cancels in progress input and prevents new input until called again with true. True by default.
	void SetInputEnabled( bool enabled );
	//! Sets the yaw and pitch of the camera. Updates the cameras position.
	void SetRotation( ae::Vec2 angles );
	//! Updates pivot and position over time
	void Refocus( ae::Vec3 pivot );
	//! Call this every frame even when no input has taken place so refocus works as expected.
	//! See ae::DebugCamera::SetInputEnabled() if you would like to prevent the camera from moving.
	void Update( const ae::Input* input, float dt );

	//! Check if this returns ae::DebugCamera::Mode::None to see if mouse clicks should be ignored by other systems
	Mode GetMode() const;
	ae::Vec3 GetPosition() const { return m_pivot + m_offset; }
	ae::Vec3 GetPivot() const { return m_pivot; }
	ae::Vec3 GetForward() const { return m_forward; }
	ae::Vec3 GetRight() const { return m_right; }
	ae::Vec3 GetUp() const { return m_up; }
	float GetYaw() const { return m_yaw; }
	float GetPitch() const { return m_pitch; }
	float GetDistanceFromPivot() const { return m_dist; }
	
	bool GetRefocusTarget( ae::Vec3* targetOut ) const;
	float GetMinDistance() const { return m_min; }
	float GetMaxDistance() const { return m_max; }
	ae::Vec3 GetWorldUp() const { return GetWorldUp( m_worldUp ); }
	ae::Axis GetWorldUpAxis() const { return m_worldUp; }

	static ae::Vec3 RotationToForward( ae::Axis up, float yaw, float pitch );
	static ae::Vec3 GetWorldUp( ae::Axis up ) { return ( up == Axis::Z ) ? ae::Vec3(0,0,1) : ae::Vec3(0,1,0); }

private:
	void m_Precalculate();
	// Params
	float m_min = 1.0f;
	float m_max = ae::MaxValue< float >();
	Axis m_worldUp = Axis::Z;
	// Mode
	bool m_inputEnabled = true;
	Mode m_mode = Mode::None;
	ae::Vec3 m_refocusPos = ae::Vec3( 0.0f );
	bool m_refocus = false;
	float m_moveAccum = 0.0f;
	uint32_t m_preventModeExitImm = 0;
	// Positioning
	ae::Vec3 m_pivot = ae::Vec3( 0.0f );
	float m_dist = 5.0f;
	// Rotation
	float m_yaw = 0.77f;
	float m_pitch = 0.0f;
	// Pre-calculated values for getters
	ae::Vec3 m_offset;
	ae::Vec3 m_forward;
	ae::Vec3 m_right;
	ae::Vec3 m_up;
};

//------------------------------------------------------------------------------
// ae::CatmullRom @TODO: Implement, and Spline should use internally.
//------------------------------------------------------------------------------
ae::Vec3 CatmullRom( ae::Vec3 p0, ae::Vec3 p1, ae::Vec3 p2, ae::Vec3 p3, float t );

//------------------------------------------------------------------------------
// ae::Spline class
//------------------------------------------------------------------------------
class Spline
{
public:
	Spline( ae::Tag tag );
	Spline( ae::Tag tag, const ae::Vec3* controlPoints, uint32_t count, bool loop );
	void Reserve( uint32_t controlPointCount );

	//! Enables looped spline calculations on other functions. Call this before
	//! other functions to avoid recalculating the spline internally.
	void SetLooping( bool enabled );
	void AppendControlPoint( ae::Vec3 p );
	void RemoveControlPoint( uint32_t index );
	void Clear();

	ae::Vec3 GetControlPoint( uint32_t index ) const;
	uint32_t GetControlPointCount() const;

	ae::Vec3 GetPoint( float distance ) const; // 0 <= distance <= length
	float GetMinDistance( ae::Vec3 p, ae::Vec3* nearestOut = nullptr, float* tOut = nullptr ) const;
	float GetLength() const;

	ae::AABB GetAABB() const { return m_aabb; }

private:
	class Segment
	{
	public:
		void Init( ae::Vec3 p0, ae::Vec3 p1, ae::Vec3 p2, ae::Vec3 p3 );
		ae::Vec3 GetPoint01( float t ) const;
		ae::Vec3 GetPoint0() const;
		ae::Vec3 GetPoint1() const;
		ae::Vec3 GetPoint( float d ) const;
		float GetMinDistance( ae::Vec3 p, ae::Vec3* pOut, float* tOut ) const;
		float GetLength() const { return m_length; }
		ae::AABB GetAABB() const { return m_aabb; }

	private:
		ae::Vec3 m_a;
		ae::Vec3 m_b;
		ae::Vec3 m_c;
		ae::Vec3 m_d;
		float m_length;
		uint32_t m_resolution;
		ae::AABB m_aabb;
	};

	void m_RecalculateSegments();
	ae::Vec3 m_GetControlPoint( int32_t index ) const;

	bool m_loop = false;
	ae::Array< ae::Vec3 > m_controlPoints;
	ae::Array< Segment > m_segments;
	float m_length = 0.0f;
	ae::AABB m_aabb;
};

//------------------------------------------------------------------------------
// ae::CollisionExtra
//! AE_COLLISION_EXTRA_CONFIG can be defined to provide extra vertex data returned
//! from Raycasts and PushOuts. See AE_CONFIG_FILE for more info.
//------------------------------------------------------------------------------
#ifdef AE_COLLISION_EXTRA_CONFIG
	typedef AE_COLLISION_EXTRA_CONFIG CollisionExtra;
#else
	typedef uint32_t CollisionExtra;
#endif

//------------------------------------------------------------------------------
// ae::RaycastParams
//------------------------------------------------------------------------------
struct RaycastParams
{
	ae::Matrix4 transform = ae::Matrix4::Identity();
	const void* userData = nullptr;
	ae::Vec3 source = ae::Vec3( 0.0f );
	ae::Vec3 ray = ae::Vec3( 0.0f, 0.0f, -1.0f );
	uint32_t maxHits = 1;
	bool hitCounterclockwise = true;
	bool hitClockwise = false;
	ae::DebugLines* debug = nullptr; // Draw collision results
	ae::Color debugColor = ae::Color::Red();
};

//------------------------------------------------------------------------------
// ae::RaycastResult
//------------------------------------------------------------------------------
struct RaycastResult
{
	struct Hit
	{
		ae::Vec3 position = ae::Vec3( 0.0f );
		ae::Vec3 normal = ae::Vec3( 0.0f );
		float distance = 0.0f;
		const void* userData = nullptr;
		CollisionExtra extra;
	};
	ae::Array< Hit, 8 > hits;
	
	static void Accumulate( const RaycastParams& params, const RaycastResult& prev, RaycastResult* next );
};

//------------------------------------------------------------------------------
// ae::PushOutParams
//! Sphere collision
//------------------------------------------------------------------------------
struct PushOutParams
{
	ae::Matrix4 transform = ae::Matrix4::Identity();
	const void* userData = nullptr;
	ae::DebugLines* debug = nullptr; // Draw collision results
	ae::Color debugColor = ae::Color::Red();
};

//------------------------------------------------------------------------------
// ae::PushOutInfo
//! Sphere collision
//------------------------------------------------------------------------------
struct PushOutInfo
{
	ae::Sphere sphere;
	ae::Vec3 velocity = ae::Vec3( 0.0f );
	struct Hit
	{
		ae::Vec3 position;
		ae::Vec3 normal;
		CollisionExtra extra;
	};
	ae::Array< Hit, 8 > hits;

	static void Accumulate( const PushOutParams& params, const PushOutInfo& prev, PushOutInfo* next );
};

//------------------------------------------------------------------------------
// ae::CollisionMesh class
//------------------------------------------------------------------------------
template< uint32_t VertMax = 0, uint32_t TriMax = 0, uint32_t BVHMax = 0 >
class CollisionMesh
{
public:
	CollisionMesh(); //!< Static (V == T == B == 0)
	CollisionMesh( ae::Tag tag ); //!< Dynamic (V != 0) && (T != 0) && (B != 0)
	void Reserve( uint32_t vertCount, uint32_t triCount, uint32_t bvhNodeCount );

	struct AddIndexedParams
	{
		ae::Matrix4 transform = ae::Matrix4::Identity();
		
		const float* vertexPositions = nullptr;
		const CollisionExtra* vertexExtras = nullptr;
		uint32_t vertexPositionStride = 0;
		uint32_t vertexExtraStride = 0;
		uint32_t vertexCount = 0;

		const void* indices = nullptr;
		uint32_t indexCount = 0;
		uint32_t indexSize = 0;
	};
	void AddIndexed( const AddIndexedParams& params );
	void AddIndexed( ae::Matrix4 transform, const float* positions, uint32_t positionCount, uint32_t positionStride, const void* indices, uint32_t indexCount, uint32_t indexSize );
	
	//! Must be called after AddIndexed() or Reserve() for Raycast() and PushOut()
	//! to work. This can be slightly expensive, so try to only call this once
	//! when all mesh data is submitted. Internally this will early out if no
	//! rebuild is required.
	void BuildBVH();
	//! Returns true if  BuildBVH() should be called. Returns false if BuildBVH()
	//! will early out.
	bool RequiresBVHRebuild() const { return m_requiresRebuild; }
	//! Resets CollisionMesh to its original empty state, except reserved buffer
	//! sizes are maintained.
	void Clear();

	RaycastResult Raycast( const RaycastParams& params, const RaycastResult& prevResult = RaycastResult() ) const;
	PushOutInfo PushOut( const PushOutParams& params, const PushOutInfo& prevInfo ) const;
	// @TODO: GetClosestPoint()
	ae::AABB GetAABB() const { return m_bvh.GetAABB(); }
	
	const ae::Vec3* GetVertices() const { return m_positions.Data(); }
	const uint32_t* GetIndices() const { return (uint32_t*)m_tris.Data(); }
	uint32_t GetVertexCount() const { return m_positions.Length(); }
	uint32_t GetIndexCount() const { return m_tris.Length() * 3; }

private:
	// @TODO: Support user data returned with raycast results
	struct BVHTri { uint32_t idx[ 3 ]; };
	const ae::Tag m_tag;
	ae::AABB m_aabb;
	bool m_requiresRebuild = false;
	ae::Array< ae::Vec3, VertMax > m_positions;
	ae::Array< CollisionExtra, VertMax > m_collisionExtras;
	ae::Array< BVHTri, TriMax > m_tris;
	ae::BVH< BVHTri, BVHMax > m_bvh;
};

//------------------------------------------------------------------------------
// ae::AStarNode example interface
// ae::AStar is a generic A* algorithm implementation. It requires a type T
// which requires the following interface:
//------------------------------------------------------------------------------
template< uint32_t N = 0 >
struct AStarNode
{
	AStarNode() : pos( 0.0f ) {}
	AStarNode( const ae::Tag& tag ) : next( tag ), pos( 0.0f ) {}

	// ae::AStar type T must implement these following functions:
	//! Returns the next node in the path. \p index is the index of the next
	//! node in the path. Edges between paths can be uni-directional or
	//! bi-directional.
	const ae::AStarNode< N >* GetNext( uint32_t index ) const { return next[ index ]; }
	//! Returns the number of nodes directly visitable from this node.
	uint32_t GetNextCount() const { return next.Length(); }
	//! Returns the heuristic distance between this node and \p other. For
	//! ae::AStar to return a least cost path, this function must be 'admissible'.
	//! This means this function should never overestimate the cost of
	//! traveling between this node and \p other.
	float GetHeuristic( const ae::AStarNode< N >* other ) const { return ( pos - other->pos ).Length(); }

	ae::Array< const ae::AStarNode< N >*, N > next;
	ae::Vec3 pos = ae::Vec3( 0.0f );
};

//------------------------------------------------------------------------------
// ae::AStar algorithm
//! \brief A generic A* algorithm implementation. Requires a type T which
//! implements the functions: GetNext(), GetNextCount(), and GetHeuristic().
//! See ae::AStarNode for exact function signatures. \p startNode is the node
//! to start the search from. \p nodes and \p nodeCount is the array of nodes to
//! search through (including the start node and goals nodes). \p goalNodes and
//! \p goalCount is the array of nodes to search for, the closest goal node will
//! be found. \p pathOut is the array to write the path to, up to \p pathOutMax.
//! Returns the number of nodes written to \p pathOut. If the path is longer
//! than \p pathOutMax, the path is truncated from the end, so only the beginning
//! of the path is written. If no path is found, 0 is returned.
//------------------------------------------------------------------------------
template< typename T >
uint32_t AStar( const T* startNode, const T* nodes, uint32_t nodeCount, const T** goalNodes, uint32_t goalCount, const T** pathOut, uint32_t pathOutMax );

//------------------------------------------------------------------------------
// ae::Keyframe struct
//------------------------------------------------------------------------------
struct Keyframe
{
	Keyframe() = default;
	Keyframe( const ae::Matrix4& transform );
	ae::Matrix4 GetLocalTransform() const;
	Keyframe Lerp( const Keyframe& target, float t ) const;
	
	ae::Vec3 translation = ae::Vec3( 0.0f );
	ae::Quaternion rotation = ae::Quaternion::Identity();
	ae::Vec3 scale = ae::Vec3( 1.0f );
};

//------------------------------------------------------------------------------
// ae::Bone struct
//------------------------------------------------------------------------------
struct Bone
{
	ae::Str64 name;
	uint32_t index = 0;
	ae::Matrix4 modelToBone = ae::Matrix4::Identity();
	ae::Matrix4 parentToChild = ae::Matrix4::Identity();
	ae::Matrix4 boneToModel = ae::Matrix4::Identity();
	Bone* firstChild = nullptr;
	Bone* nextSibling = nullptr;
	Bone* parent = nullptr;
};

//------------------------------------------------------------------------------
// ae::Animation class
//------------------------------------------------------------------------------
class Animation
{
public:
	Animation( const ae::Tag& tag ) : keyframes( tag ) {}
	ae::Keyframe GetKeyframeByTime( const char* boneName, float time ) const;
	ae::Keyframe GetKeyframeByPercent( const char* boneName, float percent ) const;
	void AnimateByTime( class Skeleton* target, float time, float strength, const Bone** mask, uint32_t maskCount ) const;
	void AnimateByPercent( class Skeleton* target, float percent, float strength, const Bone** mask, uint32_t maskCount ) const;
	
	float duration = 0.0f;
	bool loop = false;
	ae::Map< ae::Str64, ae::Array< ae::Keyframe > > keyframes; // @TODO: boneKeyframes. Maybe private
};

//------------------------------------------------------------------------------
// ae::Skeleton class
//------------------------------------------------------------------------------
class Skeleton
{
public:
	Skeleton( const ae::Tag& tag ) : m_bones( tag ) {}
	void Initialize( uint32_t maxBones );
	void Initialize( const Skeleton* otherPose );
	const Bone* AddBone( const Bone* parent, const char* name, const ae::Matrix4& parentToChild );
	void SetLocalTransforms( const Bone** targets, const ae::Matrix4* parentToChildTransforms, uint32_t count );
	void SetLocalTransform( const Bone* target, const ae::Matrix4& parentToChild );
	void SetTransform( const Bone* target, const ae::Matrix4& modelToBone );
	
	const Bone* GetRoot() const;
	const Bone* GetBoneByName( const char* name ) const;
	const Bone* GetBoneByIndex( uint32_t index ) const;
	const Bone* GetBones() const;
	uint32_t GetBoneCount() const;
	
private:
	Skeleton( const Skeleton& ) = delete;
	ae::Array< ae::Bone > m_bones;
};

//------------------------------------------------------------------------------
// ae::IKConstraints struct
//------------------------------------------------------------------------------
struct IKConstraints
{
	//! The axis that points towards the next bone. A specific/prominent
	//! industry auto-rigger will inconsistently orient bones so that the
	//! negative x axis points towards the next bone for "Right" bones and
	//! the positive x axis points towards the next bone for all center and
	//! Left bones.
	ae::Axis twistAxis = ae::Axis::NegativeX;
	//! The axis that corresponds to the 'vertical' rotation limits y component.
	//! The implicitly specified tertiary axis corresponds to the horizontal
	//! rotation limits x component.
	ae::Axis bendAxis = ae::Axis::Z;
	// @TODO
	ae::Axis horizontalAxis = ae::Axis::Y;
	//! The half-range of motion of this joint in radians. @TODO: Array element
	//! details. @TODO: Should support no limits.
	float rotationLimits[ 4 ] = { 1.25f, 1.25f, 1.25f, 1.25f };
	//! The amount in radians that this joint is allowed to twist around the
	//! primary axis in either direction. Lower limit is negative, upper limit
	//! is positive. Zero is no twist. @TODO: Should support no limits.
	float twistLimits[ 2 ] = { -ae::QuarterPi, ae::QuarterPi };
};

//------------------------------------------------------------------------------
// ae::IK struct
//------------------------------------------------------------------------------
struct IK
{
	IK( ae::Tag tag );
	void Run( uint32_t iterationCount, ae::Skeleton* poseOut );

	const ae::Tag tag;
	ae::Matrix4 targetTransform = ae::Matrix4::Identity();
	//! Bone indices. Ordered from root to extent.
	ae::Array< uint32_t > chain;
	//! Joint info for each bone in the skeleton. Leave this empty to use the
	//! default ae::IKConstraints, or append a single ae::IKJoint to use that for all
	//! bones. Otherwise this should be the same length as 'pose.GetBoneCount()'. 
	ae::Array< ae::IKConstraints > joints;
	//! Referenced for bone lengths and joint limits
	const ae::Skeleton* bindPose = nullptr;
	//! Used as the starting point for the IK.
	ae::Skeleton pose;
	//! If false, the IK will not respect joint limits
	bool enableRotationLimits = true;

	ae::DebugLines* debugLines = nullptr;
	ae::Matrix4 debugModelToWorld = ae::Matrix4::Identity();
	float debugJointScale = 0.1f; //!< Useful default when working in meters

	// @TODO: Cleaup IK helpers
	static ae::Vec2 GetNearestPointOnEllipse( ae::Vec2 halfSize, ae::Vec2 center, ae::Vec2 p );
	static ae::Vec3 GetAxisVector( ae::Axis axis, bool negative = true );
	ae::Vec3 ClipJoint( float bindBoneLength, ae::Vec3 j0Pos, ae::Quaternion j0Ori, ae::Vec3 j1, const ae::IKConstraints& j1Constraints, ae::Color debugColor );

	static float GetAxis( ae::Axis axis, const ae::Vec3 v )
	{
		switch( axis )
		{
			case ae::Axis::X: return v.x;
			case ae::Axis::Y: return v.y;
			case ae::Axis::Z: return v.z;
			case ae::Axis::NegativeX: return -v.x;
			case ae::Axis::NegativeY: return -v.y;
			case ae::Axis::NegativeZ: return -v.z;
			default: return 0.0f;
		}
	}

	static ae::Vec3 Build3D( ae::Axis horizontalAxis, ae::Axis bendAxis, ae::Axis twistAxis, float horizontalVal, float verticalVal, float primaryVal, bool negative = true )
	{
		ae::Vec3 result = GetAxisVector( horizontalAxis, negative ) * horizontalVal;
		result += GetAxisVector( bendAxis, negative ) * verticalVal;
		result += GetAxisVector( twistAxis, negative ) * primaryVal;
		return result;
	}
};

//------------------------------------------------------------------------------
// ae::Skin class
//------------------------------------------------------------------------------
const uint32_t kMaxSkinWeights = 4;
class Skin
{
public:
	//! ae::SkinVertex has no constructor, so take care to initialize all member variables
	struct Vertex
	{
		ae::Vec3 position;
		ae::Vec3 normal;
		uint16_t bones[ kMaxSkinWeights ];
		uint8_t weights[ kMaxSkinWeights ];
	};
	
	Skin( const ae::Tag& tag ) : m_bindPose( tag ), m_verts( tag ) {}
	void Initialize( const Skeleton& bindPose, const ae::Skin::Vertex* vertices, uint32_t vertexCount );
	
	const class Skeleton& GetBindPose() const;
	const ae::Matrix4& GetInvBindPose( const char* name ) const;
	
	void ApplyPoseToMesh( const Skeleton* pose, float* positionsOut, float* normalsOut, uint32_t positionStride, uint32_t normalStride, bool positionsW, bool normalsW, uint32_t count ) const;
	
	uint32_t GetBoneCount() const { return m_bindPose.GetBoneCount(); }
	uint32_t GetVertCount() const { return m_verts.Length(); }
	
private:
	Skin( const Skin& ) = delete;
	Skeleton m_bindPose;
	ae::Array< Vertex > m_verts;
};

//------------------------------------------------------------------------------
// ae::OBJLoader class
//------------------------------------------------------------------------------
class OBJLoader
{
public:
	//! Vertex storage for loaded OBJ mesh
	struct Vertex
	{
		ae::Vec4 position;
		ae::Vec2 texture;
		ae::Vec4 normal;
		ae::Vec4 color;
	};
	//! Index type for loaded OBJ mesh
	typedef uint32_t Index;
	//! Parameters to OBJLoader::Initialize()
	struct InitializeParams
	{
		//! OBJ file data
		const uint8_t* data = nullptr;
		//! Length of the OBJ file data
		uint32_t length = 0;
		//! All loaded vertex positions and normals are transformed by this
		//! matrix on load. This is useful for rotating the mesh or changing
		//! units etc (eg. meters to centimeters).
		ae::Matrix4 localToWorld = ae::Matrix4::Identity();
		//! Setting this to true will flip the winding order of all triangles
		//! loaded from the OBJ file.
		bool flipWinding = false;
	};
	//! Helper struct to load OBJ files directly into an ae::VertexArray
	struct VertexDataParams
	{
		ae::VertexBuffer* vertexData = nullptr;
		const char* posAttrib = "a_position";
		const char* normalAttrib = "a_normal";
		const char* colorAttrib = "a_color";
		const char* uvAttrib = "a_uv";
	};
	
	//! The given tag is used for all internal of the loaded OBJ mesh data
	OBJLoader( ae::Tag allocTag ) : allocTag( allocTag ), vertices( allocTag ), indices( allocTag ) {}
	//! Loads an OBJ file from memory. See ae::OBJLoader::InitializeParams for
	//! details. Returns false if the OBJ file could not be loaded.
	bool Load( ae::OBJLoader::InitializeParams params );
	//! Helper function to load OBJ files directly into an ae::VertexArray
	void InitializeVertexData( const ae::OBJLoader::VertexDataParams& params );
	//! Helper function to load OBJ files directly into an ae::CollisionMesh
	template< uint32_t V, uint32_t T, uint32_t B >
	void InitializeCollisionMesh( ae::CollisionMesh< V, T, B >* mesh );
	
	ae::Tag allocTag;
	ae::Array< ae::OBJLoader::Vertex > vertices;
	ae::Array< ae::OBJLoader::Index > indices;
	ae::AABB aabb;
};

//------------------------------------------------------------------------------
// ae::TargaFile class
//------------------------------------------------------------------------------
class TargaFile
{
public:
	TargaFile( ae::Tag allocTag ) : m_data( allocTag ) {}
	bool Load( const uint8_t* data, uint32_t length );
	
	ae::TextureParams textureParams;
private:
	ae::Array< uint8_t > m_data;
};

//------------------------------------------------------------------------------
// ae::AudioData class
//------------------------------------------------------------------------------
class AudioData
{
public:
	AudioData();
	uint32_t buffer;
	float length;
};

//------------------------------------------------------------------------------
// ae::Audio class
//------------------------------------------------------------------------------
class Audio
{
public:
	void Initialize( uint32_t musicChannels, uint32_t sfxChannels, uint32_t sfxLoopChannels, uint32_t maxAudioDatas );
	void Terminate();
	
	const AudioData* LoadWavFile( const uint8_t* data, uint32_t length );

	void SetVolume( float volume );
	void SetMusicVolume( float volume, uint32_t channel );
	void SetSfxLoopVolume( float volume, uint32_t channel );
	void PlayMusic( const AudioData* audioFile, float volume, uint32_t channel );
	//! Lower priority values interrupt sfx with higher values
	void PlaySfx( const AudioData* audioFile, float volume, int32_t priority );
	void PlaySfxLoop( const AudioData* audioFile, float volume, uint32_t channel );
	void StopMusic( uint32_t channel );
	void StopSfxLoop( uint32_t channel );
	void StopAllSfx();
	void StopAllSfxLoops();
	
	uint32_t GetMusicChannelCount() const;
	uint32_t GetSfxChannelCount() const;
	uint32_t GetSfxLoopChannelCount() const;
	void Log();

private:
	struct Channel
	{
		Channel();
		uint32_t source;
		int32_t priority;
		const AudioData* resource;
	};
	uint32_t m_maxAudioDatas = 0;
	ae::Array< AudioData > m_audioDatas = AE_ALLOC_TAG_AUDIO;
	ae::Array< Channel > m_musicChannels = AE_ALLOC_TAG_AUDIO;
	ae::Array< Channel > m_sfxChannels = AE_ALLOC_TAG_AUDIO;
	ae::Array< Channel > m_sfxLoopChannels = AE_ALLOC_TAG_AUDIO;
};

class BinaryWriter;
class BinaryReader;
//------------------------------------------------------------------------------
// ae::BinaryStream base class
//------------------------------------------------------------------------------
class BinaryStream
{
public:
	//! BinaryStream can't be constructed directly, see ae::BinaryWriter and
	//! ae::BinaryReader. Use this function to check if this is a writer stream
	//! or to call writer only serialization functions. Returns this as an
	//! ae::BinaryWriter if the stream is configured to write to the given
	//! buffer, otherwise returns nullptr.
	ae::BinaryWriter* AsWriter() { return ( m_mode == Mode::WriteBuffer ) ? reinterpret_cast< ae::BinaryWriter* >( this ) : nullptr; }
	//! BinaryStream can't be constructed directly, see ae::BinaryWriter and
	//! ae::BinaryReader. Use this function to check if this is a reader stream
	//! or to call reader only serialization functions. Returns this as an
	//! ae::BinaryReader if the stream is configured to read from the given
	//! buffer, otherwise returns nullptr.
	ae::BinaryReader* AsReader() { return ( m_mode == Mode::ReadBuffer ) ? reinterpret_cast< ae::BinaryReader* >( this ) : nullptr; }
	//! BinaryStream can't be constructed directly, see ae::BinaryWriter and
	//! ae::BinaryReader. Use this function to check if this is a writer stream
	//! or to call writer only serialization functions. Returns this as an
	//! ae::BinaryWriter if the stream is configured to write to the given
	//! buffer, otherwise returns nullptr.
	const ae::BinaryWriter* AsWriter() const { return ( m_mode == Mode::WriteBuffer ) ? reinterpret_cast< const ae::BinaryWriter* >( this ) : nullptr; }
	//! BinaryStream can't be constructed directly, see ae::BinaryWriter and
	//! ae::BinaryReader. Use this function to check if this is a reader stream
	//! or to call reader only serialization functions. Returns this as an
	//! ae::BinaryReader if the stream is configured to read from the given
	//! buffer, otherwise returns nullptr.
	const ae::BinaryReader* AsReader() const { return ( m_mode == Mode::ReadBuffer ) ? reinterpret_cast< const ae::BinaryReader* >( this ) : nullptr; }

	void SerializeUInt8( uint8_t& valInOut );
	void SerializeUInt16( uint16_t& valInOut );
	void SerializeUInt32( uint32_t& valInOut );
	void SerializeUInt64( uint64_t& valInOut );
	void SerializeInt8( int8_t& valInOut );
	void SerializeInt16( int16_t& valInOut );
	void SerializeInt32( int32_t& valInOut );
	void SerializeInt64( int64_t& valInOut );
	void SerializeFloat( float& valInOut );
	void SerializeDouble( double& valInOut );
	void SerializeBool( bool& valInOut );
	template< typename E > void SerializeEnum( E& valInOut );
	template< uint32_t N > void SerializeString( Str< N >& strInOut );
	//! \p bufferSize should include room for the null terminator, so the maximum
	//! string length is \p bufferSize - 1. In write mode the null terminator is
	//! always written to \p strInOut if bufferSize is greater than 0. If the
	//! string is longer than \p bufferSize - 1 the stream will be invalidated.
	void SerializeString( char* strInOut, uint32_t bufferSize );
	//! Calls 'T ::Serialize( ae::BinaryStream* )' on \p valInOut, or
	//! void Serialize( ae::BinaryStream*, T& ) if a member function serialize
	//! is not found. If the stream is a writer stream or T is const, this
	//! can fallback to calling 'T ::Serialize( ae::BinaryWriter* ) const' if
	//! it exists or 'void Serialize( ae::BinaryWriter*, T& )' if it does not.
	template< typename T > void SerializeObject( T& valInOut );
	//! Serialize \p data to or from the stream. The \p length of the data is
	//! not serialized, and so the same length must be used for reading and
	//! writing. Use this with caution as platforms will have different struct
	//! packing, byte order, and alignment.
	void SerializeRaw( void* dataInOut, uint32_t length );

	//! Invalidates the stream for future reading and writing. Use this when
	//! a serialization issue is detected. Use in conjunction with IsValid().
	//! Once the stream is invalid serialization calls will result in silent no-ops.
	void Invalidate() { m_isValid = false; }
	//! Returns true if the stream is valid for reading and writing. Once the
	//! stream is invalid serialization calls will result in silent no-ops.
	bool IsValid() const { return m_isValid; }

	//! Get the data buffer provided on construction. Valid only until the next
	//! serialization call for writer streams initialized with an ae::Array,
	//! otherwise the data buffer location is constant.
	const uint8_t* GetData() const { return m_extArray ? m_extArray->Data() : m_data; }
	//! Current read/write head position in bytes. When writing to a file or
	//! socket etc call this function and ae::BinaryStream::GetData() to get the
	//! contained binary data.
	uint32_t GetOffset() const { return m_offset; }
	//! Returns the number of bytes remaining after GetOffset() in the read/write
	//! buffer. Returns 0 if called on an invalid stream.
	uint32_t GetRemainingBytes() const { return IsValid() ? ( m_length - m_offset ) : 0; }
	//! Total length in bytes of the data buffer. This can grow when in write
	//! mode with initialized with an ae::Array, but will otherwise stay
	//! consistent across the lifetime of the stream.
	uint32_t GetSize() const { return m_length; }

	template< typename T > void SetUserData( T* userData );
	template< typename T > T* GetUserData();

	void* m_userData = nullptr;
	uint32_t m_userDataTypeId = 0; // ae::TypeId
	uint32_t m_userDataConstTypeId = 0; // ae::TypeId

protected:
	enum class Mode
	{
		None,
		ReadBuffer,
		WriteBuffer,
	};
	Mode m_mode = Mode::None;
	bool m_isValid = false;
	uint8_t* m_data = nullptr;
	uint32_t m_length = 0;
	uint32_t m_offset = 0;
	Array< uint8_t >* m_extArray = nullptr;
	BinaryStream() = default;
	BinaryStream( Mode mode, void* data, uint32_t size ); // Read or write
	BinaryStream( Mode mode, const void* data, uint32_t size ); // Read only
	BinaryStream( Array< uint8_t >*array ); // Write only
	AE_DISABLE_COPY_ASSIGNMENT( BinaryStream );
	// Prevent Serialize functions from being called accidentally through automatic conversions
	template< typename T > void SerializeUInt8( T ) = delete;
	template< typename T > void SerializeUInt16( T ) = delete;
	template< typename T > void SerializeUInt32( T ) = delete;
	template< typename T > void SerializeUInt64( T ) = delete;
	template< typename T > void SerializeInt8( T ) = delete;
	template< typename T > void SerializeInt16( T ) = delete;
	template< typename T > void SerializeInt32( T ) = delete;
	template< typename T > void SerializeInt64( T ) = delete;
	template< typename T > void SerializeFloat( T ) = delete;
	template< typename T > void SerializeDouble( T ) = delete;
	template< typename T > void SerializeBool( T ) = delete;
	template< typename T > void SerializeString( T ) = delete;
};

//------------------------------------------------------------------------------
// ae::BinaryWriter class
//------------------------------------------------------------------------------
class BinaryWriter : public ae::BinaryStream
{
public:
	//! Creates a stream where all serialization calls will write to \p data, up
	//! to \p length bytes. The stream will be invalidated if \p data is nullptr
	//! or \p length is 0. The lifetime of \p data must exceed the lifetime of
	//! the stream.
	BinaryWriter( void* data, uint32_t length );
	//! Creates a stream where all serialization calls will write to \p data. The
	//! internal buffer will grow as needed. The stream will be invalidated if
	//! \p data is nullptr. The lifetime of \p data must exceed the lifetime of
	//! the stream.
	BinaryWriter( ae::Array< uint8_t >* data );

	void SerializeUInt8( const uint8_t& valIn );
	void SerializeUInt16( const uint16_t& valIn );
	void SerializeUInt32( const uint32_t& valIn );
	void SerializeUInt64( const uint64_t& valIn );
	void SerializeInt8( const int8_t& valIn );
	void SerializeInt16( const int16_t& valIn );
	void SerializeInt32( const int32_t& valIn );
	void SerializeInt64( const int64_t& valIn );
	void SerializeFloat( const float& valIn );
	void SerializeDouble( const double& valIn );
	void SerializeBool( const bool& valIn );
	template< typename E > void SerializeEnum( const E& valIn );
	template< uint32_t N > void SerializeString( const Str< N >& strIn );
	void SerializeString( const char* strIn );
	//! Calls 'T ::Serialize( ae::BinaryWriter* ) const' on \p valIn if it
	//! exists or 'void Serialize( ae::BinaryWriter*, T& )' if it does not.
	template< typename T > void SerializeObject( const T& valIn );
	//! Serialize \p data to the stream. The \p length of the data is not
	//! serialized, and so the same length must be used for reading and writing.
	//! Use this with caution as platforms will have different struct packing,
	//! byte order, and alignment.
	void SerializeRaw( const void* dataIn, uint32_t length );

	//! See ae::BinaryStream::SerializeString(). This function only prevents
	//! shadowing of the base class function.
	void SerializeString( char* strInOut, uint32_t bufferSize ) { BinaryStream::SerializeString( strInOut, bufferSize ); }
	//! See ae::BinaryStream::SerializeObject(). This function only prevents
	//! shadowing of the base class function.
	template< typename T > void SerializeObject( T& valInOut );
private:
	// Prevent Serialize functions from being called accidentally through automatic conversions
	template< typename T > void SerializeUInt8( T ) = delete;
	template< typename T > void SerializeUInt16( T ) = delete;
	template< typename T > void SerializeUInt32( T ) = delete;
	template< typename T > void SerializeUInt64( T ) = delete;
	template< typename T > void SerializeInt8( T ) = delete;
	template< typename T > void SerializeInt16( T ) = delete;
	template< typename T > void SerializeInt32( T ) = delete;
	template< typename T > void SerializeInt64( T ) = delete;
	template< typename T > void SerializeFloat( T ) = delete;
	template< typename T > void SerializeDouble( T ) = delete;
	template< typename T > void SerializeBool( T ) = delete;
	template< typename T > void SerializeString( T ) = delete;
};

//------------------------------------------------------------------------------
// ae::BinaryReader class
//------------------------------------------------------------------------------
class BinaryReader : public ae::BinaryStream
{
public:
	//! Creates a stream where all serialization calls will read from \p data,
	//! up to \p length bytes. The stream will be invalidated if \p data is
	//! nullptr or \p length is 0. The lifetime of \p data must exceed the
	//! lifetime of the stream.
	BinaryReader( const void* data, uint32_t length );
	//! Creates a stream where all serialization calls will read from \p data.
	//! The lifetime of \p data must exceed the lifetime of the stream.
	BinaryReader( const ae::Array< uint8_t >& data );

	//! Returns the data at the current read head. Can be used with
	//! ae::BinaryStream::GetRemainingBytes() and ae::BinaryStream::DiscardReadData()
	//! to read chunks of data from the stream. Returns null if called on an
	//! invalid stream.
	const uint8_t* PeekReadData() const;
	//! Advances the read head by \p length bytes. If the end of the buffer
	//! is reached the stream is invalidated. Has no effect if called on an
	//! invalid stream.
	void DiscardReadData( uint32_t length );
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
	void Serialize( BinaryStream* s ) { s->SerializeUInt32( m_id ); }
	void Serialize( BinaryWriter* w ) const { w->SerializeUInt32( m_id ); }
private:
	uint32_t m_id = 0;
};
using RemoteId = NetId;
template<> inline uint32_t GetHash32( const ae::NetId& value ) { return ae::Hash32().HashType( value.GetInternalId() ).Get(); }

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
	//! Call once after ae::NetObjectServer::CreateNetObject(), will trigger Create
	//! event on clients. You can pass 'nullptr' and '0' as params, but you still
	//! must call this before the object will be created remotely. Clients can
	//! call ae::NetObject::GetInitData() to get the data set here.
	void SetInitData( const void* initData, uint32_t initDataLength );
	//! Call SetSyncData each frame to update that state of the clients NetObject.
	//! Only the most recent data is sent. Data is only sent when changed.
	void SetSyncData( const void* data, uint32_t length );
	//! Call as many times as necessary each tick
	void SendMessage( const void* data, uint32_t length );

	//------------------------------------------------------------------------------
	// Client
	//------------------------------------------------------------------------------
	//! Use GetInitData() after receiving a new NetObject from NetObjectClient::PumpCreate()
	//! to construct the object. Retrieves the data set by NetObject::SetInitData() on
	//! the server.
	const uint8_t* GetInitData() const;
	//! Retrieves the length of the data set by NetObject::SetInitData() on the server.
	//! Use in conjunction with NetObject::GetInitData().
	uint32_t InitDataLength() const;

	//! Only the latest sync data is ever available, so there's no need to read this
	//! data as if it was a stream.
	const uint8_t* GetSyncData() const;
	//! Check for new data from server
	uint32_t SyncDataLength() const;
	//! (Optional) Call to clear SyncDataLength() until new data is received
	void ClearSyncData();

	//! Get messages sent from the server. Call repeatedly until false is returned
	bool PumpMessages( Msg* msgOut );

	//! Returns true once the NetObject has been deleted on the server.
	//! Call NetObjectClient::Destroy() when you're done with it.
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
	~NetObjectClient();
	// The following sequence should be performed each frame
	
	//! 1) Handle raw data from server (call once when new data arrives)
	void ReceiveData( const uint8_t* data, uint32_t length );
	
	//! 2) Get new objects (call this repeatedly until no new NetObjects are returned)
	NetObject* PumpCreate();
	
	// 3) Handle new sync data with NetObject::GetSyncData()
	
	//! 4) Call this on ae::NetObjects once NetObject::IsPendingDestroy() returns true
	void Destroy( NetObject* pendingDestroy );
	
	
	NetId GetLocalId( RemoteId remoteId ) const { return m_remoteToLocalIdMap.Get( remoteId, {} ); }
	RemoteId GetRemoteId( NetId localId ) const { return m_localToRemoteIdMap.Get( localId, {} ); }

private:
	NetObject* m_CreateNetObject( ae::BinaryReader* rStream, bool allowResolve );
	void m_StartNetObjectDestruction( NetObject* netObject );
	uint32_t m_serverSignature = 0;
	uint32_t m_lastNetId = 0;
	bool m_delayCreationForDestroy = false;
	ae::Map< NetId, NetObject*, 0, ae::Hash32, ae::MapMode::Stable > m_netObjects = AE_ALLOC_TAG_NET;
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
	//! This data should be sent to a client with and consumed with
	//! ae::NetObjectClient::ReceiveData(). Call ae::NetObjectServer::UpdateSendData()
	//! once each network tick before calling this.
	const uint8_t* GetSendData() const;
	//! The length of the data that should be sent to a client with and consumed
	//! with ae::NetObjectClient::ReceiveData(). Call ae::NetObjectServer::UpdateSendData()
	//! once each network tick before calling this.
	uint32_t GetSendLength() const;

public:
	void m_UpdateSendData();
	void m_ClearPending();

	bool m_first = true;
	class NetObjectServer* m_replicaDB = nullptr;
	bool m_pendingClear = false;
	ae::Array< uint8_t > m_connData = AE_ALLOC_TAG_NET;
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
	//! Call each network tick before ae::NetObjectConnection::GetSendData()
	void UpdateSendData();
	
	//! Creates a server authoritative NetObject which will be replicated to
	//! clients through ae::NetObjectConnection and ae::NetObjectClient. Call
	//! ae::NetObject::SetInitData() on the object to finalize the object for
	//! remote creation. Call ae::NetObjectServer::DestroyNetObject() when finished.
	NetObject* CreateNetObject();
	//! Will cause the ae::NetObject to be detroyed on remote clients.
	//! Must be called for each ae::NetObject allocated with
	//! ae::NetObjectServer::CreateNetObject().
	void DestroyNetObject( NetObject* netObject );

	//! Allocate one ae::NetObjectConnection per client. Call
	//! ae::NetObjectServer::DestroyConnection() to clean it up.
	NetObjectConnection* CreateConnection();
	//! Must be called for each ae::NetObjectConnection allocated with ae::NetObjectServer::CreateConnection().
	void DestroyConnection( NetObjectConnection* connection );

private:
	uint32_t m_signature = 0;
	uint32_t m_lastNetId = 0;
	ae::Array< NetObject* > m_pendingCreate = AE_ALLOC_TAG_NET;
	ae::Map< NetId, NetObject*, 0, ae::Hash32, ae::MapMode::Stable > m_netObjects = AE_ALLOC_TAG_NET;
	ae::Array< NetObjectConnection* > m_connections = AE_ALLOC_TAG_NET; // @TODO: Rename m_connections
public:
	// Internal
	NetObject* GetNetObject( uint32_t index ) { return m_netObjects.GetValue( index ); }
	uint32_t GetNetObjectCount() const { return m_netObjects.Length(); }
};

//------------------------------------------------------------------------------
// Internal ae::IsosurfaceExtractor types
// @TODO: Move to IsosurfaceExtractor::VoxelIndex, and friend this gethash?
//------------------------------------------------------------------------------
struct VoxelIndex
{
	VoxelIndex() {}
	VoxelIndex( int32_t x, int32_t y, int32_t z ) : x( x ), y( y ), z( z ) {}
	bool operator==( const VoxelIndex& other ) const { return x == other.x && y == other.y && z == other.z; }
	int32_t x;
	int32_t y;
	int32_t z;
};
template<> inline uint32_t GetHash32( const VoxelIndex& index )
{
	// Create a hash using the index as a seed to prevent large consecutive map
	// entries, where collisions become very expensive to handle.
	constexpr uint32_t uint32MaxGridSize = 1625; // UINT32_MAX ^ (1/3)
	return ae::Hash32().HashType( index.x + uint32MaxGridSize * ( index.y + index.z * uint32MaxGridSize ) ).Get();
}
//------------------------------------------------------------------------------
// ae::IsosurfaceExtractor types
//------------------------------------------------------------------------------
struct IsosurfaceVertex
{
	ae::Vec4 position;
	ae::Vec3 normal;
};
typedef uint32_t IsosurfaceIndex;
struct IsosurfaceValue
{
	//! Signed distance to the isosurface
	float distance = INFINITY;
	//! Error margin (in the same units as distance), that will be checked near
	//! the surface during mesh extraction/generation. This would typically be
	//! the distance from the isosurface that non-strictly-SDF deformations are
	//! applied. Eg. Noise applied to the surface of a sphere.
	float distanceErrorMargin = 0.0f;
};
struct IsosurfaceStats
{
	double elapsedTime = 0.0; //!< The time it took to generate the mesh so far in seconds
	double voxelTime = 0.0; //!< The time it took to locate surface voxels so far in seconds
	double meshTime = 0.0; //!< The time it took to create the vertex data from the voxels so far in seconds
	float voxelProgress01 = 0.0f; //!< The progress of the voxel search in percent, from 0.0 to 1.0
	float meshProgress01 = 0.0f; //!< The progress of the vertex generation in percent, from 0.0 to 1.0
	// 64bit because some values are dealing with volumes that could be 2000 voxels cubed (or 8*10^9)
	uint64_t vertexCount = 0; //!< The max number of vertices to generate or 0 for no limit.
	uint64_t indexCount = 0; //!< The max number of indices to generate or 0 for no limit.
	uint64_t sampleRawCount = 0; //!< The number of samples done against IsosurfaceParams::samplefn
	uint64_t sampleCacheCount = 0; //!< The number of samples against the cache instead of IsosurfaceParams::samplefn
	uint64_t voxelCheckCount = 0; //!< The number of voxels processed
	uint64_t voxelMissCount = 0; //!< The number of voxels processed resulting in no vertex
	uint64_t voxelWorkingSize = 0; //!< The number of "duals" stored for work on edges
	uint64_t voxelSearchProgress = 0; //!< The number of voxels of the input bounds searched so far
	uint64_t voxelAABBSize = 0; //!< The total number of voxels that could have been processed given the input bounds
};
using IsosurfaceSampleFn = ae::IsosurfaceValue(*)( const void* userData, ae::Vec3 position );
using IsosurfaceStatsFn = void(*)( const void* userData, const ae::IsosurfaceStats& stats );
struct IsosurfaceParams
{
	IsosurfaceSampleFn sampleFn = nullptr;
	IsosurfaceStatsFn statsFn = nullptr;
	const void* userData = nullptr;
	
	//! The bounds for sampling the sdf function and mesh generation
	ae::AABB aabb = ae::AABB();
	
	//! The maximum number of vertices that will be generated
	uint32_t maxVerts = 0;
	//! The maximum number of indices that will be generated
	uint32_t maxIndices = 0;
	
	//! The offset from a particular positional sample for its normals to be
	//! sampled. Low values will be more accurate, but higher values will take
	//! the average of a larger area. This should be low for an SDF with high
	//! frequency noise.
	float normalSampleOffset = 0.1f;
	//! Performs extra dual contouring operations when true. The curves,
	//! corners, and edges of the results will be more precise, but the mesh
	//! generation will be slower. The vertex count and triangle indices will be
	//! nearly identical.
	bool dualContouring = false;

	ae::Array< ae::AABB >* octree = nullptr;
	ae::Array< ae::Vec3 >* errors = nullptr;
	std::optional< ae::Vec3 > debugPos;
};

//------------------------------------------------------------------------------
// ae::IsosurfaceExtractor class
//------------------------------------------------------------------------------
struct IsosurfaceExtractor
{
	IsosurfaceExtractor( ae::Tag tag );
	//! Optionally call this to pre-allocate internal vertex and index storage.
	//! This can dramatically speed up one-off mesh generation, as internal buffers
	//! use standard dynamic array growth mechanisms. When an ae::IsosurfaceExtractor
	//! is used multiple times, the effect of this reserve will be very minimal
	//! as buffer sizes are maintained between calls to ae::IsosurfaceExtractor::Generate().
	void Reserve( uint32_t vertexCount, uint32_t indexCount );
	//! Generates the isosurface mesh, writing it to ae::IsosurfaceExtractor::vertices
	//! and ae::IsosurfaceExtractor::indices. Returns true on completion, unless
	//! ae::IsosurfaceParams::maxVerts or ae::IsosurfaceParams::maxIndices are
	//! set and a limit is reached. Partial mesh data will not be available on
	//! failure, but ae::IsosurfaceStats can be retrieved with ae::IsosurfaceExtractor::GetStats().
	bool Generate( const ae::IsosurfaceParams& params );
	//! Clears all results from ae::IsosurfaceExtractor::Generate(), including
	//! from ae::IsosurfaceStats. Note that buffer sizes will be maintained even
	//! if this is called.
	void Reset();
	//! Returns stats from the previous call to ae::IsosurfaceExtractor::Generate()
	const IsosurfaceStats& GetStats() const { return m_stats; }
	
	ae::Array< IsosurfaceVertex > vertices;
	ae::Array< IsosurfaceIndex > indices;

private:
	static constexpr IsosurfaceIndex kInvalidIsosurfaceIndex = ~0;
	const ae::Int3 kChildOffsets[ 8 ] = { { -1, -1, -1 }, { 1, -1, -1 }, { -1, 1, -1 }, { 1, 1, -1 }, { -1, -1, 1 }, { 1, -1, 1 }, { -1, 1, 1 }, { 1, 1, 1 } };
	const ae::Int3 cornerOffsets[ 3 ] = { { 0, 1, 1 }, /* EDGE_TOP_FRONT_BIT */ { 1, 0, 1 }, /* EDGE_TOP_RIGHT_BIT */ { 1, 1, 0 } /* EDGE_SIDE_FRONTRIGHT_BIT */ };
	static constexpr uint16_t EDGE_TOP_FRONT_BIT = ( 1 << 0 );
	static constexpr uint16_t EDGE_TOP_RIGHT_BIT = ( 1 << 1 );
	static constexpr uint16_t EDGE_SIDE_FRONTRIGHT_BIT = ( 1 << 2 );
	const ae::Int3 offsets_EDGE_TOP_FRONT_BIT[ 4 ] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
	const ae::Int3 offsets_EDGE_TOP_RIGHT_BIT[ 4 ] = { { 0, 0, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 1, 0, 1 } };
	const ae::Int3 offsets_EDGE_SIDE_FRONTRIGHT_BIT[ 4 ] = { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 0, 0 }, { 1, 1, 0 } };
	static constexpr uint16_t mask[ 3 ] = { EDGE_TOP_FRONT_BIT, EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT };
	struct Voxel
	{
		// 3 planes whose intersections are used to position vertices within voxel
		// EDGE_TOP_FRONT_BIT, EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT
		ae::Vec3 edgeNormal[ 3 ];
		ae::Vec3 edgePos[ 3 ];
		IsosurfaceIndex index = kInvalidIsosurfaceIndex;
		uint16_t edgeBits = 0;
	};
	bool m_Generate( ae::Int3 center, uint32_t halfSize );
	bool m_DoVoxel( int32_t x, int32_t y, int32_t z );
	inline IsosurfaceValue m_DualSample( ae::Int3 pos );
	inline IsosurfaceValue m_Sample( ae::Vec3 pos );
	void m_UpdateStats();
	IsosurfaceParams m_params;
	ae::Int3 m_minInclusive = ae::Int3( 0 );
	ae::Int3 m_maxInclusive = ae::Int3( 0 );
	double m_startVoxelTime = 0.0;
	double m_startMeshTime = 0.0;
	IsosurfaceStats m_stats;
	IsosurfaceStats m_statsPrev;
	ae::Map< VoxelIndex, Voxel > m_voxels;
	ae::Map< VoxelIndex, IsosurfaceValue > m_dualSamples;
};

//! \defgroup Meta
//! @{

//------------------------------------------------------------------------------
// Macros to force module linking
//------------------------------------------------------------------------------
// clang-format off
//! Call signature: AE_FORCE_LINK_CLASS( Namespace0, ..., NameSpaceN, MyType );
#define AE_FORCE_LINK_CLASS(...) \
	extern int AE_GLUE_UNDERSCORE(_ae_force_link, __VA_ARGS__); \
	struct AE_GLUE_UNDERSCORE(_ae_ForceLink, __VA_ARGS__) { AE_GLUE_UNDERSCORE(_ae_ForceLink, __VA_ARGS__)() { AE_GLUE_UNDERSCORE(_ae_force_link, __VA_ARGS__) = 1; } }; \
	AE_GLUE_UNDERSCORE(_ae_ForceLink, __VA_ARGS__) AE_GLUE_UNDERSCORE(_ae_forceLink, __VA_ARGS__);

//------------------------------------------------------------------------------
// Meta class registration macros
//------------------------------------------------------------------------------
//! Registers a new type that can be retrieved with ae::GetClassType( "Namespace0::NameSpace1::MyType" )
//! or ae::GetClassType< Namespace0::NameSpace1::MyType >(). Call this once in the
//! global scope of any cpp file. The class must indirectly inherit from from
//! ae::Inheritor< ae::Object, MyType >.
#define AE_REGISTER_CLASS( _CLASS ) AE_REGISTER_CLASS_IMPL( AE_GLUE_UNDERSCORE(_CLASS), AE_GLUE_TYPE(_CLASS) )
//! Registers the class variable 'Namespace0::...::NamespaceN::MyType::classVar'
#define AE_REGISTER_CLASS_VAR( _CLASS, _V ) AE_REGISTER_NAMESPACECLASS_VAR( (_CLASS), _V )
//! Registers an instance of an attribute with a class. The attribute type must
//! be registered with AE_REGISTER_CLASS() before this is called.
#define AE_REGISTER_CLASS_ATTRIBUTE( _CLASS, _A, _ARGS ) AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (_CLASS), (_A), _ARGS )
//! Registers an instance of an attribute with a class variable. The attribute
//! must be registered with AE_REGISTER_CLASS() before this is called.
#define AE_REGISTER_CLASS_VAR_ATTRIBUTE( _CLASS, _V, _A, _ARGS ) AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( (_CLASS), _V, (_A), _ARGS )

//! Registers a new type that can be retrieved with ae::GetClassType( "Namespace0::NameSpace1::MyType" )
//! or ae::GetClassType< Namespace0::NameSpace1::MyType >(). Call this once in the
//! global scope of any cpp file. The class must indirectly inherit from from
//! ae::Inheritor< ae::Object, MyType >.
//! Call signature: AE_REGISTER_NAMESPACECLASS( (Namespace0, ..., NameSpaceN, MyType) );
#define AE_REGISTER_NAMESPACECLASS( _CLASS ) AE_REGISTER_CLASS_IMPL( AE_GLUE_UNDERSCORE _CLASS, AE_GLUE_TYPE _CLASS)
//! Registers the class variable 'Namespace0::...::NamespaceN::MyType::classVar'
//! Call signature: AE_REGISTER_NAMESPACECLASS_VAR( (Namespace0, ..., NameSpaceN, MyType), classVar );
#define AE_REGISTER_NAMESPACECLASS_VAR( _CLASS, _V ) AE_REGISTER_CLASS_VAR_IMPL(AE_GLUE_UNDERSCORE _CLASS, AE_GLUE_TYPE _CLASS, _V)
//! Registers an instance of an attribute with a class. The attribute type must
//! be registered with AE_REGISTER_NAMESPACECLASS() before this is called.
#define AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( _CLASS, _A, _ARGS ) AE_REGISTER_CLASS_ATTRIBUTE_IMPL( AE_GLUE_UNDERSCORE _CLASS, AE_GLUE_TYPE _CLASS, AE_GLUE_UNDERSCORE _A, AE_GLUE_TYPE _A, _ARGS )
//! Registers an instance of an attribute with a class variable. The attribute
//! must be registered with AE_REGISTER_NAMESPACECLASS() before this is called.
#define AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( _CLASS, _V, _A, _ARGS ) AE_REGISTER_CLASS_VAR_ATTRIBUTE_IMPL( AE_GLUE_UNDERSCORE _CLASS, AE_GLUE_TYPE _CLASS, _V, AE_GLUE_UNDERSCORE _A, AE_GLUE_TYPE _A, _ARGS )

//------------------------------------------------------------------------------
// External enum definer and registerer
//------------------------------------------------------------------------------
//! Define a new enum (must register with AE_REGISTER_ENUM_CLASS)
#define AE_DEFINE_ENUM_CLASS( E, T, ... ) \
	enum class E : T { \
		__VA_ARGS__ \
	}; \
	template<> const ae::EnumType* ae::GetEnumType< E >(); \
	template<> \
	struct ae::TypeT< E > : public ae::EnumType { \
		TypeT() : EnumType( #E, "", sizeof(E), std::is_signed_v< T > ) {}\
		static ae::Type* Get() { static ae::TypeT< E > s_type; return &s_type; } \
		ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< E >(); } \
	}; \
	struct _EnumValues##E { _EnumValues##E( const char* values = #__VA_ARGS__ ) : values( values ) {} const char* values; };\
	inline std::ostream &operator << ( std::ostream &os, E e ) { os << ae::GetEnumType< E >()->GetNameByValue( (int32_t)e ); return os; } \
	namespace ae { template<> inline std::string ToString( E e ) { return ae::GetEnumType< E >()->GetNameByValue( e ); } } \
	namespace ae { template<> inline E FromString( const char* str, const E& e ) { return ae::GetEnumType< E >()->GetValueFromString( str, e ); } } \
	namespace ae { template<> inline uint32_t GetHash32( const E& e ) { return (uint32_t)e; } }

//! Register an enum defined with AE_DEFINE_ENUM_CLASS
#define AE_REGISTER_ENUM_CLASS( E )\
	ae::_RegisterEnum< E > ae_enum_creator_##E( #E, _EnumValues##E().values );\
	template<> ae::Type* ae::FindMetaRegistrationFor< E >() { return ae::TypeT< E >::Get(); }\
	template<> const ae::EnumType* ae::GetEnumType< E >(){\
		static _StaticCacheVar< const ae::EnumType* > s_enum = nullptr;\
		if( !s_enum ) { s_enum = GetEnumType( #E ); }\
		return s_enum;\
	}
//------------------------------------------------------------------------------
// External c-style enum registerer
//------------------------------------------------------------------------------
//! Register an already defined c-style enum type
#define AE_REGISTER_ENUM( E ) \
	template<> const ae::EnumType* ae::GetEnumType< E >() {\
		static _StaticCacheVar< const ae::EnumType* > s_enum = nullptr;\
		if( !s_enum ) { s_enum = GetEnumType( #E ); }\
		return s_enum;\
	}\
	template<> \
	struct ae::TypeT< E > : public ae::EnumType { \
		TypeT() : EnumType( #E, "", sizeof(E), std::is_signed_v< std::underlying_type_t< E > > ) {}\
		static ae::Type* Get() { static ae::TypeT< E > s_type; return &s_type; } \
		ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< E >(); } \
	}; \
	ae::_RegisterExistingEnumOrValue< E > ae_enum_creator_##E; \
	template<> ae::Type* ae::FindMetaRegistrationFor< E >() { return ae::TypeT< E >::Get(); }\
	namespace ae { template<> std::string ToString( E e ) { return ae::GetEnumType< E >()->GetNameByValue( e ); } } \
	namespace ae { template<> E FromString( const char* str, const E& e ) { return ae::GetEnumType< E >()->GetValueFromString( str, e ); } }

//! Register an already defined c-style enum type where each value has a prefix
#define AE_REGISTER_ENUM_PREFIX( E, PREFIX ) \
	template<> const ae::EnumType* ae::GetEnumType< E >() {\
		static _StaticCacheVar< const ae::EnumType* > s_enum = nullptr;\
		if( !s_enum ) { s_enum = GetEnumType( #E ); }\
		return s_enum;\
	} \
	template<> \
	struct ae::TypeT< E > : public ae::EnumType { \
		TypeT() : EnumType( #E, #PREFIX, sizeof(E), std::is_signed_v< std::underlying_type_t< E > > ) {}\
		static ae::Type* Get() { static ae::TypeT< E > s_type; return &s_type; } \
		ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< E >(); } \
	}; \
	ae::_RegisterExistingEnumOrValue< E > ae_enum_creator_##E;\
	template<> ae::Type* ae::FindMetaRegistrationFor< E >() { return ae::TypeT< E >::Get(); }\

//! Register c-style enum value
#define AE_REGISTER_ENUM_VALUE( E, V ) \
	ae::_RegisterExistingEnumOrValue< E > ae_enum_creator_##E##_##V( #V, V );

//! Register c-style enum value with a manually specified name
#define AE_REGISTER_ENUM_VALUE_NAME( E, V, N ) \
	ae::_RegisterExistingEnumOrValue< E > ae_enum_creator_##E##_##V( #N, V );

//------------------------------------------------------------------------------
// External enum class registerer
//------------------------------------------------------------------------------
//! Register an already defined enum class type
#define AE_REGISTER_ENUM_CLASS2( E ) \
	template<> const ae::EnumType* ae::GetEnumType< E >() {\
		static _StaticCacheVar< const ae::EnumType* > s_enum = nullptr;\
		if( !s_enum ) { s_enum = GetEnumType( #E ); }\
		return s_enum;\
	} \
	template<> \
	struct ae::TypeT< E > : public ae::EnumType { \
		TypeT() : EnumType( #E, "", sizeof(E), std::is_signed_v< std::underlying_type_t< E > > ) {}\
		static ae::Type* Get() { static ae::TypeT< E > s_type; return &s_type; } \
		ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< E >(); } \
	}; \
	namespace aeEnums::_##E { ae::_RegisterExistingEnumOrValue< E > ae_enum_creator; }\
	template<> ae::Type* ae::FindMetaRegistrationFor< E >() { return ae::TypeT< E >::Get(); }
	// @NOTE: Nested namespace declaration requires C++17

//! Register enum class value
#define AE_REGISTER_ENUM_CLASS2_VALUE( E, V ) \
	namespace aeEnums::_##E { ae::_RegisterExistingEnumOrValue< E > ae_enum_creator_##V( #V, E::V ); }

// clang-format on

//------------------------------------------------------------------------------
// Meta constants
//------------------------------------------------------------------------------
using TypeId = uint32_t;
using TypeName = ae::Str64;
class Type;
class ClassType;
class ClassVar;
class EnumType;
const TypeId kInvalidTypeId = 0;

//------------------------------------------------------------------------------
// Meta limit defines
//------------------------------------------------------------------------------
#ifndef AE_MAX_META_TYPES_CONFIG
	#define AE_MAX_META_TYPES_CONFIG 128
#endif
#ifndef AE_MAX_META_VARS_CONFIG
	#define AE_MAX_META_VARS_CONFIG 16
#endif
#ifndef AE_MAX_META_ENUM_TYPES_CONFIG
	#define AE_MAX_META_ENUM_TYPES_CONFIG 32
#endif
#ifndef AE_MAX_META_ENUM_VALUES_CONFIG
	#define AE_MAX_META_ENUM_VALUES_CONFIG 64
#endif
#ifndef AE_MAX_META_PROPS_CONFIG
	#define AE_MAX_META_PROPS_CONFIG 8
#endif
#ifndef AE_MAX_META_PROP_LIST_LENGTH_CONFIG
	#define AE_MAX_META_PROP_LIST_LENGTH_CONFIG 8
#endif
#ifndef AE_MAX_META_ATTRIBUTES_CONFIG
	#define AE_MAX_META_ATTRIBUTES_CONFIG 8
#endif
//! The maximum number of types that can be registered with AE_REGISTER_CLASS().
//! This value can be overridden by defining AE_MAX_META_TYPES_CONFIG. See
//! AE_CONFIG_FILE for more details.
const uint32_t kMaxMetaTypes = AE_MAX_META_TYPES_CONFIG;
//! The maximum number of variables that can be registered per type with
//! AE_REGISTER_CLASS_VAR(). This value can be overridden by defining
//! AE_MAX_META_VARS_CONFIG. See AE_CONFIG_FILE for more details.
const uint32_t kMaxMetaVars = AE_MAX_META_VARS_CONFIG;
//! The maximum number of enum types that can be registered with the
//! AE_REGISTER_ENUM*() functions. This value can be overridden by defining
//! AE_MAX_META_ENUM_TYPES_CONFIG. See AE_CONFIG_FILE for more details.
const uint32_t kMaxMetaEnumTypes = AE_MAX_META_ENUM_TYPES_CONFIG;
//! The maximum number of enum values allowed per registered enum type. This
//! value can be overridden by defining AE_MAX_META_ENUM_VALUES_CONFIG. See
//! AE_CONFIG_FILE for more details.
const uint32_t kMaxMetaEnumValues = AE_MAX_META_ENUM_VALUES_CONFIG;
//! The maximum number of properties that can be registered with the
//! AE_REGISTER_*_PROPERTY() functions per class, var, enum, etc. This value
//! can be overridden by defining AE_MAX_META_PROPS_CONFIG. See AE_CONFIG_FILE
//! for more details.
const uint32_t kMaxMetaProps = AE_MAX_META_PROPS_CONFIG;
//! The maximum number of values that can be registered per property with the
//! AE_REGISTER_*_PROPERTY_VALUE() functions. This value can be
//! overridden by defining AE_MAX_META_PROP_LIST_LENGTH_CONFIG. See
//! AE_CONFIG_FILE for more details.
const uint32_t kMaxMetaPropListLength = AE_MAX_META_PROP_LIST_LENGTH_CONFIG;
//! The maximum number of attributes that can be registered with
//! AE_REGISTER_CLASS_ATTRIBUTE(), AE_REGISTER_CLASS_VAR_ATTRIBUTE(), and
//! AE_REGISTER_ENUM_ATTRIBUTE() per class, var, or enum. This value can
//! be overridden by defining AE_MAX_META_ATTRIBUTES_CONFIG. See AE_CONFIG_FILE
//! for more details.
const uint32_t kMaxMetaAttributes = AE_MAX_META_ATTRIBUTES_CONFIG;

//------------------------------------------------------------------------------
// ae::Object
//! Base class for all meta registered objects. Inherit from this using
//! ae::Inheritor and register your classes with AE_REGISTER_CLASS.
//------------------------------------------------------------------------------
class Object
{
public:
	virtual ~Object() {}
	static const char* GetParentTypeName() { return ""; }
	static const ae::ClassType* GetParentType() { return nullptr; }
	ae::TypeId GetTypeId() const { return _metaTypeId; }
	ae::TypeId _metaTypeId = ae::kInvalidTypeId;
};

//------------------------------------------------------------------------------
// ae::Inheritor
//! See ae::Object and AE_REGISTER_CLASS() for usage.
//------------------------------------------------------------------------------
template< typename Parent, typename This >
class Inheritor : public Parent
{
public:
	Inheritor();
	typedef Parent aeBaseType;
	static const char* GetParentTypeName();
	static const ae::ClassType* GetParentType();
};

//------------------------------------------------------------------------------
// External meta functions
//------------------------------------------------------------------------------
//! Get the number of registered ae::ClassType's
uint32_t GetClassTypeCount();
//! Get a registered ae::ClassType by index
const ae::ClassType* GetClassTypeByIndex( uint32_t i );
//! Get a registered ae::ClassType by id. Same as ae::ClassType::GetId()
const ae::ClassType* GetClassTypeById( ae::TypeId id );
//! Get a registered ae::ClassType from a type name
const ae::ClassType* GetClassTypeByName( const char* typeName );
//! Get a registered ae::ClassType from an ae::Object
const ae::ClassType* GetClassTypeFromObject( const ae::Object& obj );
//! Get a registered ae::ClassType from a pointer to an ae::Object
const ae::ClassType* GetClassTypeFromObject( const ae::Object* obj );
//! Get a registered ae::ClassType directly from a type
template< typename T > const ae::ClassType* GetClassType();
//! Get a registered ae::EnumType by name
const class ae::EnumType* GetEnumType( const char* enumName );
//! Get a registered ae::TypeId from an ae::Object
ae::TypeId GetObjectTypeId( const ae::Object* obj );
//! Get a registered ae::TypeId from a type name
ae::TypeId GetTypeIdFromName( const char* name );
//! Removes const, pointer, and reference qualifiers from a type. Usage:
//! using U = typename ae::RemoveTypeQualifiers< T >;
template< typename T > using RemoveTypeQualifiers = std::remove_cv_t< std::remove_reference_t< std::remove_pointer_t< std::decay_t< T > > > >;
//! Returns an integer id for the given type, which ignores const, pointer, and
//! reference qualifiers.
template< typename T > ae::TypeId GetTypeIdWithoutQualifiers();
//! Returns an integer id for the given type, which respects const, pointer, and
//! reference qualifiers.
template< typename T > ae::TypeId GetTypeIdWithQualifiers();

//------------------------------------------------------------------------------
// ae::Attribute class
//! Register with AE_REGISTER_CLASS()
//------------------------------------------------------------------------------
class Attribute
{
public:
	static const char* GetParentTypeName() { return ""; }
	static const ae::ClassType* GetParentType() { return nullptr; }
	ae::TypeId GetTypeId() const { return _metaTypeId; }
	ae::TypeId _metaTypeId = ae::kInvalidTypeId;
};

//------------------------------------------------------------------------------
// ae::SourceFileAttribute class
//------------------------------------------------------------------------------
class SourceFileAttribute final : public ae::Inheritor< ae::Attribute, SourceFileAttribute >
{
public:
	ae::Str256 path;
	uint32_t line = 0;
};
inline std::ostream& operator << ( std::ostream& os, const ae::SourceFileAttribute& attribute ) { os << attribute.path.c_str() << ":" << attribute.line; return os; }

//------------------------------------------------------------------------------
// ae::AttributeList class
//! Contains attributes registered with AE_REGISTER_CLASS_ATTRIBUTE(),
//! AE_REGISTER_CLASS_VAR_ATTRIBUTE(), or AE_REGISTER_ENUM_ATTRIBUTE()
//! functions. Define AE_MAX_META_ATTRIBUTES_CONFIG in aeConfig.h to change the
//! maximum number of attributes that can be registered per class, var, and
//! enum. Note that attributes must be registered with AE_REGISTER_CLASS().
//------------------------------------------------------------------------------
class AttributeList
{
public:
	template< typename T > const T* TryGet( uint32_t idx = 0 ) const;
	template< typename T > uint32_t GetCount() const;
	template< typename T > bool Has() const;

private:
	template< typename T > friend class _AttributeCreator;
	void m_Add( ae::Attribute* attribute );
	struct _Info { uint32_t start; uint32_t count; };
	ae::Map< ae::TypeId, _Info, kMaxMetaAttributes > m_attributeTypes;
	ae::Array< ae::Attribute*, kMaxMetaAttributes > m_attributes;
};

//------------------------------------------------------------------------------
// ae::DataPointer
//------------------------------------------------------------------------------
class DataPointer
{
public:
	DataPointer() = default;
	template< typename T > explicit DataPointer( T* data );
	DataPointer( const ae::Type& varType, void* data );
	DataPointer( const ae::ClassVar* var, ae::Object* object );

	//! Returns true if the data is valid.
	explicit operator bool() const;
	//! Returns the ae::Type of the data. Must have valid data.
	const ae::Type& GetVarType() const;
	//! Returns data pointer, optionally provide the type of the caller to
	//! perform a type check.
	void* Get( const ae::Type* caller = nullptr ) const;
	
	bool operator == ( const ae::DataPointer& other ) const;
	bool operator != ( const ae::DataPointer& other ) const;

private:
	friend class ConstDataPointer;
	const ae::Type* m_varType = nullptr;
	void* m_data = nullptr;
};

//------------------------------------------------------------------------------
// ae::ConstDataPointer
//------------------------------------------------------------------------------
class ConstDataPointer
{
public:
	ConstDataPointer() = default;
	ConstDataPointer( DataPointer varData );
	template< typename T > explicit ConstDataPointer( const T* data );
	ConstDataPointer( const ae::Type& varType, const void* data );
	ConstDataPointer( const ae::ClassVar* var, const ae::Object* object );
	
	//! Returns true if the data is valid.
	explicit operator bool() const;
	//! Returns the ae::Type of the data. Must have valid data.
	const ae::Type& GetVarType() const;
	//! Returns data pointer, optionally provide the type of the caller to
	//! perform a type check.
	const void* Get( const ae::Type* caller = nullptr ) const;

	bool operator == ( const ae::ConstDataPointer& other ) const;
	bool operator != ( const ae::ConstDataPointer& other ) const;

private:
	const ae::Type* m_varType = nullptr;
	const void* m_data = nullptr;
};

//------------------------------------------------------------------------------
// ae::ObjectPointerToStringFn
//! Used with ae::PointerType to convert object pointers to strings, most
//! useful when serializing references to game objects to json etc. Should be
//! implemented with an encoding that matches ae::StringToObjectPointerFn.
//------------------------------------------------------------------------------
using ObjectPointerToStringFn = std::string ( * )( const void* userData, const ae::Object* obj );

//------------------------------------------------------------------------------
// ae::StringToObjectPointerFn
//! Used with ae::PointerType to convert strings to object pointers, most
//! useful when serializing game objects from json etc. Should be implemented
//! with a lookup into the game object system. The encoding of the given string
//! should match ae::ObjectPointerToStringFn.
//------------------------------------------------------------------------------
using StringToObjectPointerFn = bool ( * )( const void* userData, const char* pointerVal, ae::Object** objOut );

//------------------------------------------------------------------------------
// ae::Type
//------------------------------------------------------------------------------
class Type
{
public:
	virtual ~Type() {}
	template< typename T > const T* AsVarType() const;
	template< typename T > bool IsSameBaseVarType() const;
	template< typename T > bool IsSameExactVarType() const;
	bool IsSameBaseVarType( const Type& other ) const;
	bool IsSameExactVarType( const Type& other ) const;

	virtual ae::TypeId GetBaseVarTypeId() const = 0;
	virtual ae::TypeId GetExactVarTypeId() const = 0;
protected:
	Type() = default;
	Type( const Type& ) = delete;
	Type( Type&& ) = delete;
	Type& operator=( const Type& ) = delete;
	Type& operator=( Type&& ) = delete;
};

//------------------------------------------------------------------------------
// ae::BasicType
//------------------------------------------------------------------------------
class BasicType : public ae::Type
{
public:
	enum Type
	{
		String,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		Int8,
		Int16,
		Int32,
		Int64,
		Int2,
		Int3,
		Bool,
		Float,
		Double,
		Vec2,
		Vec3,
		Vec4,
		// @TODO: Quaternion
		Matrix4,
		Color,

		Class, // @TODO: Remove
		Enum, // @TODO: Remove
		Pointer, // @TODO: Remove
		CustomRef, // @TODO: Remove
		None, // @TODO: Remove
	};

	virtual ae::BasicType::Type GetType() const = 0;
	virtual uint32_t GetSize() const = 0;

	std::string GetVarDataAsString( ae::ConstDataPointer varData ) const;
	bool SetVarDataFromString( ae::DataPointer varData, const char* value ) const;
	template< typename T > bool GetVarData( ae::ConstDataPointer varData, T* valueOut ) const;
	template< typename T > bool SetVarData( ae::DataPointer varData, const T& value ) const;
	
	// Internal
	ae::TypeId GetBaseVarTypeId() const override { return ae::GetTypeIdWithoutQualifiers< BasicType >(); }
};

//------------------------------------------------------------------------------
// ae::EnumType
//------------------------------------------------------------------------------
class EnumType : public ae::Type
{
public:
	//--------------------------------------------------------------------------
	// Type info
	//--------------------------------------------------------------------------
	const char* GetName() const { return m_name.c_str(); }
	const char* GetPrefix() const { return m_prefix.c_str(); }
	uint32_t TypeSize() const { return m_size; }
	bool TypeIsSigned() const { return m_isSigned; }

	//--------------------------------------------------------------------------
	// Enum values
	//--------------------------------------------------------------------------
	template< typename T > std::string GetNameByValue( T value ) const;
	template< typename T > bool GetValueFromString( const char* str, T* valueOut ) const;
	template< typename T > T GetValueFromString( const char* str, T defaultValue ) const;
	template< typename T > bool HasValue( T value ) const;
	int32_t GetValueByIndex( int32_t index ) const;
	std::string GetNameByIndex( int32_t index ) const;
	uint32_t Length() const;

	//--------------------------------------------------------------------------
	// ae::DataPointer
	//--------------------------------------------------------------------------
	std::string GetVarDataAsString( ae::ConstDataPointer varData ) const;
	bool SetVarDataFromString( ae::DataPointer varData, const char* value ) const;
	template< typename T > bool GetVarData( ae::ConstDataPointer varData, T* valueOut ) const;
	template< typename T > bool SetVarData( ae::DataPointer varData, const T& value ) const;

	//--------------------------------------------------------------------------
	// Attributes
	//--------------------------------------------------------------------------
	ae::AttributeList attributes;

	//--------------------------------------------------------------------------
	// Internal
	//--------------------------------------------------------------------------
private:
	ae::TypeName m_name;
	ae::TypeName m_prefix;
	uint32_t m_size;
	bool m_isSigned;
	ae::Map< int32_t, std::string, kMaxMetaEnumValues > m_enumValueToName;
	ae::Map< std::string, int32_t, kMaxMetaEnumValues > m_enumNameToValue;
protected:
	EnumType( const char* name, const char* prefix, uint32_t size, bool isSigned );
public:
	const ae::EnumType* GetEnumType() const { return this; } // @TODO: Remove
	void m_AddValue( const char* name, int32_t value );
	ae::TypeId GetBaseVarTypeId() const override { return ae::GetTypeIdWithoutQualifiers< EnumType >(); }
};

//------------------------------------------------------------------------------
// ae::PointerType
//------------------------------------------------------------------------------
class PointerType : public ae::Type
{
public:
	//! Returns the type pointed to by this type
	virtual const ae::Type& GetInnerVarType() const = 0;

	//! Returns a pointer to the data referenced by the given \p varData. Note
	//! that like a normal C-pointer dereference, the const-ness of the
	//! referenced value is separate from the const-ness of the pointer.
	ae::DataPointer Dereference( ae::ConstDataPointer pointer ) const;

	//! Writes \p value to the given \p pointer, returning true on success.
	virtual bool SetRef( ae::DataPointer pointer, ae::Object* value ) const = 0;
	//! Returns a pointer to the inner value of \p pointer, unless given null.
	template< typename T > T** GetRef( ae::DataPointer pointer ) const;
	//! Returns a pointer to the inner value of \p pointer, unless given null.
	template< typename T > T*const* GetRef( ae::ConstDataPointer pointer ) const;


	// Internal
	ae::TypeId GetBaseVarTypeId() const override { return ae::GetTypeIdWithoutQualifiers< PointerType >(); }
	virtual ae::BasicType::Type GetBasicType() const { return ae::BasicType::Pointer; } // @HACK: Remove
	virtual bool SetRefFromString( ae::DataPointer pointer, const char* value, StringToObjectPointerFn fn, const void* userData ) const = 0;
	virtual std::string GetStringFromRef( ae::ConstDataPointer pointer, ObjectPointerToStringFn fn, const void* userData ) const = 0;
};

//------------------------------------------------------------------------------
// ae::OptionalType
//------------------------------------------------------------------------------
class OptionalType : public ae::Type
{
public:
	//! Returns the type contained by this optional type
	virtual const ae::Type& GetInnerVarType() const = 0;
	
	//! Conditionally returns a pointer to the value held by \p optional, or
	//! or null if the underlying optional type holds no valid data. Returns
	//! null if \p optional is null.
	virtual ae::DataPointer TryGet( ae::DataPointer optional ) const = 0;
	//! Conditionally returns a pointer to the value contained by \p optional,
	//! or null if the underlying optional type holds no valid data. Returns
	//! null if \p optional is null.
	virtual ae::ConstDataPointer TryGet( ae::ConstDataPointer optional ) const = 0;
	//! This function attempts to return a pointer to the value contained by
	//! \p optional, otherwise a value will be constructed if possible and
	//! returned. Return null if \p optional is null.
	virtual ae::DataPointer GetOrInsert( ae::DataPointer optional ) const = 0;
	
	//! Destroys the contained value if \p optional is not null, otherwise does
	//! nothing.
	virtual void Clear( ae::DataPointer optional ) const = 0;

	// Internal
	ae::TypeId GetBaseVarTypeId() const override { return ae::GetTypeIdWithoutQualifiers< OptionalType >(); }
};

//------------------------------------------------------------------------------
// ae::ArrayType
//------------------------------------------------------------------------------
class ArrayType : public ae::Type
{
public:
	//! Returns the type contained by this array type
	virtual const ae::Type& GetInnerVarType() const = 0;
	
	//! Returns a pointer into the given \p array if \p idx is valid (less than
	//! the result of ae::ArrayType::GetLength()), otherwise returns a null
	//! data pointer. Note that depending on the implementation of the
	//! underlying array, it may be unsafe to hold a pointer to an element after
	//! modifying the array.
	virtual ae::DataPointer GetElement( ae::DataPointer array, uint32_t idx ) const = 0;
	//! Returns a pointer into the given \p array if \p idx is valid (less than
	//! the result of ae::ArrayType::GetLength()), otherwise returns a null
	//! data pointer. Note that depending on the implementation of the
	//! underlying array, it may be unsafe to hold a pointer to an element after
	//! modifying the array.
	virtual ae::ConstDataPointer GetElement( ae::ConstDataPointer array, uint32_t idx ) const = 0;
	
	//! Attempts to resize the underlying \p array to \p size, returning the new
	//! length. Resizing to the given \p size may not be possible depending on
	//! the its type, so take care to check the return value.
	virtual uint32_t Resize( ae::DataPointer array, uint32_t size ) const = 0;
	//! Returns the current number of elements contained by the array
	virtual uint32_t GetLength( ae::ConstDataPointer array ) const = 0;
	
	//! Returns the largest size this array type could grow to, or uint max for
	//! no hard limit.
	virtual uint32_t GetMaxLength() const = 0;
	//! Returns true if the number of elements that this array type can hold
	//! can be modified.
	virtual uint32_t IsFixedLength() const = 0;

	// Internal
	ae::TypeId GetBaseVarTypeId() const override { return ae::GetTypeIdWithoutQualifiers< ArrayType >(); }
};

//------------------------------------------------------------------------------
// ae::MapType
//------------------------------------------------------------------------------
class MapType : public ae::Type
{
public:
	virtual const ae::Type& GetKeyVarType() const = 0;
	virtual const ae::Type& GetValueVarType() const = 0;

	//! Gets a value in the map by key, inserts a new value if the key does not
	//! exist. Will return an empty value if there is a type mismatch with
	//! either parameter or the map is full.
	virtual ae::DataPointer Get( ae::DataPointer map, ae::ConstDataPointer key ) const = 0;
	//! Gets a value in the map by key, returns an empty value if the key does not exist.
	virtual ae::DataPointer TryGet( ae::DataPointer map, ae::ConstDataPointer key ) const = 0;
	//! Gets a value in the map by key, returns an empty value if the key does  not exist.
	virtual ae::ConstDataPointer TryGet( ae::ConstDataPointer map, ae::ConstDataPointer key ) const = 0;

	//! Current number of map elements
	virtual uint32_t GetLength( ae::ConstDataPointer map ) const = 0;
	//! Get the key at the given index. Keys are always const because
	//! modification would require re-indexing. Returns null if the index is out
	//! of bounds.
	virtual ae::ConstDataPointer GetKey( ae::ConstDataPointer map, uint32_t index ) const = 0;
	//! Returns a const pointer to the value at the given index. Returns null if
	//! the index is out of bounds.
	virtual ae::ConstDataPointer GetValue( ae::ConstDataPointer map, uint32_t index ) const = 0;
	//! Returns a pointer to the value at the given index. The value can be
	//! safely modified with this pointer. Returns null if the index is out of
	//! bounds.
	virtual ae::DataPointer GetValue( ae::DataPointer map, uint32_t index ) const = 0;
	
	//! Return uint max for no hard limit
	virtual uint32_t GetMaxLength() const = 0;

	// Internal
	ae::TypeId GetBaseVarTypeId() const override { return ae::GetTypeIdWithoutQualifiers< MapType >(); }
};

//------------------------------------------------------------------------------
// ae::ClassVar class
//! Information about a member variable registered with AE_REGISTER_CLASS_VAR().
//! To set and get variable data construct an ae::DataPointer or
//! ae::ConstDataPointer with an ae::ClassVar.
//------------------------------------------------------------------------------
class ClassVar
{
public:
	//--------------------------------------------------------------------------
	// Basic Info
	//--------------------------------------------------------------------------
	//! The name of this data member in the class type given by
	//! ae::ClassVar::GetClassType().
	const char* GetName() const;
	//! Byte offset of this data member into the class type returned by
	//! ae::ClassVar::GetClassType().
	uint32_t GetOffset() const;
	//! Returns the class type that this member variable belongs too.
	const ae::ClassType& GetClassType() const;
	//! Returns the 'outermost' type of this var, eg. if this is an array of ints
	//! the outer type would be ArrayType
	const ae::Type& GetOuterVarType() const;

	//--------------------------------------------------------------------------
	// ae::AttributeList
	//--------------------------------------------------------------------------
	ae::AttributeList attributes;

	//--------------------------------------------------------------------------
	// Internal
	//--------------------------------------------------------------------------
	struct _TypePointer
	{
		_TypePointer() : type( Null ) {}
		_TypePointer( const ae::Type& varType );
		_TypePointer( const ae::ClassType* type );
		_TypePointer( const ae::TypeId typeId );
		const ae::ClassType* GetClassType() const; // Returns a value for Class only
		const ae::Type* Get() const; // Returns a value for all types except Null
		bool IsValid() const { return type != Null; }
		enum { Static, Class, Null } type; // @TODO: Enum
		union
		{
			const ae::Type* varType;
			ae::TypeId typeId;
			// @TODO: Enum
		};
	};
	_TypePointer m_owner;
	_TypePointer m_varType;
	ae::TypeName m_name;
	uint32_t m_offset = 0;
	// Deprecated
	ae::Map< ae::Str32, ae::Array< ae::Str32, kMaxMetaPropListLength >, kMaxMetaProps > m_props;
	void m_AddProp( const char* prop, const char* value );
	class Serializer
	{
	public:
		virtual ~Serializer();
		virtual std::string ObjectPointerToString( const ae::Object* obj ) const = 0;
		virtual bool StringToObjectPointer( const char* pointerVal, ae::Object** objOut ) const = 0;
	};
	static void SetSerializer( const ae::ClassVar::Serializer* serializer );
	std::string GetObjectValueAsString( const ae::Object* obj, int32_t arrayIdx = -1 ) const;
	bool SetObjectValueFromString( ae::Object* obj, const char* value, int32_t arrayIdx = -1 ) const;
	template< typename T > bool GetObjectValue( const ae::Object* obj, T* valueOut, int32_t arrayIdx = -1 ) const;
	template< typename T > bool SetObjectValue( ae::Object* obj, const T& value, int32_t arrayIdx = -1 ) const;
	template< typename T > T* GetPointer( ae::Object* obj, int32_t arrayIdx = -1 ) const;
	template< typename T > const T* GetPointer( const ae::Object* obj, int32_t arrayIdx = -1 ) const;
	bool HasProperty( const char* prop ) const;
	int32_t GetPropertyIndex( const char* prop ) const;
	int32_t GetPropertyCount() const;
	const char* GetPropertyName( int32_t propIndex ) const;
	uint32_t GetPropertyValueCount( int32_t propIndex ) const;
	uint32_t GetPropertyValueCount( const char* propName ) const;
	const char* GetPropertyValue( int32_t propIndex, uint32_t valueIndex ) const;
	const char* GetPropertyValue( const char* propName, uint32_t valueIndex ) const;
	ae::BasicType::Type GetType() const;
	const class ae::EnumType* GetEnumType() const;
	const char* GetTypeName() const;
	const ae::ClassType* GetSubType() const;
	bool IsArray() const;
	bool IsArrayFixedLength() const;
	uint32_t SetArrayLength( ae::Object* obj, uint32_t length ) const;
	uint32_t GetArrayLength( const ae::Object* obj ) const;
	uint32_t GetArrayMaxLength() const;
	template< typename T >
	const T* m_HACK_FindInnerVarType() const
	{
		static_assert( std::is_base_of_v< ae::Type, T >, "" );
		const ae::Type* iter = m_varType.Get();
		while( iter )
		{
			if( const T* innerType = iter->AsVarType< T >() ) { return innerType; }
			else if( const ae::PointerType* pointerVarType = iter->AsVarType< ae::PointerType >() ) { iter = &pointerVarType->GetInnerVarType(); }
			else if( const ae::ArrayType* arrayVarType = iter->AsVarType< ae::ArrayType >() ) { iter = &arrayVarType->GetInnerVarType(); }
			else { iter = nullptr; }
		}
		return nullptr;
	}
};

//------------------------------------------------------------------------------
// ae::ClassType
//------------------------------------------------------------------------------
class ClassType : public ae::Type
{
public:
	virtual ae::TypeId GetTypeId() const = 0;

	//--------------------------------------------------------------------------
	// C++ type info
	//--------------------------------------------------------------------------
	template< typename T = ae::Object > T* New( void* obj ) const;
	//! Creates a temporary instance of this type and copies the vtable from
	//! the instance. This type must be default constructible.
	void PatchVTable( ae::Object* obj ) const;
	uint32_t GetSize() const;
	uint32_t GetAlignment() const;
	const char* GetName() const;
	bool HasNew() const;
	bool IsAbstract() const;
	bool IsPolymorphic() const;
	bool IsDefaultConstructible() const;
	bool IsFinal() const;

	//--------------------------------------------------------------------------
	// Inheritance
	//--------------------------------------------------------------------------
	const char* GetParentTypeName() const;
	const ae::ClassType* GetParentType() const;
	bool IsType( const ae::ClassType* otherType ) const;
	template< typename T > bool IsType() const;

	//--------------------------------------------------------------------------
	// ae::DataPointer
	//--------------------------------------------------------------------------
	//! Gets the class type of the given DataPointer if possible.
	const ae::ClassType* GetClassType( ae::ConstDataPointer varData ) const;
	template< typename T > T* TryGet( ae::DataPointer varData ) const;
	template< typename T > const T* TryGet( ae::ConstDataPointer varData ) const;
	ae::DataPointer GetVarData( const ae::ClassVar* var, ae::DataPointer varData ) const;
	ae::ConstDataPointer GetVarData( const ae::ClassVar* var, ae::ConstDataPointer varData ) const;

	//--------------------------------------------------------------------------
	// ae::ClassVar
	//--------------------------------------------------------------------------
	uint32_t GetVarCount( bool parents ) const;
	const ae::ClassVar* GetVarByIndex( uint32_t i, bool parents ) const;
	const ae::ClassVar* GetVarByName( const char* name, bool parents ) const;

	//--------------------------------------------------------------------------
	// ae::AttributeList
	//--------------------------------------------------------------------------
	ae::AttributeList attributes;

	//------------------------------------------------------------------------------
	// Internal
	//------------------------------------------------------------------------------
private:
	ae::Object* ( *m_placementNew )( ae::Object* ) = nullptr;
	ae::TypeName m_name;
	ae::TypeId m_id = ae::kInvalidTypeId;
	uint32_t m_size = 0;
	uint32_t m_align = 0;
	ae::Map< ae::Str32, ae::Array< ae::Str32, kMaxMetaPropListLength >, kMaxMetaProps > m_props;
	ae::Array< const ae::ClassVar*, kMaxMetaVars > m_vars;
	ae::TypeName m_parent;
	bool m_isAbstract = false;
	bool m_isPolymorphic = false;
	bool m_isDefaultConstructible = false;
	bool m_isFinal = false;
public:
	template< typename T > typename std::enable_if< !std::is_abstract< T >::value && std::is_default_constructible< T >::value, void >::type Init( const char* name );
	template< typename T > typename std::enable_if< std::is_abstract< T >::value || !std::is_default_constructible< T >::value, void >::type Init( const char* name );
	void m_AddProp( const char* prop, const char* value );
	void m_AddVar( const ae::ClassVar* var );
	ae::TypeId GetBaseVarTypeId() const override { return ae::GetTypeIdWithoutQualifiers< ClassType >(); }
	// Deprecated
	ae::TypeId GetId() const;
	const ae::ClassType* GetClassType() const { return this; }
	bool HasProperty( const char* property ) const;
	const ae::ClassType* GetTypeWithProperty( const char* property ) const;
	int32_t GetPropertyIndex( const char* prop ) const;
	int32_t GetPropertyCount() const;
	const char* GetPropertyName( int32_t propIndex ) const;
	uint32_t GetPropertyValueCount( int32_t propIndex ) const;
	uint32_t GetPropertyValueCount( const char* propName ) const;
	const char* GetPropertyValue( int32_t propIndex, uint32_t valueIndex ) const;
	const char* GetPropertyValue( const char* propName, uint32_t valueIndex ) const;
};

//------------------------------------------------------------------------------
// ae::Cast
//------------------------------------------------------------------------------
template< typename T, typename C > const T* Cast( const C* obj );
template< typename T, typename C > T* Cast( C* obj );

//------------------------------------------------------------------------------
// ae::PatchVTable
//! Overwrites the v-table of the given \p obj with the v-table of the given
//! type. Use this over ae::ClassType::PatchVTable() when the type of \p obj
//! is known at compile time. T Must be the bottom-most class in the given
//! \p obj inheritance hierarchy. A temporary instance of the object will be
//! constructed. \p ctorArgs may be provided if the type does not have a default
//! constructor. 
//------------------------------------------------------------------------------
template< typename T, typename... Args >
void PatchVTable( T* obj, Args... ctorArgs )
{
	T temp = T( ctorArgs... );
	void* vtable = *(void**)&temp;
	memcpy( (void*)obj, &vtable, sizeof(void*) );
}

//! @}

//------------------------------------------------------------------------------
//
//
//
//
//
//
//
//
//
//
//
//
// Internal implementation beyond this point
//
//
//
//
//
//
//
//
//
//
//
//
//------------------------------------------------------------------------------
// Internal ae::_DefaultAllocator
//------------------------------------------------------------------------------
class _DefaultAllocator : public Allocator
{
public:
	~_DefaultAllocator() override;
	void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) override;
	void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) override;
	void Free( void* data ) override;
	bool IsThreadSafe() const override;
#if AE_MEMORY_CHECKS
private:
	enum class AllocStatus : uint8_t { Allocated, Freed };
	struct AllocInfo
	{
		AllocInfo() = default;
		AllocInfo( ae::Tag tag, uint32_t bytes, AllocStatus status ) : tag( tag ), bytes( bytes ), status( status ) {}
		ae::Tag tag;
		uint32_t bytes;
		AllocStatus status;
	};
	std::mutex m_allocLock;
	std::unordered_map< void*, AllocInfo > m_allocations;
#endif
};

//------------------------------------------------------------------------------
// Internal ae::_ScratchBuffer storage
//------------------------------------------------------------------------------
class _ScratchBuffer
{
public:
	_ScratchBuffer( uint32_t size );
	~_ScratchBuffer();
	static uint32_t GetScratchBytes( uint32_t bytes );

#if _AE_EMSCRIPTEN_
	static const uint32_t kScratchAlignment = 8; // Emscripten only supports up to 8 byte alignment
#else
	static const uint32_t kScratchAlignment = 16;
#endif
	uint8_t* data = nullptr;
	uint32_t offset = 0;
	uint32_t size = 0;
};

//------------------------------------------------------------------------------
// Internal ae::_Globals
//------------------------------------------------------------------------------
struct _Globals
{
	static _Globals* Get();
	~_Globals();
	
	// Allocation
	bool allocatorInitialized = false;
	Allocator* allocator = nullptr;
	bool allocatorIsThreadSafe = false;
	std::thread::id allocatorThread;
	_DefaultAllocator defaultAllocator;

	// Scratch
	_ScratchBuffer scratchBuffer = ae::Scratch< uint8_t >::kMaxScratchSize;

	// Reflection
	uint32_t metaCacheSeq = 0;
	ae::Map< ae::TypeId, const ae::EnumType*, kMaxMetaEnumTypes, ae::Hash32, ae::MapMode::Stable > enumTypes;
	ae::Map< ae::TypeId, ae::ClassType*, kMaxMetaTypes, ae::Hash32, ae::MapMode::Stable > classTypes;
	const ae::ClassVar::Serializer* varSerializer = nullptr;
	bool varSerializerInitialized = false;

	// Graphics
	class GraphicsDevice* graphicsDevice = nullptr;
};

//------------------------------------------------------------------------------
// Internal ae::_ThreadLocals
//------------------------------------------------------------------------------
struct _ThreadLocals
{
	static _ThreadLocals* Get();

	ae::Array< ae::Str64, 8 > logTagStack;
};

//------------------------------------------------------------------------------
// Internal ae::_StaticCacheVar
// This is used to cache static meta variables that are expensive to
// find/compute. All cached variables are reset to their initial value when
// the meta system is changed.
//------------------------------------------------------------------------------
template< typename T >
class _StaticCacheVar
{
public:
	_StaticCacheVar( T defaultValue ) :
		m_value( defaultValue ),
		m_defaultValue( defaultValue ),
		m_seq( _Globals::Get()->metaCacheSeq )
	{}
	operator T& ()
	{
		if( m_seq != _Globals::Get()->metaCacheSeq )
		{
			m_seq = _Globals::Get()->metaCacheSeq;
			m_value = m_defaultValue;
		}
		return m_value;
	}
private:
	T m_value;
	T m_defaultValue;
	uint32_t m_seq;
};

//------------------------------------------------------------------------------
// GetTypeName() internal implementation
// https://stackoverflow.com/a/59522794
//------------------------------------------------------------------------------
template< typename T >
constexpr const auto& _RawTypeName()
{
#ifdef _MSC_VER
	return __FUNCSIG__;
#else
	return __PRETTY_FUNCTION__;
#endif
}

struct _RawTypeNameFormat
{
	std::size_t leadingJunk = 0;
	std::size_t trailingJunk = 0;
};

inline constexpr bool _GetRawTypeNameFormat( ae::_RawTypeNameFormat* format )
{
	const auto& str = ae::_RawTypeName< int >();
	for( std::size_t i = 0; str[ i ]; i++ )
	{
		if( str[ i ] == 'i' && str[ i + 1 ] == 'n' && str[ i + 2 ] == 't' )
		{
			if( format )
			{
				format->leadingJunk = i;
				format->trailingJunk = sizeof( str ) - i - sizeof( "int" );
			}
			return true;
		}
	}
	return false;
}

inline static constexpr _RawTypeNameFormat _rawTypeNameFormat = []
{
	static_assert( ae::_GetRawTypeNameFormat( nullptr ), "Unable to figure out how to generate type names on this compiler." );
	ae::_RawTypeNameFormat format;
	ae::_GetRawTypeNameFormat( &format );
	return format;
}();

template< typename T >
constexpr auto _GetTypeName() // @TODO: Return ae::Str
{
	constexpr std::size_t len = sizeof( ae::_RawTypeName< T >() ) - ae::_rawTypeNameFormat.leadingJunk - ae::_rawTypeNameFormat.trailingJunk;
	std::array< char, len > name{}; // @TODO: Compile time ae::Array
	for( std::size_t i = 0; i < len - 1; i++ )
	{
		name[ i ] = ae::_RawTypeName< T >()[ i + ae::_rawTypeNameFormat.leadingJunk ];
	}
	return name;
}

template< typename T >
const char* GetTypeName()
{
	static constexpr auto name = ae::_GetTypeName< T >();
	return name.data();
}

template< typename T >
const char* GetTypeName( const T& )
{
	return ae::GetTypeName< T >();
}

//------------------------------------------------------------------------------
// Log levels internal implementation
//------------------------------------------------------------------------------
extern const char* LogLevelNames[ 6 ]; // Indexed by ae::LogSeverity
extern const char* LogLevelColors[ 6 ]; // Indexed by ae::LogSeverity
// The following functions are used to internally format log messages before submitting them to AE_LOG_FUNCTION_CONFIG()
template< typename... Args > std::string _Log( ae::LogSeverity severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format, Args... args );
template< typename T, typename... Args > void _BuildLogMessage( std::stringstream& os, const char* format, T value, Args... args );
static void _BuildLogMessage( std::stringstream& os, const char* message ) { os << message; } // Recursive base case for _BuildLogMessage()
void _LogImpl( ae::LogSeverity severity, const char* filePath, uint32_t line, const char** tags, uint32_t tagCount, const char* message );

//------------------------------------------------------------------------------
// Logging functions implementation
//------------------------------------------------------------------------------
static void _ApplyLogStreamSettings( std::ostream& os )
{
	os << std::setprecision( 4 );
	os << std::boolalpha;
}

template< typename... Args >
void PushLogTag( const char* format, Args... args )
{
	ae::_ThreadLocals* threadLocals = ae::_ThreadLocals::Get();
	const bool canAppendMessage = ( threadLocals->logTagStack.Length() < threadLocals->logTagStack.Size() );
	AE_DEBUG_ASSERT( canAppendMessage );
	if( canAppendMessage )
	{
		std::stringstream os;
		_ApplyLogStreamSettings( os );
		_BuildLogMessage( os, format, args... );
		threadLocals->logTagStack.Append( os.str().c_str() );
	}
}

template< typename T, typename... Args >
void _BuildLogMessage( std::stringstream& os, const char* format, T value, Args... args )
{
	const char* head = format;
	while( *head && *head != '#' )
	{
		head++;
	}
	if( head > format )
	{
		os.write( format, head - format );
	}

	if( *head == '#' )
	{
		os << value;
		head++;
	}

	if( *head )
	{
		_BuildLogMessage( os, head, args... );
	}
	else
	{
		_BuildLogMessage( os, "" );
	}
}

template< typename... Args >
std::string _Log( ae::LogSeverity severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format, Args... args )
{
	ae::_ThreadLocals* threadLocals = ae::_ThreadLocals::Get();
	std::stringstream os;
	_ApplyLogStreamSettings( os );
	if( assertInfo[ 0 ] )
	{
		os << assertInfo << " ";
	}
	_BuildLogMessage( os, format, args... );
	const char* tags[ decltype(threadLocals->logTagStack)::Size() ];
	for( uint32_t i = 0; i < threadLocals->logTagStack.Length(); i++ )
	{
		tags[ i ] = threadLocals->logTagStack[ i ].c_str();
	}
	( AE_LOG_FUNCTION_CONFIG )
	(
		severity,
		filePath, line,
		tags, threadLocals->logTagStack.Length(),
		os.str().c_str()
	);
	if( severity == ae::LogSeverity::Fatal )
	{
		std::stringstream ss;
		ss << os.rdbuf();
		return ss.str();
	}
	return "";
}

//------------------------------------------------------------------------------
// C++ style allocation functions
//------------------------------------------------------------------------------
#if _AE_EMSCRIPTEN_
// @NOTE: Max alignment is 8 bytes https://github.com/emscripten-core/emscripten/issues/10072
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

template< typename T, typename ... Args >
T* NewArray( ae::Tag tag, uint32_t count, Args&& ... args )
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
	header->check = 0xBDBD;
	header->count = count;
	header->size = size;
	header->typeSize = sizeof( T );

	T* result = (T*)( base + _kHeaderSize );
	if( !std::is_trivially_constructible< T >::value )
	{
		for( uint32_t i = 0; i < count; i++ )
		{
			new( &result[ i ] ) T( args ... ); // Can't std::forward args multiple times
		}
	}
	
	return result;
}

template< typename T, typename ... Args >
T* New( ae::Tag tag, Args&& ... args )
{
	AE_STATIC_ASSERT( alignof( T ) <= _kDefaultAlignment );

	uint32_t size = _kHeaderSize + sizeof( T );
	uint8_t* base = (uint8_t*)ae::Allocate( tag, size, _kDefaultAlignment );
	AE_ASSERT_MSG( (intptr_t)base % _kDefaultAlignment == 0, "Alignment off by # bytes", (intptr_t)base % _kDefaultAlignment );
#if _AE_DEBUG_
	memset( (void*)base, 0xCD, size );
#endif

	_Header* header = (_Header*)base;
	header->check = 0xBDBD;
	header->count = 1;
	header->size = size;
	header->typeSize = sizeof( T );

	return new( (T*)( base + _kHeaderSize ) ) T( std::forward< Args >( args ) ... );
}

template< typename T >
void Delete( T* obj )
{
	if( !obj )
	{
		return;
	}

	AE_ASSERT( (intptr_t)obj % _kDefaultAlignment == 0 );
	uint8_t* base = (uint8_t*)obj - _kHeaderSize;

	_Header* header = (_Header*)( base );
	AE_ASSERT( header->check == 0xBDBD );
	AE_ASSERT_MSG( sizeof( T ) <= header->typeSize, "Released type T '#' does not match allocated type of size #", ae::GetTypeName< T >(), header->typeSize );

	if( !std::is_trivially_destructible< T >::value )
	{
		uint32_t count = header->count;
		for( uint32_t i = 0; i < count; i++ )
		{
			T* o = (T*)( (uint8_t*)obj + header->typeSize * i );
			o->~T();
		}
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
#if _AE_DEBUG_
	AE_ASSERT_MSG( tag != ae::Tag(), "Allocation of # bytes and alignment # is not tagged", bytes, alignment );
	AE_ASSERT_MSG( alignment, "Allocation '#' has invalid 0 byte alignment", tag );
#endif
	void* result = ae::GetGlobalAllocator()->Allocate( tag, bytes, alignment );
#if _AE_DEBUG_
	AE_ASSERT_MSG( result, "Failed to allocate # bytes with alignment # (#)", bytes, alignment, tag );
	intptr_t alignmentOffset = (intptr_t)result % alignment;
	AE_ASSERT_MSG( alignmentOffset == 0, "Allocation of # bytes (#) with alignment # off by # bytes", bytes, tag, alignment, alignmentOffset );
#endif
	return result;
}

inline void* Reallocate( void* data, uint32_t bytes, uint32_t alignment )
{
	return ae::GetGlobalAllocator()->Reallocate( data, bytes, alignment );
}

inline void Free( void* data )
{
	if( data )
	{
		ae::GetGlobalAllocator()->Free( data );
	}
}

//------------------------------------------------------------------------------
// ae::Scratch< T > member functions
//------------------------------------------------------------------------------
template< typename T >
Scratch< T >::Scratch( uint32_t count )
{
	AE_STATIC_ASSERT( alignof(T) <= _ScratchBuffer::kScratchAlignment );
	ae::_ScratchBuffer* globalScratch = &ae::_Globals::Get()->scratchBuffer;
	const uint32_t bytes = globalScratch->GetScratchBytes( count * sizeof(T) );
	
	m_size = count;
	m_data = (T*)( globalScratch->data + globalScratch->offset );
	AE_DEBUG_ASSERT( ( (intptr_t)m_data % ae::_ScratchBuffer::kScratchAlignment ) == 0 );
	m_prevOffsetCheck = globalScratch->offset;
	globalScratch->offset += bytes;
	AE_ASSERT_MSG( globalScratch->offset <= globalScratch->size, "Global scratch buffer size exceeded: # bytes / (# bytes). The global max can be set with AE_MAX_SCRATCH_BYTES_CONFIG.", globalScratch->offset, globalScratch->size );
	
#if _AE_DEBUG_
	memset( m_data, 0xCD, m_size * sizeof(T) );
	// Guard
	uint8_t* guard = (uint8_t*)m_data + m_size * sizeof(T);
	const intptr_t guardLength = ( (uint8_t*)m_data + bytes ) - guard;
	for( uint32_t i = 0; i < guardLength; i++ ) { guard[ i ] = 0xBD; }
#endif
	if( !std::is_trivially_constructible< T >::value )
	{
		for( uint32_t i = 0; i < m_size; i++ )
		{
			new ( &m_data[ i ] ) T();
		}
	}
}

template< typename T >
Scratch< T >::~Scratch()
{
	ae::_ScratchBuffer* scratchBuffer = &ae::_Globals::Get()->scratchBuffer;
	
	const uint32_t bytes = scratchBuffer->GetScratchBytes( m_size * sizeof(T) );
#if _AE_DEBUG_
	// Guard
	const uint8_t* guard = (uint8_t*)m_data + m_size * sizeof(T);
	const intptr_t guardLength = ( (uint8_t*)m_data + bytes ) - guard;
	for( uint32_t i = 0; i < guardLength; i++ ) { AE_ASSERT_MSG( guard[ i ] == 0xBD, "Scratch buffer guard has been overwritten" ); }
#endif
	AE_ASSERT( scratchBuffer->offset >= bytes );
	
	if( !std::is_trivially_constructible< T >::value )
	{
		for( int32_t i = m_size - 1; i >= 0; i-- )
		{
			m_data[ i ].~T();
		}
	}
	m_data = nullptr;
	scratchBuffer->offset -= bytes;
	AE_ASSERT_MSG( scratchBuffer->offset == m_prevOffsetCheck, "ae::Scratch destroyed out of order" );
}

template< typename T >
T* Scratch< T >::Data()
{
	return m_data;
}

template< typename T >
uint32_t Scratch< T >::Length() const
{
	return m_size;
}

template< typename T >
T& Scratch< T >::operator[] ( int32_t index )
{
	return m_data[ index ];
}

template< typename T >
const T& Scratch< T >::operator[] ( int32_t index ) const
{
	return m_data[ index ];
}

template< typename T >
T& Scratch< T >::GetSafe( int32_t index )
{
	AE_ASSERT( index < (int32_t)m_size );
	return m_data[ index ];
}

template< typename T >
const T& Scratch< T >::GetSafe( int32_t index ) const
{
	AE_ASSERT( index < (int32_t)m_size );
	return m_data[ index ];
}

//------------------------------------------------------------------------------
// Math function implementations
//------------------------------------------------------------------------------
inline float Abs( float x ) { return ( x < 0.0f ) ? -x : x; }
inline int32_t Abs( int32_t x ) { return ( x < 0 ) ? -x : x; }
template< typename T >
inline T Abs( const VecT< T >& x )
{
	T result;
	for( uint32_t i = 0; i < countof(T::data); i++ )
	{
		result[ i ] = ae::Abs( x[ i ] );
	}
	return result;
}

// https://stackoverflow.com/a/63330289/2423134
template< typename T0, typename T1, typename... TTT >
constexpr auto Min( const T0& v0, const T1& v1, const TTT&... tail )
{
	if constexpr( sizeof...(tail) == 0 ) { return v0 < v1 ? v0 : v1; }
	else { return Min( Min( v0, v1 ), tail... ); }
}

template< typename T0, typename T1, typename... TTT >
constexpr auto Max( const T0& v0, const T1& v1, const TTT&... tail )
{
	if constexpr( sizeof...(tail) == 0 ) { return v0 > v1 ? v0 : v1; }
	else { return Max( Max( v0, v1 ), tail... ); }
}

inline ae::Vec2 Min( ae::Vec2 v0, ae::Vec2 v1 )
{
	return ae::Vec2( Min( v0.x, v1.x ), Min( v0.y, v1.y ) );
}

inline ae::Vec3 Min( ae::Vec3 v0, ae::Vec3 v1 )
{
	return ae::Vec3( Min( v0.x, v1.x ), Min( v0.y, v1.y ), Min( v0.z, v1.z ) );
}

inline ae::Vec4 Min( ae::Vec4 v0, ae::Vec4 v1 )
{
	return ae::Vec4( Min( v0.x, v1.x ), Min( v0.y, v1.y ), Min( v0.z, v1.z ), Min( v0.w, v1.w ) );
}

inline ae::Vec2 Max( ae::Vec2 v0, ae::Vec2 v1 )
{
	return ae::Vec2( Max( v0.x, v1.x ), Max( v0.y, v1.y ) );
}

inline ae::Vec3 Max( ae::Vec3 v0, ae::Vec3 v1 )
{
	return ae::Vec3( Max( v0.x, v1.x ), Max( v0.y, v1.y ), Max( v0.z, v1.z ) );
}

inline ae::Vec4 Max( ae::Vec4 v0, ae::Vec4 v1 )
{
	return ae::Vec4( Max( v0.x, v1.x ), Max( v0.y, v1.y ), Max( v0.z, v1.z ), Max( v0.w, v1.w ) );
}

inline ae::Int2 Min( ae::Int2 v0, ae::Int2 v1 )
{
	return ae::Int2( Min( v0.x, v1.x ), Min( v0.y, v1.y ) );
}

inline ae::Int3 Min( ae::Int3 v0, ae::Int3 v1 )
{
	return ae::Int3( Min( v0.x, v1.x ), Min( v0.y, v1.y ), Min( v0.z, v1.z ) );
}

inline ae::Int2 Max( ae::Int2 v0, ae::Int2 v1 )
{
	return ae::Int2( Max( v0.x, v1.x ), Max( v0.y, v1.y ) );
}

inline ae::Int3 Max( ae::Int3 v0, ae::Int3 v1 )
{
	return ae::Int3( Max( v0.x, v1.x ), Max( v0.y, v1.y ), Max( v0.z, v1.z ) );
}

template< typename T >
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

inline float Tan( float x )
{
	return tanf( x );
}

inline float Acos( float x )
{
	return acosf( x );
}

inline float Asin( float x )
{
	return asinf( x );
}

inline float Atan( float x )
{
	return atanf( x );
}

inline float Atan2( float y, float x )
{
	return atan2( y, x );
}

inline float Sqrt( float x )
{
	return sqrtf( x );
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

constexpr uint32_t NextPowerOfTwo( uint32_t x )
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

template< typename T0, typename T1 >
T0 Lerp( T0 start, T0 end, T1 t )
{
	return start + ( end - start ) * t;
}

inline float AngleDifference( float end, float start )
{
	return ae::Mod( ( end - start ) + ae::PI, ae::TWO_PI ) - ae::PI;
}

inline float LerpAngle( float start, float end, float t )
{
	return ae::Lerp( start, start + AngleDifference( end, start ), t );
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
T DtLerp( T start, float snappiness, float dt, T end )
{
	if( snappiness == 0.0f || dt == 0.0f )
	{
		return start;
	}
	return ae::Lerp( end, start, exp2( -exp2( snappiness ) * dt ) );
}

template< typename T >
T DtSlerp( T start, float snappiness, float dt, T end )
{
	return start.DtSlerp( end, snappiness, dt );
}

inline float DtLerpAngle( float start, float snappiness, float dt, float end )
{
	return ae::DtLerp( start, snappiness, dt, start + ae::AngleDifference( end, start ) );
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

//------------------------------------------------------------------------------
// ae::Random functions
//------------------------------------------------------------------------------
inline uint32_t _Random( uint64_t* seed )
{
	// splitmix https://arvid.io/2018/07/02/better-cxx-prng/
	uint64_t z = ( *seed += UINT64_C( 0x9E3779B97F4A7C15 ) );
	z = ( z ^ ( z >> 30 ) ) * UINT64_C( 0xBF58476D1CE4E5B9 );
	z = ( z ^ ( z >> 27 ) ) * UINT64_C( 0x94D049BB133111EB );
	return uint32_t( ( z ^ ( z >> 31 ) ) >> 31 );
}

inline float Random01( uint64_t* seed )
{
	return ae::_Random( seed ) / (float)ae::MaxValue< uint32_t >();
}

inline bool RandomBool( uint64_t* seed )
{
	return Random( 0, 2, seed );
}

inline int32_t Random( int32_t minInclusive, int32_t maxExclusive, uint64_t* seed )
{
	// Before check so random call count is not based on params
	const uint32_t r = ae::_Random( seed );
	if( minInclusive >= maxExclusive )
	{
		return minInclusive;
	}
	return minInclusive + ( r % ( maxExclusive - minInclusive ) );
}

inline float Random( float min, float max, uint64_t* seed )
{
	// Before check so random call count is not based on params
	const uint32_t r = ae::_Random( seed );
	if( min >= max )
	{
		return min;
	}
	return min + ( ( r / (float)ae::MaxValue< uint32_t >() ) * ( max - min ) );
}

//------------------------------------------------------------------------------
// RandomValue member functions
//------------------------------------------------------------------------------
template< typename T >
inline ae::RandomValue< T >::RandomValue( T min, T max, uint64_t* seed ) : m_seed( seed ), m_min(min), m_max(max) {}

template< typename T >
inline ae::RandomValue< T >::RandomValue( T value, uint64_t* seed ) : m_seed( seed ), m_min(value), m_max(value) {}

template< typename T >
inline void ae::RandomValue< T >::SetMin( T min )
{
	m_min = min;
}

template< typename T >
inline void ae::RandomValue< T >::SetMax( T max )
{
	m_max = max;
}

template< typename T >
inline T ae::RandomValue< T >::GetMin() const
{
	return m_min;
}

template< typename T >
inline T ae::RandomValue< T >::GetMax() const
{
	return m_max;
}

template< typename T >
inline T ae::RandomValue< T >::Get() const
{
	return Random( m_min, m_max, m_seed );
}

template< typename T >
inline ae::RandomValue< T >::operator T() const
{
	return Get();
}

//------------------------------------------------------------------------------
// ae::Vec2 shared member functions
// ae::Vec3 shared member functions
// ae::Vec4 shared member functions
//------------------------------------------------------------------------------
template< typename T >
bool VecT< T >::operator==( const T& v ) const
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(T::data); i++ )
	{
		if( self.data[ i ] != v.data[ i ] )
		{
			return false;
		}
	}
	return true;
}

template< typename T >
bool VecT< T >::operator!=( const T& v ) const
{
	return !operator ==( v );
}

template< typename T >
float VecT< T >::operator[]( uint32_t idx ) const
{
	auto&& self = *(T*)this;
#if _AE_DEBUG_
	AE_ASSERT( idx < countof(self.data) );
#endif
	return self.data[ idx ];
}

template< typename T >
float& VecT< T >::operator[]( uint32_t idx )
{
	auto&& self = *(T*)this;
#if _AE_DEBUG_
	AE_ASSERT( idx < countof(self.data) );
#endif
	return self.data[ idx ];
}

template< typename T >
T VecT< T >::operator+( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(T::data); i++ )
	{
		result.data[ i ] = self.data[ i ] + v.data[ i ];
	}
	return result;
}

template< typename T >
void VecT< T >::operator+=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(T::data); i++ )
	{
		self.data[ i ] += v.data[ i ];
	}
}

template< typename T >
T VecT< T >::operator-() const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = -self.data[ i ];
	}
	return result;
}

template< typename T >
T VecT< T >::operator-( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] - v.data[ i ];
	}
	return result;
}

template< typename T >
void VecT< T >::operator-=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] -= v.data[ i ];
	}
}

template< typename T >
T VecT< T >::operator*( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] * v.data[ i ];
	}
	return result;
}

template< typename T >
T VecT< T >::operator/( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] / v.data[ i ];
	}
	return result;
}

template< typename T >
void VecT< T >::operator*=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] *= v.data[ i ];
	}
}

template< typename T >
void VecT< T >::operator/=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] /= v.data[ i ];
	}
}

template< typename T >
T VecT< T >::operator*( float s ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] * s;
	}
	return result;
}

template< typename T >
void VecT< T >::operator*=( float s )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] *= s;
	}
}

template< typename T >
T VecT< T >::operator/( float s ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] / s;
	}
	return result;
}

template< typename T >
void VecT< T >::operator/=( float s )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] /= s;
	}
}

template< typename T >
float VecT< T >::Dot( const T& v0, const T& v1 )
{
	float result = 0.0f;
	for( uint32_t i = 0; i < countof(v0.data); i++ )
	{
		result += v0.data[ i ] * v1.data[ i ];
	}
	return result;
}

template< typename T >
float VecT< T >::Dot( const T& v ) const
{
	return Dot( *(T*)this, v );
}

template< typename T >
float VecT< T >::Length() const
{
	return sqrt( LengthSquared() );
}

template< typename T >
float VecT< T >::LengthSquared() const
{
	return Dot( *(T*)this );
}

template< typename T >
float VecT< T >::Normalize()
{
	float length = Length();
	*(T*)this /= length;
	return length;
}

template< typename T >
float VecT< T >::SafeNormalize( float epsilon )
{
	auto&& self = *(T*)this;
	float length = Length();
	if( length < epsilon )
	{
		self = T( 0.0f );
		return 0.0f;
	}
	self /= length;
	return length;
}

template< typename T >
T VecT< T >::NormalizeCopy() const
{
	T result = *(T*)this;
	result.Normalize();
	return result;
}

template< typename T >
T VecT< T >::SafeNormalizeCopy( float epsilon ) const
{
	T result = *(T*)this;
	result.SafeNormalize( epsilon );
	return result;
}

template< typename T >
float VecT< T >::Trim( float trimLength )
{
	const float length = Length();
	if( length > trimLength )
	{
		*(T*)this *= ( trimLength / length );
		return trimLength;
	}
	return length;
}

template< typename T >
T VecT< T >::TrimCopy( float trimLength ) const
{
	const float length = Length();
	if( length > trimLength )
	{
		return *((T*)this) * ( trimLength / length );
	}
	return *((T*)this);
}

template< typename T >
bool VecT< T >::IsNAN() const
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		if( std::isnan( self.data[ i ] ) )
		{
			return true;
		}
	}
	return false;

}

template< typename T >
T operator*( float f, const VecT< T >& v )
{
	return v * f;
}

template< typename T >
inline std::ostream& operator<<( std::ostream& os, const VecT< T >& v )
{
	constexpr uint32_t count = countof( T::data );
	for( uint32_t i = 0; i < count - 1; i++ )
	{
		os << v[ i ] << " ";
	}
	return os << v[ count - 1 ];
}

#if _AE_WINDOWS_
	#pragma warning(disable:26495) // Hide incorrect Vec2 initialization warning due to union
#endif

//------------------------------------------------------------------------------
// ae::Vec2 member functions
//------------------------------------------------------------------------------
inline Vec2::Vec2( float v ) : x( v ), y( v ) {}
inline Vec2::Vec2( float x, float y ) : x( x ), y( y ) {}
inline Vec2::Vec2( const float* xy ) : x( xy[ 0 ] ), y( xy[ 1 ] ) {}
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
	if( LengthSquared() < 0.0001f )
	{
		return 0.0f;
	}
	return ae::Atan2( y, x );
}
inline Vec2 Vec2::Reflect( Vec2 v, Vec2 n )
{
	return n * ( 2.0f * v.Dot( n ) / n.LengthSquared() ) - v;
}
inline Vec2 Vec2::DtSlerp( const Vec2& end, float snappiness, float dt, float epsilon ) const
{
	if( snappiness == 0.0f || dt == 0.0f )
	{
		return *this;
	}
	return Slerp( end, 1.0f - exp2( -exp2( snappiness ) * dt ), epsilon );
}

//------------------------------------------------------------------------------
// ae::Vec3 member functions
//------------------------------------------------------------------------------
inline Vec3::Vec3( float v ) : x( v ), y( v ), z( v ), pad( 0.0f ) {}
inline Vec3::Vec3( float x, float y, float z ) : x( x ), y( y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( const float* xyz ) : x( xyz[ 0 ] ), y( xyz[ 1 ] ), z( xyz[ 2 ] ), pad( 0.0f ) {}
inline Vec3::Vec3( struct Int3 i3 ) : x( (float)i3.x ), y( (float)i3.y ), z( (float)i3.z ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy, float z ) : x( xy.x ), y( xy.y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy ) : x( xy.x ), y( xy.y ), z( 0.0f ), pad( 0.0f ) {}
inline Vec3::operator Vec2() const { return Vec2( x, y ); }
inline Vec3 Vec3::XZY( Vec2 xz, float y ) { return Vec3( xz.x, y, xz.y ); }
inline void Vec3::SetXY( Vec2 xy ) { x = xy.x; y = xy.y; }
inline void Vec3::SetXZ( Vec2 xz ) { x = xz.x; z = xz.y; }
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
	if( d > 0.0f )
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
inline Vec4::Vec4( const float* xyz, float w ) : x( xyz[ 0 ] ), y( xyz[ 1 ] ), z( xyz[ 2 ] ), w( w ) {}
inline Vec4::Vec4( const float* xyzw ) : x( xyzw[ 0 ] ), y( xyzw[ 1 ] ), z( xyzw[ 2 ] ), w( xyzw[ 3 ] ) {}
inline void Vec4::SetXY( Vec2 xy ) { x = xy.x; y = xy.y; }
inline void Vec4::SetXZ( Vec2 xz ) { x = xz.x; z = xz.y; }
inline void Vec4::SetZW( Vec2 zw ) { z = zw.x; w = zw.y; }
inline void Vec4::SetXYZ( Vec3 xyz ) { x = xyz.x; y = xyz.y; z = xyz.z; }
inline Vec2 Vec4::GetXY() const { return Vec2( x, y ); }
inline Vec2 Vec4::GetXZ() const { return Vec2( x, z ); }
inline Vec2 Vec4::GetZW() const { return Vec2( z, w ); }
inline Vec3 Vec4::GetXYZ() const { return Vec3( x, y, z ); }

//------------------------------------------------------------------------------
// ae::Quaternion member functions
//------------------------------------------------------------------------------
inline std::ostream& operator << ( std::ostream& os, const Quaternion& quat )
{
	os << quat.i << " " << quat.j << " " << quat.k << " " << quat.r;
	return os;
}

//------------------------------------------------------------------------------
// ae::Matrix4 member functions
//------------------------------------------------------------------------------
inline std::ostream& operator << ( std::ostream& os, const Matrix4& mat )
{
	os << mat.data[ 0 ] << " " << mat.data[ 1 ] << " " << mat.data[ 2 ] << " " << mat.data[ 3 ]
		<< " " << mat.data[ 4 ] << " " << mat.data[ 5 ] << " " << mat.data[ 6 ] << " " << mat.data[ 7 ]
		<< " " << mat.data[ 8 ] << " " << mat.data[ 9 ] << " " << mat.data[ 10 ] << " " << mat.data[ 11 ]
		<< " " << mat.data[ 12 ] << " " << mat.data[ 13 ] << " " << mat.data[ 14 ] << " " << mat.data[ 15 ];
	return os;
}

//------------------------------------------------------------------------------
// ae::Int2 shared member functions
// ae::Int3 shared member functions
//------------------------------------------------------------------------------
template< typename T >
bool IntT< T >::operator==( const T& v ) const
{
	auto&& self = *(T*)this;
	return memcmp( self.data, v.data, sizeof(T::data) ) == 0;
}

template< typename T >
bool IntT< T >::operator!=( const T& v ) const
{
	auto&& self = *(T*)this;
	return memcmp( self.data, v.data, sizeof(T::data) ) != 0;
}

template< typename T >
int32_t IntT< T >::operator[]( uint32_t idx ) const
{
	auto&& self = *(T*)this;
#if _AE_DEBUG_
	AE_ASSERT( idx < countof(self.data) );
#endif
	return self.data[ idx ];
}

template< typename T >
int32_t& IntT< T >::operator[]( uint32_t idx )
{
	auto&& self = *(T*)this;
#if _AE_DEBUG_
	AE_ASSERT( idx < countof(self.data) );
#endif
	return self.data[ idx ];
}

template< typename T >
T IntT< T >::operator+( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(T::data); i++ )
	{
		result.data[ i ] = self.data[ i ] + v.data[ i ];
	}
	return result;
}

template< typename T >
void IntT< T >::operator+=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(T::data); i++ )
	{
		self.data[ i ] += v.data[ i ];
	}
}

template< typename T >
T IntT< T >::operator-() const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = -self.data[ i ];
	}
	return result;
}

template< typename T >
T IntT< T >::operator-( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] - v.data[ i ];
	}
	return result;
}

template< typename T >
void IntT< T >::operator-=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] -= v.data[ i ];
	}
}

template< typename T >
T IntT< T >::operator*( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] * v.data[ i ];
	}
	return result;
}

template< typename T >
T IntT< T >::operator/( const T& v ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] / v.data[ i ];
	}
	return result;
}

template< typename T >
void IntT< T >::operator*=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] *= v.data[ i ];
	}
}

template< typename T >
void IntT< T >::operator/=( const T& v )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] /= v.data[ i ];
	}
}

template< typename T >
T IntT< T >::operator*( int32_t s ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] * s;
	}
	return result;
}

template< typename T >
void IntT< T >::operator*=( int32_t s )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] *= s;
	}
}

template< typename T >
T IntT< T >::operator/( int32_t s ) const
{
	auto&& self = *(T*)this;
	T result;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		result.data[ i ] = self.data[ i ] / s;
	}
	return result;
}

template< typename T >
void IntT< T >::operator/=( int32_t s )
{
	auto&& self = *(T*)this;
	for( uint32_t i = 0; i < countof(self.data); i++ )
	{
		self.data[ i ] /= s;
	}
}

template< typename T >
inline std::ostream& operator<<( std::ostream& os, const IntT< T >& v )
{
	constexpr uint32_t count = countof( T::data );
	for( uint32_t i = 0; i < count - 1; i++ )
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
inline Int2 Int3::GetXZ() const { return Int2( x, z ); }
inline void Int3::SetXY( Int2 xy ) { x = xy.x; y = xy.y; }
inline void Int3::SetXZ( Int2 xz ) { x = xz.x; z = xz.y; }

//------------------------------------------------------------------------------
// ae::Colors
// It's expensive to do the srgb conversion everytime these are constructed so
// do it once and then return a copy each time static Color functions are called.
//------------------------------------------------------------------------------
// clang-format off
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
// Other
inline Color Color::Cyan() { static Color c = Color( 0.0f, 1.0f, 1.0f ); return c; }
inline Color Color::Magenta() { static Color c = Color( 1.0f, 0.0f, 1.0f ); return c; }
// aether
inline Color Color::AetherDarkRed() { static Color c = Color::SRGB8( 175, 65, 90 ); return c; }
inline Color Color::AetherRed() { static Color c = Color::SRGB8( 240, 75, 90 ); return c; }
inline Color Color::AetherOrange() { static Color c = Color::SRGB8( 255, 150, 60 ); return c; }
inline Color Color::AetherYellow() { static Color c = Color::SRGB8( 250, 205, 100 ); return c; }
inline Color Color::AetherGreen() { static Color c = Color::SRGB8( 180, 240, 80 ); return c; }
inline Color Color::AetherTeal() { static Color c = Color::SRGB8( 90, 195, 185 ); return c; }
inline Color Color::AetherBlue() { static Color c = Color::SRGB8( 70, 120, 225 ); return c; }
inline Color Color::AetherPurple() { static Color c = Color::SRGB8( 120, 90, 195 ); return c; }
inline Color Color::AetherWhite() { static Color c = Color::SRGB8( 235, 230, 215 ); return c; }
inline Color Color::AetherGray() { static Color c = Color::SRGB8( 145, 135, 130 ); return c; }
inline Color Color::AetherDarkGray() { static Color c = Color::SRGB8( 105, 105, 100 ); return c; }
inline Color Color::AetherBlack() { static Color c = Color::SRGB8( 45, 45, 45 ); return c; }
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
// clang-format on

//------------------------------------------------------------------------------
// ae::Color functions
//------------------------------------------------------------------------------
inline std::ostream& operator<<( std::ostream& os, Color c )
{
	return os << c.r << " " << c.g << " " << c.b << " " << c.a;
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
inline ae::Color Color::HSV( float hue, float saturation, float value )
{
	if( saturation <= 0.0f )
	{
		return ae::Color( value );
	}
	float hh = ae::Clip01( hue ) * 6.0f;
	uint32_t i = (uint32_t)hh;
	float ff = hh - i;
	float p = value * ( 1.0f - saturation );
	float q = value * ( 1.0f - ( saturation * ff ) );
	float t = value * ( 1.0f - ( saturation * ( 1.0f - ff ) ) );
	switch( i )
	{
		case 0:
			return ae::Color( value, t, p );
		case 1:
			return ae::Color( q, value, p );
		case 2:
			return ae::Color( p, value, t );
		case 3:
			return ae::Color( p, q, value );
		case 4:
			return ae::Color( t, p, value );
		case 5:
		default:
			return ae::Color( value, p, q );
	}
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
	if( snappiness == 0.0f || dt == 0.0f )
	{
		return *this;
	}
	return target.Lerp( *this, exp2( -exp2( snappiness ) * dt ) );
}
inline Color Color::ScaleRGB( float s ) const { return Color( r * s, g * s, b * s, a ); }
inline Color Color::ScaleA( float s ) const { return Color( r, g, b, a * s ); }
inline Color Color::SetA( float alpha ) const { return Color( r, g, b, alpha ); }
inline float Color::SRGBToRGB( float x ) { return powf( x , 2.2f ); }
inline float Color::RGBToSRGB( float x ) { return powf( x, 1.0f / 2.2f ); }

#if _AE_WINDOWS_
	#pragma warning(default:26495) // Re-enable uninitialized variable warning
#endif

//------------------------------------------------------------------------------
// ae::ToString functions
//------------------------------------------------------------------------------
// No implementation so this acts as a forward declaration. Also a default
// templated ae::ToString function would prevent the compiler/linker from looking
// for ae::ToString implementations in other modules.
template< typename T > std::string ToString( T value ); // @TODO: Add ref

// template<> // @TODO: Where should this empty template parameter list go?
template< uint32_t N >
std::string ToString( Str< N > value )
{
	return value;
}

template<>
inline std::string ToString( char const * value )
{
	return value;
}

template<>
inline std::string ToString( std::string value )
{
	return value;
}

template<>
inline std::string ToString( int8_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRId8, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( int16_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRId16, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( int32_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRId32, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( int64_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRId64, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( uint8_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRIu8, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( uint16_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRIu16, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( uint32_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRIu32, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( uint64_t value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%" PRIu64, value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( float value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%.3f", value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( double value )
{
	char str[ 32 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%.6lf", value );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( bool v )
{
	return v ? "true" : "false";
}

template<>
inline std::string ToString( ae::Vec2 v )
{
	char str[ 128 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%.3f %.3f", v.x, v.y );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( ae::Vec3 v )
{
	char str[ 128 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%.3f %.3f %.3f", v.x, v.y, v.z );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( ae::Vec4 v )
{
	char str[ 128 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%.3f %.3f %.3f %.3f", v.x, v.y, v.z, v.w );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( ae::Int2 value )
{
	char str[ 128 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%d %d", value.x, value.y );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( ae::Int3 value )
{
	char str[ 128 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%d %d %d", value.x, value.y, value.z );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( ae::Color v )
{
	char str[ 128 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1, "%.3f %.3f %.3f %.3f", v.r, v.g, v.b, v.a );
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

template<>
inline std::string ToString( ae::Matrix4 v )
{
	char str[ 256 ];
	const uint32_t length = snprintf( str, sizeof( str ) - 1,
		"%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f",
		v.data[ 0 ], v.data[ 1 ], v.data[ 2 ], v.data[ 3 ],
		v.data[ 4 ], v.data[ 5 ], v.data[ 6 ], v.data[ 7 ],
		v.data[ 8 ], v.data[ 9 ], v.data[ 10 ], v.data[ 11 ],
		v.data[ 12 ], v.data[ 13 ], v.data[ 14 ], v.data[ 15 ]
	);
	AE_ASSERT( length < sizeof(str) );
	return std::string( str, length );
}

//------------------------------------------------------------------------------
// ae::FromString functions
//------------------------------------------------------------------------------
// No implementation so this acts as a forward declaration. A default templated
// ae::ToString implementation would prevent the compiler/linker from looking
// for ae::ToString definitions in other modules.
template< typename T > T FromString( const char* str, const T& defaultValue );

template<>
inline std::string FromString( const char* str, const std::string& )
{
	return std::string( str );
}

template<>
inline int8_t FromString( const char* str, const int8_t& defaultValue )
{
	int8_t v;
	if( sscanf( str, "%" SCNi8, &v ) == 1 )
	{
		return v;
	}

	return defaultValue;
}

template<>
inline int16_t FromString( const char* str, const int16_t& defaultValue )
{
	int16_t v;
	if( sscanf( str, "%" SCNi16, &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline int32_t FromString( const char* str, const int32_t& defaultValue )
{
	int32_t v;
	if( sscanf( str, "%" SCNi32, &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline int64_t FromString( const char* str, const int64_t& defaultValue )
{
	int64_t v;
	if( sscanf( str, "%" SCNi64, &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline uint8_t FromString( const char* str, const uint8_t& defaultValue )
{
	uint8_t v;
	if( sscanf( str, "%" SCNu8, &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline uint16_t FromString( const char* str, const uint16_t& defaultValue )
{
	uint16_t v;
	if( sscanf( str, "%" SCNu16, &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline uint32_t FromString( const char* str, const uint32_t& defaultValue )
{
	uint32_t v;
	if( sscanf( str, "%" SCNu32, &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline uint64_t FromString( const char* str, const uint64_t& defaultValue )
{
	uint64_t v;
	if( sscanf( str, "%" SCNu64, &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline float FromString( const char* str, const float& defaultValue )
{
	float v;
	if( sscanf( str, "%f", &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline double FromString( const char* str, const double& defaultValue )
{
	double v;
	if( sscanf( str, "%lf", &v ) == 1 )
	{
		return v;
	}
	return defaultValue;
}

template<>
inline ae::Int2 FromString( const char* str, const ae::Int2& defaultValue )
{
	ae::Int2 r;
	if( sscanf( str, "%d %d", r.data, r.data + 1 ) == 2 )
	{
		return r;
	}
	return defaultValue;
}

template<>
inline ae::Int3 FromString( const char* str, const ae::Int3& defaultValue )
{
	ae::Int3 r;
	if( sscanf( str, "%d %d %d", r.data, r.data + 1, r.data + 2 ) == 3 )
	{
		return r;
	}
	return defaultValue;
}

template<>
inline ae::Vec2 FromString( const char* str, const ae::Vec2& defaultValue )
{
	ae::Vec2 r;
	if( sscanf( str, "%f %f", r.data, r.data + 1 ) == 2 )
	{
		return r;
	}
	return defaultValue;
}

template<>
inline ae::Vec3 FromString( const char* str, const ae::Vec3& defaultValue )
{
	ae::Vec3 r;
	if( sscanf( str, "%f %f %f", r.data, r.data + 1, r.data + 2 ) == 3 )
	{
		return r;
	}
	return defaultValue;
}

template<>
inline ae::Vec4 FromString( const char* str, const ae::Vec4& defaultValue )
{
	ae::Vec4 r;
	if( sscanf( str, "%f %f %f %f", r.data, r.data + 1, r.data + 2, r.data + 3 ) == 4 )
	{
		return r;
	}
	return defaultValue;
}

template<>
inline ae::Matrix4 FromString( const char* str, const ae::Matrix4& defaultValue )
{
	ae::Matrix4 r;
	if( sscanf( str, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
		r.data, r.data + 1, r.data + 2, r.data + 3,
		r.data + 4, r.data + 5, r.data + 6, r.data + 7,
		r.data + 8, r.data + 9, r.data + 10, r.data + 11,
		r.data + 12, r.data + 13, r.data + 14, r.data + 15 ) == 16 )
	{
		return r;
	}
	return defaultValue;
}

template<>
inline ae::Color FromString( const char* str, const ae::Color& defaultValue )
{
	ae::Color r;
	if( sscanf( str, "%f %f %f %f", r.data, r.data + 1, r.data + 2, r.data + 3 ) == 4 )
	{
		return r;
	}
	return defaultValue;
}

template<>
inline bool FromString( const char* str, const bool& defaultValue )
{
	auto StrCmp = []( const char* boolStr, const char* inputStr )
	{
		while( *boolStr && *inputStr )
		{
			if( *boolStr != tolower( *inputStr ) ) { return false; }
			boolStr++;
			inputStr++;
		}
		return ( !*boolStr && !*inputStr );
	};
	
	float f;
	if( StrCmp( "true", str ) ) { return true; }
	if( StrCmp( "false", str ) ) { return false; }
	// @TODO: Check int first
	if( sscanf( str, "%f", &f ) == 1 ) { return (bool)f; }
	return defaultValue;
}

//------------------------------------------------------------------------------
// ae::Str functions
//------------------------------------------------------------------------------
template< uint32_t N >
std::ostream& operator<<( std::ostream& out, const Str< N >& str )
{
	return out << str.c_str();
}

template< uint32_t N >
std::istream& operator>>( std::istream& in, Str< N >& str )
{
	in.getline( str.m_str, Str< N >::MaxLength() );
	str.m_length = in.gcount();
	str.m_str[ str.m_length ] = 0;
	return in;
}

template< uint32_t N >
Str< N >::Str()
{
	AE_STATIC_ASSERT_MSG( sizeof( *this ) == N, "Incorrect Str size" );
	m_length = 0;
	m_str[ 0 ] = 0;
}

template< uint32_t N >
template< uint32_t N2 >
Str< N >::Str( const Str<N2>& str )
{
	AE_ASSERT_MSG( str.m_length <= (uint16_t)MaxLength(), "Str:'#' Length:# Max:#", str, str.m_length, MaxLength() );
	m_length = str.m_length;
	memcpy( m_str, str.m_str, m_length + 1u );
}

template< uint32_t N >
Str< N >::Str( const char* str )
{
	m_length = (uint16_t)strlen( str );
	AE_ASSERT_MSG( m_length <= (uint16_t)MaxLength(), "Str:'#' Length:# Max:#", str, m_length, MaxLength() );
	memcpy( m_str, str, m_length + 1u );
}

template< uint32_t N >
Str< N >::Str( uint32_t length, const char* str )
{
	AE_ASSERT_MSG( length <= (uint16_t)MaxLength(), "'#' > #", str, MaxLength() );
	m_length = (uint16_t)length;
	memcpy( m_str, str, m_length );
	m_str[ length ] = 0;
}

template< uint32_t N >
Str< N >::Str( const char* begin, const char* end )
{
	m_length = (uint16_t)( end - begin );
	AE_ASSERT_MSG( m_length <= (uint16_t)MaxLength(), "Str:'#' Length:# Max:#", begin, m_length, MaxLength() );
	memcpy( m_str, begin, m_length );
	m_str[ m_length ] = 0;
}

template< uint32_t N >
Str< N >::Str( uint32_t length, char c )
{
	AE_ASSERT_MSG( length <= (uint16_t)MaxLength(), "# > #", length, MaxLength() );
	m_length = (uint16_t)length;
	memset( m_str, c, m_length );
	m_str[ length ] = 0;
}

template< uint32_t N >
Str< N >::operator const char*() const
{
	return m_str;
}

template< uint32_t N >
const char* Str< N >::c_str() const
{
	return m_str;
}

template< uint32_t N >
template< uint32_t N2 >
void Str< N >::operator =( const Str<N2>& str )
{
	AE_ASSERT_MSG( str.m_length <= (uint16_t)MaxLength(), "'#' > #", str, MaxLength() );
	m_length = str.m_length;
	memcpy( m_str, str.m_str, str.m_length + 1u );
}

template< uint32_t N >
Str<N> Str< N >::operator +( const char* str ) const
{
	Str<N> out( *this );
	out += str;
	return out;
}

template< uint32_t N >
template< uint32_t N2 >
Str<N> Str< N >::operator +( const Str<N2>& str ) const
{
	Str<N> out( *this );
	out += str;
	return out;
}

template< uint32_t N >
void Str< N >::operator +=( const char* str )
{
	uint32_t len = (uint32_t)strlen( str );
	AE_ASSERT_MSG( m_length + len <= (uint16_t)MaxLength(), "'#' + '#' > #", m_str, str, MaxLength() );
	memcpy( m_str + m_length, str, len + 1u );
	m_length += len;
}

template< uint32_t N >
template< uint32_t N2 >
void Str< N >::operator +=( const Str<N2>& str )
{
	AE_ASSERT_MSG( m_length + str.m_length <= (uint16_t)MaxLength(), "'#' + '#' > #", m_str, str, MaxLength() );
	memcpy( m_str + m_length, str.c_str(), str.m_length + 1u );
	m_length += str.m_length;
}

template< uint32_t N >
template< uint32_t N2 >
bool Str< N >::operator ==( const Str<N2>& str ) const
{
	return strcmp( m_str, str.c_str() ) == 0;
}

template< uint32_t N >
bool Str< N >::operator ==( const char* str ) const
{
	return strcmp( m_str, str ) == 0;
}

template< uint32_t N >
bool operator ==( const char* str0, const Str<N>& str1 )
{
	return strcmp( str0, str1.m_str ) == 0;
}

template< uint32_t N >
template< uint32_t N2 >
bool Str< N >::operator !=( const Str<N2>& str ) const
{
	return !operator==( str );
}

template< uint32_t N >
bool Str< N >::operator !=( const char* str ) const
{
	return !operator==( str );
}

template< uint32_t N >
bool operator !=( const char* str0, const Str<N>& str1 )
{
	return !operator==( str0, str1 );
}

template< uint32_t N >
template< uint32_t N2 >
bool Str< N >::operator <( const Str<N2>& str ) const
{
	return strcmp( m_str, str.c_str() ) < 0;
}

template< uint32_t N >
bool Str< N >::operator <( const char* str ) const
{
	return strcmp( m_str, str ) < 0;
}

template< uint32_t N >
bool operator <( const char* str0, const Str<N>& str1 )
{
	return strcmp( str0, str1.m_str ) < 0;
}

template< uint32_t N >
template< uint32_t N2 >
bool Str< N >::operator >( const Str<N2>& str ) const
{
	return strcmp( m_str, str.c_str() ) > 0;
}

template< uint32_t N >
bool Str< N >::operator >( const char* str ) const
{
	return strcmp( m_str, str ) > 0;
}

template< uint32_t N >
bool operator >( const char* str0, const Str<N>& str1 )
{
	return strcmp( str0, str1.m_str ) > 0;
}

template< uint32_t N >
template< uint32_t N2 >
bool Str< N >::operator <=( const Str<N2>& str ) const
{
	return strcmp( m_str, str.c_str() ) <= 0;
}

template< uint32_t N >
bool Str< N >::operator <=( const char* str ) const
{
	return strcmp( m_str, str ) <= 0;
}

template< uint32_t N >
bool operator <=( const char* str0, const Str<N>& str1 )
{
	return strcmp( str0, str1.m_str ) <= 0;
}

template< uint32_t N >
template< uint32_t N2 >
bool Str< N >::operator >=( const Str<N2>& str ) const
{
	return strcmp( m_str, str.c_str() ) >= 0;
}

template< uint32_t N >
bool Str< N >::operator >=( const char* str ) const
{
	return strcmp( m_str, str ) >= 0;
}

template< uint32_t N >
bool operator >=( const char* str0, const Str<N>& str1 )
{
	return strcmp( str0, str1.m_str ) >= 0;
}

template< uint32_t N >
char& Str< N >::operator[]( uint32_t i )
{
	AE_ASSERT_MSG( i <= m_length, "'#'[ # ]", m_str, i ); // @NOTE: Allow indexing null (length + 1)
	return m_str[ i ];
}

template< uint32_t N >
const char Str< N >::operator[]( uint32_t i ) const
{
	AE_ASSERT_MSG( i <= m_length, "'#'[ # ]", m_str, i ); // @NOTE: Allow indexing null (length + 1)
	return m_str[ i ];
}

template< uint32_t N >
uint32_t Str< N >::Length() const
{
	return m_length;
}

template< uint32_t N >
uint32_t Str< N >::Size() const
{
	return MaxLength();
}

template< uint32_t N >
bool Str< N >::Empty() const
{
	return m_length == 0;
}

template< uint32_t N >
template< uint32_t N2 >
void Str< N >::Append( const Str<N2>& str )
{
	*this += str;
}

template< uint32_t N >
void Str< N >::Append( const char* str )
{
	*this += str;
}

template< uint32_t N >
void Str< N >::Trim( uint32_t len )
{
	if( len >= m_length )
	{
		return; // Not longer than desired length
	}
	m_length = (uint16_t)len;
	m_str[ m_length ] = 0;
}

template< uint32_t N >
template< typename... Args >
Str< N > Str< N >::Format( const char* format, Args... args )
{
	Str< N > result( "" );
	result.m_Format( format, args... );
	return result;
}

template< uint32_t N >
void Str< N >::m_Format( const char* format )
{
	*this += format;
}

template< uint32_t N >
template< typename T, typename... Args >
void Str< N >::m_Format( const char* format, T value, Args... args )
{
	if( !*format )
	{
		return;
	}

	const char* head = format;
	while( *head && *head != '#' )
	{
		head++;
	}
	if( head > format )
	{
		*this += Str< N >( (uint32_t)( head - format ), format );
	}

	if( *head == '#' )
	{
		// @TODO: Replace with ae::ToString()
		std::ostringstream stream;
		stream << std::setprecision( 4 );
		stream << std::boolalpha;
		stream << value;
		*this += stream.str().c_str();
		head++;
	}
	m_Format( head, args... );
}

//------------------------------------------------------------------------------
// ae::GetHash32 inline helpers
//------------------------------------------------------------------------------
template<> inline uint32_t GetHash32( const bool& value ) { return (uint32_t)value; }
template<> inline uint32_t GetHash32( const int8_t& value ) { return (uint32_t)value; }
template<> inline uint32_t GetHash32( const int16_t& value ) { return (uint32_t)value; }
template<> inline uint32_t GetHash32( const int32_t& value ) { return (uint32_t)value; }
template<> inline uint32_t GetHash32( const int64_t& value ) { return ae::Hash32().HashData( &value, sizeof(value) ).Get(); }
template<> inline uint32_t GetHash32( const uint8_t& value ) { return (uint32_t)value; }
template<> inline uint32_t GetHash32( const uint16_t& value ) { return (uint32_t)value; }
template<> inline uint32_t GetHash32( const uint32_t& value ) { return value; }
template<> inline uint32_t GetHash32( const uint64_t& value ) { return ae::Hash32().HashData( &value, sizeof(value) ).Get(); }
template<> inline uint32_t GetHash32( const float& value ) { return ae::Hash32().HashData( &value, sizeof(value) ).Get(); }
template<> inline uint32_t GetHash32( const double& value ) { return ae::Hash32().HashData( &value, sizeof(value) ).Get(); }
template<> inline uint32_t GetHash32( const ae::Vec2& value ) { return ae::Hash32().HashType( value.data ).Get(); }
template<> inline uint32_t GetHash32( const ae::Vec3& value ) { return ae::Hash32().HashType( value.data ).Get(); }
template<> inline uint32_t GetHash32( const ae::Vec4& value ) { return ae::Hash32().HashType( value.data ).Get(); }
template<> inline uint32_t GetHash32( const ae::Matrix4& value ) { return ae::Hash32().HashType( value.data ).Get(); }
template<> inline uint32_t GetHash32( const char* const& value ) { return ae::Hash32().HashString( value ).Get(); }
template<> inline uint32_t GetHash32( char* const& value ) { return ae::Hash32().HashString( value ).Get(); }
template< uint32_t N > inline uint32_t GetHash32( const char (&value)[ N ] ) { return ae::Hash32().HashString( value ).Get(); }
template<> inline uint32_t GetHash32( const std::string& value ) { return ae::Hash32().HashString( value.c_str() ).Get(); }
template<> inline uint32_t GetHash32( const ae::Hash32& value ) { return value.Get(); }
template< typename T > inline uint32_t GetHash32( T* const& value ) { return ae::Hash32().HashData( &value, sizeof(value) ).Get(); }
template< uint32_t N > inline uint32_t GetHash32( const ae::Str< N >& value ) { return ae::Hash32().HashString( value.c_str() ).Get(); }

//------------------------------------------------------------------------------
// ae::GetHash64 inline helpers
//------------------------------------------------------------------------------
template<> inline uint64_t GetHash64( const bool& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const int8_t& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const int16_t& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const int32_t& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const int64_t& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const uint8_t& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const uint16_t& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const uint32_t& value ) { return (uint64_t)value; }
template<> inline uint64_t GetHash64( const uint64_t& value ) { return value; }
template<> inline uint64_t GetHash64( const float& value ) { return ae::Hash64().HashData( &value, sizeof(value) ).Get(); }
template<> inline uint64_t GetHash64( const double& value ) { return ae::Hash64().HashData( &value, sizeof(value) ).Get(); }
template<> inline uint64_t GetHash64( const ae::Vec2& value ) { return ae::Hash64().HashType( value.data ).Get(); }
template<> inline uint64_t GetHash64( const ae::Vec3& value ) { return ae::Hash64().HashType( value.data ).Get(); }
template<> inline uint64_t GetHash64( const ae::Vec4& value ) { return ae::Hash64().HashType( value.data ).Get(); }
template<> inline uint64_t GetHash64( const ae::Matrix4& value ) { return ae::Hash64().HashType( value.data ).Get(); }
template<> inline uint64_t GetHash64( const char* const& value ) { return ae::Hash64().HashString( value ).Get(); }
template<> inline uint64_t GetHash64( char* const& value ) { return ae::Hash64().HashString( value ).Get(); }
template< uint32_t N > inline uint64_t GetHash64( const char (&value)[ N ] ) { return ae::Hash64().HashString( value ).Get(); }
template<> inline uint64_t GetHash64( const std::string& value ) { return ae::Hash64().HashString( value.c_str() ).Get(); }
template<> inline uint64_t GetHash64( const ae::Hash64& value ) { return value.Get(); }
template< typename T > inline uint64_t GetHash64( T* const& value ) { return ae::Hash64().HashData( &value, sizeof(value) ).Get(); }
template< uint32_t N > inline uint64_t GetHash64( const ae::Str< N >& value ) { return ae::Hash64().HashString( value.c_str() ).Get(); }

//------------------------------------------------------------------------------
// ae::GetHash
//------------------------------------------------------------------------------
template< class C >
struct _HasMethodHash32
{
	template< class T > static std::true_type TestSignature( uint32_t ( T::* )() const );
	template< class T > static decltype( TestSignature( &T::GetHash32 ) ) Test( std::nullptr_t );
	template< class T > static std::false_type Test( ... );
	static const bool value = decltype( Test< C >( nullptr ) )::value;
};
template< class C >
struct _HasMethodHash64
{
	template< class T > static std::true_type TestSignature( uint64_t ( T::* )() const );
	template< class T > static decltype( TestSignature( &T::GetHash64 ) ) Test( std::nullptr_t );
	template< class T > static std::false_type Test( ... );
	static const bool value = decltype( Test< C >( nullptr ) )::value;
};
template< typename U, typename T >
U GetHash( const T& v )
{
	AE_STATIC_ASSERT_MSG( sizeof(U) == 4 || sizeof(U) == 8, "Unsupported Hash< T >" );
	if constexpr( sizeof(U) == 4 )
	{
		if constexpr( _HasMethodHash32< T >::value ){ return v.GetHash32(); }
		else{ return ae::GetHash32( v ); }
	}
	else if constexpr( sizeof(U) == 8 )
	{
		if constexpr( _HasMethodHash64< T >::value ){ return v.GetHash64(); }
		else{ return ae::GetHash64( v ); }
	}
	return 0;
}

//------------------------------------------------------------------------------
// ae::Hash templated member functions
//------------------------------------------------------------------------------
template< typename U >
Hash< U >::Hash()
{
	if constexpr( sizeof(U) == 4 )
	{
		m_hash = (U)AE_HASH32_FNV1A_OFFSET_BASIS_CONFIG;
	}
	else if constexpr( sizeof(U) == 8 )
	{
		m_hash = (U)AE_HASH64_FNV1A_OFFSET_BASIS_CONFIG;
	}
	else
	{
		m_hash = 0;
	}
}

template< typename U >
Hash< U >::Hash( U initialValue )
{
	m_hash = initialValue;
}

template<>
constexpr uint32_t Hash< uint32_t >::m_GetPrime()
{
	return AE_HASH32_FNV1A_PRIME_CONFIG;
}

template<>
constexpr uint64_t Hash< uint64_t >::m_GetPrime()
{
	return AE_HASH64_FNV1A_PRIME_CONFIG;
}

template< typename U >
template< typename T, uint32_t N >
Hash< U >& Hash< U >::HashType( const T (&array)[ N ] )
{
	for( const T& v : array )
	{
		HashType( v );
	}
	return *this;
}

template< typename U >
template< typename T >
Hash< U >& Hash< U >::HashType( const T& v )
{
	m_hash = m_hash ^ ae::GetHash< U >( v );
	m_hash *= m_GetPrime();
	return *this;
}

template< typename U >
Hash< U >& Hash< U >::HashString( const char* str )
{
	while( *str )
	{
		m_hash = m_hash ^ str[ 0 ];
		m_hash *= m_GetPrime();
		str++;
	}

	return *this;
}

template< typename U >
Hash< U >& Hash< U >::HashData( const void* _data, uint32_t length )
{
	const uint8_t* data = (const uint8_t*)_data;
	for( uint32_t i = 0; i < length; i++ )
	{
		m_hash = m_hash ^ data[ i ];
		m_hash *= m_GetPrime();
	}

	return *this;
}

template< typename U >
void Hash< U >::Set( U hash )
{
	m_hash = hash;
}

template< typename U >
U Hash< U >::Get() const
{
	return m_hash;
}

//------------------------------------------------------------------------------
// ae::Optional templated member functions
//------------------------------------------------------------------------------
template< typename T >
Optional< T >::Optional( const T& value ) : m_hasValue( false )
{
	*this = value;
}

template< typename T >
Optional< T >::Optional( T&& value ) noexcept : m_hasValue( false )
{
	*this = std::move( value );
}

template< typename T >
Optional< T >::Optional( const Optional< T >& other ) : m_hasValue( false )
{
	*this = other;
}

template< typename T >
Optional< T >::Optional( Optional< T >&& other ) noexcept : m_hasValue( false )
{
	*this = std::move( other );
}

template< typename T >
Optional< T >::~Optional()
{
	Clear();
}

template< typename T >
T& Optional< T >::operator =( const T& value )
{
	T* result = reinterpret_cast< T* >( &m_value );
	if( result == &value )
	{
		return *result;
	}
	else if( m_hasValue )
	{
		*result = value;
	}
	else
	{
		new( result ) T( value );
		m_hasValue = true;
	}
	return *result;
}

template< typename T >
T& Optional< T >::operator =( T&& value ) noexcept
{
	T* result = reinterpret_cast< T* >( &m_value );
	if( result == &value )
	{
		return *result;
	}
	else if( m_hasValue )
	{
		*result = std::move( value );
	}
	else
	{
		new( &m_value ) T( std::move( value ) );
		m_hasValue = true;
	}
	return *result;
}

template< typename T >
void Optional< T >::operator =( const Optional< T >& other )
{
	if( this == &other )
	{
		return;
	}
	else if( other.m_hasValue )
	{
		*this = *reinterpret_cast< const T* >( &other.m_value );
	}
	else
	{
		Clear();
	}
}

template< typename T >
void Optional< T >::operator =( Optional< T >&& other ) noexcept
{
	if( this == &other )
	{
		return;
	}
	else if( other.m_hasValue )
	{
		*this = std::move( *reinterpret_cast< T* >( &other.m_value ) );
		other.Clear();
	}
	else
	{
		Clear();
	}
}

template< typename T >
void Optional< T >::Clear()
{
	if( m_hasValue )
	{
		reinterpret_cast< T* >( &m_value )->~T();
		m_hasValue = false;
	}
}

template< typename T >
T* Optional< T >::TryGet()
{
	return m_hasValue ? reinterpret_cast< T* >( &m_value ) : nullptr;
}

template< typename T >
const T* Optional< T >::TryGet() const
{
	return m_hasValue ? reinterpret_cast< const T* >( &m_value ) : nullptr;
}

template< typename T >
T Optional< T >::Get( T defaultValue ) const
{
	if( m_hasValue )
	{
		return *reinterpret_cast< const T* >( &m_value );
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
// ae::Array functions
//------------------------------------------------------------------------------
template< typename T, uint32_t N >
inline std::ostream& operator<<( std::ostream& os, const Array< T, N >& array )
{
	os << "<";
	for( uint32_t i = 0; i < array.Length(); i++ )
	{
		os << array[ i ];
		if( i != array.Length() - 1 )
		{
			os << ", ";
		}
	}
	return os << ">";
}

template< typename T, uint32_t N >
Array< T, N >::Array()
{
	AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
	
	m_length = 0;
	m_size = N;
	m_array = (T*)&m_storage;
}

template< typename T, uint32_t N >
Array< T, N >::Array( const T& value, uint32_t length )
{
	AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
	
	m_length = length;
	m_size = N;
	m_array = (T*)&m_storage;
	for( uint32_t i = 0; i < length; i++ )
	{
		new ( &m_array[ i ] ) T ( value );
	}
}

template< typename T, uint32_t N >
Array< T, N >::Array( std::initializer_list< T > initList )
{
	AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
	AE_ASSERT_MSG( N >= initList.size(), "Initializer list is longer than max length (# >= #)", N, initList.size() );
	
	m_length = (uint32_t)initList.size();
	m_size = N;
	m_array = (T*)&m_storage;
	uint32_t i = 0;
	for( const T& value : initList )
	{
		new ( &m_array[ i ] ) T ( value );
		i++;
	}
}

template< typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag ) :
	m_length( 0 ),
	m_size( 0 ),
	m_array( nullptr ),
	m_tag( tag )
{
	AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
	AE_ASSERT( tag != ae::Tag() );
}

template< typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, uint32_t size ) :
	m_length( 0 ),
	m_size( 0 ),
	m_array( nullptr ),
	m_tag( tag )
{
	AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
	Reserve( size );
}

template< typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, const T& value, uint32_t length ) :
	m_length( 0 ),
	m_size( 0 ),
	m_array( nullptr ),
	m_tag( tag )
{
	AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
	Append( value, length );
}

template< typename T, uint32_t N >
Array< T, N >::Array( const Array< T, N >& other )
{
	m_length = 0;
	m_size = N;
	m_array = N ? (T*)&m_storage : nullptr;
	m_tag = other.m_tag;
	
	// Array must be initialized above before calling Reserve
	Reserve( other.m_length );

	m_length = other.m_length;
	for( uint32_t i = 0; i < m_length; i++ )
	{
		new ( &m_array[ i ] ) T ( other.m_array[ i ] );
	}
}

template< typename T, uint32_t N >
Array< T, N >::Array( Array< T, N >&& other ) noexcept
{
	if( N )
	{
		AE_DEBUG_ASSERT( m_tag == ae::Tag() );
		AE_DEBUG_ASSERT( other.m_tag == ae::Tag() );
		m_length = 0;
		m_size = N;
		m_array = (T*)&m_storage;
		*this = other; // Regular assignment (without std::move)
	}
	else
	{
		m_tag = other.m_tag;
		m_length = other.m_length;
		m_size = other.m_size;
		m_array = other.m_array;
		
		other.m_length = 0;
		other.m_size = 0;
		other.m_array = nullptr;
		// @NOTE: Don't reset tag. 'other' must remain in a valid state.
	}
}

template< typename T, uint32_t N >
void Array< T, N >::operator =( const Array< T, N >& other )
{
	if( this == &other )
	{
		return;
	}
	
	Clear();
	
	Reserve( other.m_length );

	m_length = other.m_length;
	for( uint32_t i = 0; i < m_length; i++ )
	{
		new ( &m_array[ i ] ) T ( other.m_array[ i ] );
	}
}

template< typename T, uint32_t N >
void Array< T, N >::operator =( Array< T, N >&& other ) noexcept
{
	if( this == &other )
	{
		return;
	}
	else if( N || m_tag != other.m_tag )
	{
		*this = other; // Regular assignment (without std::move)
	}
	else
	{
		if( m_array )
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

template< typename T, uint32_t N >
Array< T, N >::~Array()
{
	Clear();
	if( N == 0 )
	{
		ae::Free( m_array );
	}
	m_size = 0;
	m_array = nullptr;
}

template< typename T, uint32_t N >
T& Array< T, N >::Append( const T& value )
{
	return *Append( value, 1 );
}

template< typename T, uint32_t N >
T* Array< T, N >::Append( const T& value, uint32_t count )
{
	Reserve( m_length + count );
	T* result = m_array + m_length;
	for( uint32_t i = 0; i < count; i++ )
	{
		new ( &m_array[ m_length ] ) T ( value );
		m_length++;
	}
	return result;
}

template< typename T, uint32_t N >
T* Array< T, N >::AppendArray( const T* values, uint32_t count )
{
	Reserve( m_length + count );
	AE_DEBUG_ASSERT( m_size >= m_length + count );
	T* result = m_array + m_length;
	for( uint32_t i = 0; i < count; i++ )
	{
		new ( &m_array[ m_length ] ) T ( values[ i ] );
		m_length++;
	}
	return result;
}

template< typename T, uint32_t N >
T& Array< T, N >::Insert( uint32_t index, const T& value )
{
	return *Insert( index, value, 1 );
}

template< typename T, uint32_t N >
T* Array< T, N >::Insert( uint32_t index, const T& value, uint32_t count )
{
	AE_DEBUG_ASSERT( index <= m_length );
	Reserve( m_length + count );
	T* result = m_array + index;
	const int32_t indexCount = index + count;
	const uint32_t pivot0 = ae::Min( (uint32_t)indexCount, m_length );
	const int32_t pivot1 = ae::Max( indexCount, (int32_t)m_length );
	m_length += count;
	// Move elements back
	for( int32_t i = m_length - 1; i >= pivot1; i-- )
	{
		new ( &m_array[ i ] ) T ( std::move( m_array[ i - count ] ) );
	}
	for( int32_t i = pivot1 - 1; i >= indexCount; i-- )
	{
		m_array[ i ] = std::move( m_array[ i - count ] );
	}
	// Copy new elements
	for( uint32_t i = index; i < pivot0; i++ )
	{
		m_array[ i ] = value;
	}
	for( uint32_t i = pivot0; i < indexCount; i++ )
	{
		new ( &m_array[ i ] ) T ( value );
	}
	return result;
}

template< typename T, uint32_t N >
T* Array< T, N >::InsertArray( uint32_t index, const T* values, uint32_t count )
{
	AE_DEBUG_ASSERT( index <= m_length );
	Reserve( m_length + count );
	T* result = m_array + index;
	const int32_t indexCount = index + count;
	const uint32_t pivot0 = ae::Min( (uint32_t)indexCount, m_length );
	const int32_t pivot1 = ae::Max( indexCount, (int32_t)m_length );
	m_length += count;
	// Move elements back
	for( int32_t i = m_length - 1; i >= pivot1; i-- )
	{
		new ( &m_array[ i ] ) T ( std::move( m_array[ i - count ] ) );
	}
	for( int32_t i = pivot1 - 1; i >= indexCount; i-- )
	{
		m_array[ i ] = std::move( m_array[ i - count ] );
	}
	// Copy new elements
	uint32_t j = 0;
	for( uint32_t i = index; i < pivot0; i++, j++ )
	{
		m_array[ i ] = values[ j ];
	}
	for( uint32_t i = pivot0; i < indexCount; i++, j++ )
	{
		new ( &m_array[ i ] ) T ( values[ j ] );
	}
	AE_DEBUG_ASSERT( j == count );
	return result;
}

template< typename T, uint32_t N >
void Array< T, N >::Remove( uint32_t index, uint32_t count )
{
	AE_DEBUG_ASSERT( index <= m_length );
	AE_DEBUG_ASSERT( index + count <= m_length );
	m_length -= count;
	for( uint32_t i = index; i < m_length; i++ )
	{
		m_array[ i ] = std::move( m_array[ i + count ] );
	}
	for( uint32_t i = 0; i < count; i++ )
	{
		m_array[ m_length + i ].~T();
	}
}

template< typename T, uint32_t N >
template< typename U >
uint32_t Array< T, N >::RemoveAll( const U& value )
{
	uint32_t count = 0;
	int32_t index = 0;
	while( ( index = Find( value ) ) >= 0 )
	{
		// @TODO: Update this to be single loop, so array is only compacted once
		Remove( index );
		count++;
	}
	return count;
}

template< typename T, uint32_t N >
template< typename Fn >
uint32_t Array< T, N >::RemoveAllFn( Fn testFn )
{
	uint32_t count = 0;
	int32_t index = 0;
	while( ( index = FindFn( testFn ) ) >= 0 )
	{
		// @TODO: Update this to be single loop, so array is only compacted once
		Remove( index );
		count++;
	}
	return count;
}

template< typename T, uint32_t N >
template< typename U >
int32_t Array< T, N >::Find( const U& value ) const
{
	for( uint32_t i = 0; i < m_length; i++ )
	{
		if( m_array[ i ] == value )
		{
			return i;
		}
	}
	return -1;
}

template< typename T, uint32_t N >
template< typename Fn >
int32_t Array< T, N >::FindFn( Fn testFn ) const
{
	for( uint32_t i = 0; i < m_length; i++ )
	{
		if( testFn( m_array[ i ] ) )
		{
			return i;
		}
	}
	return -1;
}

template< typename T, uint32_t N >
template< typename U >
int32_t Array< T, N >::FindLast( const U& value ) const
{
	for( int32_t i = m_length - 1; i >= 0; i-- )
	{
		if( m_array[ i ] == value )
		{
			return i;
		}
	}
	return -1;
}

template< typename T, uint32_t N >
template< typename Fn >
int32_t Array< T, N >::FindLastFn( Fn testFn ) const
{
	for( int32_t i = m_length - 1; i >= 0; i-- )
	{
		if( testFn( m_array[ i ] ) )
		{
			return i;
		}
	}
	return -1;
}

template< typename T, uint32_t N >
void Array< T, N >::Reserve( uint32_t _size )
{
	if( N > 0 )
	{
		AE_DEBUG_ASSERT_MSG( m_array == (T*)&m_storage, "Static array reference has been overwritten" );
		AE_ASSERT_MSG( N >= _size, "# >= #", N, _size );
		return;
	}
	else if( _size <= m_size )
	{
		return;
	}
	
	AE_DEBUG_ASSERT( m_tag != ae::Tag() );
	
	constexpr uint32_t maxExponentialSize = ( 1 << 18 );
	uint32_t size = _size;
	if( m_size == 0 && size > 1 )
	{
		// Exact amount specified, so use that
	}
	else if( size <= maxExponentialSize )
	{
		if( m_size == 0 )
		{
			// Initially allocate at least 64 bytes (rounded down) of type
			size = ae::Max( size, 64u / (uint32_t)sizeof(T) );
		}
		else
		{
			// At least double the size, to reduce the number of resizes
			size = ae::Max( size, m_size * 2 );
		}
		size = ae::NextPowerOfTwo( size );
	}
	else
	{
		// Don't double the size or use powers of two here, as the array will
		// become very very large. At this point the user should be manually
		// calling reserve with the expected needed size.
		size = ( size / maxExponentialSize + 1 ) * maxExponentialSize;
	}
	AE_DEBUG_ASSERT( size );
	AE_DEBUG_ASSERT( size >= _size );
	m_size = size;
	
	// @TODO: Try to use realloc
	T* arr = (T*)ae::Allocate( m_tag, m_size * sizeof(T), alignof(T) );
	for( uint32_t i = 0; i < m_length; i++ )
	{
		new ( &arr[ i ] ) T ( std::move( m_array[ i ] ) );
		m_array[ i ].~T();
	}
	
	ae::Free( m_array );
	m_array = arr;
}

template< typename T, uint32_t N >
void Array< T, N >::Clear()
{
	for( uint32_t i = 0; i < m_length; i++ )
	{
		m_array[ i ].~T(); // @TODO: Skip this for basic types
	}
	m_length = 0;
}

template< typename T, uint32_t N >
const T& Array< T, N >::operator[]( int32_t index ) const
{
	AE_DEBUG_ASSERT( index >= 0 );
	AE_DEBUG_ASSERT_MSG( index < (int32_t)m_length, "index: # length: #", index, m_length );
	return m_array[ index ];
}

template< typename T, uint32_t N >
T& Array< T, N >::operator[]( int32_t index )
{
	AE_DEBUG_ASSERT( index >= 0 );
	AE_DEBUG_ASSERT_MSG( index < (int32_t)m_length, "index: # length: #", index, m_length );
	return m_array[ index ];
}

//------------------------------------------------------------------------------
// ae::HashMap member functions
//------------------------------------------------------------------------------
template< typename Key, uint32_t N, typename Hash >
HashMap< Key, N, Hash >::HashMap() :
	m_entries( (Entry*)&m_storage ),
	m_size( N ),
	m_length( 0 )
{
	AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
}

template< typename Key, uint32_t N, typename Hash >
HashMap< Key, N, Hash >::HashMap( ae::Tag tag ) :
	m_tag( tag ),
	m_entries( nullptr ),
	m_size( 0 ),
	m_length( 0 )
{
	AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
	AE_ASSERT( tag != ae::Tag() );
}

template< typename Key, uint32_t N, typename Hash >
void HashMap< Key, N, Hash >::Reserve( uint32_t size )
{
	if( N )
	{
		AE_DEBUG_ASSERT_MSG( m_size >= size, "Static array size is fixed (# >= #)", m_size, size );
		return;
	}
	else if( m_size >= size )
	{
		return;
	}
	
	Entry* prevEntries = m_entries;
	const uint32_t prevSize = m_size;
	const uint32_t prevLength = m_length;
	m_length = 0;
	m_size = size;
	m_entries = ae::NewArray< Entry >( m_tag, m_size );
	if( prevEntries )
	{
		for( uint32_t i = 0; i < prevSize; i++ )
		{
			const Entry& e = prevEntries[ i ];
			if( e.index >= 0 )
			{
				const bool success = m_Insert( e.newKey, e.hash, e.index );
				AE_DEBUG_ASSERT( success );
			}
		}
		ae::Delete( prevEntries );
		AE_DEBUG_ASSERT( prevLength == m_length );
	}
}

template< typename Key, uint32_t N, typename Hash >
HashMap< Key, N, Hash >::HashMap( const HashMap< Key, N, Hash >& other ) :
	m_size( N ),
	m_length( 0 )
{
	if( N )
	{
		AE_DEBUG_ASSERT( other.m_tag == ae::Tag() );
		m_entries = (Entry*)&m_storage;
	}
	else
	{
		AE_DEBUG_ASSERT( other.m_tag != ae::Tag() );
		m_tag = other.m_tag;
		m_entries = nullptr;
	}
	*this = other;
}

template< typename Key, uint32_t N, typename Hash >
void HashMap< Key, N, Hash >::operator =( const HashMap< Key, N, Hash >& other )
{
	if( this == &other )
	{
		return;
	}
	Clear();
	Reserve( other.m_size );
	if( m_size == other.m_size )
	{
		std::copy_n( other.m_entries, m_size, m_entries );
		m_length = other.m_length;
	}
	else
	{
		for( uint32_t i = 0; i < other.m_size; i++ )
		{
			Entry e = other.m_entries[ i ];
			if( e.index >= 0 )
			{
				m_Insert( e.newKey, e.hash, e.index );
			}
		}
	}
}

template< typename Key, uint32_t N, typename Hash >
HashMap< Key, N, Hash >::~HashMap()
{
	if( N == 0 )
	{
		ae::Delete( m_entries );
	}
	m_length = 0;
	m_size = 0;
	m_entries = nullptr;
}

template< typename Key, uint32_t N, typename Hash >
bool HashMap< Key, N, Hash >::Set( Key key, uint32_t index )
{
	// Find existing
	const typename Hash::UInt hash = ae::GetHash< typename Hash::UInt >( key );
	if( m_length )
	{
		AE_DEBUG_ASSERT( m_size );
		const uint32_t startIdx = hash % m_size;
		for( uint32_t i = 0; i < m_size; i++ )
		{
			Entry* e = &m_entries[ ( i + startIdx ) % m_size ];
			if( e->index < 0 )
			{
				break;
			}
			else if( e->newKey == key )
			{
				e->hash = hash;
				e->index = index;
				return true;
			}
		}
	}
	
	// Insert new
	if( N && ( m_length >= N ) )
	{
		return false;
	}
	else if( !N && ( !m_size || ( m_length / (float)m_size ) > 0.8f ) )
	{
		Reserve( m_size ? m_size * 2 : 32 );
	}
	return m_Insert( key, hash, index );
}

template< typename Key, uint32_t N, typename Hash >
int32_t HashMap< Key, N, Hash >::Remove( Key key )
{
	if( !m_length )
	{
		return -1;
	}
	Entry* entry = nullptr;
	{
		AE_DEBUG_ASSERT( m_size );
		const typename Hash::UInt hash = ae::GetHash< typename Hash::UInt >( key );
		const uint32_t startIdx = hash % m_size;
		for( uint32_t i = 0; i < m_size; i++ )
		{
			Entry* e = &m_entries[ ( i + startIdx ) % m_size ];
			if( e->index < 0 )
			{
				return -1;
			}
			else if( e->newKey == key )
			{
				entry = e;
				break;
			}
		}
	}
	if( entry )
	{
		int32_t result = entry->index;
		AE_DEBUG_ASSERT( result >= 0 );
		// Compact section of table at removed entry until an entry matches
		// their hash index exactly or a gap is found.
		const uint32_t startIndex = uint32_t( entry - m_entries );
		uint32_t targetIndex = startIndex;
		for( uint32_t i = 1; i < m_size; i++ )
		{
			uint32_t fromIndex = ( i + startIndex ) % m_size;
			Entry* e = &m_entries[ fromIndex ];
			if( e->index < 0 || ( ( e->hash % m_size ) == fromIndex ) )
			{
				break;
			}
			m_entries[ targetIndex ] = *e;
			targetIndex = fromIndex;
		}
		m_entries[ targetIndex ].index = -1;
		AE_DEBUG_ASSERT( m_length > 0 );
		m_length--;
		return result;
	}
	return -1;
}

template< typename Key, uint32_t N, typename Hash >
void HashMap< Key, N, Hash >::Decrement( uint32_t index )
{
	if( m_length )
	{
		for( uint32_t i = 0; i < m_size; i++ )
		{
			Entry* e = &m_entries[ i ];
			if( e->index > index )
			{
				e->index--;
			}
		}
	}
}

template< typename Key, uint32_t N, typename Hash >
int32_t HashMap< Key, N, Hash >::Get( Key key ) const
{
	if( m_length )
	{
		AE_DEBUG_ASSERT( m_size );
		const typename Hash::UInt hash = ae::GetHash< typename Hash::UInt >( key );
		const uint32_t startIdx = hash % m_size;
		for( uint32_t i = 0; i < m_size; i++ )
		{
			Entry* e = &m_entries[ ( i + startIdx ) % m_size ];
			if( e->index < 0 )
			{
				return -1;
			}
			else if( e->newKey == key )
			{
				return e->index;
			}
		}
	}
	return -1;
}

template< typename Key, uint32_t N, typename Hash >
void HashMap< Key, N, Hash >::Clear()
{
	if( m_length )
	{
		m_length = 0;
		for( uint32_t i = 0; i < m_size; i++ )
		{
			m_entries[ i ].index = -1;
		}
	}
}

template< typename Key, uint32_t N, typename Hash >
uint32_t HashMap< Key, N, Hash >::Length() const
{
	return m_length;
}

template< typename Key, uint32_t N, typename Hash >
bool HashMap< Key, N, Hash >::m_Insert( Key key, typename Hash::UInt hash, int32_t index )
{
	AE_DEBUG_ASSERT( index >= 0 );
	AE_DEBUG_ASSERT( ae::GetHash< typename Hash::UInt >( key ) == hash );
	// 'hash' is modified in loop
	const uint32_t startIdx = ( hash % m_size );
	for( uint32_t i = 0; i < m_size; i++ )
	{
		uint32_t currentIdx = ( i + startIdx ) % m_size;
		Entry* e = &m_entries[ currentIdx ];
		if( e->index < 0 )
		{
			e->newKey = key;
			e->hash = hash;
			e->index = index;
			m_length++;
			return true;
		}
#define mod_subtract( idx, baseIdx ) ( ( (idx) + m_size - (baseIdx) ) % m_size )
		const uint32_t currDist = mod_subtract( currentIdx, ( e->hash % m_size ) );
		const uint32_t dist = mod_subtract( currentIdx, ( hash % m_size ) );
#undef mod_subtract
		if( dist > currDist )
		{
			std::swap( e->newKey, key );
			std::swap( e->hash, hash );
			std::swap( e->index, index );
		}
	}
	return false;
};

//------------------------------------------------------------------------------
// ae::Map member functions
//------------------------------------------------------------------------------
template< typename K, typename V, uint32_t N, typename H, MapMode M >
Map< K, V, N, H, M >::Map()
{
	AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static maps" );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
Map< K, V, N, H, M >::Map( ae::Tag pool ) :
	m_hashMap( pool ),
	m_pairs( pool )
{
	AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static maps" );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
V& Map< K, V, N, H, M >::Set( const K& key, const V& value )
{
	int32_t index = GetIndex( key ); // @TODO: SetIfMissing()? to avoid double lookup of key
	Pair< K, V >* pair = ( index >= 0 ) ? &m_pairs[ index ] : nullptr;
	if( pair )
	{
		pair->value = value;
		return pair->value;
	}
	else
	{
		uint32_t idx = m_pairs.Length();
		m_hashMap.Set( key, idx ); // @TODO: Handle bad return value
		return m_pairs.Append( Pair( key, value ) ).value;
	}
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
V& Map< K, V, N, H, M >::Get( const K& key )
{
	return m_pairs[ GetIndex( key ) ].value;
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
const V& Map< K, V, N, H, M >::Get( const K& key ) const
{
	return m_pairs[ GetIndex( key ) ].value;
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
template< typename V2 > 
V Map< K, V, N, H, M >::Get( const K& key, V2&& defaultValue ) const&
{
	int32_t index = GetIndex( key );
	return ( index >= 0 ) ? m_pairs[ index ].value : static_cast< V >( std::forward< V2 >( defaultValue ) );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
V* Map< K, V, N, H, M >::TryGet( const K& key )
{
	return const_cast< V* >( const_cast< const Map< K, V, N, H, M >* >( this )->TryGet( key ) );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
const V* Map< K, V, N, H, M >::TryGet( const K& key ) const
{
	int32_t index = GetIndex( key );
	if( index >= 0 )
	{
		return &m_pairs[ index ].value;
	}
	else
	{
		return nullptr;
	}
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
bool Map< K, V, N, H, M >::TryGet( const K& key, V* valueOut )
{
	return const_cast< const Map< K, V, N, H, M >* >( this )->TryGet( key, valueOut );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
bool Map< K, V, N, H, M >::TryGet( const K& key, V* valueOut ) const
{
	const V* val = TryGet( key );
	if( val )
	{
		if( valueOut )
		{
			*valueOut = *val;
		}
		return true;
	}
	return false;
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
bool Map< K, V, N, H, M >::Remove( const K& key, V* valueOut )
{
	return m_RemoveIndex( m_hashMap.Remove(  key ), valueOut );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
void Map< K, V, N, H, M >::RemoveIndex( uint32_t index, V* valueOut )
{
	const K& key = m_pairs[ index ].key;
#if _AE_DEBUG_
	const int32_t checkIdx = m_hashMap.Remove( key );
	AE_ASSERT( checkIdx == index );
#else
	m_hashMap.Remove( key );
#endif
	m_RemoveIndex( index, valueOut );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
bool Map< K, V, N, H, M >::m_RemoveIndex( int32_t index, V* valueOut )
{
	if( index >= 0 )
	{
		AE_DEBUG_ASSERT( m_pairs.Length() );
		if( valueOut ) { *valueOut = m_pairs[ index ].value; }
		if( index == m_pairs.Length() - 1 )
		{
			m_pairs.Remove( index );
		}
		else if constexpr( M == ae::MapMode::Stable )
		{
			m_pairs.Remove( index );
			m_hashMap.Decrement( index );
		}
		else if constexpr( M == ae::MapMode::Fast )
		{
			uint32_t lastIdx = m_pairs.Length() - 1;
			K lastKey = m_pairs[ lastIdx ].key;
			m_pairs[ index ] = std::move( m_pairs[ lastIdx ] );
			m_pairs.Remove( lastIdx );
			m_hashMap.Set( lastKey, index );
		}
		AE_DEBUG_ASSERT( m_pairs.Length() == m_hashMap.Length() );
		return true;
	}
	else
	{
		return false;
	}
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
void Map< K, V, N, H, M >::Reserve( uint32_t count )
{
	m_pairs.Reserve( count );
	m_hashMap.Reserve( m_pairs.Size() ); // @TODO: Should this be bigger than storage, so it's faster to do lookups?
	AE_DEBUG_ASSERT( m_pairs.Length() == m_hashMap.Length() );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
void Map< K, V, N, H, M >::Clear()
{
	m_hashMap.Clear();
	m_pairs.Clear();
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
const K& Map< K, V, N, H, M >::GetKey( int32_t index ) const
{
	return m_pairs[ index ].key;
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
V& Map< K, V, N, H, M >::GetValue( int32_t index )
{
	return m_pairs[ index ].value;
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
int32_t Map< K, V, N, H, M >::GetIndex( const K& key ) const
{
	return m_hashMap.Get( key );
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
const V& Map< K, V, N, H, M >::GetValue( int32_t index ) const
{
	return m_pairs[ index ].value;
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
uint32_t Map< K, V, N, H, M >::Length() const
{
	AE_DEBUG_ASSERT( m_hashMap.Length() == m_pairs.Length() );
	return m_pairs.Length();
}

template< typename K, typename V, uint32_t N, typename H, MapMode M >
std::ostream& operator<<( std::ostream& os, const Map< K, V, N, H, M >& map )
{
	os << "{";
	for( uint32_t i = 0; i < map.m_pairs.Length(); i++ )
	{
		os << "(" << map.m_pairs[ i ].key << ", " << map.m_pairs[ i ].value << ")";
		if( i != map.m_pairs.Length() - 1 )
		{
			os << ", ";
		}
	}
	return os << "}";
}

//------------------------------------------------------------------------------
// ae::Dict members
//------------------------------------------------------------------------------
template< uint32_t N >
Dict< N >::Dict( ae::Tag tag ) :
	m_entries( tag )
{}

template< uint32_t N >
void Dict< N >::SetString( const char* key, const char* value )
{
	m_entries.Set( key, value );
}

template< uint32_t N >
void Dict< N >::SetInt( const char* key, int64_t value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetUInt( const char* key, uint64_t value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetFloat( const char* key, float value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetBool( const char* key, bool value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetVec2( const char* key, ae::Vec2 value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetVec3( const char* key, ae::Vec3 value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetVec4( const char* key, ae::Vec4 value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetInt2( const char* key, ae::Int2 value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
void Dict< N >::SetMatrix4( const char* key, const ae::Matrix4& value )
{
	const auto str = ToString( value );
	m_entries.Set( key, str.c_str() );
}

template< uint32_t N >
bool Dict< N >::Remove( const char* key )
{
	return m_entries.Remove( key );
}

template< uint32_t N >
void Dict< N >::Clear()
{
	m_entries.Clear();
}

template< uint32_t N >
const char* Dict< N >::GetString( const char* key, const char* defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return value->c_str();
	}
	return defaultValue;
}

template< uint32_t N >
int64_t Dict< N >::GetInt( const char* key, int64_t defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
uint64_t Dict< N >::GetUInt( const char* key, uint64_t defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
float Dict< N >::GetFloat( const char* key, float defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
bool Dict< N >::GetBool( const char* key, bool defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
ae::Vec2 Dict< N >::GetVec2( const char* key, ae::Vec2 defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
ae::Vec3 Dict< N >::GetVec3( const char* key, ae::Vec3 defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
ae::Vec4 Dict< N >::GetVec4( const char* key, ae::Vec4 defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
ae::Int2 Dict< N >::GetInt2( const char* key, ae::Int2 defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
ae::Matrix4 Dict< N >::GetMatrix4( const char* key, const ae::Matrix4& defaultValue ) const
{
	if( const ae::Str128* value = m_entries.TryGet( key ) )
	{
		return ae::FromString( value->c_str(), defaultValue );
	}
	return defaultValue;
}

template< uint32_t N >
bool Dict< N >::Has( const char* key ) const
{
	return m_entries.TryGet( key ) != nullptr;
}

template< uint32_t N >
const char* Dict< N >::GetKey( uint32_t idx ) const
{
	return m_entries.GetKey( idx ).c_str();
}

template< uint32_t N >
const char* Dict< N >::GetValue( uint32_t idx ) const
{
	return m_entries.GetValue( idx ).c_str();
}

template< uint32_t N >
std::ostream& operator<<( std::ostream& os, const Dict< N >& dict )
{
	os << "[";
	for( uint32_t i = 0; i < dict.Length(); i++ )
	{
		if( i )
		{
			os << ",";
		}
		os << "<'" << dict.GetKey( i ) << "','" << dict.GetValue( i ) << "'>";
	}
	return os << "]";
}

//------------------------------------------------------------------------------
// ae::ListNode member functions
//------------------------------------------------------------------------------
template< typename T >
ListNode< T >::ListNode( T* owner )
{
	m_root = nullptr;
	m_next = this;
	m_prev = this;
	m_owner = owner;
}

template< typename T >
ListNode< T >::~ListNode()
{
	Remove();
}

template< typename T >
void ListNode< T >::Remove()
{
	if( !m_root )
	{
		return;
	}

	AE_ASSERT( m_root->m_first );
	if( m_root->m_first == this )
	{
		if( m_next == this )
		{
			// Last node in list
			m_root->m_first = nullptr;
		}
		else
		{
			// Was head. Set next as head.
			m_root->m_first = m_next;
		}
	}

	m_next->m_prev = m_prev;
	m_prev->m_next = m_next;

	m_root = nullptr;
	m_next = this;
	m_prev = this;
}

template< typename T >
T* ListNode< T >::GetFirst()
{
	return const_cast< T* >( const_cast< const ListNode< T >* >( this )->GetFirst() );
}

template< typename T >
T* ListNode< T >::GetNext()
{
	return const_cast< T* >( const_cast< const ListNode< T >* >( this )->GetNext() );
}

template< typename T >
T* ListNode< T >::GetPrev()
{
	return const_cast< T* >( const_cast< const ListNode< T >* >( this )->GetPrev() );
}

template< typename T >
T* ListNode< T >::GetLast()
{
	return const_cast<T*>( const_cast<const ListNode< T >*>( this )->GetLast() );
}

template< typename T >
const T* ListNode< T >::GetFirst() const
{
	return m_root ? m_root->GetFirst() : nullptr;
}

template< typename T >
const T* ListNode< T >::GetNext() const
{
	if( !m_root || m_root->m_first == m_next )
	{
		return nullptr;
	}
	return m_next->m_owner;
}

template< typename T >
const T* ListNode< T >::GetPrev() const
{
	if( !m_root || m_root->m_first == this )
	{
		return nullptr;
	}
	return m_prev->m_owner;
}

template< typename T >
const T* ListNode< T >::GetLast() const
{
	return m_root ? m_root->GetLast() : nullptr;
}

template< typename T >
List< T >* ListNode< T >::GetList()
{
	return m_root;
}

template< typename T >
const List< T >* ListNode< T >::GetList() const
{
	return m_root;
}

//------------------------------------------------------------------------------
// ae::List member functions
//------------------------------------------------------------------------------
template< typename T >
List< T >::List() : m_first( nullptr )
{}

template< typename T >
List< T >::~List()
{
	Clear();
}

template< typename T >
void List< T >::Append( ListNode< T >& node )
{
	if( m_first )
	{
		node.Remove();

		node.m_root = this;

		node.m_next = m_first;
		node.m_prev = m_first->m_prev;

		node.m_next->m_prev = &node;
		node.m_prev->m_next = &node;
	}
	else
	{
		m_first = &node;
		node.m_root = this;
	}
}

template< typename T >
void List< T >::Clear()
{
	while( m_first )
	{
		m_first->Remove();
	}
}

template< typename T >
T* List< T >::GetFirst()
{
	return m_first ? m_first->m_owner : nullptr;
}

template< typename T >
T* List< T >::GetLast()
{
	return m_first ? m_first->m_prev->m_owner : nullptr;
}

template< typename T >
const T* List< T >::GetFirst() const
{
	return m_first ? m_first->m_owner : nullptr;
}

template< typename T >
const T* List< T >::GetLast() const
{
	return m_first ? m_first->m_prev->m_owner : nullptr;
}

template< typename T >
template< typename U >
T* List< T >::Find( const U& value )
{
	return const_cast< T* >( const_cast< const List< T >* >( this )->Find( value ) );
}

template< typename T >
template< typename Fn >
T* List< T >::FindFn( Fn predicateFn )
{
	return const_cast< T* >( const_cast< const List< T >* >( this )->FindFn( predicateFn ) );
}

template< typename T >
template< typename U >
const T* List< T >::Find( const U& value ) const
{
	if( !m_first )
	{
		return nullptr;
	}

	const ListNode< T >* current = m_first;
	do
	{
		if( *( current->m_owner ) == value )
		{
			return current->m_owner;
		}
		current = current->m_next;
	} while( current != m_first );

	return nullptr;
}

template< typename T >
template< typename Fn >
const T* List< T >::FindFn( Fn predicateFn ) const
{
	if( !m_first )
	{
		return nullptr;
	}

	const ListNode< T >* current = m_first;
	do
	{
		if( predicateFn( current->m_owner ) )
		{
			return current->m_owner;
		}
		current = current->m_next;
	} while( current != m_first );

	return nullptr;
}

template< typename T >
uint32_t List< T >::Length() const
{
	if( !m_first )
	{
		return 0;
	}

	// @TODO: Should be constant time
	uint32_t count = 1;
	ListNode< T >* current = m_first;
	while( current->m_next != m_first )
	{
		current = current->m_next;
		count++;
	}

	return count;
}

//------------------------------------------------------------------------------
// ae::RingBuffer member functions
//------------------------------------------------------------------------------
template< typename T, uint32_t N >
RingBuffer< T, N >::RingBuffer() :
	m_first( 0 ),
	m_size( N )
{}

template< typename T, uint32_t N >
RingBuffer< T, N >::RingBuffer( ae::Tag tag, uint32_t size ) :
	m_first( 0 ),
	m_size( size ),
	m_buffer( tag )
{}

template< typename T, uint32_t N >
T& RingBuffer< T, N >::Append( const T& val )
{
	if( m_buffer.Length() < Size() )
	{
		return m_buffer.Append( val );
	}
	else
	{
		AE_DEBUG_ASSERT( m_buffer.Length() == Size() );
		uint32_t idx = m_first % Size();
		m_buffer[ idx ] = val;
		m_first++;
		return m_buffer[ idx ];
	}
}

template< typename T, uint32_t N >
void RingBuffer< T, N >::Clear()
{
	m_first = 0;
	m_buffer.Clear();
}

template< typename T, uint32_t N >
T& RingBuffer< T, N >::Get( uint32_t index )
{
	AE_ASSERT( index < m_buffer.Length() );
	return m_buffer[ ( m_first + index ) % Size() ];
}

template< typename T, uint32_t N >
const T& RingBuffer< T, N >::Get( uint32_t index ) const
{
	AE_ASSERT( index < m_buffer.Length() );
	return m_buffer[ ( m_first + index ) % Size() ];
}

//------------------------------------------------------------------------------
// ae::FreeList member functions
//------------------------------------------------------------------------------
template< uint32_t N >
FreeList< N >::FreeList() :
	m_pool( Entry(), N )
{
	AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
	FreeAll();
}

template< uint32_t N >
FreeList< N >::FreeList( const ae::Tag& tag, uint32_t size ) :
	m_pool( tag, Entry(), size )
{
	AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
	FreeAll();
}

template< uint32_t N >
int32_t FreeList< N >::Allocate()
{
	if( !m_free ) { return -1; }
	Entry* entry = m_free;
	// Advance the free pointer until the sentinel is reached.
	m_free = ( m_free->next == m_free ) ? nullptr : m_free->next;
	entry->next = nullptr;
	m_length++;
	return (int32_t)( entry - m_pool.begin() );
}

template< uint32_t N >
void FreeList< N >::Free( int32_t idx )
{
	if( idx < 0 ) { return; }
	Entry* entry = &m_pool[ idx ];
#if _AE_DEBUG_
	AE_ASSERT( m_length );
	AE_ASSERT( 0 <= idx && idx < (int32_t)m_pool.Length() );
	AE_ASSERT( !entry->next );
#endif
	
	// List length of 1, last element points to itself.
	entry->next = m_free ? m_free : entry;
	m_free = entry;
	m_length--;

#if _AE_DEBUG_
	if( !m_length )
	{
		for( uint32_t i = 0; i < m_pool.Length(); i++ )
		{
			AE_ASSERT( m_pool[ i ].next );
		}
	}
#endif
}

template< uint32_t N >
void FreeList< N >::FreeAll()
{
	m_length = 0;
	for( uint32_t i = 0; i < m_pool.Length() - 1; i++ )
	{
		m_pool[ i ].next = &m_pool[ i + 1 ];
	}
	// Last element points to itself so it can be used as a sentinel.
	m_pool[ m_pool.Length() - 1 ].next = &m_pool[ m_pool.Length() - 1 ];
	m_free = &m_pool[ 0 ];
}

template< uint32_t N >
int32_t FreeList< N >::GetFirst() const
{
	if( !m_length )
	{
		return -1;
	}
	for( uint32_t i = 0; i < m_pool.Length(); i++ )
	{
		if( !m_pool[ i ].next )
		{
			return (int32_t)i;
		}
	}
#if _AE_DEBUG_
	AE_FAIL();
#endif
	return -1;
}

template< uint32_t N >
int32_t FreeList< N >::GetNext( int32_t idx ) const
{
	if( idx < 0 )
	{
		return -1;
	}
	for( uint32_t i = idx + 1; i < m_pool.Length(); i++ )
	{
		if( !m_pool[ i ].next )
		{
			return (int32_t)i;
		}
	}
	return -1;
}

template< uint32_t N >
bool FreeList< N >::IsAllocated( int32_t idx ) const
{
	if( idx < 0 )
	{
		return false;
	}
#if _AE_DEBUG_
	AE_ASSERT( (uint32_t)idx < m_pool.Length() );
#endif
	return !m_pool[ idx ].next;
}

template< uint32_t N >
bool FreeList< N >::HasFree() const
{
	return m_free;
}

template< uint32_t N >
uint32_t FreeList< N >::Length() const
{
	return m_length;
}

//------------------------------------------------------------------------------
// ae::ObjectPool member functions
//------------------------------------------------------------------------------
template< typename T, uint32_t N, bool Paged >
ObjectPool< T, N, Paged >::ObjectPool()
{
	AE_STATIC_ASSERT_MSG( !Paged, "Paged ae::ObjectPool requires an allocation tag" );
	m_pages.Append( m_firstPage.Get()->node );
}

template< typename T, uint32_t N, bool Paged >
ObjectPool< T, N, Paged >::ObjectPool( const ae::Tag& tag )
	: m_tag( tag )
{
	AE_STATIC_ASSERT_MSG( Paged, "Static ae::ObjectPool does not need an allocation tag" );
	AE_ASSERT( m_tag != ae::Tag() );
}

template< typename T, uint32_t N, bool Paged >
ObjectPool< T, N, Paged >::~ObjectPool()
{
	AE_ASSERT( Length() == 0 );
}

template< typename T, uint32_t N, bool Paged >
template< typename ... Args >
T* ObjectPool< T, N, Paged >::New( Args&& ... args )
{
	Page* page = m_pages.FindFn( []( const Page* page ) { return page->freeList.HasFree(); } );
	if( Paged && !page )
	{
		page = ae::New< Page >( m_tag );
		m_pages.Append( page->node );
	}
	if( page )
	{
		int32_t index = page->freeList.Allocate();
		if( index >= 0 )
		{
			m_length++;
			return new ( &page->objects[ index ] ) T( std::forward< Args >( args ) ... );
		}
	}
	return nullptr;
}

template< typename T, uint32_t N, bool Paged >
void ObjectPool< T, N, Paged >::Delete( T* obj )
{
	if( !obj ) { return; }
	if( (intptr_t)obj % alignof(T) != 0 ) { return; } // @TODO: Should this be an assert?

	int32_t index;
	Page* page = m_pages.GetFirst();
	while( page )
	{
		index = (int32_t)( obj - (const T*)page->objects );
		if( 0 <= index && index < N )
		{
			break;
		}
		page = page->node.GetNext();
	}
	if( !Paged || page )
	{
		AE_DEBUG_ASSERT( (T*)&page->objects[ index ] == obj );
		AE_DEBUG_ASSERT_MSG( page->freeList.IsAllocated( index ), "Can't Delete() previously deleted object" );
		obj->~T();
#if _AE_DEBUG_
		memset( (void*)obj, 0xDD, sizeof(*obj) ); // Cast to silence clang vtable warning
#endif
		page->freeList.Free( index );
		m_length--;

		if( Paged && page->freeList.Length() == 0 )
		{
			ae::Delete( page );
		}
	}
}

template< typename T, uint32_t N, bool Paged >
void ObjectPool< T, N, Paged >::DeleteAll()
{
	auto deleteAllFn = []( Page* page )
	{
		for( uint32_t i = 0; i < N; i++ )
		{
			if( page->freeList.IsAllocated( i ) )
			{
				( (T*)&page->objects[ i ] )->~T(); // @TODO: Skip this for basic types
			}
		}
		page->freeList.FreeAll();
	};
	if( Paged )
	{
		Page* page = m_pages.GetLast();
		while( page )
		{
			Page* prev = page->node.GetPrev();
			deleteAllFn( page );
			ae::Delete( page );
			page = prev;
		}
	}
	else
	{
		deleteAllFn( m_firstPage.Get() );
	}
	m_length = 0;
}

template< typename T, uint32_t N, bool Paged >
const T* ObjectPool< T, N, Paged >::GetFirst() const
{
	if( Paged )
	{
		const Page* page = m_pages.GetFirst();
		if( page )
		{
			AE_ASSERT( page->freeList.Length() );
			return page->freeList.Length() ? (const T*)&page->objects[ page->freeList.GetFirst() ] : nullptr;
		}
	}
	else if( !Paged && m_length )
	{
		int32_t index = m_firstPage.Get()->freeList.GetFirst();
		AE_ASSERT( index >= 0 );
		return (const T*)&m_firstPage.Get()->objects[ index ];
	}
	AE_ASSERT( m_length == 0 );
	return nullptr;
}

template< typename T, uint32_t N, bool Paged >
const T* ObjectPool< T, N, Paged >::GetNext( const T* obj ) const
{
	if( !obj ) { return nullptr; }
	const Page* page = m_pages.GetFirst();
	while( page )
	{
		AE_ASSERT( !Paged || page->freeList.Length() );
		int32_t index = (int32_t)( obj - (const T*)page->objects );
		bool foundPage = ( 0 <= index && index < N );
		if( foundPage )
		{
			AE_ASSERT( (const T*)&page->objects[ index ] == obj );
			AE_ASSERT_MSG( page->freeList.IsAllocated( index ), "Can't GetNext() with previously deleted object" );
			int32_t next = page->freeList.GetNext( index );
			if( next >= 0 )
			{
				return (const T*)&page->objects[ next ];
			}
		}
		page = page->node.GetNext();
		if( foundPage && page )
		{
			// Given object is last element of previous page
			int32_t next = page->freeList.GetFirst();
			AE_ASSERT( 0 <= next && next < N );
			return (const T*)&page->objects[ next ];
		}
	}
	return nullptr;
}

template< typename T, uint32_t N, bool Paged >
T* ObjectPool< T, N, Paged >::GetFirst()
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N, Paged >* >( this )->GetFirst() );
}

template< typename T, uint32_t N, bool Paged >
T* ObjectPool< T, N, Paged >::GetNext( T* obj )
{
	return const_cast< T* >( const_cast< const ObjectPool< T, N, Paged >* >( this )->GetNext( obj ) );
}

template< typename T, uint32_t N, bool Paged >
bool ObjectPool< T, N, Paged >::HasFree() const
{
	return Length() < Size();
}

template< typename T, uint32_t N, bool Paged >
uint32_t ObjectPool< T, N, Paged >::Length() const
{
	return m_length;
}

template< typename T, uint32_t N, bool Paged >
typename ObjectPool< T, N, Paged >::template Iterator< T > ObjectPool< T, N, Paged >::begin()
{
	return Iterator< T >( this, m_pages.GetFirst(), GetFirst() );
}

template< typename T, uint32_t N, bool Paged >
typename ObjectPool< T, N, Paged >::template Iterator< T > ObjectPool< T, N, Paged >::end()
{
	return begin().end();
}

template< typename T, uint32_t N, bool Paged >
typename ObjectPool< T, N, Paged >::template Iterator< const T > ObjectPool< T, N, Paged >::begin() const
{
	return Iterator< const T >( this, m_pages.GetFirst(), GetFirst() );
}

template< typename T, uint32_t N, bool Paged >
typename ObjectPool< T, N, Paged >::template Iterator< const T > ObjectPool< T, N, Paged >::end() const
{
	return begin().end();
}

//------------------------------------------------------------------------------
// ae::ObjectPool::Iterator member functions
//------------------------------------------------------------------------------
template< typename T, uint32_t N, bool Paged >
template< typename T2 >
ObjectPool< T, N, Paged >::Iterator< T2 >::Iterator( const ObjectPool* pool, const struct Page* page, pointer ptr ) :
	m_pool( pool ),
	m_page( page ),
	m_ptr( ptr )
{}

template< typename T, uint32_t N, bool Paged >
template< typename T2 >
typename ObjectPool< T, N, Paged >::template Iterator< T2 >& ObjectPool< T, N, Paged >::Iterator< T2 >::operator++()
{
	if( m_pool )
	{
		m_ptr = const_cast< T* >( m_pool->GetNext( m_ptr ) ); // @TODO: More efficient m_GetNext( m_page, m_ptr )
		if( !m_ptr )
		{
			*this = end();
		}
	}
	return *this;
}

template< typename T, uint32_t N, bool Paged >
template< typename T2 >
typename ObjectPool< T, N, Paged >::template Iterator< T2 > ObjectPool< T, N, Paged >::Iterator< T2 >::operator++( int )
{
	Iterator result = *this;
	++(*this);
	return result;
}

template< typename T, uint32_t N, bool Paged >
template< typename T2 >
typename ObjectPool< T, N, Paged >::template Iterator< T2 > ObjectPool< T, N, Paged >::Iterator< T2 >::begin()
{
	return m_pool ? const_cast< ObjectPool* >( m_pool )->begin() : Iterator< T2 >();
}

template< typename T, uint32_t N, bool Paged >
template< typename T2 >
typename ObjectPool< T, N, Paged >::template Iterator< T2 > ObjectPool< T, N, Paged >::Iterator< T2 >::end()
{
	if( const Page* lastPage = ( m_pool ? m_pool->m_pages.GetLast() : nullptr ) )
	{
		T* endPtr = const_cast< T* >( reinterpret_cast< const T* >( &lastPage->objects[ N ] ) );
		return Iterator< T2 >( m_pool, lastPage, endPtr );
	}
	return Iterator< T2 >();
}

//------------------------------------------------------------------------------
// ae::OpaquePool member functions
//------------------------------------------------------------------------------
template< typename T, typename ... Args >
T* OpaquePool::New( Args&& ... args )
{
	AE_DEBUG_ASSERT( sizeof( T ) == m_objectSize );
	AE_DEBUG_ASSERT( alignof( T ) == m_objectAlignment );
	void* obj = Allocate();
	if( obj )
	{
		return new( obj ) T( std::forward< Args >( args ) ... );
	}
	return nullptr;
}

template< typename T >
void OpaquePool::Delete( T* obj )
{
	AE_DEBUG_ASSERT( sizeof( T ) == m_objectSize );
	AE_DEBUG_ASSERT( alignof( T ) == m_objectAlignment );
	if( obj )
	{
		obj->~T();
		Free( obj );
	}
}

template< typename T >
void OpaquePool::DeleteAll()
{
	AE_DEBUG_ASSERT( sizeof( T ) == m_objectSize );
	AE_DEBUG_ASSERT( alignof( T ) == m_objectAlignment );
	for( T& p : Iterate< T >() )
	{
		p.~T();
	}
	FreeAll();
}

template< typename T >
OpaquePool::Iterator< T > OpaquePool::Iterate()
{
	AE_DEBUG_ASSERT_MSG( m_objectSize >= sizeof( T ), "Object size does not match the initial configuration of this ae::OpaquePool: (# >= #)", m_objectSize, sizeof(T) );
	AE_DEBUG_ASSERT_MSG( m_objectAlignment >= alignof( T ), "Object alignment does not match the initial configuration of this ae::OpaquePool: (# >= #)", m_objectAlignment, alignof(T) );
	return Iterator< T >( this, m_pages.GetFirst(), (T*)m_GetFirst(), m_seq );
}

template< typename T >
OpaquePool::Iterator< const T > OpaquePool::Iterate() const
{
	AE_DEBUG_ASSERT_MSG( m_objectSize >= sizeof( T ), "Object size does not match the initial configuration of this ae::OpaquePool: (# >= #)", m_objectSize, sizeof(T) );
	AE_DEBUG_ASSERT_MSG( m_objectAlignment >= alignof( T ), "Object alignment does not match the initial configuration of this ae::OpaquePool: (# >= #)", m_objectAlignment, alignof(T) );
	return Iterator< const T >( this, m_pages.GetFirst(), (const T*)m_GetFirst(), m_seq );
}

//------------------------------------------------------------------------------
// ae::OpaquePool::Iterator member functions
//------------------------------------------------------------------------------
template< typename T >
OpaquePool::Iterator< T >::Iterator( const OpaquePool* pool, const struct Page* page, pointer ptr, uint32_t seq ) :
	m_ptr( ptr ),
	m_page( page ),
	m_pool( pool ),
	m_seq( seq )
{}

template< typename T >
OpaquePool::Iterator< T >& OpaquePool::Iterator< T >::operator++()
{
	if( m_pool )
	{
		m_ptr = (T*)m_pool->m_GetNext( m_page, m_ptr, m_seq );
		if( !m_ptr )
		{
			*this = end();
		}
	}
	return *this;
}

template< typename T >
OpaquePool::Iterator< T > OpaquePool::Iterator< T >::operator++( int )
{
	Iterator< T > result = *this;
	++(*this);
	return result;
}

template< typename T >
OpaquePool::Iterator< T > OpaquePool::Iterator< T >::begin()
{
	return m_pool ? const_cast< OpaquePool* >( m_pool )->Iterate< T >() : Iterator< T >();
}

template< typename T >
OpaquePool::Iterator< T > OpaquePool::Iterator< T >::end()
{
	if( const Page* lastPage = ( m_pool ? m_pool->m_pages.GetLast() : nullptr ) )
	{
		// Special values for iterator end, nullptr object and page. This allows
		// pages to be added and the end() pointer to remain the same.
		return Iterator< T >( m_pool, nullptr, nullptr, m_seq );
	}
	return Iterator< T >();
}

//------------------------------------------------------------------------------
// ae::BVH member functions
//------------------------------------------------------------------------------
template< typename T, uint32_t N >
BVH< T, N >::BVH() :
	m_limit( N )
{}

template< typename T, uint32_t N >
BVH< T, N >::BVH( const ae::Tag& allocTag ) :
	m_limit( 0 ),
	m_nodes( allocTag ),
	m_leaves( allocTag )
{}

template< typename T, uint32_t N >
BVH< T, N >::BVH( const ae::Tag& allocTag, uint32_t nodeLimit ) :
	m_limit( nodeLimit ),
	m_nodes( allocTag, nodeLimit ),
	m_leaves( allocTag, (nodeLimit + 1)/2 )
{}

template< typename T, uint32_t N >
BVH< T, N >::BVH( const BVH< T, N >& other ) :
	m_limit( other.m_limit ),
	m_nodes( other.m_nodes.Tag(), m_limit ),
	m_leaves( other.m_leaves.Tag(), (m_limit + 1)/2 )
{
	m_nodes = other.m_nodes;
	m_leaves = other.m_leaves;
}

template< typename T, uint32_t N >
BVH< T, N >& BVH< T, N >::operator = ( const BVH< T, N >& other )
{
	m_limit = other.m_limit;
	m_nodes.Clear();
	m_leaves.Clear();
	m_nodes.Reserve( m_limit );
	m_leaves.Reserve( (m_limit + 1)/2 );
	m_nodes = other.m_nodes;
	m_leaves = other.m_leaves;
	return *this;
}

template< typename T, uint32_t N >
template< typename AABBFn >
void BVH< T, N >::Build( T* data, uint32_t count, AABBFn aabbFn, uint32_t targetLeafCount )
{
	Clear();
	if( count )
	{
		AE_ASSERT_MSG( data, "Non-zero count provided with null data param" );
		ae::AABB rootAABB;
		for( uint32_t i = 0; i < count; i++ )
		{
			rootAABB.Expand( ae::AABB( aabbFn( data[ i ] ) ) );
		}
		m_nodes.Append( {} ).aabb = rootAABB;
		m_Build( data, count, aabbFn, targetLeafCount, 0, GetAvailable() );
	}
}

template< typename T, uint32_t N >
template< typename AABBFn >
void BVH< T, N >::m_Build( T* data, uint32_t count, AABBFn aabbFn, uint32_t targetLeafCount, int32_t bvhNodeIdx, uint32_t availableNodes )
{
	AE_DEBUG_ASSERT( !GetLimit() || ( GetAvailable() >= availableNodes ) );
	AE_DEBUG_ASSERT( count );
	if( count <= targetLeafCount )
	{
		SetLeaf( bvhNodeIdx, data, count );
		return;
	}
	
	const ae::AABB bvhNodeAABB = GetNode( bvhNodeIdx )->aabb;
	ae::Vec3 splitAxis( 0.0f );
	ae::Vec3 halfSize = bvhNodeAABB.GetHalfSize();
	if( halfSize.x > halfSize.y && halfSize.x > halfSize.z ) { splitAxis = ae::Vec3( 1.0f, 0.0f, 0.0f ); }
	else if( halfSize.y > halfSize.z ) { splitAxis = ae::Vec3( 0.0f, 1.0f, 0.0f ); }
	else { splitAxis = ae::Vec3( 0.0f, 0.0f, 1.0f ); }
	ae::Plane splitPlane( bvhNodeAABB.GetCenter(), splitAxis );
	
	ae::AABB leftBoundary;
	ae::AABB rightBoundary;
	T* middle = std::partition( data, data + count, [splitPlane, &leftBoundary, &rightBoundary, &aabbFn]( const T& t )
	{
		ae::AABB temp( aabbFn( t ) );
		ae::Vec3 aabbCenter = temp.GetCenter();
		if( splitPlane.GetSignedDistance( aabbCenter ) < 0.0f )
		{
			leftBoundary.Expand( temp );
			return true;
		}
		else
		{
			rightBoundary.Expand( temp );
			return false;
		}
	});
	uint32_t leftCount = (uint32_t)( middle - data );
	uint32_t rightCount = (uint32_t)( ( data + count ) - middle );

	if( !leftCount || !rightCount )
	{
		SetLeaf( bvhNodeIdx, data, count );
		return;
	}

	auto childIndices = AddNodes( bvhNodeIdx, leftBoundary, rightBoundary );
	if( availableNodes )
	{
		AE_DEBUG_ASSERT( GetLimit() );
		availableNodes -= 2;
		AE_DEBUG_ASSERT( leftCount && rightCount );
		float leftWeight = availableNodes * ( leftCount / (float)count );
		float rightWeight = availableNodes * ( rightCount / (float)count );
		uint32_t leftNodes = ae::Round( leftWeight );
		uint32_t rightNodes = ( availableNodes - leftNodes );
		if( leftNodes < 2 || rightNodes < 2 )
		{
			if( leftWeight < rightWeight )
			{
				leftNodes = 0;
				rightNodes = availableNodes;
			}
			else
			{
				leftNodes = availableNodes;
				rightNodes = 0;
			}
		}
		else if( ( leftNodes % 2 ) && ( rightNodes % 2 ) )
		{
			// Give node to bigger side if both have an odd number
			if( leftWeight > rightWeight ) { leftNodes++; rightNodes--; }
			else { leftNodes--; rightNodes++; }
		}
		AE_DEBUG_ASSERT( leftNodes + rightNodes == availableNodes );
		AE_DEBUG_ASSERT( availableNodes <= GetAvailable() );

		if( leftNodes >= 2 )
		{
			m_Build( data, leftCount, aabbFn, targetLeafCount, childIndices.first, leftNodes );
		}
		else
		{
			SetLeaf( childIndices.first, data, leftCount );
		}
		
		if( rightNodes >= 2 )
		{
			m_Build( middle, rightCount, aabbFn, targetLeafCount, childIndices.second, rightNodes );
		}
		else
		{
			SetLeaf( childIndices.second, middle, rightCount );
		}
	}
	else
	{
		AE_DEBUG_ASSERT( !GetLimit() );
		m_Build( data, leftCount, aabbFn, targetLeafCount, childIndices.first, 0 );
		m_Build( middle, rightCount, aabbFn, targetLeafCount, childIndices.second, 0 );
	}
}

template< typename T, uint32_t N >
std::pair< int32_t, int32_t > BVH< T, N >::AddNodes( int32_t parentIdx, const ae::AABB& leftAABB, const ae::AABB& rightAABB )
{
	if( !m_nodes.Length() )
	{
		m_nodes.Append( {} ); // Create root
	}
#if _AE_DEBUG_ && ( N == 0 )
	if( m_limit )
	{
		AE_ASSERT( m_nodes.Size() >= m_limit );
	}
	auto* preCheck = m_nodes.Data();
#endif

	BVHNode* parent = &m_nodes[ parentIdx ];
	AE_ASSERT( parent->leftIdx == -1 && parent->rightIdx == -1 );
	parent->leftIdx = (int16_t)m_nodes.Length();
	parent->rightIdx = (int16_t)( m_nodes.Length() + 1 );
	parent->aabb = leftAABB;
	parent->aabb.Expand( rightAABB );

	m_nodes.Append( {} );
	m_nodes.Append( {} );
	int32_t leftIdx = m_nodes.Length() - 2;
	int32_t rightIdx = m_nodes.Length() - 1;
	BVHNode* left = &m_nodes[ leftIdx ];
	BVHNode* right = &m_nodes[ rightIdx ];
	
	left->aabb = leftAABB;
	left->parentIdx = (int16_t)parentIdx;
	right->aabb = rightAABB;
	right->parentIdx = (int16_t)parentIdx;

#if _AE_DEBUG_ && ( N == 0 )
	if( m_limit )
	{
		AE_ASSERT( preCheck == m_nodes.Data() );
	}
#endif
	
	return { leftIdx, rightIdx };
}

template< typename T, uint32_t N >
void BVH< T, N >::SetLeaf( int32_t nodeIdx, T* data, uint32_t count )
{
	BVHLeaf< T >* leaf;
	BVHNode* node = &m_nodes[ nodeIdx ];
	if( node->leafIdx >= 0 )
	{
		leaf = &m_leaves[ node->leafIdx ];
	}
	else
	{
		node->leafIdx = (int16_t)m_leaves.Length();
		leaf = &m_leaves.Append( {} );
	}
	leaf->data = data;
	leaf->count = count;
	// @TODO: Return leaf?
}

template< typename T, uint32_t N >
void BVH< T, N >::Clear()
{
	m_nodes.Clear();
	m_leaves.Clear();
}

template< typename T, uint32_t N >
const BVHNode* BVH< T, N >::GetRoot() const
{
	return m_nodes.Length() ? GetNode( 0 ) : nullptr;
}

template< typename T, uint32_t N >
const BVHNode* BVH< T, N >::GetNode( int32_t nodeIdx ) const
{
	return ( nodeIdx >= 0 ) ? &m_nodes[ nodeIdx ] : nullptr;
}

template< typename T, uint32_t N >
const BVHLeaf< T >& BVH< T, N >::GetLeaf( int32_t leafIdx ) const
{
	return m_leaves[ leafIdx ];
}

template< typename T, uint32_t N >
const BVHLeaf< T >* BVH< T, N >::TryGetLeaf( int32_t leafIdx ) const
{
	return ( leafIdx >= 0 ) ? &m_leaves[ leafIdx ] : nullptr;
}

template< typename T, uint32_t N >
ae::AABB BVH< T, N >::GetAABB() const
{
	return GetRoot()->aabb;
}

//------------------------------------------------------------------------------
// HotLoader member functions
//------------------------------------------------------------------------------
template< typename Fn, typename... Args >
decltype(auto) HotLoader::CallFn( const char* name, Args... args )
{
	AE_ASSERT_MSG( m_dylib, "No library loaded" );
	Fn fn = (Fn)m_fns.Get( name, nullptr );
	if( !fn ) { fn = (Fn)m_LoadFn( name ); }
	return fn( args... );
}

//------------------------------------------------------------------------------
// ae::CollisionMesh member functions
//------------------------------------------------------------------------------
template< uint32_t V, uint32_t T, uint32_t B >
CollisionMesh< V, T, B >::CollisionMesh()
{
	AE_STATIC_ASSERT_MSG( V > 0 && T > 0 && B > 0, "ae::CollisionMesh does not support partial dynamic allocation" );
}

template< uint32_t V, uint32_t T, uint32_t B >
CollisionMesh< V, T, B >::CollisionMesh( ae::Tag tag ) :
	m_tag( tag ),
	m_positions( tag ),
	m_collisionExtras( tag ),
	m_tris( tag ),
	m_bvh( tag )
{}

template< uint32_t V, uint32_t T, uint32_t B >
void CollisionMesh< V, T, B >::Reserve( uint32_t vertCount, uint32_t triCount, uint32_t bvhNodeCount )
{
	AE_DEBUG_ASSERT( m_positions.Length() == m_collisionExtras.Length() );
	if( m_positions.Size() < vertCount || m_tris.Size() < triCount || m_bvh.GetLimit() < bvhNodeCount )
	{
		m_positions.Reserve( vertCount );
		m_collisionExtras.Reserve( vertCount );
		m_tris.Reserve( triCount );
		m_bvh = std::move( ae::BVH< BVHTri, B >( m_tag, bvhNodeCount ) ); // Clear bvh because pointers into m_tris could be invalid after Reserve()
		m_requiresRebuild = true;
	}
}

template< uint32_t V, uint32_t T, uint32_t B >
void CollisionMesh< V, T, B >::AddIndexed( const AddIndexedParams& params )
{
	AE_STATIC_ASSERT( sizeof(BVHTri) == sizeof(uint32_t) * 3 ); // Safe to cast BVHTri's to a uint32_t array
	AE_ASSERT_MSG( params.vertexPositionStride >= sizeof(float) * 3, "Must specify the number of bytes between each position" );
	AE_ASSERT( params.indexSize == 1 || params.indexSize == 2 || params.indexSize == 4 );
	AE_ASSERT_MSG( params.vertexCount || !params.indexCount, "Mesh indices supplied without vertex data" );
	if( !params.vertexPositions || !params.vertexCount || !params.indices || !params.indexCount )
	{
		return;
	}
	AE_ASSERT( params.indexCount % 3 == 0 );

	const bool identityTransform = ( params.transform == ae::Matrix4::Identity() );
	AE_DEBUG_ASSERT( m_positions.Length() == m_collisionExtras.Length() );
	const uint32_t initialVertexCount = m_positions.Length();
	const uint32_t triCount = params.indexCount / 3;
	
	m_positions.Reserve( initialVertexCount + params.vertexCount );
	m_collisionExtras.Reserve( initialVertexCount + params.vertexCount );
	for( uint32_t i = 0; i < params.vertexCount; i++ )
	{
		ae::Vec3 pos( (const float*)( (const uint8_t*)params.vertexPositions + params.vertexPositionStride * i ) );
		CollisionExtra extra = params.vertexExtras ? *(const CollisionExtra*)( (const uint8_t*)params.vertexExtras + params.vertexExtraStride * i ) : CollisionExtra();
		if( !identityTransform )
		{
			pos = ( params.transform * ae::Vec4( pos, 1.0f ) ).GetXYZ();
		}
		m_aabb.Expand( pos ); // Expand root aabb before calling m_BuildBVH() for the first partition
		m_positions.Append( pos );
		m_collisionExtras.Append( extra );
	}
	
	m_tris.Reserve( m_tris.Length() + triCount );
	// clang-format off
#define COPY_INDICES( intType )\
	BVHTri tri;\
	const intType* indices = (const intType*)params.indices;\
	for( uint32_t i = 0; i < triCount; i++ )\
	{\
		for( uint32_t j = 0; j < 3; j++ )\
		{\
			uint32_t idx = (uint32_t)indices[ i * 3 + j ];\
			AE_DEBUG_ASSERT_MSG( idx < params.vertexCount, "Index out of bounds: # Vertex count: #", idx, params.vertexCount );\
			tri.idx[ j ] = initialVertexCount + idx;\
		}\
		m_tris.Append( tri );\
	}
	if( params.indexSize == 8 ) { COPY_INDICES( uint64_t ); }
	else if( params.indexSize == 4 ) { COPY_INDICES( uint32_t ); }
	else if( params.indexSize == 2 ) { COPY_INDICES( uint16_t ); }
	else if( params.indexSize == 1 ) { COPY_INDICES( uint8_t ); }
	else { AE_FAIL_MSG( "Invalid index size" ); }
#undef COPY_INDICES
	// clang-format on

	m_requiresRebuild = true;
}

template< uint32_t V, uint32_t T, uint32_t B >
void CollisionMesh< V, T, B >::AddIndexed( ae::Matrix4 transform, const float* positions, uint32_t positionCount, uint32_t positionStride, const void* indices, uint32_t indexCount, uint32_t indexSize )
{
	AddIndexedParams params;
	params.transform = transform;
	params.vertexPositions = positions;
	params.vertexPositionStride = positionStride;
	params.vertexCount = positionCount;
	params.indices = indices;
	params.indexCount = indexCount;
	params.indexSize = indexSize;
	AddIndexed( params );
}

template< uint32_t V, uint32_t T, uint32_t B >
void CollisionMesh< V, T, B >::BuildBVH()
{
	if( m_requiresRebuild )
	{
		AE_DEBUG_ASSERT( m_positions.Length() );
		AE_DEBUG_ASSERT( m_tris.Length() );
		const ae::Vec3* verts = m_positions.begin();
		auto aabbFn = [verts]( BVHTri tri )
		{
			ae::AABB aabb;
			aabb.Expand( verts[ tri.idx[ 0 ] ] );
			aabb.Expand( verts[ tri.idx[ 1 ] ] );
			aabb.Expand( verts[ tri.idx[ 2 ] ] );
			return aabb;
		};
		m_bvh.Build( m_tris.begin(), m_tris.Length(), aabbFn, 32 );
		m_requiresRebuild = false;
	}
}

template< uint32_t V, uint32_t T, uint32_t B >
void CollisionMesh< V, T, B >::Clear()
{
	m_aabb = ae::AABB();
	m_requiresRebuild = false;
	m_positions.Clear();
	m_collisionExtras.Clear();
	m_tris.Clear();
	m_bvh.Clear();
}

template< uint32_t V, uint32_t T, uint32_t B >
RaycastResult CollisionMesh< V, T, B >::Raycast( const RaycastParams& params, const RaycastResult& prevResult ) const
{
	// Early out for parameters that will give no results
	if( params.maxHits == 0 || !m_bvh.GetRoot() )
	{
		return prevResult;
	}
	
	// Sphere/OBB check in world space
	{
		float t = 0.0f;
		
		// Sphere
		ae::Vec3 aabbMin = ( params.transform * ae::Vec4( m_aabb.GetMin(), 1.0f ) ).GetXYZ();
		ae::Vec3 aabbMax = ( params.transform * ae::Vec4( m_aabb.GetMax(), 1.0f ) ).GetXYZ();
		ae::Sphere sphere( ( aabbMin + aabbMax ) * 0.5f, ( aabbMax - aabbMin ).Length() * 0.5f );
		if( !sphere.IntersectRay( params.source, params.ray, nullptr, &t ) )
		{
			return prevResult; // Early out if ray doesn't touch sphere
		}
		else if( params.maxHits == prevResult.hits.Length() && prevResult.hits[ prevResult.hits.Length() - 1 ].distance < t )
		{
			return prevResult; // Early out if sphere is farther away than previous hits
		}
		
		// OBB
		ae::OBB obb( params.transform * m_aabb.GetTransform() );
		if( ae::DebugLines* debug = params.debug )
		{
			// Ray intersects obb
			debug->AddOBB( obb, params.debugColor );
		}
		if( !obb.IntersectRay( params.source, params.ray, nullptr, nullptr, &t ) )
		{
			if( ae::DebugLines* debug = params.debug )
			{
				debug->AddLine( params.source, params.source + params.ray, params.debugColor );
			}
			return prevResult; // Early out if ray doesn't touch obb
		}
		else if( params.maxHits == prevResult.hits.Length() && prevResult.hits[ prevResult.hits.Length() - 1 ].distance < t )
		{
			return prevResult; // Early out if obb is farther away than previous hits
		}
	}
	
	const ae::Matrix4 invTransform = params.transform.GetInverse();
	const ae::Matrix4 normalTransform = invTransform.GetTranspose();
	const ae::Vec3 source( invTransform * ae::Vec4( params.source, 1.0f ) );
	const ae::Vec3 rayEnd( invTransform * ae::Vec4( params.source + params.ray, 1.0f ) );
	const ae::Vec3 ray = rayEnd - source;
	const bool ccw = params.hitCounterclockwise;
	const bool cw = params.hitClockwise;
	
	RaycastResult result;
	uint32_t hitCount  = 0;
	RaycastResult::Hit hits[ result.hits.Size() + 1 ];
	const uint32_t maxHits = ae::Min( params.maxHits, result.hits.Size() );
	auto bvhFn = [&]( auto&& bvhFn, const ae::BVH< BVHTri, B >* bvh, const BVHNode* current ) -> void
	{
		if( !current->aabb.IntersectRay( source, ray ) )
		{
			return;
		}
		if( params.debug )
		{
			ae::OBB obb( params.transform * current->aabb.GetTransform() );
			params.debug->AddOBB( obb, params.debugColor );
		}
		if( const BVHLeaf< BVHTri >* leaf = bvh->TryGetLeaf( current->leafIdx ) )
		{
			for( uint32_t i = 0; i < leaf->count; i++ )
			{
				ae::Vec3 p, n;
				const uint32_t idx0 = leaf->data[ i ].idx[ 0 ];
				ae::Vec3 a = m_positions[ idx0 ];
				ae::Vec3 b = m_positions[ leaf->data[ i ].idx[ 1 ] ];
				ae::Vec3 c = m_positions[ leaf->data[ i ].idx[ 2 ] ];
				if( IntersectRayTriangle( source, ray, a, b, c, ccw, cw, &p, &n, nullptr ) )
				{
					RaycastResult::Hit& outHit = hits[ hitCount ];
					hitCount++;
					AE_ASSERT( hitCount <= maxHits + 1 ); // Allow one extra hit, then sort and remove last hit below

					// Undo local space transforms
					outHit.position = ae::Vec3( params.transform * ae::Vec4( p, 1.0f ) );
					outHit.normal = ae::Vec3( normalTransform * ae::Vec4( n, 0.0f ) );
					outHit.distance = ( outHit.position - params.source ).Length(); // Calculate here because transform might not have uniform scale
					outHit.userData = params.userData;
					outHit.extra = m_collisionExtras[ idx0 ];

					if( hitCount > maxHits )
					{
						std::sort( hits, hits + hitCount, []( const RaycastResult::Hit& a, const RaycastResult::Hit& b )
						{
							return a.distance < b.distance;
						});
						hitCount = maxHits;
					}
				}
			}
		}
		// @TODO: Depth-first here is not ideal. See Real-time Collision Detection: 6.3.1 Descent Rules
		// Improving this will require early out when max hits have been recorded
		// and pending search volumes are farther away than the farthest hit.
		if( const BVHNode* left = bvh->GetNode( current->leftIdx ) )
		{
			bvhFn( bvhFn, bvh, left );
		}
		if( const BVHNode* right = bvh->GetNode( current->rightIdx ) )
		{
			bvhFn( bvhFn, bvh, right );
		}
	};
	bvhFn( bvhFn, &m_bvh, m_bvh.GetRoot() );
	
	if( ae::DebugLines* debug = params.debug )
	{
		ae::Vec3 rayEnd = hitCount ? hits[ hitCount - 1 ].position : params.source + params.ray;
		debug->AddLine( params.source, rayEnd, params.debugColor );
		
		for( uint32_t i = 0; i < hitCount; i++ )
		{
			const RaycastResult::Hit* hit = &hits[ i ];
			const ae::Vec3 p = hit->position;
			const ae::Vec3 n = hit->normal;
			float s = ( hitCount > 1 ) ? ( i / ( hitCount - 1.0f ) ) : 1.0f;
			debug->AddCircle( p, n, ae::Lerp( 0.25f, 0.3f, s ), params.debugColor, 8 );
			debug->AddLine( p, p + n, params.debugColor );
		}
	}
	
	std::sort( hits, hits + hitCount, []( const RaycastResult::Hit& a, const RaycastResult::Hit& b ) { return a.distance < b.distance; } );
	for( uint32_t i = 0; i < hitCount; i++ )
	{
		hits[ i ].normal.SafeNormalize();
		result.hits.Append( hits[ i ] );
	}
	RaycastResult::Accumulate( params, prevResult, &result );
	return result;
}

template< uint32_t V, uint32_t T, uint32_t B >
PushOutInfo CollisionMesh< V, T, B >::PushOut( const PushOutParams& params, const PushOutInfo& prevInfo ) const
{
	if( !m_bvh.GetRoot() )
	{
		return prevInfo;
	}
	
	if( ae::DebugLines* debug = params.debug )
	{
		debug->AddSphere( prevInfo.sphere.center, prevInfo.sphere.radius, params.debugColor, 8 );
	}

	ae::OBB obb( params.transform * m_aabb.GetTransform() );
	if( obb.GetSignedDistanceFromSurface( prevInfo.sphere.center ) > prevInfo.sphere.radius )
	{
		return prevInfo; // Early out if sphere is to far from mesh
	}
	
	if( ae::DebugLines* debug = params.debug )
	{
		// Sphere intersects obb
		debug->AddOBB( obb, params.debugColor );
	}
	
	PushOutInfo result;
	result.sphere = prevInfo.sphere;
	result.velocity = prevInfo.velocity;
	const bool hasIdentityTransform = ( params.transform == ae::Matrix4::Identity() );
	
	auto bvhFn = [&]( auto&& bvhFn, const ae::BVH< BVHTri, B >* bvh, const BVHNode* current ) -> void
	{
		// AABB/OBB early out
		ae::AABB aabb = current->aabb;
		if( hasIdentityTransform )
		{
			if( aabb.GetSignedDistanceFromSurface( prevInfo.sphere.center ) > prevInfo.sphere.radius )
			{
				return;
			}
			if( params.debug )
			{
				params.debug->AddAABB( aabb.GetCenter(), aabb.GetHalfSize(), params.debugColor );
			}
		}
		else
		{
			ae::OBB obb( params.transform * aabb.GetTransform() );
			if( obb.GetSignedDistanceFromSurface( prevInfo.sphere.center ) > prevInfo.sphere.radius )
			{
				return;
			}
			if( params.debug )
			{
				params.debug->AddOBB( obb, params.debugColor );
			}
		}
		// Triangle checks
		if( const BVHLeaf< BVHTri >* leaf = bvh->TryGetLeaf( current->leafIdx ) )
		{
			for( uint32_t i = 0; i < leaf->count; i++ )
			{
				const uint32_t idx0 = leaf->data[ i ].idx[ 0 ];
				const CollisionExtra extra = m_collisionExtras[ idx0 ];
				ae::Vec3 a = m_positions[ idx0 ];
				ae::Vec3 b = m_positions[ leaf->data[ i ].idx[ 1 ] ];
				ae::Vec3 c = m_positions[ leaf->data[ i ].idx[ 2 ] ];
				if( !hasIdentityTransform )
				{
					a = ae::Vec3( params.transform * ae::Vec4( a, 1.0f ) );
					b = ae::Vec3( params.transform * ae::Vec4( b, 1.0f ) );
					c = ae::Vec3( params.transform * ae::Vec4( c, 1.0f ) );
				}
		
				const ae::Vec3 triNormal = ( ( b - a ).Cross( c - a ) ).SafeNormalizeCopy();
				const ae::Vec3 triCenter( ( a + b + c ) / 3.0f );
		
				ae::Vec3 triToSphereDir = ( result.sphere.center - triCenter );
				if( triNormal.Dot( triToSphereDir ) < 0.0f )
				{
					continue;
				}
		
				ae::Vec3 triHitPos;
				if( result.sphere.IntersectTriangle( a, b, c, &triHitPos ) )
				{
					triToSphereDir = ( result.sphere.center - triHitPos );
					if( triNormal.Dot( triToSphereDir ) < 0.0f )
					{
						continue;
					}
		
					ae::Vec3 closestSpherePoint = ( triHitPos - result.sphere.center ).SafeNormalizeCopy();
					closestSpherePoint *= result.sphere.radius;
					closestSpherePoint += result.sphere.center;
		
					result.sphere.center += triHitPos - closestSpherePoint;
					result.velocity.ZeroDirection( -triNormal );
		
					// @TODO: Sort. Shouldn't randomly discard hits.
					if( result.hits.Length() < result.hits.Size() )
					{
						ae::PushOutInfo::Hit& hitOut = result.hits.Append( {} );
						hitOut.position = triHitPos;
						hitOut.normal = triNormal;
						hitOut.extra = extra;
					}
		
					if( ae::DebugLines* debug = params.debug )
					{
						debug->AddLine( a, b, params.debugColor );
						debug->AddLine( b, c, params.debugColor );
						debug->AddLine( c, a, params.debugColor );
		
						debug->AddLine( triHitPos, triHitPos + triNormal * 2.0f, params.debugColor );
						debug->AddSphere( triHitPos, 0.05f, params.debugColor, 4 );
					}
				}
			}
		}
		// @TODO: Depth-first here is not ideal. See Real-time Collision Detection: 6.3.1 Descent Rules
		if( const BVHNode* left = bvh->GetNode( current->leftIdx ) )
		{
			bvhFn( bvhFn, bvh, left );
		}
		if( const BVHNode* right = bvh->GetNode( current->rightIdx ) )
		{
			bvhFn( bvhFn, bvh, right );
		}
	};
	bvhFn( bvhFn, &m_bvh, m_bvh.GetRoot() );
	
	if( result.hits.Length() )
	{
		PushOutInfo::Accumulate( params, prevInfo, &result );
		return result;
	}
	else
	{
		return prevInfo;
	}
}

//------------------------------------------------------------------------------
// ae::AStar implementation
//------------------------------------------------------------------------------
template< typename T >
uint32_t AStar( const T* _startNode,
	const T* _nodes, uint32_t nodeCount,
	const T** _goalNodes, uint32_t goalCount,
	const T** pathOut, uint32_t pathOutMax )
{
	if( !_startNode || !nodeCount || !goalCount )
	{
		return 0;
	}

	struct Node
	{
		Node* prev;
		Node* nextOpen;
		Node* prevOpen;
		float g; // Current cost of traversal from start to this node
		float f; // Estimated total cost of traversal from start node to goal
		bool closed;
		bool isGoal;
		const T* userData;
	};

	ae::Scratch< Node > nodeBuffer( nodeCount );
	ae::Scratch< Node* > goalBuffer( nodeCount );
	Node* nodes = nodeBuffer.Data();
	Node** goals = goalBuffer.Data();
	memset( nodes, 0, sizeof(*nodes) * nodeCount );
	memset( goals, 0, sizeof(*goals) * nodeCount );
	for( uint32_t i = 0; i < nodeCount; i++ )
	{
		nodes[ i ].userData = &_nodes[ i ];
	}

	// Estimates the cost to reach goal from node
	auto GetFScore = [ goals, goalCount ]( const Node* from ) -> float
	{
		float h = INFINITY;
		for( uint32_t i = 0; i < goalCount; i++ )
		{
			h = ae::Min( h, from->userData->GetHeuristic( goals[ i ]->userData ) );
		}
		return from->g + h;
	};

	for( uint32_t i = 0; i < goalCount; i++ )
	{
		const uint32_t goalIndex = (uint32_t)( _goalNodes[ i ] - _nodes );
		AE_ASSERT( goalIndex < nodeCount );
		goals[ i ] = &nodes[ goalIndex ];
		goals[ i ]->isGoal = true;
	}
	
	AE_ASSERT( ( _startNode - _nodes ) < nodeCount );
	Node* startNode = &nodes[ _startNode - _nodes ];
	startNode->f = GetFScore( startNode );

	Node* openSet = startNode;
	Node* current = nullptr;
	while( openSet ) // While open set is not empty
	{
		current = [ openSet ]()
		{
			Node* result = openSet;
			for( Node* node = openSet; node; node = node->nextOpen )
			{
				if( node->f == result->f )
				{
					result = ( node < result ) ? node : result;
				}
				else
				{
					result = ( node->f < result->f ) ? node : result;
				}
			}
			return result;
		}();
		if( current->isGoal )
		{
			break;
		}

		// Process current and remove from open set
		if( openSet == current ) { openSet = current->nextOpen; }
		if( current->nextOpen ) { current->nextOpen->prevOpen = current->prevOpen; }
		if( current->prevOpen ) { current->prevOpen->nextOpen = current->nextOpen; }
		current->nextOpen = nullptr;
		current->prevOpen = nullptr;
		// Add current to closed set
		current->closed = true;

		for( uint32_t i = 0; i < current->userData->GetNextCount(); i++ )
		{
			const T* nextNode = current->userData->GetNext( i );
			if( !nextNode )
			{
				continue;
			}
			const uint32_t neighborIndex = (uint32_t)( nextNode - _nodes );
			AE_ASSERT( neighborIndex < nodeCount );
			Node* neighbor = &nodes[ neighborIndex ];
			if( neighbor->closed )
			{
				continue;
			}

			const float g = current->g + current->userData->GetHeuristic( neighbor->userData );
			if( neighbor != openSet && !neighbor->nextOpen && !neighbor->prevOpen )
			{
				neighbor->prev = current;
				neighbor->g = g;
				neighbor->f = GetFScore( neighbor );
				if( openSet )
				{
					neighbor->nextOpen = openSet;
					neighbor->nextOpen->prevOpen = neighbor;
				}
				openSet = neighbor;
			}
			else if( g < neighbor->g )
			{
				neighbor->prev = current;
				neighbor->g = g;
				neighbor->f = GetFScore( neighbor );
			}
		}
	}

	AE_ASSERT( current );
	if( current->isGoal )
	{
		const uint32_t pathLength = [ current, startNode ]()
		{
			uint32_t length = 0;
			for( Node* iter = current; iter; iter = iter->prev )
			{
				length++;
			}
			return length;
		}();

		const uint32_t result = ae::Min( pathLength, pathOutMax );
		uint32_t i = ( result - 1 );
		for( Node* iter = current; iter; iter = iter->prev )
		{
			if( i < result )
			{
				pathOut[ i ] = iter->userData;
			}
			i--;
		}
		return result;
	}
	
	return 0;
}

//------------------------------------------------------------------------------
// ae::OBJLoader member functions
//------------------------------------------------------------------------------
template< uint32_t V, uint32_t T, uint32_t B >
void OBJLoader::InitializeCollisionMesh( ae::CollisionMesh< V, T, B >* mesh )
{
	if( !mesh )
	{
		return;
	}

	mesh->Clear();
	mesh->AddIndexed(
		ae::Matrix4::Identity(),
		vertices[ 0 ].position.data,
		vertices.Length(),
		sizeof( Vertex ),
		indices.Data(),
		indices.Length(),
		sizeof( uint32_t )
	);
	mesh->BuildBVH();
}

//------------------------------------------------------------------------------
// ae::VertexArray member functions
//------------------------------------------------------------------------------
template<> const void* VertexArray::GetVertices() const;
template<> const void* VertexArray::GetIndices() const;

template< typename T >
const T* VertexArray::GetVertices() const
{
	AE_ASSERT( m_buffer.GetVertexSize() == sizeof( T ) );
	return static_cast< const T* >( m_vertexReadable );
}

template< typename T >
const T* VertexArray::GetIndices() const
{
	AE_ASSERT( m_buffer.GetIndexSize() == sizeof( T ) );
	return static_cast< const T* >( m_indexReadable );
}

//------------------------------------------------------------------------------
// ae::BinaryStream member functions
//------------------------------------------------------------------------------
template< typename T >
void BinaryStream::SetUserData( T* userData )
{
	m_userData = const_cast< void* >( reinterpret_cast< const void* >( userData ) );
	m_userDataTypeId = std::is_const_v< T > ? ae::kInvalidTypeId : ae::GetTypeIdWithQualifiers< T >(); // Don't allow removal of const with GetUserData()
	m_userDataConstTypeId = ae::GetTypeIdWithQualifiers< const T >();
}
template< typename T >
T* BinaryStream::GetUserData()
{
	const ae::TypeId returnTypeId = ae::GetTypeIdWithQualifiers< T >();
	if( m_userDataTypeId == returnTypeId )
	{
		return reinterpret_cast< T* >( m_userData );
	}
	else if( m_userDataConstTypeId == returnTypeId )
	{
		return reinterpret_cast< T* >( m_userData );
	}
	return nullptr;
}

template< typename E >
void BinaryStream::SerializeEnum( E& v )
{
	AE_STATIC_ASSERT( std::is_enum< E >::value );
	SerializeRaw( &v, sizeof( E ) );
}

template< typename E >
void BinaryWriter::SerializeEnum( const E& v )
{
	AE_STATIC_ASSERT( std::is_enum< E >::value );
	SerializeRaw( &v, sizeof( E ) );
}

template< uint32_t N >
void BinaryStream::SerializeString( Str< N >& strInOut )
{
	if( ae::BinaryReader* reader = AsReader() )
	{
		if( auto end = (const uint8_t*)memchr( reader->PeekReadData(), '\0', GetRemainingBytes() ) )
		{
			const uint32_t strLength = (uint32_t)( end - reader->PeekReadData() );
			if( strLength <= strInOut.MaxLength() )
			{
				strInOut = Str< N >( strLength, (const char*)reader->PeekReadData() );
				reader->DiscardReadData( strLength + 1 );
			}
			else
			{
				Invalidate();
			}
		}
		else
		{
			Invalidate();
		}
	}
	else if( ae::BinaryWriter* writer = AsWriter() )
	{
		writer->SerializeRaw( strInOut.c_str(), strInOut.Length() + 1 );
	}
}

template< uint32_t N >
void BinaryWriter::SerializeString( const Str< N >& strIn )
{
	SerializeRaw( strIn.c_str(), strIn.Length() + 1 );
}

template< class C >
struct HasSerializeMethod
{
	template< class T > static std::true_type testSignature(void (T::*)( ae::BinaryStream* ));
	template< class T > static decltype(testSignature(&T::Serialize)) test(std::nullptr_t);
	template< class T > static std::false_type test(...);
	static const bool value = decltype(test<C>(nullptr))::value;
};

template< class C >
struct HasConstSerializeMethod
{
	template< class T > static std::true_type testSignature(void (T::*)( ae::BinaryWriter* ) const);
	template< class T > static decltype(testSignature(&T::Serialize)) test(std::nullptr_t);
	template< class T > static std::false_type test(...);
	static const bool value = decltype(test<C>(nullptr))::value;
};

template< typename T >
typename std::enable_if< HasSerializeMethod< T >::value >::type
BinaryStream_SerializeObjectInternal( ae::BinaryStream* stream, T& v )
{
	v.Serialize( stream );
}

template< typename T >
typename std::enable_if< !HasSerializeMethod< T >::value && HasConstSerializeMethod< T >::value >::type
BinaryStream_SerializeObjectInternal( ae::BinaryWriter* stream, T& v )
{
	v.Serialize( stream );
}

template< typename T >
typename std::enable_if< !HasSerializeMethod< T >::value && !HasConstSerializeMethod< T >::value >::type
BinaryStream_SerializeObjectInternal( ae::BinaryStream* stream, T& v, ... )
{
	Serialize( stream, &v );
}

template< typename T >
typename std::enable_if< HasConstSerializeMethod< T >::value >::type
BinaryStream_SerializeObjectInternalConst( ae::BinaryWriter* stream, const T& v )
{
	v.Serialize( stream );
}

template< typename T >
typename std::enable_if< !HasConstSerializeMethod< T >::value >::type
BinaryStream_SerializeObjectInternalConst( ae::BinaryWriter* stream, const T& v, ... )
{
	Serialize( stream, &v );
}

template< typename T >
void BinaryWriter::SerializeObject( T& v )
{
	BinaryStream_SerializeObjectInternal< T >( this, v );
}

template< typename T >
void BinaryStream::SerializeObject( T& v )
{
	BinaryStream_SerializeObjectInternal< T >( this, v );
}

template< typename T >
void BinaryWriter::SerializeObject( const T& v )
{
	BinaryStream_SerializeObjectInternalConst< T >( this, v );
}

//------------------------------------------------------------------------------
// Internal helpers
//------------------------------------------------------------------------------
template< typename T > ae::Object* _PlacementNew( ae::Object* d ) { return new( d ) T(); }
#define AE_EVAL(...) __VA_ARGS__
#define AE_STRINGIFY_IMPL(S) #S
#define AE_VA_ARGS_COUNT_IMPL(_,_9,_8,_7,_6,_5,_4,_3,_2,X_,...) X_
#define AE_GLUE_IMPL(X,Y) AE_GLUE_IMPL_IMPL(X,Y)
#define AE_GLUE_IMPL_IMPL(X,Y) X##Y
#define AE_GLUE_1(X) X
#define AE_GLUE_2(X,Y) X##Y
#define AE_GLUE_3(X,Y,Z) X##Y##Z
#define AE_GLUE_4(X,Y,Z,W) X##Y##Z##W
#define AE_GLUE_5(X,Y,Z,W,V) X##Y##Z##W##V
#define AE_GLUE_6(X,Y,Z,W,V,U) X##Y##Z##W##V##U
#define AE_GLUE_7(X,Y,Z,W,V,U,T) X##Y##Z##W##V##U##T
#define AE_GLUE_8(X,Y,Z,W,V,U,T,S) X##Y##Z##W##V##U##T##S
#define AE_GLUE_9(X,Y,Z,W,V,U,T,S,R) X##Y##Z##W##V##U##T##S##R
#define AE_GLUE_10(X,Y,Z,W,V,U,T,S,R,Q) X##Y##Z##W##V##U##T##S##R##Q
#define AE_GLUE_TYPE_1(T) T
#define AE_GLUE_TYPE_2(N0, T) N0::T
#define AE_GLUE_TYPE_3(N0, N1, T) N0::N1::T
#define AE_GLUE_TYPE_4(N0, N1, N2, T) N0::N1::N2::T
#define AE_GLUE_TYPE_5(N0, N1, N2, N3, T) N0::N1::N2::N3::T
#define AE_GLUE_TYPE_6(N0, N1, N2, N3, N4, T) N0::N1::N2::N3::N4::T
#define AE_GLUE_TYPE_7(N0, N1, N2, N3, N4, N5, T) N0::N1::N2::N3::N4::N5::T
#define AE_GLUE_TYPE_8(N0, N1, N2, N3, N4, N5, N6, T) N0::N1::N2::N3::N4::N5::N6::T
#define AE_GLUE_TYPE_9(N0, N1, N2, N3, N4, N5, N6, N7, T) N0::N1::N2::N3::N4::N5::N6::N7::T
#define AE_GLUE_TYPE_10(N0, N1, N2, N3, N4, N5, N6, N7, N8, T) N0::N1::N2::N3::N4::N5::N6::N7::N8::T
#define AE_GLUE_UNDERSCORE_1(X) X
#define AE_GLUE_UNDERSCORE_2(X,Y) X##_##Y
#define AE_GLUE_UNDERSCORE_3(X,Y,Z) X##_##Y##_##Z
#define AE_GLUE_UNDERSCORE_4(X,Y,Z,W) X##_##Y##_##Z##_##W
#define AE_GLUE_UNDERSCORE_5(X,Y,Z,W,V) X##_##Y##_##Z##_##W##_##V
#define AE_GLUE_UNDERSCORE_6(X,Y,Z,W,V,U) X##_##Y##_##Z##_##W##_##V##_##U
#define AE_GLUE_UNDERSCORE_7(X,Y,Z,W,V,U,T) X##_##Y##_##Z##_##W##_##V##_##U##_##T
#define AE_GLUE_UNDERSCORE_8(X,Y,Z,W,V,U,T,S) X##_##Y##_##Z##_##W##_##V##_##U##_##T##_##S
#define AE_GLUE_UNDERSCORE_9(X,Y,Z,W,V,U,T,S,R) X##_##Y##_##Z##_##W##_##V##_##U##_##T##_##S##_##R
#define AE_GLUE_UNDERSCORE_10(X,Y,Z,W,V,U,T,S,R,Q) X##_##Y##_##Z##_##W##_##V##_##U##_##T##_##S##_##R##_##Q
#define AE_GET_ELEM_0(_0, ...) _0
#define AE_GET_ELEM_1(_0, _1, ...) _1
#define AE_GET_ELEM_2(_0, _1, _2, ...) _2
#define AE_GET_ELEM_3(_0, _1, _2, _3, ...) _3
#define AE_GET_ELEM_4(_0, _1, _2, _3, _4, ...) _4
#define AE_GET_ELEM_5(_0, _1, _2, _3, _4, _5, ...) _5
#define AE_GET_ELEM_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define AE_GET_ELEM_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define AE_GET_ELEM_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define AE_GET_ELEM_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define AE_GET_ELEM_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
#define AE_DROP_LAST_1(_1)
#define AE_DROP_LAST_2(_1,_2) _1
#define AE_DROP_LAST_3(_1,_2,_3) _1,_2
#define AE_DROP_LAST_4(_1,_2,_3,_4) _1,_2,_3
#define AE_DROP_LAST_5(_1,_2,_3,_4,_5) _1,_2,_3,_4
#define AE_DROP_LAST_6(_1,_2,_3,_4,_5,_6) _1,_2,_3,_4,_5
#define AE_DROP_LAST_7(_1,_2,_3,_4,_5,_6,_7) _1,_2,_3,_4,_5,_6
#define AE_DROP_LAST_8(_1,_2,_3,_4,_5,_6,_7,_8) _1,_2,_3,_4,_5,_6,_7
#define AE_DROP_LAST_9(_1,_2,_3,_4,_5,_6,_7,_8,_9) _1,_2,_3,_4,_5,_6,_7,_8
#define AE_DROP_LAST_10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10) _1,_2,_3,_4,_5,_6,_7,_8,_9

//------------------------------------------------------------------------------
// Deprecated meta class property registration macros
//------------------------------------------------------------------------------
//! Register a class property
//! Call signature: AE_REGISTER_CLASS_PROPERTY( (Namespace0, ..., NameSpaceN, MyType), typeProperty );
#define AE_REGISTER_NAMESPACECLASS_PROPERTY( _C, _P ) AE_REGISTER_CLASS_PROPERTY_IMPL( AE_GLUE_UNDERSCORE _C, AE_GLUE_TYPE _C, _P )
#define AE_REGISTER_CLASS_PROPERTY( _C, _P ) AE_REGISTER_NAMESPACECLASS_PROPERTY( (_C), _P )

//! Register a class property with an additional value. Multiple values can be
//! specified per property.
//! Call signature: AE_REGISTER_CLASS_PROPERTY_VALUE( (Namespace0, ..., NameSpaceN, MyType), typeProperty, typePropertyValue );
#define AE_REGISTER_NAMESPACECLASS_PROPERTY_VALUE( _C, _P, _V ) AE_REGISTER_CLASS_PROPERTY_VALUE_IMPL( AE_GLUE_UNDERSCORE _C, AE_GLUE_TYPE _C, _P, _V )
#define AE_REGISTER_CLASS_PROPERTY_VALUE( _C, _P, _V ) AE_REGISTER_NAMESPACECLASS_PROPERTY_VALUE( (_C), _P, _V )

//! Register a property for a specific class variable
#define AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY( _C, _V, _P ) AE_REGISTER_CLASS_VAR_PROPERTY_IMPL( AE_GLUE_UNDERSCORE _C, AE_GLUE_TYPE _C, _V, _P )
#define AE_REGISTER_CLASS_VAR_PROPERTY( _C, _V, _P ) AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY( (_C), _V, _P )

//! Register a property for a specific class variable with an additional value.
//! Multiple values can be specified per property.
#define AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY_VALUE( _C, _V, _P, _PV ) AE_REGISTER_CLASS_VAR_PROPERTY_VALUE_IMPL( AE_GLUE_UNDERSCORE _C, AE_GLUE_TYPE _C, _V, _P, _PV )
#define AE_REGISTER_CLASS_VAR_PROPERTY_VALUE( _C, _V, _P, _PV ) AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY_VALUE( (_C), _V, _P, _PV )

//------------------------------------------------------------------------------
// Internal meta class registration macros
//------------------------------------------------------------------------------
#define AE_REGISTER_CLASS_IMPL( _N, _T )\
	int AE_GLUE_UNDERSCORE(_ae_force_link, _N) = 0;\
	template<> const char* ae::_TypeName< ::_T >::Get() { return AE_STRINGIFY(_T); }\
	template<> struct ae::TypeT< ::_T > : public ae::_ClassTypeT< ::_T > { TypeT() : _ClassTypeT< ::_T >( AE_STRINGIFY(_T) ) {} };\
	ae::_TypeCreator< ::_T > AE_GLUE_UNDERSCORE(_ae_type_creator, _N);\
	template<> ae::Type* ae::FindMetaRegistrationFor< ::_T >() { return ae::TypeT< ::_T >::Get(); }\
	static ae::SourceFileAttribute AE_GLUE_UNDERSCORE(ae_attrib, _N, SourceFileAttribute) { .path=_AE_SRCCHK(__FILE__,""), .line=_AE_SRCCHK(__LINE__, 0) }; static ae::_AttributeCreator< ::_T > AE_GLUE_UNDERSCORE(ae_attrib_creator, _N, SourceFileAttribute)( AE_GLUE_UNDERSCORE(_ae_type_creator, _N), _AE_SRCCHK(&AE_GLUE_UNDERSCORE(ae_attrib, _N, SourceFileAttribute), nullptr) );

#define AE_REGISTER_CLASS_PROPERTY_IMPL( _N, _T, _P ) static ae::_PropCreator< ::_T > AE_GLUE_UNDERSCORE(ae_prop_creator, _N , _P)( AE_GLUE_UNDERSCORE(_ae_type_creator, _N), #_T, #_P, "" )

#define AE_REGISTER_CLASS_PROPERTY_VALUE_IMPL( _N, _T, _P, _V ) static ae::_PropCreator< ::_T > AE_GLUE_UNDERSCORE(ae_prop_creator, _N, _P, _V)( AE_GLUE_UNDERSCORE(_ae_type_creator, _N), #_T, #_P, #_V )

#define AE_REGISTER_CLASS_VAR_IMPL( _N, _T, _V )\
	AE_DISABLE_INVALID_OFFSET_WARNING\
	static ae::_VarCreator< ::_T, decltype(::_T::_V), offsetof( ::_T, _V ) > AE_GLUE_UNDERSCORE(ae_var_creator, _N, _V)( AE_GLUE_UNDERSCORE(_ae_type_creator, _N), #_T, #_V );\
	static ae::SourceFileAttribute AE_GLUE_UNDERSCORE(ae_attrib, _N, _V, SourceFileAttribute) { .path=_AE_SRCCHK(__FILE__,""), .line=_AE_SRCCHK(__LINE__, 0) }; static ae::_AttributeCreator< ::_T > AE_GLUE_UNDERSCORE(ae_attrib_creator, _N, _V, SourceFileAttribute)( AE_GLUE_UNDERSCORE(ae_var_creator, _N, _V), _AE_SRCCHK(&AE_GLUE_UNDERSCORE(ae_attrib, _N, _V, SourceFileAttribute), nullptr) );\
	AE_ENABLE_INVALID_OFFSET_WARNING

#define AE_REGISTER_CLASS_VAR_PROPERTY_IMPL( _N, _T, _V, _P )\
	AE_DISABLE_INVALID_OFFSET_WARNING\
	static ae::_VarPropCreator< ::_T, decltype(::_T::_V), offsetof( ::_T, _V ) > AE_GLUE_UNDERSCORE(ae_var_prop_creator, _N, _V, _P)( AE_GLUE_UNDERSCORE(ae_var_creator, _N, _V), #_P, "" );\
	AE_ENABLE_INVALID_OFFSET_WARNING

#define AE_REGISTER_CLASS_VAR_PROPERTY_VALUE_IMPL( _N, _T, _V, _P, _PV )\
	AE_DISABLE_INVALID_OFFSET_WARNING\
	static ae::_VarPropCreator< ::_T, decltype(::_T::_V), offsetof( ::_T, _V ) > AE_GLUE_UNDERSCORE(ae_var_prop_creator, _N, _V, _P, _PV)( AE_GLUE_UNDERSCORE(ae_var_creator, _N, _V), #_P, #_PV );\
	AE_ENABLE_INVALID_OFFSET_WARNING

#define AE_REGISTER_CLASS_ATTRIBUTE_IMPL( _N, _T, _AN, _AT, _ARGS )\
	static ::_AT AE_GLUE_UNDERSCORE(ae_attrib, _N, _AN, __LINE__) _ARGS;\
	static ae::_AttributeCreator< ::_T > AE_GLUE_UNDERSCORE(ae_attrib_creator, _N, _AN, __LINE__)( AE_GLUE_UNDERSCORE(_ae_type_creator, _N), &AE_GLUE_UNDERSCORE(ae_attrib, _N, _AN, __LINE__) )

#define AE_REGISTER_CLASS_VAR_ATTRIBUTE_IMPL( _N, _T, _V, _AN, _AT, _ARGS )\
	static ::_AT AE_GLUE_UNDERSCORE(ae_attrib, _N, _V, _AN, __LINE__) _ARGS;\
	static ae::_AttributeCreator< ::_T > AE_GLUE_UNDERSCORE(ae_attrib_creator, _N, _V, _AN,__LINE__)( AE_GLUE_UNDERSCORE(ae_var_creator, _N, _V), &AE_GLUE_UNDERSCORE(ae_attrib, _N, _V, _AN, __LINE__) )

//------------------------------------------------------------------------------
// External meta initialization helpers
//------------------------------------------------------------------------------
// Forward declaration for TypeT< T >::Get()
template< typename T > ae::Type* FindMetaRegistrationFor();
// Proxy for the real TypeT< T > if its definition is not available. This
// version of TypeT< T >::Get() will be called when the real one is not
// included (like for meta registered classes). Calls will instead fallback to
// FindMetaRegistrationFor< T >() where the linker will have searched the
// compilation units for the real TypeT< T >::Get() implementation. This is
// required because meta registered classes are registered within their own
// compilation units so they can be hotloaded and have registered vars and
// attributes.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// AN UNEXPECTED LINKER ERROR HERE LIKELY MEANS THAT YOU HAVE NOT INCLUDED THE
// REAL IMPLEMENTATION OF THE ae::TypeT<> CLASS FOR THE TYPE YOU ARE USING.
// TRY INCLUDING THE FILE THAT REGISTERS THE TYPE YOU ARE USING. SEE BELOW.
/* Undefined symbol:
		"ae::Type* ae::FindMetaRegistrationFor< MyCustomArray< int > >()"
		Referenced from Foo.cpp

// Try including the header with the definition of this in Foo.cpp:
	template< typename T >
	ae::TypeT< MyCustomArray< T > > : ae::ArrayType
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
template< typename T > struct TypeT
{
	// Param with default value so real TypeT< T >::Get() will be prioritized if
	// available. Using a templated class with a static member function here is
	// needed to work around ODR (One Definition Rule) for type definitions. Its
	// possible for a member function of a a templated class to be static, but
	// its not possible for a free templated function to be static. Generated
	// templated types like TypeT< T* > (implicitly defined in headers) need
	// storage for their ae::Type, and a static member function of a templated
	// class (with a fall back to FindMetaRegistrationFor<> for classes and
	// enums) is the best solution, where one instance will be compiled into
	// each compilation unit.
	// @TODO: Use const instead of discarding it
	static ae::Type* Get( uint32_t _i = 0 ) { return ae::FindMetaRegistrationFor< std::remove_const_t< T > >(); }
};

template< typename T > ae::TypeId GetTypeIdWithoutQualifiers()
{
	return ae::GetTypeIdWithQualifiers< ae::RemoveTypeQualifiers< T > >();
}

template< typename T > ae::TypeId GetTypeIdWithQualifiers()
{
	return ae::GetTypeIdFromName( ae::_GetTypeName< T >().data() );
}

template< typename T >
struct _TypeName
{
	// @TODO: Can this be replaced by ae::GetTypeName< T >()?
	static const char* Get();
};

//------------------------------------------------------------------------------
// Internal meta (Inheritor)
//------------------------------------------------------------------------------
template< typename Parent, typename This >
Inheritor< Parent, This >::Inheritor()
{
	// @NOTE: Don't get type here because this object could be constructed
	// before meta types are constructed.
	this->_metaTypeId = ae::GetTypeIdFromName( ae::_TypeName< This >::Get() );
}

template< typename Parent, typename This >
const char* Inheritor< Parent, This >::GetParentTypeName()
{
	return ae::_TypeName< Parent >::Get();
}

template< typename Parent, typename This >
const ae::ClassType* Inheritor< Parent, This >::GetParentType()
{
	return ae::GetClassTypeByName( ae::_TypeName< Parent >::Get() );
}

//------------------------------------------------------------------------------
// Internal meta initialization functions
//------------------------------------------------------------------------------
// @NOTE: Internal. Non-specialized GetEnumType() has no implementation so templated GetEnumType() calls (defined
// with AE_DEFINE_ENUM_CLASS, AE_META_ENUM, and AE_META_ENUM_PREFIX) will call the specialized function.
template< typename T >
const ae::EnumType* GetEnumType();
	
template< typename E >
class _RegisterEnum
{
public:
	using T = typename std::underlying_type_t< E >;
	_RegisterEnum( const char* typeName, std::string strMap )
	{
		ae::_Globals* globals = ae::_Globals::Get();
		ae::EnumType* enumType = const_cast< ae::EnumType* >( ae::TypeT< E >::Get()->template AsVarType< ae::EnumType >() );
		globals->enumTypes.Set( ae::GetTypeIdWithoutQualifiers< E >(), enumType );
			
		// Remove whitespace
		strMap.erase( std::remove( strMap.begin(), strMap.end(), ' ' ), strMap.end() );
		strMap.erase( std::remove( strMap.begin(), strMap.end(), '\t' ), strMap.end() );
		
		// Remove comments
		for( std::size_t s0 = strMap.find( "/*" ); s0 != std::string::npos; s0 = strMap.find( "/*" ) )
		{
			std::size_t s1 = strMap.find( "*/", s0 + 2 );
			AE_ASSERT( s1 != std::string::npos );
			s1 += 2;
			strMap.erase( s0, s1 - s0 );
		}
		for( std::size_t s0 = strMap.find( "//" ); s0 != std::string::npos; s0 = strMap.find( "//" ) )
		{
			std::size_t s1 = strMap.find( "\n", s0 + 2 );
			if( s1 == std::string::npos ) { s1 = strMap.length(); }
			strMap.erase( s0, s1 - s0 );
		}
		
		// Remove new lines (after comments)
		strMap.erase( std::remove( strMap.begin(), strMap.end(), '\n' ), strMap.end() );

		T currentValue = 0;
		std::vector< std::string > enumTokens( m_SplitString( strMap, ',' ) );
		for( auto iter = enumTokens.begin(); iter != enumTokens.end(); ++iter )
		{
			std::string enumName;
			if( iter->find( '=' ) == std::string::npos )
			{
				enumName = *iter;
			}
			else
			{
				std::vector<std::string> enumNameValue( m_SplitString( *iter, '=' ) );
				enumName = enumNameValue[ 0 ];
				if( std::is_unsigned< T >::value )
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
		globals->metaCacheSeq++;
	}

	~_RegisterEnum()
	{
		ae::_Globals* globals = ae::_Globals::Get();
		globals->enumTypes.Remove( ae::GetTypeIdWithoutQualifiers< E >() );
		globals->metaCacheSeq++;
	}
		
private:
	static std::vector< std::string > m_SplitString( std::string str, char separator )
	{
		std::vector< std::string > result;

		std::string item;
		std::stringstream stringStream( str );
		while( std::getline( stringStream, item, separator ) )
		{
			result.push_back( item );
		}

		return result;
	}
};
	
template< typename E >
class _RegisterExistingEnumOrValue
{
public:
	// Enum types
	_RegisterExistingEnumOrValue()
	{
		ae::_Globals* globals = ae::_Globals::Get();
		globals->enumTypes.Set( ae::GetTypeIdWithoutQualifiers< E >(), ae::TypeT< E >::Get()->template AsVarType< ae::EnumType >() );
		globals->metaCacheSeq++;
	}
	~_RegisterExistingEnumOrValue()
	{
		ae::_Globals* globals = ae::_Globals::Get();
		globals->enumTypes.Remove( ae::GetTypeIdWithoutQualifiers< E >() );
		globals->metaCacheSeq++;
	}
	
	// Enum values
	_RegisterExistingEnumOrValue( const char* valueName, E value )
	{
		ae::EnumType* enumType = const_cast< ae::EnumType* >( ae::TypeT< E >::Get()->template AsVarType< ae::EnumType >() );
		AE_ASSERT( enumType );
		const char* prefix = enumType->GetPrefix();
		const uint32_t prefixLen = (uint32_t)strlen( prefix );
		AE_ASSERT( prefixLen < strlen( valueName ) );
		AE_ASSERT( memcmp( prefix, valueName, prefixLen ) == 0 );
		enumType->m_AddValue( valueName + prefixLen, (int32_t)value );
		ae::_Globals::Get()->metaCacheSeq++;
	}
};

} // ae end

//------------------------------------------------------------------------------
// Internal meta var registration
//------------------------------------------------------------------------------
#define _ae_DefineBasicVarType( T, e )\
template<>\
struct ae::TypeT< T > : public ae::BasicType {\
	ae::BasicType::Type GetType() const override { return ae::BasicType::e; }\
	uint32_t GetSize() const override { return sizeof(T); }\
	static ae::Type* Get() { static ae::TypeT< T > s_type; return &s_type; }\
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< T >(); }\
};

// @TODO: Split into C++ basic types and aether basic types?
_ae_DefineBasicVarType( uint8_t, UInt8 );
_ae_DefineBasicVarType( uint16_t, UInt16 );
_ae_DefineBasicVarType( uint32_t, UInt32 );
_ae_DefineBasicVarType( uint64_t, UInt64 );
_ae_DefineBasicVarType( int8_t, Int8 );
_ae_DefineBasicVarType( int16_t, Int16 );
_ae_DefineBasicVarType( int32_t, Int32 );
_ae_DefineBasicVarType( int64_t, Int64 );
_ae_DefineBasicVarType( ae::Int2, Int2 );
_ae_DefineBasicVarType( ae::Int3, Int3 );
_ae_DefineBasicVarType( bool, Bool );
_ae_DefineBasicVarType( float, Float );
_ae_DefineBasicVarType( double, Double );
_ae_DefineBasicVarType( ae::Vec2, Vec2 );
_ae_DefineBasicVarType( ae::Vec3, Vec3 );
_ae_DefineBasicVarType( ae::Vec4, Vec4 );
_ae_DefineBasicVarType( ae::Color, Color );
_ae_DefineBasicVarType( ae::Matrix4, Matrix4 );
_ae_DefineBasicVarType( ae::Str16, String );
_ae_DefineBasicVarType( ae::Str32, String );
_ae_DefineBasicVarType( ae::Str64, String );
_ae_DefineBasicVarType( ae::Str128, String );
_ae_DefineBasicVarType( ae::Str256, String );
_ae_DefineBasicVarType( ae::Str512, String );

namespace ae {

//------------------------------------------------------------------------------
// ae::PointerType templated member functions
//------------------------------------------------------------------------------
template< typename T >
T** ae::PointerType::GetRef( ae::DataPointer varData ) const
{
	if( GetInnerVarType().IsSameExactVarType< T >() )
	{
		return static_cast< T** >( varData.Get( this ) );
	}
	return nullptr;
}

template< typename T >
T*const* ae::PointerType::GetRef( ae::ConstDataPointer varData ) const
{
	if( GetInnerVarType().IsSameExactVarType< T >() )
	{
		return static_cast< T*const* >( varData.Get( this ) );
	}
	return nullptr;
}

template< typename T >
struct TypeT< T* > : public ae::PointerType
{
	static_assert( std::is_base_of< ae::Object, T >::value, "T must be derived from ae::Object" );

	const ae::Type& GetInnerVarType() const override { return *ae::TypeT< T >::Get(); }
	static ae::Type* Get() { static ae::TypeT< T* > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< T* >(); }

	bool SetRef( ae::DataPointer _varData, ae::Object* value ) const override
	{
		if( T** varData = static_cast< T** >( _varData.Get( this ) ) )
		{
			if( !value )
			{
				*varData = nullptr;
				return true;
			}
			else
			{
				const ae::ClassType* varType = GetClassType< T >();
				const ae::ClassType* valueType = ae::GetClassTypeFromObject( value );
				if( varType && valueType && valueType->IsType( varType ) )
				{
					*varData = static_cast< T* >( value );
					return true;
				}
			}
		}
		return false;
	}

	bool SetRefFromString( ae::DataPointer _varData, const char* value, StringToObjectPointerFn fn, const void* userData ) const override
	{
		if( T** varData = static_cast< T** >( _varData.Get( this ) ) )
		{
			ae::Object* obj = nullptr;
			if( fn && fn( userData, value, &obj ) )
			{
				if( !obj )
				{
					// When fn return true and null, clear pointer value
					*varData = nullptr;
					return true;
				}
				const ae::ClassType* varType = GetClassType< T >();
				const ae::ClassType* valueType = ae::GetClassTypeFromObject( obj );
				if( varType && valueType && valueType->IsType( varType ) )
				{
					*varData = static_cast< T* >( obj );
					return true;
				}
			}
		}
		return false;
	}

	std::string GetStringFromRef( ae::ConstDataPointer _varData, ObjectPointerToStringFn fn, const void* userData ) const override
	{
		if( fn )
		{
			// @TODO: Type compatibility checks
			if( T* const* varData = static_cast< T* const* >( _varData.Get( this ) ) )
			{
				return fn( userData, *varData );
			}
		}
		return "";
	}
};
template<>
struct TypeT< std::nullptr_t > : public ae::PointerType
{
	const ae::Type& GetInnerVarType() const override { AE_FAIL(); return *Get(); } // @TODO: Must return something, add Void type
	static ae::Type* Get() { static ae::TypeT< std::nullptr_t > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< std::nullptr_t >(); }
	bool SetRef( ae::DataPointer varData, ae::Object* value ) const override { AE_FAIL(); return false; }
	bool SetRefFromString( ae::DataPointer varData, const char* value, StringToObjectPointerFn fn, const void* userData ) const override { AE_FAIL(); return false; }
	std::string GetStringFromRef( ae::ConstDataPointer varData, ObjectPointerToStringFn fn, const void* userData ) const override { AE_FAIL(); return ""; }
};

template< typename T, uint32_t N >
class DynamicArrayVarType : public ae::ArrayType
{
public:
	typedef ae::Array< T, N > Array;

	ae::DataPointer GetElement( ae::DataPointer _array, uint32_t idx ) const override
	{
		Array* array = static_cast< Array* >( _array.Get( this ) );
		if( array && idx < array->Length() )
		{
			return { GetInnerVarType(), &array->operator[]( idx ) };
		}
		return {};
	}
	ae::ConstDataPointer GetElement( ae::ConstDataPointer _array, uint32_t idx ) const override
	{
		const Array* array = static_cast< const Array* >( _array.Get( this ) );
		if( array && idx < array->Length() )
		{
			return { GetInnerVarType(), &array->operator[]( idx ) };
		}
		return {};
	}
	uint32_t Resize( ae::DataPointer _array, uint32_t length ) const override
	{
		Array* array = static_cast< Array* >( _array.Get( this ) );
		if( !array )
		{
			return 0;
		}
		if( array->Length() < length )
		{
			array->Reserve( length );
			for( uint32_t i = array->Length(); i < length; i++ )
			{
				array->Append( {} );
			}
		}
		else if( length < array->Length() )
		{
			while( array->Length() > length )
			{
				array->Remove( array->Length() - 1 );
			}
		}
		return array->Length();
	}
	uint32_t GetLength( ae::ConstDataPointer _array ) const override
	{
		const Array* array = static_cast< const Array* >( _array.Get( this ) );
		return array ? array->Length() : 0;
	}
	uint32_t GetMaxLength() const override { return ( N == 0 ) ? ae::MaxValue< uint32_t >() : N; }
	uint32_t IsFixedLength() const override { return false; }
};

template< typename T, uint32_t N >
class StaticArrayVarType : public ae::ArrayType
{
public:
	ae::DataPointer GetElement( ae::DataPointer _array, uint32_t idx ) const override
	{
		T* array = static_cast< T* >( _array.Get( this ) );
		return { GetInnerVarType(), ( array && idx < N ) ? &array[ idx ] : nullptr };
	}
	ae::ConstDataPointer GetElement( ae::ConstDataPointer _array, uint32_t idx ) const override
	{
		const T* array = static_cast< const T* >( _array.Get( this ) );
		return { GetInnerVarType(), ( array && idx < N ) ? &array[ idx ] : nullptr };
	}
	uint32_t Resize( ae::DataPointer array, uint32_t length ) const override { return N; }
	uint32_t GetLength( ae::ConstDataPointer array ) const override { return N; }
	uint32_t GetMaxLength() const override { return N; }
	uint32_t IsFixedLength() const override { return true; }
};

//------------------------------------------------------------------------------
// ae::_ClassTypeT class
//------------------------------------------------------------------------------
template< typename T >
class _ClassTypeT : public ae::ClassType
{
public:
	_ClassTypeT( const char* typeName )
	{
		_Globals* globals = _Globals::Get();
		AE_ASSERT_MSG( globals->classTypes.Length() < globals->classTypes.Size(), "Set/increase AE_MAX_META_TYPES_CONFIG (Currently: #)", globals->classTypes.Size() );
		Init< T >( typeName );
		globals->classTypes.Set( GetId(), this ); // @TODO: Should check for hash collision
		globals->metaCacheSeq++;
	}
	~_ClassTypeT()
	{
		_Globals* globals = _Globals::Get();
		globals->classTypes.Remove( GetId() );
		globals->metaCacheSeq++;
	}
	ae::TypeId GetTypeId() const override { return ae::GetTypeIdWithoutQualifiers< T >(); }
	static ae::Type* Get() { static ae::TypeT< T > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< T >(); }
};

template< typename T >
struct _TypeCreator
{
	_TypeCreator() { Get(); }
	ae::ClassType* Get() { return static_cast< ae::ClassType* >( ae::TypeT< T >::Get() ); }
};

template< typename C >
struct _PropCreator
{
	// Take _TypeCreator param as a safety check that _PropCreator typeName is provided correctly
	_PropCreator( ae::_TypeCreator< C >& typeCreator, const char* typeName, const char* propName, const char* propValue )
	{
		typeCreator.Get()->m_AddProp( propName, propValue );
	}
};

template< typename C, typename V, uint32_t Offset >
struct _VarCreator
{
	// Take _TypeCreator param as a safety check that _VarCreator typeName is provided correctly
	_VarCreator( ae::_TypeCreator< C >& typeCreator, const char* typeName, const char* varName )
	{
		m_var.m_owner = typeCreator.Get();
		m_var.m_varType = *ae::TypeT< V >::Get();
		m_var.m_name = varName;
#if !_AE_WINDOWS_
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
		m_var.m_offset = Offset; // @TODO: Verify var is not member of base class
#if !_AE_WINDOWS_
	#pragma clang diagnostic pop
#endif
		typeCreator.Get()->m_AddVar( &m_var );
	}
	ae::ClassVar m_var;
};

template< typename C, typename V, uint32_t Offset >
struct _VarPropCreator
{
	// Take _VarCreator param as a safety check
	_VarPropCreator( ae::_VarCreator< C, V, Offset >& varCreator, const char* propName, const char* propValue )
	{
		varCreator.m_var.m_AddProp( propName, propValue );
	}
};

//------------------------------------------------------------------------------
// _AttributeCreator class
//------------------------------------------------------------------------------
template< typename T >
class _AttributeCreator
{
public:
	_AttributeCreator( ae::_TypeCreator< T >& typeCreator, ae::Attribute* attribute ) { if( attribute ){ typeCreator.Get()->attributes.m_Add( attribute ); } }
	template< typename T1, uint32_t T2 > _AttributeCreator( ae::_VarCreator< T, T1, T2 >& varCreator, ae::Attribute* attribute ) { if( attribute ){ varCreator.m_var.attributes.m_Add( attribute ); } }
	// _AttributeCreator( ae::_RegisterEnum< T >& creator, const ae::Attribute* attribute ) { creator.m_enum.attributes.m_Add( attribute ); }
	// @NOTE: No need to remove added attributes on hotload because they must be in the same compilation unit as types etc.
};

} // ae end

template< typename T, uint32_t N >
struct ae::TypeT< ae::Array< T, N > > : public ae::DynamicArrayVarType< T, N >
{
	const ae::Type& GetInnerVarType() const override { return *ae::TypeT< T >::Get(); }
	static ae::Type* Get() { static ae::TypeT< ae::Array< T, N > > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< ae::Array< T, N > >(); }
};

template< typename T, uint32_t N >
struct ae::TypeT< T[ N ] > : public ae::StaticArrayVarType< T, N >
{
	const ae::Type& GetInnerVarType() const override { return *ae::TypeT< T >::Get(); }
	static ae::Type* Get() { static ae::TypeT< T[ N ] > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< T[ N ] >(); }
};

//------------------------------------------------------------------------------
// ae::MapType template implementation
//------------------------------------------------------------------------------
template< typename K, typename V, uint32_t N, typename H >
struct ae::TypeT< ae::Map< K, V, N, H > > : public ae::MapType
{
	typedef ae::Map< K, V, N, H > MapType;
	static ae::Type* Get() { static ae::TypeT< MapType > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< MapType >(); }

	const ae::Type& GetKeyVarType() const override { return *ae::TypeT< K >::Get(); }
	const ae::Type& GetValueVarType() const override { return *ae::TypeT< V >::Get(); }

	ae::DataPointer Get( ae::DataPointer _map, ae::ConstDataPointer _key ) const override
	{
		if( MapType* map = static_cast< MapType* >( _map.Get( this ) ) )
		{
			if( const K* key = static_cast< const K* >( _key.Get( &GetKeyVarType() ) ) )
			{
				V* value = map->TryGet( *key );
				if( !value && ( ( N == 0 ) || ( map->Length() < map->Size() ) ) )
				{
					value = &map->Set( *key, {} );
				}
				return { GetValueVarType(), value };
			}
		}
		return {};
	}

	ae::DataPointer TryGet( ae::DataPointer _map, ae::ConstDataPointer _key ) const override
	{
		if( MapType* map = static_cast< MapType* >( _map.Get( this ) ) )
		{
			if( const K* key = static_cast< const K* >( _key.Get( &GetKeyVarType() ) ) )
			{
				return { GetValueVarType(), map->TryGet( *key ) };
			}
		}
		return {};
	}

	ae::ConstDataPointer TryGet( ae::ConstDataPointer _map, ae::ConstDataPointer _key ) const override
	{
		if( const MapType* map = static_cast< const MapType* >( _map.Get( this ) ) )
		{
			if( const K* key = static_cast< const K* >( _key.Get( &GetKeyVarType() ) ) )
			{
				return { GetValueVarType(), map->TryGet( *key ) };
			}
		}
		return {};
	}

	uint32_t GetLength( ae::ConstDataPointer _map ) const override
	{
		const MapType* map = static_cast< const MapType* >( _map.Get( this ) );
		return map ? map->Length() : 0;
	}

	ae::ConstDataPointer GetKey( ae::ConstDataPointer _map, uint32_t index ) const override
	{
		const MapType* map = static_cast< const MapType* >( _map.Get( this ) );
		if( map && map->Length() > index )
		{
			return { GetKeyVarType(), &map->GetKey( index ) };
		}
		return {};
	}

	ae::ConstDataPointer GetValue( ae::ConstDataPointer _map, uint32_t index ) const override
	{
		const MapType* map = static_cast< const MapType* >( _map.Get( this ) );
		if( map && map->Length() > index )
		{
			return { GetValueVarType(), &map->GetValue( index ) };
		}
		return {};
	}

	ae::DataPointer GetValue( ae::DataPointer _map, uint32_t index ) const override
	{
		MapType* map = static_cast< MapType* >( _map.Get( this ) );
		if( map && map->Length() > index )
		{
			return { GetValueVarType(), &map->GetValue( index ) };
		}
		return {};
	}

	uint32_t GetMaxLength() const override
	{
		return N ? N : ae::MaxValue< uint32_t >();
	}
};

template< typename T >
bool ae::ClassType::IsType() const
{
	const ae::ClassType* type = ::ae::GetClassType< T >();
	AE_ASSERT( type );
	return IsType( type );
}

template< typename T >
const ae::ClassType* ae::GetClassType()
{
	static _StaticCacheVar< const ae::ClassType* > s_type = nullptr;
	if( s_type )
	{
		return s_type;
	}
	else
	{
		_Globals* globals = _Globals::Get();
		// @TODO: Conditionally enable this check when T is not a forward declaration
		//AE_STATIC_ASSERT( (std::is_base_of< ae::Object, T >::value) );
		s_type = globals->classTypes.Get( ae::GetTypeIdWithoutQualifiers< T >(), nullptr );
		AE_ASSERT_MSG( s_type, "No meta info for type name: #", ae::GetTypeName< ae::RemoveTypeQualifiers< T > >() );
		return s_type;
	}
}

//------------------------------------------------------------------------------
// ae::EnumType templated member functions
//------------------------------------------------------------------------------
template< typename T >
std::string ae::EnumType::GetNameByValue( T value ) const
{
	return m_enumValueToName.Get( (int32_t)value, "" );
}

template< typename T >
bool ae::EnumType::GetValueFromString( const char* str, T* valueOut ) const
{
	if( !str )
	{
		return false;
	}
	int32_t value = 0;
	if( m_enumNameToValue.TryGet( str, &value ) ) // Set object var with named enum value
	{
		*valueOut = (T)value;
		return true;
	}
	else if( isdigit( str[ 0 ] ) || str[ 0 ] == '-' ) // Set object var with a numerical enum value
	{
		value = atoi( str );
		if( HasValue( value ) )
		{
			*valueOut = (T)value;
			return true;
		}
	}
	return false;
}

template< typename T >
T ae::EnumType::GetValueFromString( const char* str, T defaultValue ) const
{
	GetValueFromString( str, &defaultValue );
	return defaultValue;
}

template< typename T >
bool ae::EnumType::HasValue( T value ) const
{
	return m_enumValueToName.TryGet( value );
}

//------------------------------------------------------------------------------
// ae::DataPointer templated member functions
//------------------------------------------------------------------------------
template< typename T >
ae::DataPointer::DataPointer( T* data )
{
	m_data = data;
	m_varType = ae::TypeT< T >::Get();
	AE_ASSERT( m_varType );
}

//------------------------------------------------------------------------------
// ae::ConstDataPointer templated member functions
//------------------------------------------------------------------------------
template< typename T >
ae::ConstDataPointer::ConstDataPointer( const T* data )
{
	m_data = data;
	m_varType = ae::TypeT< T >::Get();
	AE_ASSERT( m_varType );
}

//------------------------------------------------------------------------------
// ae::Type templated member functions
//------------------------------------------------------------------------------
template< typename T >
const T* ae::Type::AsVarType() const
{
	return IsSameBaseVarType< T >() ? static_cast< const T* >( this ) : nullptr;
}

template< typename T >
bool ae::Type::IsSameBaseVarType() const
{
	using U = typename ae::RemoveTypeQualifiers< T >;
	if constexpr( std::is_same_v< ae::Type, U > )
	{
		return true;
	}
	else
	{
		static_assert( std::is_base_of_v< ae::Type, U >, "T must inherit from ae::Type" );
		return GetBaseVarTypeId() == ae::GetTypeIdWithoutQualifiers< U >();
	}
}

template< typename T >
bool ae::Type::IsSameExactVarType() const
{
	using TCheck = typename ae::RemoveTypeQualifiers< T >;
	static_assert( !std::is_same_v< ae::Type, TCheck > );
	static_assert( std::is_base_of_v< ae::Type, TCheck >, "T must inherit from ae::Type" );
	return GetExactVarTypeId() == ae::GetTypeIdWithQualifiers< T >();
}

//------------------------------------------------------------------------------
// ae::BasicType templated member functions
//------------------------------------------------------------------------------
template< typename T >
bool ae::BasicType::GetVarData( ae::ConstDataPointer _varData, T* valueOut ) const
{
	if( valueOut )
	{
		if( const void* varData = _varData.Get( this ) )
		{
			const ae::Type* varType = ae::TypeT< T >::Get();
			const ae::BasicType* basicVarType = varType ? varType->AsVarType< ae::BasicType >() : nullptr;
			if( basicVarType && basicVarType->GetType() == GetType() )
			{
				*valueOut = *static_cast< const T* >( varData );
				return true;
			}
		}
	}
	return false;
}

template< typename T >
bool ae::BasicType::SetVarData( ae::DataPointer _varData, const T& value ) const
{
	if( void* varData = _varData.Get( this ) )
	{
		const ae::Type* varType = ae::TypeT< T >::Get();
		const ae::BasicType* basicVarType = varType ? varType->AsVarType< ae::BasicType >() : nullptr;
		if( basicVarType && basicVarType->GetType() == GetType() )
		{
			*static_cast< T* >( varData ) = value;
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
// ae::EnumType templated member functions
//------------------------------------------------------------------------------
template< typename T >
bool ae::EnumType::GetVarData( ae::ConstDataPointer _varData, T* valueOut ) const
{
	if constexpr( !std::is_integral_v< T > && !std::is_enum_v< T > )
	{
		AE_DEBUG_ASSERT_MSG( false, "Invalid type conversion attempted" );
		return false;
	}
	else if( valueOut )
	{
		if( const void* varData = _varData.Get( this ) )
		{
			if( TypeIsSigned() )
			{
				switch( TypeSize() )
				{
					case 1: { *valueOut = static_cast< T >( *static_cast< const int8_t* >( varData ) ); return true; }
					case 2: { *valueOut = static_cast< T >( *static_cast< const int16_t* >( varData ) ); return true; }
					case 4: { *valueOut = static_cast< T >( *static_cast< const int32_t* >( varData ) ); return true; }
					case 8: { *valueOut = static_cast< T >( *static_cast< const int64_t* >( varData ) ); return true; }
				}
			}
			else
			{
				switch( TypeSize() )
				{
					case 1: { *valueOut = static_cast< T >( *static_cast< const uint8_t* >( varData ) ); return true; }
					case 2: { *valueOut = static_cast< T >( *static_cast< const uint16_t* >( varData ) ); return true; }
					case 4: { *valueOut = static_cast< T >( *static_cast< const uint32_t* >( varData ) ); return true; }
					case 8: { *valueOut = static_cast< T >( *static_cast< const uint64_t* >( varData ) ); return true; }
				}
			}
		}
	}
	return false;
}

template< typename T >
bool ae::EnumType::SetVarData( ae::DataPointer _varData, const T& value ) const
{
	if constexpr( !std::is_integral_v< T > && !std::is_enum_v< T > )
	{
		AE_DEBUG_ASSERT_MSG( false, "Invalid type conversion attempted" );
		return false;
	}
	else if( void* varData = _varData.Get( this ) )
	{
		if( TypeIsSigned() )
		{
			switch( TypeSize() )
			{
				case 1: { *static_cast< int8_t* >( varData ) = static_cast< int8_t >( value ); return true; }
				case 2: { *static_cast< int16_t* >( varData ) = static_cast< int16_t >( value ); return true; }
				case 4: { *static_cast< int32_t* >( varData ) = static_cast< int32_t >( value ); return true; }
				case 8: { *static_cast< int64_t* >( varData ) = static_cast< int64_t >( value ); return true; }
			}
		}
		else
		{
			switch( TypeSize() )
			{
				case 1: { *static_cast< uint8_t* >( varData ) = static_cast< uint8_t >( value ); return true; }
				case 2: { *static_cast< uint16_t* >( varData ) = static_cast< uint16_t >( value ); return true; }
				case 4: { *static_cast< uint32_t* >( varData ) = static_cast< uint32_t >( value ); return true; }
				case 8: { *static_cast< uint64_t* >( varData ) = static_cast< uint64_t >( value ); return true; }
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------
// ae::ClassType templated member functions
//------------------------------------------------------------------------------
template< typename T >
T* ae::ClassType::TryGet( ae::DataPointer varData ) const
{
	return const_cast< T* >( TryGet< T >( (ae::ConstDataPointer)varData ) );
}

template< typename T >
const T* ae::ClassType::TryGet( ae::ConstDataPointer varData ) const
{
	if( IsType< T >() )
	{
		return static_cast< const T* >( varData.Get( this ) );
	}
	return nullptr;
}

template< typename T >
T* ae::ClassType::New( void* obj ) const
{
	AE_ASSERT( obj );
	AE_ASSERT_MSG( !m_isAbstract, "Placement new not available for abstract type: #", m_name.c_str() );
	AE_ASSERT_MSG( m_isDefaultConstructible, "Placement new not available for type without default constructor: #", m_name.c_str() );
	AE_ASSERT( m_placementNew );
	AE_ASSERT( IsType< T >() );
	AE_ASSERT( (uint64_t)obj % GetAlignment() == 0 );
	return (T*)m_placementNew( (T*)obj );
}

template< typename T >
typename std::enable_if< !std::is_abstract< T >::value && std::is_default_constructible< T >::value, void >::type
ae::ClassType::Init( const char* name )
{
	if constexpr( std::is_base_of_v< ae::Object, T > )
	{
		m_placementNew = &( _PlacementNew< T > );
	}
	else
	{
		m_placementNew = nullptr;
	}
	m_name = name;
	m_id = GetTypeIdFromName( name );
	m_size = sizeof( T );
	m_align = alignof( T );
	m_parent = T::GetParentTypeName();
	m_isAbstract = false;
	m_isPolymorphic = std::is_polymorphic< T >::value;
	m_isDefaultConstructible = (bool)m_placementNew;
	m_isFinal = std::is_final< T >::value;
}
template< typename T >
typename std::enable_if< std::is_abstract< T >::value || !std::is_default_constructible< T >::value, void >::type
ae::ClassType::Init( const char* name )
{
	m_placementNew = nullptr;
	m_name = name;
	m_id = GetTypeIdFromName( name );
	m_size = sizeof( T );
	m_align = 0;
	m_parent = T::GetParentTypeName();
	m_isAbstract = std::is_abstract< T >::value;
	m_isPolymorphic = std::is_polymorphic< T >::value;
	m_isDefaultConstructible = (bool)m_placementNew;
	m_isFinal = std::is_final< T >::value;
}

//------------------------------------------------------------------------------
// ae::AttributeList templated member functions
//------------------------------------------------------------------------------
template< typename T >
const T* ae::AttributeList::TryGet( uint32_t idx ) const
{
	if constexpr( std::is_same_v< ae::Attribute, T > )
	{
		return m_attributes[ idx ];
	}
	else
	{
		static_assert( std::is_final_v< T >, "ae::AttributeList::TryGet() does not support intermediate levels of inheritance." );
		const ae::TypeId attributeType = ae::GetTypeIdFromName( ae::_TypeName< T >::Get() ); // @TODO: Compile time
		const _Info* info = m_attributeTypes.TryGet( attributeType );
		if( info && idx < info->count )
		{
			const ae::Attribute* result = m_attributes[ info->start + idx ];
			AE_DEBUG_ASSERT( !result || result->_metaTypeId == attributeType );
			return static_cast< const T* >( result );
		}
	}
	return nullptr;
}

template< typename T >
uint32_t ae::AttributeList::GetCount() const
{
	if constexpr( std::is_same_v< ae::Attribute, T > )
	{
		return m_attributes.Length();
	}
	else
	{
		static_assert( std::is_final_v< T >, "ae::AttributeList::TryGet() does not support intermediate levels of inheritance." );
		const ae::TypeId attributeType = ae::GetTypeIdFromName( ae::_TypeName< T >::Get() ); // @TODO: Compile time
		const _Info* info = m_attributeTypes.TryGet( attributeType );
		return info ? info->count : 0;
	}
}

template< typename T >
bool ae::AttributeList::Has() const
{
	return (bool)GetCount< T >();
}

//------------------------------------------------------------------------------
// ae::ClassVar templated member functions
//------------------------------------------------------------------------------
template< typename T >
bool ae::ClassVar::SetObjectValue( ae::Object* obj, const T& value, int32_t arrayIdx ) const
{
	if( !obj )
	{
		return false;
	}

	const ae::ClassType* objType = ae::GetClassTypeFromObject( obj );
	AE_ASSERT( objType );
	AE_ASSERT_MSG( objType->IsType( m_owner.GetClassType() ), "Attempting to set var on '#' with unrelated type '#'", objType->GetName(), m_owner.GetClassType()->GetName() );

	ae::DataPointer varData( this, obj );
	if( const ae::ArrayType* arrayVarType = varData.GetVarType().AsVarType< ae::ArrayType >() )
	{
		if( arrayIdx < 0 )
		{
			return false;
		}
		varData = arrayVarType->GetElement( varData, arrayIdx );
	}
	else if( arrayIdx >= 0 )
	{
		return false;
	}

	if( varData )
	{
		const ae::Type* varType = &varData.GetVarType();
		if( const ae::BasicType* basicVarType = varType->AsVarType< ae::BasicType >() )
		{
			return basicVarType->SetVarData( varData, value );
		}
		else if( const ae::EnumType* enumVarType = varType->AsVarType< ae::EnumType >() )
		{
			return enumVarType->SetVarData( varData, value );
		}
		else if( const ae::ClassType* classType = varType->AsVarType< ae::ClassType >() )
		{
			if( T* innerObject = classType->TryGet< T >( varData ) )
			{
				*innerObject = value;
				return true;
			}
		}
		else if constexpr( std::is_pointer_v< T > || std::is_null_pointer_v< T > )
		{
			const ae::PointerType* pointerVarType = varType->AsVarType< ae::PointerType >();
			return pointerVarType ? pointerVarType->SetRef( varData, (ae::Object*)value ) : false;
		}
	}
	return false;
}

template< typename T >
bool ae::ClassVar::GetObjectValue( const ae::Object* object, T* valueOut, int32_t arrayIdx ) const
{
	if( !object )
	{
		return false;
	}

	ae::ConstDataPointer varData;
	if( const ae::ArrayType* arrayType = GetOuterVarType().AsVarType< ae::ArrayType >() )
	{
		if( arrayIdx < 0 )
		{
			return false;
		}
		ae::ConstDataPointer array( this, object );
		varData = arrayType->GetElement( array, arrayIdx );
	}
	else if( arrayIdx < 0 )
	{
		varData = { this, object };
	}

	if( varData )
	{
		const ae::Type* varType = &varData.GetVarType();
		if( const ae::BasicType* basicType = varType->AsVarType< ae::BasicType >() )
		{
			return basicType->GetVarData( varData, valueOut );
		}
		else if( const ae::EnumType* enumType = varType->AsVarType< ae::EnumType >() )
		{
			return enumType->GetVarData( varData, valueOut );
		}
		else if( const ae::ClassType* classType = varType->AsVarType< ae::ClassType >() )
		{
			if( valueOut )
			{
				if( const T* innerObject = classType->TryGet< T >( varData ) )
				{
					*valueOut = *innerObject;
					return true;
				}
			}
		}
	}
	return false;
}

template< typename T >
T* ae::ClassVar::GetPointer( ae::Object* obj, int32_t arrayIdx ) const
{
	return const_cast< T* >( GetPointer< T >( const_cast< const ae::Object* >( obj ), arrayIdx ) );
}

template< typename T >
const T* ae::ClassVar::GetPointer( const ae::Object* obj, int32_t arrayIdx ) const
{
	if( !obj )
	{
		return nullptr;
	}

	ae::ConstDataPointer varData( this, obj );
	if( const ae::ArrayType* arrayVarType = varData.GetVarType().AsVarType< ae::ArrayType >() )
	{
		if( arrayIdx < 0 )
		{
			return nullptr;
		}
		varData = arrayVarType->GetElement( varData, arrayIdx );
	}
	else if( arrayIdx >= 0 )
	{
		return nullptr;
	}

	if( varData )
	{
		const ae::ClassType* classVarType = varData.GetVarType().AsVarType< ae::ClassType >();
		return classVarType ? classVarType->TryGet< T >( varData ) : nullptr;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
// ae::Cast implementation
//------------------------------------------------------------------------------
template< typename T, typename C >
const T* ae::Cast( const C* obj )
{
	static_assert( std::is_base_of< C, T >::value || std::is_base_of< T, C >::value, "Unrelated types" );
	if( !obj )
	{
		return nullptr;
	}
	const ae::ClassType* type = ae::GetClassType< T >();
	if( type->GetId() == obj->_metaTypeId )
	{
		return static_cast< const T* >( obj );
	}
	const ae::ClassType* objType = ae::GetClassTypeById( obj->_metaTypeId );
	AE_ASSERT( objType );
	return objType->IsType( type ) ? static_cast< const T* >( obj ) : nullptr; // No use of RTTI
}

template< typename T, typename C >
T* ae::Cast( C* obj )
{
	return const_cast< T* >( ae::Cast< T >( const_cast< const C* >( obj ) ) );
}

#endif // AE_AETHER_H

//------------------------------------------------------------------------------
// AE_MAIN
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
#elif _AE_APPLE_
	#define GL_SILENCE_DEPRECATION
#endif

//------------------------------------------------------------------------------
// Platform includes, required for logging, windowing, file io
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
	#define WIN32_LEAN_AND_MEAN 1
	#include <Windows.h>
	#include <Windowsx.h>
	#include <Winuser.h>
	#include <shellapi.h>
	#include <Shlobj_core.h>
	#include <commdlg.h>
	#include "processthreadsapi.h" // For GetCurrentProcessId()
	#include <filesystem> // @HACK: Shouldn't need this just for Windows
	#include <timeapi.h>
	#include <xinput.h>
	#pragma comment (lib, "Comdlg32.lib")
	#pragma comment (lib, "Gdi32.lib")
	#pragma comment (lib, "Imm32.lib")
	#pragma comment (lib, "Ole32.lib")
	#pragma comment (lib, "Setupapi.lib")
	#pragma comment (lib, "Shell32.lib")
	#pragma comment (lib, "User32.lib")
	#pragma comment (lib, "version.lib")
	#pragma comment (lib, "Winmm.lib")
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "XInput.lib")
	#pragma comment (lib, "OpenGL32.lib")
	#ifndef AE_USE_OPENAL
		#define AE_USE_OPENAL 0
	#endif
#elif _AE_APPLE_
	#include <sys/sysctl.h>
	#include <unistd.h>
	#include <pwd.h>
	#include <dlfcn.h>
	#include <mach-o/dyld.h>
	#ifdef AE_USE_MODULES
		@import AppKit;
		@import Carbon;
		@import Cocoa;
		@import CoreFoundation;
		@import OpenGL;
		@import OpenAL;
		@import GameController;
	#else
		#if _AE_IOS_
		#import <Foundation/Foundation.h>
		#else
		#include <Cocoa/Cocoa.h>
		#include <Carbon/Carbon.h>
		#endif
		#include <GameController/GameController.h>
	#endif
	#ifndef AE_USE_OPENAL
		#define AE_USE_OPENAL 1
	#endif
#elif _AE_LINUX_
	#include <unistd.h>
	#include <pwd.h>
	#include <limits.h>
	#include <sys/stat.h>
	#ifndef AE_USE_OPENAL
		#define AE_USE_OPENAL 0
	#endif
#elif _AE_EMSCRIPTEN_
	#ifndef AE_USE_OPENAL
		#define AE_USE_OPENAL 1
	#endif
#endif
#include <inttypes.h>
#include <thread>
#include <random>
// Socket
#if _AE_WINDOWS_
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	typedef uint16_t _ae_sa_family_t;
	typedef char _ae_sock_err_t;
	typedef WSAPOLLFD _ae_poll_fd_t;
	typedef char _ae_sock_buff_t;
	#define _ae_sock_poll WSAPoll
	#define _ae_ioctl ioctlsocket
#else
	#include <netdb.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <poll.h>
	#include <netinet/tcp.h>
	#include <fcntl.h>
	typedef sa_family_t _ae_sa_family_t;
	typedef int _ae_sock_err_t;
	typedef pollfd _ae_poll_fd_t;
	typedef uint8_t _ae_sock_buff_t;
	#define _ae_sock_poll poll
	#define _ae_ioctl ioctl
#endif
#if AE_USE_OPENAL
	#if _AE_APPLE_
		#include <OpenAL/al.h>
		#include <OpenAL/alc.h>
	#else
		#include "AL/al.h"
		#include "AL/alc.h"
	#endif
#endif
#if !_AE_EMSCRIPTEN_
#define EMSCRIPTEN_KEEPALIVE
#endif
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
	#include <arm_neon.h>
#endif

namespace ae {

//------------------------------------------------------------------------------
// ae::_Globals
//------------------------------------------------------------------------------
ae::_Globals::~_Globals()
{
	AE_ASSERT( !enumTypes.Length() );
	AE_ASSERT( !classTypes.Length() );
	AE_ASSERT( !scratchBuffer.offset );
}

//------------------------------------------------------------------------------
// Internal ae::_ScratchBuffer storage
//------------------------------------------------------------------------------
_ScratchBuffer::_ScratchBuffer( uint32_t size ) : size( size )
{
	offset = 0;
	data = new uint8_t[ size ]; // @TODO: Maybe this shouldn't use new/delete?
	AE_ASSERT( (intptr_t)data % kScratchAlignment == 0 );
}
_ScratchBuffer::~_ScratchBuffer()
{
	AE_ASSERT( offset == 0 );
	delete [] data;
}

uint32_t _ScratchBuffer::GetScratchBytes( uint32_t bytes )
{
	// Round up allocation size as needed to maintain offset alignment
#if _AE_DEBUG_
	bytes += 2; // At least 2 byte guard
#endif
	return ( ( bytes + kScratchAlignment - 1 ) / kScratchAlignment ) * kScratchAlignment;
}

//------------------------------------------------------------------------------
// Internal ae::_Globals functions
//------------------------------------------------------------------------------
ae::_Globals* ae::_Globals::Get()
{
	static ae::_Globals s_globals;
	return &s_globals;
}

//------------------------------------------------------------------------------
// Internal ae::_ThreadLocals functions
//------------------------------------------------------------------------------
ae::_ThreadLocals* ae::_ThreadLocals::Get()
{
	static thread_local ae::_ThreadLocals s_threadLocals;
	return &s_threadLocals;
}

//------------------------------------------------------------------------------
// Platform functions internal implementation
//------------------------------------------------------------------------------
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

#if _AE_EMSCRIPTEN_
EM_JS( float, _ae_performance_now, (),
{
	return performance.now();
} );
#endif

double GetTime()
{
#if _AE_WINDOWS_
	static LARGE_INTEGER counterFrequency = { 0 };
	if( !counterFrequency.QuadPart )
	{
		bool success = QueryPerformanceFrequency( &counterFrequency ) != 0;
		AE_ASSERT( success );
	}

	LARGE_INTEGER performanceCount = { 0 };
	bool success = QueryPerformanceCounter( &performanceCount ) != 0;
	AE_ASSERT( success );
	return performanceCount.QuadPart / (double)counterFrequency.QuadPart;
#elif _AE_EMSCRIPTEN_
	return _ae_performance_now() / 1000.0f;
#else
	return std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::high_resolution_clock::now().time_since_epoch() ).count() / 1000000.0;
#endif
}

void ShowMessage( const char* msg )
{
#if _AE_WINDOWS_
	MessageBoxA( nullptr, msg, nullptr, MB_OK );
#elif _AE_OSX_
	if( [NSThread isMainThread] )
	{
		NSAlert* alert = [[NSAlert alloc] init];
		[alert setMessageText: [NSString stringWithUTF8String: msg]];
		[alert runModal];
	}
	else
	{
		AE_WARN( "ae::ShowMessage() called from non-main thread, ignoring." );
	}
#endif
}

void SetClipboardText( const char* text )
{
#if _AE_WINDOWS_
	// if( OpenClipboard( nullptr ) )
	// {
	// 	EmptyClipboard();
	// 	HGLOBAL hglbCopy = GlobalAlloc( GMEM_MOVEABLE, strlen( text ) + 1 );
	// 	if( hglbCopy )
	// 	{
	// 		if( char* buffer = (char*)GlobalLock( hglbCopy ) )
	// 		{
	// 			strcpy_s( buffer, strlen( text ) + 1, text );
	// 			GlobalUnlock( hglbCopy );
	// 			SetClipboardData( CF_TEXT, hglbCopy );
	// 		}
	// 	}
	// 	CloseClipboard();
	// }
#elif _AE_OSX_
	NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
	[pasteboard clearContents];
	[pasteboard setString: [NSString stringWithUTF8String: text] forType: NSStringPboardType];
#endif
}

std::string GetClipboardText()
{
	std::string result;
#if _AE_WINDOWS_
	// if( OpenClipboard( nullptr ) )
	// {
	// 	if( HANDLE hglb = GetClipboardData( CF_TEXT ) )
	// 	{
	// 		char* buffer = (char*)GlobalLock( hglb );
	// 		if( buffer )
	// 		{
	// 			result = buffer;
	// 			GlobalUnlock( hglb );
	// 		}
	// 	}
	// 	CloseClipboard();
	// }
#elif _AE_OSX_
	NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
	if( NSString* str = [pasteboard stringForType: NSStringPboardType] )
	{
		result = [str UTF8String];
	}
#endif
	return result;
}

//------------------------------------------------------------------------------
// ae::Random functions
//------------------------------------------------------------------------------
thread_local uint64_t _randomSeed = 0;

void RandomSeed()
{
	std::random_device r;
	_randomSeed = r();
}

//------------------------------------------------------------------------------
// ae::Vec2 functions
//------------------------------------------------------------------------------
Vec2 Vec2::Slerp( const Vec2& end, float t, float epsilon ) const
{
	Vec2 v0 = *this;
	Vec2 v1 = end;
	float l0 = v0.SafeNormalize();
	float l1 = v1.SafeNormalize();
	if( l0 < epsilon && l1 < epsilon )
	{
		return Vec2( 0.0f );
	}
	else if( l1 < epsilon ) // Zero length target
	{
		return v0 * ae::Lerp( l0, 0.0f, t );
	}
	else if( l0 < epsilon ) // Zero length initial vector
	{
		return v1 * ae::Lerp( 0.0f, l1, t );
	}
	float d = ae::Clip( v0.Dot( v1 ), -1.0f, 1.0f );
	if( d > ( 1.0f - epsilon ) )
	{
		return v1;
	}
	if( d < -( 1.0f - epsilon ) )
	{
		return v0;
	}
	float angle = std::acos( d ) * t;
	Vec2 v2 = v1 - v0 * d;
	v2.Normalize();
	return ( ( v0 * std::cos( angle ) ) + ( v2 * std::sin( angle ) ) );
}

//------------------------------------------------------------------------------
// ae::Vec3 functions
//------------------------------------------------------------------------------
float Vec3::GetAngleBetween( const Vec3& v, float epsilon ) const
{
	float l0 = Length();
	float l1 = v.Length();
	if( l0 < epsilon || l1 < epsilon )
	{
		return 0.0f;
	}
	float result = acosf( ae::Clip( Dot( v ) / ( l0 * l1 ), -1.0f, 1.0f ) ); // No std::, for linux builds
	return ( result <= ae::PI ) ? result : ( result - ae::PI );
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

Vec3 Vec3::AddRotationXYCopy( float rotation ) const
{
	Vec3 r = *this;
	r.AddRotationXY( rotation );
	return r;
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
	Vec3 v0 = *this;
	Vec3 v1 = end;
	float l0 = v0.SafeNormalize();
	float l1 = v1.SafeNormalize();
	if( l0 < epsilon && l1 < epsilon )
	{
		return Vec3( 0.0f );
	}
	else if( l1 < epsilon ) // Zero length target
	{
		return v0 * ae::Lerp( l0, 0.0f, t );
	}
	else if( l0 < epsilon ) // Zero length initial vector
	{
		return v1 * ae::Lerp( 0.0f, l1, t );
	}
	const float l = ae::Lerp( l0, l1, t );
	const float d = ae::Clip( v0.Dot( v1 ), -1.0f + epsilon, 1.0f ); // Don't allow vector to directly apose
	if( d > ( 1.0f - epsilon ) )
	{
		return v1 * l; // Vectors almost directly align, so just lerp length
	}
	const float angle = std::acos( d ) * t;
	const Vec3 v2 = ( v1 - v0 * d ).NormalizeCopy();
	return ( ( v0 * std::cos( angle ) ) + ( v2 * std::sin( angle ) ) ) * l;
}

Vec3 Vec3::DtSlerp( const Vec3& end, float snappiness, float dt, float epsilon ) const
{
	if( snappiness == 0.0f || dt == 0.0f )
	{
		return *this;
	}
	return Slerp( end, 1.0f - exp2( -exp2( snappiness ) * dt ), epsilon );
}

//------------------------------------------------------------------------------
// ae::Matrix4 member functions
//------------------------------------------------------------------------------
// clang-format off
Matrix4 Matrix4::Identity()
{
	Matrix4 r;
	r.data[ 0 ] = 1; r.data[ 4 ] = 0; r.data[ 8 ] = 0;  r.data[ 12 ] = 0;
	r.data[ 1 ] = 0; r.data[ 5 ] = 1; r.data[ 9 ] = 0;  r.data[ 13 ] = 0;
	r.data[ 2 ] = 0; r.data[ 6 ] = 0; r.data[ 10 ] = 1; r.data[ 14 ] = 0;
	r.data[ 3 ] = 0; r.data[ 7 ] = 0; r.data[ 11 ] = 0; r.data[ 15 ] = 1;
	return r;
}

Matrix4 Matrix4::Translation( float tx, float ty, float tz )
{
	Matrix4 r;
	r.data[ 0 ] = 1.0f; r.data[ 4 ] = 0.0f; r.data[ 8 ] = 0.0f;  r.data[ 12 ] = tx;
	r.data[ 1 ] = 0.0f; r.data[ 5 ] = 1.0f; r.data[ 9 ] = 0.0f;  r.data[ 13 ] = ty;
	r.data[ 2 ] = 0.0f; r.data[ 6 ] = 0.0f; r.data[ 10 ] = 1.0f; r.data[ 14 ] = tz;
	r.data[ 3 ] = 0.0f; r.data[ 7 ] = 0.0f; r.data[ 11 ] = 0.0f; r.data[ 15 ] = 1.0f;
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

Matrix4 Matrix4::Rotation( ae::Quaternion q )
{
	return q.GetTransformMatrix();
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

Matrix4 Matrix4::Scaling( float sx, float sy, float sz )
{
	Matrix4 r;
	r.data[ 0 ] = sx;   r.data[ 4 ] = 0.0f; r.data[ 8 ] = 0.0f;  r.data[ 12 ] = 0.0f;
	r.data[ 1 ] = 0.0f; r.data[ 5 ] = sy;   r.data[ 9 ] = 0.0f;  r.data[ 13 ] = 0.0f;
	r.data[ 2 ] = 0.0f; r.data[ 6 ] = 0.0f; r.data[ 10 ] = sz;   r.data[ 14 ] = 0.0f;
	r.data[ 3 ] = 0.0f; r.data[ 7 ] = 0.0f; r.data[ 11 ] = 0.0f; r.data[ 15 ] = 1.0f;
	return r;
}
// clang-format on

Matrix4 Matrix4::Scaling( float s )
{
	return Scaling( s, s, s );
}

Matrix4 Matrix4::Scaling( const Vec3& s )
{
	return Scaling( s.x, s.y, s.z );
}

Matrix4 Matrix4::LocalToWorld( ae::Vec3 position, ae::Quaternion rotation, ae::Vec3 scale )
{
	return Matrix4::Rotation( rotation ).SetScale( scale ).SetTranslation( position );
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
#if _AE_DEBUG_
	AE_ASSERT( forward == forward );
	AE_ASSERT( right == right );
	AE_ASSERT( up == up );
	AE_ASSERT( forward.LengthSquared() );
	AE_ASSERT( right.LengthSquared() );
	AE_ASSERT( up.LengthSquared() );
#endif

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
	if( ReverseZ )
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

Matrix4& Matrix4::SetTranslation( float x, float y, float z )
{
	data[ 12 ] = x;
	data[ 13 ] = y;
	data[ 14 ] = z;
	return *this;
}

Matrix4& Matrix4::SetTranslation( const Vec3& translation )
{
	data[ 12 ] = translation.x;
	data[ 13 ] = translation.y;
	data[ 14 ] = translation.z;
	return *this;
}

Matrix4& Matrix4::SetRotation( const Quaternion& q2 )
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
	return *this;
}

Matrix4& Matrix4::SetScale( const Vec3& s )
{
	for( uint32_t i = 0; i < 3; i++ )
	{
		SetAxis( i, GetAxis( i ).NormalizeCopy() * s[ i ] );
	}
	return *this;
}

Matrix4& Matrix4::SetScale( float s )
{
	for( uint32_t i = 0; i < 3; i++ )
	{
		SetAxis( i, GetAxis( i ).NormalizeCopy() * s );
	}
	return *this;
}

Matrix4& Matrix4::SetTranspose()
{
	for( uint32_t i = 0; i < 4; i++ )
	{
		for( uint32_t j = i + 1; j < 4; j++ )
		{
			std::swap( data[ i * 4 + j ], data[ j * 4 + i ] );
		}
	}
	return *this;
}

Matrix4& Matrix4::SetInverse()
{
	*this = GetInverse();
	return *this;
}

Matrix4& Matrix4::SetAxis( uint32_t col, const Vec3& v )
{
	data[ col * 4 ] = v.x;
	data[ col * 4 + 1 ] = v.y;
	data[ col * 4 + 2 ] = v.z;
	return *this;
}

Matrix4& Matrix4::SetAxis( uint32_t col, const Vec4& v )
{
	columns[ col ] = v;
	return *this;
}

Matrix4& Matrix4::SetRow( uint32_t row, const Vec3 &v )
{
	data[ row ] = v.x;
	data[ row + 4 ] = v.y;
	data[ row + 8 ] = v.z;
	return *this;
}

Matrix4& Matrix4::SetRow( uint32_t row, const Vec4 &v)
{
	data[ row ] = v.x;
	data[ row + 4 ] = v.y;
	data[ row + 8 ] = v.z;
	data[ row + 12 ] = v.w;
	return *this;
}

Vec3 Matrix4::GetTranslation() const
{
	return Vec3( &data[ 12 ] );
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
	if( trace > 0.0f )
	{
		float s = sqrt( trace + 1.0f ) * 2.0f;
		return Quaternion(
			( m21 - m12 ) / s,
			( m02 - m20 ) / s,
			( m10 - m01 ) / s,
			0.25f * s
		);
	}
	else if( ( m00 > m11 ) && ( m00 > m22 ) )
	{
		float s = sqrt( 1.0f + m00 - m11 - m22 ) * 2.0f;
		return Quaternion(
			0.25f * s,
			( m01 + m10 ) / s,
			( m02 + m20 ) / s,
			( m21 - m12 ) / s
		);
	}
	else if( m11 > m22 )
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

Vec3 Matrix4::GetScale() const
{
	return Vec3(
		Vec3( &data[ 0 ] ).Length(),
		Vec3( &data[ 4 ] ).Length(),
		Vec3( &data[ 8 ] ).Length()
	);
}

Matrix4 Matrix4::GetTranspose() const
{
	Matrix4 r = *this;
	r.SetTranspose();
	return r;
}

// clang-format off
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
#if _AE_DEBUG_
	AE_ASSERT_MSG( det == det, "Non-invertible matrix '#'", *this );
	AE_ASSERT_MSG( det, "Non-invertible matrix '#'", *this );
#endif
	det = 1.0f / det;
	for( uint32_t i = 0; i < 16; i++ )
	{
		r.data[ i ] *= det;
	}
	
	return r;
}
// clang-format on

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

Vec3 Matrix4::GetAxis( uint32_t col ) const
{
	return Vec3( columns[ col ] );
}

Vec4 Matrix4::GetColumn( uint32_t col ) const
{
	return columns[ col ];
}

Vec4 Matrix4::GetRow( uint32_t row ) const
{
	return Vec4( data[ row ], data[ row + 4 ], data[ row + 8 ], data[ row + 12 ] );
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

Vec4 Matrix4::operator*(const Vec4& v) const
{
	return Vec4(
		v.x*data[0] + v.y*data[4] + v.z*data[8] + v.w*data[12],
		v.x*data[1] + v.y*data[5] + v.z*data[9] + v.w*data[13],
		v.x*data[2] + v.y*data[6] + v.z*data[10] + v.w*data[14],
		v.x*data[3] + v.y*data[7] + v.z*data[11] + v.w*data[15]);
}

ae::Vec3 Matrix4::TransformPoint3x4( ae::Vec3 v ) const
{
	return Vec3(
		v.x * data[ 0 ] + v.y * data[ 4 ] + v.z * data[ 8 ] + data[ 12 ],
		v.x * data[ 1 ] + v.y * data[ 5 ] + v.z * data[ 9 ] + data[ 13 ],
		v.x * data[ 2 ] + v.y * data[ 6 ] + v.z * data[ 10 ] + data[ 14 ] );
}

ae::Vec3 Matrix4::TransformVector3x4( ae::Vec3 v ) const
{
	return Vec3(
		v.x * data[ 0 ] + v.y * data[ 1 ] + v.z * data[ 2 ],
		v.x * data[ 4 ] + v.y * data[ 5 ] + v.z * data[ 6 ],
		v.x * data[ 8 ] + v.y * data[ 9 ] + v.z * data[ 10 ] );
}

bool Matrix4::IsNAN() const
{
	for( float f : data )
	{
		if( f != f )
		{
			return true;
		}
	}
	return false;
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
	if( prioritizeUp )
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
	if( trace > 0.0f )
	{
		float S = sqrtf( trace + 1.0f ) * 2;
		r = 0.25f * S;
		i = ( m21 - m12 ) / S;
		j = ( m02 - m20 ) / S;
		k = ( m10 - m01 ) / S;
	}
	else if( ( m00 > m11 ) & ( m00 > m22 ) )
	{
		float S = sqrtf( 1.0f + m00 - m11 - m22 ) * 2;
		r = ( m21 - m12 ) / S;
		i = 0.25f * S;
		j = ( m01 + m10 ) / S;
		k = ( m02 + m20 ) / S;
	}
	else if( m11 > m22 )
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

	if( invMagnitude == 0.0f )
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

Quaternion Quaternion::NormalizeCopy() const
{
	Quaternion r = *this;
	r.Normalize();
	return r;
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

Quaternion Quaternion::operator*( float s ) const
{
	return Quaternion( s * i, s * j, s * k, s * r );
}

Quaternion Quaternion::operator- () const
{
	return Quaternion( -i, -j, -k, -r );
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
	if( v.x < 0 && v.y >= 0 )
		theta += ae::PI;
	else if( v.x < 0 && v.y < 0 )
		theta -= ae::PI;

	r = std::cos( theta / 2.0f );
	i = 0.0f;
	j = 0.0f;
	k = std::sin( theta / 2.0f );
}

Vec3 Quaternion::GetDirectionXY() const
{
	float theta;
	if( k >= 0.0f )
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
	if( angle )
	{
		*angle = 2 * acos( r );
	}
	if( axis )
	{
		const float s = sqrt( 1.0f - r * r );
		// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle/index.htm
		if( s >= 0.0001f )
		{
			axis->x = i / s;
			axis->y = j / s;
			axis->z = k / s;
		}
		else
		{
			axis->x = i;
			axis->y = j;
			axis->z = k;
		}
	}
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

	if( epsilon < 0.0f )
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

Quaternion Quaternion::RelativeCopy( const Quaternion& reference ) const
{
	return reference.GetInverse() * (*this);
}

void Quaternion::GetTwistSwing( Vec3 axis, Quaternion* twistOut, Quaternion* swingOut ) const
{
	// https://theorangeduck.com/page/joint-limits#swingtwist
	const ae::Vec3 p = ae::Vec3( i, j, k ).Dot( axis ) * axis;
	const ae::Quaternion twistRot = ae::Quaternion( p.x, p.y, p.z, r ).NormalizeCopy();
	if( twistOut )
	{
		*twistOut = twistRot;
	}
	if( swingOut )
	{
		*swingOut = -( *this * twistRot.GetInverse() );
	}
}

//------------------------------------------------------------------------------
// ae::Sphere member functions
//------------------------------------------------------------------------------
Sphere::Sphere( const OBB& obb )
{
	center = obb.GetCenter();
	radius = obb.GetHalfSize().Length();
}

void Sphere::Expand( ae::Vec3 p0 )
{
	if( radius < 0.00001f )
	{
		radius = ( center - p0 ).Length() * 0.5f;
		center = ( center + p0 ) * 0.5f;
	}
	else
	{
		ae::Vec3 p1 = ( center - p0 ).SafeNormalizeCopy() * radius;
		center = ( p0 + p1 ) * 0.5f;
		radius = ( center - p1 ).Length();
	}
}

bool Sphere::IntersectRay( Vec3 origin, Vec3 direction, Vec3* pOut, float* tOut ) const
{
	direction.SafeNormalize();

	Vec3 m = origin - center;
	float b = m.Dot( direction );
	float c = m.Dot( m ) - radius * radius;
	// Exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
	if( c > 0.0f && b > 0.0f )
	{
		return false;
	}

	// A negative discriminant corresponds to ray missing sphere
	float discr = b * b - c;
	if( discr < 0.0f )
	{
		return false;
	}

	// @TODO: This check should be against the ray segment, and so should be limited here
	// Ray now found to intersect sphere, compute smallest t value of intersection
	float t = -b - sqrtf( discr );
	if( t < 0.0f )
	{
		t = 0.0f; // If t is negative, ray started inside sphere so clamp t to zero
	}
	
	if( tOut )
	{
		*tOut = t;
	}
	if( pOut )
	{
		*pOut = origin + direction * t;
	}

	return true;
}

bool Sphere::IntersectTriangle( ae::Vec3 t0, ae::Vec3 t1, ae::Vec3 t2, ae::Vec3* outNearestIntersectionPoint ) const
{
	ae::Vec3 closest = ClosestPointOnTriangle( center, t0, t1, t2 );
	if( ( closest - center ).LengthSquared() <= radius * radius )
	{
	if( outNearestIntersectionPoint )
	{
		*outNearestIntersectionPoint = closest;
	}
	return true;
	}
	return false;
}

ae::Vec3 Sphere::GetNearestPointOnSurface( ae::Vec3 p, float* signedDistOut ) const
{
	ae::Vec3 v = ( p - center );
	const float d = v.SafeNormalize();
	if( signedDistOut )
	{
		*signedDistOut = ( d - radius );
	}
	return center + v * radius;
}

//------------------------------------------------------------------------------
// ae::Plane member functions
//------------------------------------------------------------------------------
Plane::Plane( ae::Vec4 pointNormal ) :
	m_plane( pointNormal / pointNormal.GetXYZ().Length() ) // Normalize
{}

Plane::Plane( ae::Vec3 point, ae::Vec3 normal )
{
	m_plane = ae::Vec4( normal.NormalizeCopy(), 0.0f );
	m_plane.w = GetSignedDistance( point );
}

Plane::Plane( ae::Vec3 p0, ae::Vec3 p1, ae::Vec3 p2 )
{
	ae::Vec3 normal = ( p1 - p0 ).Cross( p2 - p0 ).NormalizeCopy();
	m_plane = ae::Vec4( normal, 0.0f );
	m_plane.w = GetSignedDistance( p0 );
}

Plane::operator Vec4() const
{
	return m_plane;
}

ae::Vec3 Plane::GetNormal() const
{
	return m_plane.GetXYZ();
}

ae::Vec3 Plane::GetClosestPointToOrigin() const
{
	return m_plane.GetXYZ() * m_plane.w;
}

bool Plane::IntersectLine( ae::Vec3 p, ae::Vec3 d, Vec3* hitOut, float* tOut ) const
{
	ae::Vec3 n = m_plane.GetXYZ();
	ae::Vec3 q = n * m_plane.w;
	float a = d.Dot( n );
	if( ae::Abs( a ) < 0.001f )
	{
		return false; // Line is parallel to plane
	}
	ae::Vec3 diff = q - p;
	float b = diff.Dot( n );
	float t = b / a;
	if( hitOut )
	{
		*hitOut = p + d * t;
	}
	if( tOut )
	{
		*tOut = t;
	}
	return true;
}

bool Plane::IntersectRay( ae::Vec3 source, ae::Vec3 ray, Vec3* hitOut, float* tOut ) const
{
	const ae::Vec4 plane = GetSignedDistance( source ) >= 0.0f ? m_plane : -m_plane; // Plane normal towards ray source
	const ae::Vec3 n = plane.GetXYZ();
	const ae::Vec3 p = n * plane.w;
	const float a = ray.Dot( n );
	if( a > -0.001f )
	{
		return false; // Ray is pointing away from or parallel to plane
	}
	const ae::Vec3 diff = p - source;
	const float b = diff.Dot( n );
	const float t = b / a;
	if( t < 0.0f || t > 1.0f )
	{
		return false;
	}
	if( hitOut )
	{
		*hitOut = source + ray * t;
	}
	if( tOut )
	{
		*tOut = t;
	}
	return true;
}

ae::Vec3 Plane::GetClosestPoint( ae::Vec3 pos, float* distanceOut ) const
{
	ae::Vec3 n = m_plane.GetXYZ();
	float t = pos.Dot( n ) - m_plane.w;
	if( distanceOut )
	{
		*distanceOut = t;
	}
	return pos - n * t;
}

float Plane::GetSignedDistance( ae::Vec3 pos ) const
{
	 return pos.Dot( m_plane.GetXYZ() ) - m_plane.w;
}

//------------------------------------------------------------------------------
// ae::Line member functions
//------------------------------------------------------------------------------
Line::Line( ae::Vec3 p0, ae::Vec3 p1 )
	: m_p( p0 ), m_n( ( p1 - p0 ).SafeNormalizeCopy() )
{}

ae::Vec3 Line::GetClosest( ae::Vec3 p, float* distanceOut ) const
{
	ae::Vec3 result;
	float dist = GetDistance( p, &result );
	if( distanceOut )
	{
		*distanceOut = dist;
	}
	return result;
}

float Line::GetDistance( ae::Vec3 p, ae::Vec3* closestOut ) const
{
	float d = m_n.Dot( p - m_p );
	ae::Vec3 closest = m_p + m_n * d;
	if( closestOut )
	{
		*closestOut = closest;
	}
	return ( p - closest ).Length();
}

//------------------------------------------------------------------------------
// ae::LineSegment member functions
//------------------------------------------------------------------------------
LineSegment::LineSegment( ae::Vec3 p0, ae::Vec3 p1 )
{
	m_p0 = p0;
	m_p1 = p1;
}

ae::Vec3 LineSegment::GetClosest( ae::Vec3 p, float* distanceOut ) const
{
	ae::Vec3 result;
	float dist = GetDistance( p, &result );
	if( distanceOut )
	{
		*distanceOut = dist;
	}
	return result;
}

float LineSegment::GetDistance( ae::Vec3 p, ae::Vec3* closestOut ) const
{
	const ae::Vec3 n = ( m_p1 - m_p0 );
	const float lengthSq = n.LengthSquared();
	const float t = ( lengthSq > 0.001f ) ? ae::Clip01( ( p - m_p0 ).Dot( n ) / lengthSq ) : 0.0f;
	const ae::Vec3 linePos = ae::Lerp( m_p0, m_p1, t );
	if( closestOut )
	{
		*closestOut = linePos;
	}
	return ( p - linePos ).Length();
}

float LineSegment::GetLength() const
{
	return ( m_p1 - m_p0 ).Length();
}

ae::Vec3 LineSegment::GetStart() const
{
	return m_p0;
}

ae::Vec3 LineSegment::GetEnd() const
{
	return m_p1;
}

//------------------------------------------------------------------------------
// ae::Circle member functions
//------------------------------------------------------------------------------
Circle::Circle( ae::Vec2 point, float radius )
{
	m_point = point;
	m_radius = radius;
}

float Circle::GetArea( float radius )
{
	return ae::PI * radius * radius;
}

bool Circle::Intersect( const Circle& other, ae::Vec2* out ) const
{
	ae::Vec2 diff = other.m_point - m_point;
	float dist = diff.Length();
	if( dist > m_radius + other.m_radius )
	{
		return false;
	}

	if( out )
	{
		*out = m_point + diff.SafeNormalizeCopy() * ( ( m_radius + dist - other.m_radius ) * 0.5f );
	}
	return true;
}

ae::Vec2 Circle::GetRandomPoint( uint64_t* seed ) const
{
	float r = m_radius * sqrt( ae::Random01( seed ) );
	float theta = ae::Random( 0.0f, ae::TWO_PI, seed );
	return ae::Vec2( ae::Cos( theta ) * r + m_point.x, ae::Sin( theta ) * r + m_point.y );
}

//------------------------------------------------------------------------------
// ae::Frustum member functions
//------------------------------------------------------------------------------
Frustum::Frustum( ae::Matrix4 worldToProjection )
{
	ae::Vec4 row0 = worldToProjection.GetRow( 0 );
	ae::Vec4 row1 = worldToProjection.GetRow( 1 );
	ae::Vec4 row2 = worldToProjection.GetRow( 2 );
	ae::Vec4 row3 = worldToProjection.GetRow( 3 );

	ae::Vec4 planes[ countof( m_planes ) ];
	planes[ (int)ae::Frustum::Plane::Near ] = -row0 - row3;
	planes[ (int)ae::Frustum::Plane::Far ] = row0 - row3;
	planes[ (int)ae::Frustum::Plane::Left ] = -row1 - row3;
	planes[ (int)ae::Frustum::Plane::Right ] = row1 - row3;
	planes[ (int)ae::Frustum::Plane::Top ] = -row2 - row3;
	planes[ (int)ae::Frustum::Plane::Bottom ] = row2 - row3;

	for( uint32_t i = 0; i < countof( m_planes ); i++ )
	{
		planes[ i ].w = -planes[ i ].w;
		m_planes[ i ] = planes[ i ];
	}
}

bool Frustum::Intersects( ae::Vec3 point ) const
{
	for( uint32_t i = 0; i < countof(m_planes); i++ )
	{
		if( m_planes[ i ].GetSignedDistance( point ) > 0.0f )
		{
			return false;
		}
	}
	return true;
}

bool Frustum::Intersects( const ae::Sphere& sphere ) const
{
	for( int i = 0; i < countof(m_planes); i++ )
	{
		float distance = m_planes[ i ].GetSignedDistance( sphere.center );
		if( distance > 0.0f && distance - sphere.radius > 0.0f )
		{
			return false;
		}
	}
	return true;
}

Plane Frustum::GetPlane( ae::Frustum::Plane plane ) const
{
	return m_planes[ (int)plane ];
}

//------------------------------------------------------------------------------
// ae::AABB member functions
//------------------------------------------------------------------------------
AABB::AABB( ae::Vec3 p0, ae::Vec3 p1 )
{
	m_min = ae::Min( p0, p1 );
	m_max = ae::Max( p0, p1 );
}

AABB::AABB( const Sphere& sphere )
{
	ae::Vec3 r( sphere.radius );
	m_min = sphere.center - r;
	m_max = sphere.center + r;
}

bool AABB::operator == ( const AABB& aabb ) const
{
	return ( aabb.m_min == m_min ) && ( aabb.m_max == m_max );
}

bool AABB::operator != ( const AABB& aabb ) const
{
	return !( operator == ( aabb ) );
}

AABB& AABB::Expand( ae::Vec3 p )
{
	m_min = ae::Min( p, m_min );
	m_max = ae::Max( p, m_max );
	return *this;
}

AABB& AABB::Expand( AABB other )
{
	m_min = ae::Min( other.m_min, m_min );
	m_max = ae::Max( other.m_max, m_max );
	return *this;
}

AABB& AABB::Expand( float boundary )
{
	m_min -= ae::Vec3( boundary );
	m_max += ae::Vec3( boundary );
	return *this;
}

ae::Matrix4 AABB::GetTransform() const
{
	ae::Matrix4 r = ae::Matrix4::Scaling( m_max - m_min );
	r.SetAxis( 3, GetCenter() );
	return r;
}

float AABB::GetSignedDistanceFromSurface( ae::Vec3 p ) const
{
	ae::Vec3 q = ae::Abs( p - GetCenter() ) - GetHalfSize();
	return ae::Max( q, ae::Vec3( 0.0f ) ).Length() + ae::Min( ae::Max( q.x, q.y, q.z ), 0.0f );
}

bool AABB::Contains( Vec3 p ) const
{
	return !( p.x < m_min.x || m_max.x < p.x
		|| p.y < m_min.y || m_max.y < p.y
		|| p.z < m_min.z || m_max.z < p.z );
}

bool AABB::Intersect( AABB other ) const
{
	// @TODO: Handle 0 size overlapping edge case
	if( m_min.x > other.m_max.x || m_max.x < other.m_min.x
		|| m_min.y > other.m_max.y || m_max.y < other.m_min.y
		|| m_min.z > other.m_max.z || m_max.z < other.m_min.z )
	{
		return false;
	}
	return true;
}

Vec3 AABB::GetClosestPointOnSurface( Vec3 p, bool* containsOut ) const
{
	ae::Vec3 result;
	bool outside = false;
	for( uint32_t i = 0; i < 3; i++ )
	{
		result[ i ] = p[ i ];
		if( result[ i ] < m_min[ i ] )
		{
			result[ i ] = m_min[ i ];
			outside = true;
		}
		if( result[ i ] > m_max[ i ] )
		{
			result[ i ] = m_max[ i ];
			outside = true;
		}
	}
	if( !outside )
	{
		ae::Vec3 d = p - GetCenter();
		ae::Vec3 q = ae::Abs( d ) - GetHalfSize();
		int32_t cs;
		if( q.x > q.y && q.x > q.z ) { cs = 0; }
		else if( q.y > q.z ) { cs = 1; }
		else { cs = 2; }
		if( d[ cs ] > 0.0f ) { result[ cs ] = m_max[ cs ]; }
		else { result[ cs ] = m_min[ cs ]; }
	}
	if( containsOut )
	{
		*containsOut = !outside;
	}
	return result;
}

bool AABB::IntersectLine( Vec3 p, Vec3 d, float* t0Out, float* t1Out, ae::Vec3* n0Out, ae::Vec3* n1Out ) const
{
	float tMin = -INFINITY;
	float tMax = INFINITY;
	ae::Vec3 nMin, nMax;
	ae::Vec3 axes[] =
	{
		ae::Vec3( 1.0f, 0.0f, 0.0f ),
		ae::Vec3( 0.0f, 1.0f, 0.0f ),
		ae::Vec3( 0.0f, 0.0f, 1.0f ),
	};
	for( int32_t i = 0; i < 3; i++ )
	{
		if( ae::Abs( d[ i ] ) < 0.001f )
		{
			if( p[ i ] < m_min[ i ] || p[ i ] > m_max[ i ] )
			{
				return false;
			}
		}
		else
		{
			float ood = 1.0f / d[ i ];
			float t0 = ( m_min[ i ] - p[ i ] ) * ood;
			float t1 = ( m_max[ i ] - p[ i ] ) * ood;
			ae::Vec3 n0 = -axes[ i ];
			ae::Vec3 n1 = axes[ i ];
			if( t0 > t1 )
			{
				std::swap( t0, t1 );
				std::swap( n0, n1 );
			}
			if( t0 > tMin )
			{
				tMin = t0;
				nMin = n0;
			}
			if( t1 < tMax )
			{
				tMax = t1;
				nMax = n1;
			}
			if( tMin > tMax )
			{
				return false;
			}
		}
	}
	if( t0Out ) { *t0Out = tMin; }
	if( t1Out ) { *t1Out = tMax; }
	if( n0Out ) { *n0Out = nMin; }
	if( n1Out ) { *n1Out = nMax; }
	return true;
}

bool AABB::IntersectRay( Vec3 source, Vec3 ray, Vec3* hitOut, ae::Vec3* normOut, float* tOut ) const
{
	float t;
	if( IntersectLine( source, ray, &t, nullptr, normOut, nullptr ) && t <= 1.0f )
	{
		if( t >= 0.0f )
		{
			if( tOut ) { *tOut = t; }
			if( hitOut ) { *hitOut = source + ray * t; }
			return true;
		}
		else
		{
			bool inside;
			ae::Vec3 closest = GetClosestPointOnSurface( source, &inside );
			if( inside )
			{
				if( tOut ) { *tOut = 0.0f; }
				if( hitOut ) { *hitOut = source; }
				if( normOut ) { *normOut = ( closest - source ).SafeNormalizeCopy(); }
				return true;
			}
		}
		
	}
	return false;
}

std::ostream& operator<<( std::ostream& os, AABB aabb )
{
	return os << "[" << aabb.GetMin() << ", " << aabb.GetMax() << "]";
}

//------------------------------------------------------------------------------
// ae::OBB member functions
//------------------------------------------------------------------------------
OBB::OBB( const ae::Matrix4& transform )
{
	SetTransform( transform );
}

bool OBB::operator == ( const OBB& obb ) const
{
	return ( obb.m_center == m_center )
		&& ( obb.m_axes[ 0 ] == m_axes[ 0 ] )
		&& ( obb.m_axes[ 1 ] == m_axes[ 1 ] )
		&& ( obb.m_axes[ 2 ] == m_axes[ 2 ] )
		&& ( obb.m_halfSize == m_halfSize );
}

bool OBB::operator != ( const OBB& obb ) const
{
	return !( operator == ( obb ) );
}

void OBB::SetTransform( const ae::Matrix4& transform )
{
	m_center = transform.GetTranslation();
	m_axes[ 0 ] = transform.GetAxis( 0 );
	m_axes[ 1 ] = transform.GetAxis( 1 );
	m_axes[ 2 ] = transform.GetAxis( 2 );
	m_halfSize[ 0 ] = m_axes[ 0 ].Normalize() * 0.5f;
	m_halfSize[ 1 ] = m_axes[ 1 ].Normalize() * 0.5f;
	m_halfSize[ 2 ] = m_axes[ 2 ].Normalize() * 0.5f;
	if( m_halfSize[ 0 ] == 0.0f ) { m_axes[ 0 ] = m_axes[ 1 ].Cross( m_axes[ 2 ] ).SafeNormalizeCopy(); }
	else if( m_halfSize[ 1 ] == 0.0f ) { m_axes[ 1 ] = m_axes[ 2 ].Cross( m_axes[ 0 ] ).SafeNormalizeCopy(); }
	else if( m_halfSize[ 2 ] == 0.0f ) { m_axes[ 2 ] = m_axes[ 0 ].Cross( m_axes[ 1 ] ).SafeNormalizeCopy(); }
}

ae::Matrix4 OBB::GetTransform() const
{
	ae::Matrix4 result;
	result.SetAxis( 0, m_axes[ 0 ] * ( m_halfSize[ 0 ] * 2.0f ) );
	result.SetAxis( 1, m_axes[ 1 ] * ( m_halfSize[ 1 ] * 2.0f ) );
	result.SetAxis( 2, m_axes[ 2 ] * ( m_halfSize[ 2 ] * 2.0f ) );
	result.SetTranslation( m_center );
	result.SetRow( 3, ae::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
	return result;
}

float OBB::GetSignedDistanceFromSurface( ae::Vec3 p ) const
{
	p -= m_center;
	p = ae::Vec3( p.Dot( m_axes[ 0 ] ), p.Dot( m_axes[ 1 ] ), p.Dot( m_axes[ 2 ] ) );
	ae::Vec3 q = ae::Abs( p ) - m_halfSize;
	return ae::Max( q, ae::Vec3( 0.0f ) ).Length() + ae::Min( ae::Max( q.x, ae::Max( q.y, q.z ) ), 0.0f );
}

bool OBB::IntersectLine( Vec3 p, Vec3 d, float* t0Out, float* t1Out, ae::Vec3* n0Out, ae::Vec3* n1Out ) const
{
	float tfirst = -INFINITY;
	float tlast = INFINITY;
	ae::Vec3 n0, n1;
	ae::Plane sides[] =
	{
		{ m_center + m_axes[ 0 ] * m_halfSize[ 0 ], m_axes[ 0 ] },
		{ m_center + m_axes[ 1 ] * m_halfSize[ 1 ], m_axes[ 1 ] },
		{ m_center + m_axes[ 2 ] * m_halfSize[ 2 ], m_axes[ 2 ] },
		{ m_center - m_axes[ 0 ] * m_halfSize[ 0 ], -m_axes[ 0 ] },
		{ m_center - m_axes[ 1 ] * m_halfSize[ 1 ], -m_axes[ 1 ] },
		{ m_center - m_axes[ 2 ] * m_halfSize[ 2 ], -m_axes[ 2 ] },
	};
	for( uint32_t i = 0; i < countof(sides); i++ )
	{
		ae::Plane side = sides[ i ];
		float denom = d.Dot( side.GetNormal() );
		float dist = side.GetSignedDistance( p );
		if( ae::Abs( denom ) < 0.001f )
		{
			if( dist > 0.0f )
			{
				return false;
			}
		}
		else
		{
			float t = -dist / denom;
			if( denom < 0.0f )
			{
				if( t > tfirst )
				{
					tfirst = t;
					n0 = ( i < 3 ) ? m_axes[ i % 3 ] : -m_axes[ i % 3 ];
				}
			}
			else if( t < tlast )
			{
				tlast = t;
				n1 = ( i < 3 ) ? m_axes[ i % 3 ] : -m_axes[ i % 3 ];
			}
			if( tfirst > tlast )
			{
				return false;
			}
		}
	}
	if( t0Out ) { *t0Out = tfirst; }
	if( t1Out ) { *t1Out = tlast; }
	if( n0Out ) { *n0Out = n0; }
	if( n1Out ) { *n1Out = n1; }
	return true;
}

bool OBB::IntersectRay( Vec3 source, Vec3 ray, Vec3* hitOut, ae::Vec3* normOut, float* tOut ) const
{
	float t;
	if( IntersectLine( source, ray, &t, nullptr, normOut, nullptr ) && t <= 1.0f )
	{
		if( t >= 0.0f )
		{
			if( tOut ) { *tOut = t; }
			if( hitOut ) { *hitOut = source + ray * t; }
			return true;
		}
		else
		{
			bool inside;
			ae::Vec3 closest = GetClosestPointOnSurface( source, &inside );
			if( inside )
			{
				if( tOut ) { *tOut = 0.0f; }
				if( hitOut ) { *hitOut = source; }
				if( normOut ) { *normOut = ( closest - source ).SafeNormalizeCopy(); }
				return true;
			}
		}
		
	}
	return false;
}

Vec3 OBB::GetClosestPointOnSurface( Vec3 p, bool* containsOut ) const
{
	Vec3 result = m_center;
	const Vec3 d = p - m_center;
	const Vec3 l = Vec3( d.Dot( m_axes[ 0 ] ), d.Dot( m_axes[ 1 ] ), d.Dot( m_axes[ 2 ] ) );
	const Vec3 l2 = ae::Abs( l ) - m_halfSize;
	const float m = ae::Max( l2.x, l2.y, l2.z );
	if( m > 0.0f ) // Outside
	{
		for( uint32_t i = 0; i < 3; i++ )
		{
			float dist = d.Dot( m_axes[ i ] );
			if( dist > m_halfSize[ i ] )
			{
				dist = m_halfSize[ i ];
			}
			if( dist < -m_halfSize[ i ] )
			{
				dist = -m_halfSize[ i ];
			}
			result += m_axes[ i ] * dist;
		}
		if( containsOut ) { *containsOut = false; }
	}
	else // Inside
	{
		int32_t cs;
		if( l2.x > l2.y && l2.x > l2.z ) { cs = 0; }
		else if( l2.y > l2.z ) { cs = 1; }
		else { cs = 2; }
		for( uint32_t i = 0; i < 3; i++ )
		{
			float dist;
			if( i == cs )
			{
				dist = ( l[ i ] > 0.0f ) ? m_halfSize[ i ] : -m_halfSize[ i ];
			}
			else
			{
				dist = l[ i ];
			}
			result += m_axes[ i ] * dist;
		}
		if( containsOut ) { *containsOut = true; }
	}
	return result;
}

AABB OBB::GetAABB() const
{
	ae::Matrix4 transform = GetTransform();
	// @TODO: Only have to transform 4 of these and negate them in local space
	ae::Vec4 corners[] =
	{
		transform * ae::Vec4( -0.5f, -0.5f, -0.5f, 1.0f ),
		transform * ae::Vec4( 0.5f, -0.5f, -0.5f, 1.0f ),
		transform * ae::Vec4( 0.5f, 0.5f, -0.5f, 1.0f ),
		transform * ae::Vec4( -0.5f, 0.5f, -0.5f, 1.0f ),
		transform * ae::Vec4( -0.5f, -0.5f, 0.5f, 1.0f ),
		transform * ae::Vec4( 0.5f, -0.5f, 0.5f, 1.0f ),
		transform * ae::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ),
		transform * ae::Vec4( -0.5f, 0.5f, 0.5f, 1.0f ),
	};
	AABB result( corners[ 0 ].GetXYZ(), corners[ 1 ].GetXYZ() );
	for( uint32_t i = 2; i < countof( corners ); i++ )
	{
		result.Expand( corners[ i ].GetXYZ() );
	}
	return result;
}

//------------------------------------------------------------------------------
// Geometry helpers
//------------------------------------------------------------------------------
bool IntersectRayTriangle( Vec3 p, Vec3 ray, Vec3 a, Vec3 b, Vec3 c, bool ccw, bool cw, Vec3* pOut, Vec3* nOut, float* tOut )
{
	const ae::Vec3 ab = b - a;
	const ae::Vec3 ac = c - a;
	const ae::Vec3 n = ab.Cross( ac );
	const ae::Vec3 qp = -ray;
	
	// Compute denominator d
	const float d = qp.Dot( n );
	if( !ccw && d > 0.001f )
	{
		return false;
	}
	if( !cw && d < 0.001f )
	{
		return false;
	}
	// Parallel
	if( ae::Abs( d ) < 0.001f )
	{
		return false;
	}
	const float ood = 1.0f / d;
	
	// Compute intersection t value of pq with plane of triangle
	const ae::Vec3 ap = p - a;
	const float t = ap.Dot( n ) * ood;
	// Ray intersects if 0 <= t <= 1
	if( t < 0.0f || t > 1.0f )
	{
		return false;
	}
	
	// Compute barycentric coordinate components and test if within bounds
	const ae::Vec3 e = qp.Cross( ap );
	const float v = ac.Dot( e ) * ood;
	if( v < 0.0f || v > 1.0f )
	{
		return false;
	}
	const float w = -ab.Dot( e ) * ood;
	if( w < 0.0f || v + w > 1.0f )
	{
		return false;
	}
	
	// Result
	AE_DEBUG_ASSERT( !p.IsNAN() );
	if( pOut )
	{
		*pOut = p + ray * t;
	}
	if( nOut )
	{
		*nOut = n.SafeNormalizeCopy();
	}
	if( tOut )
	{
		*tOut = t;
	}
	return true;
}

ae::Vec3 ClosestPointOnTriangle( ae::Vec3 p, ae::Vec3 a, ae::Vec3 b, ae::Vec3 c )
{
	ae::Vec3 ab = b - a;
	ae::Vec3 ac = c - a;
	ae::Vec3 bc = c - b;
	
	// Compute parametric position s for projection P’ of P on AB,
	// P’ = A + s*AB, s = snom/(snom+sdenom)
	float snom = (p - a).Dot( ab );
	float sdenom = (p - b).Dot(a - b);
	
	// Compute parametric position t for projection P’ of P on AC,
	// P’ = A + t*AC, s = tnom/(tnom+tdenom)
	float tnom = (p - a).Dot( ac );
	float tdenom = (p - c).Dot( a - c);
	if(snom <= 0.0f && tnom <= 0.0f) return a; // Vertex region early out
	
	// Compute parametric position u for projection P’ of P on BC,
	// P’ = B + u*BC, u = unom/(unom+udenom)
	float unom = (p - b).Dot( bc ), udenom = (p - c).Dot(b - c);
	if(sdenom <= 0.0f && unom <= 0.0f) return b; // Vertex region early out
	if(tdenom <= 0.0f && udenom <= 0.0f) return c; // Vertex region early out
	
	// P is outside (or on) AB if the triple scalar product [N PA PB] <= 0
	ae::Vec3 n = (b - a).Cross(c - a);
	float vc = n.Dot((a - p).Cross(b - p));
	// If P outside AB and within feature region of AB,
	// return projection of P onto AB
	if(vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f)
		return a + snom / (snom + sdenom) * ab;
	
	// P is outside (or on) BC if the triple scalar product [N PB PC] <= 0
	float va = n.Dot((b - p).Cross(c - p));
	// If P outside BC and within feature region of BC,
	// return projection of P onto BC
	if(va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
	return b + unom / (unom + udenom) * bc;
	
	// P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
	float vb = n.Dot((c - p).Cross(a - p));
	// If P outside CA and within feature region of CA,
	// return projection of P onto CA
	if(vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f)
		return a + tnom / (tnom + tdenom) * ac;

	// P must project inside face region. Compute Q using barycentric coordinates
	float u = va / (va + vb + vc);
	float v = vb / (va + vb + vc);
	float w=1.0f-u-v; //=vc/(va+vb+vc)
	return u * a + v * b + w * c;
}

//------------------------------------------------------------------------------
// Log levels internal implementation
//------------------------------------------------------------------------------
const char* LogLevelNames[] =
{
	"FATAL",
	"ERROR",
	"WARN ",
	"INFO ",
	"DEBUG",
	"TRACE",
};

//------------------------------------------------------------------------------
// Log colors internal implementation
//------------------------------------------------------------------------------
const char* LogLevelColors[] =
{
	"\x1b[35m",
	"\x1b[31m",
	"\x1b[33m",
	"\x1b[32m",
	"\x1b[36m",
	"\x1b[94m",
};

//------------------------------------------------------------------------------
// Logging functions internal implementation
//------------------------------------------------------------------------------
bool _ae_logColors = false;

void _LogFormat( std::ostream& os, ae::LogSeverity severity, const char* filePath, uint32_t line, const char** tags, uint32_t tagCount, const char* message )
{
	char timeBuf[ 16 ];
	time_t t = time( nullptr );
	tm* lt = localtime( &t );
	timeBuf[ strftime( timeBuf, sizeof( timeBuf ), "%H:%M:%S", lt ) ] = '\0';

	const char* fileName = strrchr( filePath, '/' );
	if( fileName )
	{
		fileName++; // Remove end forward slash
	}
	else if( ( fileName = strrchr( filePath, '\\' ) ) )
	{
		fileName++; // Remove end backslash
	}
	else
	{
		fileName = filePath;
	}

	if( _ae_logColors )
	{
		os << "\x1b[90m";
	}
	os << timeBuf << " [" << ae::GetPID() << "] [";
	for( uint32_t i = 0; i < tagCount; i++ )
	{
		if( i > 0 )
		{
			os << " ";
		}
		os << tags[ i ];
	}
	os << "] ";

	if( _ae_logColors )
	{
		os << LogLevelColors[ (uint32_t)severity ];
	}
	os << LogLevelNames[ (uint32_t)severity ];
#if AE_ENABLE_SOURCE_INFO
	if( _ae_logColors )
	{
		os << "\x1b[90m";
	}
	os << " " << fileName << ":" << line;
#endif

	if( message && message[ 0 ] )
	{
		os << ": ";
	}
	if( _ae_logColors )
	{
		os << "\x1b[0m";
	}
	if( message )
	{
		os << message;
	}
}

#if _AE_WINDOWS_
void _LogImpl( ae::LogSeverity severity, const char* filePath, uint32_t line, const char** tags, uint32_t tagCount, const char* message )
{
	std::stringstream os;
	_LogFormat( os, severity, filePath, line, tags, tagCount, message );
	static bool s_logStdOut = !ae::IsDebuggerAttached();
	if( s_logStdOut )
	{
		printf( os.str().c_str() ); // std out
	}
	else
	{
		OutputDebugStringA( os.str().c_str() ); // visual studio debug output
	}
}
#else
void _LogImpl( ae::LogSeverity severity, const char* filePath, uint32_t line, const char** tags, uint32_t tagCount, const char* message )
{
	// @TODO: os_log_error() etc on OSX?
	_LogFormat( std::cout, severity, filePath, line, tags, tagCount, message );
	std::cout << std::endl;
}
#endif

void SetLogColorsEnabled( bool enabled )
{
	_ae_logColors = enabled;
}

void PopLogTag()
{
	ae::_ThreadLocals* threadLocals = ae::_ThreadLocals::Get();
	const bool canPopMessage = ( threadLocals->logTagStack.Length() > 0 );
	AE_DEBUG_ASSERT( canPopMessage );
	if( canPopMessage )
	{
		threadLocals->logTagStack.Remove( threadLocals->logTagStack.Length() - 1 );
	}
}

//------------------------------------------------------------------------------
// _DefaultAllocator class
//------------------------------------------------------------------------------
_DefaultAllocator::~_DefaultAllocator()
{
#if AE_MEMORY_CHECKS
	std::lock_guard< std::mutex > lock( m_allocLock );
	if( m_allocations.size() )
	{
		uint32_t leakCount = 0;
		for( const auto& allocation : m_allocations )
		{
			if( allocation.second.status == AllocStatus::Allocated )
			{
				leakCount++;
				if( leakCount == 1 )
				{
					AE_ERR( "Memory leak(s) detected:" );
				}
				if( leakCount <= 32 )
				{
					AE_ERR( "Leak tagged [#] (#B)", allocation.second.tag, allocation.second.bytes );
				}
				else
				{
					break;
				}
			}
		}
		if( leakCount )
		{
			const uint32_t displayCount = ae::Min( leakCount, 32u );
			AE_ERR( "Leak count: ##", displayCount, ( displayCount < leakCount ) ? "+" : "" ); // @TODO: More Info here
		}
	}
#endif
}

void* _DefaultAllocator::Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment )
{
	alignment = ae::Max( 2u, alignment );
#if _AE_WINDOWS_
	void* result = _aligned_malloc( bytes, alignment );
#elif _AE_OSX_
	void* result = malloc( bytes ); // @HACK: macosx clang c++11 does not have aligned alloc
#elif _AE_EMSCRIPTEN_
	void* result = malloc( bytes ); // Emscripten malloc always uses 8 byte alignment https://github.com/emscripten-core/emscripten/issues/10072
#else
	void* result = aligned_alloc( alignment, bytes );
#endif
#if AE_MEMORY_CHECKS
	std::lock_guard< std::mutex > lock( m_allocLock );
	auto iter = m_allocations.find( result );
	if( iter == m_allocations.end() )
	{
		m_allocations.insert( { result, AllocInfo( tag, bytes, AllocStatus::Allocated ) } );
	}
	else
	{
		AE_ASSERT_MSG( iter->second.status == AllocStatus::Freed, "Memory already allocated: #", result );
		iter->second.tag = tag;
		iter->second.bytes = bytes;
		iter->second.status = AllocStatus::Allocated;
	}
#endif
	return result;
}

void* _DefaultAllocator::Reallocate( void* data, uint32_t bytes, uint32_t alignment )
{
	alignment = ae::Max( 2u, alignment );
#if AE_MEMORY_CHECKS
	std::lock_guard< std::mutex > lock( m_allocLock );
	auto iter = m_allocations.find( data );
	AE_ASSERT_MSG( iter != m_allocations.end(), "Can't realloc, not allocated: #", data );
	AE_ASSERT_MSG( iter->second.status == AllocStatus::Allocated, "Can't realloc, already freed: #", data );
#endif
#if _AE_WINDOWS_
	void* result = _aligned_realloc( data, bytes, alignment );
#else
	void* result = realloc( data, bytes );
#endif
#if AE_MEMORY_CHECKS
	if( result != data )
	{
		iter->second.status = AllocStatus::Freed;
		AE_ASSERT_MSG( m_allocations.find( result ) == m_allocations.end(), "Memory already allocated: #", result );
		m_allocations.insert( { result, AllocInfo( iter->second.tag, bytes, AllocStatus::Allocated ) } );
	}
	else
	{
		iter->second.bytes = bytes;
	}
#endif
	return result;
}

void _DefaultAllocator::Free( void* data )
{
#if AE_MEMORY_CHECKS
	{
		std::lock_guard< std::mutex > lock( m_allocLock );
		auto iter = m_allocations.find( data );
		AE_ASSERT_MSG( iter != m_allocations.end(), "Tried to free unallocated memory: #", data );
		AE_ASSERT_MSG( iter->second.status == AllocStatus::Allocated, "Double freed: #", data );
		iter->second.status = AllocStatus::Freed;
	}
#endif
#if _AE_WINDOWS_
	_aligned_free( data );
#else
	free( data );
#endif
}

bool _DefaultAllocator::IsThreadSafe() const
{
	return true;
}

//------------------------------------------------------------------------------
// Allocator functions
//------------------------------------------------------------------------------
Allocator::~Allocator()
{
	if( ae::_Globals::Get()->allocator == this )
	{
		ae::_Globals::Get()->allocator = nullptr;
	}
}

void SetGlobalAllocator( Allocator* allocator )
{
	AE_ASSERT_MSG( allocator, "No allocator provided to ae::SetGlobalAllocator()" );
	AE_ASSERT_MSG( !ae::_Globals::Get()->allocator, "Call ae::SetGlobalAllocator() before making any allocations to use your own allocator" );
	ae::_Globals::Get()->allocatorThread = std::this_thread::get_id();
	ae::_Globals::Get()->allocatorIsThreadSafe = allocator->IsThreadSafe();
	ae::_Globals::Get()->allocator = allocator;
	ae::_Globals::Get()->allocatorInitialized = true;
}

Allocator* GetGlobalAllocator()
{
	if( !ae::_Globals::Get()->allocator )
	{
		AE_ASSERT_MSG( !ae::_Globals::Get()->allocatorInitialized, "Global Allocator has already been destroyed" );
		SetGlobalAllocator( &ae::_Globals::Get()->defaultAllocator );
	}
#if _AE_DEBUG_
	AE_ASSERT_MSG( ae::_Globals::Get()->allocatorIsThreadSafe || std::this_thread::get_id() == ae::_Globals::Get()->allocatorThread, "The specified global ae::Allocator is not thread safe and can only be accessed on the thread it was set on." );
#endif
	return ae::_Globals::Get()->allocator;
}

//------------------------------------------------------------------------------
// ae::TimeStep member functions
//------------------------------------------------------------------------------
TimeStep::TimeStep()
{
	m_stepCount = 0;
	m_timeStep = 0.0;
	m_sleepOverhead = 0.0;
	m_prevFrameLength = 0.0;

	SetTimeStep( 1.0f / 60.0f );
}

void TimeStep::SetTimeStep( float timeStep )
{
	AE_ASSERT_MSG( timeStep < 1.0f, "Invalid timestep: #sec", timeStep );
	m_timeStep = timeStep;
}

float TimeStep::GetTimeStep() const
{
	return m_timeStep;
}

uint32_t TimeStep::GetStepCount() const
{
	return m_stepCount;
}

float TimeStep::GetDt() const
{
	return m_prevFrameLength;
}

void TimeStep::SetDt( float sec )
{
	m_prevFrameLength = sec;
}

void TimeStep::Tick()
{
#if _AE_EMSCRIPTEN_
	// Frame rate of emscripten builds is controlled by the browser
	const bool allowSleep = false;
#else
	const bool allowSleep = ( m_timeStep > 0.0 );
#endif
	
	if( m_stepCount == 0 )
	{
		m_frameStart = ae::GetTime();
	}
	else if( !allowSleep )
	{
		double currentTime = ae::GetTime();
		m_sleepOverhead = 0.0;
		m_prevFrameLength = currentTime - m_frameStart;
		m_frameStart = ae::Max( m_frameStart, currentTime ); // Don't go backwards
	}
	else
	{
		double sleepStart = ae::GetTime();
		double execDuration = ae::Max( 0.0, sleepStart - m_frameStart ); // Prevent negative dt
		double sleepDuration = m_timeStep - ( execDuration + m_sleepOverhead );

		if( sleepDuration > 0.0 )
		{
#if _AE_WINDOWS_
			// See https://stackoverflow.com/questions/64633336/new-thread-sleep-behaviour-under-windows-10-october-update-20h2
			// Increase default system timer resolution
			MMRESULT result = timeBeginPeriod( 1 );
			AE_ASSERT( result == TIMERR_NOERROR );
#endif
			std::this_thread::sleep_for( std::chrono::duration< double >( sleepDuration ) );
#if _AE_WINDOWS_
			result = timeEndPeriod( 1 );
			AE_ASSERT( result == TIMERR_NOERROR );
#endif

			double sleepEnd = ae::GetTime();
			m_prevFrameLength = sleepEnd - m_frameStart;
			m_frameStart = ae::Max( m_frameStart, sleepEnd ); // Don't go backwards

			double sleepOverhead = ( sleepEnd - sleepStart ) - sleepDuration;
			m_sleepOverhead = ae::Lerp( m_sleepOverhead, sleepOverhead, 0.05 );
		}
		else
		{
			m_prevFrameLength = execDuration;
			m_frameStart = ae::Max( m_frameStart, sleepStart ); // Don't go backwards
		}
	}
	
	m_stepCount++;
}

//------------------------------------------------------------------------------
// ae::GetHash32 helper
//------------------------------------------------------------------------------
template<> uint32_t GetHash32( const ae::Int2& value )
{
	// NxN->N Pairing: https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
	uint32_t hash = (int16_t)value.x;
	hash = ( hash << 16 );
	return hash + (int16_t)value.y;
}
template<> uint32_t GetHash32( const ae::Int3& value )
{
	// Szudzik Pairing: https://dmauro.com/post/77011214305/a-hashing-function-for-x-y-z-coordinates
	const uint32_t i = ( value.x >= 0 ) ? ( 2 * value.x ) : ( -2 * value.x - 1 );
	const uint32_t j = ( value.y >= 0 ) ? ( 2 * value.y ) : ( -2 * value.y - 1 );
	const uint32_t k = ( value.z >= 0 ) ? ( 2 * value.z ) : ( -2 * value.z - 1 );
	const uint32_t ijk = ae::Max( i, j, k );
	uint32_t hash = ijk * ijk * ijk + ( 2 * ijk * k ) + k;
	if( ijk == k ) { const uint32_t ij = ae::Max( i, j ); hash += ij * ij; }
	if( j >= i ) { hash += i + j; }
	else { hash += j; }
	return hash;
}
template<> uint32_t GetHash32( const ae::Color& v ) { return ae::Hash32().HashType( v.data ).Get(); }

//------------------------------------------------------------------------------
// ae::GetHash64 helper
//------------------------------------------------------------------------------
template<> uint64_t GetHash64( const ae::Int2& value )
{
	// NxN->N Pairing: https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
	uint64_t hash = (uint32_t)value.x;
	hash = ( hash << 32ull );
	return hash + (uint32_t)value.y;
}
template<> uint64_t GetHash64( const ae::Int3& value )
{
	// Szudzik Pairing: https://dmauro.com/post/77011214305/a-hashing-function-for-x-y-z-coordinates
	const uint64_t i = ( value.x >= 0 ) ? ( 2 * value.x ) : ( -2 * value.x - 1 );
	const uint64_t j = ( value.y >= 0 ) ? ( 2 * value.y ) : ( -2 * value.y - 1 );
	const uint64_t k = ( value.z >= 0 ) ? ( 2 * value.z ) : ( -2 * value.z - 1 );
	const uint64_t ijk = ae::Max( i, j, k );
	uint64_t hash = ijk * ijk * ijk + ( 2 * ijk * k ) + k;
	if( ijk == k ) { const uint64_t ij = ae::Max( i, j ); hash += ij * ij; }
	if( j >= i ) { hash += i + j; }
	else { hash += j; }
	return hash;
}
template<> uint64_t GetHash64( const ae::Color& v ) { return ae::Hash64().HashType( v.data ).Get(); }

//------------------------------------------------------------------------------
// ae::OpaquePool member functions
//------------------------------------------------------------------------------
#define _AE_POOL_ELEMENT( _arr, _idx ) ( (uint8_t*)_arr + (intptr_t)_idx * m_objectSize )

OpaquePool::OpaquePool( const ae::Tag& tag, uint32_t objectSize, uint32_t objectAlignment, uint32_t size, bool paged ) :
	m_firstPage( tag, size )
{
	AE_ASSERT( tag != ae::Tag() );
	AE_ASSERT( size > 0 );
	m_tag = tag;
	m_pageSize = size;
	m_paged = paged;
	m_objectSize = objectSize;
	m_objectAlignment = objectAlignment;
	m_length = 0;
	m_seq = 0;
}

OpaquePool::~OpaquePool()
{
	AE_ASSERT_MSG( Length() == 0, "ae::OpaquePool can't be destructed while objects are allocated (count:# size:#)", Length(), m_objectSize );
}

void* OpaquePool::Allocate()
{
	Page* page = m_pages.FindFn( []( const Page* page ) { return page->freeList.HasFree(); } );
	if( !page )
	{
		if( !m_firstPage.node.GetList() )
		{
			AE_DEBUG_ASSERT( m_firstPage.freeList.Length() == 0 );
			page = &m_firstPage;
			page->objects = ae::Allocate( m_tag, m_pageSize * m_objectSize, m_objectAlignment );
			m_pages.Append( page->node );
		}
		else if( m_paged )
		{
			page = ae::New< Page >( m_tag, m_tag, m_pageSize );
			page->objects = ae::Allocate( m_tag, m_pageSize * m_objectSize, m_objectAlignment );
			m_pages.Append( page->node );
		}
	}
	if( page )
	{
		int32_t index = page->freeList.Allocate();
		AE_ASSERT( index >= 0 );
		m_length++;
		m_seq++;
		return _AE_POOL_ELEMENT( page->objects, index );
	}
	return nullptr;
}

void OpaquePool::Free( void* obj )
{
	if( !obj )
	{
		return;
	}
	AE_DEBUG_ASSERT( (intptr_t)obj % m_objectAlignment == 0 );

	int32_t index = -1;
	Page* page = m_pages.GetFirst();
	while( page )
	{
		index = (int32_t)( ( (uint8_t*)obj - (uint8_t*)page->objects ) / m_objectSize );
		bool found = ( 0 <= index && index < (int32_t)m_pageSize );
		if( found )
		{
			break;
		}
		page = page->node.GetNext();
	}
	if( page )
	{
#if _AE_DEBUG_
		AE_ASSERT( m_length > 0 );
		AE_ASSERT( _AE_POOL_ELEMENT( page->objects, index ) == obj );
		AE_ASSERT_MSG( page->freeList.IsAllocated( index ), "Can't Free() previously deleted object" );
		memset( obj, 0xDD, m_objectSize );
#endif
		page->freeList.Free( index );
		m_length--;
		m_seq++;

		if( page->freeList.Length() == 0 )
		{
			ae::Free( page->objects );
			if( page == &m_firstPage )
			{
				m_firstPage.node.Remove();
				m_firstPage.freeList.FreeAll();
			}
			else
			{
				ae::Delete( page );
			}
		}
		return;
	}
#if _AE_DEBUG_
	AE_FAIL_MSG( "Object '#' not found in pool '#:#:#:#'", obj, m_objectSize, m_objectAlignment, m_pageSize, m_paged );
#endif
}

void OpaquePool::FreeAll()
{
	Page* page = m_pages.GetLast();
	while( page )
	{
		Page* prev = page->node.GetPrev();
		ae::Free( page->objects );
		if( page == &m_firstPage )
		{
			m_firstPage.node.Remove();
			m_firstPage.freeList.FreeAll();
		}
		else
		{
			ae::Delete( page );
		}
		page = prev;
	}
	m_length = 0;
	m_seq++;
}

bool OpaquePool::HasFree() const
{
	return Length() < Size();
}

const void* OpaquePool::m_GetFirst() const
{
	if( const Page* page = m_pages.GetFirst() )
	{
		AE_DEBUG_ASSERT( m_length > 0 );
		AE_DEBUG_ASSERT( page->freeList.Length() );
		return _AE_POOL_ELEMENT( page->objects, page->freeList.GetFirst() );
	}
	AE_DEBUG_ASSERT( m_length == 0 );
	return nullptr;
}

const void* OpaquePool::m_GetNext( const Page*& page, const void* obj, uint32_t seq ) const
{
	if( !obj ) { return nullptr; }
	AE_DEBUG_ASSERT( page );
	if( m_seq != seq )
	{
		// Handle pool modifications since last iteration
		const uint8_t* pageObj = reinterpret_cast< const uint8_t* >( obj );
		page = m_pages.FindFn( [&]( const Page* page )
		{
			const uint8_t* pageStart = reinterpret_cast< const uint8_t* >( page->objects );
			const uint8_t* pageEnd = pageStart + m_pageSize * m_objectSize;
			return ( pageStart <= pageObj ) && ( pageObj < pageEnd );
		} );
	}
	while( page )
	{
		AE_DEBUG_ASSERT( m_length > 0 );
		AE_DEBUG_ASSERT( page->freeList.Length() );
		const int32_t index = (int32_t)( ( (uint8_t*)obj - (uint8_t*)page->objects ) / m_objectSize );
		const bool found = ( 0 <= index && index < (int32_t)m_pageSize );
		if( found )
		{
			AE_DEBUG_ASSERT( _AE_POOL_ELEMENT( page->objects, index ) == obj );
			AE_DEBUG_ASSERT_MSG( page->freeList.IsAllocated( index ), "Can't iterate with previously deleted object" );
			const int32_t next = page->freeList.GetNext( index );
			if( next >= 0 )
			{
				return _AE_POOL_ELEMENT( page->objects, next );
			}
		}
		page = page->node.GetNext();
		if( found && page )
		{
			// Given object is last element of previous page so return the first element on next page
			AE_DEBUG_ASSERT( page->freeList.Length() > 0 );
			const int32_t next = page->freeList.GetFirst();
			AE_DEBUG_ASSERT( 0 <= next && next < (int32_t)m_pageSize );
			return _AE_POOL_ELEMENT( page->objects, next );
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
// ae::Rect member functions
//------------------------------------------------------------------------------
Rect Rect::FromCenterAndSize( ae::Vec2 center, ae::Vec2 size )
{
	Rect rect;
	rect.ExpandPoint( center - size * 0.5f );
	rect.ExpandPoint( center + size * 0.5f );
	return rect;
}

Rect Rect::FromPoints( ae::Vec2 p0, ae::Vec2 p1 )
{
	Rect rect;
	rect.ExpandPoint( p0 );
	rect.ExpandPoint( p1 );
	return rect;
}

bool Rect::Contains( Vec2 pos ) const
{
	return ( m_min.x <= pos.x && pos.x <= m_max.x ) && ( m_min.y <= pos.y && pos.y <= m_max.y );
}

Vec2 Rect::Clip( Vec2 pos ) const
{
	return ae::Vec2( ae::Clip( pos.x, m_min.x, m_max.x ), ae::Clip( pos.y, m_min.y, m_max.y ) );
}

void Rect::ExpandPoint( Vec2 pos )
{
	m_min = ae::Min( m_min, pos );
	m_max = ae::Max( m_max, pos );
}

void Rect::ExpandEdge( Vec2 amount )
{
	m_min -= amount;
	m_max += amount;
	if( m_max.x < m_min.x ) { m_min.x = ( m_min.x + m_max.x ) * 0.5f; m_max.x = m_min.x; }
	if( m_max.y < m_min.y ) { m_min.y = ( m_min.y + m_max.y ) * 0.5f; m_max.y = m_min.y; }
}

bool Rect::GetIntersection( const Rect& other, Rect* intersectionOut ) const
{
	ae::Vec2 min = ae::Max( m_min, other.m_min );
	ae::Vec2 max = ae::Min( m_max, other.m_max );
	if( min.x <= max.x && min.y <= max.y )
	{
		if( intersectionOut )
		{
			intersectionOut->m_min = min;
			intersectionOut->m_max = max;
		}
		return true;
	}
	else
	{
		return false;
	}
}

Rect Rect::operator*( float s ) const
{
	Rect rect;
	rect.m_min = m_min * s;
	rect.m_max = m_max * s;
	return rect;
}

Rect Rect::operator/( float s ) const
{
	Rect rect;
	rect.m_min = m_min / s;
	rect.m_max = m_max / s;
	return rect;
}

Rect Rect::operator+( const Vec2& v ) const
{
	Rect rect;
	rect.m_min = m_min + v;
	rect.m_max = m_max + v;
	return rect;
}

Rect Rect::operator-( const Vec2& v ) const
{
	Rect rect;
	rect.m_min = m_min - v;
	rect.m_max = m_max - v;
	return rect;
}

Rect Rect::operator*( const Vec2& v ) const
{
	Rect rect;
	rect.m_min = m_min * v;
	rect.m_max = m_max * v;
	return rect;
}

Rect Rect::operator/( const Vec2& v ) const
{
	Rect rect;
	rect.m_min = m_min / v;
	rect.m_max = m_max / v;
	return rect;
}

void Rect::operator*=( float s )
{
	m_min *= s;
	m_max *= s;
}

void Rect::operator/=( float s )
{
	m_min /= s;
	m_max /= s;
}

void Rect::operator+=( const Vec2& v )
{
	m_min += v;
	m_max += v;
}

void Rect::operator-=( const Vec2& v )
{
	m_min -= v;
	m_max -= v;
}

void Rect::operator*=( const Vec2& v )
{
	m_min *= v;
	m_max *= v;
}

void Rect::operator/=( const Vec2& v )
{
	m_min /= v;
	m_max /= v;
}

//------------------------------------------------------------------------------
// ae::RectInt member functions
//------------------------------------------------------------------------------
RectInt RectInt::FromPointAndSize( ae::Int2 pos, ae::Int2 size )
{
	return FromPointAndSize( pos.x, pos.y, size.x, size.y );
}

RectInt RectInt::FromPointAndSize( int32_t x, int32_t y, int32_t w, int32_t h )
{
	RectInt rect;// @TODO: Handle negative width and height
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	return rect;
}

RectInt RectInt::FromPoints( ae::Int2 p0, ae::Int2 p1 )
{
	return FromPoints( p0.x, p0.y, p1.x, p1.y );
}

RectInt RectInt::FromPoints( int32_t x0, int32_t y0, int32_t x1, int32_t y1 )
{
	RectInt rect;
	rect.Expand( ae::Int2( x0, y0 ) );
	rect.Expand( ae::Int2( x1, y1 ) );
	return rect;
}

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
	if( w == 0 )
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

	if( h == 0 )
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
// ae::HotLoader member functions
//------------------------------------------------------------------------------
HotLoader::~HotLoader()
{
	Close();
}

void HotLoader::Initialize( const char* buildCmd, const char* postBuildCmd, const char* libPath )
{
	m_fns.Clear();
	Close();
	m_buildCmd = buildCmd;
	m_postBuildCmd = postBuildCmd;
	m_libPath = libPath;
	Reload();
}

void HotLoader::Reload()
{
	bool reload = true;
#if _AE_IOS_
	reload = false;
#else
	if( m_buildCmd.Length() )
	{
		AE_INFO( m_buildCmd.c_str() );
		reload = reload && ( system( m_buildCmd.c_str() ) == 0 );
	}
	if( m_postBuildCmd.Length() )
	{
		AE_INFO( m_postBuildCmd.c_str() );
		reload = reload && ( system( m_postBuildCmd.c_str() ) == 0 );
	}
#endif

	if( reload )
	{
		Close();

		AE_INFO( "Loading: '#'", m_libPath );
#if _AE_APPLE_
		m_dylib = dlopen( m_libPath.c_str(), RTLD_NOW | RTLD_LOCAL );
		AE_ASSERT_MSG( m_dylib, "dlopen() failed: #", dlerror() );
#else
		AE_FAIL_MSG( "HotLoader not implemented for this platform" );
#endif

		for( auto& fn : m_fns )
		{
#if _AE_APPLE_
			fn.value = dlsym( m_dylib, fn.key.c_str() );
			AE_ASSERT_MSG( fn.value, "dlsym( \"#\" ) failed: #", fn.key, dlerror() );
#endif
		}
	}
	else
	{
		AE_WARN( "Reload aborted" );
	}
}

void HotLoader::Close()
{
	for( auto& fn : m_fns )
	{
		fn.value = nullptr;
	}
	if( m_dylib )
	{
		AE_INFO( "Closing '#'", m_libPath );
#if _AE_APPLE_
		if( dlclose( m_dylib ) )
		{
			AE_FAIL_MSG( "dlclose() failed: #", dlerror() );
		}
#endif
		
#if _AE_APPLE_
		const bool isLoaded = dlopen( m_libPath.c_str(), RTLD_NOLOAD | RTLD_LOCAL );
		AE_ASSERT_MSG( !isLoaded, "Could not unload library '#'. See AE_EXPORT comments.", m_libPath );
#endif
		
		m_dylib = nullptr;
	}
}

void* HotLoader::m_LoadFn( const char* name )
{
#if _AE_APPLE_
	void* fn = m_fns.Set( name, dlsym( m_dylib, name ) );
	AE_ASSERT_MSG( fn, "Could not load function '#'", name );
	return fn;
#else
	return nullptr;
#endif
}

bool HotLoader::GetCMakeBuildCommand( ae::Str256* buildCmdOut, const char* cmakeBuildDir, const char* cmakeTargetName )
{
	if( buildCmdOut && cmakeBuildDir[ 0 ] && cmakeTargetName[ 0 ] )
	{
		*buildCmdOut = ae::Str256::Format( "cmake --build \"#\" --target #", cmakeBuildDir, cmakeTargetName );
		return true;
	}
	return false;
}

bool HotLoader::GetCopyCommand( ae::Str256* copyCmdOut, const char* dest, const char* src )
{
	if( copyCmdOut && dest[ 0 ] && src[ 0 ] )
	{
		*copyCmdOut = ae::Str256::Format( "cp \"#\" \"#\"", src, dest );
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
// ae::Window MSVC/Windows event callback
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
// @TODO: Cleanup namespace
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	ae::Window* window = (ae::Window*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
	switch( msg )
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
		case WM_MOVE:
		{
			ae::Int2 pos( (int16_t)LOWORD( lParam ), (int16_t)HIWORD( lParam ) );
			window->m_UpdatePos( pos );
			if( window->GetLoggingEnabled() ) { AE_INFO( "window pos #", pos ); }
			break;
		}
		case WM_SIZE:
		{
			uint32_t width = LOWORD( lParam );
			uint32_t height = HIWORD( lParam );
			window->m_UpdateSize( width, height, 1.0f ); // @TODO: Scale factor with PROCESS_DPI_AWARENESS
			switch( wParam )
			{
				case SIZE_MAXIMIZED:
					window->m_UpdateMaximized( true );
					if( window->GetLoggingEnabled() ) { AE_INFO( "maximize # #", width, height ); }
					break;
				default:
					if( window->GetLoggingEnabled() ) { AE_INFO( "unmaximize # #", width, height ); }
					window->m_UpdateMaximized( false );
					break;
			}
			break;
		}
		case WM_CLOSE:
		{
			PostQuitMessage( 0 );
			break;
		}
		default:
			break;
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
@property ae::Window* aeWindow;
@end
@implementation aeApplicationDelegate
- (void)applicationWillFinishLaunching:(NSNotification*)notification
{
	// Create the application menu bar
	NSMenu* menubar = [NSMenu new];
	[NSApp setMainMenu:menubar];
	
	// Create the button in the menu bar
	NSMenuItem* menuBarItem = [NSMenuItem new];
	[menubar addItem:menuBarItem];
	
	// Create the menu and its contents
	// @TODO: Currently this menu must be open for cmd+q to work, fix this
	NSMenu* appMenu = [NSMenu new];
	NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
	[appMenu addItem:quitMenuItem];
	[menuBarItem setSubmenu:appMenu];
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	// Makes sure applicationShouldTerminate will be called
	NSProcessInfo* processInfo = [NSProcessInfo processInfo];
	processInfo.automaticTerminationSupportEnabled = false;
	[processInfo disableSuddenTermination];
	
	// Prevents app run from blocking
	[NSApp stop:nil];
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	AE_ASSERT( _aeWindow );
	AE_ASSERT( _aeWindow->input );
	// @TODO: Prevent Q from being sent to the window?
	_aeWindow->input->quit = true;
	return NSTerminateCancel;
}
@end

//------------------------------------------------------------------------------
// ae::Window Objective-C aeWindowDelegate class
//------------------------------------------------------------------------------
@interface aeWindowDelegate : NSObject< NSWindowDelegate >
@property ae::Window* aeWindow;
@end
@implementation aeWindowDelegate
- (BOOL)windowShouldClose:(NSWindow *)sender
{
	AE_ASSERT( _aeWindow );
	AE_ASSERT( _aeWindow->input );
	_aeWindow->input->quit = true;
	return false;
}
- (void)windowDidResize:(NSWindow*)sender
{
	AE_ASSERT( _aeWindow );
	NSWindow* window = (NSWindow*)_aeWindow->window;
	AE_ASSERT( window );
	
	NSRect contentScreenRect = [window convertRectToScreen:[window contentLayoutRect]];
	_aeWindow->m_UpdatePos( ae::Int2( contentScreenRect.origin.x, contentScreenRect.origin.y ) );
	_aeWindow->m_UpdateSize( contentScreenRect.size.width, contentScreenRect.size.height, [window backingScaleFactor] );
	
	if( _aeWindow->input )
	{
		NSPoint mouseScreenPos = [NSEvent mouseLocation];
		_aeWindow->input->m_SetMousePos( ae::Int2( mouseScreenPos.x, mouseScreenPos.y ) );
	}
}
- (void)windowDidMove:(NSNotification *)notification
{
	AE_ASSERT( _aeWindow );
	NSWindow* window = (NSWindow*)_aeWindow->window;
	AE_ASSERT( window );
	
	NSRect contentScreenRect = [window convertRectToScreen:[window contentLayoutRect]];
	_aeWindow->m_UpdatePos( ae::Int2( contentScreenRect.origin.x, contentScreenRect.origin.y ) );
	
	// Check for input, because windowDidMove can be indirectly called by 
	if( _aeWindow->input )
	{
		NSPoint mouseScreenPos = [NSEvent mouseLocation];
		_aeWindow->input->m_SetMousePos( ae::Int2( mouseScreenPos.x, mouseScreenPos.y ) );
	}
}
- (void)windowDidBecomeKey:(NSNotification *)notification
{
	AE_ASSERT( _aeWindow );
	_aeWindow->m_UpdateFocused( true );
}
- (void)windowDidResignKey:(NSNotification *)notification
{
	AE_ASSERT( _aeWindow );
	_aeWindow->m_UpdateFocused( false );
}
@end
namespace ae {
#endif

//------------------------------------------------------------------------------
// ae::Screen functions
//------------------------------------------------------------------------------
#if _AE_EMSCRIPTEN_
void _aeEmscriptenGetCanvasInfo( int32_t* widthOut, int32_t* heightOut, float* scaleOut )
{
	double width, height;
	if( emscripten_get_element_css_size( "#canvas", &width, &height ) != EMSCRIPTEN_RESULT_SUCCESS )
	{
		AE_FAIL_MSG( "Failed to get canvas size" );
	}
	if( widthOut ) { *widthOut = width + 0.5; }
	if( heightOut ) { *heightOut = height + 0.5; }
	if( scaleOut ) { *scaleOut = emscripten_get_device_pixel_ratio(); }
}
#endif

ae::Array< ae::Screen, 16 > GetScreens()
{
	ae::Array< Screen, 16 > result;
#if _AE_OSX_
	NSArray< NSScreen* >* screens = [NSScreen screens];
	for( uint32_t i = 0; i < screens.count; i++ )
	{
		const NSScreen* screen = screens[ i ];
		Screen& s = result.Append( {} );
		s.position = ae::Int2( screen.frame.origin.x, screen.frame.origin.y );
		s.size = ae::Int2( screen.frame.size.width, screen.frame.size.height );
		s.scaleFactor = screen.backingScaleFactor;

		s.isPrimary = [screen isEqualTo:[NSScreen mainScreen]];

		const NSDictionary* description = [screen deviceDescription];
		const NSNumber* screenIDNumber = [description objectForKey:@"NSScreenNumber"];
		const CGDirectDisplayID screenID = [screenIDNumber unsignedIntValue];
		s.isExternal = !CGDisplayIsBuiltin( screenID );
	}
#elif _AE_WINDOWS_
	auto monitorEnumProc = []( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData ) -> BOOL
	{
		ae::Array< Screen, 16 >& result = *(ae::Array< Screen, 16 >*)dwData;
		MONITORINFO monitorInfo;
		memset( &monitorInfo, 0, sizeof( monitorInfo ) );
		monitorInfo.cbSize = sizeof( monitorInfo );
		if( GetMonitorInfo( hMonitor, &monitorInfo ) )
		{
			Screen& s = result.Append( {} );
			// Use MONITORINFO::rcWork so that the taskbar is accounted for
			s.position = ae::Int2( monitorInfo.rcWork.left, monitorInfo.rcWork.top );
			s.size = ae::Int2( monitorInfo.rcWork.right - monitorInfo.rcWork.left, monitorInfo.rcWork.bottom - monitorInfo.rcWork.top );
			return true;
		}
		result.Clear();
		return false;
	};
	if( !EnumDisplayMonitors( nullptr, nullptr, monitorEnumProc, (LPARAM)&result ) )
	{
		result.Clear();
	}
#elif _AE_EMSCRIPTEN_
	{
		Screen& s = result.Append( {} );
		s.position = ae::Int2( 0, 0 );
		_aeEmscriptenGetCanvasInfo( &s.size.x, &s.size.y, nullptr );
	}
#endif
	return result;
}

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
	m_scaleFactor = 1.0f;
}

bool Window::Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor, bool rememberPosition )
{
	AE_ASSERT( !window );

	m_width = width;
	m_height = height;
	m_fullScreen = false;

	// Center window on primary screen
	const ae::Array< ae::Screen, 16 > screens = ae::GetScreens();
	AE_ASSERT( screens.Length() > 0 );
	m_pos = ( screens[ 0 ].size - ae::Int2( width, height ) ) / 2;
	m_pos += screens[ 0 ].position;

	m_Initialize( rememberPosition );

	if( fullScreen )
	{
		SetFullScreen( true );
	}

	return true;
}

bool Window::Initialize( Int2 pos, uint32_t width, uint32_t height, bool showCursor, bool rememberPosition )
{
	AE_ASSERT( !window );

	m_pos = pos;
	m_width = width;
	m_height = height;
	m_fullScreen = false;

	m_Initialize( rememberPosition );

	return true;
}

void Window::m_UpdateSize( int32_t width, int32_t height, float scaleFactor )
{
	m_width = width;
	m_height = height;
	m_scaleFactor = scaleFactor;
}

void Window::m_UpdateFocused( bool focused )
{
	m_focused = focused;
	if( !m_focused && input )
	{
		// @TODO: Input::m_UpdateFocused()
		input->SetMouseCaptured( false );
		input->m_mousePosSet = false;
	}
}

ae::Int2 Window::m_aeToNative( ae::Int2 pos, ae::Int2 size )
{
#if _AE_WINDOWS_
	return ae::Int2( pos.x, GetSystemMetrics( SM_YVIRTUALSCREEN ) - pos.y + size.y );
#elif _AE_OSX_
	return ae::Int2( pos.x, pos.y );
#else
	return ae::Int2( 0 );
#endif
}

ae::Int2 Window::m_nativeToAe( ae::Int2 pos, ae::Int2 size )
{
#if _AE_WINDOWS_
	return ae::Int2( pos.x, GetSystemMetrics( SM_YVIRTUALSCREEN ) - ( pos.y - size.y ) );
#elif _AE_OSX_
	return ae::Int2( pos.x, pos.y );
#else
	return ae::Int2( 0 );
#endif
}

void Window::m_Initialize( bool rememberPosition )
{
#if _AE_WINDOWS_
#define WNDCLASSNAME L"wndclass"
	HINSTANCE hinstance = GetModuleHandle( NULL );

	WNDCLASSEX ex;
	memset( &ex, 0, sizeof( ex ) );
	ex.cbSize = sizeof( ex );
	ex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	ex.lpfnWndProc = WndProc;
	ex.hInstance = hinstance;
	ex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	ex.hCursor = LoadCursor( NULL, IDC_ARROW );
	ex.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	ex.lpszClassName = WNDCLASSNAME;
	if( !RegisterClassEx( &ex ) ) // Create the window
	{
		AE_FAIL_MSG( "Failed to register window. Error: #", GetLastError() );
	}

	// @TODO: WS_POPUP for full screen
	uint32_t windowStyle = WS_OVERLAPPEDWINDOW;
	windowStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	RECT windowRect;
	//windowRect.left = m_pos.x;
	//windowRect.right = m_pos.x + GetWidth();
	//windowRect.top = m_pos.y;
	//windowRect.bottom = m_pos.y + GetHeight();
	bool windowSuccess = false;// AdjustWindowRectEx( &windowRect, windowStyle, false, 0 );
	////AE_ASSERT( windowSuccess );
	////m_width = ( windowRect.right - windowRect.left );
	////m_height = ( windowRect.bottom - windowRect.top );
	//{
	//	LPRECT prc = &windowRect;
	//	int         w = prc->right - prc->left;
	//	int         h = prc->bottom - prc->top;

	//	// get the nearest monitor to the passed rect. 
	//	HMONITOR hMonitor = MonitorFromRect( prc, MONITOR_DEFAULTTONEAREST );

	//	// get the work area or entire monitor rect. 
	//	MONITORINFO mi;
	//	memset( &mi, 0, sizeof(mi) );
	//	mi.cbSize = sizeof( mi );
	//	GetMonitorInfo( hMonitor, &mi );

	//	RECT rc;
	//	//if( flags & MONITOR_WORKAREA )
	//		rc = mi.rcWork;
	//	//else
	//	//	rc = mi.rcMonitor;

	//	prc->left = max( rc.left, min( rc.right - w, prc->left ) );
	//	prc->top = max( rc.top, min( rc.bottom - h, prc->top ) );
	//	prc->right = prc->left + w;
	//	prc->bottom = prc->top + h;
	//}

	HWND hwnd = CreateWindowEx( 0, WNDCLASSNAME, L"Window", WS_OVERLAPPEDWINDOW, m_pos.x, m_pos.y, GetWidth(), GetHeight(), NULL, NULL, hinstance, this );
	AE_ASSERT_MSG( hwnd, "Failed to create window. Error: #", GetLastError() );

	windowSuccess = GetClientRect( hwnd, &windowRect );
	AE_ASSERT( windowSuccess );
	m_width = ( windowRect.right - windowRect.left );
	m_height = ( windowRect.bottom - windowRect.top );

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
	if( !DescribePixelFormat( hdc, indexPixelFormat, sizeof( pfd ), &pfd ) )
	{
		AE_FAIL_MSG( "Failed to read chosen pixel format. Error: #", GetLastError() );
	}
	AE_INFO( "Chosen Pixel format: #bit RGB #bit Depth",
		(int)pfd.cColorBits,
		(int)pfd.cDepthBits
	);
	if( !SetPixelFormat( hdc, indexPixelFormat, &pfd ) )
	{
		AE_FAIL_MSG( "Could not set window pixel format. Error: #", GetLastError() );
	}

	// Finish window setup
	ShowWindow( hwnd, SW_SHOW );
	SetForegroundWindow( hwnd ); // Slightly Higher Priority
	SetFocus( hwnd ); // Sets Keyboard Focus To The Window
	if( !UpdateWindow( hwnd ) )
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
	applicationDelegate.aeWindow = this;
	[NSApp setDelegate:applicationDelegate];
	
	// Make sure run is only called when executable is bundled, and is also only called once
	NSRunningApplication* currentApp = [NSRunningApplication currentApplication];
	if( [currentApp bundleIdentifier] && ![currentApp isFinishedLaunching] )
	{
		dispatch_async( dispatch_get_main_queue(), ^{ [NSApp activateIgnoringOtherApps:YES]; } );
	}

	// Main window
	aeWindowDelegate* windowDelegate = [[aeWindowDelegate alloc] init];
	windowDelegate.aeWindow = this;
	NSWindow* nsWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect( m_pos.x, m_pos.y, m_width, m_height )
		styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
		backing:NSBackingStoreBuffered
		defer:YES
	];
	nsWindow.delegate = windowDelegate;
	[nsWindow setColorSpace:[NSColorSpace sRGBColorSpace]];
	this->window = nsWindow;
	
	NSRect initFrame = [nsWindow contentRectForFrameRect:[nsWindow frame]];
	NSRect frame = NSMakeRect( m_pos.x, m_pos.y, m_width, m_height );
	if( !CGRectEqualToRect( initFrame, frame ) )
	{
		// Try once more to set window position, OSX doesn't do a good job of creating windows on second monitors
		[nsWindow setFrame:frame display:YES];
		frame = [nsWindow contentRectForFrameRect:[nsWindow frame]];
	}
	
	NSOpenGLPixelFormatAttribute openglProfile;
	if( ae::GLMajorVersion >= 4 )
	{
		openglProfile = NSOpenGLProfileVersion4_1Core;
	}
	else if( ae::GLMajorVersion >= 3 )
	{
		openglProfile = NSOpenGLProfileVersion3_2Core;
	}
	else
	{
		openglProfile = NSOpenGLProfileVersionLegacy;
	}
	// clang-format off
	NSOpenGLPixelFormatAttribute nsPixelAttribs[] =
	{
		NSOpenGLPFAAccelerated,
		NSOpenGLPFAClosestPolicy,
		NSOpenGLPFAOpenGLProfile, openglProfile,
		NSOpenGLPFABackingStore, YES,
		NSOpenGLPFAColorSize, 24, // @TODO: Allow 64bit size for wide color support (implicitly disables srgb)
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADoubleBuffer, YES,
		0
	};
	// clang-format on
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
	if( rememberPosition )
	{
		if( NSString* appName = [[NSProcessInfo processInfo] processName] )
		{
			// @TODO: Doesn't work on external monitors
			// https://stackoverflow.com/a/36992518/2423134
			[nsWindow setFrameAutosaveName:appName];
		}
	}
	
	NSRect contentScreenRect = [nsWindow convertRectToScreen:[nsWindow contentLayoutRect]];
	m_pos = ae::Int2( contentScreenRect.origin.x, contentScreenRect.origin.y );
	m_width = contentScreenRect.size.width;
	m_height = contentScreenRect.size.height;
	m_scaleFactor = nsWindow.backingScaleFactor;
	
	// This prevents keystrokes from being output to the terminal when running
	// as a console app.
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
#elif _AE_EMSCRIPTEN_
	_aeEmscriptenGetCanvasInfo( &m_width, &m_height, &m_scaleFactor );
	if( m_width == 300 && m_height == 150 )
	{
		AE_WARN( "Canvas size was not configured. Defaulting to WxH 100\%." );
		m_fixCanvasStyle = true;
	}
#endif
}

void Window::Terminate()
{
	// @TODO
}

int32_t Window::GetWidth() const
{
	return m_width;
}

int32_t Window::GetHeight() const
{
	return m_height;
}

void Window::SetTitle( const char* title )
{
	if( m_windowTitle != title )
	{
#if _AE_WINDOWS_
		if( window ) { SetWindowTextA( (HWND)window, title ); }
#elif _AE_OSX_
		if( window ) { ((NSWindow*)window).title = [NSString stringWithUTF8String:title]; }
#elif _AE_EMSCRIPTEN_
		emscripten_set_window_title( title );
#endif
		m_windowTitle = title;
	}
}

void Window::SetFullScreen( bool fullScreen )
{
	if( GetLoggingEnabled() ) { AE_INFO( "fullscreen #", fullScreen ); }
#if _AE_OSX_
	if( window )
	{
		m_fullScreen = fullScreen;
		NSWindow* nsWindow = (NSWindow*)window;
		const bool isFullScreen = ( ( [nsWindow styleMask] & NSFullScreenWindowMask ) == NSFullScreenWindowMask );
		if( m_fullScreen != isFullScreen )
		{
			[nsWindow toggleFullScreen:[NSApplication sharedApplication]];
		}
	}
#elif _AE_WINDOWS_
	if( window )
	{
		m_fullScreen = fullScreen; // First so triggered events can use this value
		HWND hwnd = (HWND)window;
		if( fullScreen )
		{
			WINDOWPLACEMENT wp;
			memset( &wp, 0, sizeof( wp ) );
			wp.length = sizeof( wp );
			if( GetWindowPlacement( hwnd, &wp ) )
			{
				const ae::Int2 restoreSize( wp.rcNormalPosition.right - wp.rcNormalPosition.left, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top );
				const ae::Int2 aeRestorePos = m_nativeToAe( ae::Int2( wp.rcNormalPosition.left, wp.rcNormalPosition.top ), restoreSize );
				m_restoreRect = ae::RectInt::FromPointAndSize( aeRestorePos, restoreSize );
			}
			if( GetLoggingEnabled() )
			{
				const ae::Int2 restorePos = m_restoreRect.GetPos();
				const ae::Int2 restoreSize = m_restoreRect.GetSize();
				if( GetLoggingEnabled() ) { AE_INFO( "restore rect # # # #", restorePos.x, restorePos.y, restoreSize.x, restoreSize.y ); }
			}

			RECT windowRect;
			GetWindowRect( hwnd, &windowRect );
			if( HMONITOR hMonitor = MonitorFromRect( &windowRect, MONITOR_DEFAULTTOPRIMARY ) )
			{
				MONITORINFO monitorInfo;
				memset( &monitorInfo, 0, sizeof( MONITORINFO ) );
				monitorInfo.cbSize = sizeof( MONITORINFO );
				if( GetMonitorInfo( hMonitor, &monitorInfo ) )
				{
					const RECT monitorRect = monitorInfo.rcMonitor;
					const ae::Int2 size( monitorRect.right - monitorRect.left, monitorRect.bottom - monitorRect.top );
					SetWindowLongPtr( hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP );
					SetWindowPos( hwnd, HWND_TOP, monitorRect.left, monitorRect.top, size.x, size.y, SWP_FRAMECHANGED );
				}
			}
		}
		else 
		{
			const ae::Int2 aeRestorePos = m_restoreRect.GetPos();
			const ae::Int2 restoreSize = m_restoreRect.GetSize();
			const ae::Int2 nativeRestorePos = m_aeToNative( aeRestorePos, restoreSize );
			SetWindowLongPtr( hwnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW );
			SetWindowPos( hwnd, nullptr, nativeRestorePos.x, nativeRestorePos.y, restoreSize.x, restoreSize.y, SWP_FRAMECHANGED );
			if( GetLoggingEnabled() ) { AE_INFO( "unfullscreen # # # #", aeRestorePos.x, aeRestorePos.y, restoreSize.x, restoreSize.y ); }
		}
	}
#elif _AE_EMSCRIPTEN_
	if( m_fullScreen != fullScreen )
	{
		if( fullScreen )
		{
			if( GetLoggingEnabled() ) { AE_INFO( "request full screen" ); }
			EmscriptenFullscreenStrategy strategy;
			memset( &strategy, 0, sizeof(strategy) );
			strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
			strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
			strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			emscripten_request_fullscreen_strategy( "canvas", true, &strategy );
		}
		else
		{
			if( GetLoggingEnabled() ) { AE_INFO( "exit full screen" ); }
			emscripten_exit_fullscreen();
		}
	}
#endif
}

void Window::SetPosition( Int2 pos )
{
//	if( window )
//	{
//		SDL_SetWindowPosition( (SDL_Window*)window, pos.x, pos.y );
//		m_pos = pos;
//	}
}

void Window::SetSize( uint32_t width, uint32_t height )
{
//	if( window )
//	{
//		SDL_SetWindowSize( (SDL_Window*)window, width, height );
//		m_width = width;
//		m_height = height;
//	}
}

void Window::SetMaximized( bool maximized )
{
#if _AE_WINDOWS_
	if( maximized )
	{
		ShowWindow( (HWND)window, SW_MAXIMIZE );
	}
	else
	{
		ShowWindow( (HWND)window, SW_RESTORE );
	}
	m_maximized = maximized;
#endif
}

void Window::SetAlwaysOnTop( bool alwaysOnTop )
{
#if _AE_WINDOWS_
	SetWindowPos( (HWND)window, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
#elif _AE_OSX_
	NSWindow* nsWindow = (NSWindow*)window;
	[nsWindow setLevel:( alwaysOnTop ? NSFloatingWindowLevel : NSNormalWindowLevel )];
#endif
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
	if( !_input.aeinput->m_text.empty() )
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
	if( self != nil )
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
	if([_keyInput respondsToSelector:selector])
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
void _aeEmscriptenTryNewFrame( Input* input )
{
	if( input->newFrame_HACK )
	{
		memcpy( input->m_keysPrev, input->m_keys, sizeof(input->m_keys) );
		input->mousePrev = input->mouse;
		input->mouse.movement = ae::Int2( 0 );
		input->m_touchesPrev = input->m_touches;
		for( ae::Touch& touch : input->m_touches )
		{
			touch.movement = ae::Int2( 0 );
		}
		input->newFrame_HACK = false;
	}
}

EM_BOOL _aeEmscriptenHandleKey( int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData )
{
	static ae::Key s_keyMap[ 255 ];
	static bool s_first = true;
	// First time this function is called only
	if( s_first )
	{
		s_first = false;
		memset( s_keyMap, 0, sizeof( s_keyMap ) );
		s_keyMap[ 8 ] = ae::Key::Backspace;
		s_keyMap[ 9 ] = ae::Key::Tab;
		s_keyMap[ 13 ] = ae::Key::Enter;
		s_keyMap[ 16 ] = ae::Key::LeftShift;
		s_keyMap[ 17 ] = ae::Key::LeftControl;
		s_keyMap[ 18 ] = ae::Key::LeftAlt;
		s_keyMap[ 19 ] = ae::Key::Pause;
		s_keyMap[ 20 ] = ae::Key::CapsLock;
		s_keyMap[ 27 ] = ae::Key::Escape;
		s_keyMap[ 32 ] = ae::Key::Space;
		s_keyMap[ 33 ] = ae::Key::PageUp;
		s_keyMap[ 34 ] = ae::Key::PageDown;
		s_keyMap[ 35 ] = ae::Key::End;
		s_keyMap[ 36 ] = ae::Key::Home;
		s_keyMap[ 37 ] = ae::Key::Left;
		s_keyMap[ 38 ] = ae::Key::Up;
		s_keyMap[ 39 ] = ae::Key::Right;
		s_keyMap[ 40 ] = ae::Key::Down;
		s_keyMap[ 45 ] = ae::Key::Insert;
		s_keyMap[ 46 ] = ae::Key::Delete;
		for( uint32_t i = 0; i <= 9; i++ )
		{
			s_keyMap[ 48 + i ] = (ae::Key)((int)ae::Key::Num0 + i);
		}
		for( uint32_t i = 0; i < 26; i++ )
		{
			s_keyMap[ 65 + i ] = (ae::Key)((int)ae::Key::A + i);
		}
		s_keyMap[ 91 ] = ae::Key::LeftSuper;
		s_keyMap[ 92 ] = ae::Key::RightSuper;
		for( uint32_t i = 0; i <= 9; i++ )
		{
			s_keyMap[ 96 + i ] = (ae::Key)((int)ae::Key::NumPad0 + i);
		}
		s_keyMap[ 106 ] = ae::Key::NumPadMultiply;
		s_keyMap[ 107 ] = ae::Key::NumPadPlus;
		s_keyMap[ 109 ] = ae::Key::NumPadMinus;
		s_keyMap[ 110 ] = ae::Key::NumPadPeriod;
		s_keyMap[ 111 ] = ae::Key::NumPadDivide;
		for( uint32_t i = 0; i < 12; i++ )
		{
			s_keyMap[ 112 + i ] = (ae::Key)((int)ae::Key::F1 + i);
		}
		s_keyMap[ 144 ] = ae::Key::NumLock;
		s_keyMap[ 145 ] = ae::Key::ScrollLock;
		s_keyMap[ 186 ] = ae::Key::Semicolon;
		s_keyMap[ 187 ] = ae::Key::Equals;
		s_keyMap[ 188 ] = ae::Key::Comma;
		s_keyMap[ 189 ] = ae::Key::Minus;
		s_keyMap[ 190 ] = ae::Key::Period;
		s_keyMap[ 191 ] = ae::Key::Slash;
		s_keyMap[ 192 ] = ae::Key::Tilde;
		s_keyMap[ 219 ] = ae::Key::LeftBracket;
		s_keyMap[ 220 ] = ae::Key::Backslash;
		s_keyMap[ 221 ] = ae::Key::RightBracket;
		s_keyMap[ 222 ] = ae::Key::Apostrophe;
	}

	// Start key handling
	AE_ASSERT( userData );
	Input* input = (Input*)userData;
	_aeEmscriptenTryNewFrame( input );

	if( keyEvent->which < countof(s_keyMap) && (int)s_keyMap[ keyEvent->which ] )
	{
		bool pressed = ( EMSCRIPTEN_EVENT_KEYUP != eventType );
		input->m_keys[ (int)s_keyMap[ keyEvent->which ] ] = pressed;
	}
	input->m_keys[ (int)ae::Key::RightShift ] = input->m_keys[ (int)ae::Key::LeftShift ];
	input->m_keys[ (int)ae::Key::RightControl ] = input->m_keys[ (int)ae::Key::LeftControl ];
	input->m_keys[ (int)ae::Key::RightAlt ] = input->m_keys[ (int)ae::Key::LeftAlt ];
	input->m_UpdateModifiers();
	return true;
}

EM_BOOL _aeEmscriptenHandleMouse( int32_t eventType, const EmscriptenMouseEvent* mouseEvent, void* userData )
{
	AE_ASSERT( userData );
	Input* input = (Input*)userData;
	_aeEmscriptenTryNewFrame( input );
	
	switch( eventType )
	{
		case EMSCRIPTEN_EVENT_MOUSEENTER: // @TODO: It seems like this event is never received
			input->m_window->m_UpdateFocused( true );
			break;
		case EMSCRIPTEN_EVENT_MOUSELEAVE: // @TODO: It seems like this event is never received
			input->m_window->m_UpdateFocused( false );
			break;
		default:
			break;
	}
	
	const ae::Vec2 pos = ae::Vec2( mouseEvent->targetX, input->m_window->GetHeight() - mouseEvent->targetY );
	input->m_SetMousePos( pos.FloorCopy(), ae::Int2( mouseEvent->movementX, -mouseEvent->movementY ) );
	input->mouse.leftButton = ( mouseEvent->buttons & 1 );
	input->mouse.rightButton = ( mouseEvent->buttons & 2 );
	input->mouse.middleButton = ( mouseEvent->buttons & 4 );
	
	return true;
}

EM_BOOL _aeEmscriptenHandleLockChange( int eventType, const EmscriptenPointerlockChangeEvent* pointerlockChangeEvent, void* userData )
{
	AE_ASSERT( eventType == EMSCRIPTEN_EVENT_POINTERLOCKCHANGE );
	Input* input = (Input*)userData;
	input->m_SetMouseCaptured( pointerlockChangeEvent->isActive );
	return true;
}

EM_BOOL _aeEmscriptenHandleTouch( int eventType, const EmscriptenTouchEvent* touchEvent, void* userData )
{
	AE_ASSERT( userData );
	Input* input = (Input*)userData;
	_aeEmscriptenTryNewFrame( input );

	for( uint32_t i = 0; i < touchEvent->numTouches; i++ )
	{
		const EmscriptenTouchPoint* emTouch = &touchEvent->touches[ i ];
		if( emTouch->isChanged )
		{
			ae::Int2 pos( emTouch->targetX, emTouch->targetY );
			pos.y = input->m_window->GetHeight() - pos.y;
			switch( eventType )
			{
				case EMSCRIPTEN_EVENT_TOUCHSTART:
				{
					if( input->m_touches.Length() < input->m_touches.Size() )
					{
						ae::Touch* touch = &input->m_touches.Append( {} );
						touch->id = emTouch->identifier;
						touch->startPosition = pos;
						touch->position = pos;
					}
					break;
				}
				case EMSCRIPTEN_EVENT_TOUCHEND:
				{
					const int32_t touchIdx = input->m_touches.FindFn( [&]( const ae::Touch& t ){ return t.id == emTouch->identifier; } );
					if( touchIdx >= 0 ) { input->m_touches.Remove( touchIdx ); }
					break;
				}
				case EMSCRIPTEN_EVENT_TOUCHCANCEL:
				{
					const int32_t touchIdx = input->m_touches.FindFn( [&]( const ae::Touch& t ){ return t.id == emTouch->identifier; } );
					const int32_t prevTouchIdx = input->m_touchesPrev.FindFn( [&]( const ae::Touch& t ){ return t.id == emTouch->identifier; } );
					if( touchIdx >= 0 ) { input->m_touches.Remove( touchIdx ); }
					if( prevTouchIdx >= 0 ) { input->m_touchesPrev.Remove( prevTouchIdx ); }
					break;
				}
				case EMSCRIPTEN_EVENT_TOUCHMOVE:
				{
					const int32_t touchIdx = input->m_touches.FindFn( [&]( const ae::Touch& t ){ return t.id == emTouch->identifier; } );
					const int32_t prevTouchIdx = input->m_touchesPrev.FindFn( [&]( const ae::Touch& t ){ return t.id == emTouch->identifier; } );
					if( touchIdx >= 0 )
					{
						input->m_touches[ touchIdx ].position = pos;
						if( prevTouchIdx >= 0 )
						{
							input->m_touches[ touchIdx ].movement += pos - input->m_touchesPrev[ prevTouchIdx ].position;
						}
					}
					break;
				}
				default:
					break;
			}
		}
	}

	return true;
}

EM_BOOL _aeEmscriptenHandleFullScreen( int eventType, const EmscriptenFullscreenChangeEvent* fullscreenChangeEvent, void* userData )
{
	AE_ASSERT( userData );
	AE_ASSERT( eventType == EMSCRIPTEN_EVENT_FULLSCREENCHANGE );
	ae::Window* window = ( (Input*)userData )->m_window;
	if( window->GetLoggingEnabled() ) { AE_INFO( "full screen # -> #", ( window->GetFullScreen() ? "true" : "false" ), ( fullscreenChangeEvent->isFullscreen ? "true" : "false" ) ); }
	window->m_UpdateFullScreen( fullscreenChangeEvent->isFullscreen );
	return true;
}

#endif

Input::Input()
{
	memset( m_keys, 0, sizeof(m_keys) );
	memset( m_keysPrev, 0, sizeof(m_keysPrev) );
}

void Input::Initialize( Window* window )
{
	m_window = window;
	if( window )
	{
		window->input = this;
	}
	memset( m_keys, 0, sizeof(m_keys) );
	memset( m_keysPrev, 0, sizeof(m_keysPrev) );

	AE_STATIC_ASSERT( countof(gamepads) == countof(gamepadsPrev) );
	for( uint32_t i = 0; i < countof(gamepads); i++ )
	{
		gamepads[ i ].playerIndex = i;
		gamepadsPrev[ i ].playerIndex = i;
	}

#if _AE_EMSCRIPTEN_
	emscripten_set_keydown_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleKey );
	emscripten_set_keyup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleKey );
	emscripten_set_mousedown_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleMouse );
	emscripten_set_mouseup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleMouse );
	emscripten_set_mousemove_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleMouse );
	emscripten_set_mouseenter_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleMouse );
	emscripten_set_mouseleave_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleMouse );
	emscripten_set_touchstart_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleTouch );
	emscripten_set_touchend_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleTouch );
	emscripten_set_touchmove_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleTouch );
	emscripten_set_touchcancel_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleTouch );
	emscripten_set_fullscreenchange_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleFullScreen );
	emscripten_set_pointerlockchange_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, &_aeEmscriptenHandleLockChange );
#elif _AE_OSX_
	aeTextInputDelegate* textInput = [[aeTextInputDelegate alloc] initWithFrame: NSMakeRect(0.0, 0.0, 0.0, 0.0)];
	textInput.aeinput = this;
	m_textInputHandler = textInput;
	NSWindow* nsWindow = (NSWindow*)m_window->window;
	NSView* nsWindowContent = [nsWindow contentView];
	[nsWindowContent addSubview:textInput];
	
	// Do this here so input is ready to handle events
	[nsWindow makeKeyAndOrderFront:nil]; // nil sender
	[nsWindow orderFrontRegardless];
	[GCController setShouldMonitorBackgroundEvents: YES];
	[[NSApplication sharedApplication] run];
#endif

	Pump(); // Pump once to process any system window creation events
}

void Input::Terminate()
{}

void Input::Pump()
{
	m_timeStep.Tick();
#if _AE_EMSCRIPTEN_
	_aeEmscriptenTryNewFrame( this );
	newFrame_HACK = true;
#else
	// Clear keys each frame and then check for presses below
	// Emscripten doesn't do this because it uses a callback to set m_keys
	memcpy( m_keysPrev, m_keys, sizeof(m_keys) );
	memset( m_keys, 0, sizeof(m_keys) );
	mousePrev = mouse;
	mouse.movement = ae::Int2( 0 );
	mouse.scroll = ae::Vec2( 0.0f );
	m_touchesPrev = m_touches;
	for( ae::Touch& touch : m_touches )
	{
		touch.movement = ae::Int2( 0 );
	}
#endif
	m_textInput = ""; // Clear last frames text input

	// Handle system events
#if _AE_WINDOWS_
	m_window->m_UpdateFocused( m_window->window == GetFocus() );
	// @TODO: Use GameInput https://docs.microsoft.com/en-us/gaming/gdk/_content/gc/input/porting/input-porting-xinput#optimizingSection
#pragma warning( push )
#pragma warning( disable : 4995 ) // Disable deprecation warnings for XInput
	XInputEnable( m_window->GetFocused() );
#pragma warning( pop )
	MSG msg; // Get messages for current thread
	while( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
	{
		if( msg.message == WM_QUIT )
		{
			quit = true;
		}
		else if( m_window->GetFocused() )
		{
			switch( msg.message )
			{
				case WM_LBUTTONDOWN:
					mouse.leftButton = true;
					break;
				case WM_LBUTTONUP:
					mouse.leftButton = false;
					break;
				case WM_MBUTTONDOWN:
					mouse.middleButton = true;
					break;
				case WM_MBUTTONUP:
					mouse.middleButton = false;
					break;
				case WM_RBUTTONDOWN:
					mouse.rightButton = true;
					break;
				case WM_RBUTTONUP:
					mouse.rightButton = false;
					break;
				case WM_MOUSEWHEEL:
					mouse.scroll.y += GET_WHEEL_DELTA_WPARAM( msg.wParam ) / (float)WHEEL_DELTA;
					break;
				case WM_MOUSEHWHEEL:
					mouse.scroll.x += GET_WHEEL_DELTA_WPARAM( msg.wParam ) / (float)WHEEL_DELTA;
					break;
				case WM_CHAR:
				{
					char c[ MB_LEN_MAX ];
					if( wctomb( c, (wchar_t)msg.wParam ) == 1 && isprint( c[ 0 ] ) )
					{
						m_text += c[ 0 ];
						m_textInput += c[ 0 ];
					}
					break;
				}
				case WM_KEYDOWN:
					if( msg.wParam == VK_RETURN )
					{
						m_text += '\n';
						m_textInput += '\n';
					}
					else if( msg.wParam == VK_TAB )
					{
						m_text += '\t';
						m_textInput += '\t';
					}
					else if( msg.wParam == VK_BACK && !m_text.empty() )
					{
						// Don't modify m_textInput on backspace presses, it only stores incoming printable keys and is cleared each frame
						m_text.pop_back();
					}
					break;
			}
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	// Update mouse pos
	bool mouseJustSet = false; // Don't enable m_mousePosSet because m_SetMousePos() checks it
	if( m_window )
	{
		POINT mouseWindowPt;
		if( GetCursorPos( &mouseWindowPt ) )
		{
			if( ScreenToClient( (HWND)m_window->window, &mouseWindowPt ) )
			{
				ae::RectInt windowRect = ae::RectInt::FromPointAndSize( 0, 0, m_window->GetWidth(), m_window->GetHeight() );
				ae::Int2 localMouse( mouseWindowPt.x, m_window->GetHeight() - mouseWindowPt.y );
				if( windowRect.Contains( localMouse ) )
				{
					m_SetMousePos( localMouse );
					mouseJustSet = true;
				}
			}
		}
	}
	if( !mouseJustSet )
	{
		m_mousePosSet = false;
	}
#elif _AE_OSX_
	@autoreleasepool
	{
		while( true )
		{
			NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
				untilDate:[NSDate distantPast]
				inMode:NSDefaultRunLoopMode
				dequeue:YES];
			if( event == nil )
			{
				break;
			}
			
			// Cursor
			const ae::RectInt windowRect = ae::RectInt::FromPointAndSize(
				0,
				0,
				m_window->GetWidth(),
				m_window->GetHeight() - 4 );
			const NSPoint cursorScreenPos = [NSEvent mouseLocation];
			const ae::Int2 cursorLocalPos = ae::Int2( cursorScreenPos.x, cursorScreenPos.y ) - m_window->GetPosition();
			const bool cursorWithinWindow = windowRect.Contains( cursorLocalPos );
			if( cursorWithinWindow )
			{
				m_SetMousePos( cursorLocalPos );
			}

			bool anyClick = false;
			switch( event.type )
			{
				// @NOTE: Move events are not sent if any mouse button is clicked
				case NSEventTypeMouseMoved:
				case NSEventTypeLeftMouseDragged:
				case NSEventTypeRightMouseDragged:
				case NSEventTypeOtherMouseDragged:
					if( cursorWithinWindow )
					{
						mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
					}
					break;
				case NSEventTypeLeftMouseDown:
					if( cursorWithinWindow )
					{
						mouse.leftButton = true;
						mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
						anyClick = true;
					}
					break;
				case NSEventTypeLeftMouseUp:
					mouse.leftButton = false;
					mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
					break;
				case NSEventTypeRightMouseDown:
					if( cursorWithinWindow )
					{
						mouse.rightButton = true;
						mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
						anyClick = true;
					}
					break;
				case NSEventTypeRightMouseUp:
					mouse.rightButton = false;
					mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
					break;
				case NSEventTypeOtherMouseDown:
					if( cursorWithinWindow )
					{
						mouse.middleButton = true;
						mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
						anyClick = true;
					}
					break;
				case NSEventTypeOtherMouseUp:
					mouse.middleButton = false;
					mouse.usingTouch = ( event.subtype == NSEventSubtypeTouch );
					break;
				case NSEventTypeScrollWheel:
					if( cursorWithinWindow )
					{
						mouse.usingTouch = [event hasPreciseScrollingDeltas]; // @NOTE: Scroll is never NSEventSubtypeTouch
						float mult = mouse.usingTouch ? m_timeStep.GetDt() : 1.0f;
						mouse.scroll.x += event.scrollingDeltaX * mult;
						mouse.scroll.y += event.scrollingDeltaY * mult;
					}
					break;
				default:
					break;
			}
			
			// By default only left click activates the window, so force activation on middle and right click
			if( cursorWithinWindow && anyClick && !m_window->GetFocused() )
			{
				[NSApp activateIgnoringOtherApps:YES];
			}
			
			// Keyboard
			switch( event.type )
			{
				case NSEventTypeKeyDown:
					if( m_textMode )
					{
						[(aeTextInputDelegate*)m_textInputHandler interpretKeyEvents:[NSArray arrayWithObject:event]];
					}
					if( event.keyCode == kVK_ANSI_Q && ( event.modifierFlags & NSEventModifierFlagCommand ) )
					{
						break; // All 'Quit' key combos should propagate to the system
					}
					continue; // Don't propagate keyboard events or OSX will make the clicking error sound
				default:
					break;
			}
			[NSApp sendEvent:event];
		}
	}
#elif _AE_EMSCRIPTEN_
	{
		if( m_window->m_fixCanvasStyle )
		{
			// @NOTE: This is a hack to fix the canvas style when the size is
			// not specified by the web page. This is needed in particular for
			// the default Emscripten template.
			EM_ASM( { document.getElementById('canvas').style.width = '100%'; } );
			EM_ASM( { document.getElementById('canvas').style.height = '100%'; } );
			// This is needed in addition to the width and height above to fix the
			// aspect ratio when the canvas is resized so the height is not zero.
			EM_ASM( { document.getElementById('canvas').style.aspectRatio = '2 / 1'; } );
		}

		if( m_hideCursor )
		{
			EM_ASM( { document.getElementById('canvas').style.cursor = 'none'; } );
		}
		else
		{
			EM_ASM( { document.getElementById('canvas').style.cursor = 'auto'; } );
		}

		int32_t width, height;
		float scale;
		_aeEmscriptenGetCanvasInfo( &width, &height, &scale );
		m_window->m_UpdateSize( width, height, scale );
	}
#endif

#if !_AE_EMSCRIPTEN_
	// Mouse capture
	if( m_captureMouse )
	{
		mouse.movement = ae::Int2( 0 );
		if( m_window )
		{
			// Calculate center in case the window height is an odd number
			ae::Int2 localCenter( m_window->GetWidth() / 2, m_window->GetHeight() / 2 );
			m_SetCursorPos( localCenter );
			// Mouse pos is previously set elsewhere, so when the mouse position is set
			// to the window center the movement vector needs to be reversed.
			m_SetMousePos( localCenter );
			mouse.movement = -mouse.movement;
		}
	}
#endif

#if _AE_WINDOWS_
#define AE_UPDATE_KEY( _aek, _vk ) m_keys[ (int)ae::Key::_aek ] = keyStates[ _vk ] & ( 1 << 7 )
	uint8_t keyStates[ 256 ];
	if( m_window->GetFocused() && GetKeyboardState( keyStates ) )
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
		for( uint32_t i = 0; i <= ('9' - '1'); i++ )
		{
			AE_UPDATE_KEY( Num1 + i, '1' + i );
		}
		AE_UPDATE_KEY( Num0, '0' );
		for( uint32_t i = 0; i <= ('Z' - 'A'); i++ )
		{
			AE_UPDATE_KEY( A + i, 'A' + i );
		}
		AE_UPDATE_KEY( LeftSuper, VK_LWIN );
		AE_UPDATE_KEY( RightSuper, VK_RWIN );
		// AE_UPDATE_KEY( ?, VK_APPS );
		// AE_UPDATE_KEY( ?, VK_SLEEP );
		for( uint32_t i = 0; i <= (VK_NUMPAD9 - VK_NUMPAD1); i++ )
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
		for( uint32_t i = 0; i <= (VK_F12 - VK_F1); i++ )
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
	if( [(NSWindow*)m_window->window isMainWindow] )
	{
#define AE_UPDATE_KEY( _aek, _vk ) m_keys[ (int)ae::Key::_aek ] = keyStates[ _vk / 32 ].bigEndianValue & ( 1 << ( _vk % 32 ) )
		KeyMap keyStates;
		GetKeys( keyStates );
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
	}
#endif

	m_UpdateModifiers();

	// Reset gamepad states
	for( uint32_t i = 0; i < countof(gamepads); i++ )
	{
		gamepadsPrev[ i ] = gamepads[ i ];
		gamepads[ i ] = GamepadState();
		gamepads[ i ].playerIndex = i;
	}

#if _AE_WINDOWS_
	{
		auto& gp = this->gamepads[ 0 ];
		DWORD i = 0;
		// for( DWORD i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			XINPUT_STATE state;
			ZeroMemory( &state, sizeof(state) );
			DWORD dwResult = XInputGetState( i, &state );
			if( dwResult == ERROR_SUCCESS )
			{
				const XINPUT_GAMEPAD& gamepad = state.Gamepad;

				gp.connected = true;
				
				gp.leftAnalog = Vec2( gamepad.sThumbLX / 32767.0f, gamepad.sThumbLY / 32767.0f );
				gp.rightAnalog = Vec2( gamepad.sThumbRX / 32767.0f, gamepad.sThumbRY / 32767.0f );
				
				gp.up = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
				gp.down = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				gp.left = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				gp.right = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
				
				gp.start = gamepad.wButtons & XINPUT_GAMEPAD_START;
				gp.select = gamepad.wButtons & XINPUT_GAMEPAD_BACK;
				gp.a = gamepad.wButtons & XINPUT_GAMEPAD_A;
				gp.b = gamepad.wButtons & XINPUT_GAMEPAD_B;
				gp.x = gamepad.wButtons & XINPUT_GAMEPAD_X;
				gp.y = gamepad.wButtons & XINPUT_GAMEPAD_Y;
				gp.leftBumper = gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
				gp.rightBumper = gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
				gp.leftTrigger = gamepad.bLeftTrigger / 255.0f;
				gp.rightTrigger = gamepad.bRightTrigger / 255.0f;
				gp.leftAnalogClick = gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
				gp.rightAnalogClick = gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;

				XINPUT_BATTERY_INFORMATION batteryInfo;
				ZeroMemory( &batteryInfo, sizeof(batteryInfo) );
				dwResult = XInputGetBatteryInformation( i, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo );
				if( dwResult == ERROR_SUCCESS )
				{
					switch( batteryInfo.BatteryType )
					{
						case BATTERY_TYPE_WIRED:
							gp.batteryState = GamepadState::BatteryState::Wired;
							break;
						case BATTERY_TYPE_ALKALINE:
						case BATTERY_TYPE_NIMH:
							// @TODO: How to detect BatteryState::Charging?
							gp.batteryState = GamepadState::BatteryState::InUse;
							break;
						default:
							gp.batteryState = GamepadState::BatteryState::None;
							break;
					}
					switch( gp.batteryState )
					{
						case GamepadState::BatteryState::Wired:
							gp.batteryLevel = 1.0f;
							break;
						case GamepadState::BatteryState::InUse:
						case GamepadState::BatteryState::Charging:
							switch( batteryInfo.BatteryLevel )
							{
								case BATTERY_LEVEL_LOW:
									gp.batteryLevel = 0.25f;
									break;
								case BATTERY_LEVEL_MEDIUM:
									gp.batteryLevel = 0.5f;
									break;
								case BATTERY_LEVEL_FULL:
									gp.batteryState = GamepadState::BatteryState::Full;
									gp.batteryLevel = 1.0f;
									break;
								default:
									gp.batteryLevel = 0.0f;
									break;
							}
							break;
						default:
							break;
					}
				}
			}
		}
	}
#elif _AE_APPLE_
	{
		const NSArray< GCController* >* controllers = [GCController controllers];
		const uint32_t controllerCount = (uint32_t)[controllers count];
		auto GetAppleControllerFn = [&]( int32_t playerIndex ) -> const GCController*
		{
			for( uint32_t i = 0; i < controllerCount; i++ )
			{
				if( controllers[ i ].playerIndex == playerIndex )
				{
					return controllers[ i ];
				}
			}
			return nullptr;
		};

		// Assign player indices to newly connected gamepads
		for( uint32_t i = 0; i < controllerCount; i++ )
		{
			GCController* appleController = controllers[ i ];
			if( appleController.playerIndex == GCControllerPlayerIndexUnset )
			{
				for( int32_t j = 0; j < controllerCount; j++ )
				{
					if( !GetAppleControllerFn( j ) )
					{
						appleController.playerIndex = GCControllerPlayerIndex( j );
						// This makes sure that the Options button doesn't activate the 'record'/Share
						// system gesture on controllers without a dedicated share button
						if( @available(macOS 11.0, iOS 14.0, tvOS 14.0, *) )
						{
							const GCController* appleController = GetAppleControllerFn( j );
							if( appleController && appleController.extendedGamepad )
							{
								GCExtendedGamepad *gamepad = appleController.extendedGamepad;
								if( [gamepad.buttonOptions isBoundToSystemGesture] )
								{
									gamepad.buttonOptions.preferredSystemGestureState = GCSystemGestureStateDisabled;
								}
							}
						}
						break;
					}
				}
				AE_ASSERT( appleController.playerIndex != GCControllerPlayerIndexUnset );
			}
		}

		// Update gamepad states
		for( GamepadState& gp : gamepads )
		{
			const GCController* appleController = GetAppleControllerFn( gp.playerIndex );
			const GCExtendedGamepad* appleGamepad = appleController ? [appleController extendedGamepad] : nullptr;
			gp.connected = (bool)appleGamepad;
			if( gp.connected && ( !m_gamepadRequiresFocus 
				#if !_AE_IOS_
					 || [(NSWindow*)m_window->window isMainWindow] 
			 	#endif
			 ) )
			{
				auto leftAnalog = [appleGamepad leftThumbstick];
				auto rightAnalog = [appleGamepad rightThumbstick];
				gp.leftAnalog = Vec2( [leftAnalog xAxis].value, [leftAnalog yAxis].value );
				gp.rightAnalog = Vec2( [rightAnalog xAxis].value, [rightAnalog yAxis].value );
				
				auto dpad = [appleGamepad dpad];
				gp.up = [dpad up].value;
				gp.down = [dpad down].value;
				gp.left = [dpad left].value;
				gp.right = [dpad right].value;
				
				gp.start = [appleGamepad buttonMenu].value;
				gp.select = [appleGamepad buttonOptions].value;
				gp.a = [appleGamepad buttonA].value;
				gp.b = [appleGamepad buttonB].value;
				gp.x = [appleGamepad buttonX].value;
				gp.y = [appleGamepad buttonY].value;
				gp.leftBumper = [appleGamepad leftShoulder].value;
				gp.rightBumper = [appleGamepad rightShoulder].value;
				gp.leftTrigger = [appleGamepad leftTrigger].value;
				gp.rightTrigger = [appleGamepad rightTrigger].value;
				gp.leftAnalogClick = [appleGamepad leftThumbstickButton].value;
				gp.rightAnalogClick = [appleGamepad rightThumbstickButton].value;
				
				gp.batteryLevel = [[appleController battery] batteryLevel];
				switch( [[appleController battery] batteryState] )
				{
					case GCDeviceBatteryStateDischarging: gp.batteryState = GamepadState::BatteryState::InUse; break;
					case GCDeviceBatteryStateCharging: gp.batteryState = GamepadState::BatteryState::Charging; break;
					case GCDeviceBatteryStateFull: gp.batteryState = GamepadState::BatteryState::Full; break;
					default: gp.batteryState = GamepadState::BatteryState::None; break;
				};
			}
		}
	}
#elif _AE_EMSCRIPTEN_
	if( emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS )
	{
		const uint32_t gamepadCount = emscripten_get_num_gamepads();
		for( uint32_t i = 0; i < gamepadCount; i++ )
		{
			GamepadState& gp = gamepads[ i ];
			EmscriptenGamepadEvent gamepadState;
			if( emscripten_get_gamepad_status( i, &gamepadState ) == EMSCRIPTEN_RESULT_SUCCESS )
			{
				gp.connected = true;
				gp.leftAnalog = Vec2( gamepadState.axis[ 0 ], -gamepadState.axis[ 1 ] );
				gp.rightAnalog = Vec2( gamepadState.axis[ 2 ], -gamepadState.axis[ 3 ] );
				gp.up = gamepadState.digitalButton[ 12 ];
				gp.down = gamepadState.digitalButton[ 13 ];
				gp.left = gamepadState.digitalButton[ 14 ];
				gp.right = gamepadState.digitalButton[ 15 ];
				gp.start = gamepadState.digitalButton[ 9 ];
				gp.select = gamepadState.digitalButton[ 8 ];
				gp.a = gamepadState.digitalButton[ 0 ];
				gp.b = gamepadState.digitalButton[ 1 ];
				gp.x = gamepadState.digitalButton[ 2 ];
				gp.y = gamepadState.digitalButton[ 3 ];
				gp.leftBumper = gamepadState.digitalButton[ 4 ];
				gp.rightBumper = gamepadState.digitalButton[ 5 ];
				gp.leftTrigger = gamepadState.analogButton[ 6 ];
				gp.rightTrigger = gamepadState.analogButton[ 7 ];
				gp.leftAnalogClick = gamepadState.digitalButton[ 10 ];
				gp.rightAnalogClick = gamepadState.digitalButton[ 11 ];
				gp.batteryLevel = 1.0f;
				gp.batteryState = GamepadState::BatteryState::Wired;
			}
			else
			{
				gp.connected = false;
			}
		}
	}
#endif
	// Additional gamepad state processing shared across platforms
	for( GamepadState& gp : gamepads )
	{
		gp.leftAnalog *= ae::Clip01( ae::Delerp( m_leftAnalogThreshold, 1.0f, gp.leftAnalog.SafeNormalize() ) );
		gp.rightAnalog *= ae::Clip01( ae::Delerp( m_rightAnalogThreshold, 1.0f, gp.rightAnalog.SafeNormalize() ) );
		gp.dpad = ae::Int2( ( gp.right ? 1 : 0 ) + ( gp.left ? -1 : 0 ), ( gp.up ? 1 : 0 ) + ( gp.down ? -1 : 0 ) );
		gp.anyButton = gp.up || gp.down || gp.left || gp.right
			|| gp.start || gp.select
			|| gp.a || gp.b || gp.x || gp.y
			|| gp.leftBumper || gp.rightBumper
			|| gp.leftTrigger > 0.0f || gp.rightTrigger > 0.0f
			|| gp.leftAnalogClick || gp.rightAnalogClick;
		gp.anyInput = gp.anyButton
			|| fabsf(gp.leftAnalog.x) > 0.0f || fabsf(gp.leftAnalog.y) > 0.0f
			|| fabsf(gp.rightAnalog.x) > 0.0f || fabsf(gp.rightAnalog.y) > 0.0f;
	}
}

void Input::SetMouseCaptured( bool enable )
{
#if !_AE_EMSCRIPTEN_
	if( !m_window )
	{
		return;
	}
	else if( enable && !m_window->GetFocused() )
	{
		AE_ASSERT( !m_captureMouse );
		return;
	}
#endif
	
	if( enable != m_captureMouse )
	{
		if( enable )
		{
			// Remember original cursor position
			m_capturedMousePos = m_mousePosSet ? mouse.position : ae::Int2( INT_MAX );
#if _AE_WINDOWS_
			ShowCursor( FALSE );
#elif _AE_OSX_
			CGDisplayHideCursor( kCGDirectMainDisplay );
#elif _AE_EMSCRIPTEN_
			emscripten_request_pointerlock( "canvas", true );
#endif
			ae::Int2 localCenter( m_window->GetWidth() / 2, m_window->GetHeight() / 2 );
			m_SetCursorPos( localCenter );
			m_mousePosSet = false;
		}
		else
		{
			// Restore original cursor position
			if( m_capturedMousePos != ae::Int2( INT_MAX ) )
			{
				m_SetCursorPos( m_capturedMousePos );
				mouse.position = m_capturedMousePos;
			}
#if _AE_WINDOWS_
			ShowCursor( TRUE );
#elif _AE_OSX_
			CGDisplayShowCursor( kCGDirectMainDisplay );
#elif _AE_EMSCRIPTEN_
			emscripten_exit_pointerlock();
#endif
		}
		
		m_captureMouse = enable;
	}
}

void Input::SetTextMode( bool enabled )
{
	if( m_textMode != enabled )
	{
		m_textMode = enabled;
#if _AE_OSX_
		NSWindow* nsWindow = (NSWindow*)m_window->window;
		if( m_textMode )
		{
			aeTextInputDelegate* textInput = (aeTextInputDelegate*)m_textInputHandler;
			[nsWindow makeFirstResponder:textInput];
		}
		else
		{
			NSOpenGLView* glView = [nsWindow contentView];
			[nsWindow makeFirstResponder:glView];
		}
#endif
	}
}

bool Input::Get( ae::Key key ) const
{
	const bool result = m_keys[ static_cast< int >( key ) ];
#if _AE_EMSCRIPTEN_ && _AE_DEBUG_
	if( key == ae::Key::Escape && result && !GetPrev( key ) )
	{
		AE_WARN( "The escape key is reserved in browsers to release the cursor and exit fullscreen" );
	}
#endif
	return result;
}

bool Input::GetPrev( ae::Key key ) const
{
	return m_keysPrev[ static_cast< int >( key ) ];
}

const ae::Touch* Input::GetTouchById( uint32_t id ) const
{
	const int32_t touchIdx = m_touches.FindFn( [&]( const ae::Touch& t ){ return t.id == id; } );
	return ( touchIdx >= 0 ) ? &m_touches[ touchIdx ] : nullptr;
}

const ae::Touch* Input::GetFinishedTouchById( uint32_t id ) const
{
	const int32_t touchIdx = m_touches.FindFn( [&]( const ae::Touch& t ){ return t.id == id; } );
	const int32_t prevTouchIdx = m_touchesPrev.FindFn( [&]( const ae::Touch& t ){ return t.id == id; } );
	return ( touchIdx < 0 && prevTouchIdx >= 0 ) ? &m_touchesPrev[ prevTouchIdx ] : nullptr;
}

ae::TouchArray Input::GetNewTouches() const
{
	ae::TouchArray result;
	for( const ae::Touch& touch : m_touches )
	{
		const int32_t prevTouchIdx = m_touchesPrev.FindFn( [&]( const ae::Touch& t ){ return t.id == touch.id; } );
		if( prevTouchIdx < 0 )
		{
			result.Append( touch );
		}
	}
	return result;
}

ae::TouchArray Input::GetFinishedTouches() const
{
	ae::TouchArray result;
	for( const ae::Touch& touch : m_touchesPrev )
	{
		const int32_t touchIdx = m_touches.FindFn( [&]( const ae::Touch& t ){ return t.id == touch.id; } );
		if( touchIdx < 0 )
		{
			result.Append( touch );
		}
	}
	return result;
}

const ae::TouchArray& Input::GetTouches() const
{
	return m_touches;
}

const ae::TouchArray& Input::GetPreviousTouches() const
{
	return m_touchesPrev;
}

void Input::m_SetMousePos( ae::Int2 pos )
{
	AE_ASSERT( m_window );
	if( m_mousePosSet )
	{
		mouse.movement += pos - mouse.position;
	}
	mouse.position = pos;
	m_mousePosSet = true;
}

void Input::m_SetMousePos( ae::Int2 pos, ae::Int2 movement )
{
	AE_ASSERT( m_window );
	mouse.movement += movement;
	mouse.position = pos;
	m_mousePosSet = true;
}

void Input::m_SetCursorPos( ae::Int2 pos )
{
#if _AE_WINDOWS_
	{
		POINT centerPt = { pos.x, m_window->GetHeight() - pos.y };
		if( ClientToScreen( (HWND)m_window->window, &centerPt ) )
		{
			SetCursorPos( centerPt.x, centerPt.y );
		}
	}
#elif _AE_OSX_
	@autoreleasepool
	{
		NSWindow* nsWindow = (NSWindow*)m_window->window;
		NSPoint posScreen = [ nsWindow convertPointToScreen : NSMakePoint( pos.x, pos.y ) ];
		// @NOTE: Quartz coordinate space has (0,0) at the top left, Cocoa uses bottom left
		posScreen.y = NSMaxY( NSScreen.screens[ 0 ].frame ) - posScreen.y;
		CGWarpMouseCursorPosition( CGPointMake( posScreen.x, posScreen.y ) );
		CGAssociateMouseAndMouseCursorPosition( true );
	}
#endif
}

void Input::m_SetMouseCaptured( bool captured )
{
	m_captureMouse = captured;
}

void Input::m_UpdateModifiers()
{
#if _AE_APPLE_
	m_keys[ (int)ae::Key::LeftMeta ] = m_keys[ (int)ae::Key::LeftSuper ];
	m_keys[ (int)ae::Key::RightMeta ] = m_keys[ (int)ae::Key::RightSuper ];
#else
	m_keys[ (int)ae::Key::LeftMeta ] = m_keys[ (int)ae::Key::LeftControl ];
	m_keys[ (int)ae::Key::RightMeta ] = m_keys[ (int)ae::Key::RightControl ];
#endif
	m_keys[ (int)ae::Key::Control ] = m_keys[ (int)ae::Key::LeftControl ] || m_keys[ (int)ae::Key::RightControl ];
	m_keys[ (int)ae::Key::Shift ] = m_keys[ (int)ae::Key::LeftShift ] || m_keys[ (int)ae::Key::RightShift ];
	m_keys[ (int)ae::Key::Alt ] = m_keys[ (int)ae::Key::LeftAlt ] || m_keys[ (int)ae::Key::RightAlt ];
	m_keys[ (int)ae::Key::Super ] = m_keys[ (int)ae::Key::LeftSuper ] || m_keys[ (int)ae::Key::RightSuper ];
	m_keys[ (int)ae::Key::Meta ] = m_keys[ (int)ae::Key::LeftMeta ] || m_keys[ (int)ae::Key::RightMeta ];
}

//------------------------------------------------------------------------------
// ae::File member functions
//------------------------------------------------------------------------------
const char* File::GetUrl() const
{
	return m_url.c_str();
}

File::Status File::GetStatus() const
{
	return m_status;
}

uint32_t File::GetCode() const
{
	return m_code;
}

const uint8_t* File::GetData() const
{
	return m_data;
}

uint32_t File::GetLength() const
{
	return m_length;
}

float File::GetElapsedTime() const
{
	return m_finishTime ? ( m_finishTime - m_startTime ) : ( ae::GetTime() - m_startTime );
}

float File::GetTimeout() const
{
	return m_timeout;
}

uint32_t File::GetRetryCount() const
{
	return m_retryCount;
}

//------------------------------------------------------------------------------
// ae::FileFilter member functions
//------------------------------------------------------------------------------
FileFilter::FileFilter( const char* desc, const char** ext, uint32_t extensionCount )
{
	extensionCount = ae::Min( extensionCount, countof( extensions ) );
	description = desc;
	for( uint32_t i = 0; i < extensionCount; i++ )
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

bool FileSystem::IsAbsolutePath( const char* path )
{
#if _AE_EMSCRIPTEN_
	const char* result = strchr( path, ':' );
	return ( result && result[ 1 ] == '/' && result[ 2 ] == '/' );
#elif _AE_WINDOWS_
	return std::filesystem::path( path ).is_absolute();
#else
	return path[ 0 ] == '/' || path[ 0 ] == '~';
#endif
}

#if _AE_APPLE_ || _AE_LINUX_
const char* FileSystem_GetHomeDir()
{
	const char* homeDir = getenv( "HOME" );
	if( homeDir && homeDir[ 0 ] )
	{
		return homeDir;
	}
	else if( const passwd* pw = getpwuid( getuid() ) )
	{
		const char* homeDir = pw->pw_dir;
		if( homeDir && homeDir[ 0 ] )
		{
			return homeDir;
		}
	}
	return nullptr;
}
#endif
#if _AE_APPLE_
bool FileSystem_GetUserDir( Str256* outDir )
{
	// Something like /Users/someone/Library/Application Support
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString* prefsPath = [paths lastObject];
	if( [prefsPath length] )
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
	if( [cachesPath length] )
	{
		*outDir = [cachesPath UTF8String];
		return true;
	}
	return false;
}
#elif _AE_LINUX_
bool FileSystem_GetUserDir( Str256* outDir )
{
	// Something like /users/someone/.local/share
	if( const char* homeDir = FileSystem_GetHomeDir() )
	{
		*outDir = homeDir;
		FileSystem::AppendToPath( outDir, ".local/share" );
		return true;
	}
	return false;
}

bool FileSystem_GetCacheDir( Str256* outDir )
{
	// Something like /users/someone/.cache
	if( const char* homeDir = FileSystem_GetHomeDir() )
	{
		*outDir = homeDir;
		FileSystem::AppendToPath( outDir, ".cache" );
		return true;
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
	if( pathResult == S_OK )
	{
#if _AE_WINDOWS_
		constexpr uint32_t kBufLen = outDir->MaxLength() + 1;
		char path[ kBufLen ];
#else
		char path[ outDir->MaxLength() + 1 ];
#endif
		int32_t pathLen = (int32_t)wcstombs( path, wpath, outDir->MaxLength() );
		if( pathLen > 0 )
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
void _ae_GetCurrentWorkingDir( Str256* outDir )
{
	char url[ 256 ];
	url[ 0 ] = 0;
	EM_ASM( { stringToUTF8(window.location.href, $0, 256) }, url );
	*outDir = ae::FileSystem::GetDirectoryFromPath( url );
}
#endif

} // namespace ae

extern "C" void EMSCRIPTEN_KEEPALIVE _ae_FileSystem_ReadSuccess( void* arg, void* data, uint32_t length )
{
	ae::File* file = (ae::File*)arg;
	file->m_finishTime = ae::GetTime();
	file->m_data = (uint8_t*)ae::Allocate( AE_ALLOC_TAG_FILE, length + 1, 8 );
	memcpy( file->m_data, data, length );
	file->m_data[ length ] = 0;
	file->m_length = length;

	file->m_status = ae::File::Status::Success;
	file->m_code = 200;
}

extern "C" void EMSCRIPTEN_KEEPALIVE _ae_FileSystem_ReadFail( void* arg, uint32_t code, bool timeout )
{
	ae::File* file = (ae::File*)arg;
	file->m_finishTime = ae::GetTime();
	file->m_code = code;
	if( timeout )
	{
		file->m_status = ae::File::Status::Timeout;
	}
	else
	{
		switch( code )
		{
			case 404:
				file->m_status = ae::File::Status::NotFound;
				break;
			default:
				file->m_status = ae::File::Status::Error;
				break;
		}
	}
}

#if _AE_EMSCRIPTEN_
extern "C" void EMSCRIPTEN_KEEPALIVE _ae_em_free( void* p )
{
	free( p ); // -Oz prevents free() from being linked so force it
}

EM_JS( void, _ae_FileSystem_ReadImpl, ( const char* url, void* arg, uint32_t timeoutMs ),
{
	var xhr = new XMLHttpRequest();
	xhr.timeout = timeoutMs;
	xhr.open('GET', UTF8ToString(url), true);
	xhr.responseType = 'arraybuffer';
	xhr.ontimeout = function xhr_ontimeout() {
		__ae_FileSystem_ReadFail(arg, xhr.status, true);
	};
	xhr.onload = function xhr_onload() {
		if(xhr.status == 200) {
			if(xhr.response) {
				var byteArray = new Uint8Array(xhr.response);
				var buffer = _malloc(byteArray.length);
				if(buffer) {
					HEAPU8.set(byteArray, buffer);
					__ae_FileSystem_ReadSuccess(arg, buffer, byteArray.length);
					__ae_em_free(buffer);
				}
				else {
					__ae_FileSystem_ReadFail(arg, 0, false);
				}
			}
			else {
				__ae_FileSystem_ReadSuccess(arg, 0, 0); // Empty response but request succeeded
			}
			
		}
	};
	xhr.onerror = function xhrError() {
		__ae_FileSystem_ReadFail(arg, xhr.status, false);
	};
	xhr.send(null);
} );
#endif

namespace ae {

FileSystem::~FileSystem()
{
	AE_ASSERT_MSG( !m_files.Length(), "All files must be destroyed before destroying the loader" );
}

void FileSystem::Initialize( const char* dataDir, const char* organizationName, const char* applicationName )
{
	AE_ASSERT_MSG( organizationName && organizationName[ 0 ], "Organization name must not be empty" );
	AE_ASSERT_MSG( applicationName && applicationName[ 0 ], "Application name must not be empty" );

	const char* validateOrgName = organizationName;
	while( *validateOrgName )
	{
		AE_ASSERT_MSG( isalnum( *validateOrgName ) || ( *validateOrgName == '_' )  || ( *validateOrgName == '-' ), "Invalid organization name '#'. Only alphanumeric characters and undersrcores are supported.", organizationName );
		validateOrgName++;
	}
	const char* validateAppName = applicationName;
	while( *validateAppName )
	{
		AE_ASSERT_MSG( isalnum( *validateAppName ) || ( *validateAppName == '_' ) || ( *validateAppName == '-' ), "Invalid application name '#'. Only alphanumeric characters and undersrcores are supported.", applicationName );
		validateAppName++;
	}

	m_SetBundleDir();
	m_SetDataDir( dataDir ? dataDir : "" );
	m_SetUserDir( organizationName, applicationName );
	m_SetCacheDir( organizationName, applicationName );
	m_SetUserSharedDir( organizationName );
	m_SetCacheSharedDir( organizationName );
}

void FileSystem::m_SetBundleDir()
{
#if _AE_OSX_
	CFURLRef appUrl = CFBundleCopyBundleURL( CFBundleGetMainBundle() );
	if( appUrl )
	{
		CFStringRef bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
		m_bundleDir = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
	}
	else
	{
		char path[ PATH_MAX ];
		uint32_t pathLen = countof(path);
		if( _NSGetExecutablePath( path, &pathLen ) == 0 ) // If successful
		{
			m_bundleDir = path;
			for( int32_t len = m_bundleDir.Length() - 1; len > 0; len-- )
			{
				if( m_bundleDir[ len ] == '/' )
				{
					m_bundleDir.Trim( len );
					return;
				}
			}
			m_bundleDir = "";
		}
	}
#else
	// @TODO: Implement
#endif
}

void FileSystem::m_SetDataDir( const char* dataDir )
{
	m_dataDir = GetAbsolutePath( dataDir );
	// Append slash if not empty and is currently missing
	if( m_dataDir.Length() )
	{
		char sepatator[ 2 ] = { AE_PATH_SEPARATOR, 0 };
		AppendToPath( &m_dataDir, sepatator );
	}
}

void FileSystem::m_SetUserDir( const char* organizationName, const char* applicationName )
{
	const Str16 pathChar( 1, AE_PATH_SEPARATOR );
	m_userDir = "";
	if( FileSystem_GetUserDir( &m_userDir ) )
	{
		AE_ASSERT( m_userDir.Length() );
		m_userDir += pathChar;
		m_userDir += organizationName;
		m_userDir += pathChar;
		m_userDir += applicationName;
		m_userDir += pathChar;
		if( !CreateFolder( m_userDir.c_str() ) )
		{
			m_userDir = "";
		}
	}
}

void FileSystem::m_SetCacheDir( const char* organizationName, const char* applicationName )
{
	const Str16 pathChar( 1, AE_PATH_SEPARATOR );
	m_cacheDir = "";
	if( FileSystem_GetCacheDir( &m_cacheDir ) )
	{
		AE_ASSERT( m_cacheDir.Length() );
		m_cacheDir += pathChar;
		m_cacheDir += organizationName;
		m_cacheDir += pathChar;
		m_cacheDir += applicationName;
		m_cacheDir += pathChar;
		if( !CreateFolder( m_cacheDir.c_str() ) )
		{
			m_cacheDir = "";
		}
	}
}

void FileSystem::m_SetUserSharedDir( const char* organizationName )
{
	const Str16 pathChar( 1, AE_PATH_SEPARATOR );
	m_userSharedDir = "";
	if( FileSystem_GetUserDir( &m_userSharedDir ) )
	{
		AE_ASSERT( m_userSharedDir.Length() );
		m_userSharedDir += pathChar;
		m_userSharedDir += organizationName;
		m_userSharedDir += pathChar;
		m_userSharedDir += "shared";
		m_userSharedDir += pathChar;
		if( !CreateFolder( m_userSharedDir.c_str() ) )
		{
			m_userSharedDir = "";
		}
	}
}

void FileSystem::m_SetCacheSharedDir( const char* organizationName )
{
	const Str16 pathChar( 1, AE_PATH_SEPARATOR );
	m_cacheSharedDir = "";
	if( FileSystem_GetCacheDir( &m_cacheSharedDir ) )
	{
		AE_ASSERT( m_cacheSharedDir.Length() );
		m_cacheSharedDir += pathChar;
		m_cacheSharedDir += organizationName;
		m_cacheSharedDir += pathChar;
		m_cacheSharedDir += "shared";
		m_cacheSharedDir += pathChar;
		if( !CreateFolder( m_cacheSharedDir.c_str() ) )
		{
			m_cacheSharedDir = "";
		}
	}
}

uint32_t FileSystem::GetSize( Root root, const char* filePath ) const
{
	Str256 fullName;
	if( IsAbsolutePath( filePath ) || GetRootDir( root, &fullName ) )
	{
		fullName += filePath;
		return GetSize( fullName.c_str() );
	}
	return 0;
}

uint32_t FileSystem::Read( Root root, const char* filePath, void* buffer, uint32_t bufferSize ) const
{
	Str256 fullName;
	if( IsAbsolutePath( filePath ) || GetRootDir( root, &fullName ) )
	{
		fullName += filePath;
		return Read( fullName.c_str(), buffer, bufferSize );
	}
	return 0;
}

uint32_t FileSystem::Write( Root root, const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs ) const
{
	Str256 fullName;
	if( IsAbsolutePath( filePath ) || GetRootDir( root, &fullName ) )
	{
		fullName += filePath;
		return Write( fullName.c_str(), buffer, bufferSize, createIntermediateDirs );
	}
	return 0;
}

bool FileSystem::CreateFolder( Root root, const char* folderPath ) const
{
	Str256 fullName;
	if( IsAbsolutePath( folderPath ) || GetRootDir( root, &fullName ) )
	{
		fullName += folderPath;
		return CreateFolder( fullName.c_str() );
	}
	return false;
}

void FileSystem::ShowFolder( Root root, const char* folderPath ) const
{
	Str256 fullName;
	if( IsAbsolutePath( folderPath ) || GetRootDir( root, &fullName ) )
	{
		fullName += folderPath;
		ShowFolder( fullName.c_str() );
	}
}

const File* FileSystem::Read( Root root, const char* url, float timeoutSec )
{
	Str256 fullName;
	if( url[ 0 ] && ( IsAbsolutePath( url ) || GetRootDir( root, &fullName ) ) )
	{
		fullName += url;
		return Read( fullName.c_str(), timeoutSec );
	}
	else
	{
		double t = ae::GetTime();
		File* file = ae::New< File >( AE_ALLOC_TAG_FILE );
		file->m_url = url;
		file->m_startTime = t;
		file->m_finishTime = t;
		file->m_status = File::Status::Error;
		file->m_timeout = timeoutSec;
		m_files.Append( file );
		return file;
	}
}

const File* FileSystem::Read( const char* url, float timeoutSec )
{
	int32_t idx = m_files.FindFn( [&]( File* f ){ return f->m_url == url; } );
	if( idx >= 0 )
	{
		return m_files[ idx ];
	}
	File* file = ae::New< File >( AE_ALLOC_TAG_FILE );
	file->m_url = url;
	m_Read( file, timeoutSec );
	m_files.Append( file );
	return file;
}

void FileSystem::Retry( const ae::File* _file, float timeoutSec )
{
	if( _file )
	{
		switch( _file->m_status )
		{
			case ae::File::Status::Success:
			case ae::File::Status::Pending:
				break;
			default:
			{
				ae::File* file = const_cast< ae::File* >( _file );
				m_Read( file, timeoutSec );
				file->m_retryCount++;
				break;
			}
		}
	}
}

uint32_t FileSystem::GetFileStatusCount( ae::File::Status status ) const
{
	uint32_t count = 0;
	for( auto file : m_files )
	{
		if( file->GetStatus() == status )
		{
			count++;
		}
	}
	return count;
}

void FileSystem::m_Read( ae::File* file, float timeoutSec ) const
{
	AE_ASSERT( file );
	AE_ASSERT( file->m_url.Length() );
	AE_ASSERT( !file->m_data && !file->m_length );

	file->m_status = ae::File::Status::Pending;
	file->m_code = 0;
	file->m_startTime = ae::GetTime();
	file->m_finishTime = 0.0;
	file->m_timeout = timeoutSec;

	uint32_t timeoutMs;
	if( timeoutSec <= 0.0f )
	{
		timeoutMs = 0.0f;
	}
	else
	{
		timeoutMs = timeoutSec * 1000.0f;
		timeoutMs = ae::Max( 1u, timeoutMs ); // Prevent rounding down to infinite timeout
	}
#if _AE_EMSCRIPTEN_
	_ae_FileSystem_ReadImpl( file->m_url.c_str(), file, timeoutMs );
#else
	if( uint32_t length = GetSize( file->m_url.c_str() ) )
	{
		file->m_data = (uint8_t*)ae::Allocate( AE_ALLOC_TAG_FILE, length + 1, 8 );
		Read( file->m_url.c_str(), file->m_data, length );
		file->m_data[ length ] = 0;
		file->m_length = length;
		file->m_status = ae::File::Status::Success;
	}
	else
	{
		file->m_status = File::Status::Error;
	}
	file->m_finishTime = ae::GetTime();
#endif
}

void FileSystem::Destroy( const File* file )
{
	if( file )
	{
		const int32_t idx = m_files.Find( file );
		AE_ASSERT_MSG( idx >= 0, "Unknown file pointer provided for destruction" );
		m_files.Remove( idx );
		ae::Free( file->m_data );
		ae::Delete( file );
	}
}

void FileSystem::DestroyAll()
{
	for( auto file : m_files )
	{
		ae::Free( file->m_data );
		ae::Delete( file );
	}
	m_files.Clear();
}

const File* FileSystem::GetFile( uint32_t idx ) const
{
	return m_files[ idx ];
}

uint32_t FileSystem::GetFileCount() const
{
	return m_files.Length();
}

bool FileSystem::GetAbsolutePath( Root root, const char* filePath, Str256* outPath ) const
{
	if( IsAbsolutePath( filePath ) )
	{
		*outPath = filePath;
		NormalizePath( outPath );
		return true;
	}
	else if( GetRootDir( root, outPath ) )
	{
		AppendToPath( outPath, filePath );
		NormalizePath( outPath );
		return true;
	}
	return false;
}

bool FileSystem::GetRootDir( Root root, Str256* outDir ) const
{
	if( !outDir )
	{
		return false;
	}
	switch( root )
	{
		case Root::Bundle:
			if( m_bundleDir.Length() )
			{
				*outDir = m_bundleDir;
				return true;
			}
			break;
		case Root::Data:
			if( m_dataDir.Length() )
			{
				*outDir = m_dataDir;
				return true;
			}
			break;
		case Root::User:
			if( m_userDir.Length() )
			{
				*outDir = m_userDir;
				return true;
			}
			break;
		case Root::Cache:
			if( m_cacheDir.Length() )
			{
				*outDir = m_cacheDir;
				return true;
			}
			break;
		case Root::UserShared:
			if( m_userSharedDir.Length() )
			{
				*outDir = m_userSharedDir;
				return true;
			}
			break;
		case Root::CacheShared:
			if( m_cacheSharedDir.Length() )
			{
				*outDir = m_cacheSharedDir;
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
	if( appUrl )
	{
		bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
		filePath = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
	}
#endif
	
	uint32_t fileSize = 0;
	if( FILE* file = fopen( filePath, "rb" ) )
	{
		fseek( file, 0, SEEK_END );
		fileSize = (uint32_t)ftell( file );
		fclose( file );
	}
	
#if _AE_APPLE_
	if( bundlePath ) { CFRelease( bundlePath ); }
	if( appUrl ) { CFRelease( appUrl ); }
	CFRelease( filePathIn );
#endif
	
	return fileSize;
}

uint32_t FileSystem::Read( const char* filePath, void* buffer, uint32_t bufferSize )
{
	// @TODO: Handle paths to folders gracefully and return 0
#if _AE_APPLE_
	CFStringRef filePathIn = CFStringCreateWithCString( kCFAllocatorDefault, filePath, kCFStringEncodingUTF8 );
	CFURLRef appUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), filePathIn, nullptr, nullptr );
	CFStringRef bundlePath = nullptr;
	if( appUrl )
	{
		CFStringRef bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
		filePath = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
	}
#endif

	uint32_t resultLen = 0;
	
	if( FILE* file = fopen( filePath, "rb" ) )
	{
		fseek( file, 0, SEEK_END );
		resultLen = (uint32_t)ftell( file );
		fseek( file, 0, SEEK_SET );

		if( resultLen <= bufferSize )
		{
			resultLen = (uint32_t)fread( buffer, sizeof(uint8_t), resultLen, file );
		}
		else
		{
			resultLen = 0;
		}

		fclose( file );
	}
	
#if _AE_APPLE_
	if( bundlePath ) { CFRelease( bundlePath ); }
	if( appUrl ) { CFRelease( appUrl ); }
	CFRelease( filePathIn );
#endif

	return resultLen;
}

uint32_t FileSystem::Write( const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs )
{
	if( createIntermediateDirs )
	{
		auto dir = GetDirectoryFromPath( filePath );
		if( dir.Length() && !FileSystem::CreateFolder( dir.c_str() ) )
		{
			return 0;
		}
	}
	
	FILE* file = fopen( filePath, "wb" );
	if( !file )
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
#elif _AE_LINUX_
	char path[ PATH_MAX + 1 ];
	size_t pathLength = strlcpy( path, folderPath, PATH_MAX );
	if( pathLength >= PATH_MAX )
	{
		return false;
	}
	else if( path[ pathLength - 1 ] != '/' )
	{
		path[ pathLength++ ] = '/';
		path[ pathLength ] = 0;
	}
	for( char* p = path + 1; *p; p++ )
	{
		if( *p == '/' )
		{
			*p = 0;
			if( mkdir( path, S_IRWXU ) == -1 && errno != EEXIST ) // Only accessible by owner
			{
				return false;
			}
			*p = '/';
		}
	}
	return true;
#elif _AE_WINDOWS_
	switch( SHCreateDirectoryExA( nullptr, folderPath, nullptr ) )
	{
		case ERROR_SUCCESS:
		case ERROR_ALREADY_EXISTS:
			return true;
		default:
			return false;
	}
#endif
	return false;
}

void FileSystem::ShowFolder( const char* folderPath )
{
#if _AE_OSX_
	NSString* path = [NSString stringWithUTF8String:folderPath];
	[[NSWorkspace sharedWorkspace] selectFile:path inFileViewerRootedAtPath:@""];
#elif _AE_LINUX_
	// @TODO: Linux
#elif _AE_WINDOWS_
	ShellExecuteA( NULL, "explore", folderPath, NULL, NULL, SW_SHOWDEFAULT );
#endif
}

Str256 FileSystem::GetAbsolutePath( const char* filePath )
{
#if _AE_APPLE_
	// @TODO: Should match ae::FileSystem::GetSize behavior and check resource dir in bundles
	if( filePath[ 0 ] == '/' )
	{
		// Already absolute
		return filePath;
	}
	else if( filePath[ 0 ] == '~' && filePath[ 1 ] == '/' )
	{
		// Relative to home directory
		char path[ PATH_MAX + 1 ];
		const char* homeDir = FileSystem_GetHomeDir();
		if( !homeDir )
		{
			return "";
		}
		size_t pathLength = strlcpy( path, homeDir, PATH_MAX );
		if( pathLength >= PATH_MAX )
		{
			return "";
		}
		pathLength = strlcat( path, filePath + 1, PATH_MAX );
		if( pathLength >= PATH_MAX )
		{
			return "";
		}
		char realPathBuf[ PATH_MAX ];
		realPathBuf[ 0 ] = 0;
		if( char* resolvedPath = realpath( path, realPathBuf ) )
		{
			ae::Str256 result( resolvedPath );
			return result;
		}
		else
		{
			return "";
		}
	}
	else if( CFBundleGetMainBundle() )
	{
		// Assume filePath is relative to the app resource folder
		char path[ PATH_MAX ];
		path[ 0 ] = 0;
		CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL( CFBundleGetMainBundle() );
		CFURLGetFileSystemRepresentation( resourcesURL, TRUE, (UInt8*)path, PATH_MAX );
		CFRelease( resourcesURL );
		strlcat( path, "/", sizeof(path) );
		strlcat( path, filePath, sizeof(path) );
		return path;
	}
	else
	{
		// Assume filePath is relative to the executables directory
		NSString* path = [NSString stringWithUTF8String:filePath];
		NSString* currentPath = [[NSFileManager defaultManager] currentDirectoryPath];
		AE_ASSERT( [currentPath characterAtIndex:0] != '~' );
		NSURL* currentPathUrl = [NSURL fileURLWithPath:currentPath];
		NSURL* absoluteUrl = [NSURL URLWithString:path relativeToURL:currentPathUrl];
		return [absoluteUrl.path UTF8String];
	}
#elif _AE_LINUX_
	// @TODO: Handle non-existing dirs
	char* resolvedPath;
	char realPathBuf[ PATH_MAX ];
	realPathBuf[ 0 ] = 0;
	if( filePath[ 0 ] == '~' && filePath[ 1 ] == '/' )
	{
		char path[ PATH_MAX + 1 ];
		const char* homeDir = FileSystem_GetHomeDir();
		if( !homeDir )
		{
			return "";
		}
		size_t pathLength = strlcpy( path, homeDir, PATH_MAX );
		if( pathLength >= PATH_MAX )
		{
			return "";
		}
		pathLength = strlcat( path, filePath + 1, PATH_MAX );
		if( pathLength >= PATH_MAX )
		{
			return "";
		}
		resolvedPath = realpath( path, realPathBuf );
	}
	else
	{
		resolvedPath = realpath( filePath, realPathBuf );
	}
	if( resolvedPath )
	{
		ae::Str256 result( resolvedPath );
		return result;
	}
	else
	{
		return "";
	}
#elif _AE_WINDOWS_
	if( IsAbsolutePath( filePath ) )
	{
		return filePath;
	}
	else
	{
		char result[ ae::Str256::MaxLength() ];
		result[ 0 ] = 0;
		GetModuleFileNameA( nullptr, result, sizeof( result ) );
		const_cast< char* >( GetFileNameFromPath( result ) )[ 0 ] = 0;
		strlcat( result, filePath, sizeof( result ) );

		char buf[ _MAX_PATH ];
		// 'Naturalize' path to remove relative path elements
		if( _fullpath( buf, result, _MAX_PATH ) )
		{
			return buf;
		}
		else
		{
			return result;
		}
	}
#elif _AE_EMSCRIPTEN_
	ae::Str256 result;
	if( IsAbsolutePath( filePath ) )
	{
		result = filePath;
		return result;
	}
	_ae_GetCurrentWorkingDir( &result );
	AppendToPath( &result, filePath );
	return result;
#else
	#warning "ae::FileSystem::GetAbsolutePath() not implemented. ae::FileSystem functionality will be limited."
	return filePath;
#endif
}

const char* FileSystem::GetFileNameFromPath( const char* filePath )
{
	const char* s0 = strrchr( filePath, '/' );
	const char* s1 = strrchr( filePath, '\\' );
	
	if( s1 && s0 )
	{
		return ( ( s1 > s0 ) ? s1 : s0 ) + 1;
	}
	else if( s0 )
	{
		return s0 + 1;
	}
	else if( s1 )
	{
		return s1 + 1;
	}
	else
	{
		return filePath;
	}
}

const char* FileSystem::GetFileExtFromPath( const char* filePath, bool includeDot )
{
	// Find first dot after last separator
	const char* fileName = GetFileNameFromPath( filePath );
	const char* s = strchr( fileName, '.' );
	if( s )
	{
		return ( includeDot ? s : s + 1 );
	}
	else
	{
		// @NOTE: Return end of given string in case pointer arithmetic is being done by user
		uint32_t len = (uint32_t)strlen( fileName );
		return fileName + len;
	}
}

std::pair< const char*, const char* > FileSystem::TraversePath( const char* filePath )
{
	const char delimeters[] = "/\\";
	const char* filePathEnd = filePath + strlen( filePath );
	const char* start = filePath;
	while( start[ 0 ] == delimeters[ 0 ] || start[ 0 ] == delimeters[ 1 ] )
	{
		start++;
	}
	const char* end = std::find_first_of(
		start,
		filePathEnd,
		delimeters,
		delimeters + 2
	);
	return std::make_pair( start, end );
}

Str256 FileSystem::GetDirectoryFromPath( const char* filePath )
{
	const char* fileName = GetFileNameFromPath( filePath );
	return Str256( (uint32_t)( fileName - filePath ), filePath );
}

void FileSystem::AppendToPath( Str256* path, const char* str )
{
	if( !path )
	{
		return;
	}

	// Fix existing path for platform
	const char otherSeparator = ( AE_PATH_SEPARATOR == '/' ) ? '\\' : '/';
	for( uint32_t i = 0; i < path->Length(); i++ )
	{
		char& c = path->operator[]( i );
		if( c == otherSeparator )
		{
			c = AE_PATH_SEPARATOR;
		}
	}
	
	// @TODO: Handle paths that already have a file name and extension
	
	// @TODO: Handle one or more path separators at end of path
	if( uint32_t pathLen = path->Length() )
	{
		char lastChar = path->operator[]( pathLen - 1 );
		if( lastChar != '/' && lastChar != '\\' )
		{
			path->Append( Str16( 1, AE_PATH_SEPARATOR ) );

			if( ( str[ 0 ] == '/' || str[ 0 ] == '\\' ) && !str[ 1 ] )
			{
				// @HACK: Append single separator when given separator only string
				return;
			}
		}
	}
	
	// @TODO: Handle one or more path separators at front of str
	*path += str;
}

bool ae::FileSystem::SetExtension( Str256* path, const char* ext )
{
	if( !path || !path->Length()
		|| !ext || !ext[ 0 ]
		|| IsDirectory( path->c_str() ) )
	{
		return false;
	}
	const uint32_t extLength = (uint32_t)strlen( ext );
	if( std::find_if(
			ext, ext + extLength,
			[]( char c ) { return !std::isalnum( c );
		} ) != ext + extLength )
	{
		return false;
	}

	for( int32_t i = path->Length() - 1;
		i >= 0 && (*path)[ i ] != '/' && (*path)[ i ] != '\\';
		i-- )
	{
		if( (*path)[ i ] == '.' )
		{
			path->Trim( i );
			break;
		}
	}
	path->Append( "." );
	path->Append( ext );
	return true;
}

bool ae::FileSystem::IsDirectory( const char* path )
{
	uint32_t length = (uint32_t)strlen( path );
	if( length == 0 )
	{
		return false;
	}
	return path[ length - 1 ] == '/' || path[ length - 1 ] == '\\';
}

void ae::FileSystem::NormalizePath( Str256* path )
{
	if( !path || !path->Length() )
	{
		return;
	}
	std::filesystem::path fsPath = std::filesystem::path( path->c_str() ).lexically_normal();
	*path = fsPath.string().c_str();
}

#if _AE_WINDOWS_

void FixPathExtension( const char* extension, std::filesystem::path* pathOut )
{
	// Set if path has no extension
	if( !pathOut->has_extension() )
	{
		pathOut->replace_extension( extension );
		return;
	}

	// Set if extension chars are just periods
	std::string pathExt = pathOut->extension().string();
	if( pathExt[ pathExt.length() - 1 ] == '.' )
	{
		pathOut->concat( std::string( "." ) + extension );
		return;
	}
}

ae::Array< char > CreateFilterString( const Array< FileFilter, 8 >& filters )
{
	ae::Array< char > result = AE_ALLOC_TAG_FILE;
	if( !filters.Length() )
	{
		return result;
	}

	ae::Array< char > tempFilterStr = AE_ALLOC_TAG_FILE;
	for( uint32_t i = 0; i < filters.Length(); i++ )
	{
		const FileFilter& filter = filters[ i ];
		tempFilterStr.Clear();

		uint32_t extCount = 0;
		for( uint32_t j = 0; j < countof( FileFilter::extensions ); j++ )
		{
			const auto& ext = filter.extensions[ j ];
			if( !ext.Length() )
			{
				continue;
			}

			// Validate extension
			if( ext != "*" )
			{
				for( const char* extCheck = ext.c_str(); *extCheck; extCheck++ )
				{
					if( !std::isalnum( *extCheck ) )
					{
						AE_FAIL_MSG( "File extensions must only contain alphanumeric characters or '*': #", ext );
						result.Clear();
						return result;
					}
				}
			}

			if( extCount == 0 )
			{
				tempFilterStr.AppendArray( "*.", 2 );
			}
			else
			{
				tempFilterStr.AppendArray( ";*.", 3 );
			}

			tempFilterStr.AppendArray( ext.c_str(), ext.Length() );
			extCount++;
		}

		if( extCount == 0 )
		{
			tempFilterStr.AppendArray( "*.*", 3 );
		}

		// Description
		result.AppendArray( filter.description.c_str(), filter.description.Length() );
		result.AppendArray( " (", 2 );
		result.AppendArray( tempFilterStr.Data(), tempFilterStr.Length() );
		result.AppendArray( ")", 2 ); // Every description must be null terminated

		result.AppendArray( tempFilterStr.Data(), tempFilterStr.Length() );
		result.Append( '\0' ); // Every filter must be null terminated
	}

	// Append final null terminator because GetOpenFileName requires double termination at end of string
	result.Append( '\0' );

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
	if( params.windowTitle && params.windowTitle[ 0 ] )
	{
		winParams.lpstrTitle = params.windowTitle;
	}
	winParams.lpstrFile = fileNameBuf;
	winParams.nMaxFile = sizeof( fileNameBuf );
	winParams.lpstrFilter = filterStr.Length() ? &filterStr[ 0 ] : "All Files (*.*)\0*.*\0";
	winParams.nFilterIndex = 1;
	winParams.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if( params.allowMultiselect )
	{
		winParams.Flags |= OFN_ALLOWMULTISELECT;
	}

	// Open window
	if( GetOpenFileNameA( &winParams ) )
	{
		if( !params.allowMultiselect )
		{
			return ae::Array< std::string >( AE_ALLOC_TAG_FILE, winParams.lpstrFile, 1 );
		}
		else
		{
			// Null separated and double null terminated when OFN_ALLOWMULTISELECT is specified
			uint32_t offset = (uint32_t)strlen( winParams.lpstrFile ) + 1; 
			if( winParams.lpstrFile[ offset ] == 0 ) // One result
			{
				return ae::Array< std::string >( AE_ALLOC_TAG_FILE, winParams.lpstrFile, 1 );
			}
			else // Multiple results
			{
				const char* head = winParams.lpstrFile;
				const char* directory = head;
				head += offset; // Null separated
				ae::Array< std::string > result = AE_ALLOC_TAG_FILE;
				while( *head )
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
	if( params.windowTitle && params.windowTitle[ 0 ] )
	{
		winParams.lpstrTitle = params.windowTitle;
	}
	winParams.lpstrFile = fileNameBuf;
	winParams.nMaxFile = sizeof( fileNameBuf );
	winParams.lpstrFilter = filterStr.Length() ? &filterStr[ 0 ] : "All Files (*.*)\0*.*\0";
	winParams.nFilterIndex = 1;
	winParams.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if( GetSaveFileNameA( &winParams ) )
	{
		std::filesystem::path result = winParams.lpstrFile;
		if( winParams.nFilterIndex >= 1 )
		{
			winParams.nFilterIndex--;
			const char* ext = params.filters[ winParams.nFilterIndex ].extensions[ 0 ].c_str();
			
			FixPathExtension( ext, &result );
		}
		return result.string();
	}

	return "";
}

#elif _AE_OSX_

//------------------------------------------------------------------------------
// OpenDialog
//------------------------------------------------------------------------------
ae::Array< std::string > FileSystem::OpenDialog( const FileDialogParams& params )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	ae::RunOnDestroy poolCleanup( [&]() { [pool release]; } );
	
	NSWindow* window = (NSWindow*)( params.window ? params.window->window : nullptr );
	NSOpenPanel* dialog = [NSOpenPanel openPanel]; // https://developer.apple.com/forums/thread/782271
	if( !dialog )
	{
		AE_ERR( "Failed to create NSOpenPanel" );
		return ae::Array< std::string >( AE_ALLOC_TAG_FILE );
	}
	dialog.canChooseFiles = YES;
	dialog.canChooseDirectories = NO;
	dialog.allowsMultipleSelection = params.allowMultiselect;
	if( params.windowTitle && params.windowTitle[ 0 ] )
	{
		dialog.message = [NSString stringWithUTF8String:params.windowTitle];
	}
	if( params.defaultPath && params.defaultPath[ 0 ] )
	{
		ae::Str256 dir = "file://";
		dir += params.defaultPath;
		dialog.directoryURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:dir.c_str()]];
	}
	
	bool allowAny = false;
	NSMutableArray* filters = [NSMutableArray arrayWithCapacity:params.filters.Length()];
	for( const FileFilter& filter : params.filters )
	{
		for( const auto& ext : filter.extensions )
		{
			if( ext.Length() )
			{
				if( ext == "*" )
				{
					allowAny = true;
				}
				[filters addObject:[NSString stringWithUTF8String:ext.c_str()]];
			}
		}
	}
	if( !allowAny )
	{
		[dialog setAllowedFileTypes:filters];
	}
	
	__block bool finished = false;
	__block bool success = false;
	ae::Array< std::string > result = AE_ALLOC_TAG_FILE;
	// Show
	if( window )
	{
		AE_ASSERT_MSG( params.window->input, "Must initialize ae::Input with ae::Window before creating a file dialog" );
		[dialog beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode)
		{
			success = ( returnCode == NSFileHandlingPanelOKButton );
			finished = true;
		}];
		// Block here until panel returns
		while( !finished )
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
	if( success )
	{
		if( dialog.URLs.count )
		{
			for(NSURL* url in dialog.URLs)
			{
				result.Append( url.fileSystemRepresentation );
			}
		}
		else if( dialog.URL )
		{
			result.Append( dialog.URL.fileSystemRepresentation );
		}
	}
	
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
	if( params.windowTitle && params.windowTitle[ 0 ] )
	{
		dialog.message = [NSString stringWithUTF8String:params.windowTitle];
	}
	if( params.defaultPath && params.defaultPath[ 0 ] )
	{
		ae::Str256 dir = "file://";
		dir += params.defaultPath;
		dialog.directoryURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:dir.c_str()]];
	}
	
	bool allowAny = false;
	NSMutableArray* filters = [NSMutableArray arrayWithCapacity:params.filters.Length()];
	for( const FileFilter& filter : params.filters )
	{
		for( const auto& ext : filter.extensions )
		{
			if( ext.Length() )
			{
				if( ext == "*" )
				{
					allowAny = true;
				}
				[filters addObject:[NSString stringWithUTF8String:ext.c_str()]];
			}
		}
	}
	if( !allowAny )
	{
		[dialog setAllowedFileTypes:filters];
	}
	
	__block bool finished = false;
	__block bool success = false;
	std::string result;
	// Show
	if( window )
	{
		AE_ASSERT_MSG( params.window->input, "Must initialize ae::Input with ae::Window before creating a file dialog" );
		[dialog beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode)
		{
			success = ( returnCode == NSFileHandlingPanelOKButton );
			finished = true;
		}];
		// Block here until panel returns
		while( !finished )
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
	if( success && dialog.URL )
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
	return Array< std::string >( AE_ALLOC_TAG_FILE );
}

//------------------------------------------------------------------------------
// SaveDialog not implemented
//------------------------------------------------------------------------------
std::string FileSystem::SaveDialog( const FileDialogParams& params )
{
	return "";
}

#endif

//------------------------------------------------------------------------------
// ae::Socket and ae::ListenerSocket helpers
//------------------------------------------------------------------------------
uint32_t _winsockCount = 0;
bool _WinsockInit()
{
#if _AE_WINDOWS_
	if( !_winsockCount )
	{
		WSADATA wsaData;
		if( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) != 0 )
		{
			return false;
		}
	}
	_winsockCount++;
#endif
	return true;
}

void _CloseSocket( int sock )
{
	if( sock < 0 )
	{
		return;
	}
#if _AE_WINDOWS_
	closesocket( sock );
#else
	shutdown( sock, SHUT_RDWR );
	#if !_AE_EMSCRIPTEN_
		close( sock );
	#endif
#endif
}

bool _DisableBlocking( int sock )
{
	if( sock < 0 )
	{
		return false;
	}
#if _AE_WINDOWS_
	u_long mode = 1;
	return ioctlsocket( sock, FIONBIO, &mode ) != -1;
#else
	return fcntl( sock, F_SETFL, O_NONBLOCK ) != -1;
#endif
}

bool _DisableNagles( int sock )
{
	if( sock < 0 )
	{
		return false;
	}
#if _AE_WINDOWS_
	const char* yes = "1";
	socklen_t optlen = 1;
#else
	int yesValue = 1;
	int* yes = &yesValue;
	socklen_t optlen = sizeof(int);
#endif
	return setsockopt( sock, SOL_SOCKET, TCP_NODELAY, yes, optlen ) != -1;
}

bool _ReuseAddress( int sock )
{
	if( sock < 0 )
	{
		return false;
	}
#if _AE_WINDOWS_
	const char* yes = "1";
	socklen_t optlen = 1;
#else
	int yesValue = 1;
	int* yes = &yesValue;
	socklen_t optlen = sizeof(int);
#endif
	if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, yes, optlen ) != -1 )
	{
#if _AE_APPLE_
		// Apple platforms require both SO_REUSE_PORT and SO_REUSEADDR to be set or
		// listening sockets will run into issues with the firewall.
		return setsockopt( sock, SOL_SOCKET, 0x0200, yes, optlen ) != -1; // SO_REUSE_PORT
#endif
		return true;
	}
	return false;
}

int _IsConnected( int sock ) // 1 connected, 0 connecting, -1 not connected
{
	if( sock < 0 )
	{
		return -1;
	}
	_ae_poll_fd_t pollParam;
	memset( &pollParam, 0, sizeof(pollParam) );
	pollParam.fd = sock;
	pollParam.events = POLLOUT;

	if( _ae_sock_poll( &pollParam, 1, 0 ) > 0 )
	{
		if( pollParam.revents & POLLOUT )
		{
			return 1;
		}
		else if( pollParam.revents & ( POLLERR | POLLHUP | POLLNVAL ) )
		{
			_ae_sock_err_t err = 0;
			socklen_t optLen = sizeof(err);
			if( getsockopt( sock, SOL_SOCKET, SO_ERROR, &err, &optLen ) == 0 )
			{
				return ( err == 0 ) ? 0 : -1;
			}
			return -1;
		}
		return 0;
	}
	return ( errno == EINPROGRESS ) ? 0 : -1;
}

bool _GetAddressString( const sockaddr* addr, char (&addrStr)[ INET6_ADDRSTRLEN ] )
{
	addrStr[ 0 ] = 0;
	void* inAddr = nullptr;
	socklen_t inAddrLen = 0;
	_ae_sa_family_t family = addr->sa_family;
	if( family == AF_INET )
	{
		inAddr = &( ( (sockaddr_in*)addr )->sin_addr );
		inAddrLen = sizeof(sockaddr_in);
	}
	else if( family == AF_INET6 )
	{
		inAddr = &( ( (sockaddr_in6*)addr )->sin6_addr );
		inAddrLen = sizeof(sockaddr_in6);
	}
	else
	{
		return false;
	}
	return inet_ntop( addr->sa_family, inAddr, addrStr, inAddrLen ) != nullptr;
}

uint16_t _GetPort( const sockaddr* addr )
{
	_ae_sa_family_t family = addr->sa_family;
	if( family == AF_INET )
	{
		return ntohs( ( (sockaddr_in*)addr )->sin_port );
	}
	else if( family == AF_INET6 )
	{
		return ntohs( ( (sockaddr_in6*)addr )->sin6_port );
	}
	return 0;
}

//------------------------------------------------------------------------------
// ae::Socket member functions
//------------------------------------------------------------------------------
Socket::Socket( ae::Tag tag ) :
	m_sendData( tag ),
	m_recvData( tag )
{}

Socket::Socket( ae::Tag tag, int s, Protocol proto, const char* addr, uint16_t port ) :
	m_protocol( proto ),
	m_address( addr ),
	m_port( port ),
	m_sock( s ),
	m_isConnected( true ),
	m_resolvedAddress( addr ),
	m_sendData( tag ),
	m_recvData( tag )
{}

Socket::~Socket()
{
	Disconnect();
}

bool Socket::Connect( ae::Socket::Protocol proto, const char* address, uint16_t port )
{
	m_readHead = 0;
	m_sendData.Clear();
	m_recvData.Clear();
	
	if( !_WinsockInit() )
	{
		return false;
	}
	
	address = address ? address : "";
	if( m_protocol != proto || m_address != address || m_port != port )
	{
		Disconnect();
		if( proto == ae::Socket::Protocol::None || !address[ 0 ] || port == 0 )
		{
			return false;
		}
		m_protocol = proto;
		m_address = address;
		m_port = port;
		m_resolvedAddress = "";
	}
	
	if( m_sock < 0 )
	{
		if( !m_addrInfo )
		{
			AE_ASSERT( !m_currAddrInfo );
			ae::Str16 portStr = ae::Str16::Format( "#", (uint32_t)port );
			addrinfo hints;
			memset( &hints, 0, sizeof hints );
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = ( proto == ae::Socket::Protocol::TCP ) ? SOCK_STREAM : SOCK_DGRAM;
			// @TODO: Fix error '[si_destination_compare] send failed: Bad file descriptor'
			if( getaddrinfo( address, portStr.c_str(), &hints, (addrinfo**)&m_addrInfo ) == -1 )
			{
				m_addrInfo = nullptr;
				return false;
			}
			m_currAddrInfo = m_addrInfo;
		}
		else
		{
			m_currAddrInfo = ((addrinfo*)m_currAddrInfo)->ai_next;
			if( !m_currAddrInfo )
			{
				m_currAddrInfo = m_addrInfo;
			}
		}
		AE_ASSERT( m_currAddrInfo );
		addrinfo* addrInfo = (addrinfo*)m_currAddrInfo;

		m_sock = socket( addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol );
		if( m_sock == -1 )
		{
			return false;
		}
		
		if( !_DisableBlocking( m_sock )
			|| !_DisableNagles( m_sock )
			|| ( connect( m_sock, addrInfo->ai_addr, addrInfo->ai_addrlen ) == -1
				&& errno != EAGAIN && errno != EALREADY && errno != EINPROGRESS && errno != EISCONN ) )
		{
			_CloseSocket( m_sock );
			m_sock = -1;
			return false;
		}
		
		AE_ASSERT( !m_isConnected );
		if( m_protocol == Protocol::UDP )
		{
			m_isConnected = true;
		}
	}
	
	if( !m_isConnected && m_protocol == Protocol::TCP )
	{
		int connectedResult = _IsConnected( m_sock );
		if( connectedResult > 0 )
		{
			char addrStr[ INET6_ADDRSTRLEN ];
			AE_STATIC_ASSERT( decltype(m_resolvedAddress)::MaxLength() > INET6_ADDRSTRLEN );
			_GetAddressString( ((addrinfo*)m_currAddrInfo)->ai_addr, addrStr );
			m_resolvedAddress = addrStr;
			
			m_isConnected = true;
			freeaddrinfo( (addrinfo*)m_addrInfo );
			m_addrInfo = nullptr;
			m_currAddrInfo = nullptr;
		}
		else if( connectedResult == 0 )
		{
			return false;
		}
		else if( connectedResult < 0 )
		{
			_CloseSocket( m_sock );
			m_sock = -1;
			return false;
		}
	}
	
	return true;
}

void Socket::Disconnect()
{
	_CloseSocket( m_sock );
	freeaddrinfo( (addrinfo*)m_addrInfo );
	m_protocol = Protocol::None;
	m_sock = -1;
	m_isConnected = false;
	m_addrInfo = nullptr;
	m_currAddrInfo = nullptr;
	// @NOTE: Do not modify buffers here, Connect() will perform actual cleanup
}

bool Socket::IsConnected() const
{
	return m_isConnected;
}

bool Socket::QueueData( const void* data, uint32_t length )
{
	if( !IsConnected() )
	{
		return false;
	}
	m_sendData.AppendArray( (const uint8_t*)data, length );
	return true;
}

bool Socket::PeekData( void* dataOut, uint16_t length, uint32_t offset )
{
	if( !length )
	{
		return false;
	}
	
	while( IsConnected() && m_recvData.Length() < m_readHead + offset + length )
	{
#if _AE_WINDOWS_
		u_long readSize = 0;
#else
		int readSize = 0;
#endif
		if( _ae_ioctl( m_sock, FIONREAD, &readSize ) == -1 )
		{
			Disconnect();
			return false;
		}
		
		if( readSize == 0 )
		{
			// Check for closed connection
			if( m_protocol == Protocol::TCP )
			{
				_ae_sock_buff_t buffer;
				const int32_t result = (int32_t)recv( m_sock, &buffer, 1, MSG_PEEK );
				if( result == 0 || ( result == -1 && errno != EWOULDBLOCK && errno != EAGAIN ) )
				{
					Disconnect();
				}
			}
			else if( m_protocol == Protocol::UDP )
			{
				_ae_sock_buff_t buffer;
				const int32_t result = (int32_t)recv( m_sock, &buffer, 1, MSG_PEEK );
				if( result == -1 && errno != EWOULDBLOCK && errno != EAGAIN )
				{
					Disconnect();
				}
				else if( result == 0 )
				{
					// Discard zero length packet
					if( recv( m_sock, &buffer, 0, 0 ) != 0 )
					{
						Disconnect();
					}
					continue;
				}
			}
			return false;
		}

		AE_ASSERT( readSize );
		uint32_t totalSize = m_recvData.Length() + readSize;
		m_recvData.Reserve( totalSize );
		_ae_sock_buff_t* buffer = (_ae_sock_buff_t*)m_recvData.end();
		while( m_recvData.Length() < totalSize ) { m_recvData.Append( {} ); } // @TODO: Should be single function call
		AE_ASSERT( buffer == (_ae_sock_buff_t*)m_recvData.end() - readSize );
		
		const int32_t result = (int32_t)recv( m_sock, buffer, readSize, 0 );
		if( result < 0 && ( errno == EWOULDBLOCK || errno == EAGAIN ) )
		{
			return false;
		}
		else if( result == 0 && m_protocol == Protocol::TCP )
		{
			Disconnect(); // Orderly shutdown
			return false;
		}
		else if( result )
		{
			AE_ASSERT( result <= (int32_t)readSize );
			// ioctl with FIONREAD includes udp headers on some platforms so use actual read length here
			if( result < (int32_t)readSize )
			{
				totalSize -= ( readSize - result );
				while( m_recvData.Length() > totalSize ) { m_recvData.Remove( m_recvData.Length() - 1 ); } // @TODO: Should be single function call
			}
			break; // Received new data!
		}
	}
	
	if( m_recvData.Length() >= m_readHead + offset + length )
	{
		if( dataOut )
		{
			memcpy( dataOut, m_recvData.Data() + m_readHead + offset, length );
		}
		return true;
	}
	return false;
}

bool Socket::ReceiveData( void* dataOut, uint16_t length )
{
	if( PeekData( dataOut, length, 0 ) )
	{
		bool discardSuccess = DiscardData( length );
		AE_ASSERT( discardSuccess );
		return true;
	}
	return false;
}

bool Socket::DiscardData( uint16_t length )
{
	if( m_recvData.Length() >= m_readHead + length )
	{
		m_readHead += length;
		if( m_readHead == m_recvData.Length() )
		{
			m_recvData.Clear();
			m_readHead = 0;
		}
		return true;
	}
	return false;
}

uint32_t Socket::ReceiveDataLength()
{
	PeekData( nullptr, 1, 0 );
	return m_recvData.Length() - m_readHead;
}

bool Socket::QueueMsg( const void* data, uint16_t length )
{
	if( !IsConnected() || !length )
	{
		return false;
	}
	AE_ASSERT( length <= ae::MaxValue< uint16_t >() );
	uint16_t length16 = htons( length );
	m_sendData.AppendArray( (const uint8_t*)&length16, sizeof(length16) );
	m_sendData.AppendArray( (const uint8_t*)data, length );
	return true;
}

uint16_t Socket::ReceiveMsg( void* dataOut, uint16_t maxLength )
{
	uint16_t length = 0;
	if( PeekData( &length, sizeof(length), 0 ) )
	{
		length = ntohs( length );
		if( length > maxLength )
		{
			return length;
		}
		else if( PeekData( dataOut, length, 2 ) )
		{
			DiscardData( length + 2 );
			return length;
		}
	}
	return 0;
}

bool Socket::DiscardMsg()
{
	uint16_t length = 0;
	if( PeekData( &length, sizeof(length), 0 ) )
	{
		length = ntohs( length );
		if( PeekData( nullptr, length, 2 ) )
		{
			DiscardData( length + 2 );
			return true;
		}
	}
	return false;
}

uint32_t Socket::SendAll()
{
	if( !IsConnected() || !m_sendData.Length() )
	{
		return 0;
	}
	
	int sendFlags = 0;
#if !_AE_WINDOWS_
	sendFlags |= MSG_NOSIGNAL;
#endif
	int32_t result = (int32_t)send( m_sock, (const _ae_sock_buff_t*)m_sendData.Data(), m_sendData.Length(), sendFlags );
	if( result == -1 && errno != EAGAIN && errno != EWOULDBLOCK )
	{
		Disconnect();
		return 0;
	}
	else
	{
		AE_ASSERT( m_sendData.Length() == result );
		m_sendData.Clear();
		return result;
	}
	return 0;
}

//------------------------------------------------------------------------------
// ae::ListenerSocket member functions
//------------------------------------------------------------------------------
ListenerSocket::ListenerSocket( ae::Tag tag ) :
	m_tag( tag ),
	m_connections( tag )
{}

ListenerSocket::~ListenerSocket()
{
	AE_ASSERT_MSG( !m_connections.Length(), "Allocated connections must be destroyed before ae::ListenerSocket destruction" );
	StopListening();
}

bool ListenerSocket::Listen( ae::Socket::Protocol proto, bool allowRemote, uint16_t port, uint32_t maxConnections )
{
	if( proto == ae::Socket::Protocol::None || !port )
	{
		return false;
	}

	if( !_WinsockInit() )
	{
		return false;
	}
	
	StopListening();
	
	addrinfo* addrInfo = nullptr;
	addrinfo hints;
	memset( &hints, 0, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = ( proto == ae::Socket::Protocol::TCP ) ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	ae::Str16 portStr = ae::Str16::Format( "#", port );
	if( getaddrinfo( allowRemote ? nullptr : "localhost", portStr.c_str(), &hints, (addrinfo**)&addrInfo ) == -1 )
	{
		return false;
	}
	for(; addrInfo; addrInfo = addrInfo->ai_next )
	{
		int* sock;
		if( addrInfo->ai_family == AF_INET && m_sock4 < 0 )
		{
			sock = &m_sock4;
		}
		else if( addrInfo->ai_family == AF_INET6 && m_sock6 < 0 )
		{
			sock = &m_sock6;
		}
		else
		{
			continue;
		}

		*sock = socket( addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol );
		if( *sock == -1 )
		{
			continue;
		}
		
		if( _DisableBlocking( *sock )
			&& _ReuseAddress( *sock )
			&& bind( *sock, addrInfo->ai_addr, addrInfo->ai_addrlen ) != -1
			&& ( proto == ae::Socket::Protocol::UDP || listen( *sock, 1 ) != -1 ) )
		{
			continue; // Success!
		}

		_CloseSocket( *sock );
		*sock = -1;
	}

	m_maxConnections = maxConnections;
	m_connections.Reserve( maxConnections );
	m_protocol = proto;
	m_port = port;
	return ( m_sock4 >= 0 ) || ( m_sock6 >= 0 );
}

	
bool ListenerSocket::IsListening() const
{
	if( ( m_sock4 >= 0 ) || ( m_sock6 >= 0 ) )
	{
		return true;
	}
	AE_ASSERT( m_protocol == ae::Socket::Protocol::None );
	AE_ASSERT( m_port == 0 );
	return false;
}

ae::Socket* ListenerSocket::Accept()
{
	if( !m_sock4 && !m_sock6 )
	{
		AE_ASSERT( m_protocol == ae::Socket::Protocol::None );
		AE_ASSERT( m_port == 0 );
		return nullptr;
	}
	AE_ASSERT( m_protocol != ae::Socket::Protocol::None );
	
	// @TODO: It's possible that m_maxConnections should be handled by not listening
	// (in addition to the existing checks) so that failed connections attempts
	// are handled at a lower level in the networking stack (ICMP) for both
	// TCP and UDP. This should prevent connecting clients from seeing a successful
	// connection which is immediately lost.
	
	int* listenSocks[] = { &m_sock4, &m_sock6 };
	for( uint32_t i = 0; i < countof(listenSocks); i++ )
	{
		int& listenSock = *(listenSocks[ i ]);
		if( !listenSock )
		{
			continue;
		}
		
		int newSock = -1;
		sockaddr_storage sockAddr;
		socklen_t sockAddrLen = sizeof(sockAddr);
		if( m_protocol == ae::Socket::Protocol::TCP )
		{
			newSock = accept( listenSock, (sockaddr*)&sockAddr, &sockAddrLen );
			if( newSock == -1 )
			{
				if( errno != EAGAIN && errno != EWOULDBLOCK )
				{
					StopListening();
					return nullptr;
				}
				continue;
			}
			
			if( ( m_connections.Length() >= m_maxConnections )
				|| !_DisableBlocking( newSock )
				|| !_DisableNagles( newSock ) )
			{
				_CloseSocket( newSock );
				newSock = -1;
				continue;
			}
		}
		else if( m_protocol == ae::Socket::Protocol::UDP )
		{
			// Discard all pending messages when max connections are established
			if( m_connections.Length() >= m_maxConnections )
			{
				_ae_sock_buff_t buffer;
				int32_t result = (int32_t)recv( listenSock, &buffer, sizeof(buffer), 0 );
				if( result == -1 && errno != EAGAIN && errno != EWOULDBLOCK )
				{
					StopListening();
					return nullptr;
				}
				continue;
			}
			
			_ae_sock_buff_t buffer;
			int32_t numbytes = (int32_t)recvfrom( listenSock, &buffer, sizeof(buffer), MSG_PEEK, (sockaddr*)&sockAddr, &sockAddrLen );
			if( numbytes == -1 )
			{
				if( errno != EAGAIN && errno != EWOULDBLOCK )
				{
					StopListening();
					return nullptr;
				}
				continue;
			}
			
			sockaddr_storage listenSockAddr;
			socklen_t listenSockAddrLen = sizeof(listenSockAddr);
			if( getsockname( listenSock, (sockaddr*)&listenSockAddr, &listenSockAddrLen ) == -1 )
			{
				continue;
			}
			char addrStr[ INET6_ADDRSTRLEN ];
			_GetAddressString( (sockaddr*)&listenSockAddr, addrStr );
			
			// Connect and give old listening socket to new ae::Socket
			newSock = listenSock;
			if( !_DisableBlocking( newSock )
				|| !_DisableNagles( newSock )
				|| ( connect( newSock, (sockaddr*)&sockAddr, sockAddrLen ) == -1 ) )
			{
				_CloseSocket( newSock );
				newSock = -1;
				continue;
			}
			
			// Create another listening socket
			listenSock = socket( listenSockAddr.ss_family, SOCK_DGRAM, 0 );
			if( listenSock == -1 )
			{
				listenSock = -1;
				continue;
			}
			if( !_DisableBlocking( listenSock )
				|| !_ReuseAddress( listenSock )
				|| bind( listenSock, (sockaddr*)&listenSockAddr, listenSockAddrLen ) == -1 )
			{
				_CloseSocket( listenSock );
				listenSock = -1;
			}
		}
		
		char addrStr[ INET6_ADDRSTRLEN ];
		if( !_GetAddressString( (sockaddr*)&sockAddr, addrStr ) )
		{
			_CloseSocket( newSock );
			continue;
		}
		
		AE_ASSERT( newSock >= 0 );
		ae::Socket* s = ae::New< ae::Socket >( m_tag, m_tag, newSock, m_protocol, addrStr, _GetPort( (sockaddr*)&sockAddr ) );
		return m_connections.Append( s );
	}
	return nullptr;
}

void ListenerSocket::StopListening()
{
	_CloseSocket( m_sock4 );
	_CloseSocket( m_sock6 );
	m_sock4 = -1;
	m_sock6 = -1;
	m_protocol = ae::Socket::Protocol::None;
	m_port = 0;
}

void ListenerSocket::Destroy( ae::Socket* sock )
{
	ae::Delete( sock );
	m_connections.RemoveAll( sock );
}

void ListenerSocket::DestroyAll()
{
	for( ae::Socket* sock : m_connections )
	{
		ae::Delete( sock );
	}
	m_connections.Clear();
}

ae::Socket* ListenerSocket::GetConnection( uint32_t idx )
{
	return m_connections[ idx ];
}

uint32_t ListenerSocket::GetConnectionCount() const
{
	return m_connections.Length();
}

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
	#define GL_GLEXT_PROTOTYPES 1
	#include <GL/glcorearb.h>
#elif _AE_IOS_
	#include <OpenGLES/ES3/gl.h>
#else
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
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
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
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
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
// WGL extensions
bool ( *wglSwapIntervalEXT ) ( int interval ) = nullptr;
int ( *wglGetSwapIntervalEXT ) () = nullptr;
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
void ( *glVertexAttribDivisor )( GLuint index, GLuint divisor ) = nullptr;
void ( *glDrawElementsInstanced )( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount ) = nullptr;
void ( *glDrawArraysInstanced )( GLenum mode, GLint first, GLsizei count, GLsizei instancecount ) = nullptr;
// Debug functions
void ( *glDebugMessageCallback ) ( GLDEBUGPROC callback, const void* userParam ) = nullptr;
#endif

#if _AE_EMSCRIPTEN_ || _AE_IOS_
#define glClearDepth glClearDepthf
#endif

// Helpers
// clang-format off
#if _AE_DEBUG_
	#define AE_CHECK_GL_ERROR() do { if( GLenum err = glGetError() ) { AE_FAIL_MSG( "GL Error: #", err ); } } while( 0 )
#else
	#define AE_CHECK_GL_ERROR() do {} while( 0 )
#endif
// clang-format on

namespace ae {

int32_t _GLGetTypeCount( uint32_t glType )
{
	switch( glType )
	{
		case GL_SAMPLER_2D: return 0;
		case GL_SAMPLER_3D: return 0;
		case GL_FLOAT: return 1;
		case GL_FLOAT_VEC2: return 2;
		case GL_FLOAT_VEC3: return 3;
		case GL_FLOAT_VEC4: return 4;
		case GL_FLOAT_MAT4: return 16;
		default: return -1;
	}
}

void CheckFramebufferComplete( GLuint framebuffer )
{
	GLenum fboStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( fboStatus != GL_FRAMEBUFFER_COMPLETE )
	{
		const char* errStr = "unknown";
		switch( fboStatus )
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

#if _AE_DEBUG_ && !_AE_APPLE_ && !_AE_EMSCRIPTEN_
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
	//switch( type )
	//{
	//	case GL_DEBUG_TYPE_ERROR:
	//		std::cout << "ERROR";
	//		break;
	//	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
	//		std::cout << "DEPRECATED_BEHAVIOR";
	//		break;
	//	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
	//		std::cout << "UNDEFINED_BEHAVIOR";
	//		break;
	//	case GL_DEBUG_TYPE_PORTABILITY:
	//		std::cout << "PORTABILITY";
	//		break;
	//	case GL_DEBUG_TYPE_PERFORMANCE:
	//		std::cout << "PERFORMANCE";
	//		break;
	//	case GL_DEBUG_TYPE_OTHER:
	//		std::cout << "OTHER";
	//		break;
	//}
	//std::cout << std::endl;

	//std::cout << "id: " << id << std::endl;
	//std::cout << "severity: ";
	switch( severity )
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

	if( severity == GL_DEBUG_SEVERITY_HIGH )
	{
		AE_FAIL();
	}
}
#endif

GLenum VertexTypeToGL( Vertex::Type type )
{
	switch( type )
	{
		case Vertex::Type::UInt8:
			return GL_UNSIGNED_BYTE;
		case Vertex::Type::UInt16:
			return GL_UNSIGNED_SHORT;
		case Vertex::Type::UInt32:
			return GL_UNSIGNED_INT;
		case Vertex::Type::NormalizedUInt8:
			return GL_UNSIGNED_BYTE;
		case Vertex::Type::NormalizedUInt16:
			return GL_UNSIGNED_SHORT;
		case Vertex::Type::NormalizedUInt32:
			return GL_UNSIGNED_INT;
		case Vertex::Type::Float:
			return GL_FLOAT;
		default:
			AE_FAIL();
			return 0;
	}
}

typedef uint32_t _kQuadIndex;
const uint32_t _kQuadVertCount = 4;
const uint32_t _kQuadIndexCount = 6;
extern const Vec3 _kQuadVertPos[ _kQuadVertCount ];
extern const Vec2 _kQuadVertUvs[ _kQuadVertCount ];
extern const _kQuadIndex _kQuadIndices[ _kQuadIndexCount ];
const Vec3 _kQuadVertPos[ _kQuadVertCount ] = {
	Vec3( -0.5f, -0.5f, 0.0f ),
	Vec3( 0.5f, -0.5f, 0.0f ),
	Vec3( 0.5f, 0.5f, 0.0f ),
	Vec3( -0.5f, 0.5f, 0.0f )
};
const Vec2 _kQuadVertUvs[ _kQuadVertCount ] = {
	Vec2( 0.0f, 0.0f ),
	Vec2( 1.0f, 0.0f ),
	Vec2( 1.0f, 1.0f ),
	Vec2( 0.0f, 1.0f )
};
const _kQuadIndex _kQuadIndices[ _kQuadIndexCount ] = {
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
	AE_ASSERT_MSG( m_uniforms.Length() < m_uniforms.Size() || m_uniforms.TryGet( name ), "Max uniforms: #", m_uniforms.Size() );
	Value& uniform = m_uniforms.Set( name, Value() );
	uniform.size = 1;
	uniform.value.data[ 0 ] = value;
	m_hash.HashString( name );
	m_hash.HashType( value );
}

void UniformList::Set( const char* name, Vec2 value )
{
	AE_ASSERT( name );
	AE_ASSERT( name[ 0 ] );
	AE_ASSERT_MSG( m_uniforms.Length() < m_uniforms.Size() || m_uniforms.TryGet( name ), "Max uniforms: #", m_uniforms.Size() );
	Value& uniform = m_uniforms.Set( name, Value() );
	uniform.size = 2;
	uniform.value.data[ 0 ] = value.x;
	uniform.value.data[ 1 ] = value.y;
	m_hash.HashString( name );
	m_hash.HashType( value.data );
}

void UniformList::Set( const char* name, Vec3 value )
{
	AE_ASSERT( name );
	AE_ASSERT( name[ 0 ] );
	AE_ASSERT_MSG( m_uniforms.Length() < m_uniforms.Size() || m_uniforms.TryGet( name ), "Max uniforms: #", m_uniforms.Size() );
	Value& uniform = m_uniforms.Set( name, Value() );
	uniform.size = 3;
	uniform.value.data[ 0 ] = value.x;
	uniform.value.data[ 1 ] = value.y;
	uniform.value.data[ 2 ] = value.z;
	m_hash.HashString( name );
	m_hash.HashType( value.data );
}

void UniformList::Set( const char* name, Vec4 value )
{
	AE_ASSERT( name );
	AE_ASSERT( name[ 0 ] );
	AE_ASSERT_MSG( m_uniforms.Length() < m_uniforms.Size() || m_uniforms.TryGet( name ), "Max uniforms: #", m_uniforms.Size() );
	Value& uniform = m_uniforms.Set( name, Value() );
	uniform.size = 4;
	uniform.value.data[ 0 ] = value.x;
	uniform.value.data[ 1 ] = value.y;
	uniform.value.data[ 2 ] = value.z;
	uniform.value.data[ 3 ] = value.w;
	m_hash.HashString( name );
	m_hash.HashType( value.data );
}

void UniformList::Set( const char* name, const Matrix4& value )
{
	AE_ASSERT( name );
	AE_ASSERT( name[ 0 ] );
	AE_ASSERT_MSG( m_uniforms.Length() < m_uniforms.Size() || m_uniforms.TryGet( name ), "Max uniforms: #", m_uniforms.Size() );
	Value& uniform = m_uniforms.Set( name, Value() );
	uniform.size = 16;
	uniform.value = value;
	m_hash.HashString( name );
	m_hash.HashType( value.data );
}

void UniformList::Set( const char* name, const Texture* tex )
{
	AE_ASSERT( name );
	AE_ASSERT( name[ 0 ] );
	AE_ASSERT_MSG( tex, "Texture uniform value '#' is invalid", name );
	AE_ASSERT_MSG( tex->GetTexture(), "Texture uniform value '#' is invalid", name );
	AE_ASSERT_MSG( m_uniforms.Length() < m_uniforms.Size() || m_uniforms.TryGet( name ), "Max uniforms: #", m_uniforms.Size() );
	Value& uniform = m_uniforms.Set( name, Value() );
	uniform.sampler = tex->GetTexture();
	uniform.target = tex->GetTarget();
	m_hash.HashString( name );
	m_hash.HashType( tex->GetTexture() );
	m_hash.HashType( tex->GetTarget() );
}

const UniformList::Value* UniformList::Get( const char* name ) const
{
	return m_uniforms.TryGet( name );
}

//------------------------------------------------------------------------------
// ae::Shader member functions
//------------------------------------------------------------------------------
ae::Hash32 s_shaderHash;
ae::Hash32 s_uniformHash;

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

	if( !m_vertexShader || !m_fragmentShader )
	{
		AE_FAIL();
	}

	glAttachShader( m_program, m_vertexShader );
	glAttachShader( m_program, m_fragmentShader );

	glLinkProgram( m_program );

	// immediate reflection of shader can be delayed by compiler and optimizer and can stll
	GLint status;
	glGetProgramiv( m_program, GL_LINK_STATUS, &status );
	if( status == GL_FALSE )
	{
		GLint logLength = 0;
		glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &logLength );

		char* log = nullptr;
		if( logLength > 0 )
		{
			log = new char[ logLength ];
			glGetProgramInfoLog( m_program, logLength, NULL, (GLchar*)log );
		}

		if( log )
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
	for( int32_t i = 0; i < attribCount; i++ )
	{
		_Attribute* attribute = &m_attributes.Append( _Attribute() );

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

	for( int32_t i = 0; i < uniformCount; i++ )
	{
		_Uniform uniform;

		GLint size = 0;
		char name[ Str32::MaxLength() ]; // @TODO: Read from Uniform
		glGetActiveUniform( m_program, i, sizeof( name ), nullptr, &size, (GLenum*)&uniform.type, (GLchar*)name );
		AE_ASSERT( size == 1 );

		switch( uniform.type )
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
	if( m_program )
	{
		AE_CHECK_GL_ERROR();
	}

	m_attributes.Clear();
	m_uniforms.Clear();

	if( m_fragmentShader != 0 )
	{
		glDeleteShader( m_fragmentShader );
		m_fragmentShader = 0;
	}

	if( m_vertexShader != 0 )
	{
		glDeleteShader( m_vertexShader );
		m_vertexShader = 0;
	}

	if( m_program != 0 )
	{
		glDeleteProgram( m_program );
		m_program = 0;
		AE_CHECK_GL_ERROR();
	}
}

void Shader::m_Activate( const UniformList& uniforms ) const
{
	ae::Hash32 shaderHash;
	shaderHash.HashType( this );
	shaderHash.HashType( m_blending );
	shaderHash.HashType( m_blendingPremul );
	shaderHash.HashType( m_depthWrite );
	shaderHash.HashType( m_depthTest );
	shaderHash.HashType( m_culling );
	shaderHash.HashType( m_wireframe );
	bool shaderDirty = ( s_shaderHash != shaderHash );
	if( shaderDirty )
	{
		s_shaderHash = shaderHash;
		
		AE_CHECK_GL_ERROR();

		// Blending
		if( m_blending || m_blendingPremul )
		{
			glEnable( GL_BLEND );

			// TODO: need other modes like Add, Min, Max - switch to enum then
			if( m_blendingPremul )
			{
				// Colors coming out of shader already have alpha multiplied in.
				glBlendFuncSeparate( GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
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
		if( m_depthTest )
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
		if( m_culling == Culling::None )
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
	if( !shaderDirty && s_uniformHash == uniforms.GetHash() )
	{
		return;
	}
	s_uniformHash = uniforms.GetHash();
	
	// Set shader uniforms
	bool missingUniforms = false;
	uint32_t textureIndex = 0;
	for( uint32_t i = 0; i < m_uniforms.Length(); i++ )
	{
		const char* uniformVarName = m_uniforms.GetKey( i ).c_str();
		const _Uniform* uniformVar = &m_uniforms.GetValue( i );
		const UniformList::Value* uniformValue = uniforms.Get( uniformVarName );

		// Validation
		{
			if( !uniformValue )
			{
				AE_WARN( "Shader uniform '#' value is not set", uniformVarName );
				missingUniforms = true;
				continue;
			}
			const int32_t typeSize = ae::_GLGetTypeCount( uniformVar->type );
			AE_ASSERT_MSG( typeSize >= 0, "Unsupported uniform '#' type #", uniformVarName, uniformVar->type );
			AE_ASSERT_MSG( uniformValue->size == typeSize, "Uniform size mismatch '#' type:# var:# param:#", uniformVarName, uniformVar->type, typeSize, uniformValue->size );
		}

		if( uniformVar->type == GL_SAMPLER_2D )
		{
			AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid", uniformVarName );
			glActiveTexture( GL_TEXTURE0 + textureIndex );
			glBindTexture( uniformValue->target, uniformValue->sampler );
			glUniform1i( uniformVar->location, textureIndex );
			textureIndex++;
		}
		else if( uniformVar->type == GL_SAMPLER_3D )
		{
			AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid", uniformVarName );
			glActiveTexture( GL_TEXTURE0 + textureIndex );
			glBindTexture( GL_TEXTURE_3D, uniformValue->sampler );
			glUniform1i( uniformVar->location, textureIndex );
			textureIndex++;
		}
		else if( uniformVar->type == GL_FLOAT )
		{
			glUniform1fv( uniformVar->location, 1, uniformValue->value.data );
		}
		else if( uniformVar->type == GL_FLOAT_VEC2 )
		{
			glUniform2fv( uniformVar->location, 1, uniformValue->value.data );
		}
		else if( uniformVar->type == GL_FLOAT_VEC3 )
		{
			glUniform3fv( uniformVar->location, 1, uniformValue->value.data );
		}
		else if( uniformVar->type == GL_FLOAT_VEC4 )
		{
			glUniform4fv( uniformVar->location, 1, uniformValue->value.data );
		}
		else if( uniformVar->type == GL_FLOAT_MAT4 )
		{
			glUniformMatrix4fv( uniformVar->location, 1, GL_FALSE, uniformValue->value.data );
		}
		else
		{
			AE_FAIL_MSG( "Invalid uniform type '#': #", uniformVarName, uniformVar->type );
		}

		AE_CHECK_GL_ERROR();
	}

	AE_ASSERT_MSG( !missingUniforms, "Missing shader uniform parameters" );
}

const ae::Shader::_Attribute* Shader::m_GetAttributeByIndex( uint32_t index ) const
{
	return &m_attributes[ index ];
}

int Shader::m_LoadShader( const char* shaderStr, Type type, const char* const* defines, int32_t defineCount )
{
	AE_ASSERT( defineCount <= _kMaxShaderDefines );
	GLenum glType = -1;
	if( type == Type::Vertex )
	{
		glType = GL_VERTEX_SHADER;
	}
	if( type == Type::Fragment )
	{
		glType = GL_FRAGMENT_SHADER;
	}

	const uint32_t kPrependMax = 16;
	ae::Array< const char*, kPrependMax + _kMaxShaderDefines * 2 + 1 > shaderSource;// x2 max defines to make room for newlines. Plus one for actual shader.

	// Version
	ae::Str32 glVersionStr = "#version ";
#if _AE_IOS_ || _AE_EMSCRIPTEN_
	glVersionStr += ae::Str16::Format( "##0 es", ae::GLMajorVersion, ae::GLMinorVersion );
#else
	glVersionStr += ae::Str16::Format( "##0 core", ae::GLMajorVersion, ae::GLMinorVersion );
#endif
	glVersionStr += "\n";
	if( glVersionStr.Length() )
	{
		shaderSource.Append( glVersionStr.c_str() );
	}

	// Precision
#if _AE_IOS_ || _AE_EMSCRIPTEN_
	shaderSource.Append( "precision highp float;\n" );
#else
	// No default precision specified
#endif

	// Input/output
//	#if _AE_EMSCRIPTEN_
//	shaderSource.Append( "#define AE_COLOR gl_FragColor\n" );
//	shaderSource.Append( "#define AE_TEXTURE2D texture2d\n" );
//	shaderSource.Append( "#define AE_UNIFORM_HIGHP uniform highp\n" );
//	if( type == Type::Vertex )
//	{
//		shaderSource.Append( "#define AE_IN_HIGHP attribute highp\n" );
//		shaderSource.Append( "#define AE_OUT_HIGHP varying highp\n" );
//	}
//	else if( type == Type::Fragment )
//	{
//		shaderSource.Append( "#define AE_IN_HIGHP varying highp\n" );
//		shaderSource.Append( "#define AE_UNIFORM_HIGHP uniform highp\n" );
//	}
// #else
	shaderSource.Append( "#define AE_TEXTURE2D texture\n" );
	shaderSource.Append( "#define AE_UNIFORM uniform\n" );
	shaderSource.Append( "#define AE_UNIFORM_HIGHP uniform\n" );
	shaderSource.Append( "#define AE_IN_HIGHP in\n" );
	shaderSource.Append( "#define AE_OUT_HIGHP out\n" );
	if( type == Type::Fragment )
	{
		shaderSource.Append( "out vec4 AE_COLOR;\n" );
	}
// #endif

	AE_ASSERT( shaderSource.Length() <= kPrependMax );

	for( int32_t i = 0; i < defineCount; i++ )
	{
		shaderSource.Append( defines[ i ] );
		shaderSource.Append( "\n" );
	}

	shaderSource.Append( shaderStr );

	GLuint shader = glCreateShader( glType );
	glShaderSource( shader, shaderSource.Length(), shaderSource.Data(), nullptr );
	glCompileShader( shader );

	GLint status;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status == GL_FALSE )
	{
		const char* typeStr = ( type == Type::Vertex ? "vertex" : "fragment" );
		AE_ERR( "Failed to load # shader! #", typeStr, shaderStr );

		GLint logLength;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );

		if( logLength > 0 )
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
// ae::VertexBuffer member functions
//------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{
	Terminate();
}

void VertexBuffer::Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, ae::Vertex::Primitive primitive, ae::Vertex::Usage vertexUsage, ae::Vertex::Usage indexUsage )
{
	Terminate();

	AE_ASSERT( m_vertexSize == 0 );
	AE_ASSERT( vertexSize );
	AE_ASSERT( m_indexSize == 0 );
	AE_ASSERT( indexSize <= 4 && indexSize != 3 );
	if( indexSize ) { AE_ASSERT_MSG( maxIndexCount, "Must specify maxIndexCount with non-zero index size" ); }
	if( maxIndexCount ) { AE_ASSERT_MSG( indexSize, "Must specify an indexSize with non-zero index count" ); }
	AE_ASSERT_MSG( maxVertexCount, "VertexBuffer can't be initialized without storage" );

	m_maxVertexCount = maxVertexCount;
	m_maxIndexCount = maxIndexCount;
	m_primitive = primitive;
	m_vertexUsage = vertexUsage;
	m_indexUsage = indexUsage;
	m_vertexSize = vertexSize;
	m_indexSize = indexSize;
	
	glGenVertexArrays( 1, &m_array );
	glBindVertexArray( m_array );
	
	AE_CHECK_GL_ERROR();
}

void VertexBuffer::AddAttribute( const char *name, uint32_t componentCount, ae::Vertex::Type type, uint32_t offset )
{
	AE_ASSERT( m_vertices == ~0 && m_indices == ~0 );
	
	_Attribute* attribute = &m_attributes.Append( _Attribute() );
	
	size_t length = strlen( name );
	AE_ASSERT( length < _kMaxShaderAttributeNameLength );
	strcpy( attribute->name, name );
	attribute->componentCount = componentCount;
	attribute->type = VertexTypeToGL( type );
	attribute->offset = offset;
	attribute->normalized =
		type == Vertex::Type::NormalizedUInt8 ||
		type == Vertex::Type::NormalizedUInt16 ||
		type == Vertex::Type::NormalizedUInt32;
}

void VertexBuffer::Terminate()
{
	if( m_vertexSize )
	{
		AE_CHECK_GL_ERROR();
	}

	if( m_array )
	{
		glDeleteVertexArrays( 1, &m_array );
	}
	if( m_vertices != ~0 )
	{
		glDeleteBuffers( 1, &m_vertices );
	}
	if( m_indices != ~0 )
	{
		glDeleteBuffers( 1, &m_indices );
	}
	
	m_attributes.Clear();
	
	// Params
	m_vertexSize = 0;
	m_indexSize = 0;
	m_primitive = (ae::Vertex::Primitive)-1;
	m_vertexUsage = (ae::Vertex::Usage)-1;
	m_indexUsage = (ae::Vertex::Usage)-1;
	m_maxVertexCount = 0;
	m_maxIndexCount = 0;
	// System resources
	m_array = 0;
	m_vertices = ~0;
	m_indices = ~0;
	
	if( m_vertexSize )
	{
		AE_CHECK_GL_ERROR();
	}
}

void VertexBuffer::UploadVertices( uint32_t startIdx, const void* vertices, uint32_t count )
{
	AE_ASSERT( m_vertexSize );
	AE_ASSERT_MSG( ( startIdx + count ) <= m_maxVertexCount, "Vertex start: # count: # max: #", startIdx, count, m_maxVertexCount );
	if( m_indices != ~0 )
	{
		AE_ASSERT( m_indexSize != 0 );
	}
	if( m_indexSize )
	{
		AE_ASSERT_MSG( count <= (uint64_t)1 << ( m_indexSize * 8 ), "Vertex count (#) too high for index of size #", count, m_indexSize );
	}
	
	if( m_vertexUsage == Vertex::Usage::Static )
	{
		AE_ASSERT( count );
		AE_ASSERT( m_vertices == ~0 );
		AE_ASSERT( startIdx == 0 ); // @TODO: Remove this, shouldn't force verts to start from zero

		glGenBuffers( 1, &m_vertices );
		glBindVertexArray( m_array );
		glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
		glBufferData( GL_ARRAY_BUFFER, count * m_vertexSize, vertices, GL_STATIC_DRAW );
		AE_CHECK_GL_ERROR();
		return;
	}
	if( m_vertexUsage == Vertex::Usage::Dynamic )
	{
		if( !count )
		{
			return;
		}
		
		if( m_vertices == ~0 )
		{
			glGenBuffers( 1, &m_vertices );
			glBindVertexArray( m_array );
			glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
			glBufferData( GL_ARRAY_BUFFER, m_maxVertexCount * m_vertexSize, nullptr, GL_DYNAMIC_DRAW );
		}
		else
		{
			glBindVertexArray( m_array );
			glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
		}
		
		glBufferSubData( GL_ARRAY_BUFFER, startIdx * m_vertexSize, count * m_vertexSize, vertices );
		AE_CHECK_GL_ERROR();
		return;
	}
	AE_FAIL();
}

void VertexBuffer::UploadIndices( uint32_t startIdx, const void* indices, uint32_t count )
{
	AE_ASSERT( IsIndexed() );
	AE_ASSERT_MSG( count % 3 == 0, "Index count: #", count );
	AE_ASSERT_MSG( ( startIdx + count ) <= m_maxIndexCount, "Index start: # count: # max: #", startIdx, count, m_maxIndexCount );
	
	if( m_indexUsage == Vertex::Usage::Static )
	{
		AE_ASSERT( count );
		AE_ASSERT( m_indices == ~0 );
		AE_ASSERT( startIdx == 0 ); // @TODO: Remove this, shouldn't force indices to start from zero

		glGenBuffers( 1, &m_indices );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, count * m_indexSize, indices, GL_STATIC_DRAW );
		AE_CHECK_GL_ERROR();
		return;
	}
	if( m_indexUsage == Vertex::Usage::Dynamic )
	{
		if( !count )
		{
			return;
		}
		
		if( m_indices == ~0 )
		{
			glGenBuffers( 1, &m_indices );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_maxIndexCount * m_indexSize, nullptr, GL_DYNAMIC_DRAW );
		}
		else
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
		}
		
		glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, startIdx * m_indexSize, count * m_indexSize, indices );
		AE_CHECK_GL_ERROR();
		return;
	}
	AE_FAIL();
}

void VertexBuffer::Bind( const Shader* shader, const UniformList& uniforms, const InstanceData** instanceDatas, uint32_t instanceDataCount ) const
{
	AE_ASSERT( shader );
	AE_ASSERT_MSG( m_vertexSize, "Must call Initialize() before Bind()" );
	for( uint32_t i = 0; i < instanceDataCount; i++ )
	{
		if( instanceDatas[ i ]->_GetBuffer() == ~0 )
		{
			return;
		}
	}
	if( m_vertices == ~0 || ( IsIndexed() && m_indices == ~0 ) )
	{
		return;
	}
	
	switch( m_primitive )
	{
		case Vertex::Primitive::Triangle:
		case Vertex::Primitive::Line:
		case Vertex::Primitive::Point:
			break;
		default:
			AE_FAIL();
			return;
	}

	shader->m_Activate( uniforms );

	glBindVertexArray( m_array );
	AE_CHECK_GL_ERROR();

	for( uint32_t i = 0; i < shader->m_GetAttributeCount(); i++ )
	{
		const Shader::_Attribute* shaderAttribute = shader->m_GetAttributeByIndex( i );
		const ae::Str32 attribName = shaderAttribute->name;
		AE_STATIC_ASSERT( attribName.MaxLength() >= _kMaxShaderAttributeNameLength );

		GLint location = shaderAttribute->location;
		AE_ASSERT( location >= 0 );
		glEnableVertexAttribArray( location );
		AE_CHECK_GL_ERROR();

		const ae::InstanceData* instanceData = nullptr;
		const _Attribute* instanceAttrib = nullptr;
		for( uint32_t i = 0; i < instanceDataCount; i++ )
		{
			if( const ae::InstanceData* inst = instanceDatas[ i ] )
			{
				instanceAttrib = inst->_GetAttribute( attribName.c_str() );
				if( instanceAttrib )
				{
					instanceData = inst;
					break;
				}
			}
		}

		if( instanceData )
		{
			AE_ASSERT( instanceAttrib );

			glBindBuffer( GL_ARRAY_BUFFER, instanceData->_GetBuffer() );
			AE_CHECK_GL_ERROR();

			uint32_t componentCount = instanceAttrib->componentCount;
			uint32_t type = instanceAttrib->type;
			bool normalized = instanceAttrib->normalized;
			uint32_t dataSize = instanceData->GetStride();
			uint64_t attribOffset = instanceAttrib->offset;
			if( componentCount == 16 ) // Matrix4
			{
				glEnableVertexAttribArray( location + 1 );
				glEnableVertexAttribArray( location + 2 );
				glEnableVertexAttribArray( location + 3 );
				
				glVertexAttribPointer( location, 4, type, normalized, dataSize, (void*)attribOffset );
				glVertexAttribPointer( location + 1, 4, type, normalized, dataSize, (void*)( attribOffset + sizeof(ae::Vec4) ) );
				glVertexAttribPointer( location + 2, 4, type, normalized, dataSize, (void*)( attribOffset + sizeof(ae::Vec4) * 2 ) );
				glVertexAttribPointer( location + 3, 4, type, normalized, dataSize, (void*)( attribOffset + sizeof(ae::Vec4) * 3 ) );
				
				glVertexAttribDivisor( location + 1, 1 );
				glVertexAttribDivisor( location + 2, 1 );
				glVertexAttribDivisor( location + 3, 1 );
			}
			else
			{
				glVertexAttribPointer( location, componentCount, type, normalized, dataSize, (void*)attribOffset );
			}
			AE_CHECK_GL_ERROR();
		}
		else
		{
			int32_t idx = m_attributes.FindFn( [ attribName ]( const _Attribute& a ){ return a.name == attribName; } );
			AE_ASSERT_MSG( idx >= 0, "Shader requires missing vertex attribute '#'", attribName );
			const _Attribute* vertexAttribute = &m_attributes[ idx ];
			const uint32_t shaderAttribComponentCount = ae::_GLGetTypeCount( shaderAttribute->type );
			AE_ASSERT_MSG( (int32_t)vertexAttribute->componentCount >= shaderAttribComponentCount, "Shader vertex attribute '#' requires # componenents, but vertex data only provides #", attribName, shaderAttribComponentCount, vertexAttribute->componentCount );
			// @TODO: Verify attribute type matches

			glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
			AE_CHECK_GL_ERROR();

			uint32_t componentCount = vertexAttribute->componentCount;
			uint64_t attribOffset = vertexAttribute->offset;
			glVertexAttribPointer( location, componentCount, vertexAttribute->type, vertexAttribute->normalized, m_vertexSize, (void*)attribOffset );
			AE_CHECK_GL_ERROR();
		}

		glVertexAttribDivisor( location, instanceAttrib ? 1 : 0 );
		AE_CHECK_GL_ERROR();
	}

	#if !_AE_EMSCRIPTEN_ && !_AE_IOS_
	if( m_primitive == Vertex::Primitive::Point )
	{
		glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	}
	else
#endif
	if( IsIndexed() )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
		AE_CHECK_GL_ERROR();
	}
}

void VertexBuffer::Draw() const
{
	m_Draw( 0, GetMaxPrimitiveCount(), -1 );
}

void VertexBuffer::Draw( uint32_t primitiveStartIdx, uint32_t primitiveCount ) const
{
	m_Draw( primitiveStartIdx, primitiveCount, -1 );
}

void VertexBuffer::DrawInstanced( uint32_t primitiveStartIdx, uint32_t primitiveCount, uint32_t instanceCount ) const
{
	if( instanceCount )
	{
		m_Draw( primitiveStartIdx, primitiveCount, instanceCount );
	}
}

uint32_t VertexBuffer::GetMaxPrimitiveCount() const
{
	uint32_t primitiveSize = 0;
	if( m_primitive == Vertex::Primitive::Triangle ) { primitiveSize = 3; }
	else if( m_primitive == Vertex::Primitive::Line ) { primitiveSize = 2; }
	else if( m_primitive == Vertex::Primitive::Point ) { primitiveSize = 1; }
	else { AE_FAIL(); return 0; }
	return ( IsIndexed() ? m_maxIndexCount : m_maxVertexCount ) / primitiveSize;
}

void VertexBuffer::m_Draw( uint32_t primitiveStartIdx, uint32_t primitiveCount, int32_t instanceCount ) const
{
	AE_ASSERT_MSG( m_vertexSize, "Must call Initialize() before Draw()" );
	if( !primitiveCount || m_vertices == ~0 || ( IsIndexed() && m_indices == ~0 ) )
	{
		return;
	}
	
	GLenum mode = 0;
	uint32_t primitiveSize = 0;
	const char* primitiveTypeName = "";
	if( m_primitive == Vertex::Primitive::Triangle ) { mode = GL_TRIANGLES; primitiveSize = 3; primitiveTypeName = "Triangle"; }
	else if( m_primitive == Vertex::Primitive::Line ) { mode = GL_LINES; primitiveSize = 2; primitiveTypeName = "Line"; }
	else if( m_primitive == Vertex::Primitive::Point ) { mode = GL_POINTS; primitiveSize = 1; primitiveTypeName = "Point"; }
	else { AE_FAIL(); return; }
	
	if( IsIndexed() && mode != GL_POINTS )
	{
		AE_ASSERT( primitiveStartIdx + primitiveCount <= m_maxIndexCount / primitiveSize );
		int64_t start = primitiveStartIdx * primitiveSize * m_indexSize; // Byte offset into index buffer
		int32_t count = primitiveCount * primitiveSize; // Number of indices to render
		GLenum type = 0;
		if( m_indexSize == sizeof(uint8_t) ) { type = GL_UNSIGNED_BYTE; }
		else if( m_indexSize == sizeof(uint16_t) ) { type = GL_UNSIGNED_SHORT; }
		else if( m_indexSize == sizeof(uint32_t) ) { type = GL_UNSIGNED_INT; }
		// @TODO: Should validate that instanceCount is valid somehow. Bind() is const
		// so it's not possible to save maxInstanceCount. This probably needs a rework.
		if( instanceCount >= 0 )
		{
			glDrawElementsInstanced( mode, count, type, (void*)start, instanceCount );
		}
		else
		{
			glDrawElements( mode, count, type, (void*)start );
		}
		AE_CHECK_GL_ERROR();
	}
	else
	{
		AE_ASSERT( ( primitiveStartIdx + primitiveCount ) * primitiveSize <= m_maxVertexCount );
		GLint start = primitiveStartIdx * primitiveSize;
		GLsizei count = primitiveCount * primitiveSize;
		AE_ASSERT_MSG( count % primitiveSize == 0, "Vertex count must be a multiple of # when rendering #s without indices", primitiveSize, primitiveTypeName );
		if( instanceCount >= 0 )
		{
			glDrawArraysInstanced( mode, start, count, instanceCount );
		}
		else
		{
			glDrawArrays( mode, start, count );
		}
		AE_CHECK_GL_ERROR();
	}
}

//------------------------------------------------------------------------------
// ae::VertexArray member functions
//------------------------------------------------------------------------------
VertexArray::VertexArray( ae::Tag tag ) :
	m_tag( tag )
{}

VertexArray::~VertexArray()
{
	Terminate();
}

void VertexArray::Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, ae::Vertex::Primitive primitive, ae::Vertex::Usage vertexUsage, ae::Vertex::Usage indexUsage )
{
	Terminate();
	m_buffer.Initialize( vertexSize, indexSize, maxVertexCount, maxIndexCount, primitive, vertexUsage, indexUsage );
}

void VertexArray::AddAttribute( const char *name, uint32_t componentCount, ae::Vertex::Type type, uint32_t offset )
{
	m_buffer.AddAttribute( name, componentCount, type, offset );
}

void VertexArray::Terminate()
{
	if( m_vertexReadable )
	{
		ae::Delete( (uint8_t*)m_vertexReadable );
		m_vertexReadable = nullptr;
	}
	if( m_indexReadable )
	{
		ae::Delete( (uint8_t*)m_indexReadable );
		m_indexReadable = nullptr;
	}
	
	m_buffer.Terminate();
	m_vertexCount = 0;
	m_indexCount = 0;
	m_vertexDirty = false;
	m_indexDirty = false;
}

void VertexArray::SetVertices( const void* vertices, uint32_t count )
{
	// State validation
	AE_ASSERT( m_buffer.GetVertexSize() );
	if( !m_vertexCount && !count )
	{
		return;
	}
	AE_ASSERT_MSG( count <= m_buffer.GetMaxVertexCount(), "Vertex limit exceeded #/#", count, m_buffer.GetMaxVertexCount() );
	
	// Set vertices
	if( count )
	{
		if( m_buffer.GetVertexUsage() == Vertex::Usage::Static )
		{
			AE_ASSERT_MSG( !m_vertexCount, "Cannot re-set vertices, buffer was created as static!" );
		}
		
		if( !m_vertexReadable )
		{
			// @TODO: Realloc or use array
			m_vertexReadable = ae::NewArray< uint8_t >( m_tag, m_buffer.GetMaxVertexCount() * m_buffer.GetVertexSize() );
		}
		memcpy( m_vertexReadable, vertices, count * m_buffer.GetVertexSize() );
	}
	m_vertexCount = count;
	m_vertexDirty = true;
}

void VertexArray::SetIndices( const void* indices, uint32_t count )
{
	// State validation
	AE_ASSERT( m_buffer.IsIndexed() );
	if( !m_indexCount && !count )
	{
		return;
	}
	AE_ASSERT_MSG( count <= m_buffer.GetMaxIndexCount(), "Index limit exceeded #/#", count, m_buffer.GetMaxIndexCount() );

	// Validate indices
	uint32_t maxVertexCount = m_buffer.GetMaxVertexCount();
	if( count && _AE_DEBUG_ )
	{
		int32_t badIndex = -1;
		
		switch( m_buffer.GetIndexSize() )
		{
			case 1:
			{
				uint8_t* indicesCheck = (uint8_t*)indices;
				for( uint32_t i = 0; i < count; i++ )
				{
					if( indicesCheck[ i ] >= maxVertexCount )
					{
						badIndex = indicesCheck[ i ];
						break;
					}
				}
				break;
			}
			case 2:
			{
				uint16_t* indicesCheck = (uint16_t*)indices;
				for( uint32_t i = 0; i < count; i++ )
				{
					if( indicesCheck[ i ] >= maxVertexCount )
					{
						badIndex = indicesCheck[ i ];
						break;
					}
				}
				break;
			}
			case 4:
			{
				uint32_t* indicesCheck = (uint32_t*)indices;
				for( uint32_t i = 0; i < count; i++ )
				{
					if( indicesCheck[ i ] >= maxVertexCount )
					{
						badIndex = indicesCheck[ i ];
						break;
					}
				}
				break;
			}
			default:
				AE_FAIL();
		}

		if( badIndex >= 0 )
		{
			AE_FAIL_MSG( "Out of range index detected #", badIndex );
		}
	}
	
	// Set indices
	if( count )
	{
		if( m_buffer.GetIndexUsage() == Vertex::Usage::Static )
		{
			AE_ASSERT_MSG( !m_indexCount, "Cannot re-set indices, buffer was created as static!" );
		}
		
		if( !m_indexReadable )
		{
			// @TODO: Realloc or use array
			m_indexReadable = ae::NewArray< uint8_t >( m_tag, m_buffer.GetMaxIndexCount() * m_buffer.GetIndexSize() );
		}
		memcpy( m_indexReadable, indices, count * m_buffer.GetIndexSize() );
	}
	m_indexCount = count;
	m_indexDirty = true;
}

void VertexArray::AppendVertices( const void* vertices, uint32_t count )
{
	// State validation
	AE_ASSERT( m_buffer.GetVertexSize() );
	if( m_buffer.GetVertexUsage() == Vertex::Usage::Static )
	{
		AE_ASSERT_MSG( !m_buffer.m_HasUploadedVertices(), "Cannot re-set vertices, buffer was created as static!" );
	}
	AE_ASSERT_MSG( m_vertexCount + count <= m_buffer.GetMaxVertexCount(), "Vertex limit exceeded #/#", m_vertexCount + count, m_buffer.GetMaxVertexCount() );
	
	if( !count )
	{
		return;
	}

	if( !m_vertexReadable )
	{
		// @TODO: Realloc or use array
		m_vertexReadable = ae::NewArray< uint8_t >( m_tag, m_buffer.GetMaxVertexCount() * m_buffer.GetVertexSize() );
	}
	
	// Append vertices
	memcpy( (uint8_t*)m_vertexReadable + ( m_vertexCount * m_buffer.GetVertexSize() ), vertices, count * m_buffer.GetVertexSize() );

	m_vertexCount += count;
	m_vertexDirty = true;
}

void VertexArray::AppendIndices( const void* indices, uint32_t count, uint32_t _indexOffset )
{
	// State validation
	AE_ASSERT( m_buffer.IsIndexed() );
	if( m_buffer.GetIndexUsage() == Vertex::Usage::Static )
	{
		AE_ASSERT_MSG( !m_buffer.m_HasUploadedIndices(), "Cannot re-set indices, buffer was created as static!" );
	}
	AE_ASSERT_MSG( m_indexCount + count <= m_buffer.GetMaxIndexCount(), "Index limit exceeded #/#", m_indexCount + count, m_buffer.GetMaxIndexCount() );
	
	if( !count )
	{
		return;
	}

	if( !m_indexReadable )
	{
		// @TODO: Realloc or use array
		m_indexReadable = ae::NewArray< uint8_t >( m_tag, m_buffer.GetMaxIndexCount() * m_buffer.GetIndexSize() );
	}
	
	// Append indices
	switch( m_buffer.GetIndexSize() )
	{
		case 1:
		{
			AE_ASSERT( _indexOffset <= ae::MaxValue< uint8_t >() );
			uint8_t indexOffset = (uint8_t)_indexOffset;
			uint8_t* target = (uint8_t*)m_indexReadable + m_indexCount;
			uint8_t* source = (uint8_t*)indices;
			for( uint32_t i = 0; i < count; i++ )
			{
				target[ i ] = indexOffset + source[ i ];
			}
			break;
		}
		case 2:
		{
			AE_ASSERT( _indexOffset <= ae::MaxValue< uint16_t >() );
			uint16_t indexOffset = (uint16_t)_indexOffset;
			uint16_t* target = (uint16_t*)m_indexReadable + m_indexCount;
			uint16_t* source = (uint16_t*)indices;
			for( uint32_t i = 0; i < count; i++ )
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
			for( uint32_t i = 0; i < count; i++ )
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

void VertexArray::ClearVertices()
{
	if( m_vertexCount && m_buffer.GetVertexUsage() == Vertex::Usage::Dynamic )
	{
		m_vertexCount = 0;
		m_vertexDirty = true;
	}
}

void VertexArray::ClearIndices()
{
	if( m_indexCount && m_buffer.GetIndexUsage() == Vertex::Usage::Dynamic )
	{
		m_indexCount = 0;
		m_indexDirty = true;
	}
}

template<>
const void* VertexArray::GetVertices() const
{
	return m_vertexReadable;
}

template<>
const void* VertexArray::GetIndices() const
{
	return m_indexReadable;
}

void VertexArray::Upload()
{
	if( m_vertexDirty )
	{
		m_buffer.UploadVertices( 0, m_vertexReadable, m_vertexCount );
		m_vertexDirty = false;
	}
	if( m_indexDirty )
	{
		m_buffer.UploadIndices( 0, m_indexReadable, m_indexCount );
		m_indexDirty = false;
	}
}

void VertexArray::Draw( const Shader* shader, const UniformList& uniforms ) const
{
	uint32_t primitiveSize = 0;
	switch( m_buffer.GetPrimitiveType() )
	{
		case Vertex::Primitive::Triangle: primitiveSize = 3; break;
		case Vertex::Primitive::Line: primitiveSize = 2; break;
		case Vertex::Primitive::Point: primitiveSize = 1; break;
		default: AE_FAIL();
	}
	Draw( shader, uniforms, 0, ( m_buffer.IsIndexed() ? m_indexCount : m_vertexCount ) / primitiveSize );
}

void VertexArray::Draw( const Shader* shader, const UniformList& uniforms, uint32_t primitiveStart, uint32_t primitiveCount ) const
{
	AE_ASSERT_MSG( m_buffer.GetVertexSize(), "Must call Initialize() before Draw()" );
	const_cast< VertexArray* >( this )->Upload(); // Make sure latest vertex data has been sent to GPU
	if( !m_vertexCount || ( m_buffer.IsIndexed() && !m_indexCount ) )
	{
		return;
	}
	m_buffer.Bind( shader, uniforms );
	m_buffer.Draw( primitiveStart, primitiveCount );
}

//------------------------------------------------------------------------------
// ae::InstanceData member functions
//------------------------------------------------------------------------------
InstanceData::~InstanceData()
{
	Terminate();
}

void InstanceData::Initialize( uint32_t dataStride, uint32_t maxInstanceCount, Vertex::Usage usage )
{
	Terminate();
	
	m_dataStride = dataStride;
	m_maxInstanceCount = maxInstanceCount;
	m_usage = usage;
}

void InstanceData::Terminate()
{
	if( m_buffer != ~0 )
	{
		glDeleteBuffers( 1, &m_buffer );
		m_buffer = ~0;
	}
	m_attributes.Clear();
	m_dataStride = 0;
	m_maxInstanceCount = 0;
}

void InstanceData::AddAttribute( const char *name, uint32_t componentCount, ae::Vertex::Type type, uint32_t offset )
{
	AE_ASSERT( m_buffer == ~0 );
	
	VertexBuffer::_Attribute* attribute = &m_attributes.Append( VertexBuffer::_Attribute() );
	
	size_t length = strlen( name );
	AE_ASSERT( length < _kMaxShaderAttributeNameLength );
	strcpy( attribute->name, name );
	attribute->componentCount = componentCount;
	attribute->type = VertexTypeToGL( type );
	attribute->offset = offset;
	attribute->normalized =
		type == Vertex::Type::NormalizedUInt8 ||
		type == Vertex::Type::NormalizedUInt16 ||
		type == Vertex::Type::NormalizedUInt32;
}

void InstanceData::UploadData( uint32_t startIdx, const void* data, uint32_t count )
{
	AE_ASSERT( m_dataStride );
	AE_ASSERT_MSG( ( startIdx + count ) <= m_maxInstanceCount, "Instance start: # count: # max: #", startIdx, count, m_maxInstanceCount );
	
	if( m_usage == Vertex::Usage::Static )
	{
		AE_ASSERT( count );
		AE_ASSERT( m_buffer == ~0 );
		AE_ASSERT( startIdx == 0 ); // @TODO: Remove this, shouldn't force data to start from zero

		glGenBuffers( 1, &m_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_buffer );
		glBufferData( GL_ARRAY_BUFFER, count * m_dataStride, data, GL_STATIC_DRAW );
		AE_CHECK_GL_ERROR();
		return;
	}
	if( m_usage == Vertex::Usage::Dynamic )
	{
		if( !count )
		{
			return;
		}
		
		if( m_buffer == ~0 )
		{
			glGenBuffers( 1, &m_buffer );
			glBindBuffer( GL_ARRAY_BUFFER, m_buffer );
			glBufferData( GL_ARRAY_BUFFER, m_maxInstanceCount * m_dataStride, nullptr, GL_DYNAMIC_DRAW );
		}
		else
		{
			glBindBuffer( GL_ARRAY_BUFFER, m_buffer );
		}
		
		glBufferSubData( GL_ARRAY_BUFFER, startIdx * m_dataStride, count * m_dataStride, data );
		AE_CHECK_GL_ERROR();
		return;
	}
	AE_FAIL();
}

const VertexBuffer::_Attribute* InstanceData::_GetAttribute( const char* n ) const
{
	int32_t idx = m_attributes.FindFn( [ n ]( const VertexBuffer::_Attribute& a )
	{
		return strcmp( a.name, n ) == 0;
	} );
	return ( idx >= 0 ) ? &m_attributes[ idx ] : nullptr;
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
	//        call Terminate() on initialize, and inherited Initialize()'s should
	//        always call Base::Initialize() before any other logic.
	Terminate();

	m_target = target;

	glGenTextures( 1, &m_texture );
	AE_ASSERT( m_texture );
}

void Texture::Terminate()
{
	if( m_texture )
	{
		glDeleteTextures( 1, &m_texture );
	}

	m_texture = 0;
	m_target = 0;
}

//------------------------------------------------------------------------------
// ae::Texture2D member functions
//------------------------------------------------------------------------------
void Texture2D::Initialize( const void* data, uint32_t width, uint32_t height, Format format, Type type, Filter filter, Wrap wrap, bool autoGenerateMipmaps )
{
	TextureParams params;
	params.data = data;
	params.width = width;
	params.height = height;
	params.format = format;
	params.type = type;
	params.filter = filter;
	params.wrap = wrap;
	params.autoGenerateMipmaps = autoGenerateMipmaps;
	Initialize( params );
}

void Texture2D::Initialize( const TextureParams& params )
{
	Texture::Initialize( GL_TEXTURE_2D );

#if _AE_EMSCRIPTEN_ || _AE_IOS_
	const auto GL_BGR = GL_RGB;
	const auto GL_BGRA = GL_RGBA;
#endif

	m_width = params.width;
	m_height = params.height;

	glBindTexture( GetTarget(), GetTexture() );

	const bool mipmapsEnabled = _AE_EMSCRIPTEN_ ? false : params.autoGenerateMipmaps;
	if( mipmapsEnabled )
	{
		glTexParameteri( GetTarget(), GL_TEXTURE_MIN_FILTER, ( params.filter == Filter::Nearest ) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GetTarget(), GL_TEXTURE_MAG_FILTER, ( params.filter == Filter::Nearest ) ? GL_NEAREST : GL_LINEAR );
	}
	else
	{
		glTexParameteri( GetTarget(), GL_TEXTURE_MIN_FILTER, ( params.filter == Filter::Nearest ) ? GL_NEAREST : GL_LINEAR );
		glTexParameteri( GetTarget(), GL_TEXTURE_MAG_FILTER, ( params.filter == Filter::Nearest ) ? GL_NEAREST : GL_LINEAR );
	}
	glTexParameteri( GetTarget(), GL_TEXTURE_WRAP_S, ( params.wrap == Wrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
	glTexParameteri( GetTarget(), GL_TEXTURE_WRAP_T, ( params.wrap == Wrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );

	// this is the type of data passed in, conflating with internal format type
	GLenum glType = 0;
	switch( params.type )
	{
		case Type::UInt8:
			glType = GL_UNSIGNED_BYTE;
			break;
		case Type::UInt16:
			glType = GL_UNSIGNED_SHORT;
			break;
		case Type::HalfFloat:
			glType = GL_HALF_FLOAT;
			break;
		case Type::Float:
			glType = GL_FLOAT;
			break;
		default:
			AE_FAIL_MSG( "Invalid texture type #", (int)params.type );
			return;
	}

	GLint glInternalFormat = 0;
	GLenum glFormat = 0;
	GLint unpackAlignment = 0;
	int32_t components = 0;
	switch( params.format )
	{
		// TODO: need D32F_S8 format
		case Format::Depth16:
			glInternalFormat = GL_DEPTH_COMPONENT16;
			glFormat = GL_DEPTH_COMPONENT;
			unpackAlignment = 1;
			m_hasAlpha = false;
			components = 1;
			break;
		case Format::Depth32F:
			glInternalFormat = GL_DEPTH_COMPONENT32F;
			glFormat = GL_DEPTH_COMPONENT;
			unpackAlignment = 1;
			m_hasAlpha = false;
			components = 1;
			break;
		case Format::R8:
		case Format::R16_UNORM:
		case Format::R16F:
		case Format::R32F:
			switch( params.format )
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
			components = 1;
			break;
			
#if _AE_OSX_
		// RedGreen, TODO: extend to other ES but WebGL1 left those constants out IIRC
		case Format::RG8:
		case Format::RG16F:
		case Format::RG32F:
			switch( params.format )
			{
				case Format::RG8: glInternalFormat = GL_RG8; break;
				case Format::RG16F: glInternalFormat = GL_RG16F; break;
				case Format::RG32F: glInternalFormat = GL_RG32F; break;
				default: assert(false);
			}
					
			glFormat = GL_RG; // @TODO: Handle bgra flag
			unpackAlignment = 1;
			m_hasAlpha = false;
			components = 2;
			break;
#endif
		case Format::RGB8:
		case Format::RGB16F:
		case Format::RGB32F:
			switch( params.format )
			{
				case Format::RGB8: glInternalFormat = GL_RGB8; break;
				case Format::RGB16F: glInternalFormat = GL_RGB16F; break;
				case Format::RGB32F: glInternalFormat = GL_RGB32F; break;
				default: assert(false);
			}
			glFormat = params.bgrData ? GL_BGR : GL_RGB;
			unpackAlignment = 1;
			m_hasAlpha = false;
			components = 3;
			break;

		case Format::RGBA8:
		case Format::RGBA16F:
		case Format::RGBA32F:
			switch( params.format )
			{
				case Format::RGBA8: glInternalFormat = GL_RGBA8; break;
				case Format::RGBA16F: glInternalFormat = GL_RGBA16F; break;
				case Format::RGBA32F: glInternalFormat = GL_RGBA32F; break;
				default: assert(false);
			}
			glFormat = params.bgrData ?  GL_BGRA : GL_RGBA;
			unpackAlignment = 1;
			m_hasAlpha = true;
			components = 4;
			break;
			
			// TODO: fix these constants, but they differ on ES2/3 and GL
			// WebGL1 they require loading an extension (if present) to get at the constants.
		case Format::RGB8_SRGB:
			// ignore type
			glInternalFormat = GL_SRGB8;
			glFormat = params.bgrData ? GL_BGR : GL_RGB;
			unpackAlignment = 1;
			m_hasAlpha = false;
			components = 3;
			break;
		case Format::RGBA8_SRGB:
			// ignore type
			glInternalFormat = GL_SRGB8_ALPHA8;
			glFormat = params.bgrData ? GL_BGRA : GL_RGBA;
			unpackAlignment = 1;
			m_hasAlpha = false;
			components = 4;
			break;
		default:
			AE_FAIL_MSG( "Invalid texture format #", (int)params.format );
			return;
	}
	AE_ASSERT( components );

	if( params.data )
	{
		glPixelStorei( GL_UNPACK_ALIGNMENT, unpackAlignment );
	}

	// count the mip levels
	int numberOfMipmaps = 1;
	if( mipmapsEnabled )
	{
		int w = params.width;
		int h = params.height;
		while( w > 1 || h > 1 )
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
	glTexStorage2D( GetTarget(), numberOfMipmaps, glInternalFormat, params.width, params.height );
#else
	int w = params.width;
	int h = params.height;
	for( int i = 0; i < numberOfMipmaps; ++i )
	{
		glTexImage2D( GetTarget(), i, glInternalFormat, w, h, 0, glFormat, glType, NULL );
		w = (w+1) / 2;
		h = (h+1) / 2;
	}
#endif

	const void* data = params.data;
	void* tempData = nullptr;
#if _AE_EMSCRIPTEN_
	if( params.bgrData && components >= 3 )
	{
		const uint32_t totalComponents = params.width * params.height * components;
#define _AE_BGR_TO_RGB_COPY( _type )\
		tempData = ae::Allocate( AE_ALLOC_TAG_RENDER, totalComponents * sizeof(_type), 16 );\
		data = tempData;\
		for( uint32_t i = 0; i < totalComponents; i += components )\
		{\
			((_type*)data)[ i + 0 ] = ((_type*)params.data)[ i + 2 ];\
			((_type*)data)[ i + 1 ] = ((_type*)params.data)[ i + 1 ];\
			((_type*)data)[ i + 2 ] = ((_type*)params.data)[ i + 0 ];\
			if( components == 4 ) { ((_type*)data)[ i + 3 ] = ((_type*)params.data)[ i + 3 ]; }\
		}
		switch( params.type )
		{
			case Type::UInt8: _AE_BGR_TO_RGB_COPY( uint8_t ); break;
			case Type::UInt16: _AE_BGR_TO_RGB_COPY( uint16_t ); break;
			case Type::HalfFloat: _AE_BGR_TO_RGB_COPY( uint16_t ); break; // Use uint16_t for data copy
			case Type::Float: _AE_BGR_TO_RGB_COPY( float ); break;
			default: AE_FAIL();
		}
#undef _AE_BGR_TO_RGB_COPY
	}
#endif
	
	if( data )
	{
		// upload the first mipmap
		glTexSubImage2D( GetTarget(), 0, 0, 0, params.width, params.height, glFormat, glType, data );
		if( tempData )
		{
			ae::Free( tempData );
		}
#if !_AE_EMSCRIPTEN_
		// autogen only works for uncompressed textures
		// Also need to know if format is filterable on platform, or this will fail (f.e. R32F)
		if( mipmapsEnabled && numberOfMipmaps > 1 )
		{
			glGenerateMipmap( GetTarget() );
		}
#endif
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

	if( width * height == 0 )
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
}

void RenderTarget::Terminate()
{
	if( m_fbo )
	{
		// On Emscripten it seems to matter that the framebuffer is deleted
		// first so it's not referencing its textures.
		glDeleteFramebuffers( 1, (uint32_t*)&m_fbo );
		m_fbo = 0;
	}
	
	for( uint32_t i = 0; i < m_targets.Length(); i++ )
	{
		m_targets[ i ]->Terminate();
		ae::Delete( m_targets[ i ] );
	}
	m_targets.Clear();
	m_depth.Terminate();

	m_width = 0;
	m_height = 0;
}

void RenderTarget::AddTexture( Texture::Filter filter, Texture::Wrap wrap )
{
	AE_ASSERT( m_targets.Length() < _kMaxFrameBufferAttachments );
	if( m_width * m_height == 0 )
	{
		return;
	}

#if _AE_EMSCRIPTEN_
	Texture::Format format = Texture::Format::RGBA8;
	Texture::Type type = Texture::Type::UInt8;
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
	if( m_width * m_height == 0 )
	{
		return;
	}

#if _AE_EMSCRIPTEN_
	Texture::Format format = Texture::Format::Depth16;
	Texture::Type type = Texture::Type::UInt16;
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
	AE_ASSERT_MSG( GetWidth() && GetHeight(), "ae::RenderTarget is not initialized" );
	AE_ASSERT_MSG( m_targets.Length(), "ae::RenderTarget is not complete. Call AddTexture() before Activate()." );
	AE_CHECK_GL_ERROR();
	
	CheckFramebufferComplete( m_fbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_fbo );
	
	GLenum buffers[ _kMaxFrameBufferAttachments ];
	for( uint32_t i = 0 ; i < countof(buffers); i++ )
	{
		buffers[ i ] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers( m_targets.Length(), buffers );

	glViewport( 0, 0, GetWidth(), GetHeight() );
	AE_CHECK_GL_ERROR();
}

void RenderTarget::Clear( Color color )
{
	Activate();

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
	_Globals* globals = ae::_Globals::Get();
	AE_ASSERT( globals->graphicsDevice );

	glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
	AE_CHECK_GL_ERROR();
	
	globals->graphicsDevice->m_renderQuad.Bind( shader, uniforms );
	globals->graphicsDevice->m_renderQuad.Draw();
}

void RenderTarget::Render2D( uint32_t textureIndex, Rect ndc, float z )
{
	_Globals* globals = ae::_Globals::Get();
	AE_ASSERT( globals->graphicsDevice );

	glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
	AE_CHECK_GL_ERROR();

	UniformList uniforms;
	uniforms.Set( "u_localToNdc", RenderTarget::GetQuadToNDCTransform( ndc, z ) );
	uniforms.Set( "u_tex", GetTexture( textureIndex ) );
	Shader* shader = globals->graphicsDevice->m_rgbToSrgb
		? &globals->graphicsDevice->m_renderShaderSRGB
		: &globals->graphicsDevice->m_renderShaderRGB;
	globals->graphicsDevice->m_renderQuad.Bind( shader, uniforms );
	globals->graphicsDevice->m_renderQuad.Draw();
}

const Texture2D* RenderTarget::GetTexture( uint32_t index ) const
{
	return m_targets[ index ];
}

uint32_t RenderTarget::GetTextureCount() const
{
	return m_targets.Length();
}

const Texture2D* RenderTarget::GetDepth() const
{
	return m_depth.GetTexture() ? &m_depth : nullptr;
}

float RenderTarget::GetAspectRatio() const
{
	if( m_width * m_height == 0 )
	{
		return 0.0f;
	}
	else
	{
		return m_width / (float)m_height;
	}
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
	if( canvasAspect >= targetAspect )
	{
		float height = targetAspect / canvasAspect;
		return ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f ), ae::Vec2( 2.0f, height * 2.0f ) );
	}
	else
	{
		float width = canvasAspect / targetAspect;
		return ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f ), ae::Vec2( width * 2.0f, 2.0f ) );
	}
}

Matrix4 RenderTarget::GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const
{
	Matrix4 canvasToNdc = Matrix4::Translation( Vec3( -1.0f, -1.0f, 0.0f ) ) * Matrix4::Scaling( Vec3( 2.0f / GetWidth(), 2.0f / GetHeight(), 1.0f ) );
	return ( worldToNdc.GetInverse() * canvasToNdc * otherTargetToLocal );
}

Matrix4 RenderTarget::GetQuadToNDCTransform( Rect ndc, float z )
{
	ae::Vec2 ndcPos = ndc.GetMin();
	ae::Vec2 ndcSize = ndc.GetSize();
	Matrix4 localToNdc = Matrix4::Translation( Vec3( ndcPos.x, ndcPos.y, z ) );
	localToNdc *= Matrix4::Scaling( Vec3( ndcSize.x, ndcSize.y, 1.0f ) );
	localToNdc *= Matrix4::Translation( Vec3( 0.5f, 0.5f, 0.0f ) );
	return localToNdc;
}

//------------------------------------------------------------------------------
// ae::GraphicsDevice member functions
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

	_Globals* globals = ae::_Globals::Get();
	AE_ASSERT_MSG( !globals->graphicsDevice, "Only one instance of ae::GraphicsDevice is supported" );
	globals->graphicsDevice = this;

	AE_ASSERT( window );
	AE_ASSERT( window->GetWidth() && window->GetHeight() );
	m_window = window;
	AE_ASSERT( !window->graphicsDevice );
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
	if( !wglMakeCurrent( hdc, hglrc ) )
	{
		AE_FAIL_MSG( "Failed to make OpenGL Rendering Context current" );
	}
	m_context = hglrc;
#elif _AE_OSX_ 
	m_context = ((NSOpenGLView*)((NSWindow*)window->window).contentView).openGLContext;
#elif _AE_EMSCRIPTEN_
	EmscriptenWebGLContextAttributes attrs;
	emscripten_webgl_init_context_attributes( &attrs );
	attrs.alpha = 0;
	attrs.majorVersion = ae::GLMajorVersion;
	attrs.minorVersion = ae::GLMinorVersion;
	m_context = emscripten_webgl_create_context( "canvas", &attrs );
	AE_ASSERT( m_context > 0 );
	EMSCRIPTEN_RESULT activateResult = emscripten_webgl_make_context_current( m_context );
	AE_ASSERT( activateResult == EMSCRIPTEN_RESULT_SUCCESS );
#endif
	
	AE_CHECK_GL_ERROR();

#if _AE_WINDOWS_
	LOAD_OPENGL_FN( wglSwapIntervalEXT );
	LOAD_OPENGL_FN( wglGetSwapIntervalEXT );
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
	LOAD_OPENGL_FN( glVertexAttribDivisor );
	LOAD_OPENGL_FN( glDrawElementsInstanced );
	LOAD_OPENGL_FN( glDrawArraysInstanced );
	// Debug functions
	LOAD_OPENGL_FN( glDebugMessageCallback );
	AE_CHECK_GL_ERROR();
#endif

#if AE_GL_DEBUG_MODE
	glDebugMessageCallback( ae::OpenGLDebugCallback, nullptr );
#endif

	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFbo );
	AE_CHECK_GL_ERROR();
	
	// Initialize shared RenderTarget resources
	struct Vertex
	{
		Vec3 pos;
		Vec2 uv;
	};
	Vertex quadVerts[] =
	{
		{ _kQuadVertPos[ 0 ], _kQuadVertUvs[ 0 ] },
		{ _kQuadVertPos[ 1 ], _kQuadVertUvs[ 1 ] },
		{ _kQuadVertPos[ 2 ], _kQuadVertUvs[ 2 ] },
		{ _kQuadVertPos[ 3 ], _kQuadVertUvs[ 3 ] },
	};
	AE_STATIC_ASSERT( countof( quadVerts ) == _kQuadVertCount );
	m_renderQuad.Initialize( sizeof( Vertex ), sizeof( _kQuadIndex ), _kQuadVertCount, _kQuadIndexCount, ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	m_renderQuad.AddAttribute( "a_position", 3, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
	m_renderQuad.AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof( Vertex, uv ) );
	m_renderQuad.UploadVertices( 0, quadVerts, _kQuadVertCount );
	m_renderQuad.UploadIndices( 0, _kQuadIndices, _kQuadIndexCount );
	AE_CHECK_GL_ERROR();

	// @NOTE: GL_FRAMEBUFFER_SRGB is not completely reliable on every platform (web, wide color
	// display targets, etc), mostly because of limited control over the backbuffer format.
	// On web its not possible to specify the backbuffer format, but browsers typically expect SRGB anyway.
	// On OpenGLES GL_FRAMEBUFFER_SRGB is always enabled.
	// Because of all of this it's easiest to convert to SRGB manually on non-OpenGLES platforms.
	const char* vertexStr = R"(
		AE_UNIFORM_HIGHP mat4 u_localToNdc;
		AE_IN_HIGHP vec3 a_position;
		AE_IN_HIGHP vec2 a_uv;
		AE_OUT_HIGHP vec2 v_uv;
		void main()
		{
			v_uv = a_uv;
			gl_Position = u_localToNdc * vec4( a_position, 1.0 );
		})";
	const char* fragStr = R"(
		uniform sampler2D u_tex;
		AE_IN_HIGHP vec2 v_uv;
		void main()
		{
			vec4 color = AE_TEXTURE2D( u_tex, v_uv );
			#ifdef AE_SRGB_TARGET
				bvec3 cutoff = lessThan(color.rgb, vec3(0.0031308));
				vec3 higher = vec3(1.055) * pow(color.rgb, vec3(1.0/2.4)) - vec3(0.055);
				vec3 lower = color.rgb * vec3(12.92);
				AE_COLOR.rgb = mix(higher, lower, cutoff);
				// Always full opacity when converting to srgb. Blending does not work without GL_FRAMEBUFFER_SRGB.
				AE_COLOR.a = 1.0;
			#else
				AE_COLOR = color;
			#endif
		})";
	const char* srgbDefine = "#define AE_SRGB_TARGET";
	m_renderShaderRGB.Initialize( vertexStr, fragStr, nullptr, 0 );
	m_renderShaderRGB.SetBlending( true );  // This is required on some implementations of OpenGL for GL_FRAMEBUFFER_SRGB to work
	m_renderShaderSRGB.Initialize( vertexStr, fragStr, &srgbDefine, 1 ); // Do not blend when manually converting to srgb without GL_FRAMEBUFFER_SRGB
	AE_CHECK_GL_ERROR();
	
	Activate(); // Init primary render target
	AE_ASSERT( GetWidth() && GetHeight() );
	AE_ASSERT( m_context );
}

void GraphicsDevice::SetVsyncEnbled( bool enabled )
{
#if _AE_WINDOWS_
	wglSwapIntervalEXT( enabled ? 1 : 0 );
#endif
}

bool GraphicsDevice::GetVsyncEnabled() const
{
#if _AE_WINDOWS_
	return wglGetSwapIntervalEXT() != 0;
#endif
	return false;
}

void GraphicsDevice::Terminate()
{
	if( m_context )
	{
		_Globals* globals = ae::_Globals::Get();
		AE_ASSERT( globals->graphicsDevice == this );
		globals->graphicsDevice = nullptr;

		m_renderShaderSRGB.Terminate();
		m_renderShaderRGB.Terminate();
		m_renderQuad.Terminate();

		m_context = 0;
	}
}

void GraphicsDevice::Activate()
{
	AE_ASSERT( m_window );
	AE_ASSERT( m_context );

	const float scaleFactor = m_window->GetScaleFactor();
	const int32_t contentWidth = m_window->GetWidth() * scaleFactor;
	const int32_t contentHeight = m_window->GetHeight() * scaleFactor;
	if( contentWidth != m_canvas.GetWidth() || contentHeight != m_canvas.GetHeight() )
	{
#if _AE_EMSCRIPTEN_
		const double currentTime = ae::GetTime();
		if( m_resizeWidthPrev != contentWidth || m_resizeHeightPrev != contentHeight )
		{
			m_resizeWidthPrev = contentWidth;
			m_resizeHeightPrev = contentHeight;
			m_lastResize = currentTime;
		}
		// Quarter second delay before resizing so it doesn't change every frame
		if( m_lastResize + 0.25 < currentTime || ( m_canvas.GetWidth() * m_canvas.GetHeight() == 0 ) )
		{
			// @NOTE: The window size is the 'real' size of the canvas dom, which
			// is determined by the web page. This function sets the emscripten
			// managed backbuffer size.
			if( m_window->GetLoggingEnabled() ) { AE_INFO( "resize #x#", contentWidth, contentHeight ); }
			emscripten_set_canvas_element_size( "canvas", contentWidth, contentHeight );
			m_HandleResize( contentWidth, contentHeight );
		}
#else
		m_HandleResize( contentWidth, contentHeight );
#endif
	}

	if( m_canvas.GetWidth() * m_canvas.GetHeight() )
	{
		m_canvas.Activate();
	}
}

void GraphicsDevice::Clear( Color color )
{
	if( m_canvas.GetWidth() * m_canvas.GetHeight() == 0 )
	{
		return;
	}
	Activate();
	m_canvas.Clear( color );
}

void GraphicsDevice::Present()
{
	if( m_canvas.GetWidth() * m_canvas.GetHeight() == 0 )
	{
		return;
	}

	AE_ASSERT( m_context );
	AE_CHECK_GL_ERROR();

#if _AE_EMSCRIPTEN_
	EMSCRIPTEN_RESULT activateResult = emscripten_webgl_make_context_current( m_context );
	AE_ASSERT( activateResult == EMSCRIPTEN_RESULT_SUCCESS );
#endif
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_defaultFbo );
	glViewport( 0, 0, m_canvas.GetWidth(), m_canvas.GetHeight() );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClearDepth( 1.0f );

	glDepthMask( GL_TRUE );

	glDisable( GL_DEPTH_TEST );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	AE_CHECK_GL_ERROR();
	
	// @NOTE: Conversion to srgb is only needed for the backbuffer. The rest of the pipeline should be implemented as linear.
#if _AE_IOS_
	// SRGB conversion is automatic on ios/OpenGLES because GL_FRAMEBUFFER_SRGB is always on
	m_rgbToSrgb = false;
#else
	// Currently all platforms expect the backbuffer contents to be in sRGB space
	m_rgbToSrgb = true;
#endif
	m_canvas.Render2D( 0, ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f ), ae::Vec2( 2.0f ) ), 0.5f );
	m_rgbToSrgb = false;
	
	AE_CHECK_GL_ERROR();

	// Swap Buffers
#if _AE_OSX_
	[(NSOpenGLContext*)m_context flushBuffer];
#elif _AE_WINDOWS_
	AE_ASSERT( m_window );
	HWND hWnd = (HWND)m_window->window;
	AE_ASSERT( hWnd );
	HDC hdc = GetDC( hWnd );
	SwapBuffers( hdc );
#endif
}

float GraphicsDevice::GetAspectRatio() const
{
	return m_canvas.GetAspectRatio();
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
	// @TODO: Allow user to pass in a canvas scale factor / aspect ratio parameter
	m_canvas.Initialize( width, height );
	m_canvas.AddTexture( Texture::Filter::Nearest, Texture::Wrap::Clamp );
	m_canvas.AddDepth( Texture::Filter::Nearest, Texture::Wrap::Clamp );
	
	// Force refresh uniforms for new canvas
	s_shaderHash = ae::Hash32();
	s_uniformHash = ae::Hash32();
}

//------------------------------------------------------------------------------
// ae::TextRender member functions
//------------------------------------------------------------------------------
TextRender::TextRender( const ae::Tag& tag ) :
	m_tag( tag )
{}

TextRender::~TextRender()
{
	Terminate();
}

void TextRender::Initialize( uint32_t maxStringCount, uint32_t maxGlyphCount, const ae::Texture2D* texture, uint32_t fontSize, float spacing )
{
	Terminate();
	
	m_texture = texture;
	m_fontSize = fontSize;
	m_spacing = spacing;
	m_allocatedStrings = 0;
	m_allocatedChars = 0;
	m_maxRectCount = maxStringCount;
	m_maxGlyphCount = maxGlyphCount;

	m_vertexData.Initialize( sizeof( Vertex ), sizeof( uint16_t ), m_maxGlyphCount * _kQuadVertCount, m_maxGlyphCount * _kQuadIndexCount, ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Dynamic, ae::Vertex::Usage::Dynamic );
	m_vertexData.AddAttribute( "a_position", 3, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
	m_vertexData.AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof( Vertex, uv ) );
	m_vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );

	// Load shader
	const char* vertexStr = R"(
		AE_UNIFORM_HIGHP mat4 u_uiToScreen;
		AE_IN_HIGHP vec3 a_position;
		AE_IN_HIGHP vec2 a_uv;
		AE_IN_HIGHP vec4 a_color;
		AE_OUT_HIGHP vec2 v_uv;
		AE_OUT_HIGHP vec4 v_color;
		void main()
		{
			v_uv = a_uv;
			v_color = a_color;
			gl_Position = u_uiToScreen * vec4( a_position, 1.0 );
		})";
	const char* fragStr = R"(
		uniform sampler2D u_tex;
		AE_IN_HIGHP vec2 v_uv;
		AE_IN_HIGHP vec4 v_color;
		void main()
		{
			if( AE_TEXTURE2D( u_tex, v_uv ).r < 0.5 ) { discard; };
			AE_COLOR = v_color;
		})";
	m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );
	m_shader.SetBlending( true );
	
	m_strings = ae::NewArray< TextRect >( m_tag, m_maxRectCount );
	m_stringData = ae::NewArray< char >( m_tag, m_maxGlyphCount );
}

void TextRender::Terminate()
{
	ae::Delete( m_stringData );
	ae::Delete( m_strings );

	m_allocatedChars = 0;
	m_allocatedStrings = 0;
	m_stringData = nullptr;
	m_strings = nullptr;
	m_shader.Terminate();
	m_vertexData.Terminate();

	m_fontSize = 0;
	m_spacing = 0.0f;
	m_texture = nullptr;
	m_maxGlyphCount = 0;
	m_maxRectCount = 0;
}

void TextRender::Render( const ae::Matrix4& uiToScreen )
{
	uint32_t vertCount = 0;
	uint32_t indexCount = 0;
	ae::Scratch< Vertex > verts( m_vertexData.GetMaxVertexCount() );
	ae::Scratch< uint16_t > indices( m_vertexData.GetMaxIndexCount() );

	uint32_t charCount = 0;
	for( uint32_t i = 0; i < m_allocatedStrings; i++ )
	{
		const TextRect& rect = m_strings[ i ];
		ae::Vec3 pos = rect.pos;
		pos.y -= rect.size.y;

		const char* start = rect.str;
		const char* str = start;
		while( str[ 0 ] )
		{
			if( !isspace( str[ 0 ] ) && charCount < m_maxGlyphCount )
			{
				int32_t index = str[ 0 ];
				uint32_t columns = m_texture->GetWidth() / m_fontSize;
				ae::Vec2 offset( index % columns, columns - index / columns - 1 ); // @HACK: Assume same number of columns and rows

				for( uint32_t j = 0; j < _kQuadIndexCount; j++ )
				{
					indices.GetSafe( indexCount ) = (uint16_t)( _kQuadIndices[ j ] + vertCount );
					indexCount++;
				}

				AE_ASSERT( vertCount + _kQuadVertCount <= verts.Length() );
				// Bottom Left
				verts[ vertCount ].pos = pos;
				verts[ vertCount ].uv = ( _kQuadVertUvs[ 0 ] + offset ) / columns;
				verts[ vertCount ].color = rect.color.GetLinearRGBA();
				vertCount++;
				// Bottom Right
				verts[ vertCount ].pos = pos + ae::Vec3( rect.size.x, 0.0f, 0.0f );
				verts[ vertCount ].uv = ( _kQuadVertUvs[ 1 ] + offset ) / columns;
				verts[ vertCount ].color = rect.color.GetLinearRGBA();
				vertCount++;
				// Top Right
				verts[ vertCount ].pos = pos + ae::Vec3( rect.size.x, rect.size.y, 0.0f );
				verts[ vertCount ].uv = ( _kQuadVertUvs[ 2 ] + offset ) / columns;
				verts[ vertCount ].color = rect.color.GetLinearRGBA();
				vertCount++;
				// Top Left
				verts[ vertCount ].pos = pos + ae::Vec3( 0.0f, rect.size.y, 0.0f );
				verts[ vertCount ].uv = ( _kQuadVertUvs[ 3 ] + offset ) / columns;
				verts[ vertCount ].color = rect.color.GetLinearRGBA();
				vertCount++;

				charCount++;
			}

			if( str[ 0 ] == '\n' || str[ 0 ] == '\r' )
			{
				pos.x = rect.pos.x;
				pos.y -= rect.size.y;
			}
			else
			{
				pos.x += rect.size.x * m_spacing;
			}
			str++;
		}
	}

	m_vertexData.UploadVertices( 0, verts.Data(), vertCount );
	m_vertexData.UploadIndices( 0, indices.Data(), indexCount );

	ae::UniformList uniforms;
	uniforms.Set( "u_uiToScreen", uiToScreen );
	uniforms.Set( "u_tex", m_texture );
	m_vertexData.Bind( &m_shader, uniforms );
	m_vertexData.Draw( 0, indexCount / 3 );

	m_allocatedStrings = 0;
	m_allocatedChars = 0;
}

void TextRender::Add( ae::Vec3 pos, ae::Vec2 size, const char* str, ae::Color color, uint32_t lineLength, uint32_t charLimit )
{
	if( m_allocatedStrings >= m_maxRectCount )
	{
		return;
	}

	uint32_t remainingChars = (uint32_t)ae::Max( 0, (int32_t)m_maxGlyphCount - (int32_t)m_allocatedChars );
	charLimit = charLimit ? ae::Min( charLimit, remainingChars ) : remainingChars;
	if( !charLimit )
	{
		return;
	}

	uint32_t len = 0;
	char* rectStr = m_stringData + m_allocatedChars;
	if( m_ParseText( str, lineLength, charLimit, &rectStr, &len ) )
	{
		m_allocatedChars += len + 1; // Include null terminator
		TextRect* rect = &m_strings[ m_allocatedStrings ];
		m_allocatedStrings++;
		rect->pos = pos;
		rect->str = rectStr;
		rect->size = size;
		rect->color = color;
	}
}

uint32_t TextRender::GetLineCount( const char* str, uint32_t lineLength, uint32_t charLimit ) const
{
	return m_ParseText( str, lineLength, charLimit, nullptr, nullptr );
}

uint32_t TextRender::m_ParseText( const char* str, uint32_t lineLength, uint32_t charLimit, char** _outStr, uint32_t* lenOut ) const
{
	const char* strDataLast = m_stringData + m_maxGlyphCount - 1;
	char* outStr = nullptr;
	if( _outStr && *_outStr )
	{
		outStr = *_outStr;
		if( outStr == strDataLast )
		{
			return 0;
		}
		outStr[ 0 ] = '\0';
	}

	uint32_t lineCount = 1;
	const char* start = str;
	uint32_t lineChars = 0;
	while( str[ 0 ] )
	{
		// Truncate displayed string based on param
		if( charLimit && (uint32_t)( str - start ) >= charLimit )
		{
			break;
		}
		if( outStr == strDataLast )
		{
			break;
		}

		bool isNewlineChar = ( str[ 0 ] == '\n' || str[ 0 ] == '\r' );

		if( lineLength && !isNewlineChar && isspace( str[ 0 ] ) )
		{
			// Prevent words from being split across lines
			uint32_t wordRemainder = 1;
			while( str[ wordRemainder ] && !isspace( str[ wordRemainder ] ) )
			{
				wordRemainder++;
			}

			// If lineChars is 0 then the current word is longer than lineLength
			if( lineChars && lineChars + wordRemainder > lineLength )
			{
				if( outStr )
				{
					outStr[ 0 ] = '\n';
					outStr[ 1 ] = '\0';
					outStr++;
				}
				lineCount++;
				lineChars = 0;
				continue; // Only append one char per loop
			}
		}

		// Skip non-newline whitespace at the beginning of a line
		if( lineChars || isNewlineChar || !isspace( str[ 0 ] ) )
		{
			if( outStr )
			{
				outStr[ 0 ] = str[ 0 ];
				outStr[ 1 ] = '\0';
				outStr++;
			}

			lineChars = isNewlineChar ? 0 : lineChars + 1;
		}
		if( isNewlineChar )
		{
			lineCount++;
		}

		str++;
	}
	if( outStr && lenOut )
	{
		*lenOut = (uint32_t)( outStr - *_outStr );
	}

	return lineCount;
}

//------------------------------------------------------------------------------
// ae::DebugLines member functions
//------------------------------------------------------------------------------
DebugLines::DebugLines( const ae::Tag& tag ) :
	m_vertexArray( tag )
{}

DebugLines::~DebugLines()
{
	Terminate();
}

void DebugLines::Initialize( uint32_t maxVerts )
{
	m_vertexArray.Initialize( sizeof(DebugVertex), 0, maxVerts, 0, Vertex::Primitive::Line, Vertex::Usage::Dynamic, Vertex::Usage::Static );
	m_vertexArray.AddAttribute( "a_position", 4, Vertex::Type::Float, offsetof(DebugVertex, pos) );
	m_vertexArray.AddAttribute( "a_color", 4, Vertex::Type::Float, offsetof(DebugVertex, color) );

	// Load shader
	const char* vertexStr = R"(
		AE_UNIFORM_HIGHP mat4 u_worldToNdc;
		AE_UNIFORM float u_saturation;
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec4 a_color;
		AE_OUT_HIGHP vec4 v_color;
		void main()
		{
			float bw = ( min( a_color.r, min( a_color.g, a_color.b ) ) + max( a_color.r, max( a_color.g, a_color.b ) ) ) * 0.5;
			v_color = vec4( mix( vec3(bw), a_color.rgb, u_saturation ), a_color.a );
			gl_Position = u_worldToNdc * a_position;
		})";
	const char* fragStr = R"(
		AE_IN_HIGHP vec4 v_color;
		void main()
		{
			AE_COLOR = v_color;
		})";
	m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );
	m_shader.SetBlending( true );
	m_shader.SetDepthTest( true );
}

void DebugLines::Terminate()
{
	m_shader.Terminate();
	m_vertexArray.Terminate();
	m_xray = true;
}

void DebugLines::Render( const Matrix4& worldToNdc )
{
	m_vertexArray.Upload();
	
	UniformList uniforms;
	uniforms.Set( "u_worldToNdc", worldToNdc );

	if( m_xray )
	{
		m_shader.SetDepthTest( false );
		m_shader.SetDepthWrite( false );
		uniforms.Set( "u_saturation", 0.1f );
		m_vertexArray.Draw( &m_shader, uniforms );
	}

	m_shader.SetDepthTest( true );
	m_shader.SetDepthWrite( true );
	uniforms.Set( "u_saturation", 1.0f );
	m_vertexArray.Draw( &m_shader, uniforms );
	
	m_vertexArray.ClearVertices();
}

void DebugLines::Clear()
{
	m_vertexArray.ClearVertices();
}

uint32_t DebugLines::AddLine( Vec3 p0, Vec3 p1, Color color )
{
	if( m_vertexArray.GetVertexCount() + 2 > m_vertexArray.GetMaxVertexCount() )
	{
		return 0;
	}
	DebugVertex verts[] =
	{
		{ Vec4( p0, 1.0f ), color },
		{ Vec4( p1, 1.0f ), color }
	};
	m_vertexArray.AppendVertices( verts, countof( verts ) );
	return countof( verts );
}

uint32_t DebugLines::AddDistanceCheck( Vec3 p0, Vec3 p1, float distance, ae::Color successColor, ae::Color failColor )
{
	if( m_vertexArray.GetVertexCount() + 2 > m_vertexArray.GetMaxVertexCount() )
	{
		return 0;
	}
	ae::Color color = ( ( p1 - p0 ).Length() <= distance ) ? successColor : failColor;
	DebugVertex verts[] =
	{
		{ Vec4( p0, 1.0f ), color },
		{ Vec4( p1, 1.0f ), color }
	};
	m_vertexArray.AppendVertices( verts, countof( verts ) );
	return countof( verts );
}

uint32_t DebugLines::AddRect( ae::Vec3 pos, ae::Vec3 up, ae::Vec3 normal, ae::Vec2 halfSize, float cornerRadius, uint32_t cornerPointCount, ae::Color color )
{
	if( m_vertexArray.GetVertexCount() + 8 > m_vertexArray.GetMaxVertexCount()
		|| up.LengthSquared() < 0.001f
		|| normal.LengthSquared() < 0.001f )
	{
		return 0;
	}
	up.Normalize();
	normal.Normalize();
	if( normal.Dot( up ) > 0.999f )
	{
		return 0;
	}
	cornerRadius = ae::Min( cornerRadius, halfSize.x, halfSize.y );
	const ae::Vec2 innerSize = halfSize - ae::Vec2( cornerRadius );
	const ae::Quaternion r( normal, up );

	if( cornerRadius > 0.0f )
	{
		const ae::Vec2 is[] =
		{
			{ innerSize.x, innerSize.y },
			{ -innerSize.x, innerSize.y },
			{ -innerSize.x, -innerSize.y },
			{ innerSize.x, -innerSize.y }
		};
		const float angles[] = { 0.0f, ae::HALF_PI, ae::PI, ae::PI * 1.5f };
		for( uint32_t i = 0; i < 4; i++ )
		{
			for( uint32_t j = 0; j < cornerPointCount + 1; j++ )
			{
				const float a0 = angles[ i ] + ( j * ae::HALF_PI ) / ( cornerPointCount + 1 );
				const float a1 = angles[ i ] + ( ( j + 1 ) * ae::HALF_PI ) / ( cornerPointCount + 1 );
				const ae::Vec3 p0 = r.Rotate( ae::Vec3( is[ i ].x + ae::Cos( a0 ) * cornerRadius, 0.0f, is[ i ].y + ae::Sin( a0 ) * cornerRadius ) );
				const ae::Vec3 p1 = r.Rotate( ae::Vec3( is[ i ].x + ae::Cos( a1 ) * cornerRadius, 0.0f, is[ i ].y + ae::Sin( a1 ) * cornerRadius ) );
				const DebugVertex verts[] =
				{
					{ Vec4( pos + p0, 1.0f ), color },
					{ Vec4( pos + p1, 1.0f ), color }
				};
				m_vertexArray.AppendVertices( verts, countof( verts ) );
			}
		}
	}

	DebugVertex verts[] =
	{
		// Top
		{ Vec4( pos + r.Rotate( ae::Vec3( -innerSize.x, 0.0f, innerSize.y + cornerRadius ) ), 1.0f ), color },
		{ Vec4( pos + r.Rotate( ae::Vec3( innerSize.x, 0.0f, innerSize.y + cornerRadius ) ), 1.0f ), color },
		// Bottom
		{ Vec4( pos + r.Rotate( ae::Vec3( -innerSize.x, 0.0f, -innerSize.y - cornerRadius ) ), 1.0f ), color },
		{ Vec4( pos + r.Rotate( ae::Vec3( innerSize.x, 0.0f, -innerSize.y - cornerRadius ) ), 1.0f ), color },
		// Left
		{ Vec4( pos + r.Rotate( ae::Vec3( -innerSize.x - cornerRadius, 0.0f, innerSize.y ) ), 1.0f ), color },
		{ Vec4( pos + r.Rotate( ae::Vec3( -innerSize.x - cornerRadius, 0.0f, -innerSize.y ) ), 1.0f ), color },
		// Right
		{ Vec4( pos + r.Rotate( ae::Vec3( innerSize.x + cornerRadius, 0.0f, innerSize.y ) ), 1.0f ), color },
		{ Vec4( pos + r.Rotate( ae::Vec3( innerSize.x + cornerRadius, 0.0f, -innerSize.y ) ), 1.0f ), color }
	};
	m_vertexArray.AppendVertices( verts, countof( verts ) );
	return 10 + 2 * cornerPointCount;
}

uint32_t DebugLines::AddCircle( Vec3 pos, Vec3 normal, float radius, Color color, uint32_t pointCount )
{
	uint32_t startVerts = m_vertexArray.GetVertexCount();
	if( startVerts + pointCount * 2 > m_vertexArray.GetMaxVertexCount()
		|| normal.LengthSquared() < 0.001f )
	{
		return 0;
	}
	
	normal.Normalize();
	float dot = normal.Dot( Vec3(0,0,1) );
	ae::Quaternion rotation( normal, ( dot < 0.99f && dot > -0.99f ) ? Vec3(0,0,1) : Vec3(1,0,0) );
	float angleInc = ae::PI * 2.0f / pointCount;
	
	for( uint32_t i = 0; i < pointCount; i++ )
	{
		float angle0 = angleInc * i;
		float angle1 = angleInc * ( i + 1 );
		
		DebugVertex verts[ 2 ];
		verts[ 0 ].pos = Vec4( cosf( angle0 ) * radius, 0.0f, sinf( angle0 ) * radius, 1.0f );
		verts[ 1 ].pos = Vec4( cosf( angle1 ) * radius, 0.0f, sinf( angle1 ) * radius, 1.0f );
		verts[ 0 ].pos.SetXYZ( pos + rotation.Rotate( verts[ 0 ].pos.GetXYZ() ) );
		verts[ 1 ].pos.SetXYZ( pos + rotation.Rotate( verts[ 1 ].pos.GetXYZ() ) );
		verts[ 0 ].color = color;
		verts[ 1 ].color = color;
		m_vertexArray.AppendVertices( verts, countof( verts ) );
	}
	return m_vertexArray.GetVertexCount() - startVerts;
}

uint32_t DebugLines::AddAABB( Vec3 pos, Vec3 halfSize, Color color )
{
	if( m_vertexArray.GetVertexCount() + 24 > m_vertexArray.GetMaxVertexCount() )
	{
		return 0;
	}
	Vec4 c[] =
	{
		Vec4( pos + Vec3( -halfSize.x, halfSize.y, halfSize.z ), 1.0f ),
		Vec4( pos + halfSize, 1.0f ),
		Vec4( pos + Vec3( halfSize.x, -halfSize.y, halfSize.z ), 1.0f ),
		Vec4( pos + Vec3( -halfSize.x, -halfSize.y, halfSize.z ), 1.0f ),
		Vec4( pos + Vec3( -halfSize.x, halfSize.y, -halfSize.z ), 1.0f ),
		Vec4( pos + Vec3( halfSize.x, halfSize.y, -halfSize.z ), 1.0f ),
		Vec4( pos + Vec3( halfSize.x, -halfSize.y, -halfSize.z ), 1.0f ),
		Vec4( pos + Vec3( -halfSize.x, -halfSize.y, -halfSize.z ), 1.0f ),
	};
	AE_STATIC_ASSERT( countof( c ) == 8 );
	DebugVertex verts[] =
	{
		// Top
		{ c[ 0 ], color },
		{ c[ 1 ], color },
		{ c[ 1 ], color },
		{ c[ 2 ], color },
		{ c[ 2 ], color },
		{ c[ 3 ], color },
		{ c[ 3 ], color },
		{ c[ 0 ], color },
		// Sides
		{ c[ 0 ], color },
		{ c[ 4 ], color },
		{ c[ 1 ], color },
		{ c[ 5 ], color },
		{ c[ 2 ], color },
		{ c[ 6 ], color },
		{ c[ 3 ], color },
		{ c[ 7 ], color },
		//Bottom
		{ c[ 4 ], color },
		{ c[ 5 ], color },
		{ c[ 5 ], color },
		{ c[ 6 ], color },
		{ c[ 6 ], color },
		{ c[ 7 ], color },
		{ c[ 7 ], color },
		{ c[ 4 ], color },
	};
	AE_STATIC_ASSERT( countof( c ) * 3 == countof( verts ) );
	m_vertexArray.AppendVertices( verts, countof( verts ) );
	return countof( verts );
}

uint32_t DebugLines::AddAABB( AABB aabb, Color color )
{
	return AddAABB( aabb.GetCenter(), aabb.GetHalfSize(), color );
}

uint32_t DebugLines::AddOBB( const Matrix4& transform, Color color )
{
	if( m_vertexArray.GetVertexCount() + 24 > m_vertexArray.GetMaxVertexCount() )
	{
		return 0;
	}
	Vec4 c[] =
	{
		transform * Vec4( -0.5f, 0.5f, 0.5f, 1.0f ),
		transform * Vec4( 0.5f, 0.5f, 0.5f, 1.0f ),
		transform * Vec4( 0.5f, -0.5f, 0.5f, 1.0f ),
		transform * Vec4( -0.5f, -0.5f, 0.5f, 1.0f ),
		transform * Vec4( -0.5f, 0.5f, -0.5f, 1.0f ),
		transform * Vec4( 0.5f, 0.5f, -0.5f, 1.0f ),
		transform * Vec4( 0.5f, -0.5f, -0.5f, 1.0f ),
		transform * Vec4( -0.5f, -0.5f, -0.5f, 1.0f ),
	};
	AE_STATIC_ASSERT( countof( c ) == 8 );
	DebugVertex verts[] =
	{
		// Top
		{ c[ 0 ], color },
		{ c[ 1 ], color },
		{ c[ 1 ], color },
		{ c[ 2 ], color },
		{ c[ 2 ], color },
		{ c[ 3 ], color },
		{ c[ 3 ], color },
		{ c[ 0 ], color },
		// Sides
		{ c[ 0 ], color },
		{ c[ 4 ], color },
		{ c[ 1 ], color },
		{ c[ 5 ], color },
		{ c[ 2 ], color },
		{ c[ 6 ], color },
		{ c[ 3 ], color },
		{ c[ 7 ], color },
		//Bottom
		{ c[ 4 ], color },
		{ c[ 5 ], color },
		{ c[ 5 ], color },
		{ c[ 6 ], color },
		{ c[ 6 ], color },
		{ c[ 7 ], color },
		{ c[ 7 ], color },
		{ c[ 4 ], color },
	};
	AE_STATIC_ASSERT( countof( c ) * 3 == countof( verts ) );
	m_vertexArray.AppendVertices( verts, countof( verts ) );
	return countof( verts );
}

uint32_t DebugLines::AddOBB( const OBB& obb, Color color )
{
	return AddOBB( obb.GetTransform(), color );
}

uint32_t DebugLines::AddSphere( Vec3 pos, float radius, Color color, uint32_t pointCount )
{
	if( m_vertexArray.GetVertexCount() + pointCount * 2 * 3 > m_vertexArray.GetMaxVertexCount() )
	{
		return 0;
	}
	return AddCircle( pos, Vec3(1,0,0), radius, color, pointCount )
		+ AddCircle( pos, Vec3(0,1,0), radius, color, pointCount )
		+ AddCircle( pos, Vec3(0,0,1), radius, color, pointCount );
}

uint32_t DebugLines::AddMesh( const Vec3* _vertices, uint32_t vertexStride, uint32_t count, Matrix4 transform, Color color )
{
	uint32_t startVerts = m_vertexArray.GetVertexCount();
	if( startVerts + count * 2 > m_vertexArray.GetMaxVertexCount()
		|| count % 3 != 0 )
	{
		return 0;
	}
	const uint8_t* vertices = (const uint8_t*)_vertices;
	bool identity = ( transform == ae::Matrix4::Identity() );
	for( uint32_t i = 0; i < count; i += 3 )
	{
		ae::Vec4 p[] =
		{
			ae::Vec4( *(const Vec3*)( vertices + i * vertexStride ), 1.0f ),
			ae::Vec4( *(const Vec3*)( vertices + ( i + 1 ) * vertexStride ), 1.0f ),
			ae::Vec4( *(const Vec3*)( vertices + ( i + 2 ) * vertexStride ), 1.0f ),
		};
		if( !identity )
		{
			p[ 0 ] = transform * p[ 0 ];
			p[ 1 ] = transform * p[ 1 ];
			p[ 2 ] = transform * p[ 2 ];
		}
		DebugVertex verts[] =
		{
			{ p[ 0 ], color },
			{ p[ 1 ], color },
			{ p[ 1 ], color },
			{ p[ 2 ], color },
			{ p[ 2 ], color },
			{ p[ 0 ], color },
		};
		m_vertexArray.AppendVertices( verts, countof( verts ) ); // @TODO: AppendVertices() does a bunch of safety checks. This could be really slow for big meshes.
	}
	return m_vertexArray.GetVertexCount() - startVerts;
}

uint32_t DebugLines::AddMesh( const Vec3* _vertices, uint32_t vertexStride, uint32_t vertexCount, const void* _indices, uint32_t indexSize, uint32_t indexCount, Matrix4 transform, Color color )
{
	uint32_t startVerts = m_vertexArray.GetVertexCount();
	if( startVerts + indexCount * 2 > m_vertexArray.GetMaxVertexCount()
		|| indexCount % 3 != 0
		|| ( indexSize != 2 && indexSize != 4 ) )
	{
		return 0;
	}
	const uint8_t* vertices = (const uint8_t*)_vertices;
	const uint16_t* indices16 = ( indexSize == 2 ) ? (const uint16_t*)_indices : nullptr;
	const uint32_t* indices32 = ( indexSize == 4 ) ? (const uint32_t*)_indices : nullptr;
	bool identity = ( transform == ae::Matrix4::Identity() );
	for( uint32_t i = 0; i < indexCount; i += 3 )
	{
		uint32_t index0 = indices16 ? (uint32_t)indices16[ i ] : indices32[ i ];
		uint32_t index1 = indices16 ? (uint32_t)indices16[ i + 1 ] : indices32[ i + 1 ];
		uint32_t index2 = indices16 ? (uint32_t)indices16[ i + 2 ] : indices32[ i + 2 ];
		AE_ASSERT( index0 < vertexCount );
		AE_ASSERT( index1 < vertexCount );
		AE_ASSERT( index2 < vertexCount );
		ae::Vec4 p[] =
		{
			ae::Vec4( *(const Vec3*)( vertices + index0 * vertexStride ), 1.0f ),
			ae::Vec4( *(const Vec3*)( vertices + index1 * vertexStride ), 1.0f ),
			ae::Vec4( *(const Vec3*)( vertices + index2 * vertexStride ), 1.0f ),
		};
		if( !identity )
		{
			p[ 0 ] = transform * p[ 0 ];
			p[ 1 ] = transform * p[ 1 ];
			p[ 2 ] = transform * p[ 2 ];
		}
		DebugVertex verts[] =
		{
			{ p[ 0 ], color },
			{ p[ 1 ], color },
			{ p[ 1 ], color },
			{ p[ 2 ], color },
			{ p[ 2 ], color },
			{ p[ 0 ], color },
		};
		m_vertexArray.AppendVertices( verts, countof( verts ) ); // @TODO: AppendVertices() does a bunch of safety checks. This could be really slow for big meshes.
	}
	return m_vertexArray.GetVertexCount() - startVerts;
}

uint32_t DebugLines::GetVertexCount() const
{
	return m_vertexArray.GetVertexCount();
}

uint32_t DebugLines::GetMaxVertexCount() const
{
	return m_vertexArray.GetVertexCount();
}

//------------------------------------------------------------------------------
// ae::SpriteFont
//------------------------------------------------------------------------------
SpriteFont::GlyphData::GlyphData()
{
	quad = ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 0.0f ) );
	uvs = ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 0.0f ) );
	advance = 0.0f;
}

void SpriteFont::SetGlyph( char c, ae::Rect quad, ae::Rect uvs, float advance )
{
	if( c < 32 || c >= 32 + countof(m_glyphs) )
	{
		return;
	}
	m_glyphs[ c - 32 ].quad = quad;
	m_glyphs[ c - 32 ].uvs = uvs;
	m_glyphs[ c - 32 ].advance = advance;
}

bool SpriteFont::GetGlyph( char c, ae::Rect* quad, ae::Rect* uv, float* advance, float uiSize ) const
{
	if( c < 32 || c >= 32 + countof(m_glyphs) )
	{
		return false;
	}
	if( quad ) { *quad = m_glyphs[ c - 32 ].quad * uiSize; }
	if( uv ) { *uv = m_glyphs[ c - 32 ].uvs; }
	if( advance ) { *advance = m_glyphs[ c - 32 ].advance * uiSize; }
	return true;
}

float SpriteFont::GetTextWidth( const char* text, float uiSize ) const
{
	float width = 0.0f;
	float advance = 0.0f;
	while( *text )
	{
		if( *text == '\r' || *text == '\n' )
		{
			width = std::max( width, advance );
			advance = 0.0f;
		}
		else
		{
			GetGlyph( *text, nullptr, nullptr, &advance, uiSize );
		}
		text++;
	}
	return std::max( width, advance );
}

//------------------------------------------------------------------------------
// ae::SpriteRenderer member functions
//------------------------------------------------------------------------------
SpriteRenderer::SpriteRenderer( const ae::Tag& tag ) :
	m_params( tag ),
	m_spriteGroups( tag ),
	m_vertexArray( tag )
{}

void SpriteRenderer::Initialize( uint32_t maxGroups, uint32_t maxCount )
{
	m_params.Append( {}, maxGroups );
	m_spriteGroups.Reserve( maxCount );

	m_vertexArray.Initialize(
		sizeof(SpriteVertex), sizeof(SpriteIndex),
		4 * maxCount, 6 * maxCount,
		ae::Vertex::Primitive::Triangle,
		ae::Vertex::Usage::Dynamic, ae::Vertex::Usage::Static
	);
	m_vertexArray.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof(SpriteVertex, pos) );
	m_vertexArray.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof(SpriteVertex, color) );
	m_vertexArray.AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof(SpriteVertex, uv) );

	const uint16_t indices[] = { 3, 0, 1, 3, 1, 2 };
	ae::Scratch< SpriteIndex > indexBuffer( 6 * maxCount );
	for( uint16_t i = 0; i < maxCount; i++ )
	{
		const uint16_t offset = 4 * i;
		indexBuffer[ i * 6 ] = offset + indices[ 0 ];
		indexBuffer[ i * 6 + 1 ] = offset + indices[ 1 ];
		indexBuffer[ i * 6 + 2 ] = offset + indices[ 2 ];
		indexBuffer[ i * 6 + 3 ] = offset + indices[ 3 ];
		indexBuffer[ i * 6 + 4 ] = offset + indices[ 4 ];
		indexBuffer[ i * 6 + 5 ] = offset + indices[ 5 ];
	}
	m_vertexArray.SetIndices( indexBuffer.Data(), indexBuffer.Length() );
}

void SpriteRenderer::Terminate()
{
	m_vertexArray.Terminate();
	m_vertexArray.Terminate();
	m_spriteGroups.Clear();
	m_params.Clear();
}

void SpriteRenderer::AddSprite( uint32_t group, ae::Vec2 pos, ae::Vec2 size, ae::Rect uvs, ae::Color color )
{
	ae::Matrix4 localToScreen = ae::Matrix4::Translation( pos.x, pos.y, 0.0f ) * ae::Matrix4::Scaling( size.x, size.y, 1.0f );
	AddSprite( group, localToScreen, uvs, color );
}

void SpriteRenderer::AddSprite( uint32_t group, ae::Rect quad, ae::Rect uvs, ae::Color color )
{
	ae::Vec2 pos = quad.GetMin() + quad.GetSize() * 0.5f;
	ae::Vec2 size = quad.GetSize();
	ae::Matrix4 localToScreen = ae::Matrix4::Translation( pos.x, pos.y, 0.0f ) * ae::Matrix4::Scaling( size.x, size.y, 1.0f );
	AddSprite( group, localToScreen, uvs, color );
}

void SpriteRenderer::AddSprite( uint32_t group, const ae::Matrix4& transform, ae::Rect uvs, ae::Color color )
{
	if( m_vertexArray.GetVertexCount() >= m_vertexArray.GetMaxVertexCount() )
	{
		return;
	}
	
	ae::Vec2 min = uvs.GetMin();
	ae::Vec2 max = uvs.GetMax();
	SpriteVertex verts[] =
	{
		{ transform * ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, min.y ) },
		{ transform * ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, min.y ) },
		{ transform * ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, max.y ) },
		{ transform * ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, max.y ) }
	};
	m_vertexArray.AppendVertices( verts, countof(verts) );
	m_spriteGroups.Append( group );
}

void SpriteRenderer::AddText( uint32_t group, const char* text, const SpriteFont* font, ae::Rect region, float fontSize, float lineHeight, ae::Color color )
{
	ae::Vec2 offset( region.GetMin().x, region.GetMax().y - lineHeight );
	while( *text )
	{
		const char c = *text;
		const bool isSpace = isspace( c );
		ae::Rect quad, uv;
		float advance;
		font->GetGlyph( *text, &quad, &uv, &advance, fontSize );
		if( !isSpace )
		{
			AddSprite( group, quad + offset, uv, color );
		}
		offset.x += advance;
		text++;

		bool newline = false;
		if( isSpace && !isspace( *text ) )
		{
			const char* word = text;
			float wordSize = 0.0f;
			while( *word && !isspace( *word ) )
			{
				float advance2 = 0.0f;
				font->GetGlyph( *word, nullptr, nullptr, &advance2, fontSize );
				wordSize += advance2;
				word++;
			}
			if( offset.x + wordSize > region.GetMax().x )
			{
				newline = true;
			}
		}
		else if( c == '\r' || c == '\n' )
		{
			newline = true;
		}
		if( newline )
		{
			offset.x = region.GetMin().x;
			offset.y -= lineHeight;
		}
	}
}

void SpriteRenderer::SetParams( uint32_t group, const ae::Shader* shader, const ae::UniformList& uniforms )
{
	m_params[ group ] = { shader, uniforms };
}

void SpriteRenderer::Render()
{
	m_vertexArray.Upload();

	const uint32_t spriteCount = m_spriteGroups.Length();
	for( uint32_t i = 0; i < spriteCount; i++ )
	{
		const uint32_t group = m_spriteGroups[ i ];
		const GroupParams& params = m_params[ group ];
		// @TODO: Combine draw calls when consecutive sprites are the same group
		if( params.shader )
		{
			m_vertexArray.Draw( params.shader, params.uniforms, i * 2, 2 );
		}
	}

	Clear();
}

void SpriteRenderer::Clear()
{
	m_vertexArray.ClearVertices();
	m_spriteGroups.Clear();
	for( auto& p : m_params )
	{
		p = {};
	}
}

//------------------------------------------------------------------------------
// ae::DebugCamera member functions
//------------------------------------------------------------------------------
DebugCamera::DebugCamera( ae::Axis upAxis )
{
	m_worldUp = upAxis;
	AE_ASSERT_MSG( m_worldUp == ae::Axis::Y || m_worldUp == ae::Axis::Z, "Only +Y and +Z world up axes are supported" );
	m_Precalculate();
}

void DebugCamera::SetDistanceLimits( float min, float max )
{
	m_min = min;
	m_max = max;
	m_Precalculate();
}

void DebugCamera::Update( const ae::Input* input, float dt )
{
	if( !m_inputEnabled )
	{
		input = nullptr;
	}

	// Input
	const ae::Vec2 movement = input ? ae::Vec2( input->mouse.movement ) : ae::Vec2( 0.0f );
	const ae::Vec2 scroll = input ? input->mouse.scroll : ae::Vec2( 0.0f );
	const bool alt = input ? input->Get( ae::Key::LeftAlt ) : false;
	const bool shift = input ? input->Get( ae::Key::LeftShift ) : false;
	const bool control = input ? input->Get( ae::Key::LeftControl ) : false;
	const bool command = input ? input->Get( ae::Key::LeftSuper ) : false;
	const bool modifier = input ? ( ( control || command ) && ( !control || !command ) ) : false; // Either but not both
	const bool usingTouch = input ? input->mouse.usingTouch : false;
	const bool lmb = ( input && !usingTouch ) ? input->mouse.leftButton : false;
	const bool mmb = ( input && !usingTouch ) ? input->mouse.middleButton : false;
	const bool rmb = ( input && !usingTouch ) ? input->mouse.rightButton : false;
	const bool isTouching = usingTouch && ( input->mouse.leftButton || input->mouse.middleButton || input->mouse.rightButton );
	const bool isTouchScrolling = usingTouch && scroll.LengthSquared();

	// Update mode
	Mode nextMode = m_mode;
	if( modifier && isTouchScrolling ) // It's possible to right click
	{
		nextMode = Mode::Zoom;
	}
	else if( ( alt && mmb ) || ( shift && isTouchScrolling ) )
	{
		nextMode = Mode::Pan;
	}
	else if( ( alt && lmb ) || isTouchScrolling )
	{
		nextMode = Mode::Rotate;
	}
	else if( alt && rmb )
	{
		nextMode = Mode::Zoom;
	}
	else if( ( m_mode == Mode::Pan && mmb )
		|| ( m_mode == Mode::Rotate && lmb )
		|| ( m_mode == Mode::Zoom && rmb ) )
	{
		// Stay in mode
	}
	else
	{
		nextMode = Mode::None;
	}

	// Delay releasing the mouse so checks to mouse click up etc. fail
	if( m_preventModeExitImm )
	{
		m_preventModeExitImm--;
	}
	else
	{
		m_moveAccum += movement.Length();
	}
	// Change modes
	if( m_mode != nextMode )
	{
		if( nextMode != Mode::None )
		{
			m_mode = nextMode;
		}
		else if( !m_preventModeExitImm )
		{
			if( m_moveAccum >= 5.0f ) // In pixels
			{
				m_preventModeExitImm = 2;
			}
			else
			{
				m_mode = Mode::None;
			}
		}
		m_moveAccum = 0.0f;
	}
	
	// Rotation
	if( m_mode == Mode::Rotate )
	{
		// Assume right handed coordinate system
		// The focal point should move in the direction that the users hand is moving
		if( usingTouch )
		{
			m_yaw += scroll.x * 0.35f;
			m_pitch += scroll.y * 0.35f;
		}
		else
		{
			m_yaw -= movement.x * 0.005f; // Positive horizontal input should result in clockwise rotation around the z axis
			m_pitch += movement.y * 0.005f; // Positive vertical input should result in counter clockwise rotation around cameras right vector
		}
		m_pitch = ae::Clip( m_pitch, -ae::HALF_PI * 0.99f, ae::HALF_PI * 0.99f ); // Don't let camera flip
	}

	float zoomSpeed = m_dist / 75.0f;
	float panSpeed = m_dist / 100.0f;

	// Zoom
	if( m_mode == Mode::Zoom )
	{
		if( usingTouch )
		{
			m_dist -= scroll.y * 12.0f * zoomSpeed;
			m_dist += scroll.x * 12.0f * zoomSpeed;
		}
		else
		{
			m_dist += movement.y * 0.1f * zoomSpeed;
			m_dist -= movement.x * 0.1f * zoomSpeed;
		}
	}
	// Don't zoom when scrolling with touch
	if( !usingTouch )
	{
		m_dist += scroll.y * 2.5f * zoomSpeed; // Natural scroll dir to match pan
	}
	m_dist = ae::Clip( m_dist, m_min, m_max );

	// Recalculate camera offset from pivot and local axis'
	m_Precalculate();

	// Pan
	if( m_mode == Mode::Pan )
	{
		m_refocus = false; // Cancel refocus
	
		if( usingTouch )
		{
			m_pivot -= m_right * ( scroll.x * 2.0f * panSpeed );
			m_pivot += m_up * ( scroll.y * 2.0f * panSpeed );
		}
		else
		{
			float panSpeed = m_dist / 1000.0f;
			m_pivot -= m_right * ( movement.x * panSpeed );
			m_pivot -= m_up * ( movement.y * panSpeed );
		}
	}

	// Refocus
	if( m_refocus )
	{
		AE_ASSERT( m_mode != Mode::Pan );
		m_pivot = ae::DtLerp( m_pivot, 2.5f, dt, m_refocusPos );
		if( ( m_refocusPos - m_pivot ).Length() < 0.01f )
		{
			m_refocus = false;
			m_pivot = m_refocusPos;
		}
	}
}

void DebugCamera::Reset( ae::Vec3 pivot, ae::Vec3 pos )
{
	m_refocus = false;
	m_refocusPos = pivot;
	m_pivot = pivot;
	
	ae::Vec3 diff = pivot - pos;
	m_dist = diff.Length();
	
	if( m_worldUp == Axis::Y )
	{
		ae::Vec2 xz = diff.GetXZ();
#if _AE_DEBUG_
		AE_ASSERT( pivot.x != pos.x || pivot.z != pos.z );
#else
		if( xz != ae::Vec2( 0.0f ) )
#endif
		{
			xz.y = -xz.y; // -Z forward for right handed Y-Up
			m_yaw = xz.GetAngle();
			m_pitch = asinf( diff.y / m_dist );
		}
	}
	else if( m_worldUp == Axis::Z )
	{
		ae::Vec2 xy = diff.GetXY();
#if _AE_DEBUG_
		AE_ASSERT( pivot.x != pos.x || pivot.y != pos.y );
#else
		if( xy != ae::Vec2( 0.0f ) )
#endif
		{
			m_yaw = xy.GetAngle();
			m_pitch = asinf( diff.z / m_dist );
		}
	}
	
	m_Precalculate();
}

void DebugCamera::SetDistanceFromFocus( float distance )
{
	m_dist = distance;
	m_Precalculate();
}

void DebugCamera::Refocus( ae::Vec3 pivot )
{
	m_refocus = true;
	m_refocusPos = pivot;
	if( m_mode == Mode::Pan )
	{
		m_mode = Mode::None;
	}
}

void DebugCamera::SetInputEnabled( bool enabled )
{
	m_inputEnabled = enabled;
}

void DebugCamera::SetRotation( ae::Vec2 angles )
{
	m_yaw = angles.x;
	m_pitch = angles.y;
	m_Precalculate();
}

DebugCamera::Mode DebugCamera::GetMode() const
{
	return ( m_moveAccum >= 5.0f || m_preventModeExitImm ) ? m_mode : Mode::None;
}

bool DebugCamera::GetRefocusTarget( ae::Vec3* targetOut ) const
{
	if( !m_refocus )
	{
		return false;
	}
	if( targetOut )
	{
		*targetOut = m_refocusPos;
	}
	return true;
}

ae::Vec3 DebugCamera::RotationToForward( ae::Axis up, float yaw, float pitch )
{
	AE_ASSERT_MSG( ( up == Axis::Y || up == Axis::Z ), "Only +Y and +Z world up axes are supported");
	ae::Vec3 forward;
	if( up == Axis::Y )
	{
		forward = ae::Vec3( ae::Cos( yaw ), 0.0f, -ae::Sin( yaw ) );
	}
	else if( up == Axis::Z )
	{
		forward = ae::Vec3( ae::Cos( yaw ), ae::Sin( yaw ), 0.0f );
	}
	forward *= ae::Cos( pitch );
	forward += GetWorldUp( up ) * ae::Sin( pitch );
	return forward;
}

void DebugCamera::m_Precalculate()
{
	m_dist = ae::Clip( m_dist, m_min, m_max );
	m_forward = RotationToForward( m_worldUp, m_yaw, m_pitch );
	m_offset = -m_forward;
	m_offset *= m_dist;
	m_right = m_forward.Cross( GetWorldUp() ).SafeNormalizeCopy();
	m_up = m_right.Cross( m_forward ).SafeNormalizeCopy();
}

//------------------------------------------------------------------------------
// ae::Spline member functions
//------------------------------------------------------------------------------
Spline::Spline( ae::Tag tag ) :
	m_controlPoints( tag ),
	m_segments( tag )
{}

Spline::Spline( ae::Tag tag, const ae::Vec3* controlPoints, uint32_t count, bool loop ) :
	m_loop( loop ),
	m_controlPoints( tag ),
	m_segments( tag )
{
	Reserve( count );
	for( uint32_t i = 0; i < count; i++ )
	{
		m_controlPoints.Append( controlPoints[ i ] );
	}
	m_RecalculateSegments();
}

void Spline::Reserve( uint32_t controlPointCount )
{
	if( controlPointCount )
	{
		m_controlPoints.Reserve( controlPointCount );
		m_segments.Reserve( controlPointCount - ( m_loop ? 0 : 1 ) );
	}
}

void Spline::SetLooping( bool enabled )
{
	if( m_loop != enabled )
	{
		m_loop = enabled;
		m_RecalculateSegments();
	}
}

void Spline::AppendControlPoint( ae::Vec3 p )
{
	m_controlPoints.Append( p );
	m_RecalculateSegments();
}

void Spline::RemoveControlPoint( uint32_t index )
{
	m_controlPoints.Remove( index );
	m_RecalculateSegments();
}

void Spline::Clear()
{
	m_controlPoints.Clear();
	m_segments.Clear();
	m_length = 0.0f;
	m_aabb = ae::AABB();
}

ae::Vec3 Spline::GetControlPoint( uint32_t index ) const
{
	return m_controlPoints[ index ];
}

uint32_t Spline::GetControlPointCount() const
{
	return m_controlPoints.Length();
}

ae::Vec3 Spline::GetPoint( float distance ) const
{
	if( m_controlPoints.Length() == 0 )
	{
		return ae::Vec3( 0.0f );
	}
	else if( m_controlPoints.Length() == 1 )
	{
		return m_controlPoints[ 0 ];
	}

	if( m_length < 0.001f )
	{
		distance = 0.0f;
	}
	else if( m_loop && ( distance < 0.0f || distance >= m_length ) )
	{
		distance = ae::Mod( distance, m_length );
	}

	for( uint32_t i = 0; i < m_segments.Length(); i++ )
	{
		const Segment& segment = m_segments[ i ];
		if( segment.GetLength() >= distance )
		{
			return segment.GetPoint( distance );
		}

		distance -= segment.GetLength();
	}

	return m_controlPoints[ m_controlPoints.Length() - 1 ];
}

float Spline::GetMinDistance( ae::Vec3 p, ae::Vec3* nearestOut, float* tOut ) const
{
	ae::Vec3 closest( 0.0f );
	float closestDistance = ae::MaxValue< float >();

	float t = 0.0f;
	float tClosest = 0.0f;
	if( m_controlPoints.Length() == 1 )
	{
		closest = m_controlPoints[ 0 ];
		closestDistance = ( closest - p ).Length();
	}
	else
	{
		for( uint32_t i = 0; i < m_segments.Length(); i++ )
		{
			const Segment& segment = m_segments[ i ];
			// @NOTE: Don't check segments that are further away than the already closest point
			if( segment.GetAABB().GetSignedDistanceFromSurface( p ) <= closestDistance )
			{
				ae::Vec3 segmentP;
				float tSegment;
				float d = segment.GetMinDistance( p, &segmentP, &tSegment );
				if( d < closestDistance )
				{
					closest = segmentP;
					closestDistance = d;
					tClosest = t + tSegment;
				}
			}
			t += segment.GetLength(); // After closest check so segment is not included
		}
	}
	
	if( nearestOut )
	{
		*nearestOut = closest;
	}
	if( tOut )
	{
		*tOut = tClosest;
	}
	return closestDistance;
}

float Spline::GetLength() const
{
	return m_length;
}

void Spline::m_RecalculateSegments()
{
	m_segments.Clear();
	m_length = 0.0f;

	if( m_controlPoints.Length() < 2 )
	{
		return;
	}

	int32_t segmentCount = m_controlPoints.Length();
	if( !m_loop )
	{
		segmentCount--;
	}

	m_aabb = ae::AABB( ae::Vec3( ae::MaxValue< float >() ), ae::Vec3( ae::MinValue< float >() ) );

	for( int32_t i = 0; i < segmentCount; i++ )
	{
		ae::Vec3 p0 = m_GetControlPoint( i - 1 );
		ae::Vec3 p1 = m_GetControlPoint( i );
		ae::Vec3 p2 = m_GetControlPoint( i + 1 );
		ae::Vec3 p3 = m_GetControlPoint( i + 2 );

		Segment* segment = &m_segments.Append( Segment() );
		segment->Init( p0, p1, p2, p3 );
		
		m_length += segment->GetLength();
		m_aabb.Expand( segment->GetAABB() );
	}
}

ae::Vec3 Spline::m_GetControlPoint( int32_t index ) const
{
	if( m_loop )
	{
		return m_controlPoints[ ae::Mod( index, (int)m_controlPoints.Length() ) ];
	}
	else if( index == -1 )
	{
		ae::Vec3 p0 = m_controlPoints[ 0 ];
		ae::Vec3 p1 = m_controlPoints[ 1 ];
		return ( p0 + p0 - p1 );
	}
	else if( index == m_controlPoints.Length() )
	{
		ae::Vec3 p0 = m_controlPoints[ index - 2 ];
		ae::Vec3 p1 = m_controlPoints[ index - 1 ];
		return ( p1 + p1 - p0 );
	}
	else
	{
		return m_controlPoints[ index ];
	}
}

void Spline::Segment::Init( ae::Vec3 p0, ae::Vec3 p1, ae::Vec3 p2, ae::Vec3 p3 )
{
	const float alpha = 0.5f;
	const float tension = 0.0f;

	float t01 = pow( ( p0 - p1 ).Length(), alpha );
	float t12 = pow( ( p1 - p2 ).Length(), alpha );
	float t23 = pow( ( p2 - p3 ).Length(), alpha );

	ae::Vec3 m1 = ( p2 - p1 + ( ( p1 - p0 ) / t01 - ( p2 - p0 ) / ( t01 + t12 ) ) * t12 ) * ( 1.0f - tension );
	ae::Vec3 m2 = ( p2 - p1 + ( ( p3 - p2 ) / t23 - ( p3 - p1 ) / ( t12 + t23 ) ) * t12 ) * ( 1.0f - tension );

	m_a = ( p1 - p2 ) * 2.0f + m1 + m2;
	m_b = ( p1 - p2 ) * -3.0f - m1 - m1 - m2;
	m_c = m1;
	m_d = p1;

	m_length = ( p2 - p1 ).Length();
	m_resolution = 1;

	float nextLength = m_length;
	uint32_t nextResolution = m_resolution;
	do
	{
		m_aabb = ae::AABB( GetPoint0(), GetPoint0() );

		m_length = nextLength;
		m_resolution = nextResolution;

		nextResolution = m_resolution * 2;
		nextLength = 0.0f;
		for( uint32_t i = 0; i < nextResolution; i++ )
		{
			ae::Vec3 s0 = GetPoint01( i / (float)nextResolution );
			ae::Vec3 s1 = GetPoint01( ( i + 1 ) / (float)nextResolution );
			nextLength += ( s1 - s0 ).Length();

			m_aabb.Expand( s1 );
		}
	} while( ae::Abs( nextLength - m_length ) > 0.001f );
}

ae::Vec3 Spline::Segment::GetPoint01( float t ) const
{
	return ( m_a * t * t * t ) + ( m_b * t * t ) + ( m_c * t ) + m_d;
}

ae::Vec3 Spline::Segment::GetPoint0() const
{
	return m_d;
}

ae::Vec3 Spline::Segment::GetPoint1() const
{
	return m_a + m_b + m_c + m_d;
}

ae::Vec3 Spline::Segment::GetPoint( float d ) const
{
	if( d <= 0.0f )
	{
		return GetPoint0();
	}
	else if( d < m_length )
	{
		// @NOTE: Search is required here because even within a segment
		//        t (0-1) does not map linearly to arc length. This is
		//        an approximate mapping from arc length -> t based on
		//        the optimized resolution value calculated above.
		for( uint32_t i = 0; i < m_resolution; i++ )
		{
			ae::Vec3 s0 = GetPoint01( i / (float)m_resolution );
			ae::Vec3 s1 = GetPoint01( ( i + 1 ) / (float)m_resolution );
			float l = ( s1 - s0 ).Length();
			if( l >= d )
			{
				return ae::Lerp( s0, s1, d / l );
			}
			else
			{
				d -= l;
			}
		}
	}
	
	return GetPoint1();
}

float Spline::Segment::GetMinDistance( ae::Vec3 p, ae::Vec3* pOut, float* tOut ) const
{
	float t = 0.0f;
	ae::Vec3 s0 = GetPoint0();
	ae::Vec3 closest = s0;
	float tClosest = 0.0f;
	float closestDist = ae::MaxValue< float >();
	for( uint32_t i = 1; i <= m_resolution; i++ )
	{
		ae::Vec3 s1 = GetPoint01( i / (float)m_resolution );
		ae::LineSegment segment( s0, s1 );
		s0 = s1;

		ae::Vec3 r;
		float d = segment.GetDistance( p, &r );
		if( d < closestDist )
		{
			closest = r;
			closestDist = d;
			if( tOut )
			{
				tClosest = t + ( r - segment.GetStart() ).Length();
			}
		}

		if( tOut )
		{
			t += segment.GetLength();
		}
	}
	if( pOut )
	{
		*pOut = closest;
	}
	if( tOut )
	{
		*tOut = tClosest;
	}
	return closestDist;
}

//------------------------------------------------------------------------------
// ae::RaycastResult member functions
//------------------------------------------------------------------------------
void RaycastResult::Accumulate( const RaycastParams& params, const RaycastResult& prev, RaycastResult* next )
{
	if( !next )
	{
		return;
	}
	constexpr uint32_t hitsSize = decltype(next->hits)::Size();
	uint32_t accumHitCount = 0;
	Hit accumHits[ hitsSize * 2 ];
	
	for( uint32_t i = 0; i < next->hits.Length(); i++ )
	{
		accumHits[ accumHitCount ] = next->hits[ i ];
		accumHitCount++;
	}
	for( uint32_t i = 0; i < prev.hits.Length(); i++ )
	{
		accumHits[ accumHitCount ] = prev.hits[ i ];
		accumHitCount++;
	}
	std::sort( accumHits, accumHits + accumHitCount, []( const Hit& h0, const Hit& h1 ){ return h0.distance < h1.distance; } );
	
	next->hits.Clear();
	accumHitCount = ae::Min( accumHitCount, params.maxHits, hitsSize );
	for( uint32_t i = 0; i < accumHitCount; i++ )
	{
		next->hits.Append( accumHits[ i ] );
	}
}

//------------------------------------------------------------------------------
// ae::PushOutInfo member functions
//------------------------------------------------------------------------------
void PushOutInfo::Accumulate( const PushOutParams& params, const PushOutInfo& prev, PushOutInfo* next )
{
	if( !next )
	{
		return;
	}
	// @NOTE: Leave next::position/velocity unchanged since it's the latest
	// @TODO: Params are currently not used, but they could be used for sorting later
	auto&& nHits = next->hits;
	for( auto&& hit : prev.hits )
	{
		if( nHits.Length() < nHits.Size() )
		{
			nHits.Append( hit );
		}
		else
		{
			break;
		}
	}
}

//------------------------------------------------------------------------------
// ae::Keyframe member functions
//------------------------------------------------------------------------------
Keyframe::Keyframe( const ae::Matrix4& transform )
{
	translation = transform.GetTranslation();
	rotation = transform.GetRotation();
	scale = transform.GetScale();
}

ae::Matrix4 Keyframe::GetLocalTransform() const
{
	ae::Matrix4 rot = ae::Matrix4::Identity();
	rot.SetRotation( rotation );
	return ae::Matrix4::Translation( translation ) * rot * ae::Matrix4::Scaling( scale );
}

Keyframe Keyframe::Lerp( const Keyframe& target, float t ) const
{
	Keyframe result;
	result.translation = translation.Lerp( target.translation, t );
	result.rotation = rotation.Nlerp( target.rotation, t );
	result.scale = scale.Lerp( target.scale, t );
	return result;
}

//------------------------------------------------------------------------------
// ae::Animation member functions
//------------------------------------------------------------------------------
ae::Keyframe Animation::GetKeyframeByTime( const char* boneName, float time ) const
{
	return GetKeyframeByPercent( boneName, ae::Delerp( 0.0f, duration, time ) );
}

ae::Keyframe Animation::GetKeyframeByPercent( const char* boneName, float percent ) const
{
	const ae::Array< ae::Keyframe >* boneKeyframes = keyframes.TryGet( boneName );
	if( !boneKeyframes || !boneKeyframes->Length() )
	{
		return ae::Keyframe();
	}
	percent = loop ? ae::Mod( percent, 1.0f ) : ae::Clip01( percent );
	float f = boneKeyframes->Length() * percent;
	uint32_t f0 = (uint32_t)f;
	uint32_t f1 = ( f0 + 1 );
	f0 = loop ? ( f0 % boneKeyframes->Length() ) : ae::Clip( f0, 0u, boneKeyframes->Length() - 1 );
	f1 = loop ? ( f1 % boneKeyframes->Length() ) : ae::Clip( f1, 0u, boneKeyframes->Length() - 1 );
	return (*boneKeyframes)[ f0 ].Lerp( (*boneKeyframes)[ f1 ], ae::Clip01( f - f0 ) );
}

void Animation::AnimateByTime( class Skeleton* target, float time, float strength, const ae::Bone** mask, uint32_t maskCount ) const
{
	AnimateByPercent( target, ae::Delerp( 0.0f, duration, time ), strength, mask, maskCount );
}

void Animation::AnimateByPercent( class Skeleton* target, float percent, float strength, const ae::Bone** mask, uint32_t maskCount ) const
{
	ae::Array< const ae::Bone* > tempBones = AE_ALLOC_TAG_FIXME; // @TODO: Allocate once in Animation class
	ae::Array< ae::Matrix4 > tempTransforms = AE_ALLOC_TAG_FIXME; // @TODO: Allocate once in Animation class
	tempBones.Reserve( target->GetBoneCount() );
	tempTransforms.Reserve( target->GetBoneCount() );
	
	strength = ae::Clip01( strength );
	const ae::Bone** maskEnd = mask + maskCount;
	
	for( uint32_t i = 0; i < target->GetBoneCount(); i++ )
	{
		const ae::Bone* bone = target->GetBoneByIndex( i );
		AE_ASSERT( bone->index == i );
		AE_ASSERT( bone > bone->parent );
		
		float keyStrength = strength;
		bool found = ( std::find( mask, maskEnd, bone ) != maskEnd );
		if( found )
		{
			keyStrength = 0.0f;
		}
		
		tempBones.Append( bone );
		ae::Keyframe keyframe = GetKeyframeByPercent( bone->name.c_str(), percent );
		if( keyStrength < 1.0f )
		{
			const ae::Matrix4 current = bone->parentToChild;
			const ae::Vec3 currTranslation = current.GetTranslation();
			const ae::Quaternion currRotation = current.GetRotation();
			const ae::Vec3 currScale = current.GetScale();
			keyframe.translation = currTranslation.Lerp( keyframe.translation, keyStrength );
			keyframe.rotation = currRotation.Nlerp( keyframe.rotation, keyStrength );
			keyframe.scale = currScale.Lerp( keyframe.scale, keyStrength );
		}
		tempTransforms.Append( keyframe.GetLocalTransform() );
	}
	target->SetLocalTransforms( tempBones.Data(), tempTransforms.Data(), target->GetBoneCount() );
}

//------------------------------------------------------------------------------
// ae::Skeleton member functions
//------------------------------------------------------------------------------
void Skeleton::Initialize( uint32_t maxBones )
{
	m_bones.Clear();
	m_bones.Reserve( maxBones );
	
	Bone* bone = &m_bones.Append( {} );
	bone->name = "root";
	bone->index = 0;
	bone->modelToBone = ae::Matrix4::Identity();
	bone->parentToChild = ae::Matrix4::Identity();
	bone->parent = nullptr;
}

void Skeleton::Initialize( const Skeleton* otherPose )
{
	Initialize( otherPose->GetBoneCount() );
	
	const void* beginCheck = m_bones.Data();
	ae::Bone* root = &m_bones[ 0 ];
	const ae::Bone* otherRoot = otherPose->GetRoot();
	root->modelToBone = otherRoot->modelToBone;
	root->parentToChild = otherRoot->parentToChild;
	root->boneToModel = otherRoot->boneToModel;
	for( uint32_t i = 1; i < otherPose->m_bones.Length(); i++ ) // Skip root
	{
		const ae::Bone& otherBone = otherPose->m_bones[ i ];
		const ae::Bone* parent = &m_bones[ otherBone.parent->index ];
		AddBone( parent, otherBone.name.c_str(), otherBone.parentToChild );
	}
	AE_ASSERT( beginCheck == m_bones.Data() );
}

const Bone* Skeleton::AddBone( const Bone* _parent, const char* name, const ae::Matrix4& parentToChild )
{
	Bone* parent = const_cast< Bone* >( _parent );
	AE_ASSERT_MSG( m_bones.Size(), "Must call ae::Skeleton::Initialize() before calling ae::Skeleton::AddBone()" );
	AE_ASSERT_MSG( m_bones.begin() <= parent && parent < m_bones.end(), "ae::Bones must have a parent from the same ae::Skeleton" );
	if( !parent || m_bones.Length() == m_bones.Size() )
	{
		return nullptr;
	}
#if _AE_DEBUG_
	Bone* beginCheck = m_bones.Data();
#endif
	Bone* bone = &m_bones.Append( {} );
#if _AE_DEBUG_
	AE_ASSERT( beginCheck == m_bones.Data() );
#endif

	bone->name = name;
	bone->index = m_bones.Length() - 1;
	bone->modelToBone = parent->modelToBone * parentToChild;
	bone->parentToChild = parentToChild;
	bone->boneToModel = bone->modelToBone.GetInverse();
	bone->parent = parent;
	
	Bone** children = &parent->firstChild;
	while( *children )
	{
		children = &(*children)->nextSibling;
	}
	*children = bone;
	
	return bone;
}

void Skeleton::SetLocalTransforms( const Bone** targets, const ae::Matrix4* parentToChildTransforms, uint32_t count )
{
	if( !count )
	{
		return;
	}
	
	for( uint32_t i = 0; i < count; i++ )
	{
		ae::Bone* bone = const_cast< ae::Bone* >( targets[ i ] );
		AE_ASSERT_MSG( bone, "Null bone passed to skeleton when setting transforms" );
		AE_ASSERT_MSG( m_bones.begin() <= bone && bone < m_bones.end(), "Transform target '#' is not part of this skeleton", bone->name );
		bone->parentToChild = parentToChildTransforms[ i ];
	}
	
	m_bones[ 0 ].modelToBone = m_bones[ 0 ].modelToBone;
	for( uint32_t i = 1; i < m_bones.Length(); i++ )
	{
		ae::Bone* bone = &m_bones[ i ];
		AE_ASSERT( bone->parent );
		AE_ASSERT( bone->parent < bone );
		bone->modelToBone = bone->parent->modelToBone * bone->parentToChild;
		bone->boneToModel = bone->modelToBone.GetInverse();
	}
}

void Skeleton::SetTransform( const Bone* _target, const ae::Matrix4& modelToBone )
{
	const intptr_t _index = ( _target - m_bones.Data() );
	AE_ASSERT( _index >= 0 && _index < (intptr_t)m_bones.Length() );
	const uint32_t index = (uint32_t)_index;
	ae::Bone* target = &m_bones[ index ];
	target->modelToBone = modelToBone;
	target->boneToModel = modelToBone.GetInverse();
	if( !target->parent )
	{
		target->parentToChild = modelToBone;
	}
	else
	{
		target->parentToChild = target->parent->boneToModel * modelToBone;
	}
	for( uint32_t i = index + 1; i < m_bones.Length(); i++ )
	{
		ae::Bone* bone = &m_bones[ i ];
		bone->modelToBone = bone->parent->modelToBone * bone->parentToChild;
		bone->boneToModel = bone->modelToBone.GetInverse();
	}
}

void Skeleton::SetLocalTransform( const Bone* target, const ae::Matrix4& parentToChild )
{
	SetLocalTransforms( &target, &parentToChild, 1 );
}

const Bone* Skeleton::GetRoot() const
{
	return m_bones.Data();
}

const Bone* Skeleton::GetBoneByName( const char* name ) const
{
	int32_t idx = m_bones.FindFn( [ name ]( const Bone& b ){ return b.name == name; } );
	return ( idx >= 0 ) ? &m_bones[ idx ] : nullptr;
}

const Bone* Skeleton::GetBoneByIndex( uint32_t index ) const
{
#if _AE_DEBUG_
	AE_ASSERT( m_bones[ index ].index == index );
#endif
	return &m_bones[ index ];
}

const Bone* Skeleton::GetBones() const
{
	return m_bones.Data();
}

uint32_t Skeleton::GetBoneCount() const
{
	return m_bones.Length();
}

//------------------------------------------------------------------------------
// ae::IK member functions
//------------------------------------------------------------------------------
ae::Vec2 IK::GetNearestPointOnEllipse( ae::Vec2 halfSize, ae::Vec2 center, ae::Vec2 p )
{
	// https://stackoverflow.com/a/46007540/2423134
	// https://blog.chatfield.io/simple-method-for-distance-to-ellipse/
	// https://github.com/0xfaded/ellipse_demo/issues/1
	const float px = ae::Abs( p[ 0 ] );
	const float py = ae::Abs( p[ 1 ] );
	const float a = ae::Abs( halfSize.x );
	const float b = ae::Abs( halfSize.y );

	float tx = 0.707f;
	float ty = 0.707f;
	// Only 3 iterations should be needed for high quality results
	for( uint32_t i = 0; i < 3; i++ )
	{
		const float x = a * tx;
		const float y = b * ty;
		const float ex = ( a * a - b * b ) * ( tx * tx * tx ) / a;
		const float ey = ( b * b - a * a ) * ( ty * ty * ty ) / b;
		const float rx = x - ex;
		const float ry = y - ey;
		const float qx = px - ex;
		const float qy = py - ey;
		const float r = hypotf( ry, rx );
		const float q = hypotf( qy, qx );
		tx = ae::Min( 1.0f, ae::Max( 0.0f, ( qx * r / q + ex ) / a ) );
		ty = ae::Min( 1.0f, ae::Max( 0.0f, ( qy * r / q + ey ) / b ) );
		const float t = hypotf( ty, tx );
		tx /= t;
		ty /= t;
	}

	return ae::Vec2( copysignf( a * tx, p[ 0 ] ), copysignf( b * ty, p[ 1 ] ) );
}

ae::Vec3 IK::GetAxisVector( ae::Axis axis, bool negative )
{
	if( negative )
	{
		switch( axis )
		{
			case ae::Axis::X: return ae::Vec3( 1, 0, 0 );
			case ae::Axis::Y: return ae::Vec3( 0, 1, 0 );
			case ae::Axis::Z: return ae::Vec3( 0, 0, 1 );
			case ae::Axis::NegativeX: return ae::Vec3( -1, 0, 0 );
			case ae::Axis::NegativeY: return ae::Vec3( 0, -1, 0 );
			case ae::Axis::NegativeZ: return ae::Vec3( 0, 0, -1 );
			default: return ae::Vec3( 0.0f );
		}
	}
	else
	{
		switch( axis )
		{
			case ae::Axis::X:
			case ae::Axis::NegativeX:
				return ae::Vec3( 1, 0, 0 );
			case ae::Axis::Y:
			case ae::Axis::NegativeY:
				return ae::Vec3( 0, 1, 0 );
			case ae::Axis::Z:
			case ae::Axis::NegativeZ:
				return ae::Vec3( 0, 0, 1 );
			default:
				return ae::Vec3( 0.0f );
		}
	}
}

ae::Vec3 IK::ClipJoint(
	float bindBoneLength,
	ae::Vec3 j0Pos, // Parent
	ae::Quaternion j0Ori, // Parent
	ae::Vec3 j1Pos, // Child
	const ae::IKConstraints& j1Constraints, // Child
	ae::Color debugColor )
{
	const ae::Axis ha = j1Constraints.horizontalAxis;
	const ae::Axis va = j1Constraints.bendAxis;
	const ae::Axis pa = j1Constraints.twistAxis;
	const float (&j0AngleLimits)[ 4 ] = j1Constraints.rotationLimits;
	const float clipLen = debugLines ? ( bindBoneLength * debugJointScale ) : bindBoneLength;
	const float q[ 4 ] =
	{
		clipLen * ae::Tan( ae::Clip( j0AngleLimits[ 0 ], 0.01f, ae::HalfPi - 0.01f ) ),
		clipLen * ae::Tan( ae::Clip( j0AngleLimits[ 1 ], 0.01f, ae::HalfPi - 0.01f ) ),
		clipLen * ae::Tan( -ae::Clip( j0AngleLimits[ 2 ], 0.01f, ae::HalfPi - 0.01f ) ),
		clipLen * ae::Tan( -ae::Clip( j0AngleLimits[ 3 ], 0.01f, ae::HalfPi - 0.01f ) )
	};
	
	// (f) the joint position p2 is relocated to a new position, p2^, which is
	// the nearest point on that composite ellipsoidal shape from p2, ensuring
	// that the new joint position p02 will be within the allowed rotational range
	const ae::Vec2 unclipped = [&]()
	{
		const ae::Vec3 axis = Build3D( ha, va, pa, 0, 0, 1 );
		const ae::Vec3 bonePos = axis * clipLen;
		const ae::Plane ellipsePlane = ae::Plane( bonePos, axis );
		const ae::Vec3 joint = j0Ori.GetInverse().Rotate( j1Pos - j0Pos );
		float t;
		ae::Vec3 p;
		if( !ellipsePlane.IntersectLine( ae::Vec3( 0.0f ), joint, &p, &t ) || t < 0.0f )
		{
			p = ellipsePlane.GetClosestPoint( joint );
			p = bonePos + ( p - bonePos ).SafeNormalizeCopy() * ae::Max( q[ 0 ], q[ 1 ], q[ 2 ], q[ 3 ] );
		}
		return ae::Vec2( GetAxis( ha, p ), GetAxis( va, p ) );
	}();
	const ae::Vec2 quadrantEllipseSize = [q, unclipped]()
	{
		if( unclipped.x >= 0.0f && unclipped.y >= 0.0f ) { return ae::Vec2( q[ 0 ], q[ 1 ] ); } // +x +y
		if( unclipped.x <= 0.0f && unclipped.y >= 0.0f ) { return ae::Vec2( q[ 2 ], q[ 1 ] ); } // -x +y
		if( unclipped.x <= 0.0f && unclipped.y <= 0.0f ) { return ae::Vec2( q[ 2 ], q[ 3 ] ); } // -x -y
		return ae::Vec2( q[ 0 ], q[ 3 ] ); // +x -y
	}();

	const ae::Vec2 ellipseEdge = GetNearestPointOnEllipse( quadrantEllipseSize, ae::Vec2( 0.0f ), unclipped );
	const ae::Vec2 posClipped = ( unclipped.LengthSquared() > ellipseEdge.LengthSquared() ) ? ellipseEdge : unclipped;
	// (g) move p2^ to p2', to conserve bone length
	const ae::Vec3 resultLocal = Build3D( ha, va, pa, posClipped.x, posClipped.y, clipLen ).NormalizeCopy() * bindBoneLength;
	// Returns the offset to the child or parent bone so that the constraints are satisfied
	const ae::Vec3 resultWorldOffset = ( j0Pos + j0Ori.Rotate( resultLocal ) ) - j1Pos;

	if( debugLines )
	{
		// debugLines->AddSphere( j0Pos, bindBoneLength, debugColor, 16 ); // Bone length debug
		auto t = [&]( ae::Vec3 p )
		{
			return debugModelToWorld.TransformPoint3x4( p );
		};
		const ae::Matrix4 j0 = ae::Matrix4::Translation( j0Pos ) * j0Ori.GetTransformMatrix();
		const ae::Matrix4 tj0 = debugModelToWorld * j0;
		const ae::Vec3 unclippedWorld = ( tj0 * ae::Vec4( Build3D( ha, va, pa, unclipped.x, unclipped.y, clipLen ), 1 ) ).GetXYZ();
		const ae::Vec3 unclippedWorldClipped = ( tj0 * ae::Vec4( Build3D( ha, va, pa, posClipped.x, posClipped.y, clipLen ), 1 ) ).GetXYZ();
		debugLines->AddSphere( unclippedWorld, debugJointScale * 0.025f, debugColor, 4 );
		debugLines->AddSphere( unclippedWorldClipped, debugJointScale * 0.025f, debugColor, 4 );
		debugLines->AddLine( unclippedWorld, unclippedWorldClipped, debugColor );
		debugLines->AddLine( tj0.GetTranslation(), tj0.TransformPoint3x4( Build3D( ha, va, pa, q[ 0 ], 0, clipLen ) ), debugColor );
		debugLines->AddLine( tj0.GetTranslation(), tj0.TransformPoint3x4( Build3D( ha, va, pa, 0, q[ 1 ], clipLen ) ), debugColor );
		debugLines->AddLine( tj0.GetTranslation(), tj0.TransformPoint3x4( Build3D( ha, va, pa, q[ 2 ], 0, clipLen ) ), debugColor );
		debugLines->AddLine( tj0.GetTranslation(), tj0.TransformPoint3x4( Build3D( ha, va, pa, 0, q[ 3 ], clipLen ) ), debugColor );
		for( uint32_t i = 0; i < 8; i++ )
		{
			const ae::Vec3 q0 = Build3D( ha, va, pa, q[ 0 ], q[ 1 ], 1 ); // +x +y
			const ae::Vec3 q1 = Build3D( ha, va, pa, q[ 2 ], q[ 1 ], 1 ); // -x +y
			const ae::Vec3 q2 = Build3D( ha, va, pa, q[ 2 ], q[ 3 ], 1 ); // -x -y
			const ae::Vec3 q3 = Build3D( ha, va, pa, q[ 0 ], q[ 3 ], 1 ); // +x -y
			const float step = ( ae::HalfPi / 8 );
			const float angle = i * step;
			const ae::Vec3 l0 = Build3D( ha, va, pa, ae::Cos( angle ), ae::Sin( angle ), clipLen, false );
			const ae::Vec3 l1 = Build3D( ha, va, pa, ae::Cos( angle + step ), ae::Sin( angle + step ), clipLen, false );
			const ae::Vec4 p0 = tj0 * ae::Vec4( l0 * q0, 1.0f );
			const ae::Vec4 p1 = tj0 * ae::Vec4( l1 * q0, 1.0f );
			const ae::Vec4 p2 = tj0 * ae::Vec4( l0 * q1, 1.0f );
			const ae::Vec4 p3 = tj0 * ae::Vec4( l1 * q1, 1.0f );
			const ae::Vec4 p4 = tj0 * ae::Vec4( l0 * q2, 1.0f );
			const ae::Vec4 p5 = tj0 * ae::Vec4( l1 * q2, 1.0f );
			const ae::Vec4 p6 = tj0 * ae::Vec4( l0 * q3, 1.0f );
			const ae::Vec4 p7 = tj0 * ae::Vec4( l1 * q3, 1.0f );
			debugLines->AddLine( p0.GetXYZ(), p1.GetXYZ(), debugColor );
			debugLines->AddLine( p2.GetXYZ(), p3.GetXYZ(), debugColor );
			debugLines->AddLine( p4.GetXYZ(), p5.GetXYZ(), debugColor );
			debugLines->AddLine( p6.GetXYZ(), p7.GetXYZ(), debugColor );
		}
	}

	return resultWorldOffset;
}

IK::IK( ae::Tag tag ) :
	tag( tag ),
	chain( tag ),
	joints( tag ),
	pose( tag )
{}

void IK::Run( uint32_t iterationCount, ae::Skeleton* poseOut )
{
	AE_ASSERT( !chain.Length() || pose.GetBoneCount() );
	AE_ASSERT_MSG( bindPose, "A bind pose is required to run IK" );
	// @TODO: More thorough validation
	AE_ASSERT_MSG( bindPose->GetBoneCount() == pose.GetBoneCount(), "Bind pose and pose hierarchy must match" );

	struct IKBone
	{
		ae::Vec3 modelPos;
		ae::Quaternion modelToBoneRot;
		float length; // Fixed distance between pos and parent pos
		float defaultTwist; // The bind pose twist angle between this bone and its parent
	};

	//AE_ASSERT( joints.Length() == 0 || joints.Length() == 1 || joints.Length() == bones.Length() );
	auto GetConstraints = [ this ]( uint32_t idx ) -> const ae::IKConstraints&
	{
		switch( joints.Length() )
		{
			case 0:
			{
				static const ae::IKConstraints s_default;
				return s_default;
			}
			case 1: return joints[ 0 ];
			default: return joints[ idx ];
		}
	};

	if( debugLines )
	{
		ae::Matrix4 debugTarget = debugModelToWorld * targetTransform;
		debugTarget.SetScale( debugJointScale );
		debugLines->AddOBB( debugTarget, ae::Color::Red() );
	}

	// (a) The initial configuration of the manipulator and the target
	ae::Array< IKBone > bones( tag, chain.Length() );
	for( uint32_t i = 0; i < chain.Length(); i++ )
	{
		const Bone* bindBone = bindPose->GetBoneByIndex( chain[ i ] );
		const Bone* currentBone = pose.GetBoneByIndex( chain[ i ] );
		AE_ASSERT( currentBone->parent );
		AE_ASSERT( bindBone->parent );
		IKBone ikBone;
		ikBone.modelPos = currentBone->modelToBone.GetTranslation();
		ikBone.modelToBoneRot = currentBone->modelToBone.GetRotation();
		ikBone.length = ( bindBone->modelToBone.GetTranslation() - bindBone->parent->modelToBone.GetTranslation() ).Length();

		ae::Quaternion twist;
		float twistAngle = 0.0f;
		const ae::Vec3 twistAxis = GetAxisVector( GetConstraints( i ).twistAxis );
		const ae::Quaternion bindRot = bindBone->modelToBone.GetRotation().RelativeCopy( bindBone->parent->modelToBone.GetRotation() );
		bindRot.GetTwistSwing( twistAxis, &twist, nullptr );
		twist.GetAxisAngle( nullptr, &ikBone.defaultTwist );
		
		bones.Append( ikBone );
	}

	const ae::Vec3 rootPos = bones[ 0 ].modelPos;
	const ae::Vec3 targetPos = targetTransform.GetTranslation();
	const ae::Quaternion targetRot = targetTransform.GetRotation();

	uint32_t iters = 0;
	// Always allow one iteration when debugging to see rotation limits
	while( ( iters == 0 && debugLines ) || ( ( bones[ bones.Length() - 1 ].modelPos - targetPos ).Length() > 0.001f && iters < iterationCount ) )
	{
		// Start from end and iterate to root to move toward target
		// (b) relocate and reorient joint p4 to target t
		bones[ bones.Length() - 1 ].modelPos = targetPos;
		bones[ bones.Length() - 1 ].modelToBoneRot = targetRot;
		for( int32_t i = bones.Length() - 2; i >= 0; i-- )
		{
			IKBone* parentBone = i ? &bones[ i - 1 ] : nullptr;
			IKBone* currentBone = &bones[ i ];
			IKBone* childBone = &bones[ i + 1 ];
			const ae::IKConstraints& currentConstraints = GetConstraints( i );
			const ae::IKConstraints& childConstraints = GetConstraints( i + 1 );
			const ae::Vec3 currentPrimaryAxis = GetAxisVector( currentConstraints.twistAxis );

			// (c) move joint p0 to p0', which lies on the line that passes through the points p1' and p0 and has distance d0 from p1'
			currentBone->modelPos = childBone->modelPos + ( currentBone->modelPos - childBone->modelPos ).SafeNormalizeCopy() * childBone->length;

			if( parentBone )
			{
				if( enableRotationLimits )
				{
					// (d) reorient joint p0' in such a way that the rotor expressing the rotation between the orientation frames at joints p0' and p1' is within the motion range bounds
					// Decompose rotation into twist and swing so twist can be limited
					const ae::Quaternion relative0 = childBone->modelToBoneRot.RelativeCopy( currentBone->modelToBoneRot );
					ae::Quaternion twistRot0;
					ae::Quaternion swingRot;
					relative0.GetTwistSwing( currentPrimaryAxis, &twistRot0, &swingRot );
					float twistAngle = 0.0f;
					twistRot0.GetAxisAngle( nullptr, &twistAngle );

					twistAngle -= currentBone->defaultTwist;
					twistAngle = ae::Clip( twistAngle, currentConstraints.twistLimits[ 0 ], currentConstraints.twistLimits[ 1 ] );
					twistAngle += currentBone->defaultTwist;

					const ae::Quaternion twistRot1 = ae::Quaternion( currentPrimaryAxis, twistAngle );
					const ae::Quaternion relative1 = swingRot * twistRot1;
					currentBone->modelToBoneRot = parentBone->modelToBoneRot * relative1;

					// (e) the rotational constraints: the allowed regions shown as a shaded composite ellipsoidal shape
					currentBone->modelPos -= ClipJoint(
						childBone->length,
						currentBone->modelPos,
						parentBone->modelToBoneRot,
						childBone->modelPos,
						currentConstraints,
						ae::Color::Magenta()
					);
				}
				
				// Reorient current joint to point toward child joint
				const ae::Vec3 dir = ( childBone->modelPos - currentBone->modelPos ).SafeNormalizeCopy();
				const ae::Quaternion invRot = currentBone->modelToBoneRot.GetInverse();
				const ae::Vec3 boneDir = invRot.Rotate( dir );
				const ae::Vec3 axis = currentPrimaryAxis.Cross( boneDir );
				const float angle = boneDir.GetAngleBetween( currentPrimaryAxis );
				currentBone->modelToBoneRot *= ae::Quaternion( axis, angle );
			}
			currentBone->modelPos = childBone->modelPos + currentBone->modelToBoneRot.Rotate( currentPrimaryAxis ) * -childBone->length;

			// (h) reorient the joint p-1' in order to satisfy the orientation limits

			if( debugLines )
			{
				debugLines->AddLine(
					debugModelToWorld.TransformPoint3x4( childBone->modelPos ),
					debugModelToWorld.TransformPoint3x4( currentBone->modelPos ),
					ae::Color::Magenta()
				);
			}
		}
		
		// Iterate from root to reposition joints
		bones[ 0 ].modelPos = rootPos;
		for( uint32_t i = 0; i < bones.Length() - 1; i++ )
		{
			IKBone* parentBone = i ? &bones[ i - 1 ] : nullptr;
			IKBone* currentBone = &bones[ i ];
			IKBone* childBone = &bones[ i + 1 ];
			const ae::IKConstraints& currentConstraints = GetConstraints( i );
			const ae::IKConstraints& childConstraints = GetConstraints( i + 1 );
			const ae::Vec3 currentPrimaryAxis = GetAxisVector( currentConstraints.twistAxis );

			// (c) move joint p0 to p0', which lies on the line that passes through the points p1' and p0 and has distance d0 from p1'
			childBone->modelPos = currentBone->modelPos + ( childBone->modelPos - currentBone->modelPos ).SafeNormalizeCopy() * childBone->length;

			if( parentBone )
			{
				if( enableRotationLimits )
				{
					// (d) reorient joint p0' in such a way that the rotor expressing the rotation between the orientation frames at joints p0' and p1' is within the motion range bounds
					// Decompose rotation into twist and swing so twist can be limited
					const ae::Quaternion relative0 = currentBone->modelToBoneRot.RelativeCopy( parentBone->modelToBoneRot );
					ae::Quaternion twistRot0;
					ae::Quaternion swingRot;
					relative0.GetTwistSwing( currentPrimaryAxis, &twistRot0, &swingRot );
					float twistAngle = 0.0f;
					twistRot0.GetAxisAngle( nullptr, &twistAngle );

					twistAngle -= currentBone->defaultTwist;
					twistAngle = ae::Clip( twistAngle, currentConstraints.twistLimits[ 0 ], currentConstraints.twistLimits[ 1 ] );
					twistAngle += currentBone->defaultTwist;

					const ae::Quaternion twistRot1 = ae::Quaternion( currentPrimaryAxis, twistAngle );
					const ae::Quaternion relative1 = swingRot * twistRot1;
					currentBone->modelToBoneRot = parentBone->modelToBoneRot * relative1;
					// Move the child bone based on its parents new rotation, so
					// that it doesn't "move" in local space
					childBone->modelPos = currentBone->modelPos + currentBone->modelToBoneRot.Rotate( currentPrimaryAxis ) * childBone->length;

					// // (e) the rotational constraints: the allowed regions shown as a shaded composite ellipsoidal shape
					const ae::Quaternion currentBindModelToBone = bindPose->GetBoneByIndex( chain[ i ] )->modelToBone.GetRotation();
					const ae::Quaternion parentBindModelToBone = bindPose->GetBoneByIndex( chain[ i - 1 ] )->modelToBone.GetRotation();
					const ae::Quaternion bindRelative = currentBindModelToBone.RelativeCopy( parentBindModelToBone );
					childBone->modelPos += ClipJoint(
						childBone->length,
						currentBone->modelPos,
						( parentBone->modelToBoneRot * bindRelative ),
						childBone->modelPos,
						currentConstraints,
						ae::Color::PicoPink()
					);
				}

				// Reorient current joint to point toward child joint
				const ae::Vec3 dir = ( childBone->modelPos - currentBone->modelPos ).SafeNormalizeCopy();
				const ae::Quaternion invRot = currentBone->modelToBoneRot.GetInverse();
				const ae::Vec3 boneDir = invRot.Rotate( dir );
				const ae::Vec3 axis = currentPrimaryAxis.Cross( boneDir );
				const float angle = boneDir.GetAngleBetween( currentPrimaryAxis );
				currentBone->modelToBoneRot *= ae::Quaternion( axis, angle );
			}
			childBone->modelPos = currentBone->modelPos + currentBone->modelToBoneRot.Rotate( currentPrimaryAxis ) * childBone->length;

			// (h) reorient the joint p-1' in order to satisfy the orientation limits

			if( debugLines )
			{
				debugLines->AddLine(
					debugModelToWorld.TransformPoint3x4( childBone->modelPos ),
					debugModelToWorld.TransformPoint3x4( currentBone->modelPos ),
					ae::Color::PicoPink()
				);
			}
		}
		
		iters++;
	}

	poseOut->Initialize( &pose );
	ae::Array< const ae::Bone* > outBones( tag, bones.Length() );
	ae::Array< ae::Matrix4 > outTransforms( tag, bones.Length() );
	AE_ASSERT( chain.Length() == bones.Length() );
	for( uint32_t i = 0; i < chain.Length(); i++ )
	{
		const uint32_t idx = chain[ i ];
		const IKBone& ikBone = bones[ i ];

		outBones.Append( poseOut->GetBoneByIndex( idx ) );
		
		ae::Matrix4 modelToBone = ikBone.modelToBoneRot.GetTransformMatrix();
		modelToBone.SetTranslation( ikBone.modelPos );
		outTransforms.Append( modelToBone );
	}

	// Don't apply any results when iteration is disabled, only display debug
	// info. Skip debug lines also, there are no results.
	if( iterationCount )
	{
		ae::Matrix4* finalTransform = &outTransforms[ outTransforms.Length() - 1 ];
		*finalTransform = targetTransform;
		// @TODO: Maintain the old bones scale
		finalTransform->SetTranslation( bones[ bones.Length() - 1 ].modelPos );

		for( uint32_t i = 0; i < chain.Length(); i++ )
		{
			poseOut->SetTransform( poseOut->GetBoneByIndex( chain[ i ] ), outTransforms[ i ] );
			if( debugLines )
			{
				ae::Matrix4 worldTransform = debugModelToWorld * outTransforms[ i ];
				worldTransform.SetScale( debugJointScale );
				debugLines->AddLine(
					worldTransform.GetTranslation(),
					worldTransform.GetTranslation() + worldTransform.GetAxis( 0 ),
					ae::Color::Red()
				);
				debugLines->AddLine(
					worldTransform.GetTranslation(),
					worldTransform.GetTranslation() + worldTransform.GetAxis( 1 ),
					ae::Color::Green()
				);
				debugLines->AddLine(
					worldTransform.GetTranslation(),
					worldTransform.GetTranslation() + worldTransform.GetAxis( 2 ),
					ae::Color::Blue()
				);
				debugLines->AddOBB( worldTransform, ae::Color::Yellow() ); 
			}
		}
	}
}

//------------------------------------------------------------------------------
// ae::Skin member functions
//------------------------------------------------------------------------------
void Skin::Initialize( const Skeleton& bindPose, const ae::Skin::Vertex* vertices, uint32_t vertexCount )
{
	AE_ASSERT( bindPose.GetBoneCount() );
	m_bindPose.Initialize( &bindPose );
	
	m_verts.Clear();
	m_verts.AppendArray( vertices, vertexCount );
}

const Skeleton& Skin::GetBindPose() const
{
	return m_bindPose;
}

void Skin::ApplyPoseToMesh( const Skeleton* pose, float* positionsOut, float* normalsOut, uint32_t positionStride, uint32_t normalStride, bool positionsW, bool normalsW, uint32_t count ) const
{
	AE_ASSERT_MSG( count == m_verts.Length(), "Given mesh data does not match skin vertex count" );
	AE_ASSERT_MSG( m_bindPose.GetBoneCount() == pose->GetBoneCount(), "Given ae::Skeleton pose does not match bind pose hierarchy" );
	
	const uint32_t boneCount = pose->GetBoneCount();
	ae::Scratch< ae::Matrix4 > tempBones( boneCount );
	ae::Scratch< ae::Matrix4 > tempBoneNorm( boneCount );
	for( uint32_t i = 0; i < boneCount; i++ )
	{
		const ae::Bone* bone = pose->GetBoneByIndex( i );
		const ae::Bone* bindPoseBone = m_bindPose.GetBoneByIndex( i );
		if( bone->parent ) { AE_ASSERT_MSG( bone->parent->index == bindPoseBone->parent->index, "Given ae::Skeleton pose does not match bind pose hierarchy" ); }
		else { AE_ASSERT_MSG( !bindPoseBone->parent, "Given ae::Skeleton pose does not match bind pose hierarchy" ); }
		
		ae::Matrix4 transform = bone->modelToBone * bindPoseBone->boneToModel;
		tempBones[ i ] = transform;
		tempBoneNorm[ i ] = transform.GetNormalMatrix();
	}
	
	for( uint32_t i = 0; i < count; i++ )
	{
		ae::Vec3 pos( 0.0f );
		ae::Vec3 normal( 0.0f );
		const ae::Skin::Vertex& skinVert = m_verts[ i ];
		for( uint32_t j = 0; j < 4; j++ )
		{
			const float weight = skinVert.weights[ j ] / 255.0f;
			pos += ( tempBones[ skinVert.bones[ j ] ] * ae::Vec4( skinVert.position, 1.0f ) ).GetXYZ() * weight;
			normal += ( tempBoneNorm[ skinVert.bones[ j ] ] * ae::Vec4( skinVert.normal, 0.0f ) ).GetXYZ() * weight;
		}
		normal.SafeNormalize();
		
		float* p = (float*)( (uint8_t*)positionsOut + ( i * positionStride ) );
		float* n = (float*)( (uint8_t*)normalsOut + ( i * normalStride ) );
		p[ 0 ] = pos.x;
		p[ 1 ] = pos.y;
		p[ 2 ] = pos.z;
		if( positionsW ) { p[ 3 ] = 1.0f; }
		n[ 0 ] = normal.x;
		n[ 1 ] = normal.y;
		n[ 2 ] = normal.z;
		if( normalsW ) { n[ 3 ] = 0.0f; }
	}
}

//------------------------------------------------------------------------------
// ae::OBJLoader member functions
//------------------------------------------------------------------------------
bool OBJLoader::Load( ae::OBJLoader::InitializeParams params )
{
	vertices.Clear();
	indices.Clear();
	aabb = ae::AABB();
	
	enum class Mode
	{
		None,
		Vertex,
		Texture,
		Normal,
		Face
	};
	struct FaceIndex
	{
		int32_t position = -1;
		int32_t texture = -1;
		int32_t normal = -1;
	};
	ae::Array< ae::Vec4 > positions = allocTag;
	ae::Array< ae::Vec2 > uvs = allocTag;
	ae::Array< ae::Vec4 > normals = allocTag;
	ae::Array< FaceIndex > faceIndices = allocTag;
	ae::Array< uint8_t > faces = allocTag;
	
	const bool hasWorldTransform = ( params.localToWorld != ae::Matrix4::Identity() );
	const char* data = (const char*)params.data;
	const char* dataEnd = (const char*)params.data + params.length;
	while( data < dataEnd )
	{
		uint32_t lineLen = 0;
		while( data[ lineLen ] && data[ lineLen ] != '\n' && data[ lineLen ] != '\r' && ( data + lineLen < dataEnd ) )
		{
			lineLen++;
		}
		const char* line = data;
		const char* lineEnd = line + lineLen;
		data += lineLen;
		while( ( !data[ 0 ] || data[ 0 ] == '\n' || data[ 0 ] == '\r' ) && data < dataEnd )
		{
			data++;
		}
		AE_ASSERT( line <= lineEnd );
		AE_ASSERT( data <= dataEnd );
		AE_ASSERT( lineEnd <= dataEnd );
		
		Mode mode = Mode::None;
		switch( line[ 0 ] )
		{
			case 'v':
				switch( line[ 1 ] )
				{
					case ' ':
						mode = Mode::Vertex;
						break;
					case 't':
						mode = Mode::Texture;
						line++;
						break;
					case 'n':
						mode = Mode::Normal;
						line++;
						break;
				}
				break;
			case 'f':
				mode = Mode::Face;
				break;
			default:
				break; // Ignore bad chars
		}
		line++;
		if( line[ 0 ] != ' ' )
		{
			// Unknown line tag
			mode = Mode::None;
		}
		
		// @NOTE: strtof() takes a non-const string but does not modify it
		switch( mode )
		{
			case Mode::Vertex:
			{
				ae::Vec4 p;
				p.x = strtof( line, (char**)&line );
				p.y = strtof( line, (char**)&line );
				p.z = strtof( line, (char**)&line );
				p.w = 1.0f;
				if( hasWorldTransform )
				{
					p = params.localToWorld * p;
				}
				// @TODO: Unofficially OBJ can list 3 extra (0-1) values here representing vertex R,G,B values
				positions.Append( p );
				aabb.Expand( p.GetXYZ() );
				break;
			}
			case Mode::Texture:
			{
				ae::Vec2 uv;
				uv.x = strtof( line, (char**)&line );
				uv.y = strtof( line, (char**)&line );
				uvs.Append( uv );
				break;
			}
			case Mode::Normal:
			{
				ae::Vec4 n;
				n.x = strtof( line, (char**)&line );
				n.y = strtof( line, (char**)&line );
				n.z = strtof( line, (char**)&line );
				n.w = 0.0f;
				if( hasWorldTransform )
				{
					n = params.localToWorld.GetNormalMatrix() * n;
				}
				normals.Append( n.SafeNormalizeCopy() );
				break;
			}
			case Mode::Face:
			{
				uint32_t faceVertexCount = 0;
				while( line < lineEnd )
				{
					FaceIndex faceIndex;
					faceIndex.position = (int32_t)strtoul( line, (char**)&line, 10 ) - 1;
					if( line[ 0 ] == '/' )
					{
						line++;
						if( line[ 0 ] != '/' )
						{
							faceIndex.texture = (int32_t)strtoul( line, (char**)&line, 10 ) - 1;
						}
					}
					if( line[ 0 ] == '/' )
					{
						line++;
						faceIndex.normal = (int32_t)strtoul( line, (char**)&line, 10 ) - 1;
					}
					if( faceIndex.position < 0 )
					{
						break;
					}
					
					faceIndices.Append( faceIndex );
					faceVertexCount++;

					while( isspace( line[ 0 ] ) && line < lineEnd )
					{
						line++;
					}
				}
				faces.Append( (uint8_t)faceVertexCount );
				break;
			}
			default:
				// Ignore line
				break;
		}
	}

	if( !positions.Length() || !faceIndices.Length() )
	{
		return false;
	}

	// @TODO: Reserve vertices and indices
	
	FaceIndex* currentFaceIdx = &faceIndices[ 0 ];
	ae::Map< ae::Int3, uint32_t > vertexMap = allocTag;
	for( uint8_t f : faces )
	{
		if( f <= 2 ) { continue; } // Invalid face
		
		// Triangulate faces
		uint32_t triCount = ( f - 2 );
		for( uint32_t i = 0; i < triCount; i++ )
		{
			FaceIndex tri[ 3 ];
			tri[ 0 ] = currentFaceIdx[ 0 ];
			tri[ 1 ] = currentFaceIdx[ i + 1 ];
			tri[ 2 ] = currentFaceIdx[ i + 2 ];
			if( params.flipWinding )
			{
				std::swap( tri[ 1 ], tri[ 2 ] );
			}
			for( uint32_t j = 0; j < 3; j++ )
			{
				int posIdx = tri[ j ].position;
				int uvIdx = tri[ j ].texture;
				int normIdx = tri[ j ].normal;
				ae::Int3 key( posIdx, uvIdx, normIdx );
				uint32_t* existingIndex = vertexMap.TryGet( key );
				if( existingIndex )
				{
					indices.Append( *existingIndex );
				}
				else
				{
					Vertex vertex;
					vertex.position = positions[ posIdx ];
					vertex.texture = ( uvIdx >= 0 ? uvs[ uvIdx ] : ae::Vec2( 0.0f ) );
					vertex.normal = ( normIdx >= 0 ? normals[ normIdx ] : ae::Vec4( 0.0f ) );
					vertex.color = ae::Vec4( 1.0f, 1.0f );
					vertexMap.Set( key, vertices.Length() );
					indices.Append( vertices.Length() );
					vertices.Append( vertex );
				}
			}
		}
		
		currentFaceIdx += f;
	}
	
	return true;
}

void OBJLoader::InitializeVertexData( const ae::OBJLoader::VertexDataParams& params )
{
	if( !params.vertexData )
	{
		return;
	}

	params.vertexData->Initialize(
		sizeof(*vertices.Data()), sizeof(*indices.Data()),
		vertices.Length(), indices.Length(),
		ae::Vertex::Primitive::Triangle,
		ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	params.vertexData->AddAttribute( params.posAttrib, 4, ae::Vertex::Type::Float, offsetof( Vertex, position ) );
	params.vertexData->AddAttribute( params.uvAttrib, 2, ae::Vertex::Type::Float, offsetof( Vertex, texture ) );
	params.vertexData->AddAttribute( params.normalAttrib, 4, ae::Vertex::Type::Float, offsetof( Vertex, normal ) );
	params.vertexData->AddAttribute( params.colorAttrib, 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
	params.vertexData->UploadVertices( 0, vertices.Data(), vertices.Length() );
	params.vertexData->UploadIndices( 0, indices.Data(), indices.Length() );
}

//------------------------------------------------------------------------------
// ae::TargaFile member functions
//------------------------------------------------------------------------------
bool TargaFile::Load( const uint8_t* data, uint32_t length )
{
	m_data.Clear();
	if( !length )
	{
		return false;
	}

	AE_PACK( struct TargaHeader
	{
		uint8_t idLength;
		uint8_t colorMapType;
		uint8_t imageType;

		uint16_t colorMapOrigin;
		uint16_t colorMapLength;
		uint8_t colorMapDepth;

		uint16_t xOrigin;
		uint16_t yOrigin;
		uint16_t width;
		uint16_t height;

		uint8_t bitsPerPixel;
		uint8_t imageDescriptor;
	} );

	ae::BinaryReader rStream( data, length );
	TargaHeader header;
	rStream.SerializeRaw( &header, sizeof(header) );
	AE_ASSERT_MSG( header.imageType == 2 || header.imageType == 3, "Targa image type '#' is not supported", (int)header.imageType );
	AE_ASSERT_MSG( !header.colorMapLength, "Targa color map is not supported" );
	AE_ASSERT_MSG( !header.xOrigin && !header.yOrigin, "Targa non-zero origin is not supported" );
	AE_ASSERT_MSG( header.bitsPerPixel == 8 || header.bitsPerPixel == 24 || header.bitsPerPixel == 32, "Targa bit depth is unsupported" );
	AE_ASSERT_MSG( header.bitsPerPixel != 32 || header.imageDescriptor == 8, "Alpha mode not supported" );

	rStream.DiscardReadData( header.idLength );
	rStream.DiscardReadData( header.colorMapLength );

	const uint8_t* pixels = rStream.GetData() + rStream.GetOffset();
	uint32_t dataLength = header.width * header.height * ( header.bitsPerPixel / 8 );
	AE_ASSERT( rStream.GetRemainingBytes() >= dataLength );
	m_data.AppendArray( pixels, dataLength );
	textureParams.data = m_data.Data();
	textureParams.width = header.width;
	textureParams.height = header.height;
	if( header.bitsPerPixel == 32 )
	{
		textureParams.format = ae::Texture::Format::RGBA8_SRGB;
	}
	else if( header.bitsPerPixel == 24 )
	{
		textureParams.format = ae::Texture::Format::RGB8_SRGB;
	}
	else
	{
		textureParams.format = ae::Texture::Format::R8;
	}
	textureParams.bgrData = true;

	return true;
}

//------------------------------------------------------------------------------
// ae::Audio Helpers
//------------------------------------------------------------------------------
void _CheckALError()
{
#if AE_USE_OPENAL
	const char* errStr = "UNKNOWN_ERROR";
	switch( alGetError() )
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
#endif
}

void _LoadWavFile( const uint8_t* fileBuffer, uint32_t fileSize, uint32_t* bufferOut, float* lengthOut )
{
#if AE_USE_OPENAL
	struct ChunkHeader
	{
		char chunkId[ 4 ];
		uint32_t chunkSize;
	};

	struct RiffChunk
	{
		char waveId[ 4 ];
	};
	
	struct FormatChunk
	{
		uint16_t formatCode;
		uint16_t numChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
	};

	ChunkHeader header;
	FormatChunk waveFormat;
	bool hasReadFormat = false;
	uint32_t dataSize = 0;
	uint32_t fileOffset = 0;

	// Some wav files have weird chunk sizes, so the entire length of the file might not be used
	while( ( fileSize - fileOffset ) >= sizeof(header) )
	{
		memcpy( &header, fileBuffer + fileOffset, sizeof(header) );
		fileOffset += sizeof(header);
		AE_ASSERT( ( fileSize - fileOffset ) >= header.chunkSize );
		
		if( memcmp( header.chunkId, "RIFF", 4 ) == 0 )
		{
			RiffChunk riff;
			memcpy( &riff, fileBuffer + fileOffset, sizeof(riff) );
			AE_ASSERT( memcmp( riff.waveId, "WAVE", 4 ) == 0 );
			fileOffset += sizeof(riff);
		}
		else
		{
			if( memcmp( header.chunkId, "fmt ", 4 ) == 0 )
			{
				AE_ASSERT( header.chunkSize >= sizeof(FormatChunk) );
				memcpy( &waveFormat, fileBuffer + fileOffset, sizeof(FormatChunk) );
				hasReadFormat = true;
			}
			else if( memcmp( header.chunkId, "data", 4 ) == 0 )
			{
				AE_ASSERT( hasReadFormat );
				AE_ASSERT_MSG( dataSize == 0, "Combining WAV data chunks is currently not supported" );
				
				const uint8_t* data = ( fileBuffer + fileOffset );
				ALsizei size = header.chunkSize;
				ALsizei frequency = waveFormat.sampleRate;
				dataSize = size;
				
				ALenum format = 0;
				if( waveFormat.numChannels == 1 )
				{
					if( waveFormat.bitsPerSample == 8 ) { format = AL_FORMAT_MONO8; }
					else if( waveFormat.bitsPerSample == 16 ) { format = AL_FORMAT_MONO16; }
					else { AE_FAIL_MSG( "Unsupported WAV bits per sample: #", waveFormat.bitsPerSample ); }
				}
				else if( waveFormat.numChannels == 2 )
				{
					if( waveFormat.bitsPerSample == 8 ) { format = AL_FORMAT_STEREO8; }
					else if( waveFormat.bitsPerSample == 16 ) { format = AL_FORMAT_STEREO16; }
					else { AE_FAIL_MSG( "Unsupported WAV bits per sample: #", waveFormat.bitsPerSample ); }
				}
				else
				{
					AE_FAIL_MSG( "Unsupported WAV channel count: #", waveFormat.numChannels );
				}
				
				alGenBuffers( 1, bufferOut );
				alBufferData( *bufferOut, format, (void*)data, size, frequency );
				_CheckALError();
			}
			
			fileOffset += header.chunkSize;
		}
	}

	AE_ASSERT( hasReadFormat );
	AE_ASSERT( dataSize );
	*lengthOut = dataSize / ( waveFormat.sampleRate * waveFormat.numChannels * waveFormat.bitsPerSample / 8.0f );
#endif
}

//------------------------------------------------------------------------------
// ae::AudioData member functions
//------------------------------------------------------------------------------
AudioData::AudioData()
{
	buffer = 0;
	length = 0.0f;
}

const AudioData* Audio::LoadWavFile( const uint8_t* data, uint32_t length )
{
	if( m_audioDatas.Length() >= m_maxAudioDatas )
	{
		return nullptr;
	}
	
	uint32_t buffer = 0;
	float duration = 0.0f;
	_LoadWavFile( data, length, &buffer, &duration );
	if( buffer )
	{
		AudioData* audioData = &m_audioDatas.Append( {} );
		audioData->buffer = buffer;
		audioData->length = duration;
		return audioData;
	}
	return nullptr;
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
void Audio::Initialize( uint32_t musicChannels, uint32_t sfxChannels, uint32_t sfxLoopChannels, uint32_t maxAudioDatas )
{
#if AE_USE_OPENAL
	ALCdevice* device = alcOpenDevice( nullptr );
	AE_ASSERT( device );
	ALCcontext* ctx = alcCreateContext( device, nullptr );
	alcMakeContextCurrent( ctx );
	AE_ASSERT( ctx );
	_CheckALError();
	
	m_maxAudioDatas = maxAudioDatas;
	m_audioDatas.Reserve( m_maxAudioDatas );
	
	ae::Array< ALuint > sources( AE_ALLOC_TAG_AUDIO, 0, musicChannels + sfxChannels + sfxLoopChannels );
	alGenSources( (ALuint)sources.Length(), sources.Data() );

	m_musicChannels.Reserve( musicChannels );
	for( uint32_t i = 0; i < musicChannels; i++ )
	{
		Channel* channel = &m_musicChannels.Append( Channel() );
		channel->source = sources[ i ];
		alGenSources( (ALuint)1, &channel->source );
		alSourcef( channel->source, AL_PITCH, 1 );
		alSourcef( channel->source, AL_GAIN, 1.0f );
		alSourcef( channel->source, AL_MIN_GAIN, 0.f );
		alSource3f( channel->source, AL_POSITION, 0, 0, 0 );
		alSourcei( channel->source, AL_LOOPING, AL_TRUE );
	}

	m_sfxChannels.Reserve( sfxChannels );
	for( uint32_t i = 0; i < sfxChannels; i++ )
	{
		Channel* channel = &m_sfxChannels.Append( Channel() );
		channel->source = sources[ musicChannels + i ];
		alSourcef( channel->source, AL_PITCH, 1 );
		alSourcef( channel->source, AL_GAIN, 1.0f );
		alSourcef( channel->source, AL_MIN_GAIN, 0.f );
		alSource3f( channel->source, AL_POSITION, 0, 0, 0 );
		alSourcei( channel->source, AL_LOOPING, AL_FALSE );
	}
	
	m_sfxLoopChannels.Reserve( sfxLoopChannels );
	for( uint32_t i = 0; i < sfxLoopChannels; i++ )
	{
		Channel* channel = &m_sfxLoopChannels.Append( Channel() );
		channel->source = sources[ musicChannels + sfxChannels + i ];
		alSourcef( channel->source, AL_PITCH, 1 );
		alSourcef( channel->source, AL_GAIN, 1.0f );
		alSourcef( channel->source, AL_MIN_GAIN, 0.f );
		alSource3f( channel->source, AL_POSITION, 0, 0, 0 );
		alSourcei( channel->source, AL_LOOPING, AL_TRUE );
	}

	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	alListener3f( AL_POSITION, 0, 0, 1.0f );
	alListenerfv( AL_ORIENTATION, listenerOri );
	alListenerf( AL_GAIN, 1.0f );

	_CheckALError();
#endif
}

void Audio::Terminate()
{
#if AE_USE_OPENAL
	for( uint32_t i = 0; i < m_musicChannels.Length(); i++ )
	{
		Channel* channel = &m_musicChannels[ i ];
		alDeleteSources( 1, &channel->source );
		channel->source = -1;
	}

	for( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
	{
		Channel* channel = &m_sfxChannels[ i ];
		alDeleteSources( 1, &channel->source );
		channel->source = -1;
	}
	
	for( uint32_t i = 0; i < m_sfxLoopChannels.Length(); i++ )
	{
		Channel* channel = &m_sfxLoopChannels[ i ];
		alDeleteSources( 1, &channel->source );
		channel->source = -1;
	}
	
	// Unload buffers after channels incase they are referenced
	for( AudioData& audioData : m_audioDatas )
	{
		alDeleteBuffers( 1, &audioData.buffer );
	}

	ALCcontext* ctx = alcGetCurrentContext();
	ALCdevice* device = alcGetContextsDevice( ctx );
	alcMakeContextCurrent( nullptr );
	alcDestroyContext( ctx );
	alcCloseDevice( device );
#endif
}

void Audio::SetVolume( float volume )
{
#if AE_USE_OPENAL
	volume = ae::Clip01( volume );
	alListenerf( AL_GAIN, volume );
#endif
}

void Audio::SetMusicVolume( float volume, uint32_t channel )
{
#if AE_USE_OPENAL
	if( channel >= m_musicChannels.Length() )
	{
		return;
	}

	Channel* musicChannel = &m_musicChannels[ channel ];
	alSourcef( musicChannel->source, AL_GAIN, volume );
#endif
}

void Audio::SetSfxLoopVolume( float volume, uint32_t channel )
{
#if AE_USE_OPENAL
	if( channel >= m_sfxLoopChannels.Length() )
	{
		return;
	}

	Channel* sfxLoopChannel = &m_sfxLoopChannels[ channel ];
	alSourcef( sfxLoopChannel->source, AL_GAIN, volume );
#endif
}

void Audio::PlayMusic( const AudioData* audioFile, float volume, uint32_t channel )
{
#if AE_USE_OPENAL
	AE_ASSERT( audioFile );
	if( channel >= m_musicChannels.Length() )
	{
		return;
	}

	Channel* musicChannel = &m_musicChannels[ channel ];

	ALint state;
	alGetSourcei( musicChannel->source, AL_SOURCE_STATE, &state );
	if( ( audioFile == musicChannel->resource ) && state == AL_PLAYING )
	{
		return;
	}

	if( state == AL_PLAYING )
	{
		alSourceStop( musicChannel->source );
	}
	
	musicChannel->resource = audioFile;

	alSourcei( musicChannel->source, AL_BUFFER, audioFile->buffer );
	alSourcef( musicChannel->source, AL_GAIN, volume );
	alSourcePlay( musicChannel->source );
	_CheckALError();
#endif
}

void Audio::PlaySfx( const AudioData* audioFile, float volume, int32_t priority )
{
#if AE_USE_OPENAL
	ALint state;
	AE_ASSERT( audioFile );

	Channel* leastPriorityChannel = nullptr;
	Channel* unusedChannel = nullptr;
	for( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
	{
		Channel* sfxChannel = &m_sfxChannels[ i ];
		if( !leastPriorityChannel || sfxChannel->priority >= leastPriorityChannel->priority )
		{
			leastPriorityChannel = sfxChannel;
		}

		if( !unusedChannel )
		{
			alGetSourcei( sfxChannel->source, AL_SOURCE_STATE, &state );
			if( state != AL_PLAYING )
			{
				unusedChannel = sfxChannel;
			}
		}
	}
	AE_ASSERT( leastPriorityChannel );

	Channel* currentChannel = nullptr;
	if( unusedChannel )
	{
		currentChannel = unusedChannel;
	}
	else if( !leastPriorityChannel || leastPriorityChannel->priority < priority )
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
#endif
}

void Audio::PlaySfxLoop( const AudioData* audioFile, float volume, uint32_t channel )
{
#if AE_USE_OPENAL
	AE_ASSERT( audioFile );
	if( channel >= m_sfxLoopChannels.Length() )
	{
		return;
	}

	Channel* sfxLoopChannel = &m_sfxLoopChannels[ channel ];

	ALint state;
	alGetSourcei( sfxLoopChannel->source, AL_SOURCE_STATE, &state );
	if( ( audioFile == sfxLoopChannel->resource ) && state == AL_PLAYING )
	{
		alSourcef( sfxLoopChannel->source, AL_GAIN, volume );
		return;
	}

	if( state == AL_PLAYING )
	{
		alSourceStop( sfxLoopChannel->source );
	}
	
	sfxLoopChannel->resource = audioFile;

	alSourcei( sfxLoopChannel->source, AL_BUFFER, audioFile->buffer );
	alSourcei( sfxLoopChannel->source, AL_LOOPING, 1 );
	alSourcef( sfxLoopChannel->source, AL_GAIN, volume );
	alSourcePlay( sfxLoopChannel->source );
	_CheckALError();
#endif
}

void Audio::StopMusic( uint32_t channel )
{
#if AE_USE_OPENAL
	if( channel < m_musicChannels.Length() )
	{
		alSourceStop( m_musicChannels[ channel ].source );
		m_musicChannels[ channel ].resource = nullptr;
	}
#endif
}

void Audio::StopSfxLoop( uint32_t channel )
{
#if AE_USE_OPENAL
	if( channel < m_sfxLoopChannels.Length() )
	{
		alSourceStop( m_sfxLoopChannels[ channel ].source );
		m_sfxLoopChannels[ channel ].resource = nullptr;
	}
#endif
}

void Audio::StopAllSfx()
{
#if AE_USE_OPENAL
	for( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
	{
		alSourceStop( m_sfxChannels[ i ].source );
		m_sfxChannels[ i ].resource = nullptr;
	}
#endif
}

void Audio::StopAllSfxLoops()
{
#if AE_USE_OPENAL
	for( uint32_t i = 0; i < m_sfxLoopChannels.Length(); i++ )
	{
		alSourceStop( m_sfxLoopChannels[ i ].source );
		m_sfxLoopChannels[ i ].resource = nullptr;
	}
#endif
}

uint32_t Audio::GetMusicChannelCount() const
{
	return m_musicChannels.Length();
}

uint32_t Audio::GetSfxChannelCount() const
{
	return m_sfxChannels.Length();
}

uint32_t Audio::GetSfxLoopChannelCount() const
{
	return m_sfxLoopChannels.Length();
}

// @TODO: Should return a string with current state of audio channels
void Audio::Log()
{
#if AE_USE_OPENAL
	for( uint32_t i = 0; i < m_sfxChannels.Length(); i++ )
	{
		ALint state = 0;
		const Channel* channel = &m_sfxChannels[ i ];
		alGetSourcei( channel->source, AL_SOURCE_STATE, &state );

		if( state == AL_PLAYING )
		{
			AE_ASSERT( channel->resource );

			float playOffset = 0.0f;
			alGetSourcef( channel->source, AL_SEC_OFFSET, &playOffset );

			float playLength = channel->resource->length;

			// const char* soundName = strrchr( , '/' );
			// soundName = soundName ? soundName + 1 : channel->resource->name.c_str();
			// const char* soundNameEnd = strrchr( channel->resource->name.c_str(), '.' );
			// soundNameEnd = soundNameEnd ? soundNameEnd : soundName + strlen( soundName );
			// uint32_t soundNameLen = (uint32_t)(soundNameEnd - soundName);
			const char* soundName = "unknown";
			uint32_t soundNameLen = (uint32_t)strlen( soundName );

			char buffer[ 512 ];
			snprintf( buffer, sizeof(buffer), "channel:%u name:%.*s offset:%.2fs length:%.2fs", i, soundNameLen, soundName, playOffset, playLength );
			AE_LOG( buffer );
		}
	}

	_CheckALError();
#endif
}

//------------------------------------------------------------------------------
// ae::BinaryStream member functions
//------------------------------------------------------------------------------
BinaryStream::BinaryStream( Mode mode, void* data, uint32_t length )
{
	m_mode = mode;
	m_data = static_cast< uint8_t* >( data );
	m_length = length;
	m_isValid = m_data && m_length;
}

BinaryStream::BinaryStream( Mode mode, const void* data, uint32_t length )
{
	AE_ASSERT_MSG( mode == Mode::ReadBuffer, "Only read mode can be used with a constant data buffer." );
	m_mode = mode;
	m_data = const_cast< uint8_t* >( static_cast< const uint8_t* >( data ) );
	m_length = length;
	m_isValid = m_data && m_length;
}

BinaryStream::BinaryStream( Array< uint8_t >* array )
{
	m_mode = Mode::WriteBuffer;
	if( array )
	{
		m_extArray = array;
		m_offset = m_extArray->Length();
		m_length = m_extArray->Size();
		m_isValid = true;
	}
}

BinaryWriter::BinaryWriter( void* data, uint32_t length ) :
	BinaryStream( Mode::WriteBuffer, data, length )
{}

BinaryWriter::BinaryWriter( ae::Array< uint8_t >* array ) :
	BinaryStream( array )
{}

BinaryReader::BinaryReader( const void* data, uint32_t length ) :
	BinaryStream( Mode::ReadBuffer, data, length )
{}

BinaryReader::BinaryReader( const ae::Array< uint8_t >& data ) :
	BinaryStream( Mode::ReadBuffer, data.Data(), data.Length() )
{}

void BinaryStream::SerializeUInt8( uint8_t& v )
{
	SerializeRaw( &v, sizeof(v) );
}

void BinaryWriter::SerializeUInt8( const uint8_t& v )
{
	SerializeRaw( &v, sizeof(v) );
}

void BinaryStream::SerializeUInt16( uint16_t& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryWriter::SerializeUInt16( const uint16_t& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeUInt32( uint32_t& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryWriter::SerializeUInt32( const uint32_t& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeUInt64( uint64_t& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryWriter::SerializeUInt64( const uint64_t& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt8( int8_t& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryWriter::SerializeInt8( const int8_t& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt16( int16_t& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryWriter::SerializeInt16( const int16_t& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt32( int32_t& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryWriter::SerializeInt32( const int32_t& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeInt64( int64_t& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void BinaryWriter::SerializeInt64( const int64_t& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeFloat( float& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void BinaryWriter::SerializeFloat( const float& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeDouble( double& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void BinaryWriter::SerializeDouble( const double& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void BinaryStream::SerializeBool( bool& v )
{
	SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void BinaryWriter::SerializeBool( const bool& v )
{
	SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void BinaryStream::SerializeString( char* strInOut, uint32_t bufferSize )
{
	AE_ASSERT( bufferSize );
	if( ae::BinaryReader* reader = AsReader() )
	{
		if( auto end = (const uint8_t*)memchr( reader->PeekReadData(), '\0', GetRemainingBytes() ) )
		{
			const uint32_t strLength = (uint32_t)( end - reader->PeekReadData() );
			if( strLength < bufferSize )
			{
				SerializeRaw( strInOut, strLength + 1 );
			}
			else
			{
				Invalidate();
			}
		}
		else
		{
			Invalidate();
		}
		if( !IsValid() )
		{
			strInOut[ 0 ] = '\0';
		}
	}
	else if( AsWriter() )
	{
		const uint32_t strLength = (uint32_t)strlen( strInOut );
		if( strLength < bufferSize )
		{
			SerializeRaw( strInOut, strLength + 1 );
		}
		else
		{
			Invalidate();
		}
	}
}

void BinaryWriter::SerializeString( const char* strIn )
{
	const uint32_t strLength = (uint32_t)strlen( strIn );
	SerializeRaw( strIn, strLength + 1 );
}

void BinaryStream::SerializeRaw( void* data, uint32_t length )
{
	if( !m_isValid )
	{
		return;
	}
	else if( m_mode == Mode::ReadBuffer )
	{
		if( m_offset + length <= m_length )
		{
			memcpy( data, m_data + m_offset, length );
			m_offset += length;
		}
		else
		{
			Invalidate();
		}
	}
	else if( m_mode == Mode::WriteBuffer )
	{
		if( m_data )
		{
			if( length <= GetRemainingBytes() )
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
			AE_ASSERT( m_extArray );
			m_extArray->AppendArray( (uint8_t*)data, length );
			m_offset = m_extArray->Length();
			m_length = m_extArray->Size();
		}
	}
}

void BinaryWriter::SerializeRaw( const void* data, uint32_t length )
{
	BinaryStream::SerializeRaw( const_cast< void* >( data ), length );
}

const uint8_t* BinaryReader::PeekReadData() const
{
	return IsValid() ? GetData() + m_offset : nullptr;
}

void BinaryReader::DiscardReadData( uint32_t length )
{
	if( !length || !IsValid() )
	{
		return;
	}
	else if( GetRemainingBytes() < length )
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
	m_data.AppendArray( (const uint8_t*)data, length );
}

// @HACK: Should rearrange file so windows.h is included with as little logic as possible after it
#ifdef SendMessage
#undef SendMessage
#endif
void NetObject::SendMessage( const void* data, uint32_t length )
{
	uint16_t lengthU16 = (uint16_t)length;
	m_messageDataOut.Reserve( m_messageDataOut.Length() + sizeof( lengthU16 ) + length );
	m_messageDataOut.AppendArray( (uint8_t*)&lengthU16, sizeof( lengthU16 ) );
	m_messageDataOut.AppendArray( (const uint8_t*)data, length );
}

void NetObject::SetInitData( const void* initData, uint32_t initDataLength )
{
	m_initData.Clear();
	m_initData.AppendArray( (uint8_t*)initData, initDataLength );
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
	if( m_messageDataInOffset >= m_messageDataIn.Length() )
	{
		AE_ASSERT( m_messageDataInOffset == m_messageDataIn.Length() );
		return false;
	}
	else if( !msgOut )
	{
		// Early out
		return true;
	}

	// Write out incoming message data
	msgOut->length = *(uint16_t*)&m_messageDataIn[ m_messageDataInOffset ];
	m_messageDataInOffset += sizeof( uint16_t );

	msgOut->data = &m_messageDataIn[ m_messageDataInOffset ];
	m_messageDataInOffset += msgOut->length;

	if( m_messageDataInOffset >= m_messageDataIn.Length() )
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
	m_data.AppendArray( data, length );
}

void NetObject::m_ReceiveMessages( const uint8_t* data, uint32_t length )
{
	m_messageDataIn.AppendArray( data, length );
}

void NetObject::m_UpdateHash()
{
	if( m_data.Length() )
	{
		m_hash = ae::Hash32().HashData( &m_data[ 0 ], m_data.Length() ).Get();
	}
	else
	{
		m_hash = 0;
	}
}

//------------------------------------------------------------------------------
// ae::NetObjectClient member functions
//------------------------------------------------------------------------------
NetObjectClient::~NetObjectClient()
{
	AE_ASSERT_MSG( m_netObjects.Length() == m_created.Length(), "Not all ae::NetObjects were cleaned up correctly before destroying ae::NetObjectClient" );
	while( PumpCreate() ) {}
	while( m_netObjects.Length() )
	{
		Destroy( m_netObjects.GetValue( m_netObjects.Length() - 1 ) );
	}
	AE_DEBUG_ASSERT( !m_netObjects.Length() );
	AE_DEBUG_ASSERT( !m_remoteToLocalIdMap.Length() );
	AE_DEBUG_ASSERT( !m_localToRemoteIdMap.Length() );
	AE_DEBUG_ASSERT( !m_created.Length() );
}

void NetObjectClient::ReceiveData( const uint8_t* data, uint32_t length )
{
	ae::BinaryReader rStream( data, length );
	while( rStream.GetOffset() < rStream.GetSize() )
	{
		NetObjectConnection::EventType type;
		rStream.SerializeEnum( type );
		if( !rStream.IsValid() )
		{
			break;
		}
		switch( type )
		{
			case NetObjectConnection::EventType::Connect:
			{
				uint32_t signature = 0;
				rStream.SerializeUInt32( signature );
				AE_ASSERT( signature );

				ae::Map< NetObject*, int > toDestroy = AE_ALLOC_TAG_NET;
				bool allowResolve = ( m_serverSignature == signature );
				if( m_serverSignature )
				{
					if( allowResolve )
					{
						for( uint32_t i = 0; i < m_netObjects.Length(); i++ )
						{
							toDestroy.Set( m_netObjects.GetValue( i ), 0 );
						}
					}
					else
					{
						m_delayCreationForDestroy = true; // This prevents new server objects and old server objects overlapping for a frame
						m_created.Clear(); // Don't call delete, are pointers to m_netObjects
						for( uint32_t i = 0; i < m_netObjects.Length(); i++ )
						{
							m_StartNetObjectDestruction( m_netObjects.GetValue( i ) );
						}
						AE_ASSERT( !m_remoteToLocalIdMap.Length() );
						AE_ASSERT( !m_localToRemoteIdMap.Length() );
					}
				}
				
				uint32_t length = 0;
				rStream.SerializeUInt32( length );
				for( uint32_t i = 0; i < length && rStream.IsValid(); i++ )
				{
					NetObject* created = m_CreateNetObject( &rStream, allowResolve );
					toDestroy.Remove( created );
				}
				for( uint32_t i = 0; i < toDestroy.Length(); i++ )
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
				if( m_remoteToLocalIdMap.TryGet( remoteId, &localId )
						&& m_netObjects.TryGet( localId, &netObject ) )
				{
					m_StartNetObjectDestruction( netObject );
				}
				break;
			}
			case NetObjectConnection::EventType::Update:
			{
				uint32_t netObjectCount = 0;
				rStream.SerializeUInt32( netObjectCount );
				for( uint32_t i = 0; i < netObjectCount; i++ )
				{
					RemoteId remoteId;
					uint32_t dataLen = 0;
					rStream.SerializeObject( remoteId );
					rStream.SerializeUInt32( dataLen );

					NetId localId;
					NetObject* netObject = nullptr;
					if( dataLen
						&& m_remoteToLocalIdMap.TryGet( remoteId, &localId )
						&& m_netObjects.TryGet( localId, &netObject ) )
					{
						if( rStream.GetRemainingBytes() >= dataLen )
						{
							netObject->m_SetClientData( rStream.PeekReadData(), dataLen );
						}
						else
						{
							rStream.Invalidate();
						}
					}

					rStream.DiscardReadData( dataLen );
				}
				break;
			}
			case NetObjectConnection::EventType::Messages:
			{
				uint32_t netObjectCount = 0;
				rStream.SerializeUInt32( netObjectCount );
				for( uint32_t i = 0; i < netObjectCount; i++ )
				{
					RemoteId remoteId;
					uint32_t dataLen = 0;
					rStream.SerializeObject( remoteId );
					rStream.SerializeUInt32( dataLen );

					NetId localId;
					NetObject* netObject = nullptr;
					if( dataLen
						&& m_remoteToLocalIdMap.TryGet( remoteId, &localId )
						&& m_netObjects.TryGet( localId, &netObject ) )
					{
						if( rStream.GetRemainingBytes() >= dataLen )
						{
							netObject->m_ReceiveMessages( rStream.PeekReadData(), dataLen );
						}
						else
						{
							rStream.Invalidate();
						}
					}

					rStream.DiscardReadData( dataLen );
				}
				break;
			}
		}
	}
}

NetObject* NetObjectClient::PumpCreate()
{
	if( !m_created.Length() )
	{
		return nullptr;
	}

	if( m_delayCreationForDestroy )
	{
		for( uint32_t i = 0; i < m_netObjects.Length(); i++ )
		{
			if( m_netObjects.GetValue( i )->IsPendingDestroy() )
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
	if( !pendingDestroy )
	{
		return;
	}
	bool removed = m_netObjects.Remove( pendingDestroy->GetId() );
	AE_ASSERT_MSG( removed, "ae::NetObject can't be destroyed. It's' not managed by this ae::NetObjectClient." );
	
	if( !pendingDestroy->IsPendingDestroy() )
	{
		m_StartNetObjectDestruction( pendingDestroy );
	}
	ae::Delete( pendingDestroy );
	
#if _AE_DEBUG_
	if( !m_netObjects.Length() )
	{
		AE_ASSERT( !m_localToRemoteIdMap.Length() );
		AE_ASSERT( !m_remoteToLocalIdMap.Length() );
	}
#endif
}

NetObject* NetObjectClient::m_CreateNetObject( ae::BinaryReader* rStream, bool allowResolve )
{
	// @TODO: rStream validation

	RemoteId remoteId;
	rStream->SerializeObject( remoteId );

	NetObject* netObject = nullptr;
	if( allowResolve )
	{
		NetId localId = m_remoteToLocalIdMap.Get( remoteId, {} );
		if( localId )
		{
			netObject = m_netObjects.Get( localId );
			AE_ASSERT( netObject );
			uint16_t initDataLength = 0;
			rStream->SerializeUInt16( initDataLength );
			if( rStream->IsValid() )
			{
				AE_ASSERT( netObject->m_initData.Length() == initDataLength );
				AE_ASSERT( memcmp( netObject->m_initData.Data(), rStream->PeekReadData(), initDataLength ) == 0 );
				rStream->DiscardReadData( initDataLength );
			}
		}
	}

	if( !netObject )
	{
		NetId localId( ++m_lastNetId );
		netObject = ae::New< NetObject >( AE_ALLOC_TAG_NET );
		netObject->m_id = localId;

		m_netObjects.Set( localId, netObject );
		m_remoteToLocalIdMap.Set( remoteId, localId );
		m_localToRemoteIdMap.Set( localId, remoteId );
		m_created.Append( netObject );

		uint16_t initDataLength = 0;
		rStream->SerializeUInt16( initDataLength );
		netObject->m_initData.Append( {}, initDataLength );
		rStream->SerializeRaw( netObject->m_initData.Data(), initDataLength );
	}

	// @TODO: Handle invalid rStream
	return netObject;
}

void NetObjectClient::m_StartNetObjectDestruction( NetObject* netObject )
{
	AE_ASSERT( netObject );
	if( netObject->IsPendingDestroy() )
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
	for( uint32_t i = 0; i < m_replicaDB->GetNetObjectCount(); i++ )
	{
		NetObject* netObject = m_replicaDB->GetNetObject( i );
		if( m_first || netObject->m_Changed() )
		{
			toSync.Append( netObject );
		}

		if( netObject->m_messageDataOut.Length() )
		{
			netObjectMessageCount++;
		}
	}

	ae::BinaryWriter wStream( &m_connData );

	if( toSync.Length() )
	{
		wStream.SerializeEnum( NetObjectConnection::EventType::Update );
		wStream.SerializeUInt32( toSync.Length() );
		for( uint32_t i = 0; i < toSync.Length(); i++ )
		{
			NetObject* netObject = toSync[ i ];
			wStream.SerializeObject( netObject->GetId() );
			wStream.SerializeUInt32( netObject->SyncDataLength() );
			wStream.SerializeRaw( netObject->GetSyncData(), netObject->SyncDataLength() );
		}
	}

	if( netObjectMessageCount )
	{
		wStream.SerializeEnum( NetObjectConnection::EventType::Messages );
		wStream.SerializeUInt32( netObjectMessageCount );
		for( uint32_t i = 0; i < m_replicaDB->GetNetObjectCount(); i++ )
		{
			NetObject* netObject = m_replicaDB->GetNetObject( i );
			if( netObject->m_messageDataOut.Length() )
			{
				wStream.SerializeObject( netObject->GetId() );
				wStream.SerializeUInt32( netObject->m_messageDataOut.Length() );
				wStream.SerializeRaw( &netObject->m_messageDataOut[ 0 ], netObject->m_messageDataOut.Length() );
			}
		}
	}

	m_pendingClear = true;
	m_first = false;
}

void NetObjectConnection::m_ClearPending()
{
	if( m_pendingClear )
	{
		m_connData.Clear();
		m_pendingClear = false;
	}
}

const uint8_t* NetObjectConnection::GetSendData() const
{
	return m_connData.begin();
}

uint32_t NetObjectConnection::GetSendLength() const
{
	return m_connData.Length();
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
	if( !netObject )
	{
		return;
	}
	
	int32_t pendingIdx = m_pendingCreate.Find( netObject );
	if( pendingIdx >= 0 )
	{
		// Early out, no need to send Destroy message because Create has not been queued
		m_pendingCreate.Remove( pendingIdx );
		ae::Delete( netObject );
		return;
	}

	NetId id = netObject->GetId();
	bool removed = m_netObjects.Remove( id );
	AE_ASSERT_MSG( removed, "NetObject was not found." );

	for( uint32_t i = 0; i < m_connections.Length(); i++ )
	{
		NetObjectConnection* conn = m_connections[ i ];
		conn->m_ClearPending(); // @TODO: Should this queue up like m_pendingCreate?
		ae::BinaryWriter wStream( &conn->m_connData );
		wStream.SerializeEnum( NetObjectConnection::EventType::Destroy );
		wStream.SerializeObject( id );
	}

	ae::Delete( netObject );
}

NetObjectConnection* NetObjectServer::CreateConnection()
{
	NetObjectConnection* conn = m_connections.Append( ae::New< NetObjectConnection >( AE_ALLOC_TAG_NET ) );
	AE_ASSERT( !conn->m_pendingClear );
	conn->m_replicaDB = this;

	// Send initial net datas
	ae::BinaryWriter wStream( &conn->m_connData );
	wStream.SerializeEnum( NetObjectConnection::EventType::Connect );
	wStream.SerializeUInt32( m_signature );
	wStream.SerializeUInt32( m_netObjects.Length() );
	for( uint32_t i = 0; i < m_netObjects.Length(); i++ )
	{
		const NetObject* netObject = m_netObjects.GetValue( i );
		wStream.SerializeObject( netObject->GetId() );

		AE_ASSERT( netObject->m_initData.Length() <= ae::MaxValue< uint16_t >() );
		const uint16_t initDataLength = (uint16_t)netObject->m_initData.Length();
		wStream.SerializeUInt16( initDataLength );
		wStream.SerializeRaw( netObject->m_initData.Data(), initDataLength );
	}

	return conn;
}

void NetObjectServer::DestroyConnection( NetObjectConnection* conn )
{
	if( !conn )
	{
		return;
	}

	int32_t index = m_connections.Find( conn );
	if( index >= 0 )
	{
		m_connections.Remove( index );
		ae::Delete( conn );
	}
}

void NetObjectServer::UpdateSendData()
{
	// Clear old send data before writing new
	for( uint32_t i = 0; i < m_connections.Length(); i++ )
	{
		m_connections[ i ]->m_ClearPending();
	}
	
	// Send info about new objects (delayed until Update in case objects initData need to reference each other)
	for( NetObject* netObject : m_pendingCreate )
	{
		if( !netObject->IsPendingInit() )
		{
			// Add net data to list, remove all initialized net datas from m_pendingCreate at once below
			m_netObjects.Set( netObject->GetId(), netObject );
			
			// Send create messages on existing server connections
			for( uint32_t i = 0; i < m_connections.Length(); i++ )
			{
				ae::BinaryWriter wStream( &m_connections[ i ]->m_connData );
				wStream.SerializeEnum( NetObjectConnection::EventType::Create );
				wStream.SerializeObject( netObject->GetId() );
				AE_ASSERT( netObject->m_initData.Length() <= ae::MaxValue< uint16_t >() );
				const uint16_t initDataLength = (uint16_t)netObject->m_initData.Length();
				wStream.SerializeUInt16( initDataLength );
				wStream.SerializeRaw( netObject->m_initData.Data(), initDataLength );
			}
		}
	}
	// Remove all pending net datas that were just initialized
	m_pendingCreate.RemoveAllFn( []( const NetObject* netObject ){ return !netObject->IsPendingInit(); } );
	
	for( uint32_t i = 0; i < m_netObjects.Length(); i++ )
	{
		m_netObjects.GetValue( i )->m_UpdateHash();
	}

	for( uint32_t i = 0; i < m_connections.Length(); i++ )
	{
		m_connections[ i ]->m_UpdateSendData();
	}

	for( uint32_t i = 0; i < m_netObjects.Length(); i++ )
	{
		NetObject* netObject = m_netObjects.GetValue( i );
		netObject->m_prevHash = netObject->m_hash;
		netObject->m_messageDataOut.Clear();
	}
}

//------------------------------------------------------------------------------
// ae::IsosurfaceExtractor member functions
//------------------------------------------------------------------------------
IsosurfaceExtractor::IsosurfaceExtractor( ae::Tag tag ) :
	vertices( tag ),
	indices( tag ),
	m_voxels( tag ),
	m_dualSamples( tag )
{
	Reset();
}

void IsosurfaceExtractor::Reserve( uint32_t vertexCount, uint32_t indexCount )
{
	vertices.Reserve( vertexCount );
	indices.Reserve( indexCount );
	m_voxels.Reserve( vertexCount * 1.25f ); // Leave 20% of array free
	m_dualSamples.Reserve( vertexCount * 1.25f ); // Leave 20% of array free
}

void IsosurfaceExtractor::Reset()
{
	vertices.Clear();
	indices.Clear();
	m_params = {};
	m_stats = {};
	m_statsPrev = {};
	m_voxels.Clear();
	m_dualSamples.Clear();
}

bool IsosurfaceExtractor::m_Generate( ae::Int3 center, uint32_t halfSize )
{
	AE_DEBUG_ASSERT( halfSize % 2 == 0 || halfSize == 1 );

	AE_DEBUG_IF( m_params.debugPos.has_value() )
	{
		const ae::AABB debugAABB( ae::Vec3( center ) - ae::Vec3( halfSize ), ae::Vec3( center ) + ae::Vec3( halfSize ) );
		if( debugAABB.Contains( m_params.debugPos.value() ) )
		{
			int breakPointHere = 0;
		}
	}

	const ae::IsosurfaceValue sample = m_DualSample( center );
	const float diagonalHalfSize = sqrtf( halfSize * halfSize * 3.0f );
	const float surfaceDistance = ae::Abs( sample.distance ) - sample.distanceErrorMargin;
	if( diagonalHalfSize < surfaceDistance )
	{
		m_stats.voxelSearchProgress += ( (uint64_t)halfSize * halfSize * halfSize * 8 );
		m_UpdateStats();
 		return true; // No intersection, no need to split further
	}
	
	// Check if octant is outside of the given AABB
	const uint32_t nextHalfSize = ( halfSize / 2 );
	if( nextHalfSize )
	{
		for( uint32_t i = 0; i < 8; i++ )
		{
			const ae::Int3 nextCenter = ( center + kChildOffsets[ i ] * nextHalfSize );
			const ae::Int3 halfSize3( nextHalfSize );
			const ae::Int3 minInclusive = nextCenter - halfSize3;
			const ae::Int3 maxExclusive = nextCenter + halfSize3;
			if( m_minInclusive.x < maxExclusive.x && minInclusive.x <= m_maxInclusive.x &&
				m_minInclusive.y < maxExclusive.y && minInclusive.y <= m_maxInclusive.y &&
				m_minInclusive.z < maxExclusive.z && minInclusive.z <= m_maxInclusive.z )
			{
				if( !m_Generate( nextCenter, nextHalfSize ) )
				{
					return false;
				}
			}
			else
			{
				m_stats.voxelSearchProgress += ( (uint64_t)nextHalfSize * nextHalfSize * nextHalfSize * 8 );
				m_UpdateStats();
			}
		}
	}
	else
	{
		AE_DEBUG_ASSERT( halfSize == 1 );
		const ae::Int3 halfSize3( halfSize );
		const ae::Int3 minInclusive = center - halfSize3;
		const ae::Int3 maxExclusive = center + halfSize3;
		// Don't bother checking the distance of all 8 final octants from the
		// surface here, the next step checks anyway and will early out.
		for( int32_t z = minInclusive.z; z < maxExclusive.z; z++ )
		{
			for( int32_t y = minInclusive.y; y < maxExclusive.y; y++ )
			{
				for( int32_t x = minInclusive.x; x < maxExclusive.x; x++ )
				{
					if( m_minInclusive.x <= x && x <= m_maxInclusive.x &&
						m_minInclusive.y <= y && y <= m_maxInclusive.y &&
						m_minInclusive.z <= z && z <= m_maxInclusive.z )
					{
						if( !m_DoVoxel( x, y, z ) )
						{
							return false;
						}
					}
				}
			}
		}
		m_stats.voxelSearchProgress += 8;
		m_UpdateStats();
	}

	if( m_params.octree )
	{
		m_params.octree->Append( ae::AABB( ae::Vec3( center ) - ae::Vec3( halfSize ), ae::Vec3( center ) + ae::Vec3( halfSize ) ) );
	}

	return true;
}

bool IsosurfaceExtractor::m_DoVoxel( int32_t x, int32_t y, int32_t z )
{
	const ae::Int3 voxelPos( x, y, z );
	const ae::Int3 sdfMin = m_params.aabb.GetMin().FloorCopy();
	const ae::Int3 sdfMax = m_params.aabb.GetMax().CeilCopy();
	
	AE_DEBUG_IF( m_params.debugPos.has_value() )
	{
		const ae::Int3 debugVoxel = m_params.debugPos.value().FloorCopy();
		if( debugVoxel == voxelPos )
		{
			int breakPointHere = 0;
		}
	}

	m_stats.voxelCheckCount++;

	const ae::Int3 sharedCornerOffset( 1 );
	const ae::IsosurfaceValue sharedCornerSample = m_DualSample( voxelPos + sharedCornerOffset );
	const float sharedCornerValue = sharedCornerSample.distance;
	static const float voxelDiagonal = sqrtf( 3.0f ); // Diagonal of a voxel with size 1
	if( ae::Abs( sharedCornerValue ) > ( voxelDiagonal + sharedCornerSample.distanceErrorMargin ) )
	{
		// Early out of additional edge intersections if voxel doesn't overlap
		// the surface
		m_stats.voxelMissCount++;
		return true;
	}

	const float cornerValues[ 3 ] = {
		m_DualSample( voxelPos + cornerOffsets[ 0 ] ).distance,
		m_DualSample( voxelPos + cornerOffsets[ 1 ] ).distance,
		m_DualSample( voxelPos + cornerOffsets[ 2 ] ).distance
	};

	// @TODO: Should this get the current value of the voxel, in case it's been partially generated by adjacents?
	AE_DEBUG_ASSERT( !m_voxels.TryGet( { x + 1, y + 1, z + 1 } ) );
	Voxel voxel;// = m_voxels.Get( { x + 1, y + 1, z + 1 }, {} );
	AE_DEBUG_ASSERT( voxel.edgeBits == 0 ); // @TODO: Debug assert
	// Detect if any of the 3 new edges being tested intersect the implicit surface
	if( cornerValues[ 0 ] * sharedCornerValue < 0.0f )
	{
		voxel.edgeBits |= EDGE_TOP_FRONT_BIT;
	}
	if( cornerValues[ 1 ] * sharedCornerValue < 0.0f )
	{
		voxel.edgeBits |= EDGE_TOP_RIGHT_BIT;
	}
	if( cornerValues[ 2 ] * sharedCornerValue < 0.0f )
	{
		voxel.edgeBits |= EDGE_SIDE_FRONTRIGHT_BIT;
	}
	if( voxel.edgeBits && m_params.errors && (
		ae::Abs( cornerValues[ 0 ] - sharedCornerValue ) > 1.01f ||
		ae::Abs( cornerValues[ 1 ] - sharedCornerValue ) > 1.01f ||
		ae::Abs( cornerValues[ 2 ] - sharedCornerValue ) > 1.01f ) )
	{
		m_params.errors->Append( ae::Vec3( voxelPos ) );
	}

	// Iterate over the 3 edges that this voxel is responsible for. The
	// remaining 9 are handled by adjacent voxels.
	for( int32_t e = 0; e < 3; e++ )
	{
		if( voxel.edgeBits & mask[ e ] )
		{
			if( vertices.Length() + 4 > m_params.maxVerts || indices.Length() + 6 > m_params.maxIndices )
			{
				return false;
			}

			// Calculate edge intersection position
			// Assume a linear function for the SDF between the between the
			// two corners, and find where the value equals 0 (the surface).
			// "0 = A + s * (B - A)" -> "s = -A / (B - A) = A / (A - B)"
			const float s = sharedCornerValue / ( sharedCornerValue - cornerValues[ e ] );
			AE_DEBUG_ASSERT( s >= 0.0f && s <= 1.0f );
			const ae::Vec3 linearOffset = ae::Lerp( ae::Vec3( sharedCornerOffset ), ae::Vec3( cornerOffsets[ e ] ), s );
			ae::Vec3 edgeOffset01 = linearOffset;
			{
				// Sphere trace the voxel edge from the outside corner to the inside
				// corner to find the intersection with the SDF surface. Only
				// do this with a low error margin, and otherwise use the
				// "smoother" surface nets value. See lerp below.
				if( m_params.dualContouring && sharedCornerSample.distanceErrorMargin < 1.0f )
				{
					const bool sharedCornerInside = ( sharedCornerValue < cornerValues[ e ] );
					const ae::Vec3 start = ae::Vec3( sharedCornerInside ? cornerOffsets[ e ] : sharedCornerOffset );
					const ae::Vec3 end = ae::Vec3( sharedCornerInside ? sharedCornerOffset : cornerOffsets[ e ] );
					const ae::Vec3 rayDir = ( end - start ); // No need to normalize since voxel/ray length is 1
					float depth = 0.0f;
					for( int32_t i = 0; i < 8; i++ ) // @TODO: This should probably be adjustable
					{
						edgeOffset01 = start + rayDir * depth;
						const float closestSurfaceDist = m_Sample( ae::Vec3( voxelPos ) + edgeOffset01 ).distance;
						if( closestSurfaceDist < 0.01f )
						{
							break; // Hit the surface
						}
						depth += closestSurfaceDist;
						if( depth >= 1.0f )
						{
							AE_DEBUG_FAIL_MSG( "depth >= 1", "depth:#", depth );
							depth = 1.0f;
							break;
						}
					}
				}
				// If the error margin of the sampled value is greater than a
				// full voxel width then an approximate offset must be used,
				// because there may be no clean intersection point. If it's
				// less than a full voxel width then blend the two results.
				edgeOffset01 = ae::Lerp( edgeOffset01, linearOffset, ae::Clip01( sharedCornerSample.distanceErrorMargin ) );
			}
			AE_DEBUG_ASSERT( edgeOffset01.x == edgeOffset01.x && edgeOffset01.y == edgeOffset01.y && edgeOffset01.z == edgeOffset01.z );
			AE_DEBUG_ASSERT( edgeOffset01.x >= 0.0f && edgeOffset01.x <= 1.0f );
			AE_DEBUG_ASSERT( edgeOffset01.y >= 0.0f && edgeOffset01.y <= 1.0f );
			AE_DEBUG_ASSERT( edgeOffset01.z >= 0.0f && edgeOffset01.z <= 1.0f );
			voxel.edgePos[ e ] = edgeOffset01;

			// Calculate gradient at edge intersection
			if( m_params.dualContouring )
			{
				const ae::Vec3 p( ae::Vec3( voxelPos ) + edgeOffset01 );
				const ae::Vec3 pv( m_Sample( p ).distance );
				AE_DEBUG_IF( m_params.errors && pv != pv ) { m_params.errors->Append( p ); }

				ae::Vec3 normal0;
				for( int32_t i = 0; i < 3; i++ )
				{
					ae::Vec3 nt = p;
					nt[ i ] += m_params.normalSampleOffset;
					normal0[ i ] = m_Sample( nt ).distance;
				}
				// This should be close to 0 because it's really close to the
				// surface but not close enough to ignore.
				normal0 -= pv;
				AE_DEBUG_IF( m_params.errors && normal0 == ae::Vec3( 0.0f ) ) { m_params.errors->Append( p ); }
				normal0 /= normal0.Length();
				AE_DEBUG_IF( m_params.errors && normal0 != normal0 ) { m_params.errors->Append( p ); }

				ae::Vec3 normal1;
				for( int32_t i = 0; i < 3; i++ )
				{
					ae::Vec3 nt = p;
					nt[ i ] -= m_params.normalSampleOffset;
					normal1[ i ] = m_Sample( nt ).distance;
				}
				// This should be close to 0 because it's really close to the
				// surface but not close enough to ignore.
				normal1 = pv - normal1;
				AE_DEBUG_IF( m_params.errors && normal1 == ae::Vec3( 0.0f ) ) { m_params.errors->Append( p ); }
				normal1 /= normal1.Length();
				AE_DEBUG_IF( m_params.errors && normal1 != normal1 ) { m_params.errors->Append( p ); }
				
				voxel.edgeNormal[ e ] = ( normal1 + normal0 ).SafeNormalizeCopy();
				if( m_params.errors && voxel.edgeNormal[ e ] == ae::Vec3( 0.0f ) ) { m_params.errors->Append( p ); }
			}
			else
			{
				voxel.edgeNormal[ e ] = ae::Vec3( 0.0f );
			}

			// Only generate vertices within the given bounds. The above
			// calculations are still needed one voxel outside of the bounds,
			// because voxels share edge information to avoid duplicate work.
			if( x >= sdfMax.x || y >= sdfMax.y || z >= sdfMax.z )
			{
				continue;
			}

			// Array of 4 sampling offsets for this edge. These don't include
			// the current voxel edge being processed (x + 1, y + 1, z + 1).
			const ae::Int3* offsets;
			switch( e )
			{
				case 0: offsets = offsets_EDGE_TOP_FRONT_BIT; break;
				case 1: offsets = offsets_EDGE_TOP_RIGHT_BIT; break;
				case 2: offsets = offsets_EDGE_SIDE_FRONTRIGHT_BIT; break;
				default:
					AE_FAIL();
					offsets = nullptr;
					break;
			}

			// Expand edge intersection into two triangles. New vertices are
			// added as needed for each edge intersection, so this does some of
			// the work for adjacent voxels. Vertices are centered in voxels at
			// this point at this stage.
			IsosurfaceIndex quad[ 4 ];
			for( int32_t j = 0; j < 4; j++ )
			{
				const int32_t ox = x + offsets[ j ][ 0 ];
				const int32_t oy = y + offsets[ j ][ 1 ];
				const int32_t oz = z + offsets[ j ][ 2 ];
				AE_DEBUG_ASSERT( !( ( ox == x + 1 ) && ( oy == y + 1 ) && ( oz == z + 1 ) ) ); // The current voxel is not processed here
				Voxel* quadVoxel = m_voxels.TryGet( { ox, oy, oz } );
				quadVoxel = quadVoxel ? quadVoxel : &m_voxels.Set( { ox, oy, oz }, {} );
				IsosurfaceIndex* vertexIndex = &quadVoxel->index;
				if( *vertexIndex == kInvalidIsosurfaceIndex )
				{
					IsosurfaceVertex vertex;
					vertex.position.x = ox + 0.5f;
					vertex.position.y = oy + 0.5f;
					vertex.position.z = oz + 0.5f;
					vertex.position.w = 1.0f;
					AE_DEBUG_ASSERT( vertex.position.x == vertex.position.x && vertex.position.y == vertex.position.y && vertex.position.z == vertex.position.z );

					// Record the index of the vertex in the chunk so it can
					// be reused by adjacent quads
					*vertexIndex = (IsosurfaceIndex)vertices.Length();
					vertices.Append( vertex );
				}
				AE_DEBUG_ASSERT_MSG( *vertexIndex < (IsosurfaceIndex)vertices.Length(), "# < # ox:# oy:# oz:#", index, vertices.Length(), ox, oy, oz );
				quad[ j ] = *vertexIndex;
			}

			// @TODO: This assumes counter clockwise culling
			bool flip = false;
			if( e == 0 )
			{
				flip = ( sharedCornerValue > 0.0f );
			} // EDGE_TOP_FRONT_BIT
			else
			{
				flip = ( sharedCornerValue < 0.0f );
			} // EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT
			if( flip )
			{
				// tri0
				indices.Append( quad[ 0 ] );
				indices.Append( quad[ 1 ] );
				indices.Append( quad[ 2 ] );
				// tri1
				indices.Append( quad[ 1 ] );
				indices.Append( quad[ 3 ] );
				indices.Append( quad[ 2 ] );
			}
			else
			{
				// tri2
				indices.Append( quad[ 0 ] );
				indices.Append( quad[ 2 ] );
				indices.Append( quad[ 1 ] );
				// tri3
				indices.Append( quad[ 1 ] );
				indices.Append( quad[ 2 ] );
				indices.Append( quad[ 3 ] );
			}
		}
	}
	// Edges for the current voxel are stored in the "top" corner of the dual grid
	m_voxels.Set( { x + 1, y + 1, z + 1 }, voxel );
	return true;
}

IsosurfaceValue IsosurfaceExtractor::m_DualSample( ae::Int3 pos )
{
	const VoxelIndex v( pos.x, pos.y, pos.z );
	IsosurfaceValue* sample = m_dualSamples.TryGet( v );
	if( sample )
	{
		m_stats.sampleCacheCount++;
		return *sample;
	}
	else
	{
		m_stats.sampleRawCount++;
		return m_dualSamples.Set( v, m_Sample( ae::Vec3( pos ) ) );
	}
}

IsosurfaceValue IsosurfaceExtractor::m_Sample( ae::Vec3 pos )
{
	m_stats.sampleRawCount++;
	IsosurfaceValue v = m_params.sampleFn( m_params.userData, ( pos ) );
	// This nudge is needed to prevent the SDF from ever being exactly on
	// the voxel grid boundaries (imagine a plane at the origin with a
	// normal facing along a cardinal axis, do the vertices belong to the
	// voxels on the front or back of the plane?). Without this nudge, any
	// vertices exactly on the grid boundary would be skipped resulting in
	// holes in the mesh.
	if( v.distance == 0.0f ) { v.distance += 0.0001f; }
	return v;
}

void IsosurfaceExtractor::m_UpdateStats()
{
	if( m_params.statsFn )
	{
		m_stats.voxelProgress01 = ( m_stats.voxelSearchProgress / (float)m_stats.voxelAABBSize );
		const bool voxelProgressChanged = int32_t( m_statsPrev.voxelProgress01 * 100.0f ) != int32_t( m_stats.voxelProgress01 * 100.0f );
		const bool meshProgressChanged = int32_t( m_statsPrev.meshProgress01 * 100.0f ) != int32_t( m_stats.meshProgress01 * 100.0f );
		if( voxelProgressChanged || meshProgressChanged )
		{
			const double currentTime = ae::GetTime();
			const double voxelEndTime = m_startMeshTime ? ae::Min( currentTime, m_startMeshTime ) : currentTime;
			m_stats.voxelProgress01 = ae::Min( m_stats.voxelProgress01, 1.0f );
			m_stats.meshProgress01 = ae::Min( m_stats.meshProgress01, 1.0f );
			m_stats.elapsedTime = ( currentTime - m_startVoxelTime );
			m_stats.voxelTime = ( voxelEndTime - m_startVoxelTime );
			m_stats.meshTime = m_startMeshTime ? ( currentTime - m_startMeshTime ) : 0.0;
			m_params.statsFn( m_params.userData, m_stats );
			m_statsPrev = m_stats;
		}
	}
}

bool IsosurfaceExtractor::Generate( const ae::IsosurfaceParams& _params )
{
	if( !_params.aabb.Contains( _params.aabb.GetCenter() ) )
	{
		return true;
	}
	Reset();
	m_params = _params;
	m_stats = {};
	m_statsPrev = {};
	m_startVoxelTime = ae::GetTime();
	m_startMeshTime = 0.0;
	if( m_params.maxVerts == 0 )
	{
		m_params.maxVerts = ae::MaxValue< uint32_t >();
	}
	if( m_params.maxIndices == 0 )
	{
		m_params.maxIndices = ae::MaxValue< uint32_t >();
	}
	m_minInclusive = m_params.aabb.GetMin().FloorCopy();
	m_maxInclusive = m_params.aabb.GetMax().CeilCopy();

	const ae::Vec3 paramHalfSize = m_params.aabb.GetHalfSize();
	const float maxHalfSize = ae::Max( paramHalfSize.x, paramHalfSize.y, paramHalfSize.z );
	const uint32_t halfSize = ae::NextPowerOfTwo( maxHalfSize * 2.0f + 0.5f ) / 2;
	m_stats.voxelAABBSize = (uint64_t)halfSize * halfSize * halfSize * 8;
	m_Generate( m_params.aabb.GetCenter().FloorCopy(), halfSize );
	m_stats.vertexCount = vertices.Length();
	m_stats.indexCount = indices.Length();
	m_stats.voxelWorkingSize = m_voxels.Length();
	m_UpdateStats();
	if( indices.Length() == 0 )
	{
		vertices.Clear();
		return false;
	}

	m_startMeshTime = ae::GetTime();
	const float progressPerVert = 1.0f / vertices.Length();
	const ae::Int3 sdfMin = m_params.aabb.GetMin().FloorCopy();
	const ae::Int3 sdfMax = m_params.aabb.GetMax().CeilCopy();
	for( IsosurfaceVertex& vertex : vertices )
	{
		const int32_t x = ae::Floor( vertex.position.x );
		const int32_t y = ae::Floor( vertex.position.y );
		const int32_t z = ae::Floor( vertex.position.z );
		if( x < sdfMin.x && y < sdfMin.y && z < sdfMin.z ){ continue; }
		if( x > sdfMax.x && y > sdfMax.y && z > sdfMax.z ){ continue; }

		int32_t ec = 0;
		ae::Vec3 p[ 12 ];
		ae::Vec3 n[ 12 ];
		Voxel te = m_voxels.Get( { x + 1, y + 1, z + 1 }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			n[ ec ] = te.edgeNormal[ 1 ];
			ec++;
		}
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x, y + 1, z + 1 }, {} );
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.edgeNormal[ 1 ];
			ec++;
		}
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x + 1, y, z + 1 }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x, y, z + 1 }, {} );
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x, y + 1, z }, {} );
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 1 ];
			ec++;
		}
		te = m_voxels.Get( { x + 1, y, z }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			p[ ec ].y -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		te = m_voxels.Get( { x + 1, y + 1, z }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 1 ];
			ec++;
		}

		// Validation
		AE_DEBUG_ASSERT( ec != 0 );
		for( int32_t j = 0; j < ec; j++ )
		{
			AE_DEBUG_ASSERT( p[ j ] == p[ j ] );
			AE_DEBUG_ASSERT( p[ j ].x >= 0.0f && p[ j ].x <= 1.0f );
			AE_DEBUG_ASSERT( p[ j ].y >= 0.0f && p[ j ].y <= 1.0f );
			AE_DEBUG_ASSERT( p[ j ].z >= 0.0f && p[ j ].z <= 1.0f );
			AE_DEBUG_ASSERT( n[ j ] == n[ j ] );
		}

		ae::Vec3 position;
		if( m_params.dualContouring )
		{
			// Get intersection of edge planes for vertex positioning
			{
#if defined(__SSE2__) || defined(_M_SSE2)
				__m128 c128 = _mm_setzero_ps();
				for( uint32_t i = 0; i < ec; i++ )
				{
					__m128 p128 = _mm_load_ps( (float*)( p + i ) );
					c128 = _mm_add_ps( c128, p128 );
				}
				__m128 div = _mm_set1_ps( 1.0f / ec );
				c128 = _mm_mul_ps( c128, div );
	
				for( uint32_t i = 0; i < 10; i++ )
				{
					for( uint32_t j = 0; j < ec; j++ )
					{
						__m128 p128 = _mm_load_ps( (float*)( p + j ) );
						p128 = _mm_sub_ps( p128, c128 );
						__m128 n128 = _mm_load_ps( (float*)( n + j ) );
	
						__m128 d = _mm_mul_ps( p128, n128 );
						d = _mm_hadd_ps( d, d );
						d = _mm_hadd_ps( d, d );
	
						__m128 s = _mm_set1_ps( 0.5f );
						s = _mm_mul_ps( s, n128 );
						s = _mm_mul_ps( s, d );
						c128 = _mm_add_ps( c128, s );
					}
				}
				_mm_store_ps( (float*)&position, c128 );
#elif defined(__ARM_NEON__) || defined(__ARM_NEON)
				float32x4_t c128 = vdupq_n_f32( 0.0f );
				for( uint32_t i = 0; i < ec; i++ )
				{
					const float32x4_t p128 = vld1q_f32( (float*)( p + i ) );
					c128 = vaddq_f32( c128, p128 );
				}
				float32x4_t div = vdupq_n_f32( 1.0f / ec );
				c128 = vmulq_f32( c128, div );
	
				for( uint32_t i = 0; i < 10; i++ )
				{
					for( uint32_t j = 0; j < ec; j++ )
					{
						float32x4_t p128 = vld1q_f32( (float*)( p + j ) );
						p128 = vsubq_f32( p128, c128 );
						const float32x4_t n128 = vld1q_f32( (float*)( n + j ) );
	
						// Dot product (p128 . n128)
						const float32x4_t d = vmulq_f32( p128, n128 );
						const float32x2_t d_low = vget_low_f32( d );
						const float32x2_t d_high = vget_high_f32( d );
						const float32x2_t sum = vpadd_f32( d_low, d_high );
						const float32_t dot = vget_lane_f32( sum, 0 ) + vget_lane_f32( sum, 1 );
	
						const float32x4_t s = vmulq_n_f32( n128, 0.5f * dot );
						c128 = vaddq_f32( c128, s );
					}
				}
				vst1q_f32( (float*)&position, c128 );
#else
				position = ae::Vec3( 0.0f );
				for( uint32_t i = 0; i < ec; i++ )
				{
					position += p[ i ];
				}
				position /= ec;
				for( uint32_t i = 0; i < 10; i++ )
				{
					for( uint32_t j = 0; j < ec; j++ )
					{
						float d = n[ j ].Dot( p[ j ] - position );
						position += n[ j ] * ( d * 0.5f );
					}
				}
#endif
				AE_DEBUG_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
				// @NOTE: Bias towards average of intersection points. This solves some intersecting triangles on sharp edges.
				// Based on notes here: https://www.boristhebrave.com/2018/04/15/dual-contouring-tutorial/
				ae::Vec3 averagePos( 0.0f );
				for( int32_t i = 0; i < ec; i++ )
				{
					averagePos += p[ i ];
				}
				averagePos /= (float)ec;
				position = ae::Lerp( position, averagePos, 0.1f ); // @TODO: This bias should be removed or be adjustable
			}

			// Use the average edge normals for the vertex normal
			vertex.normal = ae::Vec3( 0.0f );
			for( int32_t j = 0; j < ec; j++ )
			{
				vertex.normal += n[ j ];
			}
			vertex.normal.SafeNormalize();
		}
		else
		{
			position = ae::Vec3( 0.0f );
			for( int32_t i = 0; i < ec; i++ )
			{
				position += p[ i ];
			}
			position /= (float)ec;

			// Calculate the normal from the 8 dual corner samples
			const float cornerSamples[ 8 ] =
			{
				m_DualSample( ae::Int3( x, y, z ) ).distance, // 0, left, back, bottom
				m_DualSample( ae::Int3( x + 1, y, z ) ).distance, // 1, right, back, bottom
				m_DualSample( ae::Int3( x, y + 1, z ) ).distance, // 2, left, front, bottom
				m_DualSample( ae::Int3( x + 1, y + 1, z ) ).distance, // 3, right, front, bottom
				m_DualSample( ae::Int3( x, y, z + 1 ) ).distance, // 4, left, back, top
				m_DualSample( ae::Int3( x + 1, y, z + 1 ) ).distance, // 5, right, back, top
				m_DualSample( ae::Int3( x, y + 1, z + 1 ) ).distance, // 6, left, front, top
				m_DualSample( ae::Int3( x + 1, y + 1, z + 1 ) ).distance // 7, right, front, top
			};
			vertex.normal = ae::Vec3( 0.0f );
			vertex.normal.x += ( cornerSamples[ 1 ] - cornerSamples[ 0 ] ); // back, bottom
			vertex.normal.x += ( cornerSamples[ 3 ] - cornerSamples[ 2 ] ); // front, bottom
			vertex.normal.x += ( cornerSamples[ 5 ] - cornerSamples[ 4 ] ); // back, top
			vertex.normal.x += ( cornerSamples[ 7 ] - cornerSamples[ 6 ] ); // front, top
			vertex.normal.y += ( cornerSamples[ 2 ] - cornerSamples[ 0 ] ); // left, bottom
			vertex.normal.y += ( cornerSamples[ 3 ] - cornerSamples[ 1 ] ); // right, bottom
			vertex.normal.y += ( cornerSamples[ 6 ] - cornerSamples[ 4 ] ); // left, top
			vertex.normal.y += ( cornerSamples[ 7 ] - cornerSamples[ 5 ] ); // right, top
			vertex.normal.z += ( cornerSamples[ 4 ] - cornerSamples[ 0 ] ); // left, back
			vertex.normal.z += ( cornerSamples[ 5 ] - cornerSamples[ 1 ] ); // right, back
			vertex.normal.z += ( cornerSamples[ 6 ] - cornerSamples[ 2 ] ); // left, front
			vertex.normal.z += ( cornerSamples[ 7 ] - cornerSamples[ 3 ] ); // right, front
			vertex.normal.SafeNormalize();
		}
		// @NOTE: Do not clamp position values to voxel boundary. It's valid for a vertex to be placed
		// outside of the voxel is was generated from. This happens when a voxel has all corners inside
		// or outside of the sdf boundary, while also still having intersections (normally two per edge)
		// on one or more edges of the voxel.
		position.x = x + position.x;
		position.y = y + position.y;
		position.z = z + position.z;
		vertex.position = ae::Vec4( position, 1.0f );

		m_stats.meshProgress01 += 1.0f / (float)vertices.Length();
		m_UpdateStats();
	}

	AE_DEBUG_ASSERT( vertices.Length() <= m_params.maxVerts );
	AE_DEBUG_ASSERT( indices.Length() <= m_params.maxIndices );
	m_stats.meshProgress01 = 1.0f;
	m_UpdateStats();
	return true;
}

} // ae end

//------------------------------------------------------------------------------
// Meta register base objects
//------------------------------------------------------------------------------
AE_REGISTER_NAMESPACECLASS( (ae, Object) );
AE_REGISTER_NAMESPACECLASS( (ae, Attribute) );
AE_REGISTER_NAMESPACECLASS( (ae, SourceFileAttribute) );

uint32_t ae::GetClassTypeCount()
{
	return _Globals::Get()->classTypes.Length();
}

const ae::ClassType* ae::GetClassTypeByIndex( uint32_t i )
{
	return _Globals::Get()->classTypes.GetValue( i );
}

const ae::ClassType* ae::GetClassTypeById( ae::TypeId id )
{
	return _Globals::Get()->classTypes.Get( id, nullptr );
}

const ae::ClassType* ae::GetClassTypeByName( const char* typeName )
{
	if( !typeName[ 0 ] ) { return nullptr; }
	const ae::TypeId typeId = (ae::TypeId)ae::GetTypeIdFromName( typeName );
	return _Globals::Get()->classTypes.Get( typeId, nullptr );
}

const ae::ClassType* ae::GetClassTypeFromObject( const ae::Object& obj )
{
	return GetClassTypeById( GetObjectTypeId( &obj ) );
}

const ae::EnumType* ae::GetEnumType( const char* enumName )
{
	const ae::TypeId typeId = (ae::TypeId)ae::GetTypeIdFromName( enumName );
	return ae::_Globals::Get()->enumTypes.Get( typeId, nullptr );
}

const ae::ClassType* ae::GetClassTypeFromObject( const ae::Object* obj )
{
	return obj ? GetClassTypeFromObject( *obj ) : nullptr;
}

//------------------------------------------------------------------------------
// ae::DataPointer member functions
//------------------------------------------------------------------------------
ae::DataPointer::DataPointer( const ae::Type& varType, void* data )
{
	m_varType = &varType;
	m_data = data;
}
ae::DataPointer::DataPointer( const ae::ClassVar* var, ae::Object* object )
{
	m_varType = &var->GetOuterVarType();
	m_data = (uint8_t*)object + var->GetOffset();
	if constexpr( _AE_DEBUG_ )
	{
		const ae::ClassType* type = ae::GetClassTypeFromObject( object );
		AE_ASSERT_MSG( type->IsType( var->m_owner.GetClassType() ), "Attempting to access '#::#' on object with type '#'", var->m_owner.GetClassType()->GetName(), var->GetName(), type->GetName() );
	}
}
ae::DataPointer::operator bool() const
{
	return m_data != nullptr;
}
const ae::Type& ae::DataPointer::GetVarType() const
{
	AE_ASSERT_MSG( m_data, "Check DataPointer before calling GetType()" );
	AE_ASSERT( m_varType );
	return *m_varType;
}
void* ae::DataPointer::Get( const ae::Type* caller ) const
{
	if( !m_varType )
	{
		return nullptr;
	}
	if( caller && !m_varType->IsSameBaseVarType( *caller ) )
	{
		AE_DEBUG_ASSERT_MSG( false, "Bad DataPointer access" );
		return nullptr;
	}
	return m_data;
}
bool ae::DataPointer::operator == ( const ae::DataPointer& other ) const
{
	if( m_data == other.m_data )
	{
		AE_DEBUG_ASSERT( !m_varType || m_varType->IsSameExactVarType( *other.m_varType ) );
		return true;
	}
	return false;
}
bool ae::DataPointer::operator != ( const ae::DataPointer& other ) const
{
	return !( m_data == other.m_data );
}

//------------------------------------------------------------------------------
// ae::ConstDataPointer member functions
//------------------------------------------------------------------------------
ae::ConstDataPointer::ConstDataPointer( const ae::Type& varType, const void* data )
{
	m_varType = &varType;
	m_data = data;
}
ae::ConstDataPointer::ConstDataPointer( DataPointer varData )
{
	m_varType = varData.m_varType;
	m_data = varData.m_data;
}
ae::ConstDataPointer::ConstDataPointer( const ae::ClassVar* var, const ae::Object* object )
{
	m_varType = &var->GetOuterVarType();
	m_data = (uint8_t*)object + var->GetOffset();
	if constexpr( _AE_DEBUG_ )
	{
		const ae::ClassType* type = ae::GetClassTypeFromObject( object );
		AE_ASSERT_MSG( type->IsType( var->m_owner.GetClassType() ), "Attempting to access '#::#' on object with type '#'", var->m_owner.GetClassType()->GetName(), var->GetName(), type->GetName() );
	}
}
ae::ConstDataPointer::operator bool() const
{
	return m_data != nullptr;
}
const ae::Type& ae::ConstDataPointer::GetVarType() const
{
	AE_ASSERT_MSG( m_data, "Check ConstDataPointer before calling GetType()" );
	AE_ASSERT( m_varType );
	return *m_varType;
}
const void* ae::ConstDataPointer::Get( const ae::Type* caller ) const
{
	if( !m_varType )
	{
		return nullptr;
	}
	if( caller && !m_varType->IsSameBaseVarType( *caller ) )
	{
		AE_DEBUG_ASSERT_MSG( false, "Bad DataPointer access" );
		return nullptr;
	}
	return m_data;
}
bool ae::ConstDataPointer::operator == ( const ae::ConstDataPointer& other ) const
{
	if( m_data == other.m_data )
	{
		AE_DEBUG_ASSERT( !m_varType || m_varType->IsSameExactVarType( *other.m_varType ) );
		return true;
	}
	return false;
}
bool ae::ConstDataPointer::operator != ( const ae::ConstDataPointer& other ) const
{
	return !( m_data == other.m_data );
}

//------------------------------------------------------------------------------
// ae::ClassVar member functions
//------------------------------------------------------------------------------
ae::ClassVar::Serializer::~Serializer()
{
	if( _Globals::Get()->varSerializer == this )
	{
		_Globals::Get()->varSerializer = nullptr;
	}
}

void ae::ClassVar::SetSerializer( const ae::ClassVar::Serializer* serializer )
{
	if( serializer )
	{
		_Globals::Get()->varSerializerInitialized = true;
	}
	_Globals::Get()->varSerializer = serializer;
}

const char* ae::ClassVar::GetName() const { return m_name.c_str(); }
uint32_t ae::ClassVar::GetOffset() const { return m_offset; }

std::string ae::ClassVar::GetObjectValueAsString( const ae::Object* obj, int32_t arrayIdx ) const
{
	if( !obj )
	{
		return "";
	}
	ae::ConstDataPointer varData( this, obj );
	const ae::Type* varType = &GetOuterVarType();
	if( const ae::ArrayType* arrayType = varType->AsVarType< ae::ArrayType >() )
	{
		if( arrayIdx < 0 )
		{
			return "";
		}
		varData = arrayType->GetElement( varData, arrayIdx );
		varType = &arrayType->GetInnerVarType();
	}
	else if( arrayIdx >= 0 )
	{
		return "";
	}

	if( varData )
	{
		if( const ae::BasicType* basicVarType = varType->AsVarType< ae::BasicType >() )
		{
			return basicVarType->GetVarDataAsString( varData );
		}
		else if( const ae::EnumType* enumVarType = varType->AsVarType< ae::EnumType >() )
		{
			return enumVarType->GetVarDataAsString( varData );
		}
		else if( const ae::PointerType* pointerVarType = varType->AsVarType< ae::PointerType >() )
		{
			ObjectPointerToStringFn fn = []( const void* userData, const ae::Object* obj ) -> std::string
			{
				const ae::ClassVar::Serializer* serializer = (const ae::ClassVar::Serializer*)userData;
				AE_ASSERT( serializer );
				return serializer->ObjectPointerToString( obj );
			};
			return pointerVarType->GetStringFromRef( varData, fn, _Globals::Get()->varSerializer );
		}
	}
	return "";
}

bool ae::ClassVar::SetObjectValueFromString( ae::Object* obj, const char* value, int32_t arrayIdx ) const
{
	if( !obj )
	{
		return false;
	}
	
	// Safety check to make sure 'this' Var belongs to 'obj' ae::ClassType
	const ae::ClassType* objType = ae::GetClassTypeFromObject( obj );
	AE_ASSERT( objType );
	AE_ASSERT_MSG( objType->IsType( m_owner.GetClassType() ), "Attempting to modify object '#' with var '#::#'", objType->GetName(), m_owner.GetClassType()->GetName(), GetName() );
	
	const ae::Type* varType = &GetOuterVarType();
	ae::DataPointer varData( this, obj );
	if( const ae::ArrayType* arrayType = varType->AsVarType< ae::ArrayType >() )
	{
		if( arrayIdx < 0 )
		{
			return false;
		}
		varType = &arrayType->GetInnerVarType();
		varData = arrayType->GetElement( varData, arrayIdx );
	}
	else if( arrayIdx >= 0 )
	{
		return false;
	}

	if( varData )
	{
		if( const ae::BasicType* basicVarType = varType->AsVarType< ae::BasicType >() )
		{
			return basicVarType->SetVarDataFromString( varData, value );
		}
		else if( const ae::EnumType* enumVarType = varType->AsVarType< ae::EnumType >() )
		{
			return enumVarType->SetVarDataFromString( varData, value );
		}
		else if( const ae::PointerType* pointerVarType = varType->AsVarType< ae::PointerType >() )
		{
			StringToObjectPointerFn fn = []( const void* userData, const char* pointerVal, ae::Object** objOut ) -> bool
			{
				const ae::ClassVar::Serializer* serializer = (const ae::ClassVar::Serializer*)userData;
				AE_ASSERT( serializer );
				return serializer->StringToObjectPointer( pointerVal, objOut );
			};
			return pointerVarType->SetRefFromString( varData, value, fn, _Globals::Get()->varSerializer );
		}
	}
	return false;
}

const ae::ClassType& ae::ClassVar::GetClassType() const
{
	const ae::ClassType* type = m_owner.GetClassType();
	AE_ASSERT_MSG( type, "Member variable # has no class type", m_name );
	return *type;

}
const ae::Type& ae::ClassVar::GetOuterVarType() const
{
	AE_ASSERT( m_varType.Get() );
	return *m_varType.Get();
}
bool ae::ClassVar::HasProperty( const char* prop ) const { return GetPropertyIndex( prop ) >= 0; }
int32_t ae::ClassVar::GetPropertyIndex( const char* prop ) const { return m_props.GetIndex( prop ); }
int32_t ae::ClassVar::GetPropertyCount() const { return m_props.Length(); }
const char* ae::ClassVar::GetPropertyName( int32_t propIndex ) const { return m_props.GetKey( propIndex ).c_str(); }
uint32_t ae::ClassVar::GetPropertyValueCount( int32_t propIndex ) const { return m_props.GetValue( propIndex ).Length(); }
uint32_t ae::ClassVar::GetPropertyValueCount( const char* propName ) const { auto* props = m_props.TryGet( propName ); return props ? props->Length() : 0; }
const char* ae::ClassVar::GetPropertyValue( int32_t propIndex, uint32_t valueIndex ) const
{
	const auto* vals = ( propIndex < m_props.Length() ) ? &m_props.GetValue( propIndex ) : nullptr;
	return ( vals && valueIndex < vals->Length() ) ? (*vals)[ valueIndex ].c_str() : "";
}
const char* ae::ClassVar::GetPropertyValue( const char* propName, uint32_t valueIndex ) const
{
	const auto* vals = m_props.TryGet( propName );
	return ( vals && valueIndex < vals->Length() ) ? (*vals)[ valueIndex ].c_str() : "";
}
ae::ClassVar::_TypePointer::_TypePointer( const ae::Type& _varType )
{
	if( const ae::ClassType* classType = _varType.AsVarType< ae::ClassType >() )
	{
		type = Class;
		typeId = classType->GetTypeId();
	}
	else
	{
		type = Static;
		this->varType = &_varType;
	}
}
ae::ClassVar::_TypePointer::_TypePointer( const ae::ClassType* type ) : type( type ? Class : Null ), typeId( type ? type->GetTypeId() : kInvalidTypeId ) {}
ae::ClassVar::_TypePointer::_TypePointer( const ae::TypeId typeId ) : type( ( typeId != kInvalidTypeId ) ? Class : Null ), typeId( typeId ) {}
const ae::ClassType* ae::ClassVar::_TypePointer::GetClassType() const
{
	return ( type == Class ) ? ae::GetClassTypeById( typeId ) : nullptr;
}
const ae::Type* ae::ClassVar::_TypePointer::Get() const
{
	switch( type )
	{
		case Static: return varType;
		case Class: return ae::GetClassTypeById( typeId );
		default: return nullptr;
	}
}
void ae::ClassVar::m_AddProp( const char* prop, const char* value )
{
	AE_ASSERT_MSG( m_props.Length() < m_props.Size(), "Set/increase AE_MAX_META_PROP_LIST_LENGTH_CONFIG (Currently: #)", m_props.Size() );
	auto* props = m_props.TryGet( prop );
	if( !props )
	{
		props = &m_props.Set( prop, {} );
	}
	if( value && value[ 0 ] ) // 'm_props' will have an empty array for properties when no value is specified
	{
		props->Append( value );
	}
}

uint32_t ae::ClassType::GetVarCount( bool parents ) const
{
	const ae::ClassType* parent = ( parents ? GetParentType() : nullptr );
	return m_vars.Length() + ( parent ? parent->GetVarCount( parents ) : 0 );
}

const ae::ClassVar* ae::ClassType::GetVarByIndex( uint32_t i, bool parents ) const
{
	if( !parents )
	{
		return m_vars[ i ];
	}
	// @HACK: This whole function should be re-written to avoid recreating this array and all of this recursion
	ae::Array< const ae::ClassVar*, kMaxMetaVars > vars;
	auto fn = [&vars]( auto fn, const ae::ClassType* type ) -> void
	{
		if( const ae::ClassType* parent = type->GetParentType() )
		{
			fn( fn, parent );
		}
		for( const ae::ClassVar* v : type->m_vars )
		{
			vars.Append( v );
		}
	};
	fn( fn, this );
	return vars[ i ];
}

const ae::ClassVar* ae::ClassType::GetVarByName( const char* name, bool parents ) const
{
	int32_t i = m_vars.FindFn( [name]( const ae::ClassVar* v )
	{
		return v->m_name == name;
	} );
	if( i >= 0 )
	{
		return m_vars[ i ];
	}
	else if( const ae::ClassType* parent = ( parents ? GetParentType() : nullptr ) )
	{
		return parent->GetVarByName( name, parents );
	}
	return nullptr;
}

const ae::ClassType* ae::ClassType::GetParentType() const
{
	return GetClassTypeByName( m_parent.c_str() );
}

bool ae::ClassType::IsType( const ae::ClassType* otherType ) const
{
	AE_ASSERT( otherType );
	for( const ae::ClassType* baseType = this; baseType; baseType = baseType->GetParentType() )
	{
		if( baseType == otherType )
		{
			return true;
		}
	}
	return false;
}

// @TODO: Remove
ae::BasicType::Type ae::ClassVar::GetType() const
{
	if( const ae::BasicType* basicType = m_HACK_FindInnerVarType< ae::BasicType >() )
	{
		return basicType->GetType();
	}
	else if( m_HACK_FindInnerVarType< ae::EnumType >() )
	{
		return ae::BasicType::Enum;
	}
	else if( const ae::PointerType* pointerType = m_HACK_FindInnerVarType< ae::PointerType >() )
	{
		return pointerType->GetBasicType();
	}
	else if( m_HACK_FindInnerVarType< ae::ClassType >() )
	{
		return ae::BasicType::Class;
	}
	return ae::BasicType::None;
}

const char* ae::ClassVar::GetTypeName() const
{
	switch( GetType() )
	{
		case ae::BasicType::UInt8: return "uint8_t";
		case ae::BasicType::UInt16: return "uint16_t";
		case ae::BasicType::UInt32: return "uint32_t";
		case ae::BasicType::UInt64: return "uint64_t";
		case ae::BasicType::Int8: return "int8_t";
		case ae::BasicType::Int16: return "int16_t";
		case ae::BasicType::Int32: return "int32_t";
		case ae::BasicType::Int64: return "int64_t";
		case ae::BasicType::Int2: return "ae::Int2";
		case ae::BasicType::Int3: return "ae::Int3";
		case ae::BasicType::Bool: return "bool";
		case ae::BasicType::Float: return "float";
		case ae::BasicType::Double: return "double";
		case ae::BasicType::Vec2: return "ae::Vec2";
		case ae::BasicType::Vec3: return "ae::Vec3";
		case ae::BasicType::Vec4: return "ae::Vec4";
		case ae::BasicType::Color: return "ae::Color";
		case ae::BasicType::Matrix4: return "ae::Matrix4";
		case ae::BasicType::String: return "String";
		case ae::BasicType::Class:
		{
			const ae::ClassType* type = GetSubType();
			AE_ASSERT( type );
			return type->GetName();
		}
		case ae::BasicType::Enum:
		{
			const ae::EnumType* enumType = GetEnumType();
			AE_ASSERT( enumType );
			return enumType->GetName();
		}
		case ae::BasicType::Pointer:
		case ae::BasicType::CustomRef:
		{
			return "ref";
		}
		case ae::BasicType::None:
		{
			return "none_type";
		}
	}
	AE_FAIL();
	return "unknown_type";
}

// @TODO: Remove
const class ae::EnumType* ae::ClassVar::GetEnumType() const
{
	const ae::EnumType* enumType = m_HACK_FindInnerVarType< ae::EnumType >();
	return enumType ? ae::GetEnumType( enumType->GetName() ) : nullptr;
}

// @TODO: Remove
const ae::ClassType* ae::ClassVar::GetSubType() const
{
	const ae::ClassType* classVarType = m_HACK_FindInnerVarType< ae::ClassType >();
	return classVarType ? ae::GetClassTypeById( classVarType->GetTypeId() ) : nullptr;
}

// @TODO: Remove
bool ae::ClassVar::IsArray() const
{
	return GetOuterVarType().AsVarType< ae::ArrayType >();
}

// @TODO: Remove
bool ae::ClassVar::IsArrayFixedLength() const
{
	const ae::ArrayType* arrayAdapter = GetOuterVarType().AsVarType< ae::ArrayType >();
	return arrayAdapter && arrayAdapter->IsFixedLength();
}

// @TODO: Remove
uint32_t ae::ClassVar::SetArrayLength( ae::Object* obj, uint32_t length ) const
{
	AE_ASSERT( length != ae::MaxValue< uint32_t >() );
	if( !obj )
	{
		return 0;
	}
	const ae::ArrayType* arrayAdapter = GetOuterVarType().AsVarType< ae::ArrayType >();
	return arrayAdapter ? arrayAdapter->Resize( { this, obj }, length ) : 0;
}

// @TODO: Remove
uint32_t ae::ClassVar::GetArrayLength( const ae::Object* obj ) const
{
	if( !obj )
	{
		return 0;
	}
	const ae::ArrayType* arrayAdapter = GetOuterVarType().AsVarType< ae::ArrayType >();
	return arrayAdapter ? arrayAdapter->GetLength( { this, obj } ) : 0;
}

// @TODO: Remove
uint32_t ae::ClassVar::GetArrayMaxLength() const
{
	const ae::ArrayType* arrayAdapter = GetOuterVarType().AsVarType< ae::ArrayType >();
	return arrayAdapter ? arrayAdapter->GetMaxLength() : 0;
}

//------------------------------------------------------------------------------
// Internal ae::Object functions
//------------------------------------------------------------------------------
ae::TypeId ae::GetObjectTypeId( const ae::Object* obj )
{
	if( obj )
	{
		AE_DEBUG_ASSERT_MSG( obj->_metaTypeId, "Object has no meta type, it's possible this object has not been constructed." );
		return obj->_metaTypeId;
	}
	return ae::kInvalidTypeId;
}

ae::TypeId ae::GetTypeIdFromName( const char* name )
{
	// @TODO: Look into https://en.cppreference.com/w/cpp/types/type_info/hash_code
	return name[ 0 ] ? ae::Hash32().HashString( name ).Get() : ae::kInvalidTypeId;
}

//------------------------------------------------------------------------------
// ae::EnumType member functions
//------------------------------------------------------------------------------
ae::EnumType::EnumType( const char* name, const char* prefix, uint32_t size, bool isSigned ) :
	m_name( name ),
	m_prefix( prefix ),
	m_size( size ),
	m_isSigned( isSigned )
{}
int32_t ae::EnumType::GetValueByIndex( int32_t index ) const { return m_enumValueToName.GetKey( index ); }
std::string ae::EnumType::GetNameByIndex( int32_t index ) const { return m_enumValueToName.GetValue( index ); }
uint32_t ae::EnumType::Length() const { return m_enumValueToName.Length(); }

void ae::EnumType::m_AddValue( const char* name, int32_t value )
{
	AE_ASSERT_MSG( m_enumValueToName.Length() < m_enumValueToName.Size(), "Set/increase AE_MAX_META_ENUM_VALUES_CONFIG (Currently: #)", m_enumValueToName.Size() );
	m_enumValueToName.Set( value, name );
	m_enumNameToValue.Set( name, value );
}

//------------------------------------------------------------------------------
// ae::Type member functions
//------------------------------------------------------------------------------
bool ae::Type::IsSameBaseVarType( const ae::Type& other ) const
{
	return ( GetBaseVarTypeId() == other.GetBaseVarTypeId() );
}

bool ae::Type::IsSameExactVarType( const ae::Type& other ) const
{
	return ( GetExactVarTypeId() == other.GetExactVarTypeId() );
}

//------------------------------------------------------------------------------
// ae::BasicType member functions
//------------------------------------------------------------------------------
std::string ae::BasicType::GetVarDataAsString( ae::ConstDataPointer _varData ) const
{
	const void* varData = _varData.Get( this );
	if( !varData )
	{
		return "";
	}
	switch( GetType() )
	{
		case BasicType::String:
			switch( GetSize() )
			{
				case 16: return reinterpret_cast< const ae::Str16* >( varData )->c_str();
				case 32: return reinterpret_cast< const ae::Str32* >( varData )->c_str();
				case 64: return reinterpret_cast< const ae::Str64* >( varData )->c_str();
				case 128: return reinterpret_cast< const ae::Str128* >( varData )->c_str();
				case 256: return reinterpret_cast< const ae::Str256* >( varData )->c_str();
				case 512: return reinterpret_cast< const ae::Str512* >( varData )->c_str();
				default: return "";
			}
		case BasicType::UInt8: return ae::Str32::Format( "#", (uint32_t)*reinterpret_cast< const uint8_t* >( varData ) ).c_str(); // Prevent char formatting
		case BasicType::UInt16: return ae::Str32::Format( "#", *reinterpret_cast< const uint16_t* >( varData ) ).c_str();
		case BasicType::UInt32: return ae::Str32::Format( "#", *reinterpret_cast< const uint32_t* >( varData ) ).c_str();
		case BasicType::UInt64: return ae::Str32::Format( "#", *reinterpret_cast< const uint64_t* >( varData ) ).c_str();
		case BasicType::Int8: return ae::Str32::Format( "#", (int32_t)*reinterpret_cast< const int8_t* >( varData ) ).c_str(); // Prevent char formatting
		case BasicType::Int16: return ae::Str32::Format( "#", *reinterpret_cast< const int16_t* >( varData ) ).c_str();
		case BasicType::Int32: return ae::Str32::Format( "#", *reinterpret_cast< const int32_t* >( varData ) ).c_str();
		case BasicType::Int64: return ae::Str32::Format( "#", *reinterpret_cast< const int64_t* >( varData ) ).c_str();
		case BasicType::Int2: return ae::Str256::Format( "#", *reinterpret_cast<const ae::Int2*>( varData ) ).c_str();
		case BasicType::Int3: return ae::Str256::Format( "#", *reinterpret_cast<const ae::Int3*>( varData ) ).c_str();
		case BasicType::Bool: return ae::Str32::Format( "#", *reinterpret_cast< const bool* >( varData ) ).c_str();
		case BasicType::Float: return ae::Str32::Format( "#", *reinterpret_cast< const float* >( varData ) ).c_str();
		case BasicType::Double: return ae::Str32::Format( "#", *reinterpret_cast< const double* >( varData ) ).c_str();
		case BasicType::Vec2: return ae::Str256::Format( "#", *reinterpret_cast< const ae::Vec2* >( varData ) ).c_str();
		case BasicType::Vec3: return ae::Str256::Format( "#", *reinterpret_cast< const ae::Vec3* >( varData ) ).c_str();
		case BasicType::Vec4: return ae::Str256::Format( "#", *reinterpret_cast< const ae::Vec4* >( varData ) ).c_str();
		case BasicType::Matrix4: return ae::Str256::Format( "#", *reinterpret_cast< const ae::Matrix4* >( varData ) ).c_str();
		case BasicType::Color: return ae::Str256::Format( "#", *reinterpret_cast< const ae::Color* >( varData ) ).c_str();
		case BasicType::Class: AE_FAIL(); break; // @TODO: Remove
		case BasicType::Enum: AE_FAIL(); break; // @TODO: Remove
		case BasicType::Pointer: AE_FAIL(); break; // @TODO: Remove
		case BasicType::CustomRef: AE_FAIL(); break; // @TODO: Remove
		case BasicType::None: AE_FAIL(); break; // @TODO: Remove
	}
	return "";
}

bool ae::BasicType::SetVarDataFromString( ae::DataPointer _varData, const char* value ) const
{
	if( !value ) // Only check for null here. Zero length strings are valid depending on the type.
	{
		return false;
	}
	void* varData = _varData.Get( this );
	if( !varData )
	{
		return false;
	}
	const uint32_t typeSize = GetSize();
	switch( GetType() )
	{
		case BasicType::String:
		{
			switch( typeSize )
			{
#define CASE_STRING( _size ) \
				case _size:\
				{\
					ae::Str##_size* str = (ae::Str##_size*)varData;\
					if( strlen( value ) > str->MaxLength() ) { return false; }\
					*str = value;\
					return true;\
				}
				CASE_STRING( 16 )
				CASE_STRING( 32 )
				CASE_STRING( 64 )
				CASE_STRING( 128 )
				CASE_STRING( 256 )
				CASE_STRING( 512 )
#undef CASE_STRING
				default:
				{
					return false;
				}
			}
		}
		case BasicType::UInt8:
		{
			AE_ASSERT( typeSize == sizeof(uint8_t) );
			*(uint8_t*)varData = ae::FromString< uint8_t >( value, 0 );
			return true;
		}
		case BasicType::UInt16:
		{
			AE_ASSERT( typeSize == sizeof(uint16_t) );
			*(uint16_t*)varData = ae::FromString< uint16_t >( value, 0 );
			return true;
		}
		case BasicType::UInt32:
		{
			AE_ASSERT( typeSize == sizeof(uint32_t) );
			*(uint32_t*)varData = ae::FromString< uint32_t >( value, 0 );
			return true;
		}
		case BasicType::UInt64:
		{
			AE_ASSERT( typeSize == sizeof(uint64_t) );
			*(uint64_t*)varData = ae::FromString< uint64_t >( value, 0 );
			return true;
		}
		case BasicType::Int8:
		{
			AE_ASSERT( typeSize == sizeof(int8_t) );
			*(int8_t*)varData = ae::FromString< int8_t >( value, 0 );
			return true;
		}
		case BasicType::Int16:
		{
			AE_ASSERT( typeSize == sizeof(int16_t) );
			*(int16_t*)varData = ae::FromString< int16_t >( value, 0 );
			return true;
		}
		case BasicType::Int32:
		{
			AE_ASSERT( typeSize == sizeof(int32_t) );
			*(int32_t*)varData = ae::FromString< int32_t >( value, 0 );
			return true;
		}
		case BasicType::Int64:
		{
			AE_ASSERT( typeSize == sizeof(int64_t) );
			*(int64_t*)varData = ae::FromString< int64_t >( value, 0 );
			return true;
		}
		case BasicType::Int2:
		{
			AE_ASSERT( typeSize == sizeof( ae::Int2 ) );
			*(ae::Int2*)varData = ae::FromString< ae::Int2 >( value, ae::Int2( 0.0f ) );
			return true;
		}
		case BasicType::Int3:
		{
			AE_ASSERT( typeSize == sizeof( ae::Int3 ) );
			*(ae::Int3*)varData = ae::FromString< ae::Int3 >( value, ae::Int3( 0.0f ) );
			return true;
		}
		case BasicType::Bool:
		{
			*(bool*)varData = ae::FromString< bool >( value, false );
			return true;
		}
		case BasicType::Float:
		{
			AE_ASSERT( typeSize == sizeof(float) );
			*(float*)varData = ae::FromString< float >( value, 0.0f );
			return true;
		}
		case BasicType::Double:
		{
			AE_ASSERT( typeSize == sizeof(double) );
			*(double*)varData = ae::FromString< double >( value, 0.0 );
			return true;
		}
		case BasicType::Vec2:
		{
			AE_ASSERT( typeSize == sizeof(ae::Vec2) );
			*(ae::Vec2*)varData = ae::FromString< ae::Vec2 >( value, ae::Vec2( 0.0f ) );
			return true;
		}
		case BasicType::Vec3:
		{
			AE_ASSERT( typeSize == sizeof(ae::Vec3) );
			*(ae::Vec3*)varData = ae::FromString< ae::Vec3 >( value, ae::Vec3( 0.0f ) );
			return true;
		}
		case BasicType::Vec4:
		{
			AE_ASSERT( typeSize == sizeof(ae::Vec4) );
			*(ae::Vec4*)varData = ae::FromString< ae::Vec4 >( value, ae::Vec4( 0.0f ) );
			return true;
		}
		case BasicType::Matrix4:
		{
			AE_ASSERT( typeSize == sizeof(ae::Matrix4) );
			*(ae::Matrix4*)varData = ae::FromString< ae::Matrix4 >( value, ae::Matrix4::Identity() );
			return true;
		}
		case BasicType::Color:
		{
			AE_ASSERT( typeSize == sizeof(ae::Color) );
			*(ae::Color*)varData = ae::FromString( value, ae::Color::Black() );
			return true;
		}
		case BasicType::Class: AE_FAIL(); break; // @TODO: Remove
		case BasicType::Enum: AE_FAIL(); break; // @TODO: Remove
		case BasicType::Pointer: AE_FAIL(); break; // @TODO: Remove
		case BasicType::CustomRef: AE_FAIL(); break; // @TODO: Remove
		case BasicType::None: AE_FAIL(); break; // @TODO: Remove
	}
	return false;
}

//------------------------------------------------------------------------------
// ae::EnumType member functions
//------------------------------------------------------------------------------
std::string ae::EnumType::GetVarDataAsString( ae::ConstDataPointer _varData ) const
{
	const void* varData = _varData.Get( this );
	if( !varData )
	{
		return "";
	}
	
	// @NOTE: Enums with very large or small values (outside the range of int32) are not currently supported
	int32_t value = 0;
	if( TypeIsSigned() )
	{
		switch( TypeSize() )
		{
			case 1: value = *reinterpret_cast< const int8_t* >( varData ); break;
			case 2: value = *reinterpret_cast< const int16_t* >( varData ); break;
			case 4: value = *reinterpret_cast< const int32_t* >( varData ); break;
			case 8:
			{
				auto v = *reinterpret_cast< const int64_t* >( varData );
				AE_DEBUG_ASSERT( v <= (int64_t)INT32_MAX );
				AE_DEBUG_ASSERT( v >= (int64_t)INT32_MIN );
				value = (int32_t)v;
				break;
			}
			default: AE_FAIL();
		}
	}
	else
	{
		switch( TypeSize() )
		{
			case 1: value = *reinterpret_cast< const uint8_t* >( varData ); break;
			case 2: value = *reinterpret_cast< const uint16_t* >( varData ); break;
			case 4:
			{
				auto v = *reinterpret_cast< const uint32_t* >( varData );
				AE_DEBUG_ASSERT( v <= (uint32_t)INT32_MAX );
				value = v;
				break;
			}
			case 8:
			{
				auto v = *reinterpret_cast< const uint64_t* >( varData );
				AE_DEBUG_ASSERT( v <= (uint64_t)INT32_MAX );
				value = (int32_t)v;
				break;
			}
			default: AE_FAIL();
		}
	}
	return GetNameByValue( value );
}

bool ae::EnumType::SetVarDataFromString( ae::DataPointer _varData, const char* value ) const
{
	void* varData = _varData.Get( this );
	if( !value || !value[ 0 ] || !varData )
	{
		return false;
	}
	if( TypeIsSigned() )
	{
		switch( TypeSize() )
		{
			case 1: return GetValueFromString( value, reinterpret_cast< int8_t* >( varData ) );
			case 2: return GetValueFromString( value, reinterpret_cast< int16_t* >( varData ) );
			case 4: return GetValueFromString( value, reinterpret_cast< int32_t* >( varData ) );
			case 8: return GetValueFromString( value, reinterpret_cast< int64_t* >( varData ) );
			default: AE_FAIL(); return false;
		}
	}
	else
	{
		switch( TypeSize() )
		{
			case 1: return GetValueFromString( value, reinterpret_cast< uint8_t* >( varData ) );
			case 2: return GetValueFromString( value, reinterpret_cast< uint16_t* >( varData ) );
			case 4: return GetValueFromString( value, reinterpret_cast< uint32_t* >( varData ) );
			case 8: return GetValueFromString( value, reinterpret_cast< uint64_t* >( varData ) );
			default: AE_FAIL(); return false;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
// ae::PointerType
//------------------------------------------------------------------------------
ae::DataPointer ae::PointerType::Dereference( ae::ConstDataPointer varData ) const
{
	if( void*const* data = static_cast< void*const* >( varData.Get( this ) ) )
	{
		return { GetInnerVarType(), *data };
	}
	return {};
}

//------------------------------------------------------------------------------
// ae::ClassType member functions
//------------------------------------------------------------------------------
const ae::ClassType* ae::ClassType::GetClassType( ae::ConstDataPointer varData ) const
{
	if( const ae::Object* data = static_cast< const ae::Object* >( varData.Get( this ) ) )
	{
		return ae::GetClassTypeFromObject( data );
	}
	return nullptr;
}

ae::DataPointer ae::ClassType::GetVarData( const ae::ClassVar* var, ae::DataPointer varData ) const
{
	if( ae::Object* data = static_cast< ae::Object* >( varData.Get( this ) ) )
	{
		return { var, data };
	}
	return {};
}

ae::ConstDataPointer ae::ClassType::GetVarData( const ae::ClassVar* var, ae::ConstDataPointer varData ) const
{
	if( const ae::Object* data = static_cast< const ae::Object* >( varData.Get( this ) ) )
	{
		return { var, data };
	}
	return {};
}

//------------------------------------------------------------------------------
// ae::ClassType member functions
//------------------------------------------------------------------------------
ae::TypeId ae::ClassType::GetId() const { return m_id; }
bool ae::ClassType::HasProperty( const char* property ) const { return GetPropertyIndex( property ) >= 0; }
const ae::ClassType* ae::ClassType::GetTypeWithProperty( const char* property ) const
{
	const ae::ClassType* result = this;
	while( result )
	{
		if( result->HasProperty( property ) )
		{
			break;
		}
		result = result->GetParentType();
	}
	return result;
}
int32_t ae::ClassType::GetPropertyIndex( const char* prop ) const { return m_props.GetIndex( prop ); }
int32_t ae::ClassType::GetPropertyCount() const { return m_props.Length(); }
const char* ae::ClassType::GetPropertyName( int32_t propIndex ) const
{
	AE_ASSERT( 0 <= propIndex && propIndex < m_props.Length() );
	return m_props.GetKey( propIndex ).c_str();
}
uint32_t ae::ClassType::GetPropertyValueCount( int32_t propIndex ) const
{
	AE_ASSERT( 0 <= propIndex && propIndex < m_props.Length() );
	return m_props.GetValue( propIndex ).Length();
}
uint32_t ae::ClassType::GetPropertyValueCount( const char* propName ) const { auto* props = m_props.TryGet( propName ); return props ? props->Length() : 0; }
const char* ae::ClassType::GetPropertyValue( int32_t propIndex, uint32_t valueIndex ) const
{
	const auto* vals = ( propIndex < m_props.Length() ) ? &m_props.GetValue( propIndex ) : nullptr;
	return ( vals && valueIndex < vals->Length() ) ? (*vals)[ valueIndex ].c_str() : "";
}
const char* ae::ClassType::GetPropertyValue( const char* propName, uint32_t valueIndex ) const
{
	const auto* vals = m_props.TryGet( propName );
	return ( vals && valueIndex < vals->Length() ) ? (*vals)[ valueIndex ].c_str() : "";
}
void ae::ClassType::PatchVTable( ae::Object* obj ) const
{
	if( obj )
	{
		// @TODO: Get this without instantiating? At least cache it...
		AE_ASSERT( obj->GetTypeId() == GetId() );
		ae::Object* temp = (ae::Object*)ae::Allocate( AE_ALLOC_TAG_FIXME, GetSize(), GetAlignment() );
		New( temp );
		void* vtable = *(void**)temp;
		temp->~Object();
		ae::Free( temp );
		memcpy( (void*)obj, &vtable, sizeof(void*) );
	}
}
uint32_t ae::ClassType::GetSize() const { return m_size; }
uint32_t ae::ClassType::GetAlignment() const { return m_align; }
const char* ae::ClassType::GetName() const { return m_name.c_str(); }
bool ae::ClassType::HasNew() const { return m_placementNew; }
bool ae::ClassType::IsAbstract() const { return m_isAbstract; }
bool ae::ClassType::IsPolymorphic() const { return m_isPolymorphic; }
bool ae::ClassType::IsDefaultConstructible() const { return m_isDefaultConstructible; }
bool ae::ClassType::IsFinal() const { return m_isFinal; }
const char* ae::ClassType::GetParentTypeName() const { return m_parent.c_str(); }

void ae::ClassType::m_AddProp( const char* prop, const char* value )
{
	auto* props = m_props.TryGet( prop );
	if( !props )
	{
		props = &m_props.Set( prop, {} );
	}
	if( value && value[ 0 ] ) // 'm_props' will have an empty array for properties when no value is specified
	{
		props->Append( value );
	}
}

void ae::ClassType::m_AddVar( const ae::ClassVar* var )
{
	AE_ASSERT_MSG( m_vars.Length() < m_vars.Size(), "Set/increase AE_MAX_META_VARS_CONFIG (Currently: #)", m_vars.Size() );
	m_vars.Append( var );
	std::sort( m_vars.begin(), m_vars.end(), []( const ae::ClassVar* a, const ae::ClassVar* b )
	{
		return a->GetOffset() < b->GetOffset();
	} );
}

//------------------------------------------------------------------------------
// ae::AttributeList member functions
//------------------------------------------------------------------------------
void ae::AttributeList::m_Add( Attribute* attribute )
{
	AE_ASSERT_MSG( m_attributes.Length() < m_attributes.Size(), "Set/increase AE_MAX_META_ATTRIBUTES_CONFIG (Currently: #)", m_attributes.Size() );
	
	m_attributes.Append( attribute );
	std::stable_sort( m_attributes.begin(), m_attributes.end(), []( const ae::Attribute* a, const ae::Attribute* b )
	{
		return a->_metaTypeId < b->_metaTypeId;
	} );

	m_attributeTypes.Clear();
	for( uint32_t i = 0; i < m_attributes.Length(); i++ )
	{
		const Attribute* attribute = m_attributes[ i ];
		const ae::TypeId attributeType = attribute->GetTypeId();
		if( _Info* info = m_attributeTypes.TryGet( attributeType ) )
		{
			info->count++;
		}
		else
		{
			m_attributeTypes.Set( attributeType, { i, 1 } );
		}
	}

	_Globals::Get()->metaCacheSeq++;
}

#endif // AE_MAIN

//------------------------------------------------------------------------------
// Warnings
//------------------------------------------------------------------------------
#ifdef AE_POP_WARNINGS
	#if _AE_WINDOWS_
		#pragma warning( pop )
	#elif _AE_APPLE_
		#pragma clang diagnostic pop
	#endif
	#undef AE_POP_WARNINGS
#endif
