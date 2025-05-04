//------------------------------------------------------------------------------
// aeMapTest.cpp
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
	ae::HashMap< uint32_t, 10 > map;
	REQUIRE( map.Size() == 10 );
	for ( uint32_t i = 0; i < 10; i++ )
	{
		REQUIRE( map.Set( 100 + i, i ) );
	}
	REQUIRE( map.Length() == 10 );
	for ( uint32_t i = 0; i < 10; i++ )
	{
		REQUIRE( !map.Set( 1000 + i, i ) );
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
		REQUIRE( map.Set( 100, 777 ) );
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
	ae::HashMap< uint32_t, 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Set( -2, 776 ) );
	REQUIRE( map.Set( 4, 777 ) );
	REQUIRE( map.Set( 14, 778 ) );
	REQUIRE( map.Set( 104, 779 ) );
	REQUIRE( map.Set( 1004, 780 ) );
	REQUIRE( !map.Set( 10004, 781 ) );
	REQUIRE( map.Length() == 5 );
	REQUIRE( !map.Set( 5, 780 ) );
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

TEST_CASE( "hash map compaction", "[ae::HashMap]" )
{
	ae::HashMap< uint32_t, 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Set( 0, 0 ) );
	REQUIRE( map.Set( 1, 1 ) );
	REQUIRE( map.Set( 5, 5 ) );
	REQUIRE( map.Set( 6, 1 ) );
	REQUIRE( map.Length() == 4 );

	REQUIRE( map.Remove( 0 ) == 0 );
	REQUIRE( map.Length() == 3 );
	REQUIRE( map.Get( 0 ) == -1 );
	REQUIRE( map.Get( 1 ) == 1 );
	REQUIRE( map.Get( 5 ) == 5 );
	REQUIRE( map.Get( 6 ) == 1 );
}

TEST_CASE( "hash map compaction 2", "[ae::HashMap]" )
{
	ae::HashMap< uint32_t, 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Set( 0, 0 ) );
	REQUIRE( map.Set( 1, 0 ) );
	REQUIRE( map.Set( 6, 0 ) );
	REQUIRE( map.Set( 5, 0 ) );
	REQUIRE( map.Length() == 4 );

	REQUIRE( map.Remove( 0 ) == 0 );
	REQUIRE( map.Length() == 3 );
	REQUIRE( map.Get( 0 ) == -1 );
	REQUIRE( map.Get( 1 ) == 0 );
	REQUIRE( map.Get( 6 ) == 0 );
	REQUIRE( map.Get( 5 ) == 0 );
}

TEST_CASE( "hash map compaction (wrapping)", "[ae::HashMap]" )
{
	ae::HashMap< uint32_t, 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Set( 3, 0 ) );
	REQUIRE( map.Set( 4, 0 ) );
	REQUIRE( map.Set( 8, 0 ) );
	REQUIRE( map.Set( 9, 0 ) );
	REQUIRE( map.Length() == 4 );

	REQUIRE( map.Remove( 3 ) == 0 );
	REQUIRE( map.Length() == 3 );
	REQUIRE( map.Get( 3 ) == -1 );
	REQUIRE( map.Get( 4 ) == 0 );
	REQUIRE( map.Get( 8 ) == 0 );
	REQUIRE( map.Get( 9 ) == 0 );
}

TEST_CASE( "hash map compaction (wrapping 2)", "[ae::HashMap]" )
{
	ae::HashMap< uint32_t, 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Set( 3, 0 ) );
	REQUIRE( map.Set( 4, 0 ) );
	REQUIRE( map.Set( 9, 0 ) );
	REQUIRE( map.Set( 8, 0 ) );
	REQUIRE( map.Length() == 4 );

	REQUIRE( map.Remove( 3 ) == 0 );
	REQUIRE( map.Length() == 3 );
	REQUIRE( map.Get( 3 ) == -1 );
	REQUIRE( map.Get( 4 ) == 0 );
	REQUIRE( map.Get( 9 ) == 0 );
	REQUIRE( map.Get( 8 ) == 0 );
}

TEST_CASE( "hash map compaction (wrapping 3)", "[ae::HashMap]" )
{
	ae::HashMap< uint32_t, 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Set( 3, 0 ) );
	REQUIRE( map.Set( 4, 0 ) );
	REQUIRE( map.Set( 0, 0 ) );
	REQUIRE( map.Set( 9, 0 ) );
	REQUIRE( map.Length() == 4 );

	REQUIRE( map.Remove( 3 ) == 0 );
	REQUIRE( map.Length() == 3 );
	REQUIRE( map.Get( 3 ) == -1 );
	REQUIRE( map.Get( 4 ) == 0 );
	REQUIRE( map.Get( 0 ) == 0 );
	REQUIRE( map.Get( 9 ) == 0 );
}

//------------------------------------------------------------------------------
// aeMap tests
//------------------------------------------------------------------------------
TEST_CASE( "map elements can be set and retrieved", "[aeMap]" )
{
	ae::Map< int, char, 10 > map;
	for ( char i = 0; i < 10; i++ )
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
		for ( char i = 0; i < 10; i++ )
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

struct BadHash
{
	BadHash() {}
	BadHash( uint32_t value ) : value( value ) {}
	bool operator==( const BadHash& other ) const { return value == other.value; }
	uint32_t value;
};
namespace ae { template <> uint32_t GetHash( const BadHash& badHash )
{
	return badHash.value % 2;
} }
TEST_CASE( "map handles collisions", "[ae::HashMap]" )
{
	ae::Map< BadHash, char, 5 > map;
	REQUIRE( map.Size() == 5 );
	REQUIRE( map.Set( -2, 'A' ) );
	REQUIRE( map.Set( 4, 'B' ) );
	REQUIRE( map.Set( 14, 'C' ) );
	REQUIRE( map.Set( 104, 'D' ) );
	REQUIRE( map.Set( 1004, 'E' ) );
	
	REQUIRE( map.Length() == 5 );
	REQUIRE( map.Get( -2 ) == 'A' );
	REQUIRE( map.Get( 4 ) == 'B' );
	REQUIRE( map.Get( 14 ) == 'C' );
	REQUIRE( map.Get( 104 ) == 'D' );
	REQUIRE( map.Get( 1004 ) == 'E' );

	char value = 0;
	REQUIRE( map.Remove( -2, &value ) );
	REQUIRE( value == 'A' );
	REQUIRE( map.Length() == 4 );
	REQUIRE( !map.TryGet( -2 ) );
	REQUIRE( map.Get( 4 ) == 'B' );
	REQUIRE( map.Get( 14 ) == 'C' );
	REQUIRE( map.Get( 104 ) == 'D' );
	REQUIRE( map.Get( 1004 ) == 'E' );

	value = 0;
	REQUIRE( map.Remove( 14, &value ) );
	REQUIRE( value == 'C' );
	REQUIRE( map.Length() == 3 );
	REQUIRE( !map.TryGet( -2 ) );
	REQUIRE( map.Get( 4 ) == 'B' );
	REQUIRE( !map.TryGet( 14 ) );
	REQUIRE( map.Get( 104 ) == 'D' );
	REQUIRE( map.Get( 1004 ) == 'E' );
}

TEST_CASE( "dynamic hash map elements can be set and retrieved", "[ae::HashMap]" )
{
	ae::HashMap< uint32_t > map = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map.Set( 1000 + i, i ) );
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
	ae::HashMap< uint32_t > map = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map.Set( 1000 + i, i ) );
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
	ae::HashMap<  uint32_t, 128 > map0;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	map0.Reserve( 100 );
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() == 128 );
	
	ae::HashMap< uint32_t, 128 > map1 = map0;
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
	ae::HashMap< uint32_t, 128 > map0;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	map0.Reserve( 100 );
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() == 128 );
	
	ae::HashMap< uint32_t, 128 > map1;
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
	ae::HashMap< uint32_t > map0 = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	
	ae::HashMap< uint32_t > map1 = TAG_TEST;
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
	ae::HashMap< uint32_t > map0 = TAG_TEST;
	ae::HashMap< uint32_t > map1 = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
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
	ae::HashMap< uint32_t > map0 = TAG_TEST;
	ae::HashMap< uint32_t > map1 = TAG_TEST;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
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

TEST_CASE( "fast stress test", "[ae::Map]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t > map = TAG_TEST;
	for ( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
	}
	REQUIRE( map.Length() == count );
	for ( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "fast stress test 2", "[ae::Map]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t > map = TAG_TEST;
	auto validateFn = [&]()
	{
		for ( auto& pair : map )
		{
			uint32_t* v = map.TryGet( pair.key );
			AE_ASSERT( v );
			AE_ASSERT( *v == pair.value );
		}
	};

	for ( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
		validateFn();
	}
	REQUIRE( map.Length() == count );

	uint64_t r = 543534;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		uint32_t removeCount = ae::Random( 0, map.Length(), &r );
		for ( uint32_t j = 0; j < removeCount; j++ )
		{
			uint32_t idx = ae::Random( 0, map.Length(), &r );
			uint32_t key = map.GetKey( idx );
			uint32_t value = map.GetValue( idx );
			uint32_t valueCheck = 0;
			AE_ASSERT( map.Remove( key, &valueCheck ) );
			AE_ASSERT( value == valueCheck );
			validateFn();
		}
		
		removeCount = ae::Random( 0, map.Length(), &r );
		for ( uint32_t j = 0; j < removeCount; j++ )
		{
			uint32_t idx = ae::Random( 0, map.Length(), &r );
			uint32_t value = map.GetValue( idx );
			uint32_t valueCheck = 0;
			map.RemoveIndex( idx, &valueCheck );
			AE_ASSERT( value == valueCheck );
			validateFn();
		}

		uint32_t addCount = ae::Random( 0, map.Length(), &r );
		for ( uint32_t j = 0; j < addCount; j++ )
		{
			map.Set( ae::Random( 0, 100000, &r ), 0 );
			validateFn();
		}
	}

	while ( map.Length() )
	{
		uint32_t idx = ae::Random( 0, map.Length(), &r );
		uint32_t key = map.GetKey( idx );
		AE_ASSERT( map.Remove( key ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "fast full map stress test", "[ae::Map]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 10000 > map;
	for ( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
	}
	REQUIRE( map.Length() == count );
	for ( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "stable stress test", "[ae::Map]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 0, ae::MapMode::Stable > map = TAG_TEST;
	auto validateFn = [&]()
	{
		for ( uint32_t i = 1; i < map.Length(); i++ )
		{
			AE_ASSERT( map.GetValue( i - 1 ) < map.GetValue( i ) );
		}
	};

	for ( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
	}
	validateFn();
	REQUIRE( map.Length() == count );
	for ( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "stable stress test 2", "[ae::Map]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 0, ae::MapMode::Stable > map = TAG_TEST;
	auto validateFn = [&]()
	{
		for ( auto& pair : map )
		{
			uint32_t* v = map.TryGet( pair.key );
			AE_ASSERT( v );
			AE_ASSERT( *v == pair.value );
		}
		for ( uint32_t i = 1; i < map.Length(); i++ )
		{
			AE_ASSERT( map.GetValue( i - 1 ) < map.GetValue( i ) );
		}
	};

	for ( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
		validateFn();
	}
	REQUIRE( map.Length() == count );

	uint64_t r = 543534;
	for ( uint32_t i = 0; i < 100; i++ )
	{
		uint32_t removeCount = ae::Random( 0, map.Length(), &r );
		for ( uint32_t j = 0; j < removeCount; j++ )
		{
			uint32_t idx = ae::Random( 0, map.Length(), &r );
			uint32_t key = map.GetKey( idx );
			uint32_t value = map.GetValue( idx );
			uint32_t valueCheck = 0;
			AE_ASSERT( map.Remove( key, &valueCheck ) );
			AE_ASSERT( value == valueCheck );
			validateFn();
		}
		
		removeCount = ae::Random( 0, map.Length(), &r );
		for ( uint32_t j = 0; j < removeCount; j++ )
		{
			uint32_t idx = ae::Random( 0, map.Length(), &r );
			uint32_t value = map.GetValue( idx );
			uint32_t valueCheck = 0;
			map.RemoveIndex( idx, &valueCheck );
			AE_ASSERT( value == valueCheck );
			validateFn();
		}

		uint32_t addCount = ae::Random( 0, map.Length(), &r );
		for ( uint32_t j = 0; j < addCount; j++ )
		{
			for (uint32_t k = 0; k < map.Length(); k++ )
			{
				map.GetValue( k ) = k;
			}
			uint32_t key = ae::Random( 0, 100000, &r );
			uint32_t val = map.Get( key, map.Length() );
			map.Set( key, val );
			validateFn();
		}
	}

	while ( map.Length() )
	{
		uint32_t idx = ae::Random( 0, map.Length(), &r );
		uint32_t key = map.GetKey( idx );
		AE_ASSERT( map.Remove( key ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "stable full map stress test", "[ae::Map]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 10000, ae::MapMode::Stable > map;
	auto validateFn = [&]()
	{
		for ( uint32_t i = 1; i < map.Length(); i++ )
		{
			AE_ASSERT( map.GetValue( i - 1 ) < map.GetValue( i ) );
		}
	};

	for ( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
		validateFn();
	}
	REQUIRE( map.Length() == count );

	for ( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}
