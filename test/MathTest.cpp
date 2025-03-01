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
