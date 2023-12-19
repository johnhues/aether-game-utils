//------------------------------------------------------------------------------
// Math.cpp
// Copyright (c) John Hughes on 12/19/23. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "catch2/catch.hpp"
#include "aether.h"

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
