//------------------------------------------------------------------------------
// ArrayTest.cpp
// Copyright (c) John Hughes on 12/2/19. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "TestUtils.h"
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// ae::Array tests
//------------------------------------------------------------------------------
const ae::Tag TAG_TEST = "test";

TEST_CASE( "arrays elements can be appended and queried", "[ae::Array]" )
{
	ae::Array< int > array = TAG_TEST;
	for ( uint32_t i = 0; i < 16; i++ )
	{
		REQUIRE( array.Append( 100 + i ) == 100 + i );
		REQUIRE( array.Length() == i + 1 );
		REQUIRE( array.Size() == 16 );
	}
	REQUIRE( array.Length() == 16 );

	SECTION( "elements can be queried" )
	{
		for ( uint32_t i = 0; i < 16; i++ )
		{
			REQUIRE( array[ i ] == 100 + i );
		}
	}

	SECTION( "array can be resized and elements can be queried" )
	{
		for ( uint32_t i = array.Length(); i < 19; i++ )
		{
			REQUIRE( array.Append( 100 + i ) == 100 + i );
			REQUIRE( array.Length() == i + 1 );
			REQUIRE( array.Size() == 32 );
		}
		REQUIRE( array.Length() == 19 );

		for ( uint32_t i = 0; i < 19; i++ )
		{
			REQUIRE( array[ i ] == 100 + i );
		}
	}
}

TEST_CASE( "array will assert if too many elements are appended" )
{
	ae::Array< int, 3 > array;
	REQUIRE_NOTHROW( array.Append( 1 ) );
	REQUIRE_NOTHROW( array.Append( 2 ) );
	REQUIRE_NOTHROW( array.Append( 3 ) );
	REQUIRE_THROWS( array.Append( 4 ) );
}

TEST_CASE( "multiple arrays elements can be appended and queried", "[ae::Array]" )
{
	ae::Array< int > array = TAG_TEST;
	for ( uint32_t i = 0; i < 5; i++ )
	{
		REQUIRE( array.Append( 100 + i ) == 100 + i );
		int* p = array.Append( 1000 + i, 2 );
		REQUIRE( p[ 0 ] == 1000 + i );
		REQUIRE( p[ 1 ] == 1000 + i );
		REQUIRE( p == array.Data() + array.Length() - 2 );
		REQUIRE( array.Length() == ( i + 1 ) * 3 );
		REQUIRE( array.Size() == 16 );
	}
	REQUIRE( array.Length() == 15 );

	SECTION( "elements can be queried" )
	{
		for ( uint32_t i = 0; i < 5; i++ )
		{
			REQUIRE( array[ i * 3 ] == 100 + i );
			REQUIRE( array[ i * 3 + 1 ] == 1000 + i );
			REQUIRE( array[ i * 3 + 2 ] == 1000 + i );
		}
	}

	SECTION( "array can be resized and elements can be queried" )
	{
		for ( uint32_t i = array.Length() / 3; i < 8; i++ )
		{
			REQUIRE( array.Append( 100 + i ) == 100 + i );
			int* p = array.Append( 1000 + i, 2 );
			REQUIRE( p[ 0 ] == 1000 + i );
			REQUIRE( p[ 1 ] == 1000 + i );
			REQUIRE( p == array.Data() + array.Length() - 2 );
			REQUIRE( array.Length() == ( i + 1 ) * 3 );
			REQUIRE( array.Size() == 32 );
		}
		REQUIRE( array.Length() == 24 );

		for ( uint32_t i = 0; i < 8; i++ )
		{
			REQUIRE( array[ i * 3 ] == 100 + i );
			REQUIRE( array[ i * 3 + 1 ] == 1000 + i );
			REQUIRE( array[ i * 3 + 2 ] == 1000 + i );
		}
	}
}

TEST_CASE( "arrays can be constructed from initializer lists", "[ae::Array]" )
{
	ae::Array< int, 8 > array = { 100, 101, 102, 103, 104, 105, 106, 107 };
	for ( uint32_t i = 0; i < 8; i++ )
	{
		REQUIRE( array[ i ] == 100 + i );
	}
}

TEST_CASE( "arrays can be appended to other arrays", "[ae::Array]" )
{
	int first[] = { 100, 101, 102, 103, 104, 105, 106, 107 };
	int second[] = { 108, 109, 110, 111, 112, 113, 114, 115 };
	ae::Array< int > array = TAG_TEST;

	array.AppendArray( first, countof(first) );
	for ( uint32_t i = 0; i < 8; i++ )
	{
		REQUIRE( array[ i ] == 100 + i );
	}

	array.AppendArray( second, countof(second) );
	for ( uint32_t i = 0; i < 16; i++ )
	{
		REQUIRE( array[ i ] == 100 + i );
	}
}

TEST_CASE( "initial size of dynamic array is exact when specified", "[ae::Array]" ) // @NOTE: This isn't 100% true, there's a minimum number of bytes allocated for the array
{
	ae::Array< int > array( TAG_TEST, 111 );
	for ( uint32_t i = 0; i < 111; i++ )
	{
		array.Append( 100 + i );
		REQUIRE( array.Length() == i + 1 );
		REQUIRE( array.Size() == 111 );
	}
	REQUIRE( array.Length() == 111 );

	SECTION( "elements can be queried" )
	{
		for ( uint32_t i = 0; i < 111; i++ )
		{
			REQUIRE( array[ i ] == 100 + i );
		}
	}
}

// @TODO: Add specific Find() FindFn() FindLast() and FindLastFn() tests

TEST_CASE( "arrays elements can be removed by value", "[ae::Array]" )
{
	ae::Array< int > array = TAG_TEST;
	for ( uint32_t i = 0; i < 10; i++ )
	{
		array.Append( i );
		array.Append( 0 );
	}
	REQUIRE( array.Length() == 20 );
	//"<0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0>"

	SECTION( "removing all values results in empty search" )
	{
		array.RemoveAll( 0 );
		REQUIRE( array[ 0 ] == 1 );
		REQUIRE( array.Length() == 9 );
		REQUIRE( array.Find( 0 ) == -1 );
	}
	
	SECTION( "removing a value results in remaining values shifting" )
	{
		REQUIRE( array[ 6 ] == 3 );
		array.RemoveAll( 3 );
		REQUIRE( array.Find( 3 ) == -1 );
		REQUIRE( array[ 6 ] == 0 );
		REQUIRE( array[ 7 ] == 4 );
		REQUIRE( array.Length() == 19 );
	}

	SECTION( "removing a value based on function results in empty search" )
	{
		array.RemoveAllFn( []( int a ){ return a >= 6; } );
		REQUIRE( array.Find( 5 ) == 10 );
		REQUIRE( array.Find( 6 ) == -1 );
		REQUIRE( array.Find( 7 ) == -1 );
		REQUIRE( array.Find( 8 ) == -1 );
		REQUIRE( array.Find( 9 ) == -1 );
		REQUIRE( array.Length() == 16 );
	}
}

TEST_CASE( "arrays can be sized and resized", "[ae::Array]" )
{
	ae::Array< int > a( TAG_TEST, 5 );

	REQUIRE( a.Length() == 0 );
	REQUIRE( a.Size() == 16 );

	SECTION( "reserving bigger changes size but not length" )
	{
		a.Reserve( 10 );

		REQUIRE( a.Length() == 0 );
		REQUIRE( a.Size() == 16 );
	}
	SECTION( "reserving smaller does not change length or size" )
	{
		a.Reserve( 0 );

		REQUIRE( a.Length() == 0 );
		REQUIRE( a.Size() == 16 );
	}
	SECTION( "clearing reduces length but does not affect size" )
	{
		a.Clear();

		REQUIRE( a.Length() == 0 );
		REQUIRE( a.Size() == 16 );
	}
}

TEST_CASE( "arrays can be constructed with a specified length", "[ae::Array]" )
{
	ae::Array< int > a( TAG_TEST, 7, 5 );

	REQUIRE( a.Length() == 5 );
	REQUIRE( a.Size() == 16 );
}

TEST_CASE( "0 array elements can be inserted by index", "[ae::Array]" )
{
	ae::Array< int > empty = TAG_TEST;
	REQUIRE( empty.Length() == 0 );

	ae::Array< int > array = TAG_TEST;
	for ( uint32_t i = 1; i <= 5; i++ )
	{
		array.Append( i );
	}
	REQUIRE( array.Length() == 5 );

	SECTION( "inserting no elements at beginning does not move elements back" )
	{
		REQUIRE( array.Insert( 0, 7, 0 ) == array.Data() );
		REQUIRE( array.Length() == 5 );
		REQUIRE( array[ 0 ] == 1 );
		REQUIRE( array[ 1 ] == 2 );
		REQUIRE( array[ 2 ] == 3 );
		REQUIRE( array[ 3 ] == 4 );
		REQUIRE( array[ 4 ] == 5 );
	}

	SECTION( "insert no elements in middle does not move later elements back" )
	{
		REQUIRE( array.Insert( 3, 7, 0 ) == &array[ 3 ] );
		REQUIRE( array.Length() == 5 );
		REQUIRE( array[ 0 ] == 1 );
		REQUIRE( array[ 1 ] == 2 );
		REQUIRE( array[ 2 ] == 3 );
		REQUIRE( array[ 3 ] == 4 );
		REQUIRE( array[ 4 ] == 5 );
	}

	SECTION( "insert no elements at end does not append value" )
	{
		REQUIRE( array.Insert( 5, 7, 0 ) == array.end() );
		REQUIRE( array.Length() == 5 );
		REQUIRE( array[ 0 ] == 1 );
		REQUIRE( array[ 1 ] == 2 );
		REQUIRE( array[ 2 ] == 3 );
		REQUIRE( array[ 3 ] == 4 );
		REQUIRE( array[ 4 ] == 5 );
	}

	SECTION( "insert no elements into empty array does not change the size" )
	{
		REQUIRE( empty.Insert( 0, 7, 0 ) == nullptr );
		REQUIRE( empty.Length() == 0 );
	}
}

TEST_CASE( "array elements can be inserted by index", "[ae::Array]" )
{
	ae::Array< int > empty = TAG_TEST;
	REQUIRE( empty.Length() == 0 );

	ae::Array< int > array = TAG_TEST;
	for ( uint32_t i = 1; i <= 5; i++ )
	{
		array.Append( i );
	}
	REQUIRE( array.Length() == 5 );

	SECTION( "insert at beginning moves all elements back" )
	{
		REQUIRE( array.Insert( 0, 7 ) == 7 );
		REQUIRE( array.Length() == 6 );
		REQUIRE( array[ 0 ] == 7 );
		REQUIRE( array[ 1 ] == 1 );
		REQUIRE( array[ 2 ] == 2 );
		REQUIRE( array[ 3 ] == 3 );
		REQUIRE( array[ 4 ] == 4 );
		REQUIRE( array[ 5 ] == 5 );
	}

	SECTION( "insert in middle moves later elements back" )
	{
		REQUIRE( array.Insert( 3, 7 ) == 7 );
		REQUIRE( array.Length() == 6 );
		REQUIRE( array[ 0 ] == 1 );
		REQUIRE( array[ 1 ] == 2 );
		REQUIRE( array[ 2 ] == 3 );
		REQUIRE( array[ 3 ] == 7 );
		REQUIRE( array[ 4 ] == 4 );
		REQUIRE( array[ 5 ] == 5 );
	}

	SECTION( "insert at end appends value" )
	{
		REQUIRE( array.Insert( 5, 7 ) == 7 );
		REQUIRE( array.Length() == 6 );
		REQUIRE( array[ 0 ] == 1 );
		REQUIRE( array[ 1 ] == 2 );
		REQUIRE( array[ 2 ] == 3 );
		REQUIRE( array[ 3 ] == 4 );
		REQUIRE( array[ 4 ] == 5 );
		REQUIRE( array[ 5 ] == 7 );
	}

	SECTION( "insert into empty array" )
	{
		REQUIRE( empty.Insert( 0, 7 ) == 7 );
		REQUIRE( empty.Length() == 1 );
		REQUIRE( empty[ 0 ] == 7 );
	}
}

TEST_CASE( "multiple array elements can be inserted by index", "[ae::Array]" )
{
	ae::Array< int > array = TAG_TEST;
	for ( uint32_t i = 0; i < 128; i++ )
	{
		array.Append( 100 + i );
	}
	for ( uint32_t i = 0; i < 128; i++ )
	{
		REQUIRE( array[ i ] == 100 + i );
	}
	for ( uint32_t i = 0; i < 128; i++ )
	{
		int val = 1000 + i;
		int* p = array.Insert( i * 4 + 1, val, 3 );
		REQUIRE( p[ 0 ] == val );
		REQUIRE( p[ 1 ] == val );
		REQUIRE( p[ 2 ] == val );
	}
	for ( uint32_t i = 0; i < 128; i++ )
	{
		REQUIRE( array[ i * 4 ] == 100 + i );
		REQUIRE( array[ i * 4 + 1 ] == 1000 + i );
		REQUIRE( array[ i * 4 + 2 ] == 1000 + i );
		REQUIRE( array[ i * 4 + 3 ] == 1000 + i );
	}
}

TEST_CASE( "arrays can be inserted into other arrays by index", "[ae::Array]" )
{
	ae::Array< uint32_t > array = TAG_TEST;
	for ( uint32_t i = 0; i < 128; i++ )
	{
		array.Append( 100 + i );
	}
	for ( uint32_t i = 0; i < 128; i++ )
	{
		REQUIRE( array[ i ] == 100 + i );
	}
	for ( uint32_t i = 0; i < 128; i++ )
	{
		uint32_t vals[] = { 200 + i, 300 + i, 400 + i };
		uint32_t* p = array.InsertArray( i * 4 + 1, vals, countof(vals) );
		REQUIRE( p[ 0 ] == vals[ 0 ] );
		REQUIRE( p[ 1 ] == vals[ 1 ] );
		REQUIRE( p[ 2 ] == vals[ 2 ] );
	}
	for ( uint32_t i = 0; i < 128; i++ )
	{
		REQUIRE( array[ i * 4 ] == 100 + i );
		REQUIRE( array[ i * 4 + 1 ] == 200 + i );
		REQUIRE( array[ i * 4 + 2 ] == 300 + i );
		REQUIRE( array[ i * 4 + 3 ] == 400 + i );
	}
}

// default ctor
TEST_CASE( "ctor/dtor is not called on default constructed arrays", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	{
		ae::Array< ae::LifetimeTester > array = TAG_TEST; // +0 ctor/dtor
		REQUIRE( array.Length() == 0 );
		REQUIRE( array.Size() == 0 );
	}
	
	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

// reserve
TEST_CASE( "array reserve does not call ctor/dtor on (reserved) zero length arrays", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	{
		ae::Array< ae::LifetimeTester > array( TAG_TEST, 10 ); // +0 ctor/dtor
		REQUIRE( array.Length() == 0 );
		REQUIRE( array.Size() >= 10 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );
	}
	
	{
		ae::Array< ae::LifetimeTester > array = TAG_TEST;
		REQUIRE( array.Length() == 0 );
		REQUIRE( array.Size() == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );
		
		array.Reserve( 10 ); // +0 ctor/dtor
		REQUIRE( array.Length() == 0 );
		REQUIRE( array.Size() >= 10 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );
	}
	
	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

// construct with values
TEST_CASE( "arrays only construct/destruct objects on non-zero length arrays", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	ae::Array< ae::LifetimeTester > array( TAG_TEST, ae::LifetimeTester(), 3 ); // +1 ctor, +1 dtor, +3 copy
	REQUIRE( array.Length() == 3 );
	REQUIRE( array.Size() >= 3 );
	
	REQUIRE( ae::LifetimeTester::ctorCount == 1 );
	REQUIRE( ae::LifetimeTester::copyCount == 3 );
	REQUIRE( ae::LifetimeTester::dtorCount == 1 );
	REQUIRE( ae::LifetimeTester::currentCount == 3 );
	
	SECTION( "clear array results in all objects being destroyed" )
	{
		array.Clear(); // +3 dtor
		REQUIRE( ae::LifetimeTester::dtorCount == 4 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );
	}
	
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
}

TEST_CASE( "copy construct", "[ae::Array]" )
{
	SECTION( "no ctor/dtor calls for default copy constructed zero length arrays" )
	{
		ae::LifetimeTester::ClearStats();
		
		{
			ae::Array< ae::LifetimeTester > array0 = TAG_TEST; // +0 ctor/dtor
			REQUIRE( array0.Length() == 0 );
			REQUIRE( array0.Size() == 0 );
			REQUIRE( ae::LifetimeTester::ctorCount == 0 );
			REQUIRE( ae::LifetimeTester::currentCount == 0 );
			
			ae::Array< ae::LifetimeTester > array1( array0 ); // +0 ctor/dtor
			REQUIRE( ae::LifetimeTester::ctorCount == 0 );
			REQUIRE( ae::LifetimeTester::currentCount == 0 );
		}
		
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );
	}
	
	SECTION( "copy construct from non-zero length array calls copy constructors" )
	{
		ae::LifetimeTester::ClearStats();
		{
			ae::Array< ae::LifetimeTester > array0( TAG_TEST, ae::LifetimeTester(), 5 ); // +1 ctor, +1 dtor, +5 copy
			REQUIRE( array0.Length() == 5 );
			REQUIRE( array0.Size() >= 6 );
			REQUIRE( ae::LifetimeTester::ctorCount == 1 );
			REQUIRE( ae::LifetimeTester::copyCount == 5 );
			REQUIRE( ae::LifetimeTester::dtorCount == 1 );
			REQUIRE( ae::LifetimeTester::currentCount == 5 );
			
			{
				ae::Array< ae::LifetimeTester > array1( array0 ); // +5 copy
				REQUIRE( ae::LifetimeTester::copyCount == 10 );
				REQUIRE( ae::LifetimeTester::currentCount == 10 );
			} // +5 dtor
		} // +5 dtor
		REQUIRE( ae::LifetimeTester::dtorCount == 11 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );
		
		REQUIRE( ae::LifetimeTester::ctorCount == 1 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	}
}

TEST_CASE( "assignment operator", "[ae::Array]" )
{
	SECTION( "assignment to zero length array by non-zero length array calls copy constructors" )
	{
		ae::LifetimeTester::ClearStats();
		{
			ae::Array< ae::LifetimeTester > array0( TAG_TEST, ae::LifetimeTester(), 5 ); // +1 ctor, +1 dtor, +5 copy
			REQUIRE( array0.Length() == 5 );
			REQUIRE( array0.Size() >= 6 );
			REQUIRE( ae::LifetimeTester::ctorCount == 1 );
			REQUIRE( ae::LifetimeTester::copyCount == 5 );
			REQUIRE( ae::LifetimeTester::dtorCount == 1 );
			REQUIRE( ae::LifetimeTester::currentCount == 5 );
			
			{
				ae::Array< ae::LifetimeTester > array1 = TAG_TEST; // +0 ctor/dtor
				REQUIRE( ae::LifetimeTester::ctorCount == 1 );
				REQUIRE( ae::LifetimeTester::copyCount == 5 );
				REQUIRE( ae::LifetimeTester::dtorCount == 1 );
				REQUIRE( ae::LifetimeTester::currentCount == 5 );

				array1 = array0; // +5 copy, +5 dtor
				REQUIRE( ae::LifetimeTester::copyCount == 10 );
				REQUIRE( ae::LifetimeTester::currentCount == 10 );
			} // +5 dtor
			REQUIRE( ae::LifetimeTester::dtorCount == 6 );
		} // +5 dtor
		REQUIRE( ae::LifetimeTester::dtorCount == 11 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );

		REQUIRE( ae::LifetimeTester::ctorCount == 1 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	}
	
	SECTION( "assignment to non-zero length array by other non-zero length array calls copy constructors" )
	{
		ae::LifetimeTester::ClearStats();
		{
			ae::Array< ae::LifetimeTester > array5( TAG_TEST, ae::LifetimeTester(), 5 ); // +1 ctor, +1 dtor, +5 copy
			REQUIRE( array5.Length() == 5 );
			REQUIRE( array5.Size() >= 5 );
			REQUIRE( ae::LifetimeTester::ctorCount == 1 );
			REQUIRE( ae::LifetimeTester::copyCount == 5 );
			REQUIRE( ae::LifetimeTester::dtorCount == 1 );
			REQUIRE( ae::LifetimeTester::currentCount == 5 );
			
			{
				ae::Array< ae::LifetimeTester > array20( TAG_TEST, ae::LifetimeTester(), 20 ); // +1 ctor, +1 dtor, +20 copy
				REQUIRE( ae::LifetimeTester::ctorCount == 2 );
				REQUIRE( ae::LifetimeTester::copyCount == 25 );
				REQUIRE( ae::LifetimeTester::dtorCount == 2 );
				REQUIRE( ae::LifetimeTester::currentCount == 25 );

				array20 = array5; // +5 copy, +20 dtor
				REQUIRE( ae::LifetimeTester::copyCount == 30 );
				REQUIRE( ae::LifetimeTester::dtorCount == 22 );
				REQUIRE( ae::LifetimeTester::currentCount == 10 );
			} // +5 dtor
			REQUIRE( ae::LifetimeTester::dtorCount == 27 );
		} // +5 dtor
		REQUIRE( ae::LifetimeTester::dtorCount == 32 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );

		REQUIRE( ae::LifetimeTester::ctorCount == 2 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	}
}

TEST_CASE( "append to empty array", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	{
		ae::Array< ae::LifetimeTester > array0 = TAG_TEST; // +0 ctor/dtor
		REQUIRE( array0.Length() == 0 );
		REQUIRE( array0.Size() == 0 );
		REQUIRE( ae::LifetimeTester::ctorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == 0 );
		
		array0.Append( ae::LifetimeTester() ); // +1 ctor, +1 dtor, +1 copy
		REQUIRE( array0.Length() == 1 );
		REQUIRE( array0.Size() >= 1 );
		REQUIRE( ae::LifetimeTester::ctorCount == 1 );
		REQUIRE( ae::LifetimeTester::copyCount == 1 );
		REQUIRE( ae::LifetimeTester::dtorCount == 1 );
		REQUIRE( ae::LifetimeTester::currentCount == 1 );
		
		array0.Append( ae::LifetimeTester() ); // +1 ctor, +1 dtor, +1 copy
		REQUIRE( array0.Length() == 2 );
		REQUIRE( array0.Size() >= 2 );
		REQUIRE( ae::LifetimeTester::ctorCount == 2 );
		REQUIRE( ae::LifetimeTester::copyCount == 2 );
		REQUIRE( ae::LifetimeTester::dtorCount == 2 + ae::LifetimeTester::moveCount ); // dtor called after move on array resize
		REQUIRE( ae::LifetimeTester::currentCount == 2 );
	} // +2 dtor
	
	REQUIRE( ae::LifetimeTester::ctorCount == 2 );
	REQUIRE( ae::LifetimeTester::copyCount == 2 );
	REQUIRE( ae::LifetimeTester::dtorCount == 4 + ae::LifetimeTester::moveCount ); // dtor called after move on array resize
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

TEST_CASE( "append to non-empty array", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	uint32_t ctor = 0;
	uint32_t copy = 0;
	uint32_t dtor = 0;
	uint32_t current = 0;
	
	{
		ae::Array< ae::LifetimeTester > array5( TAG_TEST, ae::LifetimeTester(), 5 );
		ctor++;
		copy += 5;
		dtor++;
		current += 5;
		
		REQUIRE( array5.Length() == current );
		REQUIRE( array5.Size() >= current );
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		
		for ( uint32_t i = 0; i < 111; i++ )
		{
			array5.Append( ae::LifetimeTester() );
			ctor++;
			copy++;
			dtor++;
			current++;
			
			REQUIRE( array5.Length() == current );
			REQUIRE( array5.Size() >= current );
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount ); // dtor called after move on array resize
			REQUIRE( ae::LifetimeTester::currentCount == current );
		}
	}
	dtor += current;
	current = 0;
	
	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == current );
}

TEST_CASE( "append array to empty array", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	
	{
		ae::Array< ae::LifetimeTester > array = TAG_TEST;
		REQUIRE( array.Length() == current );
		REQUIRE( array.Size() >= (uint32_t)current );
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		
		for ( uint32_t i = 0; i < 25; i++ )
		{
			{
				ae::LifetimeTester* t = new ae::LifetimeTester[ i ];
				ctor += i;
				current += i;
				REQUIRE( ae::LifetimeTester::ctorCount == ctor );
				REQUIRE( ae::LifetimeTester::currentCount == current );
				
				array.AppendArray( t, i );
				copy += i;
				current += i;

				delete[] t;
			}
			dtor += i;
			current -= i;
			
			REQUIRE( array.Length() == current );
			REQUIRE( array.Size() >= (uint32_t)current );
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount ); // dtor called after move on array resize
			REQUIRE( ae::LifetimeTester::currentCount == current );
		}
	}
	dtor += current;
	current = 0;
	
	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == current );
}

TEST_CASE( "append array to array", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	
	{
		ae::Array< ae::LifetimeTester > array( TAG_TEST, ae::LifetimeTester(), 5 );
		ctor++;
		copy += 5;
		dtor++;
		current += 5;
		
		REQUIRE( array.Length() == current );
		REQUIRE( array.Size() >= (uint32_t)current );
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		
		for ( uint32_t i = 0; i < 25; i++ )
		{
			{
				ae::LifetimeTester* t = new ae::LifetimeTester[ i ];
				ctor += i;
				current += i;
				REQUIRE( ae::LifetimeTester::ctorCount == ctor );
				REQUIRE( ae::LifetimeTester::currentCount == current );
				
				array.AppendArray( t, i );
				copy += i;
				current += i;

				delete[] t;
			}
			dtor += i;
			current -= i;
			
			REQUIRE( array.Length() == current );
			REQUIRE( array.Size() >= (uint32_t)current );
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount ); // dtor called after move on array resize
			REQUIRE( ae::LifetimeTester::currentCount == current );
		}
	}
	dtor += current;
	current = 0;
	
	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == current );
}

// insert
TEST_CASE( "insert element at end of array", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	
	{
		ae::Array< ae::LifetimeTester > array = TAG_TEST;
		REQUIRE( array.Length() == current );
		REQUIRE( array.Size() >= (uint32_t)current );
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		
		for ( uint32_t i = 0; i < 123; i++ )
		{
			array.Insert( i, ae::LifetimeTester() );
			ctor++;
			copy++;
			dtor++;
			current++;
			REQUIRE( array.Length() == current );
			REQUIRE( array.Size() >= (uint32_t)current );
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount );
			REQUIRE( ae::LifetimeTester::currentCount == current );
		}
	}
	dtor += current;
	current = 0;
	
	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == current );
}

TEST_CASE( "insert element at beginning of array", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t move = 0;
	int32_t assign = 0;
	int32_t moveAssign = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	
	{
		ae::Array< ae::LifetimeTester > array = TAG_TEST;
		REQUIRE( array.Length() == current );
		REQUIRE( array.Size() >= (uint32_t)current );
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		
		array.Insert( 0, ae::LifetimeTester() );
		ctor++;
		copy++;
		dtor++;
		current++;
		REQUIRE( array.Length() == current );
		REQUIRE( array.Size() >= (uint32_t)current );
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor + ae::LifetimeTester::moveCount );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		
		for ( uint32_t i = 0; i < 123; i++ )
		{
			uint32_t prevSize = array.Size();
			array.Insert( 0, ae::LifetimeTester() );
			ctor++;
			move++;
			assign++;
			moveAssign += current - 1;
			dtor++;
			if ( prevSize != array.Size() )
			{
				move += current;
				dtor += current;
			}
			current++;
			
			REQUIRE( array.Length() == current );
			REQUIRE( array.Size() >= (uint32_t)current );
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::moveCount == move );
			REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
			REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor );
			REQUIRE( ae::LifetimeTester::currentCount == current );
		}
	}
	dtor += current;
	current = 0;
	
	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::moveCount == move );
	REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
	REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor );
	REQUIRE( ae::LifetimeTester::currentCount == current );
}

TEST_CASE( "remove all elements from array", "[ae::Array]" )
{
	ae::LifetimeTester::ClearStats();
	
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t moveAssign = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	
	ae::Array< ae::LifetimeTester > array( TAG_TEST, ae::LifetimeTester(), 5 );
	ctor++;
	copy += 5;
	dtor++;
	current += 5;
	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor );
	REQUIRE( ae::LifetimeTester::currentCount == current );
	
	while ( array.Length() )
	{
		array.Remove( 0 );
		current--;
		moveAssign += current;
		dtor++;
		
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
	}
	
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

TEST_CASE( "array elements can be removed by index", "[ae::Array]" )
{
	ae::Array< int > array = TAG_TEST;
	for ( uint32_t i = 0; i < 16; i++ )
	{
		array.Append( i );
	}
	REQUIRE( array.Length() == 16 );

	for ( uint32_t i = 0; i < 8; i++ )
	{
		array.Remove( i + 1 );
	}
	REQUIRE( array.Length() == 8 );
	for ( uint32_t i = 0; i < 8; i++ )
	{
		REQUIRE( array[ i ] == i * 2 );
	}
}

TEST_CASE( "multiple array elements can be removed by index", "[ae::Array]" )
{
	ae::Array< int > array = TAG_TEST;
	int values[] = { 1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5 };
	array.AppendArray( values, countof(values) );
	REQUIRE( array.Length() == 15 );
	REQUIRE( array.Find( 4 ) == 6 );

	array.Remove( 6, 4 );
	REQUIRE( array.Length() == 11 );
	REQUIRE( array.Find( 4 ) == -1 );
}

TEST_CASE( "arrays support range based loop", "[ae::Array]" )
{
	ae::Array< int > array = TAG_TEST;
	const ae::Array< int >& constArray = array;
	for ( uint32_t i = 0; i < 10; i++ )
	{
		array.Append( i );
		array.Append( 0 );
	}
	REQUIRE( array.Length() == 20 );
	int compare[] = { 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0 };

	SECTION( "can iterate over non-const array" )
	{
		int i = 0;
		for ( int& v : array )
		{
			REQUIRE( v == array[ i ] );
			REQUIRE( v == compare[ i ] );
			i++;
		}
	}
	
	SECTION( "can iterate over const array" )
	{
		int i = 0;
		for ( const int& v : constArray )
		{
			REQUIRE( v == constArray[ i ] );
			REQUIRE( v == compare[ i ] );
			i++;
		}
	}
}
