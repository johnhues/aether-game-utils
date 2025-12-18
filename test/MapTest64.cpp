//------------------------------------------------------------------------------
// MapTest64.cpp
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
#define AE_HASH_N "64"
#define AE_GET_HASH GetHash64
using aeHashUInt = uint64_t;
using aeHashN = ae::Hash64;

//------------------------------------------------------------------------------
// ae::GetHash tests
//------------------------------------------------------------------------------
TEST_CASE( "GetHash() template function hashes values correctly", "[ae::HashMap64]" )
{
	REQUIRE( ae::GetHash64( 777u ) == 777ull );
	REQUIRE( ae::GetHash64( 777 ) == 777ull );
	REQUIRE( ae::GetHash64( -777 ) == (uint64_t)-777 );
	if( sizeof( void* ) == 8 )
	{
		REQUIRE( ae::GetHash64( (const void*)777u ) == 0xd19adc106a20b93dull );
		REQUIRE( ae::GetHash64( (const float*)777u ) == 0xd19adc106a20b93dull );
		REQUIRE( ae::GetHash64( (void*)777u ) == 0xd19adc106a20b93dull );
		REQUIRE( ae::GetHash64( (float*)777u ) == 0xd19adc106a20b93dull );
	}
	else if( sizeof( void* ) == 4 )
	{
		REQUIRE( ae::GetHash64( (const void*)777u ) == 0xc6fc9bba07fe37ed );
		REQUIRE( ae::GetHash64( (const float*)777u ) == 0xc6fc9bba07fe37ed );
		REQUIRE( ae::GetHash64( (void*)777u ) == 0xc6fc9bba07fe37ed );
		REQUIRE( ae::GetHash64( (float*)777u ) == 0xc6fc9bba07fe37ed );
	}
	REQUIRE( ae::GetHash64( (const char*)"777" ) == 0x34de64180ef728acull );
	REQUIRE( ae::GetHash64( (char*)"777" ) == 0x34de64180ef728acull );
	REQUIRE( ae::GetHash64( "777" ) == 0x34de64180ef728acull );
	REQUIRE( ae::GetHash64( ae::Str< 32 >( "777" ) ) == 0x34de64180ef728acull );
	REQUIRE( ae::GetHash64( ae::Str< 128 >( "777" ) ) == 0x34de64180ef728acull );
	REQUIRE( ae::GetHash64( std::string( "777" ) ) == 0x34de64180ef728acull );
	// REQUIRE( ae::GetHash64( ae::GetHash64().HashString( "777" ) ) == 0x34de64180ef728acull );
	REQUIRE( ae::GetHash64( ae::Int3( 1, 2, 3 ) ) == 316 );
	REQUIRE( ae::GetHash64( ae::Int3( -1, 0, 1 ) ) == 19 );
	//REQUIRE( ae::Vec3( 0.0f ) ) == 0 ); // Should fail to link
}

#include "MapTest.h"

TEST_CASE( "HasMethodHash64", "[ae::GetHash64]" )
{
	REQUIRE( ae::_HasMethodHash64< BadHash< uint64_t > >::value );
}
