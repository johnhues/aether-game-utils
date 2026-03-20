//------------------------------------------------------------------------------
// MathTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#include <catch2/catch_test_macros.hpp>
#include "aether.h"

//------------------------------------------------------------------------------
// Math test helpers
//------------------------------------------------------------------------------
bool IsCloseEnough( float a, float b, float epsilon = 0.001f )
{
	return std::abs( a - b ) < epsilon;
}

//------------------------------------------------------------------------------
// ae::Min/Max tests
//------------------------------------------------------------------------------
TEST_CASE( "Basic min/max value", "[ae::Min/Max]" )
{
	REQUIRE( ae::Min( 1, 2 ) == 1 );
	REQUIRE( ae::Min( 6, 4, 3, -8, 10, -13 ) == -13 );
	REQUIRE( ae::Max( 1, 2 ) == 2 );
	REQUIRE( ae::Max( 6, 4, 3, -8, 10, -13 ) == 10 );
}

TEST_CASE( "Min/max value with different types", "[ae::Min/Max]" )
{
	REQUIRE( std::is_same< decltype(ae::Min( 1, 2 )), int >() );
	REQUIRE( std::is_same< decltype(ae::Max( 1, 2 )), int >() );
	REQUIRE( std::is_same< decltype(ae::Min( 1, 2.3f )), float >() );
	REQUIRE( std::is_same< decltype(ae::Max( 1, 2.3f )), float >() );
	REQUIRE( std::is_same< decltype(ae::Min( 6, 4.5, 3.2f, -8, 10.1, -13 )), double >() );
	REQUIRE( std::is_same< decltype(ae::Max( 6, 4.5, 3.2f, -8, 10.1, -13 )), double >() );

	REQUIRE( ae::Min( 1, 2.3f ) == 1 );
	REQUIRE( ae::Min( 6, 4.5, 3.2f, -8, 10.1, -13 ) == -13 );
	REQUIRE( ae::Max( 1, 2.3f ) == 2.3f );
	REQUIRE( ae::Max( 6, 4.5, 3.2f, -8, 10.1, -13 ) == 10.1 );
}

TEST_CASE( "Min/max vectors", "[ae::Min/Max]" )
{
	REQUIRE( ae::Min( ae::Vec2( 1, 4 ), ae::Vec2( 3, 2 ) ) == ae::Vec2( 1, 2 ) );
	REQUIRE( ae::Min( ae::Vec3( 1, 4, 3 ), ae::Vec3( 3, 2, 1 ) ) == ae::Vec3( 1, 2, 1 ) );
	REQUIRE( ae::Min( ae::Vec4( 1, 4, 3, 2 ), ae::Vec4( 3, 2, 1, 4 ) ) == ae::Vec4( 1, 2, 1, 2 ) );
	REQUIRE( ae::Max( ae::Vec2( 1, 4 ), ae::Vec2( 3, 2 ) ) == ae::Vec2( 3, 4 ) );
	REQUIRE( ae::Max( ae::Vec3( 1, 4, 3 ), ae::Vec3( 3, 2, 1 ) ) == ae::Vec3( 3, 4, 3 ) );
	REQUIRE( ae::Max( ae::Vec4( 1, 4, 3, 2 ), ae::Vec4( 3, 2, 1, 4 ) ) == ae::Vec4( 3, 4, 3, 4 ) );

	REQUIRE( ae::Min( ae::Vec2( 1, 4 ), ae::Vec2( 3, 2 ), ae::Vec2( 5, 6 ) ) == ae::Vec2( 1, 2 ) );
	REQUIRE( ae::Min( ae::Vec3( 1, 4, 3 ), ae::Vec3( 3, 2, 1 ), ae::Vec3( 2, 3, 4 ) ) == ae::Vec3( 1, 2, 1 ) );
	REQUIRE( ae::Min( ae::Vec4( 1, 4, 3, 2 ), ae::Vec4( 3, 2, 1, 4 ), ae::Vec4( 2, 7, 4, 1 ) ) == ae::Vec4( 1, 2, 1, 1 ) );
	REQUIRE( ae::Max( ae::Vec2( 1, 4 ), ae::Vec2( 3, 2 ), ae::Vec2( 5, 6 ) ) == ae::Vec2( 5, 6 ) );
	REQUIRE( ae::Max( ae::Vec3( 1, 4, 3 ), ae::Vec3( 3, 2, 1 ), ae::Vec3( 2, 3, 4 ) ) == ae::Vec3( 3, 4, 4 ) );
	REQUIRE( ae::Max( ae::Vec4( 1, 4, 3, 2 ), ae::Vec4( 3, 2, 1, 4 ), ae::Vec4( 2, 7, 4, 1 ) ) == ae::Vec4( 3, 7, 4, 4 ) );
}

TEST_CASE( "Delerp function", "[ae::Delerp]" )
{
	REQUIRE( IsCloseEnough( ae::Delerp( 0.0f, 1.0f, 0.25f ), 0.25f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 0.0f, 1.0f, 0.5f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 0.0f, 1.0f, 0.75f ), 0.75f ) );
	
	REQUIRE( IsCloseEnough( ae::Delerp( 1.0f, 0.0f, 0.25f ), 0.75f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 1.0f, 0.0f, 0.5f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 1.0f, 0.0f, 0.75f ), 0.25f ) );

	REQUIRE( IsCloseEnough( ae::Delerp( 0.0f, 10.0f, 2.5f ), 0.25f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 0.0f, 10.0f, 5.0f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 0.0f, 10.0f, 7.5f ), 0.75f ) );
	
	REQUIRE( IsCloseEnough( ae::Delerp( 10.0f, 0.0f, 2.5f ), 0.75f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 10.0f, 0.0f, 5.0f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Delerp( 10.0f, 0.0f, 7.5f ), 0.25f ) );

	REQUIRE( IsCloseEnough( ae::Delerp01( 0.0f, 1.0f, -0.5f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Delerp01( 0.0f, 1.0f, 1.5f ), 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Delerp01( 1.0f, 0.0f, 1.5f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Delerp01( 1.0f, 0.0f, -0.5f ), 1.0f ) );
}

//------------------------------------------------------------------------------
// ae::Matrix4::Determinant tests
//------------------------------------------------------------------------------
TEST_CASE( "Determinant of identity is 1", "[ae::Matrix4::Determinant]" )
{
REQUIRE( IsCloseEnough( ae::Matrix4::Identity().Determinant(), 1.0f ) );
}

TEST_CASE( "Determinant of uniform scale", "[ae::Matrix4::Determinant]" )
{
REQUIRE( IsCloseEnough( ae::Matrix4::Scaling( 2.0f ).Determinant(), 8.0f ) );
}

TEST_CASE( "Determinant of non-uniform scale", "[ae::Matrix4::Determinant]" )
{
REQUIRE( IsCloseEnough( ae::Matrix4::Scaling( ae::Vec3( 2.0f, 3.0f, 4.0f ) ).Determinant(), 24.0f ) );
}

TEST_CASE( "Determinant negated by single negative axis", "[ae::Matrix4::Determinant]" )
{
REQUIRE( IsCloseEnough( ae::Matrix4::Scaling( ae::Vec3( -1.0f, 1.0f, 1.0f ) ).Determinant(), -1.0f ) );
REQUIRE( IsCloseEnough( ae::Matrix4::Scaling( ae::Vec3( 1.0f, -1.0f, 1.0f ) ).Determinant(), -1.0f ) );
REQUIRE( IsCloseEnough( ae::Matrix4::Scaling( ae::Vec3( 1.0f, 1.0f, -1.0f ) ).Determinant(), -1.0f ) );
}

TEST_CASE( "Determinant positive with two negative axes", "[ae::Matrix4::Determinant]" )
{
REQUIRE( IsCloseEnough( ae::Matrix4::Scaling( ae::Vec3( -1.0f, -1.0f, 1.0f ) ).Determinant(), 1.0f ) );
}

TEST_CASE( "Determinant of rotation is 1", "[ae::Matrix4::Determinant]" )
{
const ae::Quaternion q = ae::Quaternion( ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::HalfPi );
REQUIRE( IsCloseEnough( ae::Matrix4::Rotation( q ).Determinant(), 1.0f ) );
}

//------------------------------------------------------------------------------
// ae::Matrix4::GetScale tests
//------------------------------------------------------------------------------
TEST_CASE( "GetScale round-trips positive scale", "[ae::Matrix4::GetScale]" )
{
const ae::Vec3 s( 2.0f, 3.0f, 4.0f );
const ae::Vec3 result = ae::Matrix4::Scaling( s ).GetScale();
REQUIRE( IsCloseEnough( result.x, s.x ) );
REQUIRE( IsCloseEnough( result.y, s.y ) );
REQUIRE( IsCloseEnough( result.z, s.z ) );
}

TEST_CASE( "GetScale round-trips negative z scale", "[ae::Matrix4::GetScale]" )
{
const ae::Vec3 s( 1.0f, 1.0f, -2.0f );
const ae::Vec3 result = ae::Matrix4::Scaling( s ).GetScale();
REQUIRE( IsCloseEnough( result.x, s.x ) );
REQUIRE( IsCloseEnough( result.y, s.y ) );
REQUIRE( IsCloseEnough( result.z, s.z ) );
}

TEST_CASE( "GetScale z negated for negative-determinant matrix", "[ae::Matrix4::GetScale]" )
{
// Single negative x — det < 0, convention negates z
const ae::Vec3 s( -2.0f, 3.0f, 4.0f );
const ae::Vec3 result = ae::Matrix4::Scaling( s ).GetScale();
REQUIRE( result.z < 0.0f );
}

TEST_CASE( "GetScale from TRS round-trip", "[ae::Matrix4::GetScale]" )
{
const ae::Vec3 s( 1.5f, 2.0f, 0.5f );
const ae::Quaternion r = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::QuarterPi );
const ae::Matrix4 m = ae::Matrix4::LocalToWorld( ae::Vec3( 5.0f, 0.0f, -3.0f ), r, s );
const ae::Vec3 result = m.GetScale();
REQUIRE( IsCloseEnough( result.x, s.x ) );
REQUIRE( IsCloseEnough( result.y, s.y ) );
REQUIRE( IsCloseEnough( result.z, s.z ) );
}

//------------------------------------------------------------------------------
// Vector IsCloseEnough helpers
//------------------------------------------------------------------------------
bool IsCloseEnough( ae::Vec2 a, ae::Vec2 b, float e = 0.001f )
{
	return IsCloseEnough( a.x, b.x, e ) && IsCloseEnough( a.y, b.y, e );
}
bool IsCloseEnough( ae::Vec3 a, ae::Vec3 b, float e = 0.001f )
{
	return IsCloseEnough( a.x, b.x, e )
		&& IsCloseEnough( a.y, b.y, e )
		&& IsCloseEnough( a.z, b.z, e );
}
bool IsCloseEnough( ae::Vec4 a, ae::Vec4 b, float e = 0.001f )
{
	return IsCloseEnough( a.x, b.x, e )
		&& IsCloseEnough( a.y, b.y, e )
		&& IsCloseEnough( a.z, b.z, e )
		&& IsCloseEnough( a.w, b.w, e );
}
bool IsCloseEnough( ae::Color a, ae::Color b, float e = 0.001f )
{
	return IsCloseEnough( a.r, b.r, e )
		&& IsCloseEnough( a.g, b.g, e )
		&& IsCloseEnough( a.b, b.b, e )
		&& IsCloseEnough( a.a, b.a, e );
}
bool IsCloseEnoughQ( ae::Quaternion a, ae::Quaternion b, float e = 0.001f )
{
	// quaternions q and -q represent the same rotation
	return ( IsCloseEnough( a.i, b.i, e ) && IsCloseEnough( a.j, b.j, e )
		&& IsCloseEnough( a.k, b.k, e ) && IsCloseEnough( a.r, b.r, e ) )
		|| ( IsCloseEnough( a.i, -b.i, e ) && IsCloseEnough( a.j, -b.j, e )
		&& IsCloseEnough( a.k, -b.k, e ) && IsCloseEnough( a.r, -b.r, e ) );
}

//------------------------------------------------------------------------------
// ae::ScalarMath tests
//------------------------------------------------------------------------------
TEST_CASE( "Pow", "[ae::ScalarMath]" )
{
	REQUIRE( IsCloseEnough( ae::Pow( 2.0f, 3.0f ), 8.0f ) );
	REQUIRE( IsCloseEnough( ae::Pow( 4.0f, 0.5f ), 2.0f ) );
	REQUIRE( IsCloseEnough( ae::Pow( 1.0f, 100.0f ), 1.0f ) );
}

TEST_CASE( "Sqrt", "[ae::ScalarMath]" )
{
	REQUIRE( IsCloseEnough( ae::Sqrt( 4.0f ), 2.0f ) );
	REQUIRE( IsCloseEnough( ae::Sqrt( 9.0f ), 3.0f ) );
	REQUIRE( ae::Sqrt( 0.0f ) == 0.0f );
}

TEST_CASE( "Trig Sin Cos Tan", "[ae::ScalarMath]" )
{
	REQUIRE( ae::Cos( 0.0f ) == 1.0f );
	REQUIRE( IsCloseEnough( ae::Cos( ae::PI ), -1.0f ) );
	REQUIRE( ae::Sin( 0.0f ) == 0.0f );
	REQUIRE( IsCloseEnough( ae::Sin( ae::HalfPi ), 1.0f ) );
	REQUIRE( ae::Tan( 0.0f ) == 0.0f );
	REQUIRE( IsCloseEnough( ae::Tan( ae::PI / 4.0f ), 1.0f ) );
}

TEST_CASE( "Inverse trig Acos Asin Atan Atan2", "[ae::ScalarMath]" )
{
	REQUIRE( ae::Acos( 1.0f ) == 0.0f );
	REQUIRE( IsCloseEnough( ae::Acos( -1.0f ), ae::PI ) );
	REQUIRE( ae::Asin( 0.0f ) == 0.0f );
	REQUIRE( IsCloseEnough( ae::Asin( 1.0f ), ae::HalfPi ) );
	REQUIRE( ae::Atan( 0.0f ) == 0.0f );
	REQUIRE( IsCloseEnough( ae::Atan( 1.0f ), ae::PI / 4.0f ) );
	REQUIRE( IsCloseEnough( ae::Atan2( 1.0f, 1.0f ), ae::PI / 4.0f ) );
	REQUIRE( IsCloseEnough( ae::Atan2( 1.0f, -1.0f ), 3.0f * ae::PI / 4.0f ) );
	REQUIRE( IsCloseEnough( ae::Atan2( -1.0f, -1.0f ), -3.0f * ae::PI / 4.0f ) );
}

TEST_CASE( "Abs", "[ae::ScalarMath]" )
{
	REQUIRE( ae::Abs( -3.0f ) == 3.0f );
	REQUIRE( ae::Abs( 3.0f ) == 3.0f );
	REQUIRE( ae::Abs( 0.0f ) == 0.0f );
	REQUIRE( ae::Abs( int32_t( -3 ) ) == 3 );
	REQUIRE( ae::Abs( int32_t( 3 ) ) == 3 );
}

TEST_CASE( "Ceil Floor Round", "[ae::ScalarMath]" )
{
	REQUIRE( ae::Ceil( 1.1f ) == 2 );
	REQUIRE( ae::Ceil( -1.1f ) == -1 );
	REQUIRE( ae::Ceil( 1.0f ) == 1 );
	REQUIRE( ae::Floor( 1.9f ) == 1 );
	REQUIRE( ae::Floor( -1.1f ) == -2 );
	REQUIRE( ae::Floor( 1.0f ) == 1 );
	REQUIRE( ae::Round( 1.4f ) == 1 );
	REQUIRE( ae::Round( 1.5f ) == 2 );
	REQUIRE( ae::Round( -1.5f ) == -2 ); // (int32_t)(-1.5 - 0.5) = (int32_t)(-2.0) = -2
	REQUIRE( ae::Round( -1.6f ) == -2 );
}

TEST_CASE( "Floor integer division", "[ae::ScalarMath]" )
{
	REQUIRE( ae::Floor( int32_t( 7 ), int32_t( 2 ) ) == 3 );
	REQUIRE( ae::Floor( int32_t( -7 ), int32_t( 2 ) ) == -4 );
	REQUIRE( ae::Floor( int32_t( -6 ), int32_t( 2 ) ) == -3 );
	REQUIRE( ae::Floor( int32_t( 6 ), int32_t( 2 ) ) == 3 );
}

TEST_CASE( "Mod", "[ae::ScalarMath]" )
{
	REQUIRE( ae::Mod( uint32_t( 7 ), uint32_t( 3 ) ) == 1 );
	REQUIRE( ae::Mod( uint32_t( 6 ), uint32_t( 3 ) ) == 0 );
	REQUIRE( ae::Mod( int32_t( -1 ), int32_t( 5 ) ) == 4 );
	REQUIRE( ae::Mod( int32_t( -7 ), int32_t( 3 ) ) == 2 );
	REQUIRE( ae::Mod( int32_t( 7 ), int32_t( 3 ) ) == 1 );
	REQUIRE( IsCloseEnough( ae::Mod( -0.5f, 1.0f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Mod( 1.5f, 1.0f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Mod( -1.0f, 1.0f ), 0.0f ) );
}

TEST_CASE( "NextPowerOfTwo", "[ae::ScalarMath]" )
{
	REQUIRE( ae::NextPowerOfTwo( 0 ) == 0 ); // x-- wraps to UINT32_MAX, +1 wraps back to 0
	REQUIRE( ae::NextPowerOfTwo( 1 ) == 1 );
	REQUIRE( ae::NextPowerOfTwo( 3 ) == 4 );
	REQUIRE( ae::NextPowerOfTwo( 4 ) == 4 );
	REQUIRE( ae::NextPowerOfTwo( 5 ) == 8 );
}

//------------------------------------------------------------------------------
// ae::Clip tests
//------------------------------------------------------------------------------
TEST_CASE( "Clip integer and float", "[ae::Clip]" )
{
	REQUIRE( ae::Clip( 5, 0, 10 ) == 5 );
	REQUIRE( ae::Clip( -1, 0, 10 ) == 0 );
	REQUIRE( ae::Clip( 11, 0, 10 ) == 10 );
	REQUIRE( ae::Clip( 0.5f, 0.0f, 1.0f ) == 0.5f );
	REQUIRE( ae::Clip( 0.0f, 0.0f, 1.0f ) == 0.0f );
	REQUIRE( ae::Clip( 1.0f, 0.0f, 1.0f ) == 1.0f );
}

TEST_CASE( "Clip01", "[ae::Clip]" )
{
	REQUIRE( ae::Clip01( -0.5f ) == 0.0f );
	REQUIRE( ae::Clip01( 0.5f ) == 0.5f );
	REQUIRE( ae::Clip01( 1.5f ) == 1.0f );
	REQUIRE( ae::Clip01( 0.0f ) == 0.0f );
	REQUIRE( ae::Clip01( 1.0f ) == 1.0f );
}

//------------------------------------------------------------------------------
// ae::Lerp tests
//------------------------------------------------------------------------------
TEST_CASE( "Lerp float", "[ae::Lerp]" )
{
	REQUIRE( ae::Lerp( 0.0f, 10.0f, 0.0f ) == 0.0f );
	REQUIRE( ae::Lerp( 0.0f, 10.0f, 1.0f ) == 10.0f );
	REQUIRE( IsCloseEnough( ae::Lerp( 0.0f, 10.0f, 0.5f ), 5.0f ) );
}

TEST_CASE( "Lerp Vec3", "[ae::Lerp]" )
{
	const ae::Vec3 result = ae::Lerp( ae::Vec3( 0,0,0 ), ae::Vec3( 2,4,6 ), 0.5f );
	REQUIRE( IsCloseEnough( result, ae::Vec3( 1,2,3 ) ) );
}

TEST_CASE( "Lerp does not clamp t", "[ae::Lerp]" )
{
	REQUIRE( IsCloseEnough( ae::Lerp( 0.0f, 10.0f, 2.0f ), 20.0f ) );
	REQUIRE( IsCloseEnough( ae::Lerp( 0.0f, 10.0f, -1.0f ), -10.0f ) );
}

TEST_CASE( "CosineInterpolate float", "[ae::Lerp]" )
{
	REQUIRE( ae::CosineInterpolate( 0.0f, 10.0f, 0.0f ) == 0.0f );
	REQUIRE( ae::CosineInterpolate( 0.0f, 10.0f, 1.0f ) == 10.0f );
	REQUIRE( IsCloseEnough( ae::CosineInterpolate( 0.0f, 10.0f, 0.5f ), 5.0f ) );
}

TEST_CASE( "Interpolation Linear and Cosine", "[ae::Lerp]" )
{
	REQUIRE( IsCloseEnough( ae::Interpolation::Linear( 0.0f, 10.0f, 0.5f ), 5.0f ) );
	// Interpolation::Cosine on Vec3 calls Vec3::Lerp internally
	const ae::Vec3 result = ae::Interpolation::Cosine( ae::Vec3( 0,0,0 ), ae::Vec3( 10,0,0 ), 0.5f );
	REQUIRE( IsCloseEnough( result.x, 5.0f ) );
	REQUIRE( IsCloseEnough( result.y, 0.0f ) );
	REQUIRE( IsCloseEnough( result.z, 0.0f ) );
}

//------------------------------------------------------------------------------
// ae::AngleDifference tests
//------------------------------------------------------------------------------
TEST_CASE( "AngleDifference", "[ae::AngleDifference]" )
{
	REQUIRE( IsCloseEnough( ae::AngleDifference( ae::HalfPi, 0.0f ), ae::HalfPi ) );
	REQUIRE( IsCloseEnough( ae::AngleDifference( 0.0f, ae::HalfPi ), -ae::HalfPi ) );
	REQUIRE( IsCloseEnough( ae::AngleDifference( 0.0f, ae::PI * 1.75f ), ae::PI * 0.25f ) );
	REQUIRE( IsCloseEnough( ae::AngleDifference( ae::PI * 1.5f, 0.0f ), -ae::HalfPi ) );
}

TEST_CASE( "LerpAngle", "[ae::AngleDifference]" )
{
	REQUIRE( IsCloseEnough( ae::LerpAngle( 0.0f, ae::HalfPi, 0.5f ), ae::PI / 4.0f ) );
	REQUIRE( IsCloseEnough( ae::LerpAngle( ae::PI * 1.75f, ae::PI * 0.25f, 0.5f ), ae::TWO_PI, 0.01f ) );
}

//------------------------------------------------------------------------------
// ae::DtLerp tests
//------------------------------------------------------------------------------
TEST_CASE( "DtLerp dt zero returns start", "[ae::DtLerp]" )
{
	REQUIRE( ae::DtLerp( 0.0f, 5.0f, 0.0f, 10.0f ) == 0.0f );
	REQUIRE( ae::DtLerp( 3.0f, 10.0f, 0.0f, 99.0f ) == 3.0f );
}

TEST_CASE( "DtLerp snappiness progression", "[ae::DtLerp]" )
{
	// snappiness=0 early-returns start; snappiness>0 converges toward end
	const float r0 = ae::DtLerp( 0.0f, 0.0f, 1.0f, 10.0f );
	REQUIRE( r0 == 0.0f );
	// snappiness=5, dt=1: factor ~= 0, result very close to end
	const float r5 = ae::DtLerp( 0.0f, 5.0f, 1.0f, 10.0f );
	REQUIRE( IsCloseEnough( r5, 10.0f, 0.01f ) );
	// higher snappiness gives result closer to end
	const float r05 = ae::DtLerp( 0.0f, 0.5f, 1.0f, 10.0f );
	REQUIRE( r5 > r05 );
}

TEST_CASE( "DtLerpAngle wraps", "[ae::DtLerp]" )
{
	// DtLerpAngle uses AngleDifference for short path:
	// AngleDifference(1.75*PI, 0) = -0.25*PI, so equivalent to DtLerp toward -0.25*PI
	const float dtla = ae::DtLerpAngle( 0.0f, 5.0f, 1.0f, ae::PI * 1.75f );
	const float dtl = ae::DtLerp( 0.0f, 5.0f, 1.0f, -ae::PI * 0.25f );
	REQUIRE( IsCloseEnough( dtla, dtl ) );
}

TEST_CASE( "DtLerp Vec3", "[ae::DtLerp]" )
{
	const ae::Vec3 result = ae::DtLerp( ae::Vec3( 0,0,0 ), 5.0f, 1.0f, ae::Vec3( 10,0,0 ) );
	REQUIRE( IsCloseEnough( result.x, 10.0f, 0.01f ) );
	REQUIRE( IsCloseEnough( result.y, 0.0f ) );
	REQUIRE( IsCloseEnough( result.z, 0.0f ) );
}

//------------------------------------------------------------------------------
// ae::DegToRad tests
//------------------------------------------------------------------------------
TEST_CASE( "DegToRad and RadToDeg", "[ae::DegToRad]" )
{
	REQUIRE( ae::DegToRad( 0.0f ) == 0.0f );
	REQUIRE( IsCloseEnough( ae::DegToRad( 180.0f ), ae::PI ) );
	REQUIRE( IsCloseEnough( ae::DegToRad( 90.0f ), ae::HalfPi ) );
	REQUIRE( ae::RadToDeg( 0.0f ) == 0.0f );
	REQUIRE( IsCloseEnough( ae::RadToDeg( ae::PI ), 180.0f ) );
	REQUIRE( IsCloseEnough( ae::RadToDeg( ae::DegToRad( 45.0f ) ), 45.0f ) );
}

//------------------------------------------------------------------------------
// ae::MaxValue / ae::MinValue tests
//------------------------------------------------------------------------------
TEST_CASE( "MaxValue and MinValue", "[ae::MaxValue]" )
{
	REQUIRE( ae::MaxValue< int32_t >() == INT32_MAX );
	REQUIRE( ae::MinValue< int32_t >() == INT32_MIN );
	REQUIRE( ae::MaxValue< float >() > 0.0f );
	REQUIRE( ae::MinValue< float >() < 0.0f );
	// float MaxValue/MinValue are +/-infinity
	REQUIRE( std::isinf( ae::MaxValue< float >() ) );
	REQUIRE( std::isinf( ae::MinValue< float >() ) );
}

//------------------------------------------------------------------------------
// ae::VecT tests
//------------------------------------------------------------------------------
TEST_CASE( "Vec2 arithmetic operators", "[ae::VecT]" )
{
	REQUIRE( ae::Vec2( 1,2 ) + ae::Vec2( 3,4 ) == ae::Vec2( 4,6 ) );
	REQUIRE( ae::Vec2( 4,6 ) - ae::Vec2( 3,4 ) == ae::Vec2( 1,2 ) );
	REQUIRE( -ae::Vec2( 1,2 ) == ae::Vec2( -1,-2 ) );
	REQUIRE( ae::Vec2( 2,4 ) * 0.5f == ae::Vec2( 1,2 ) );
	REQUIRE( ae::Vec2( 2,4 ) / 2.0f == ae::Vec2( 1,2 ) );
	REQUIRE( ae::Vec2( 2,4 ) * ae::Vec2( 3,4 ) == ae::Vec2( 6,16 ) );
	REQUIRE( ae::Vec2( 4,6 ) / ae::Vec2( 2,3 ) == ae::Vec2( 2,2 ) );
	REQUIRE( ae::Vec2( 1,2 ) == ae::Vec2( 1,2 ) );
	REQUIRE_FALSE( ae::Vec2( 1,2 ) != ae::Vec2( 1,2 ) );
	REQUIRE( ae::Vec2( 1,2 ) != ae::Vec2( 1,3 ) );
	REQUIRE( ae::Vec2( 3,4 )[ 0 ] == 3.0f );
	REQUIRE( ae::Vec2( 3,4 )[ 1 ] == 4.0f );
}

TEST_CASE( "Vec2 Dot Length", "[ae::VecT]" )
{
	REQUIRE( ae::Vec2( 1,0 ).Dot( ae::Vec2( 0,1 ) ) == 0.0f );
	REQUIRE( ae::Vec2( 1,0 ).Dot( ae::Vec2( 1,0 ) ) == 1.0f );
	REQUIRE( IsCloseEnough( ae::Vec2( 3,4 ).Dot( ae::Vec2( 2,1 ) ), 10.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 3,4 ).Length(), 5.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 3,4 ).LengthSquared(), 25.0f ) );
}

TEST_CASE( "Vec2 Normalize SafeNormalize NormalizeCopy", "[ae::VecT]" )
{
	ae::Vec2 v( 3,4 );
	const float len = v.Normalize();
	REQUIRE( IsCloseEnough( len, 5.0f ) );
	REQUIRE( IsCloseEnough( v.Length(), 1.0f ) );

	ae::Vec2 zero( 0,0 );
	const float zeroLen = zero.SafeNormalize();
	REQUIRE( zeroLen == 0.0f );
	REQUIRE( zero == ae::Vec2( 0,0 ) );

	const ae::Vec2 orig( 3,4 );
	const ae::Vec2 copy = orig.NormalizeCopy();
	REQUIRE( IsCloseEnough( copy.Length(), 1.0f ) );
	REQUIRE( orig == ae::Vec2( 3,4 ) );
}

TEST_CASE( "Vec2 Trim TrimCopy", "[ae::VecT]" )
{
	ae::Vec2 v( 3,4 );
	const float len = v.Trim( 3.0f );
	REQUIRE( IsCloseEnough( len, 3.0f ) );
	REQUIRE( IsCloseEnough( v, ae::Vec2( 3,4 ) * 0.6f ) );

	const ae::Vec2 orig( 3,4 );
	const ae::Vec2 trimmed = orig.TrimCopy( 3.0f );
	REQUIRE( IsCloseEnough( trimmed, ae::Vec2( 3,4 ) * 0.6f ) );
	REQUIRE( orig == ae::Vec2( 3,4 ) );

	ae::Vec2 v2( 3,4 );
	const float noTrimLen = v2.Trim( 10.0f );
	REQUIRE( IsCloseEnough( noTrimLen, 5.0f ) );
	REQUIRE( v2 == ae::Vec2( 3,4 ) );
}

TEST_CASE( "Vec2 IsNAN", "[ae::VecT]" )
{
	REQUIRE_FALSE( ae::Vec2( 1,2 ).IsNAN() );
	const float nan = 0.0f / 0.0f;
	REQUIRE( ae::Vec2( nan, 0.0f ).IsNAN() );
	REQUIRE( ae::Vec2( 0.0f, nan ).IsNAN() );
}

TEST_CASE( "Vec3 arithmetic operators", "[ae::VecT]" )
{
	REQUIRE( ae::Vec3( 1,2,3 ) + ae::Vec3( 4,5,6 ) == ae::Vec3( 5,7,9 ) );
	REQUIRE( ae::Vec3( 4,5,6 ) - ae::Vec3( 1,2,3 ) == ae::Vec3( 3,3,3 ) );
	REQUIRE( -ae::Vec3( 1,2,3 ) == ae::Vec3( -1,-2,-3 ) );
	REQUIRE( ae::Vec3( 2,4,6 ) * 0.5f == ae::Vec3( 1,2,3 ) );
	REQUIRE( ae::Vec3( 2,4,6 ) / 2.0f == ae::Vec3( 1,2,3 ) );
	REQUIRE( ae::Vec3( 1,2,3 ) == ae::Vec3( 1,2,3 ) );
	REQUIRE( ae::Vec3( 1,2,3 ) != ae::Vec3( 1,2,4 ) );
}

TEST_CASE( "Vec3 Cross", "[ae::VecT]" )
{
	REQUIRE( ae::Vec3( 1,0,0 ).Cross( ae::Vec3( 0,1,0 ) ) == ae::Vec3( 0,0,1 ) );
	REQUIRE( ae::Vec3( 0,1,0 ).Cross( ae::Vec3( 1,0,0 ) ) == ae::Vec3( 0,0,-1 ) );
	REQUIRE( ae::Vec3( 0,0,1 ).Cross( ae::Vec3( 1,0,0 ) ) == ae::Vec3( 0,1,0 ) );
}

TEST_CASE( "Vec3 Dot Length Normalize", "[ae::VecT]" )
{
	REQUIRE( ae::Vec3( 1,0,0 ).Dot( ae::Vec3( 0,1,0 ) ) == 0.0f );
	REQUIRE( ae::Vec3( 1,0,0 ).Dot( ae::Vec3( 1,0,0 ) ) == 1.0f );
	REQUIRE( IsCloseEnough( ae::Vec3( 1,2,2 ).Length(), 3.0f ) );
	ae::Vec3 v( 1,2,2 );
	v.Normalize();
	REQUIRE( IsCloseEnough( v.Length(), 1.0f ) );
}

TEST_CASE( "Vec4 arithmetic operators", "[ae::VecT]" )
{
	REQUIRE( ae::Vec4( 1,2,3,4 ) + ae::Vec4( 1,1,1,1 ) == ae::Vec4( 2,3,4,5 ) );
	REQUIRE( ae::Vec4( 2,3,4,5 ) - ae::Vec4( 1,1,1,1 ) == ae::Vec4( 1,2,3,4 ) );
	REQUIRE( -ae::Vec4( 1,2,3,4 ) == ae::Vec4( -1,-2,-3,-4 ) );
	REQUIRE( ae::Vec4( 2,4,6,8 ) * 0.5f == ae::Vec4( 1,2,3,4 ) );
	REQUIRE( ae::Vec4( 1,2,3,4 ) == ae::Vec4( 1,2,3,4 ) );
	REQUIRE( ae::Vec4( 1,2,3,4 ) != ae::Vec4( 1,2,3,5 ) );
}

TEST_CASE( "Vec4 Dot Length Normalize", "[ae::VecT]" )
{
	REQUIRE( ae::Vec4( 1,0,0,0 ).Dot( ae::Vec4( 0,1,0,0 ) ) == 0.0f );
	REQUIRE( ae::Vec4( 1,0,0,0 ).Dot( ae::Vec4( 1,0,0,0 ) ) == 1.0f );
	REQUIRE( IsCloseEnough( ae::Vec4( 1,0,0,0 ).Length(), 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec4( 1,1,0,0 ).LengthSquared(), 2.0f ) );
	ae::Vec4 v( 2,0,0,0 );
	v.Normalize();
	REQUIRE( IsCloseEnough( v.Length(), 1.0f ) );
	REQUIRE_FALSE( ae::Vec4( 1,2,3,4 ).IsNAN() );
	const float nan = 0.0f / 0.0f;
	REQUIRE( ae::Vec4( nan, 0,0,0 ).IsNAN() );
}

//------------------------------------------------------------------------------
// ae::Vec2 tests
//------------------------------------------------------------------------------
TEST_CASE( "Vec2 constructors", "[ae::Vec2]" )
{
	REQUIRE( ae::Vec2( 3.0f ) == ae::Vec2( 3.0f, 3.0f ) );
	const float xy[] = { 1.0f, 2.0f };
	REQUIRE( ae::Vec2( xy ) == ae::Vec2( 1,2 ) );
	REQUIRE( ae::Vec2( ae::Int2( 2, 3 ) ) == ae::Vec2( 2.0f, 3.0f ) );
}

TEST_CASE( "Vec2 FromAngle and GetAngle", "[ae::Vec2]" )
{
	REQUIRE( IsCloseEnough( ae::Vec2::FromAngle( 0.0f ), ae::Vec2( 1,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec2::FromAngle( ae::HalfPi ), ae::Vec2( 0,1 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec2::FromAngle( ae::PI ), ae::Vec2( -1,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 1,0 ).GetAngle(), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 0,1 ).GetAngle(), ae::HalfPi ) );
	REQUIRE( IsCloseEnough( ae::Abs( ae::Vec2( -1,0 ).GetAngle() ), ae::PI ) );
}

TEST_CASE( "Vec2 RotateCopy", "[ae::Vec2]" )
{
	REQUIRE( IsCloseEnough( ae::Vec2( 1,0 ).RotateCopy( ae::HalfPi ), ae::Vec2( 0,1 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 0,1 ).RotateCopy( ae::HalfPi ), ae::Vec2( -1,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 1,0 ).RotateCopy( 0.0f ), ae::Vec2( 1,0 ) ) );
}

TEST_CASE( "Vec2 NearestCopy FloorCopy CeilCopy", "[ae::Vec2]" )
{
	const ae::Vec2 v( 1.7f, 2.3f );
	REQUIRE( v.NearestCopy() == ae::Int2( 2, 2 ) );
	REQUIRE( v.FloorCopy() == ae::Int2( 1, 2 ) );
	REQUIRE( v.CeilCopy() == ae::Int2( 2, 3 ) );
}

TEST_CASE( "Vec2 Slerp", "[ae::Vec2]" )
{
	const ae::Vec2 result = ae::Vec2( 1,0 ).Slerp( ae::Vec2( 0,1 ), 0.5f );
	const float expected = ae::Sqrt( 0.5f );
	REQUIRE( IsCloseEnough( result.x, expected ) );
	REQUIRE( IsCloseEnough( result.y, expected ) );
	REQUIRE( IsCloseEnough( result.Length(), 1.0f ) );
}

TEST_CASE( "Vec2 Reflect", "[ae::Vec2]" )
{
	// Formula: n*(2*(v.n)/|n|^2) - v
	REQUIRE( IsCloseEnough( ae::Vec2::Reflect( ae::Vec2( 1,0 ), ae::Vec2( 1,0 ) ), ae::Vec2( 1,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec2::Reflect( ae::Vec2( 0,1 ), ae::Vec2( 1,0 ) ), ae::Vec2( 0,-1 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec2::Reflect( ae::Vec2( 1,1 ), ae::Vec2( 1,0 ) ), ae::Vec2( 1,-1 ) ) );
}

//------------------------------------------------------------------------------
// ae::Vec3 tests
//------------------------------------------------------------------------------
TEST_CASE( "Vec3 constructors", "[ae::Vec3]" )
{
	REQUIRE( ae::Vec3( 2.0f ) == ae::Vec3( 2,2,2 ) );
	REQUIRE( ae::Vec3( ae::Vec2( 1,2 ), 3.0f ) == ae::Vec3( 1,2,3 ) );
	REQUIRE( ae::Vec3( ae::Vec2( 1,2 ) ) == ae::Vec3( 1,2,0 ) );
}

TEST_CASE( "Vec3 GetXY GetXZ SetXY SetXZ XZY", "[ae::Vec3]" )
{
	REQUIRE( ae::Vec3::XZY( ae::Vec2( 1,3 ), 2.0f ) == ae::Vec3( 1,2,3 ) );
	const ae::Vec3 v( 1,2,3 );
	REQUIRE( v.GetXY() == ae::Vec2( 1,2 ) );
	REQUIRE( v.GetXZ() == ae::Vec2( 1,3 ) );
	ae::Vec3 v2( 0,0,0 );
	v2.SetXY( ae::Vec2( 4,5 ) );
	REQUIRE( v2.GetXY() == ae::Vec2( 4,5 ) );
	v2.SetXZ( ae::Vec2( 6,7 ) );
	REQUIRE( v2.GetXZ() == ae::Vec2( 6,7 ) );
}

TEST_CASE( "Vec3 NearestCopy FloorCopy CeilCopy", "[ae::Vec3]" )
{
	const ae::Vec3 v( 1.7f, 2.3f, 3.5f );
	REQUIRE( v.NearestCopy() == ae::Int3( 2, 2, 4 ) );
	REQUIRE( v.FloorCopy() == ae::Int3( 1, 2, 3 ) );
	REQUIRE( v.CeilCopy() == ae::Int3( 2, 3, 4 ) );
}

TEST_CASE( "Vec3 AddRotationXYCopy", "[ae::Vec3]" )
{
	REQUIRE( IsCloseEnough( ae::Vec3( 1,0,0 ).AddRotationXYCopy( ae::HalfPi ), ae::Vec3( 0,1,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 0,1,0 ).AddRotationXYCopy( ae::HalfPi ), ae::Vec3( -1,0,0 ) ) );
}

TEST_CASE( "Vec3 GetAngleBetween", "[ae::Vec3]" )
{
	REQUIRE( IsCloseEnough( ae::Vec3( 1,0,0 ).GetAngleBetween( ae::Vec3( 0,1,0 ) ), ae::HalfPi ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 1,0,0 ).GetAngleBetween( ae::Vec3( -1,0,0 ) ), ae::PI ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 1,0,0 ).GetAngleBetween( ae::Vec3( 1,0,0 ) ), 0.0f ) );
}

TEST_CASE( "Vec3 RotateCopy", "[ae::Vec3]" )
{
	REQUIRE( IsCloseEnough( ae::Vec3( 1,0,0 ).RotateCopy( ae::Vec3( 0,0,1 ), ae::HalfPi ), ae::Vec3( 0,1,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 0,1,0 ).RotateCopy( ae::Vec3( 0,0,1 ), ae::HalfPi ), ae::Vec3( -1,0,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 0,0,1 ).RotateCopy( ae::Vec3( 1,0,0 ), ae::HalfPi ), ae::Vec3( 0,-1,0 ) ) );
}

TEST_CASE( "Vec3 Lerp member", "[ae::Vec3]" )
{
	REQUIRE( IsCloseEnough( ae::Vec3( 1,0,0 ).Lerp( ae::Vec3( 3,0,0 ), 0.5f ), ae::Vec3( 2,0,0 ) ) );
	// member Lerp clips t to [0,1]
	REQUIRE( IsCloseEnough( ae::Vec3( 0,0,0 ).Lerp( ae::Vec3( 10,0,0 ), 2.0f ), ae::Vec3( 10,0,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 0,0,0 ).Lerp( ae::Vec3( 10,0,0 ), -1.0f ), ae::Vec3( 0,0,0 ) ) );
}

TEST_CASE( "Vec3 ZeroAxisCopy ZeroDirectionCopy", "[ae::Vec3]" )
{
	REQUIRE( IsCloseEnough( ae::Vec3( 3,4,5 ).ZeroAxisCopy( ae::Vec3( 1,0,0 ) ), ae::Vec3( 0,4,5 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 3,4,5 ).ZeroAxisCopy( ae::Vec3( 0,1,0 ) ), ae::Vec3( 3,0,5 ) ) );
	// ZeroDirectionCopy only zeroes if the component along the direction is positive
	REQUIRE( IsCloseEnough( ae::Vec3( 3,4,5 ).ZeroDirectionCopy( ae::Vec3( 1,0,0 ) ), ae::Vec3( 0,4,5 ) ) );
	REQUIRE( IsCloseEnough( ae::Vec3( -3,4,5 ).ZeroDirectionCopy( ae::Vec3( 1,0,0 ) ), ae::Vec3( -3,4,5 ) ) );
}

//------------------------------------------------------------------------------
// ae::Vec4 tests
//------------------------------------------------------------------------------
TEST_CASE( "Vec4 constructors", "[ae::Vec4]" )
{
	REQUIRE( ae::Vec4( 2.0f ) == ae::Vec4( 2,2,2,2 ) );
	REQUIRE( ae::Vec4( 1.0f, 2.0f ) == ae::Vec4( 1,1,1,2 ) );
	REQUIRE( ae::Vec4( ae::Vec3( 1,2,3 ), 4.0f ) == ae::Vec4( 1,2,3,4 ) );
	REQUIRE( ae::Vec4( ae::Vec2( 1,2 ), ae::Vec2( 3,4 ) ) == ae::Vec4( 1,2,3,4 ) );
}

TEST_CASE( "Vec4 accessors", "[ae::Vec4]" )
{
	const ae::Vec4 v( 1,2,3,4 );
	REQUIRE( v.GetXY() == ae::Vec2( 1,2 ) );
	REQUIRE( v.GetXZ() == ae::Vec2( 1,3 ) );
	REQUIRE( v.GetZW() == ae::Vec2( 3,4 ) );
	REQUIRE( v.GetXYZ() == ae::Vec3( 1,2,3 ) );
	ae::Vec4 v2( 0,0,0,0 );
	v2.SetXY( ae::Vec2( 1,2 ) );
	REQUIRE( v2.GetXY() == ae::Vec2( 1,2 ) );
	v2.SetXZ( ae::Vec2( 3,4 ) );
	REQUIRE( v2.GetXZ() == ae::Vec2( 3,4 ) );
	v2.SetZW( ae::Vec2( 5,6 ) );
	REQUIRE( v2.GetZW() == ae::Vec2( 5,6 ) );
	v2.SetXYZ( ae::Vec3( 7,8,9 ) );
	REQUIRE( v2.GetXYZ() == ae::Vec3( 7,8,9 ) );
}

TEST_CASE( "Vec4 casts", "[ae::Vec4]" )
{
	const ae::Vec4 v( 1,2,3,4 );
	REQUIRE( (ae::Vec2)v == ae::Vec2( 1,2 ) );
	REQUIRE( (ae::Vec3)v == ae::Vec3( 1,2,3 ) );
}

//------------------------------------------------------------------------------
// ae::Int2 tests
//------------------------------------------------------------------------------
TEST_CASE( "Int2 constructors and indexing", "[ae::Int2]" )
{
	const ae::Int2 a( 3 );
	REQUIRE( a == ae::Int2( 3, 3 ) );
	const ae::Int2 b( 2, 3 );
	REQUIRE( b[ 0 ] == 2 );
	REQUIRE( b[ 1 ] == 3 );
}

TEST_CASE( "Int2 arithmetic", "[ae::Int2]" )
{
	REQUIRE( ae::Int2( 1,2 ) + ae::Int2( 3,4 ) == ae::Int2( 4,6 ) );
	REQUIRE( ae::Int2( 4,6 ) - ae::Int2( 3,4 ) == ae::Int2( 1,2 ) );
	REQUIRE( -ae::Int2( 1,2 ) == ae::Int2( -1,-2 ) );
	REQUIRE( ae::Int2( 2,4 ) * ae::Int2( 2,2 ) == ae::Int2( 4,8 ) );
	REQUIRE( ae::Int2( 4,6 ) / ae::Int2( 2,3 ) == ae::Int2( 2,2 ) );
	REQUIRE( ae::Int2( 2,4 ) * 2 == ae::Int2( 4,8 ) );
	REQUIRE( ae::Int2( 4,6 ) / 2 == ae::Int2( 2,3 ) );
	REQUIRE( ae::Int2( 1,2 ) == ae::Int2( 1,2 ) );
	REQUIRE( ae::Int2( 1,2 ) != ae::Int2( 1,3 ) );
}

//------------------------------------------------------------------------------
// ae::Int3 tests
//------------------------------------------------------------------------------
TEST_CASE( "Int3 constructors and accessors", "[ae::Int3]" )
{
	const ae::Int3 a( 2 );
	REQUIRE( a == ae::Int3( 2, 2, 2 ) );
	const ae::Int3 b( 1, 2, 3 );
	REQUIRE( b.GetXY() == ae::Int2( 1, 2 ) );
	REQUIRE( b.GetXZ() == ae::Int2( 1, 3 ) );
}

//------------------------------------------------------------------------------
// ae::Matrix4 gaps tests
//------------------------------------------------------------------------------
TEST_CASE( "Matrix4 Identity TransformPoint3x4", "[ae::Matrix4]" )
{
	const ae::Vec3 pt( 1,2,3 );
	REQUIRE( ae::Matrix4::Identity().TransformPoint3x4( pt ) == pt );
}

TEST_CASE( "Matrix4 Translation", "[ae::Matrix4]" )
{
	REQUIRE( ae::Matrix4::Translation( 1,2,3 ).GetTranslation() == ae::Vec3( 1,2,3 ) );
	REQUIRE( IsCloseEnough(
		ae::Matrix4::Translation( ae::Vec3( 1,2,3 ) ).TransformPoint3x4( ae::Vec3( 0,0,0 ) ),
		ae::Vec3( 1,2,3 ) ) );
}

TEST_CASE( "Matrix4 TransformVector3x4 unaffected by translation", "[ae::Matrix4]" )
{
	REQUIRE( ae::Matrix4::Translation( 5,5,5 ).TransformVector3x4( ae::Vec3( 1,0,0 ) ) == ae::Vec3( 1,0,0 ) );
	REQUIRE( ae::Matrix4::Translation( 5,5,5 ).TransformVector3x4( ae::Vec3( 0,1,0 ) ) == ae::Vec3( 0,1,0 ) );
}

TEST_CASE( "Matrix4 RotationZ", "[ae::Matrix4]" )
{
	// TransformPoint3x4 uses standard M*v convention; correct for rotation testing
	REQUIRE( IsCloseEnough(
		ae::Matrix4::RotationZ( ae::HalfPi ).TransformPoint3x4( ae::Vec3( 1,0,0 ) ),
		ae::Vec3( 0,1,0 ) ) );
	REQUIRE( IsCloseEnough(
		ae::Matrix4::RotationZ( ae::HalfPi ).TransformPoint3x4( ae::Vec3( 0,1,0 ) ),
		ae::Vec3( -1,0,0 ) ) );
}

TEST_CASE( "Matrix4 GetTranspose SetTranspose", "[ae::Matrix4]" )
{
	const ae::Matrix4 m = ae::Matrix4::RotationZ( ae::HalfPi );
	REQUIRE( m.GetTranspose().GetTranspose() == m );
	ae::Matrix4 m2 = m;
	m2.SetTranspose();
	REQUIRE( m2 == m.GetTranspose() );
}

TEST_CASE( "Matrix4 GetInverse SetInverse", "[ae::Matrix4]" )
{
	const ae::Quaternion q( ae::Vec3( 0,1,0 ), ae::HalfPi );
	const ae::Matrix4 m = ae::Matrix4::LocalToWorld( ae::Vec3( 1,2,3 ), q, ae::Vec3( 2,2,2 ) );
	const ae::Vec3 testPt( 3,1,4 );
	REQUIRE( IsCloseEnough( ( m * m.GetInverse() ).TransformPoint3x4( testPt ), testPt ) );
	ae::Matrix4 m2 = m;
	m2.SetInverse();
	REQUIRE( IsCloseEnough( ( m * m2 ).TransformPoint3x4( testPt ), testPt ) );
}

TEST_CASE( "Matrix4 GetNormalMatrix", "[ae::Matrix4]" )
{
	// For a pure rotation R, GetNormalMatrix = R^{-T} = R
	const ae::Quaternion q( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Matrix4 r = ae::Matrix4::Rotation( q );
	const ae::Matrix4 nm = r.GetNormalMatrix();
	const ae::Vec3 v0( 1,0,0 );
	const ae::Vec3 v1( 0,1,0 );
	REQUIRE( IsCloseEnough( r.TransformVector3x4( v0 ), nm.TransformVector3x4( v0 ) ) );
	REQUIRE( IsCloseEnough( r.TransformVector3x4( v1 ), nm.TransformVector3x4( v1 ) ) );
}

TEST_CASE( "Matrix4 GetScaleRemoved", "[ae::Matrix4]" )
{
	const ae::Quaternion q( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Matrix4 m = ae::Matrix4::LocalToWorld( ae::Vec3( 0,0,0 ), q, ae::Vec3( 2,3,4 ) );
	const ae::Matrix4 nr = m.GetScaleRemoved();
	REQUIRE( IsCloseEnough( nr.GetAxis( 0 ).Length(), 1.0f ) );
	REQUIRE( IsCloseEnough( nr.GetAxis( 1 ).Length(), 1.0f ) );
	REQUIRE( IsCloseEnough( nr.GetAxis( 2 ).Length(), 1.0f ) );
}

TEST_CASE( "Matrix4 GetAxis GetColumn GetRow SetAxis SetRow", "[ae::Matrix4]" )
{
	REQUIRE( ae::Matrix4::Identity().GetAxis( 0 ) == ae::Vec3( 1,0,0 ) );
	REQUIRE( ae::Matrix4::Identity().GetAxis( 1 ) == ae::Vec3( 0,1,0 ) );
	REQUIRE( ae::Matrix4::Identity().GetColumn( 0 ) == ae::Vec4( 1,0,0,0 ) );
	REQUIRE( ae::Matrix4::Identity().GetRow( 0 ) == ae::Vec4( 1,0,0,0 ) );
	ae::Matrix4 m = ae::Matrix4::Identity();
	m.SetAxis( 0, ae::Vec3( 2,0,0 ) );
	REQUIRE( m.GetAxis( 0 ) == ae::Vec3( 2,0,0 ) );
	m.SetRow( 1, ae::Vec4( 0,2,0,0 ) );
	REQUIRE( m.GetRow( 1 ) == ae::Vec4( 0,2,0,0 ) );
}

TEST_CASE( "Matrix4 GetRotation round-trip", "[ae::Matrix4]" )
{
	const ae::Quaternion q( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Matrix4 m = ae::Matrix4::LocalToWorld( ae::Vec3( 0,0,0 ), q, ae::Vec3( 1,1,1 ) );
	REQUIRE( IsCloseEnoughQ( m.GetRotation(), q ) );
}

TEST_CASE( "Matrix4 multiply", "[ae::Matrix4]" )
{
	const ae::Matrix4 combined =
		ae::Matrix4::Translation( 1.0f, 0.0f, 0.0f )
		* ae::Matrix4::Translation( 0.0f, 1.0f, 0.0f );
	REQUIRE( IsCloseEnough( combined.GetTranslation(), ae::Vec3( 1,1,0 ) ) );
}

TEST_CASE( "Matrix4 operator Vec4", "[ae::Matrix4]" )
{
	REQUIRE( ae::Matrix4::Identity() * ae::Vec4( 1,2,3,4 ) == ae::Vec4( 1,2,3,4 ) );
	REQUIRE( IsCloseEnough(
		ae::Matrix4::Translation( 1,2,3 ) * ae::Vec4( 0,0,0,1 ),
		ae::Vec4( 1,2,3,1 ) ) );
}

TEST_CASE( "Matrix4 IsNAN", "[ae::Matrix4]" )
{
	REQUIRE_FALSE( ae::Matrix4::Identity().IsNAN() );
	const float nan = 0.0f / 0.0f;
	ae::Matrix4 m = ae::Matrix4::Identity();
	m.SetRow( 0, ae::Vec4( nan, 0,0,0 ) );
	REQUIRE( m.IsNAN() );
}

//------------------------------------------------------------------------------
// ae::Quaternion tests
//------------------------------------------------------------------------------
TEST_CASE( "Quaternion Identity", "[ae::Quaternion]" )
{
	REQUIRE( IsCloseEnough( ae::Quaternion::Identity().Rotate( ae::Vec3( 1,0,0 ) ), ae::Vec3( 1,0,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Quaternion::Identity().Rotate( ae::Vec3( 0,1,0 ) ), ae::Vec3( 0,1,0 ) ) );
	REQUIRE( IsCloseEnough( ae::Quaternion::Identity().Rotate( ae::Vec3( 0,0,1 ) ), ae::Vec3( 0,0,1 ) ) );
}

TEST_CASE( "Quaternion rotation around axes", "[ae::Quaternion]" )
{
	REQUIRE( IsCloseEnough(
		ae::Quaternion( ae::Vec3( 0,0,1 ), ae::HalfPi ).Rotate( ae::Vec3( 1,0,0 ) ),
		ae::Vec3( 0,1,0 ) ) );
	REQUIRE( IsCloseEnough(
		ae::Quaternion( ae::Vec3( 1,0,0 ), ae::PI ).Rotate( ae::Vec3( 0,1,0 ) ),
		ae::Vec3( 0,-1,0 ) ) );
	REQUIRE( IsCloseEnough(
		ae::Quaternion( ae::Vec3( 0,1,0 ), ae::HalfPi ).Rotate( ae::Vec3( 1,0,0 ) ),
		ae::Vec3( 0,0,-1 ) ) );
}

TEST_CASE( "Quaternion NormalizeCopy", "[ae::Quaternion]" )
{
	// (i=0, j=0, k=0, r=2) — scalar r is last arg
	const ae::Quaternion q( 0.0f, 0.0f, 0.0f, 2.0f );
	const ae::Quaternion n = q.NormalizeCopy();
	REQUIRE( IsCloseEnough( n.r, 1.0f ) );
	REQUIRE( IsCloseEnough( n.i, 0.0f ) );
	REQUIRE( IsCloseEnough( n.j, 0.0f ) );
	REQUIRE( IsCloseEnough( n.k, 0.0f ) );
}

TEST_CASE( "Quaternion composition", "[ae::Quaternion]" )
{
	const ae::Quaternion q90( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Quaternion q180 = q90 * q90;
	REQUIRE( IsCloseEnough( q180.Rotate( ae::Vec3( 1,0,0 ) ), ae::Vec3( -1,0,0 ) ) );
}

TEST_CASE( "Quaternion operator negate", "[ae::Quaternion]" )
{
	const ae::Quaternion id = ae::Quaternion::Identity();
	REQUIRE( ( -id ).r == -1.0f );
	REQUIRE( ( -id ).i == 0.0f );
	// -q and q represent the same rotation
	REQUIRE( IsCloseEnough( ( -id ).Rotate( ae::Vec3( 1,0,0 ) ), ae::Vec3( 1,0,0 ) ) );
}

TEST_CASE( "Quaternion GetInverse", "[ae::Quaternion]" )
{
	const ae::Quaternion q( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Vec3 testVec( 1,0,0 );
	const ae::Vec3 rotated = q.Rotate( testVec );
	REQUIRE( IsCloseEnough( q.GetInverse().Rotate( rotated ), testVec ) );
}

TEST_CASE( "Quaternion GetAxisAngle round-trip", "[ae::Quaternion]" )
{
	const ae::Quaternion q( ae::Vec3( 0,1,0 ), ae::PI / 3.0f );
	ae::Vec3 axis;
	float angle;
	q.GetAxisAngle( &axis, &angle );
	REQUIRE( IsCloseEnough( axis, ae::Vec3( 0,1,0 ) ) );
	REQUIRE( IsCloseEnough( angle, ae::PI / 3.0f ) );
}

TEST_CASE( "Quaternion Nlerp", "[ae::Quaternion]" )
{
	const ae::Quaternion q( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Vec3 result = ae::Quaternion::Identity().Nlerp( q, 0.5f ).Rotate( ae::Vec3( 1,0,0 ) );
	REQUIRE( result.x > 0.0f );
	REQUIRE( result.y > 0.0f );
	REQUIRE( IsCloseEnough( result.z, 0.0f ) );
	REQUIRE( IsCloseEnough( result.Length(), 1.0f ) );
}

TEST_CASE( "Quaternion GetTransformMatrix", "[ae::Quaternion]" )
{
	const ae::Quaternion q( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Vec3 testVec( 1,0,0 );
	// TransformPoint3x4 uses standard M*v convention, matching Rotate()
	REQUIRE( IsCloseEnough(
		q.GetTransformMatrix().TransformPoint3x4( testVec ),
		q.Rotate( testVec ) ) );
}

TEST_CASE( "Quaternion RelativeCopy", "[ae::Quaternion]" )
{
	const ae::Quaternion q0( ae::Vec3( 0,0,1 ), ae::HalfPi );
	const ae::Quaternion q1( ae::Vec3( 1,0,0 ), ae::PI / 3.0f );
	const ae::Vec3 testVec( 1,1,0 );
	// q0 * q1.RelativeCopy(q0) should give same rotation as q1
	REQUIRE( IsCloseEnough(
		( q0 * q1.RelativeCopy( q0 ) ).Rotate( testVec ),
		q1.Rotate( testVec ) ) );
}

TEST_CASE( "Quaternion GetTwistSwing", "[ae::Quaternion]" )
{
	const ae::Quaternion q( ae::Vec3( 1,0,0 ), ae::HalfPi );
	ae::Quaternion twist, swing;
	q.GetTwistSwing( ae::Vec3( 1,0,0 ), &twist, &swing );
	// (twist * swing) and q represent the same rotation (may differ by sign)
	const ae::Vec3 testVec( 0,1,0 );
	REQUIRE( IsCloseEnough( ( twist * swing ).Rotate( testVec ), q.Rotate( testVec ) ) );
}

TEST_CASE( "Quaternion forward up constructor", "[ae::Quaternion]" )
{
	// forward=(0,1,0), up=(0,0,1): rotating (0,1,0) by this quaternion gives forward
	const ae::Quaternion q( ae::Vec3( 0,1,0 ), ae::Vec3( 0,0,1 ) );
	REQUIRE( IsCloseEnough( q.Rotate( ae::Vec3( 0,1,0 ) ), ae::Vec3( 0,1,0 ) ) );
}

//------------------------------------------------------------------------------
// ae::Color tests
//------------------------------------------------------------------------------
TEST_CASE( "Color constructors", "[ae::Color]" )
{
	const ae::Color red = ae::Color::RGB( 1.0f, 0.0f, 0.0f );
	REQUIRE( red.r == 1.0f );
	REQUIRE( red.g == 0.0f );
	REQUIRE( red.b == 0.0f );
	REQUIRE( red.a == 1.0f );
	REQUIRE( ae::Color::RGBA( 0.5f, 0.5f, 0.5f, 0.5f ).a == 0.5f );
	REQUIRE( IsCloseEnough( ae::Color::RGB8( 255, 0, 0 ).r, 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Color::RGB8( 0, 255, 0 ).g, 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Color::RGB8( 0, 0, 255 ).b, 1.0f ) );
}

TEST_CASE( "Color SRGB conversions", "[ae::Color]" )
{
	REQUIRE( ae::Color::SRGB( 1.0f, 1.0f, 1.0f ) == ae::Color::RGB( 1.0f, 1.0f, 1.0f ) );
	REQUIRE( ae::Color::SRGB( 0.0f, 0.0f, 0.0f ) == ae::Color::RGB( 0.0f, 0.0f, 0.0f ) );
	REQUIRE( ae::Color::SRGBToRGB( 0.0f ) == 0.0f );
	REQUIRE( ae::Color::SRGBToRGB( 1.0f ) == 1.0f );
	REQUIRE( ae::Color::RGBToSRGB( 0.0f ) == 0.0f );
	REQUIRE( ae::Color::RGBToSRGB( 1.0f ) == 1.0f );
	REQUIRE( IsCloseEnough( ae::Color::RGBToSRGB( ae::Color::SRGBToRGB( 0.5f ) ), 0.5f ) );
}

TEST_CASE( "Color HSV", "[ae::Color]" )
{
	REQUIRE( IsCloseEnough( ae::Color::HSV( 0.0f, 0.0f, 1.0f ), ae::Color::RGB( 1.0f, 1.0f, 1.0f ) ) );
	REQUIRE( IsCloseEnough( ae::Color::HSV( 0.0f, 0.0f, 0.0f ), ae::Color::RGB( 0.0f, 0.0f, 0.0f ) ) );
	REQUIRE( IsCloseEnough( ae::Color::HSV( 0.0f, 1.0f, 1.0f ), ae::Color::RGB( 1.0f, 0.0f, 0.0f ) ) );
	REQUIRE( IsCloseEnough( ae::Color::HSV( 1.0f / 3.0f, 1.0f, 1.0f ), ae::Color::RGB( 0.0f, 1.0f, 0.0f ) ) );
	const ae::Vec3 hsv = ae::Color::RGB( 1.0f, 0.0f, 0.0f ).GetHSV();
	REQUIRE( IsCloseEnough( hsv.x, 0.0f ) );
	REQUIRE( IsCloseEnough( hsv.y, 1.0f ) );
	REQUIRE( IsCloseEnough( hsv.z, 1.0f ) );
}

TEST_CASE( "Color ScaleRGB ScaleA SetA", "[ae::Color]" )
{
	REQUIRE( IsCloseEnough( ae::Color::RGB( 1.0f, 0.0f, 0.0f ).ScaleRGB( 0.5f ), ae::Color::RGBA( 0.5f, 0.0f, 0.0f, 1.0f ) ) );
	REQUIRE( IsCloseEnough( ae::Color::RGBA( 1.0f, 0.0f, 0.0f, 0.5f ).ScaleA( 0.5f ), ae::Color::RGBA( 1.0f, 0.0f, 0.0f, 0.25f ) ) );
	REQUIRE( IsCloseEnough( ae::Color::RGBA( 1.0f, 0.0f, 0.0f, 0.5f ).SetA( 1.0f ), ae::Color::RGB( 1.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "Color Lerp", "[ae::Color]" )
{
	const ae::Color result = ae::Color::RGB( 0.0f, 0.0f, 0.0f ).Lerp( ae::Color::RGB( 1.0f, 1.0f, 1.0f ), 0.5f );
	REQUIRE( IsCloseEnough( result, ae::Color::RGB( 0.5f, 0.5f, 0.5f ) ) );
	REQUIRE( IsCloseEnough( ae::Color::RGB( 0.0f, 0.0f, 0.0f ).Lerp( ae::Color::RGB( 1.0f, 1.0f, 1.0f ), 0.0f ), ae::Color::RGB( 0.0f, 0.0f, 0.0f ) ) );
	REQUIRE( IsCloseEnough( ae::Color::RGB( 0.0f, 0.0f, 0.0f ).Lerp( ae::Color::RGB( 1.0f, 1.0f, 1.0f ), 1.0f ), ae::Color::RGB( 1.0f, 1.0f, 1.0f ) ) );
}

TEST_CASE( "Color accessors", "[ae::Color]" )
{
	const ae::Color c = ae::Color::RGBA( 0.1f, 0.2f, 0.3f, 0.4f );
	REQUIRE( IsCloseEnough( c.GetLinearRGB(), ae::Vec3( 0.1f, 0.2f, 0.3f ) ) );
	REQUIRE( IsCloseEnough( c.GetLinearRGBA(), ae::Vec4( 0.1f, 0.2f, 0.3f, 0.4f ) ) );
	REQUIRE( ae::Color::RGB( 1.0f, 0.0f, 0.0f ).GetSRGB() == ae::Vec3( 1,0,0 ) );
}

TEST_CASE( "Color operators", "[ae::Color]" )
{
	REQUIRE( ae::Color::RGB( 1.0f, 0.0f, 0.0f ) == ae::Color::RGB( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( ae::Color::RGB( 1.0f, 0.0f, 0.0f ) != ae::Color::RGB( 0.0f, 1.0f, 0.0f ) );
	REQUIRE( IsCloseEnough(
		ae::Color::RGB( 1.0f, 0.0f, 0.0f ) * ae::Color::RGBA( 1.0f, 1.0f, 1.0f, 0.5f ),
		ae::Color::RGBA( 1.0f, 0.0f, 0.0f, 0.5f ) ) );
}


//------------------------------------------------------------------------------
// Comprehensive math unit tests
// NOTE: <limits> may already be included transitively; listed here for clarity
//------------------------------------------------------------------------------
TEST_CASE( "Scalar math: Pow, Sqrt, Abs", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Pow( 2.0f, 3.0f ), 8.0f ) );
	REQUIRE( IsCloseEnough( ae::Pow( 4.0f, 0.5f ), 2.0f ) );
	REQUIRE( IsCloseEnough( ae::Pow( 1.0f, 99.0f ), 1.0f ) );

	REQUIRE( IsCloseEnough( ae::Sqrt( 4.0f ), 2.0f ) );
	REQUIRE( IsCloseEnough( ae::Sqrt( 9.0f ), 3.0f ) );
	REQUIRE( IsCloseEnough( ae::Sqrt( 0.0f ), 0.0f ) );

	REQUIRE( IsCloseEnough( ae::Abs( 3.0f ), 3.0f ) );
	REQUIRE( IsCloseEnough( ae::Abs( -3.0f ), 3.0f ) );
	REQUIRE( IsCloseEnough( ae::Abs( 0.0f ), 0.0f ) );

	REQUIRE( ae::Abs( (int32_t)5 ) == 5 );
	REQUIRE( ae::Abs( (int32_t)-5 ) == 5 );
	REQUIRE( ae::Abs( (int32_t)0 ) == 0 );
}

TEST_CASE( "Scalar math: trig functions", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Cos( 0.0f ), 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Cos( ae::PI ), -1.0f ) );
	REQUIRE( IsCloseEnough( ae::Cos( ae::HalfPi ), 0.0f ) );

	REQUIRE( IsCloseEnough( ae::Sin( 0.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Sin( ae::HalfPi ), 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Sin( ae::PI ), 0.0f ) );

	REQUIRE( IsCloseEnough( ae::Tan( 0.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Tan( ae::QuarterPi ), 1.0f ) );

	REQUIRE( IsCloseEnough( ae::Acos( 1.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Acos( -1.0f ), ae::PI ) );
	REQUIRE( IsCloseEnough( ae::Acos( 0.0f ), ae::HalfPi ) );

	REQUIRE( IsCloseEnough( ae::Asin( 0.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Asin( 1.0f ), ae::HalfPi ) );

	REQUIRE( IsCloseEnough( ae::Atan( 0.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Atan( 1.0f ), ae::QuarterPi ) );

	REQUIRE( IsCloseEnough( ae::Atan2( 1.0f, 1.0f ), ae::QuarterPi ) );
	REQUIRE( IsCloseEnough( ae::Atan2( 1.0f, 0.0f ), ae::HalfPi ) );
}

TEST_CASE( "Scalar math: DegToRad, RadToDeg", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::DegToRad( 180.0f ), ae::PI ) );
	REQUIRE( IsCloseEnough( ae::DegToRad( 90.0f ), ae::HalfPi ) );
	REQUIRE( IsCloseEnough( ae::DegToRad( 0.0f ), 0.0f ) );

	REQUIRE( IsCloseEnough( ae::RadToDeg( ae::PI ), 180.0f ) );
	REQUIRE( IsCloseEnough( ae::RadToDeg( ae::HalfPi ), 90.0f ) );

	REQUIRE( IsCloseEnough( ae::RadToDeg( ae::DegToRad( 45.0f ) ), 45.0f ) );
}

TEST_CASE( "Scalar math: Mod", "[math]" )
{
	REQUIRE( ae::Mod( 7u, 3u ) == 1u );
	REQUIRE( ae::Mod( 6u, 3u ) == 0u );
	REQUIRE( ae::Mod( 0u, 5u ) == 0u );

	REQUIRE( ae::Mod( -1, 5 ) == 4 );
	REQUIRE( ae::Mod( -7, 3 ) == 2 );
	REQUIRE( ae::Mod( 7, 3 ) == 1 );
	REQUIRE( ae::Mod( 6, 3 ) == 0 );
	REQUIRE( ae::Mod( 0, 5 ) == 0 );

	REQUIRE( IsCloseEnough( ae::Mod( -0.5f, 1.0f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Mod( -1.5f, 1.0f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Mod( 0.3f, 1.0f ), 0.3f ) );
	REQUIRE( IsCloseEnough( ae::Mod( 1.3f, 1.0f ), 0.3f ) );
}

TEST_CASE( "Scalar math: Ceil, Floor, Round", "[math]" )
{
	REQUIRE( ae::Ceil( 1.1f ) == 2 );
	REQUIRE( ae::Ceil( -1.1f ) == -1 );
	REQUIRE( ae::Ceil( 2.0f ) == 2 );

	REQUIRE( ae::Floor( 1.9f ) == 1 );
	REQUIRE( ae::Floor( -1.1f ) == -2 );
	REQUIRE( ae::Floor( 2.0f ) == 2 );

	REQUIRE( ae::Round( 1.4f ) == 1 );
	REQUIRE( ae::Round( 1.5f ) == 2 );
	REQUIRE( ae::Round( -1.4f ) == -1 );
	REQUIRE( ae::Round( -1.5f ) == -2 );

	REQUIRE( ae::Floor( -7, 2 ) == -4 );
	REQUIRE( ae::Floor( 7, 2 ) == 3 );
	REQUIRE( ae::Floor( 6, 2 ) == 3 );
	REQUIRE( ae::Floor( -6, 2 ) == -3 );
}

TEST_CASE( "Scalar math: NextPowerOfTwo", "[math]" )
{
	REQUIRE( ae::NextPowerOfTwo( 0u ) == 0u );
	REQUIRE( ae::NextPowerOfTwo( 1u ) == 1u );
	REQUIRE( ae::NextPowerOfTwo( 2u ) == 2u );
	REQUIRE( ae::NextPowerOfTwo( 3u ) == 4u );
	REQUIRE( ae::NextPowerOfTwo( 7u ) == 8u );
	REQUIRE( ae::NextPowerOfTwo( 8u ) == 8u );
	REQUIRE( ae::NextPowerOfTwo( 9u ) == 16u );
	REQUIRE( ae::NextPowerOfTwo( 16u ) == 16u );
	REQUIRE( ae::NextPowerOfTwo( 17u ) == 32u );
}

TEST_CASE( "Scalar math: Clip and Clip01", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Clip( 0.5f, 0.0f, 1.0f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Clip( -1.0f, 0.0f, 1.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Clip( 2.0f, 0.0f, 1.0f ), 1.0f ) );

	REQUIRE( IsCloseEnough( ae::Clip01( 0.5f ), 0.5f ) );
	REQUIRE( IsCloseEnough( ae::Clip01( -0.5f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Clip01( 1.5f ), 1.0f ) );
}

TEST_CASE( "Scalar math: Lerp", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Lerp( 0.0f, 10.0f, 0.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Lerp( 0.0f, 10.0f, 1.0f ), 10.0f ) );
	REQUIRE( IsCloseEnough( ae::Lerp( 0.0f, 10.0f, 0.5f ), 5.0f ) );
	REQUIRE( IsCloseEnough( ae::Lerp( 2.0f, 4.0f, 0.25f ), 2.5f ) );
}

TEST_CASE( "Scalar math: AngleDifference and LerpAngle", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::AngleDifference( ae::PI * 1.5f, 0.0f ), -ae::PI * 0.5f ) );
	REQUIRE( IsCloseEnough( ae::AngleDifference( 0.1f, 0.0f ), 0.1f ) );
	REQUIRE( IsCloseEnough( ae::AngleDifference( 0.0f, 0.1f ), -0.1f ) );
	REQUIRE( IsCloseEnough( ae::AngleDifference( ae::PI + 0.1f, ae::PI - 0.1f ), 0.2f ) );

	REQUIRE( IsCloseEnough( ae::LerpAngle( 0.0f, ae::HalfPi, 0.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::LerpAngle( 0.0f, ae::HalfPi, 1.0f ), ae::HalfPi ) );
	REQUIRE( IsCloseEnough( ae::LerpAngle( 0.0f, ae::HalfPi, 0.5f ), ae::QuarterPi ) );
}

TEST_CASE( "Scalar math: DtLerp", "[math]" )
{
	REQUIRE( ae::DtLerp( 1.0f, 0.0f, 1.0f, 10.0f ) == 1.0f );
	REQUIRE( ae::DtLerp( 1.0f, 5.0f, 0.0f, 10.0f ) == 1.0f );
	REQUIRE( IsCloseEnough( ae::DtLerp( 1.0f, 20.0f, 1.0f, 10.0f ), 10.0f ) );
	const float dtLerpV = ae::DtLerp( 1.0f, 2.0f, 0.5f, 10.0f );
	REQUIRE( dtLerpV >= 1.0f );
	REQUIRE( dtLerpV <= 10.0f );
}

TEST_CASE( "Scalar math: DtLerpAngle", "[math]" )
{
	REQUIRE( ae::DtLerpAngle( 1.0f, 0.0f, 1.0f, 3.0f ) == 1.0f );
	REQUIRE( ae::DtLerpAngle( 1.0f, 5.0f, 0.0f, 3.0f ) == 1.0f );
	REQUIRE( IsCloseEnough( ae::DtLerpAngle( 0.0f, 20.0f, 1.0f, 1.0f ), 1.0f ) );
	const float dlaStart = ae::PI - 0.1f;
	const float dlaEnd = -ae::PI + 0.1f;
	const float dlaResult = ae::DtLerpAngle( dlaStart, 20.0f, 1.0f, dlaEnd );
	REQUIRE( dlaResult > ae::HalfPi );
}

TEST_CASE( "Scalar math: CosineInterpolate", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::CosineInterpolate( 0.0f, 10.0f, 0.0f ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::CosineInterpolate( 0.0f, 10.0f, 1.0f ), 10.0f ) );
	REQUIRE( IsCloseEnough( ae::CosineInterpolate( 0.0f, 10.0f, 0.5f ), 5.0f ) );
}

TEST_CASE( "Scalar math: MaxValue MinValue", "[math]" )
{
	REQUIRE( ae::MaxValue< int32_t >() == std::numeric_limits< int32_t >::max() );
	REQUIRE( ae::MinValue< int32_t >() == std::numeric_limits< int32_t >::min() );
	REQUIRE( ae::MaxValue< uint32_t >() == std::numeric_limits< uint32_t >::max() );
}

TEST_CASE( "VecT shared: Normalize and SafeNormalize", "[math]" )
{
	ae::Vec2 v2n( 3.0f, 4.0f );
	REQUIRE( IsCloseEnough( v2n.Normalize(), 5.0f ) );
	REQUIRE( IsCloseEnough( v2n.Length(), 1.0f ) );
	const ae::Vec2 v2nc = ae::Vec2( 3.0f, 4.0f ).NormalizeCopy();
	REQUIRE( IsCloseEnough( v2nc.Length(), 1.0f ) );

	ae::Vec3 v3n( 1.0f, 2.0f, 2.0f );
	REQUIRE( IsCloseEnough( v3n.Normalize(), 3.0f ) );
	REQUIRE( IsCloseEnough( v3n.Length(), 1.0f ) );

	ae::Vec4 v4n( 1.0f, 0.0f, 0.0f, 0.0f );
	REQUIRE( IsCloseEnough( v4n.Normalize(), 1.0f ) );

	ae::Vec2 zero2( 0.0f, 0.0f );
	REQUIRE( IsCloseEnough( zero2.SafeNormalize(), 0.0f ) );
	REQUIRE( zero2 == ae::Vec2( 0.0f, 0.0f ) );

	ae::Vec2 sv2( 3.0f, 4.0f );
	REQUIRE( IsCloseEnough( sv2.SafeNormalize(), 5.0f ) );
	REQUIRE( IsCloseEnough( sv2.Length(), 1.0f ) );

	const ae::Vec2 snOrig( 3.0f, 4.0f );
	const ae::Vec2 snCopy = snOrig.SafeNormalizeCopy();
	REQUIRE( IsCloseEnough( snOrig.Length(), 5.0f ) );
	REQUIRE( IsCloseEnough( snCopy.Length(), 1.0f ) );
}

TEST_CASE( "VecT shared: Trim and TrimCopy", "[math]" )
{
	ae::Vec2 trimV0( 3.0f, 4.0f );
	REQUIRE( IsCloseEnough( trimV0.Trim( 100.0f ), 5.0f ) );
	REQUIRE( IsCloseEnough( trimV0.Length(), 5.0f ) );

	ae::Vec2 trimV1( 3.0f, 4.0f );
	REQUIRE( IsCloseEnough( trimV1.Trim( 1.0f ), 1.0f ) );
	REQUIRE( IsCloseEnough( trimV1.Length(), 1.0f ) );

	const ae::Vec2 trimOrig( 3.0f, 4.0f );
	const ae::Vec2 trimCopy = trimOrig.TrimCopy( 1.0f );
	REQUIRE( IsCloseEnough( trimOrig.Length(), 5.0f ) );
	REQUIRE( IsCloseEnough( trimCopy.Length(), 1.0f ) );
}

TEST_CASE( "VecT shared: Dot, Length, LengthSquared", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Vec2::Dot( ae::Vec2( 1.0f, 0.0f ), ae::Vec2( 0.0f, 1.0f ) ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec3::Dot( ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) ), 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ).Dot( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) ), 30.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 3.0f, 4.0f ).Length(), 5.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 1.0f, 1.0f, 1.0f ).LengthSquared(), 3.0f ) );
}

TEST_CASE( "VecT shared: IsNAN", "[math]" )
{
	REQUIRE( ae::Vec2( 1.0f, 2.0f ).IsNAN() == false );
	REQUIRE( ae::Vec2( 0.0f / 0.0f, 0.0f ).IsNAN() == true );
	REQUIRE( ae::Vec3( 1.0f, 2.0f, 3.0f ).IsNAN() == false );
	REQUIRE( ae::Vec3( 0.0f / 0.0f, 0.0f, 0.0f ).IsNAN() == true );
	REQUIRE( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ).IsNAN() == false );
	REQUIRE( ae::Vec4( 0.0f / 0.0f, 0.0f, 0.0f, 0.0f ).IsNAN() == true );
}

TEST_CASE( "Vec2 constructors", "[math]" )
{
	const ae::Vec2 v2splat( 5.0f );
	REQUIRE( v2splat.x == 5.0f );
	REQUIRE( v2splat.y == 5.0f );

	const ae::Vec2 v2xy( 3.0f, 4.0f );
	REQUIRE( v2xy.x == 3.0f );
	REQUIRE( v2xy.y == 4.0f );

	const float v2arr[ 2 ] = { 1.0f, 2.0f };
	const ae::Vec2 v2fromArr( v2arr );
	REQUIRE( v2fromArr.x == 1.0f );
	REQUIRE( v2fromArr.y == 2.0f );

	const ae::Vec2 v2fromInt( ae::Int2( 3, 4 ) );
	REQUIRE( v2fromInt.x == 3.0f );
	REQUIRE( v2fromInt.y == 4.0f );
}

TEST_CASE( "Vec2 FromAngle and GetAngle", "[math]" )
{
	const ae::Vec2 fa0 = ae::Vec2::FromAngle( 0.0f );
	REQUIRE( IsCloseEnough( fa0.x, 1.0f ) );
	REQUIRE( IsCloseEnough( fa0.y, 0.0f ) );

	const ae::Vec2 fa90 = ae::Vec2::FromAngle( ae::HalfPi );
	REQUIRE( IsCloseEnough( fa90.x, 0.0f ) );
	REQUIRE( IsCloseEnough( fa90.y, 1.0f ) );

	const ae::Vec2 fa180 = ae::Vec2::FromAngle( ae::PI );
	REQUIRE( IsCloseEnough( fa180.x, -1.0f ) );
	REQUIRE( IsCloseEnough( fa180.y, 0.0f ) );

	REQUIRE( IsCloseEnough( ae::Vec2( 1.0f, 0.0f ).GetAngle(), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec2( 0.0f, 1.0f ).GetAngle(), ae::HalfPi ) );

	const float rtAngle = 1.23f;
	REQUIRE( IsCloseEnough( ae::Vec2::FromAngle( rtAngle ).GetAngle(), rtAngle ) );
}

TEST_CASE( "Vec2 arithmetic and comparison", "[math]" )
{
	REQUIRE( ae::Vec2( 1.0f, 2.0f ) + ae::Vec2( 3.0f, 4.0f ) == ae::Vec2( 4.0f, 6.0f ) );
	REQUIRE( ae::Vec2( 5.0f, 6.0f ) - ae::Vec2( 2.0f, 3.0f ) == ae::Vec2( 3.0f, 3.0f ) );
	REQUIRE( ae::Vec2( 2.0f, 3.0f ) * 2.0f == ae::Vec2( 4.0f, 6.0f ) );
	REQUIRE( ae::Vec2( 4.0f, 6.0f ) / 2.0f == ae::Vec2( 2.0f, 3.0f ) );
	REQUIRE( -ae::Vec2( 1.0f, 2.0f ) == ae::Vec2( -1.0f, -2.0f ) );
	REQUIRE( ae::Vec2( 1.0f, 2.0f ) * ae::Vec2( 3.0f, 4.0f ) == ae::Vec2( 3.0f, 8.0f ) );

	ae::Vec2 v2cmp( 1.0f, 2.0f );
	v2cmp += ae::Vec2( 3.0f, 4.0f );
	REQUIRE( v2cmp == ae::Vec2( 4.0f, 6.0f ) );

	REQUIRE( ae::Vec2( 1.0f, 2.0f ) == ae::Vec2( 1.0f, 2.0f ) );
	REQUIRE( ae::Vec2( 1.0f, 2.0f ) != ae::Vec2( 1.0f, 3.0f ) );
	REQUIRE( ae::Vec2( 3.0f, 4.0f )[ 0 ] == 3.0f );
	REQUIRE( ae::Vec2( 3.0f, 4.0f )[ 1 ] == 4.0f );
}

TEST_CASE( "Vec2 NearestCopy, FloorCopy, CeilCopy", "[math]" )
{
	REQUIRE( ae::Vec2( 1.4f, 2.7f ).NearestCopy() == ae::Int2( 1, 3 ) );
	REQUIRE( ae::Vec2( 1.9f, -0.1f ).FloorCopy() == ae::Int2( 1, -1 ) );
	REQUIRE( ae::Vec2( 1.1f, 1.9f ).CeilCopy() == ae::Int2( 2, 2 ) );
}

TEST_CASE( "Vec2 RotateCopy and Reflect", "[math]" )
{
	const ae::Vec2 rcR90 = ae::Vec2( 1.0f, 0.0f ).RotateCopy( ae::HalfPi );
	REQUIRE( IsCloseEnough( rcR90.x, 0.0f ) );
	REQUIRE( IsCloseEnough( rcR90.y, 1.0f ) );

	const ae::Vec2 rcR180 = ae::Vec2( 1.0f, 0.0f ).RotateCopy( ae::PI );
	REQUIRE( IsCloseEnough( rcR180.x, -1.0f ) );
	REQUIRE( IsCloseEnough( rcR180.y, 0.0f ) );

	const ae::Vec2 refR0 = ae::Vec2::Reflect( ae::Vec2( 1.0f, 0.0f ), ae::Vec2( 0.0f, 1.0f ) );
	REQUIRE( IsCloseEnough( refR0.x, -1.0f ) );
	REQUIRE( IsCloseEnough( refR0.y, 0.0f ) );

	const ae::Vec2 refR1 = ae::Vec2::Reflect( ae::Vec2( 0.0f, 1.0f ), ae::Vec2( 0.0f, 1.0f ) );
	REQUIRE( IsCloseEnough( refR1.x, 0.0f ) );
	REQUIRE( IsCloseEnough( refR1.y, 1.0f ) );
}

TEST_CASE( "Vec2 Slerp and DtSlerp", "[math]" )
{
	const ae::Vec2 sl0 = ae::Vec2( 1.0f, 0.0f ).Slerp( ae::Vec2( 0.0f, 1.0f ), 0.0f );
	REQUIRE( IsCloseEnough( sl0.x, 1.0f ) );
	REQUIRE( IsCloseEnough( sl0.y, 0.0f ) );

	const ae::Vec2 sl1 = ae::Vec2( 1.0f, 0.0f ).Slerp( ae::Vec2( 0.0f, 1.0f ), 1.0f );
	REQUIRE( IsCloseEnough( sl1.x, 0.0f ) );
	REQUIRE( IsCloseEnough( sl1.y, 1.0f ) );

	const ae::Vec2 slm = ae::Vec2( 1.0f, 0.0f ).Slerp( ae::Vec2( 0.0f, 1.0f ), 0.5f );
	REQUIRE( IsCloseEnough( slm.Length(), 1.0f ) );
	REQUIRE( IsCloseEnough( slm.x, 0.707f, 0.01f ) );
	REQUIRE( IsCloseEnough( slm.y, 0.707f, 0.01f ) );

	const ae::Vec2 dts0 = ae::Vec2( 1.0f, 0.0f ).DtSlerp( ae::Vec2( 0.0f, 1.0f ), 0.0f, 1.0f );
	REQUIRE( dts0 == ae::Vec2( 1.0f, 0.0f ) );

	const ae::Vec2 dtsL = ae::Vec2( 1.0f, 0.0f ).DtSlerp( ae::Vec2( 0.0f, 1.0f ), 20.0f, 1.0f );
	REQUIRE( IsCloseEnough( dtsL.x, 0.0f ) );
	REQUIRE( IsCloseEnough( dtsL.y, 1.0f ) );
}

TEST_CASE( "Vec3 constructors", "[math]" )
{
	const ae::Vec3 v3splat( 5.0f );
	REQUIRE( ( v3splat.x == 5.0f && v3splat.y == 5.0f && v3splat.z == 5.0f ) );

	const ae::Vec3 v3xyz( 1.0f, 2.0f, 3.0f );
	REQUIRE( ( v3xyz.x == 1.0f && v3xyz.y == 2.0f && v3xyz.z == 3.0f ) );

	const float v3arr[ 3 ] = { 1.0f, 2.0f, 3.0f };
	const ae::Vec3 v3fromArr( v3arr );
	REQUIRE( ( v3fromArr.x == 1.0f && v3fromArr.y == 2.0f && v3fromArr.z == 3.0f ) );

	const ae::Vec3 v3fromInt( ae::Int3( 1, 2, 3 ) );
	REQUIRE( ( v3fromInt.x == 1.0f && v3fromInt.y == 2.0f && v3fromInt.z == 3.0f ) );

	const ae::Vec3 v3fromV2( ae::Vec2( 1.0f, 2.0f ), 3.0f );
	REQUIRE( ( v3fromV2.x == 1.0f && v3fromV2.y == 2.0f && v3fromV2.z == 3.0f ) );

	const ae::Vec3 v3xzy = ae::Vec3::XZY( ae::Vec2( 1.0f, 3.0f ), 2.0f );
	REQUIRE( ( v3xzy.x == 1.0f && v3xzy.y == 2.0f && v3xzy.z == 3.0f ) );
}

TEST_CASE( "Vec3 arithmetic and component access", "[math]" )
{
	REQUIRE( ae::Vec3( 1.0f, 2.0f, 3.0f ) + ae::Vec3( 4.0f, 5.0f, 6.0f ) == ae::Vec3( 5.0f, 7.0f, 9.0f ) );
	REQUIRE( ae::Vec3( 4.0f, 5.0f, 6.0f ) - ae::Vec3( 1.0f, 2.0f, 3.0f ) == ae::Vec3( 3.0f, 3.0f, 3.0f ) );
	REQUIRE( ae::Vec3( 1.0f, 2.0f, 3.0f ) * 2.0f == ae::Vec3( 2.0f, 4.0f, 6.0f ) );
	REQUIRE( ae::Vec3( 2.0f, 4.0f, 6.0f ) / 2.0f == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( ae::Vec3( 1.0f, 2.0f, 3.0f ) == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( ae::Vec3( 1.0f, 2.0f, 3.0f ) != ae::Vec3( 1.0f, 2.0f, 4.0f ) );
	REQUIRE( ae::Vec3( 1.0f, 2.0f, 3.0f )[ 2 ] == 3.0f );

	ae::Vec3 v3c( 1.0f, 2.0f, 3.0f );
	v3c.SetXY( ae::Vec2( 7.0f, 8.0f ) );
	REQUIRE( v3c.GetXY() == ae::Vec2( 7.0f, 8.0f ) );
	v3c.SetXZ( ae::Vec2( 9.0f, 11.0f ) );
	REQUIRE( v3c.GetXZ() == ae::Vec2( 9.0f, 11.0f ) );

	REQUIRE( ae::Vec3( 1.4f, 2.7f, 3.5f ).NearestCopy() == ae::Int3( 1, 3, 4 ) );
	REQUIRE( ae::Vec3( 1.9f, -0.1f, 2.0f ).FloorCopy() == ae::Int3( 1, -1, 2 ) );
	REQUIRE( ae::Vec3( 1.1f, 1.9f, 2.0f ).CeilCopy() == ae::Int3( 2, 2, 2 ) );
}

TEST_CASE( "Vec3 Cross product", "[math]" )
{
	REQUIRE( ae::Vec3::Cross( ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) ) == ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	REQUIRE( ae::Vec3( 1.0f, 0.0f, 0.0f ).Cross( ae::Vec3( 0.0f, 0.0f, 1.0f ) ) == ae::Vec3( 0.0f, -1.0f, 0.0f ) );
	const ae::Vec3 cpa( 1.0f, 2.0f, 3.0f );
	const ae::Vec3 cpb( 4.0f, 5.0f, 6.0f );
	REQUIRE( ae::Vec3::Cross( cpa, cpb ) == cpa.Cross( cpb ) );
}

TEST_CASE( "Vec3 AddRotationXY", "[math]" )
{
	ae::Vec3 arxy( 1.0f, 0.0f, 0.0f );
	arxy.AddRotationXY( ae::HalfPi );
	REQUIRE( IsCloseEnough( arxy.x, 0.0f ) );
	REQUIRE( IsCloseEnough( arxy.y, 1.0f ) );

	ae::Vec3 arxyZ( 1.0f, 0.0f, 3.0f );
	arxyZ.AddRotationXY( ae::HalfPi );
	REQUIRE( IsCloseEnough( arxyZ.z, 3.0f ) );

	const ae::Vec3 arxyOrig( 1.0f, 0.0f, 0.0f );
	const ae::Vec3 arxyCopy = arxyOrig.AddRotationXYCopy( ae::HalfPi );
	REQUIRE( arxyOrig == ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( arxyCopy.y, 1.0f ) );
}

TEST_CASE( "Vec3 GetAngleBetween", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Vec3( 1.0f, 0.0f, 0.0f ).GetAngleBetween( ae::Vec3( 1.0f, 0.0f, 0.0f ) ), 0.0f ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 1.0f, 0.0f, 0.0f ).GetAngleBetween( ae::Vec3( 0.0f, 1.0f, 0.0f ) ), ae::HalfPi ) );
	REQUIRE( IsCloseEnough( ae::Vec3( 1.0f, 0.0f, 0.0f ).GetAngleBetween( ae::Vec3( -1.0f, 0.0f, 0.0f ) ), ae::PI ) );
}

TEST_CASE( "Vec3 RotateCopy", "[math]" )
{
	const ae::Vec3 rc0 = ae::Vec3( 1.0f, 0.0f, 0.0f ).RotateCopy( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	REQUIRE( IsCloseEnough( rc0.x, 0.0f ) );
	REQUIRE( IsCloseEnough( rc0.y, 1.0f ) );
	REQUIRE( IsCloseEnough( rc0.z, 0.0f ) );

	const ae::Vec3 rc1 = ae::Vec3( 0.0f, 1.0f, 0.0f ).RotateCopy( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	REQUIRE( IsCloseEnough( rc1.x, -1.0f ) );
	REQUIRE( IsCloseEnough( rc1.y, 0.0f ) );
	REQUIRE( IsCloseEnough( rc1.z, 0.0f ) );
}

TEST_CASE( "Vec3 ZeroAxis and ZeroDirection", "[math]" )
{
	ae::Vec3 zaV( 1.0f, 1.0f, 0.0f );
	zaV.ZeroAxis( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( zaV.x, 0.0f ) );
	REQUIRE( IsCloseEnough( zaV.y, 1.0f ) );

	ae::Vec3 zdV0( 1.0f, 1.0f, 0.0f );
	zdV0.ZeroDirection( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( zdV0.x, 0.0f ) );
	REQUIRE( IsCloseEnough( zdV0.y, 1.0f ) );

	ae::Vec3 zdV1( -1.0f, 1.0f, 0.0f );
	zdV1.ZeroDirection( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( zdV1.x, -1.0f ) );
	REQUIRE( IsCloseEnough( zdV1.y, 1.0f ) );

	const ae::Vec3 zaOrig( 1.0f, 1.0f, 0.0f );
	const ae::Vec3 zaCopy = zaOrig.ZeroAxisCopy( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( zaOrig == ae::Vec3( 1.0f, 1.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( zaCopy.x, 0.0f ) );
	const ae::Vec3 zdCopy = zaOrig.ZeroDirectionCopy( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( zdCopy.x, 0.0f ) );
}

TEST_CASE( "Vec3 Lerp, Slerp, DtSlerp", "[math]" )
{
	const ae::Vec3 v3lm = ae::Vec3( 0.0f, 0.0f, 0.0f ).Lerp( ae::Vec3( 10.0f, 10.0f, 10.0f ), 0.5f );
	REQUIRE( IsCloseEnough( v3lm.x, 5.0f ) );
	REQUIRE( IsCloseEnough( v3lm.y, 5.0f ) );
	REQUIRE( IsCloseEnough( v3lm.z, 5.0f ) );

	const ae::Vec3 v3sl0 = ae::Vec3( 1.0f, 0.0f, 0.0f ).Slerp( ae::Vec3( 0.0f, 1.0f, 0.0f ), 0.0f );
	REQUIRE( ( IsCloseEnough( v3sl0.x, 1.0f ) && IsCloseEnough( v3sl0.y, 0.0f ) ) );
	const ae::Vec3 v3sl1 = ae::Vec3( 1.0f, 0.0f, 0.0f ).Slerp( ae::Vec3( 0.0f, 1.0f, 0.0f ), 1.0f );
	REQUIRE( ( IsCloseEnough( v3sl1.x, 0.0f ) && IsCloseEnough( v3sl1.y, 1.0f ) ) );

	const ae::Vec3 v3slm = ae::Vec3( 1.0f, 0.0f, 0.0f ).Slerp( ae::Vec3( 0.0f, 1.0f, 0.0f ), 0.5f );
	REQUIRE( IsCloseEnough( v3slm.Length(), 1.0f ) );
	REQUIRE( IsCloseEnough( v3slm.x, ae::Cos( ae::QuarterPi ) ) );
	REQUIRE( IsCloseEnough( v3slm.y, ae::Sin( ae::QuarterPi ) ) );
}

TEST_CASE( "Vec3 ProjectPoint", "[math]" )
{
	const ae::Vec3 ppResult = ae::Vec3::ProjectPoint( ae::Matrix4::Identity(), ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( IsCloseEnough( ppResult.x, 1.0f ) );
	REQUIRE( IsCloseEnough( ppResult.y, 2.0f ) );
	REQUIRE( IsCloseEnough( ppResult.z, 3.0f ) );
}

TEST_CASE( "Vec4 constructors and operations", "[math]" )
{
	const ae::Vec4 v4splat( 5.0f );
	REQUIRE( ( v4splat.x == 5.0f && v4splat.y == 5.0f && v4splat.z == 5.0f && v4splat.w == 5.0f ) );
	REQUIRE( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ).w == 4.0f );
	REQUIRE( ae::Vec4( ae::Vec3( 1.0f, 2.0f, 3.0f ), 4.0f ).z == 3.0f );
	REQUIRE( ae::Vec4( ae::Vec2( 1.0f, 2.0f ), 3.0f, 4.0f ).x == 1.0f );
	REQUIRE( ae::Vec4( ae::Vec2( 1.0f, 2.0f ), ae::Vec2( 3.0f, 4.0f ) ).z == 3.0f );

	REQUIRE( (ae::Vec2)ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) == ae::Vec2( 1.0f, 2.0f ) );
	REQUIRE( (ae::Vec3)ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) == ae::Vec3( 1.0f, 2.0f, 3.0f ) );

	REQUIRE( ( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) + ae::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) ) == ae::Vec4( 2.0f, 3.0f, 4.0f, 5.0f ) );
	REQUIRE( ae::Vec4( 2.0f, 4.0f, 6.0f, 8.0f ) / 2.0f == ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	REQUIRE( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) == ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	REQUIRE( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) != ae::Vec4( 1.0f, 2.0f, 3.0f, 5.0f ) );
	REQUIRE( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f )[ 3 ] == 4.0f );

	ae::Vec4 v4acc( 1.0f, 2.0f, 3.0f, 4.0f );
	v4acc.SetXY( ae::Vec2( 7.0f, 8.0f ) );
	REQUIRE( v4acc.GetXY() == ae::Vec2( 7.0f, 8.0f ) );
	v4acc.SetXZ( ae::Vec2( 9.0f, 11.0f ) );
	REQUIRE( v4acc.GetXZ() == ae::Vec2( 9.0f, 11.0f ) );
	v4acc.SetZW( ae::Vec2( 5.0f, 6.0f ) );
	REQUIRE( v4acc.GetZW() == ae::Vec2( 5.0f, 6.0f ) );
	v4acc.SetXYZ( ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( v4acc.GetXYZ() == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
}

TEST_CASE( "Int2 constructors and operations", "[math]" )
{
	REQUIRE( ( ae::Int2( 5 ).x == 5 && ae::Int2( 5 ).y == 5 ) );
	REQUIRE( ( ae::Int2( 3, 4 ).x == 3 && ae::Int2( 3, 4 ).y == 4 ) );
	REQUIRE( ae::Int2( ae::Int3( 1, 2, 3 ) ) == ae::Int2( 1, 2 ) );

	REQUIRE( ae::Int2( 1, 2 ) + ae::Int2( 3, 4 ) == ae::Int2( 4, 6 ) );
	REQUIRE( ae::Int2( 5, 6 ) - ae::Int2( 2, 3 ) == ae::Int2( 3, 3 ) );
	REQUIRE( ae::Int2( 2, 3 ) * ae::Int2( 4, 5 ) == ae::Int2( 8, 15 ) );
	REQUIRE( ae::Int2( 1, 2 ) == ae::Int2( 1, 2 ) );
	REQUIRE( ae::Int2( 1, 2 ) != ae::Int2( 1, 3 ) );
	REQUIRE( ae::Int2( 3, 4 )[ 0 ] == 3 );
	REQUIRE( ae::Int2( 3, 4 )[ 1 ] == 4 );
}

TEST_CASE( "Int3 constructors and operations", "[math]" )
{
	REQUIRE( ( ae::Int3( 5 ).x == 5 && ae::Int3( 5 ).y == 5 && ae::Int3( 5 ).z == 5 ) );
	REQUIRE( ae::Int3( 1, 2, 3 ).z == 3 );
	REQUIRE( ae::Int3( ae::Int2( 1, 2 ), 3 ).z == 3 );

	REQUIRE( ae::Int3( 1, 2, 3 ) + ae::Int3( 4, 5, 6 ) == ae::Int3( 5, 7, 9 ) );
	REQUIRE( ae::Int3( 4, 5, 6 ) - ae::Int3( 1, 2, 3 ) == ae::Int3( 3, 3, 3 ) );
	REQUIRE( ae::Int3( 2, 3, 4 ) * ae::Int3( 2, 2, 2 ) == ae::Int3( 4, 6, 8 ) );
	REQUIRE( ae::Int3( 1, 2, 3 ) == ae::Int3( 1, 2, 3 ) );
	REQUIRE( ae::Int3( 1, 2, 3 ) != ae::Int3( 1, 2, 4 ) );
	REQUIRE( ae::Int3( 1, 2, 3 )[ 2 ] == 3 );

	ae::Int3 i3( 1, 2, 3 );
	i3.SetXY( ae::Int2( 7, 8 ) );
	REQUIRE( i3.GetXY() == ae::Int2( 7, 8 ) );
	i3.SetXZ( ae::Int2( 9, 11 ) );
	REQUIRE( i3.GetXZ() == ae::Int2( 9, 11 ) );
}

TEST_CASE( "Quaternion constructors", "[math]" )
{
	const ae::Quaternion qId = ae::Quaternion::Identity();
	REQUIRE( ( qId.i == 0.0f && qId.j == 0.0f && qId.k == 0.0f && qId.r == 1.0f ) );

	const ae::Quaternion qAx = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	REQUIRE( IsCloseEnough( qAx.k, ae::Sin( ae::PI / 4.0f ) ) );
	REQUIRE( IsCloseEnough( qAx.r, ae::Cos( ae::PI / 4.0f ) ) );
	REQUIRE( IsCloseEnough( qAx.i, 0.0f ) );
	REQUIRE( IsCloseEnough( qAx.j, 0.0f ) );

	// Constructor aligns local +Y to forward, local +Z to up
	const ae::Quaternion qLa = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, -1.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	const ae::Vec3 qLaFwd = qLa.Rotate( ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( qLaFwd.x, 0.0f ) );
	REQUIRE( IsCloseEnough( qLaFwd.y, 0.0f ) );
	REQUIRE( IsCloseEnough( qLaFwd.z, -1.0f ) );
}

TEST_CASE( "Quaternion normalize and operators", "[math]" )
{
	const ae::Quaternion qRaw( 1.0f, 2.0f, 3.0f, 4.0f );
	const ae::Quaternion qNc = qRaw.NormalizeCopy();
	REQUIRE( IsCloseEnough( qRaw.i, 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Sqrt( qNc.i*qNc.i + qNc.j*qNc.j + qNc.k*qNc.k + qNc.r*qNc.r ), 1.0f ) );

	ae::Quaternion qNip( 1.0f, 2.0f, 3.0f, 4.0f );
	qNip.Normalize();
	REQUIRE( IsCloseEnough( ae::Sqrt( qNip.i*qNip.i + qNip.j*qNip.j + qNip.k*qNip.k + qNip.r*qNip.r ), 1.0f ) );

	REQUIRE( ae::Quaternion::Identity() == ae::Quaternion( 0.0f, 0.0f, 0.0f, 1.0f ) );
	REQUIRE( ae::Quaternion::Identity() != ae::Quaternion( 1.0f, 0.0f, 0.0f, 0.0f ) );

	REQUIRE( ae::Quaternion( 0.0f, 0.0f, 0.0f, 1.0f ) * ae::Quaternion( 0.0f, 0.0f, 0.0f, 1.0f ) == ae::Quaternion( 0.0f, 0.0f, 0.0f, 1.0f ) );
	REQUIRE( ae::Quaternion( 1.0f, 2.0f, 3.0f, 4.0f ) * 2.0f == ae::Quaternion( 2.0f, 4.0f, 6.0f, 8.0f ) );
}

TEST_CASE( "Quaternion operator- (antipodal)", "[math]" )
{
	REQUIRE( -ae::Quaternion( 1.0f, 2.0f, 3.0f, 4.0f ) == ae::Quaternion( -1.0f, -2.0f, -3.0f, -4.0f ) );

	const ae::Quaternion qAp = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	const ae::Vec3 qApR = qAp.Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	const ae::Vec3 qApNR = ( -qAp ).Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( qApR.x, qApNR.x ) );
	REQUIRE( IsCloseEnough( qApR.y, qApNR.y ) );
	REQUIRE( IsCloseEnough( qApR.z, qApNR.z ) );
}

TEST_CASE( "Quaternion Dot", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Quaternion::Identity().Dot( ae::Quaternion::Identity() ), 1.0f ) );
	REQUIRE( IsCloseEnough( ae::Quaternion( 1.0f, 0.0f, 0.0f, 0.0f ).Dot( ae::Quaternion( 0.0f, 1.0f, 0.0f, 0.0f ) ), 0.0f ) );
}

TEST_CASE( "Quaternion AddScaledVector", "[math]" )
{
	ae::Quaternion qAsv = ae::Quaternion::Identity();
	qAsv.AddScaledVector( ae::Vec3( 0.0f, 0.0f, 1.0f ), 0.5f );
	qAsv.Normalize();
	const ae::Vec3 rAsv = qAsv.Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( rAsv.x < 1.0f );
	REQUIRE( rAsv.y > 0.0f );
}

TEST_CASE( "Quaternion RotateByVector", "[math]" )
{
	ae::Quaternion qRbv = ae::Quaternion::Identity();
	qRbv.RotateByVector( ae::Vec3( 0.0f, 0.0f, ae::HalfPi ) );
	const ae::Vec3 rRbv = qRbv.Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( rRbv.x, 0.0f ) );
	REQUIRE( IsCloseEnough( rRbv.y, 1.0f ) );
	REQUIRE( IsCloseEnough( rRbv.z, 0.0f ) );
}

TEST_CASE( "Quaternion SetDirectionXY and GetDirectionXY", "[math]" )
{
	ae::Quaternion qDir = ae::Quaternion::Identity();
	qDir.SetDirectionXY( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( qDir.GetDirectionXY().z == 0.0f );

	qDir.SetDirectionXY( ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	const ae::Vec3 dDir = qDir.GetDirectionXY();
	REQUIRE( IsCloseEnough( dDir.x, 0.0f ) );
	REQUIRE( dDir.y > 0.0f );
	REQUIRE( dDir.z == 0.0f );
}

TEST_CASE( "Quaternion GetAxisAngle round-trip", "[math]" )
{
	const ae::Quaternion qAa = ae::Quaternion( ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::PI / 3.0f );
	ae::Vec3 aaAxis;
	float aaAngle = 0.0f;
	qAa.GetAxisAngle( &aaAxis, &aaAngle );
	REQUIRE( IsCloseEnough( aaAxis.x, 0.0f ) );
	REQUIRE( IsCloseEnough( aaAxis.y, 1.0f ) );
	REQUIRE( IsCloseEnough( aaAxis.z, 0.0f ) );
	REQUIRE( IsCloseEnough( aaAngle, ae::PI / 3.0f ) );
}

TEST_CASE( "Quaternion AddRotationXY", "[math]" )
{
	ae::Quaternion qArxy = ae::Quaternion::Identity();
	qArxy.AddRotationXY( ae::HalfPi );
	qArxy.Normalize();
	const ae::Vec3 rArxy = qArxy.Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( rArxy.x, 0.0f ) );
	REQUIRE( IsCloseEnough( rArxy.y, 1.0f ) );
}

TEST_CASE( "Quaternion Nlerp", "[math]" )
{
	const ae::Quaternion qNl = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	const ae::Quaternion qNlE = ae::Quaternion( ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::HalfPi );
	REQUIRE( qNl.Nlerp( qNlE, 0.0f ).Dot( qNl ) > 0.999f );
	REQUIRE( qNl.Nlerp( qNlE, 1.0f ).Dot( qNlE ) > 0.999f );
	const ae::Quaternion qNlm = qNl.Nlerp( qNl, 0.5f );
	REQUIRE( IsCloseEnough( ae::Sqrt( qNlm.i*qNlm.i + qNlm.j*qNlm.j + qNlm.k*qNlm.k + qNlm.r*qNlm.r ), 1.0f ) );
}

TEST_CASE( "Quaternion GetTransformMatrix", "[math]" )
{
	const ae::Quaternion qTm = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	const ae::Vec4 rTm = qTm.GetTransformMatrix() * ae::Vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	REQUIRE( IsCloseEnough( rTm.x, 0.0f ) );
	REQUIRE( IsCloseEnough( rTm.y, 1.0f ) );
	REQUIRE( IsCloseEnough( rTm.z, 0.0f ) );
	const ae::Vec3 rRot = qTm.Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( rRot.x, 0.0f ) );
	REQUIRE( IsCloseEnough( rRot.y, 1.0f ) );
}

TEST_CASE( "Quaternion GetInverse and SetInverse", "[math]" )
{
	const ae::Quaternion qInv = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	REQUIRE( ( qInv * qInv.GetInverse() ).Dot( ae::Quaternion::Identity() ) > 0.999f );
	ae::Quaternion qSi = qInv;
	qSi.SetInverse();
	REQUIRE( ( qInv * qSi ).Dot( ae::Quaternion::Identity() ) > 0.999f );
}

TEST_CASE( "Quaternion Rotate", "[math]" )
{
	const ae::Vec3 rRz = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi ).Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( rRz.x, 0.0f ) );
	REQUIRE( IsCloseEnough( rRz.y, 1.0f ) );
	REQUIRE( IsCloseEnough( rRz.z, 0.0f ) );
	const ae::Vec3 rId = ae::Quaternion::Identity().Rotate( ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( IsCloseEnough( rId.x, 1.0f ) );
	REQUIRE( IsCloseEnough( rId.y, 2.0f ) );
	REQUIRE( IsCloseEnough( rId.z, 3.0f ) );
}

TEST_CASE( "Quaternion RelativeCopy", "[math]" )
{
	const ae::Quaternion qRcP = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::QuarterPi );
	const ae::Quaternion qRcC = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	const ae::Quaternion qRel = qRcC.RelativeCopy( qRcP );
	REQUIRE( ( qRcP * qRel ).Dot( qRcC ) > 0.999f );
}

TEST_CASE( "Quaternion GetTwistSwing", "[math]" )
{
	const ae::Quaternion qTs = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	ae::Quaternion tsTwist;
	ae::Quaternion tsSwing;
	qTs.GetTwistSwing( ae::Vec3( 0.0f, 0.0f, 1.0f ), &tsTwist, &tsSwing );
	REQUIRE( IsCloseEnough( tsTwist.i, 0.0f ) );
	REQUIRE( IsCloseEnough( tsTwist.j, 0.0f ) );
	const ae::Vec3 tsOrig = qTs.Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	const ae::Vec3 tsRecov = ( tsSwing * tsTwist ).Rotate( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( IsCloseEnough( tsRecov.x, tsOrig.x ) );
	REQUIRE( IsCloseEnough( tsRecov.y, tsOrig.y ) );
	REQUIRE( IsCloseEnough( tsRecov.z, tsOrig.z ) );
}

TEST_CASE( "Color constructors and named colors", "[math]" )
{
	const ae::Color cRGB = ae::Color::RGB( 1.0f, 0.0f, 0.0f );
	REQUIRE( ( cRGB.r == 1.0f && cRGB.g == 0.0f && cRGB.b == 0.0f && cRGB.a == 1.0f ) );

	const ae::Color cRGBA = ae::Color::RGBA( 1.0f, 0.0f, 0.0f, 0.5f );
	REQUIRE( ( cRGBA.r == 1.0f && IsCloseEnough( cRGBA.a, 0.5f ) ) );

	const ae::Color cSRGB = ae::Color::SRGB( 1.0f, 0.0f, 0.0f );
	REQUIRE( ( IsCloseEnough( cSRGB.r, 1.0f ) && cSRGB.g == 0.0f && cSRGB.b == 0.0f ) );

	const ae::Color c8 = ae::Color::RGB8( 255, 0, 0 );
	REQUIRE( ( IsCloseEnough( c8.r, 1.0f ) && c8.g == 0.0f && c8.b == 0.0f ) );

	const ae::Color c8a = ae::Color::RGBA8( 255, 0, 0, 128 );
	REQUIRE( IsCloseEnough( c8a.r, 1.0f ) );
	REQUIRE( IsCloseEnough( c8a.a, 128.0f / 255.0f, 0.01f ) );

	REQUIRE( ( ae::Color::White().r == 1.0f && ae::Color::White().g == 1.0f && ae::Color::White().b == 1.0f && ae::Color::White().a == 1.0f ) );
	REQUIRE( ( ae::Color::Black().r == 0.0f && ae::Color::Black().g == 0.0f && ae::Color::Black().b == 0.0f && ae::Color::Black().a == 1.0f ) );
	REQUIRE( ( ae::Color::Red().r == 1.0f && ae::Color::Red().g == 0.0f && ae::Color::Red().b == 0.0f ) );
}

TEST_CASE( "Color GetLinearRGB, GetSRGB round-trips", "[math]" )
{
	REQUIRE( ae::Color::RGB( 0.5f, 0.3f, 0.8f ).GetLinearRGB() == ae::Vec3( 0.5f, 0.3f, 0.8f ) );

	const ae::Vec3 srgbRt = ae::Color::SRGB( 0.5f, 0.3f, 0.8f ).GetSRGB();
	REQUIRE( IsCloseEnough( srgbRt.x, 0.5f, 0.01f ) );
	REQUIRE( IsCloseEnough( srgbRt.y, 0.3f, 0.01f ) );
	REQUIRE( IsCloseEnough( srgbRt.z, 0.8f, 0.01f ) );

	REQUIRE( IsCloseEnough( ae::Color::RGBToSRGB( ae::Color::SRGBToRGB( 0.5f ) ), 0.5f, 0.01f ) );
}

TEST_CASE( "Color HSV round-trip", "[math]" )
{
	const ae::Vec3 hsvRt = ae::Color::HSV( 0.5f, 0.8f, 0.6f ).GetHSV();
	REQUIRE( IsCloseEnough( hsvRt.x, 0.5f, 0.01f ) );
	REQUIRE( IsCloseEnough( hsvRt.y, 0.8f, 0.01f ) );
	REQUIRE( IsCloseEnough( hsvRt.z, 0.6f, 0.01f ) );

	const ae::Color hsvRed = ae::Color::HSV( 0.0f, 1.0f, 1.0f );
	REQUIRE( hsvRed.r > 0.0f );
	REQUIRE( IsCloseEnough( hsvRed.b, 0.0f, 0.01f ) );
}

TEST_CASE( "Color operators and modifiers", "[math]" )
{
	REQUIRE( ae::Color::RGB( 0.5f, 0.5f, 0.5f ) == ae::Color::RGB( 0.5f, 0.5f, 0.5f ) );
	REQUIRE( ae::Color::RGB( 0.5f, 0.5f, 0.5f ) != ae::Color::RGB( 0.5f, 0.5f, 0.6f ) );

	const ae::Color cmul = ae::Color::RGB( 0.5f, 0.3f, 0.8f ) * ae::Color::RGB( 1.0f, 1.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( cmul.r, 0.5f ) && IsCloseEnough( cmul.g, 0.3f ) && IsCloseEnough( cmul.b, 0.8f ) ) );

	const ae::Color cSRgb = ae::Color::RGB( 0.5f, 0.3f, 0.8f ).ScaleRGB( 2.0f );
	REQUIRE( IsCloseEnough( cSRgb.r, 1.0f ) );
	REQUIRE( IsCloseEnough( cSRgb.g, 0.6f ) );
	REQUIRE( IsCloseEnough( cSRgb.b, 1.6f ) );
	REQUIRE( IsCloseEnough( cSRgb.a, 1.0f ) );

	const ae::Color cSA = ae::Color::RGBA( 0.5f, 0.3f, 0.8f, 0.4f ).ScaleA( 0.5f );
	REQUIRE( IsCloseEnough( cSA.a, 0.2f ) );
	REQUIRE( IsCloseEnough( cSA.r, 0.5f ) );

	const ae::Color cSetA = ae::Color::RGBA( 0.5f, 0.3f, 0.8f, 0.4f ).SetA( 0.0f );
	REQUIRE( cSetA.a == 0.0f );
	REQUIRE( IsCloseEnough( cSetA.r, 0.5f ) );

	const ae::Color cLS = ae::Color::RGB( 0.0f, 0.0f, 0.0f );
	const ae::Color cLE = ae::Color::RGB( 1.0f, 1.0f, 1.0f );
	REQUIRE( cLS.Lerp( cLE, 0.0f ) == cLS );
	REQUIRE( cLS.Lerp( cLE, 1.0f ) == cLE );
	REQUIRE( IsCloseEnough( cLS.Lerp( cLE, 0.5f ).r, 0.5f ) );

	const ae::Color cDt = ae::Color::RGB( 0.5f, 0.3f, 0.8f ).DtLerp( 0.0f, 1.0f, ae::Color::RGB( 1.0f, 1.0f, 1.0f ) );
	REQUIRE( IsCloseEnough( cDt.r, 0.5f ) );
}

TEST_CASE( "Color SetHue, SetSaturation, SetValue", "[math]" )
{
	REQUIRE( IsCloseEnough( ae::Color::HSV( 0.5f, 0.8f, 0.6f ).SetHue( 0.3f ).GetHSV().x, 0.3f, 0.01f ) );
	REQUIRE( IsCloseEnough( ae::Color::HSV( 0.5f, 0.8f, 0.6f ).SetSaturation( 0.2f ).GetHSV().y, 0.2f, 0.01f ) );
	REQUIRE( IsCloseEnough( ae::Color::HSV( 0.5f, 0.8f, 0.6f ).SetValue( 0.9f ).GetHSV().z, 0.9f, 0.01f ) );
}

TEST_CASE( "Matrix4 Identity and Translation", "[math]" )
{
	REQUIRE( ae::Matrix4::Identity() * ae::Vec4( 1.0f, 2.0f, 3.0f, 1.0f ) == ae::Vec4( 1.0f, 2.0f, 3.0f, 1.0f ) );

	const ae::Vec4 tTr = ae::Matrix4::Translation( ae::Vec3( 5.0f, 6.0f, 7.0f ) ) * ae::Vec4( 1.0f, 2.0f, 3.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( tTr.x, 6.0f ) && IsCloseEnough( tTr.y, 8.0f ) && IsCloseEnough( tTr.z, 10.0f ) ) );

	REQUIRE( ae::Matrix4::Translation( ae::Vec3( 5.0f, 6.0f, 7.0f ) ).GetTranslation() == ae::Vec3( 5.0f, 6.0f, 7.0f ) );
}

TEST_CASE( "Matrix4 RotationX/Y/Z", "[math]" )
{
	const ae::Vec4 mRz = ae::Matrix4::RotationZ( ae::HalfPi ) * ae::Vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( mRz.x, 0.0f ) && IsCloseEnough( mRz.y, 1.0f ) && IsCloseEnough( mRz.z, 0.0f ) ) );

	const ae::Vec4 mRx = ae::Matrix4::RotationX( ae::HalfPi ) * ae::Vec4( 0.0f, 1.0f, 0.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( mRx.x, 0.0f ) && IsCloseEnough( mRx.y, 0.0f ) && IsCloseEnough( mRx.z, 1.0f ) ) );

	const ae::Vec4 mRy = ae::Matrix4::RotationY( ae::HalfPi ) * ae::Vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( mRy.x, 0.0f ) && IsCloseEnough( mRy.y, 0.0f ) && IsCloseEnough( mRy.z, -1.0f ) ) );
}

TEST_CASE( "Matrix4 Rotation from Quaternion", "[math]" )
{
	const ae::Vec4 mRq = ae::Matrix4::Rotation( ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi ) ) * ae::Vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( mRq.x, 0.0f ) && IsCloseEnough( mRq.y, 1.0f ) && IsCloseEnough( mRq.z, 0.0f ) ) );
}

TEST_CASE( "Matrix4 Scaling", "[math]" )
{
	const ae::Vec4 mSc = ae::Matrix4::Scaling( 2.0f, 3.0f, 4.0f ) * ae::Vec4( 1.0f, 1.0f, 1.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( mSc.x, 2.0f ) && IsCloseEnough( mSc.y, 3.0f ) && IsCloseEnough( mSc.z, 4.0f ) ) );
}

TEST_CASE( "Matrix4 LocalToWorld", "[math]" )
{
	const ae::Matrix4 mLtw = ae::Matrix4::LocalToWorld( ae::Vec3( 5.0f, 0.0f, 0.0f ), ae::Quaternion::Identity(), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	const ae::Vec4 pLtw = mLtw * ae::Vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( pLtw.x, 5.0f ) && IsCloseEnough( pLtw.y, 0.0f ) && IsCloseEnough( pLtw.z, 0.0f ) ) );
	REQUIRE( mLtw.GetTranslation() == ae::Vec3( 5.0f, 0.0f, 0.0f ) );
}

TEST_CASE( "Matrix4 WorldToView", "[math]" )
{
	const ae::Matrix4 mWtv = ae::Matrix4::WorldToView( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	const ae::Vec4 pWtv = mWtv * ae::Vec4( 0.0f, 0.0f, 3.0f, 1.0f );
	REQUIRE( IsCloseEnough( pWtv.x, 0.0f ) );
	REQUIRE( IsCloseEnough( pWtv.y, 0.0f ) );
	REQUIRE( IsCloseEnough( pWtv.z, -3.0f ) );
	REQUIRE( IsCloseEnough( pWtv.w, 1.0f ) );
}

TEST_CASE( "Matrix4 ViewToProjection", "[math]" )
{
	const ae::Matrix4 mVtp = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 1.0f, 100.0f );
	const ae::Vec4 pVtp = mVtp * ae::Vec4( 0.0f, 0.0f, -1.0f, 1.0f );
	REQUIRE( IsCloseEnough( pVtp.x, 0.0f ) );
	REQUIRE( IsCloseEnough( pVtp.y, 0.0f ) );
}

TEST_CASE( "Matrix4 SetTranslation GetTranslation", "[math]" )
{
	ae::Matrix4 mStgt = ae::Matrix4::Identity();
	mStgt.SetTranslation( ae::Vec3( 3.0f, 4.0f, 5.0f ) );
	REQUIRE( mStgt.GetTranslation() == ae::Vec3( 3.0f, 4.0f, 5.0f ) );
}

TEST_CASE( "Matrix4 SetRotation GetRotation", "[math]" )
{
	const ae::Quaternion qSrgr = ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi );
	ae::Matrix4 mSrgr = ae::Matrix4::Identity();
	mSrgr.SetRotation( qSrgr );
	REQUIRE( mSrgr.GetRotation().Dot( qSrgr ) > 0.999f );
}

TEST_CASE( "Matrix4 GetTranspose", "[math]" )
{
	const ae::Vec4 tpIn( 1.0f, 2.0f, 3.0f, 1.0f );
	const ae::Vec4 tpR0 = ae::Matrix4::Identity().GetTranspose() * tpIn;
	const ae::Vec4 tpR1 = ae::Matrix4::Identity() * tpIn;
	REQUIRE( ( IsCloseEnough( tpR0.x, tpR1.x ) && IsCloseEnough( tpR0.y, tpR1.y ) && IsCloseEnough( tpR0.z, tpR1.z ) ) );

	const ae::Matrix4 mTp = ae::Matrix4::Translation( ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	const ae::Vec4 tpOrig = mTp * tpIn;
	const ae::Vec4 tpDbl = mTp.GetTranspose().GetTranspose() * tpIn;
	REQUIRE( ( IsCloseEnough( tpOrig.x, tpDbl.x ) && IsCloseEnough( tpOrig.y, tpDbl.y ) && IsCloseEnough( tpOrig.z, tpDbl.z ) ) );
}

TEST_CASE( "Matrix4 GetInverse", "[math]" )
{
	const ae::Matrix4 mGi = ae::Matrix4::LocalToWorld( ae::Vec3( 1.0f, 2.0f, 3.0f ), ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi ), ae::Vec3( 2.0f, 1.0f, 1.0f ) );
	const ae::Vec4 giIn( 1.0f, 2.0f, 3.0f, 1.0f );
	const ae::Vec4 giT = mGi * giIn;
	const ae::Vec4 giR = mGi.GetInverse() * giT;
	REQUIRE( IsCloseEnough( giR.x, giIn.x ) );
	REQUIRE( IsCloseEnough( giR.y, giIn.y ) );
	REQUIRE( IsCloseEnough( giR.z, giIn.z ) );
}

TEST_CASE( "Matrix4 GetNormalMatrix", "[math]" )
{
	const ae::Matrix4 mNm = ae::Matrix4::Scaling( 2.0f, 1.0f, 1.0f ).GetNormalMatrix();
	const ae::Vec4 nNm = mNm * ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f );
	const ae::Vec3 nDir = ae::Vec3( nNm.x, nNm.y, nNm.z ).NormalizeCopy();
	REQUIRE( IsCloseEnough( ae::Vec3::Dot( nDir, ae::Vec3( 0.0f, 1.0f, 0.0f ) ), 0.0f ) );
}

TEST_CASE( "Matrix4 GetScaleRemoved", "[math]" )
{
	const ae::Matrix4 mGsr = ae::Matrix4::LocalToWorld( ae::Vec3( 1.0f, 2.0f, 3.0f ), ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::HalfPi ), ae::Vec3( 2.0f, 3.0f, 4.0f ) );
	const ae::Matrix4 mNs = mGsr.GetScaleRemoved();
	REQUIRE( IsCloseEnough( mNs.GetTranslation().x, 1.0f ) );
	REQUIRE( IsCloseEnough( mNs.GetTranslation().y, 2.0f ) );
	REQUIRE( IsCloseEnough( mNs.GetTranslation().z, 3.0f ) );
	REQUIRE( IsCloseEnough( mNs.GetAxis( 0 ).Length(), 1.0f ) );
	REQUIRE( IsCloseEnough( mNs.GetAxis( 1 ).Length(), 1.0f ) );
	REQUIRE( IsCloseEnough( mNs.GetAxis( 2 ).Length(), 1.0f ) );
}

TEST_CASE( "Matrix4 GetAxis SetAxis GetRow SetRow", "[math]" )
{
	REQUIRE( ae::Matrix4::Identity().GetAxis( 0 ) == ae::Vec3( 1.0f, 0.0f, 0.0f ) );

	ae::Matrix4 mAxS = ae::Matrix4::Identity();
	mAxS.SetAxis( 0, ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	REQUIRE( mAxS.GetAxis( 0 ) == ae::Vec3( 0.0f, 1.0f, 0.0f ) );

	REQUIRE( ae::Matrix4::Identity().GetRow( 0 ) == ae::Vec4( 1.0f, 0.0f, 0.0f, 0.0f ) );

	ae::Matrix4 mRowS = ae::Matrix4::Identity();
	mRowS.SetRow( 1, ae::Vec3( 2.0f, 3.0f, 4.0f ) );
	const ae::Vec4 rowR = mRowS.GetRow( 1 );
	REQUIRE( ( IsCloseEnough( rowR.x, 2.0f ) && IsCloseEnough( rowR.y, 3.0f ) && IsCloseEnough( rowR.z, 4.0f ) ) );
}

TEST_CASE( "Matrix4 operator* matrix", "[math]" )
{
	const ae::Matrix4 mMul = ae::Matrix4::Translation( ae::Vec3( 1.0f, 0.0f, 0.0f ) ) * ae::Matrix4::Translation( ae::Vec3( 2.0f, 0.0f, 0.0f ) );
	const ae::Vec4 pMul = mMul * ae::Vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	REQUIRE( ( IsCloseEnough( pMul.x, 3.0f ) && IsCloseEnough( pMul.y, 0.0f ) && IsCloseEnough( pMul.z, 0.0f ) ) );
}

TEST_CASE( "Matrix4 TransformPoint3x4 and TransformVector3x4", "[math]" )
{
	const ae::Matrix4 mTp3 = ae::Matrix4::Translation( ae::Vec3( 5.0f, 0.0f, 0.0f ) );
	const ae::Vec3 tp3Pt = mTp3.TransformPoint3x4( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( ( IsCloseEnough( tp3Pt.x, 6.0f ) && IsCloseEnough( tp3Pt.y, 0.0f ) && IsCloseEnough( tp3Pt.z, 0.0f ) ) );
	const ae::Vec3 tp3Vec = mTp3.TransformVector3x4( ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( ( IsCloseEnough( tp3Vec.x, 1.0f ) && IsCloseEnough( tp3Vec.y, 0.0f ) && IsCloseEnough( tp3Vec.z, 0.0f ) ) );
}

TEST_CASE( "Matrix4 IsNAN", "[math]" )
{
	REQUIRE( ae::Matrix4::Identity().IsNAN() == false );
	ae::Matrix4 mNaN = ae::Matrix4::Identity();
	mNaN.data[ 0 ] = 0.0f / 0.0f;
	REQUIRE( mNaN.IsNAN() == true );
}
