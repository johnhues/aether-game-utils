//------------------------------------------------------------------------------
// MapTest.h
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
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_TEST = "test";

//------------------------------------------------------------------------------
// ae::HashMap tests
//------------------------------------------------------------------------------
TEST_CASE( "hash map elements can be set and retrieved", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 10, aeHashN > map;
	REQUIRE( map.Size() == 10 );
	for( uint32_t i = 0; i < 10; i++ )
	{
		REQUIRE( map.Set( 100 + i, i ) );
	}
	REQUIRE( map.Length() == 10 );
	for( uint32_t i = 0; i < 10; i++ )
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

TEST_CASE( "hash map handles collisions", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 5, aeHashN > map;
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

TEST_CASE( "hash map compaction", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 5, aeHashN > map;
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

TEST_CASE( "hash map compaction 2", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 5, aeHashN > map;
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

TEST_CASE( "hash map compaction (wrapping)", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 5, aeHashN > map;
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

TEST_CASE( "hash map compaction (wrapping 2)", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 5, aeHashN > map;
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

TEST_CASE( "hash map compaction (wrapping 3)", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 5, aeHashN > map;
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
TEST_CASE( "map elements can be set and retrieved", "[aeMap" AE_HASH_N "]" )
{
	ae::Map< int, char, 10, aeHashN > map;
	for( char i = 0; i < 10; i++ )
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
		for( char i = 0; i < 10; i++ )
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

template< typename T >
struct BadHash
{
	BadHash() {}
	BadHash( T value ) : value( value ) {}
	uint32_t GetHash32() const { return (uint32_t)value % 2; }
	uint64_t GetHash64() const { return (uint64_t)value % 2; }
	bool operator==( const BadHash& other ) const { return value == other.value; }
	T value;
};
TEST_CASE( "map handles collisions", "[ae::Map" AE_HASH_N "]" )
{
	ae::Map< BadHash< aeHashUInt >, char, 5, aeHashN > map;
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

TEST_CASE( "dynamic hash map elements can be set and retrieved", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 0, aeHashN > map = TAG_TEST;
	for( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map.Set( 1000 + i, i ) );
	}
	REQUIRE( map.Length() == 100 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map.Get( -1 ) == -1 );
		for( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map.Get( 1000 + i ) == i );
		}
		REQUIRE( map.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after reserve", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 0, aeHashN > map = TAG_TEST;
	for( uint32_t i = 0; i < 100; i++ )
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
		for( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map.Get( 1000 + i ) == i );
		}
		REQUIRE( map.Get( -1 ) == -1 );
	}
}

TEST_CASE( "copy construct static hash map", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap<  uint32_t, 128, aeHashN > map0;
	for( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	map0.Reserve( 100 );
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() == 128 );
	
	ae::HashMap< uint32_t, 128, aeHashN > map1 = map0;
	REQUIRE( map1.Length() == 100 );
	REQUIRE( map1.Size() == 128 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
		for( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "copy assign static hash map", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 128, aeHashN > map0;
	for( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	map0.Reserve( 100 );
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() == 128 );
	
	ae::HashMap< uint32_t, 128, aeHashN > map1;
	REQUIRE( map1.Length() == 0 );
	REQUIRE( map1.Size() == 128 );
	map1 = map0;
	REQUIRE( map1.Length() == 100 );
	REQUIRE( map1.Size() == 128 );

	SECTION( "can retrieve previously set values" )
	{
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
		for( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after assignment of same size map", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 0, aeHashN > map0 = TAG_TEST;
	for( uint32_t i = 0; i < 100; i++ )
	{
		REQUIRE( map0.Set( 1000 + i, i ) );
	}
	REQUIRE( map0.Length() == 100 );
	REQUIRE( map0.Size() >= 100 );
	
	ae::HashMap< uint32_t, 0, aeHashN > map1 = TAG_TEST;
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
		for( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after assignment of smaller map", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 0, aeHashN > map0 = TAG_TEST;
	ae::HashMap< uint32_t, 0, aeHashN > map1 = TAG_TEST;
	for( uint32_t i = 0; i < 100; i++ )
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
		for( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "can access elements after assignment of bigger map", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< uint32_t, 0, aeHashN > map0 = TAG_TEST;
	ae::HashMap< uint32_t, 0, aeHashN > map1 = TAG_TEST;
	for( uint32_t i = 0; i < 100; i++ )
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
		for( uint32_t i = 0; i < 100; i++ )
		{
			REQUIRE( map0.Get( 1000 + i ) == i );
			REQUIRE( map1.Get( 1000 + i ) == i );
		}
		REQUIRE( map0.Get( -1 ) == -1 );
		REQUIRE( map1.Get( -1 ) == -1 );
	}
}

TEST_CASE( "fast stress test", "[ae::Map" AE_HASH_N "]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 0, aeHashN > map = TAG_TEST;
	for( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
	}
	REQUIRE( map.Length() == count );
	for( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "fast stress test 2", "[ae::Map" AE_HASH_N "]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 0, aeHashN > map = TAG_TEST;
	auto validateFn = [&]()
	{
		for( auto& pair : map )
		{
			uint32_t* v = map.TryGet( pair.key );
			AE_ASSERT( v );
			AE_ASSERT( *v == pair.value );
		}
	};

	for( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
		validateFn();
	}
	REQUIRE( map.Length() == count );

	uint64_t r = 543534;
	for( uint32_t i = 0; i < 100; i++ )
	{
		uint32_t removeCount = ae::Random( 0, map.Length(), &r );
		for( uint32_t j = 0; j < removeCount; j++ )
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
		for( uint32_t j = 0; j < removeCount; j++ )
		{
			uint32_t idx = ae::Random( 0, map.Length(), &r );
			uint32_t value = map.GetValue( idx );
			uint32_t valueCheck = 0;
			map.RemoveIndex( idx, &valueCheck );
			AE_ASSERT( value == valueCheck );
			validateFn();
		}

		uint32_t addCount = ae::Random( 0, map.Length(), &r );
		for( uint32_t j = 0; j < addCount; j++ )
		{
			map.Set( ae::Random( 0, 100000, &r ), 0 );
			validateFn();
		}
	}

	while( map.Length() )
	{
		uint32_t idx = ae::Random( 0, map.Length(), &r );
		uint32_t key = map.GetKey( idx );
		AE_ASSERT( map.Remove( key ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "fast full map stress test", "[ae::Map" AE_HASH_N "]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 10000, aeHashN > map;
	for( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
	}
	REQUIRE( map.Length() == count );
	for( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "stable stress test", "[ae::Map" AE_HASH_N "]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 0, aeHashN, ae::MapMode::Stable > map = TAG_TEST;
	auto validateFn = [&]()
	{
		for( uint32_t i = 1; i < map.Length(); i++ )
		{
			AE_ASSERT( map.GetValue( i - 1 ) < map.GetValue( i ) );
		}
	};

	for( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
	}
	validateFn();
	REQUIRE( map.Length() == count );
	for( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "stable stress test 2", "[ae::Map" AE_HASH_N "]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 0, aeHashN, ae::MapMode::Stable > map = TAG_TEST;
	auto validateFn = [&]()
	{
		for( auto& pair : map )
		{
			uint32_t* v = map.TryGet( pair.key );
			AE_ASSERT( v );
			AE_ASSERT( *v == pair.value );
		}
		for( uint32_t i = 1; i < map.Length(); i++ )
		{
			AE_ASSERT( map.GetValue( i - 1 ) < map.GetValue( i ) );
		}
	};

	for( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
		validateFn();
	}
	REQUIRE( map.Length() == count );

	uint64_t r = 543534;
	for( uint32_t i = 0; i < 100; i++ )
	{
		uint32_t removeCount = ae::Random( 0, map.Length(), &r );
		for( uint32_t j = 0; j < removeCount; j++ )
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
		for( uint32_t j = 0; j < removeCount; j++ )
		{
			uint32_t idx = ae::Random( 0, map.Length(), &r );
			uint32_t value = map.GetValue( idx );
			uint32_t valueCheck = 0;
			map.RemoveIndex( idx, &valueCheck );
			AE_ASSERT( value == valueCheck );
			validateFn();
		}

		uint32_t addCount = ae::Random( 0, map.Length(), &r );
		for( uint32_t j = 0; j < addCount; j++ )
		{
			for(uint32_t k = 0; k < map.Length(); k++ )
			{
				map.GetValue( k ) = k;
			}
			uint32_t key = ae::Random( 0, 100000, &r );
			uint32_t val = map.Get( key, map.Length() );
			map.Set( key, val );
			validateFn();
		}
	}

	while( map.Length() )
	{
		uint32_t idx = ae::Random( 0, map.Length(), &r );
		uint32_t key = map.GetKey( idx );
		AE_ASSERT( map.Remove( key ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "stable full map stress test", "[ae::Map" AE_HASH_N "]" )
{
	const uint32_t count = 10000;
	ae::Map< uint32_t, uint32_t, 10000, aeHashN, ae::MapMode::Stable > map;
	auto validateFn = [&]()
	{
		for( uint32_t i = 1; i < map.Length(); i++ )
		{
			AE_ASSERT( map.GetValue( i - 1 ) < map.GetValue( i ) );
		}
	};

	for( uint32_t i = 0; i < count; i++ )
	{
		map.Set( ( i * 1669 ) % count, i );
		validateFn();
	}
	REQUIRE( map.Length() == count );

	for( uint32_t i = 0; i < count; i++ )
	{
		AE_ASSERT( map.Remove( ( i * 5437 ) % count ) );
		validateFn();
	}
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "Map StableInsertIndex", "[ae::Map" AE_HASH_N "][stable]" )
{
	ae::Map< std::string, int, 0, aeHashN, ae::MapMode::Stable > map = TAG_TEST;
	
	// Add initial elements
	map.Set( "alpha", 1 );
	map.Set( "gamma", 3 );
	map.Set( "epsilon", 5 );
	
	REQUIRE( map.Length() == 3 );
	REQUIRE( map.GetKey( 0 ) == "alpha" );
	REQUIRE( map.GetKey( 1 ) == "gamma" );
	REQUIRE( map.GetKey( 2 ) == "epsilon" );
	
	// Insert at beginning
	map.Set( "aardvark", 0, 0 );
	REQUIRE( map.Length() == 4 );
	REQUIRE( map.GetKey( 0 ) == "aardvark" );
	REQUIRE( map.GetKey( 1 ) == "alpha" );
	REQUIRE( map.GetKey( 2 ) == "gamma" );
	REQUIRE( map.GetKey( 3 ) == "epsilon" );
	
	// Insert in middle
	map.Set( "beta", 2, 2 );
	REQUIRE( map.Length() == 5 );
	REQUIRE( map.GetKey( 0 ) == "aardvark" );
	REQUIRE( map.GetKey( 1 ) == "alpha" );
	REQUIRE( map.GetKey( 2 ) == "beta" );
	REQUIRE( map.GetKey( 3 ) == "gamma" );
	REQUIRE( map.GetKey( 4 ) == "epsilon" );
	
	// Insert at end (should be same as default append)
	map.Set( "zeta", 6, 5 );
	REQUIRE( map.Length() == 6 );
	REQUIRE( map.GetKey( 5 ) == "zeta" );
	
	// Verify all values are correct
	REQUIRE( map.Get( "aardvark" ) == 0 );
	REQUIRE( map.Get( "alpha" ) == 1 );
	REQUIRE( map.Get( "beta" ) == 2 );
	REQUIRE( map.Get( "gamma" ) == 3 );
	REQUIRE( map.Get( "epsilon" ) == 5 );
	REQUIRE( map.Get( "zeta" ) == 6 );
	
	// Test updating existing key (should not change order)
	map.Set( "gamma", 30 );
	REQUIRE( map.GetKey( 3 ) == "gamma" );
	REQUIRE( map.Get( "gamma" ) == 30 );
}

//------------------------------------------------------------------------------
// char* pointer key tests
//------------------------------------------------------------------------------
// HashMap::m_IsEqual() (aether.h:10821) handles char* keys specially using
// strcmp instead of pointer comparison. These tests verify content-based
// comparison works correctly with different pointers to same string content.
//------------------------------------------------------------------------------
TEST_CASE( "hash map works with char* keys (Set/Get)", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< const char*, 10, aeHashN > map;
	const char* str1 = "hello";
	const char* str2 = "world";
	char str3[] = "hello"; // Different pointer, same content as str1
	
	REQUIRE( map.Set( str1, 0 ) );
	REQUIRE( map.Set( str2, 1 ) );
	REQUIRE( map.Length() == 2 );
	
	// Should find str3 because m_IsEqual uses strcmp, not pointer comparison
	REQUIRE( map.Get( str3 ) == 0 );
	REQUIRE( map.Get( str1 ) == 0 );
	REQUIRE( map.Get( str2 ) == 1 );
}

TEST_CASE( "hash map works with char* keys (Remove)", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< const char*, 10, aeHashN > map;
	const char* str1 = "test";
	char str2[] = "test"; // Different pointer, same content
	
	REQUIRE( map.Set( str1, 0 ) );
	REQUIRE( map.Length() == 1 );
	
	// Should remove successfully because m_IsEqual uses strcmp
	REQUIRE( map.Remove( str2 ) == 0 );
	REQUIRE( map.Length() == 0 );
	REQUIRE( map.Get( str1 ) == -1 );
}

TEST_CASE( "hash map works with const char* keys", "[ae::HashMap" AE_HASH_N "]" )
{
	ae::HashMap< const char*, 10, aeHashN > map;
	const char* str1 = "alpha";
	const char* str2 = "beta";
	char str3[] = "alpha"; // Different pointer, same content as str1
	
	REQUIRE( map.Set( str1, 0 ) );
	REQUIRE( map.Set( str2, 1 ) );
	REQUIRE( map.Length() == 2 );
	
	// Should find str3 via content comparison
	REQUIRE( map.Get( str3 ) == 0 );
	REQUIRE( map.Get( str1 ) == 0 );
	REQUIRE( map.Get( str2 ) == 1 );
}

TEST_CASE( "map works with char* keys (Set/Get)", "[aeMap" AE_HASH_N "]" )
{
	ae::Map< const char*, int, 10, aeHashN > map;
	const char* key1 = "alpha";
	char key2[] = "alpha"; // Different pointer, same content
	
	map.Set( key1, 100 );
	REQUIRE( map.Length() == 1 );
	
	// Should find via content comparison (HashMap::m_IsEqual uses strcmp)
	REQUIRE( map.Get( key2 ) == 100 );
	REQUIRE( map.Get( key1 ) == 100 );
}

TEST_CASE( "map works with char* keys (Remove)", "[aeMap" AE_HASH_N "]" )
{
	ae::Map< const char*, int, 10, aeHashN > map;
	const char* key1 = "beta";
	char key2[] = "beta"; // Different pointer, same content
	
	map.Set( key1, 200 );
	REQUIRE( map.Length() == 1 );
	
	int value = 0;
	// Should remove successfully via content comparison
	REQUIRE( map.Remove( key2, &value ) );
	REQUIRE( value == 200 );
	REQUIRE( map.Length() == 0 );
}

TEST_CASE( "HashMap Increment Decrement", "[ae::HashMap" AE_HASH_N "][increment][decrement]" )
{
	ae::HashMap< std::string, 0, aeHashN > hashMap = TAG_TEST;
	
	// Add initial entries
	hashMap.Set( "first", 0 );
	hashMap.Set( "second", 1 );
	hashMap.Set( "third", 2 );
	hashMap.Set( "fourth", 3 );
	hashMap.Set( "fifth", 4 );
	
	REQUIRE( hashMap.Length() == 5 );
	REQUIRE( hashMap.Get( "first" ) == 0 );
	REQUIRE( hashMap.Get( "second" ) == 1 );
	REQUIRE( hashMap.Get( "third" ) == 2 );
	REQUIRE( hashMap.Get( "fourth" ) == 3 );
	REQUIRE( hashMap.Get( "fifth" ) == 4 );
	
	// Test increment from index 2 (should affect "third", "fourth", "fifth")
	hashMap.Increment( 2 );
	REQUIRE( hashMap.Get( "first" ) == 0 );  // unchanged
	REQUIRE( hashMap.Get( "second" ) == 1 ); // unchanged  
	REQUIRE( hashMap.Get( "third" ) == 3 );  // 2 -> 3
	REQUIRE( hashMap.Get( "fourth" ) == 4 ); // 3 -> 4
	REQUIRE( hashMap.Get( "fifth" ) == 5 );  // 4 -> 5
	
	// Test decrement from index 3 (should affect "fourth", "fifth")
	hashMap.Decrement( 3 );
	REQUIRE( hashMap.Get( "first" ) == 0 );  // unchanged
	REQUIRE( hashMap.Get( "second" ) == 1 ); // unchanged
	REQUIRE( hashMap.Get( "third" ) == 3 );  // unchanged (not > 3)
	REQUIRE( hashMap.Get( "fourth" ) == 3 ); // 4 -> 3
	REQUIRE( hashMap.Get( "fifth" ) == 4 );  // 5 -> 4
	
	// Test increment from index 0 (should affect all)
	hashMap.Increment( 0 );
	REQUIRE( hashMap.Get( "first" ) == 1 );  // 0 -> 1
	REQUIRE( hashMap.Get( "second" ) == 2 ); // 1 -> 2
	REQUIRE( hashMap.Get( "third" ) == 4 );  // 3 -> 4
	REQUIRE( hashMap.Get( "fourth" ) == 4 ); // 3 -> 4
	REQUIRE( hashMap.Get( "fifth" ) == 5 );  // 4 -> 5
}
