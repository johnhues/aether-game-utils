//------------------------------------------------------------------------------
// MiscTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2024 John Hughes
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
#include "aether.h"
#include "catch2/catch.hpp"

//------------------------------------------------------------------------------
// ae::UnsignedInt tests
//------------------------------------------------------------------------------
TEST_CASE( "correct integer size selection", "[ae::UnsignedInt]" )
{
	REQUIRE( std::is_same_v< ae::UnsignedInt< 0, uint8_t >::Type, uint8_t > );
	REQUIRE( std::is_same_v< ae::UnsignedInt< 0, uint16_t >::Type, uint16_t > );
	REQUIRE( std::is_same_v< ae::UnsignedInt< 0, uint32_t >::Type, uint32_t > );
	REQUIRE( std::is_same_v< ae::UnsignedInt< 0, uint64_t >::Type, uint64_t > );
	REQUIRE( sizeof( ae::UnsignedInt< 8 > ) == 1 );
	REQUIRE( sizeof( ae::UnsignedInt< 16 > ) == 1 );
	REQUIRE( sizeof( ae::UnsignedInt< 32 > ) == 1 );
	REQUIRE( sizeof( ae::UnsignedInt< UINT8_MAX - 1 > ) == 1 );
	REQUIRE( sizeof( ae::UnsignedInt< UINT8_MAX > ) == 2 );
	REQUIRE( sizeof( ae::UnsignedInt< (uint64_t)UINT8_MAX + 1 > ) == 2 );
	REQUIRE( sizeof( ae::UnsignedInt< UINT16_MAX - 1 > ) == 2 );
	REQUIRE( sizeof( ae::UnsignedInt< UINT16_MAX > ) == 4 );
	REQUIRE( sizeof( ae::UnsignedInt< (uint64_t)UINT16_MAX + 1 > ) == 4 );
	REQUIRE( sizeof( ae::UnsignedInt< UINT32_MAX - 1 > ) == 4 );
	REQUIRE( sizeof( ae::UnsignedInt< UINT32_MAX > ) == 8 );
	REQUIRE( sizeof( ae::UnsignedInt< (uint64_t)UINT32_MAX + 1 > ) == 8 );
}

TEST_CASE( "print integer values", "[ae::UnsignedInt]" )
{
	std::stringstream ss;
	ss << ae::UnsignedInt< 0, uint8_t >( 'a' );
	REQUIRE( ss.str() == "97" );

	ss = {};
	ss << ae::UnsignedInt< 0, uint16_t >( 333 );
	REQUIRE( ss.str() == "333" );

	ss = {};
	ss << ae::UnsignedInt< 0, uint16_t >( 444 );
	REQUIRE( ss.str() == "444" );

	ss = {};
	ss << ae::UnsignedInt< 0, uint32_t >( 555 );
	REQUIRE( ss.str() == "555" );

	ss = {};
	ss << ae::UnsignedInt< 0, uint64_t >( 666 );
	REQUIRE( ss.str() == "666" );
}
