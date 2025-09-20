//------------------------------------------------------------------------------
// MapTest32.cpp
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
#define AE_HASH_N "32"
#define AE_GET_HASH GetHash32
using aeHashUInt = uint32_t;
using aeHashN = ae::Hash32;

//------------------------------------------------------------------------------
// ae::GetHash tests
//------------------------------------------------------------------------------
TEST_CASE( "GetHash() template function hashes values correctly", "[ae::HashMap32]" )
{
	REQUIRE( ae::GetHash32( 777u ) == 777u );
	REQUIRE( ae::GetHash32( 777 ) == 777u );
	REQUIRE( ae::GetHash32( -777 ) == (uint32_t)-777 );
	if( sizeof( void* ) == 8 )
	{
		REQUIRE( ae::GetHash32( (const void*)777u ) == 0xc68b223du );
		REQUIRE( ae::GetHash32( (const float*)777u ) == 0xc68b223du );
		REQUIRE( ae::GetHash32( (void*)777u ) == 0xc68b223du );
		REQUIRE( ae::GetHash32( (float*)777u ) == 0xc68b223du );
	}
	else if( sizeof( void* ) == 4 )
	{
		REQUIRE( ae::GetHash32( (const void*)777u ) == 0x397fc86d );
		REQUIRE( ae::GetHash32( (const float*)777u ) == 0x397fc86d );
		REQUIRE( ae::GetHash32( (void*)777u ) == 0x397fc86d );
		REQUIRE( ae::GetHash32( (float*)777u ) == 0x397fc86d );
	}
	REQUIRE( ae::GetHash32( (const char*)"777" ) == 0x8bbb3eecu );
	REQUIRE( ae::GetHash32( (char*)"777" ) == 0x8bbb3eecu );
	REQUIRE( ae::GetHash32( "777" ) == 0x8bbb3eecu );
	REQUIRE( ae::GetHash32( ae::Str< 32 >( "777" ) ) == 0x8bbb3eecu );
	REQUIRE( ae::GetHash32( ae::Str< 128 >( "777" ) ) == 0x8bbb3eecu );
	REQUIRE( ae::GetHash32( std::string( "777" ) ) == 0x8bbb3eecu );
	// REQUIRE( ae::GetHash32( ae::GetHash32().HashString( "777" ) ) == 0x8bbb3eecu );
	REQUIRE( ae::GetHash32( ae::Int3( 1, 2, 3 ) ) == 316 );
	REQUIRE( ae::GetHash32( ae::Int3( -1, 0, 1 ) ) == 19 );
	//REQUIRE( ae::Vec3( 0.0f ) ) == 0 ); // Should fail to link
}

#include "MapTest.h"

TEST_CASE( "HasMethodHash32", "[ae::GetHash32]" )
{
	REQUIRE( ae::_HasMethodHash32< BadHash< uint32_t > >::value );
}

