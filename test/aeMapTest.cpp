//------------------------------------------------------------------------------
// aeMapTest.cpp
// Copyright (c) John Hughes on 4/20/21. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "catch2/catch.hpp"
#include "ae/aether.h"

const ae::Tag TAG_TEST = "test";

//------------------------------------------------------------------------------
// ae::GetHash tests
//------------------------------------------------------------------------------
TEST_CASE( "GetHash() template function hashes values correctly", "[ae::HashMap]" )
{
	REQUIRE( ae::GetHash( 777u ) == 777u );
	REQUIRE( ae::GetHash( 777 ) == 777u );
	REQUIRE( ae::GetHash( -777 ) == 4294966519 );
	REQUIRE( ae::GetHash( (const void*)777u ) == 3331007037u );
	REQUIRE( ae::GetHash( (const float*)777u ) == 3331007037u );
	REQUIRE( ae::GetHash( (void*)777u ) == 3331007037u );
	REQUIRE( ae::GetHash( (float*)777u ) == 3331007037u );
	REQUIRE( ae::GetHash( (const char*)"777" ) == 2344304364 );
	REQUIRE( ae::GetHash( (char*)"777" ) == 2344304364 );
	REQUIRE( ae::GetHash( "777" ) == 2344304364 );
	REQUIRE( ae::GetHash( ae::Str< 32 >( "777" ) ) == 2344304364 );
	REQUIRE( ae::GetHash( ae::Str< 128 >( "777" ) ) == 2344304364 );
	REQUIRE( ae::GetHash( std::string(  "777"  ) ) == 2344304364 );
	REQUIRE( ae::GetHash( ae::Hash().HashString(  "777"  ) ) == 2344304364 );
	REQUIRE( ae::GetHash( ae::Int3( 1, 2, 3 ) ) == 316 );
	REQUIRE( ae::GetHash( ae::Int3( -1, 0, 1 ) ) == 19 );
	//REQUIRE( ae::Vec3( 0.0f ) ) == 0 ); // Should fail to link
}

//------------------------------------------------------------------------------
// ae::HashMap tests
//------------------------------------------------------------------------------
TEST_CASE( "hash map elements can be set and retrieved", "[ae::HashMap]" )
{
	ae::HashMap< 10 > map;
	REQUIRE( map.Size() == 10 );
	for ( uint32_t i = 0; i < 10; i++ )
	{
		REQUIRE( map.Insert( 100 + i, i ) );
	}
	REQUIRE( map.Length() == 10 );
	for ( uint32_t i = 0; i < 10; i++ )
	{
		REQUIRE( !map.Insert( 1000 + i, i ) );
	}
	REQUIRE( map.Length() == 10 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map.Get( -1 ) == -1 );
		REQUIRE( map.Get( 100 ) == 0 );
		REQUIRE( map.Get( 101 ) == 1 );
		REQUIRE( map.Get( 102 ) == 2 );
		REQUIRE( map.Get( 103 ) == 3 );
		REQUIRE( map.Get( 104 ) == 4 );
		REQUIRE( map.Get( 105 ) == 5 );
		REQUIRE( map.Get( 106 ) == 6 );
		REQUIRE( map.Get( 107 ) == 7 );
		REQUIRE( map.Get( 108 ) == 8 );
		REQUIRE( map.Get( 109 ) == 9 );
		REQUIRE( map.Get( 10 ) == -1 );
	}

	SECTION( "can retrieve previously set values after removal" )
	{
		REQUIRE( map.Remove( 101 ) == 1 );
		REQUIRE( map.Get( 101 ) == -1 );
		REQUIRE( map.Remove( 103 ) == 3 );
		REQUIRE( map.Get( 103 ) == -1 );
		REQUIRE( map.Remove( 109 ) == 9 );
		REQUIRE( map.Get( 109 ) == -1 );
		REQUIRE( map.Remove( 105 ) == 5 );
		REQUIRE( map.Get( 105 ) == -1 );
		REQUIRE( map.Remove( 100 ) == 0 );
		REQUIRE( map.Get( 100 ) == -1 );
		REQUIRE( map.Remove( 102 ) == 2 );
		REQUIRE( map.Get( 102 ) == -1 );
		REQUIRE( map.Remove( 106 ) == 6 );
		REQUIRE( map.Get( 106 ) == -1 );
		REQUIRE( map.Remove( 108 ) == 8 );
		REQUIRE( map.Get( 108 ) == -1 );
		REQUIRE( map.Remove( 104 ) == 4 );
		REQUIRE( map.Get( 104 ) == -1 );
		REQUIRE( map.Remove( 107 ) == 7 );
		REQUIRE( map.Get( 107 ) == -1 );
	}
	
	SECTION( "can update and read previously set values" )
	{
		REQUIRE( map.Get( 100 ) == 0 );
		REQUIRE( map.Insert( 100, 777 ) );
		REQUIRE( map.Get( 100 ) == 777 );
	}
	
	SECTION( "can decrement previously set values" )
	{
		map.Decrement( 4 );
		REQUIRE( map.Get( 100 ) == 0 );
		REQUIRE( map.Get( 101 ) == 1 );
		REQUIRE( map.Get( 102 ) == 2 );
		REQUIRE( map.Get( 103 ) == 3 );
		REQUIRE( map.Get( 104 ) == 4 );
		REQUIRE( map.Get( 105 ) == 4 );
		REQUIRE( map.Get( 106 ) == 5 );
		REQUIRE( map.Get( 107 ) == 6 );
		REQUIRE( map.Get( 108 ) == 7 );
		REQUIRE( map.Get( 109 ) == 8 );
	}
}

TEST_CASE( "hash map handles collisions", "[ae::HashMap]" )
{
	ae::HashMap< 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Insert( -2, 776 ) );
	REQUIRE( map.Insert( 4, 777 ) );
	REQUIRE( map.Insert( 14, 778 ) );
	REQUIRE( map.Insert( 104, 779 ) );
	REQUIRE( map.Insert( 1004, 780 ) );
	REQUIRE( !map.Insert( 10004, 781 ) );
	REQUIRE( map.Length() == 5 );
	REQUIRE( !map.Insert( 5, 780 ) );
	REQUIRE( map.Length() == 5 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map.Get( -2 ) == 776 );
		REQUIRE( map.Get( 4 ) == 777 );
		REQUIRE( map.Get( 14 ) == 778 );
		REQUIRE( map.Get( 104 ) == 779 );
		REQUIRE( map.Get( 1004 ) == 780 );
		REQUIRE( map.Get( 10004 ) == -1 );
	}

	SECTION( "can retrieve previously set values after removal" )
	{
		REQUIRE( map.Remove( 104 ) == 779 );
		REQUIRE( map.Length() == 4 );
		REQUIRE( map.Get( -2 ) == 776 );
		REQUIRE( map.Get( 4 ) == 777 );
		REQUIRE( map.Get( 14 ) == 778 );
		REQUIRE( map.Get( 104 ) == -1 );
		REQUIRE( map.Get( 1004 ) == 780 );

		REQUIRE( map.Remove( 14 ) == 778 );
		REQUIRE( map.Length() == 3 );
		REQUIRE( map.Get( -2 ) == 776 );
		REQUIRE( map.Get( 4 ) == 777 );
		REQUIRE( map.Get( 14 ) == -1 );
		REQUIRE( map.Get( 104 ) == -1 );
		REQUIRE( map.Get( 1004 ) == 780 );

		REQUIRE( map.Remove( 1004 ) == 780 );
		REQUIRE( map.Length() == 2 );
		REQUIRE( map.Get( -2 ) == 776 );
		REQUIRE( map.Get( 4 ) == 777 );
		REQUIRE( map.Get( 14 ) == -1 );
		REQUIRE( map.Get( 104 ) == -1 );
		REQUIRE( map.Get( 1004 ) == -1 );

		REQUIRE( map.Remove( -2 ) == 776 );
		REQUIRE( map.Length() == 1 );
		REQUIRE( map.Get( -2 ) == -1 );
		REQUIRE( map.Get( 4 ) == 777 );
		REQUIRE( map.Get( 14 ) == -1 );
		REQUIRE( map.Get( 104 ) == -1 );
		REQUIRE( map.Get( 1004 ) == -1 );

		REQUIRE( map.Remove( 4 ) == 777 );
		REQUIRE( map.Length() == 0 );
		REQUIRE( map.Get( -2 ) == -1 );
		REQUIRE( map.Get( 4 ) == -1 );
		REQUIRE( map.Get( 14 ) == -1 );
		REQUIRE( map.Get( 104 ) == -1 );
		REQUIRE( map.Get( 1004 ) == -1 );
	}
}

//------------------------------------------------------------------------------
// aeMap tests
//------------------------------------------------------------------------------
TEST_CASE( "map elements can be set and retrieved", "[aeMap]" )
{
	ae::Map< int, char, 10 > map;
	for ( uint32_t i = 0; i < 10; i++ )
	{
		map.Set( i, 'a' + i );
	}
	REQUIRE( map.Length() == 10 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map.Get( 0 )  == 'a' );
		REQUIRE( map.Get( 1 )  == 'b' );
		REQUIRE( map.Get( 2 )  == 'c' );
		REQUIRE( map.Get( 3 )  == 'd' );
		REQUIRE( map.Get( 4 )  == 'e' );
		REQUIRE( map.Get( 5 )  == 'f' );
		REQUIRE( map.Get( 6 )  == 'g' );
		REQUIRE( map.Get( 7 )  == 'h' );
		REQUIRE( map.Get( 8 )  == 'i' );
		REQUIRE( map.Get( 9 )  == 'j' );
	}

	SECTION( "existing values are correctly set" )
	{
		for ( uint32_t i = 0; i < 10; i++ )
		{
			map.Set( i, 'A' + i );
		}
		REQUIRE( map.Length() == 10 );
		
		REQUIRE( map.Get( 0 )  == 'A' );
		REQUIRE( map.Get( 1 )  == 'B' );
		REQUIRE( map.Get( 2 )  == 'C' );
		REQUIRE( map.Get( 3 )  == 'D' );
		REQUIRE( map.Get( 4 )  == 'E' );
		REQUIRE( map.Get( 5 )  == 'F' );
		REQUIRE( map.Get( 6 )  == 'G' );
		REQUIRE( map.Get( 7 )  == 'H' );
		REQUIRE( map.Get( 8 )  == 'I' );
		REQUIRE( map.Get( 9 )  == 'J' );
	}
}

TEST_CASE( "dynamic hash map elements can be set and retrieved", "[ae::HashMap]" )
{
	ae::HashMap<> map = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map.Insert( 1000 + i, i ) );
	}
	REQUIRE( map.Length() == 100 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map.Get( -1 ) == -1 );
		for ( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map.Get( 1000 + i ) == i );
		}
		REQUIRE( map.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after reserve", "[ae::HashMap]" )
{
	ae::HashMap<> map = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map.Insert( 1000 + i, i ) );
	}
	REQUIRE( map.Length() == 100 );
	map.Reserve( 1000 );
	REQUIRE( map.Length() == 100 );
	REQUIRE( map.Size() >= 1000 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map.Get( -1 ) == -1 );
		for ( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map.Get( 1000 + i ) == i );
		}
		REQUIRE( map.Get( -1 ) == -1 );
	}
}

TEST_CASE( "copy construct static hash map", "[ae::HashMap]" )
{
	ae::HashMap< 128 > map0;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Insert( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	map0.Reserve( 100 );
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() == 128 );
	
	ae::HashMap< 128 > map1 = map0;
	REQUIRE( map1.Length() == 100 );
	REQUIRE( map1.Size() == 128 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
		for ( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "copy assign static hash map", "[ae::HashMap]" )
{
	ae::HashMap< 128 > map0;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Insert( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	map0.Reserve( 100 );
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() == 128 );
	
	ae::HashMap< 128 > map1;
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() == 128 );
	map1 = map0;
	REQUIRE( map1.Length() == 100 );
	REQUIRE( map1.Size() == 128 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
		for ( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after assignment of same size map", "[ae::HashMap]" )
{
	ae::HashMap<> map0 = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Insert( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	
	ae::HashMap<> map1 = TAG_TEST;
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() == 0 );
	map1.Reserve( map0.Size() );
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() == map0.Size() );
	map1 = map0;
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	REQUIRE( map1.Length() == 100 );
	REQUIRE( map1.Size() >= 100 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
		for ( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after assignment of smaller map", "[ae::HashMap]" )
{
	ae::HashMap<> map0 = TAG_TEST;
	ae::HashMap<> map1 = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Insert( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() == 0 );
	
	map1.Reserve( 1000 );
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() >= 1000 );
	
	map1 = map0;
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	REQUIRE( map1.Length() == 100 );
	REQUIRE( map1.Size() >= 100 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
		for ( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after assignment of bigger map", "[ae::HashMap]" )
{
	ae::HashMap<> map0 = TAG_TEST;
	ae::HashMap<> map1 = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Insert( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() == 0 );
	
	map1.Reserve( 10 );
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() >= 10 );
	
	map1 = map0;
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	REQUIRE( map1.Length() == 100 );
	REQUIRE( map1.Size() >= 100 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
		for ( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

