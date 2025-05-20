//------------------------------------------------------------------------------
// MacroTest.cpp
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
#include "aether.h"
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// AE_GLUE_UNDERSCORE
//------------------------------------------------------------------------------
TEST_CASE( "AE_GLUE_UNDERSCORE", "[ae macro]" )
{
	constexpr int AE_GLUE_UNDERSCORE( a ) = 0;
	STATIC_REQUIRE( a == 0 );
	constexpr int AE_GLUE_UNDERSCORE( a, b ) = 1;
	STATIC_REQUIRE( a_b == 1 );
	constexpr int AE_GLUE_UNDERSCORE( a, b, c ) = 2;
	STATIC_REQUIRE( a_b_c == 2 );
	constexpr int AE_GLUE_UNDERSCORE( a, b, c, d ) = 3;
	STATIC_REQUIRE( a_b_c_d == 3 );
}

//------------------------------------------------------------------------------
// AE_GET_LAST
//------------------------------------------------------------------------------
TEST_CASE( "AE_GET_LAST", "[ae macro]" )
{
	STATIC_REQUIRE( AE_VA_ARGS_COUNT( 1, 2, 3, 4 ) == 4 );

	constexpr int test[] = { AE_GET_LAST( 1, 2, 3, 4 ) };
	STATIC_REQUIRE( countof( test ) == 1 );
	STATIC_REQUIRE( test[ 0 ] == 4 );

	constexpr int test2[] = { AE_GET_LAST( AE_DROP_LAST( 1, 2, 3, 4 ) ) };
	STATIC_REQUIRE( countof( test2 ) == 1 );
	STATIC_REQUIRE( test2[ 0 ] == 3 );

	constexpr int test3[] = { AE_GET_LAST( AE_DROP_LAST( AE_DROP_LAST( 1, 2, 3, 4 ) ) ) };
	STATIC_REQUIRE( countof( test3 ) == 1 );
	STATIC_REQUIRE( test3[ 0 ] == 2 );
}

//------------------------------------------------------------------------------
// AE_DROP_LAST
//------------------------------------------------------------------------------
TEST_CASE( "AE_DROP_LAST", "[ae macro]" )
{
	STATIC_REQUIRE( AE_VA_ARGS_COUNT( 1, 2, 3, 4 ) == 4 );

	constexpr int test[] = { AE_DROP_LAST( 1, 2, 3, 4 ) };
	STATIC_REQUIRE( countof( test ) == 3 );
	STATIC_REQUIRE( test[ 0 ] == 1 );
	STATIC_REQUIRE( test[ 1 ] == 2 );
	STATIC_REQUIRE( test[ 2 ] == 3 );

	constexpr int test2[] = { AE_DROP_LAST( AE_DROP_LAST( 1, 2, 3, 4 ) ) };
	STATIC_REQUIRE( countof( test2 ) == 2 );
	STATIC_REQUIRE( test2[ 0 ] == 1 );
	STATIC_REQUIRE( test2[ 1 ] == 2 );
}
