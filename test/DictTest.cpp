//------------------------------------------------------------------------------
// DictTest.cpp
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
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <climits>

const ae::Tag TAG_TEST = "test";

//------------------------------------------------------------------------------
// ae::New tests
//------------------------------------------------------------------------------
TEST_CASE( "Basic", "[ae::Dict]" )
{
	ae::Dict< 15 > dict;
	dict.SetInt( "int8_t", (int8_t)SCHAR_MAX );
	dict.SetInt( "int16_t", (int16_t)SHRT_MAX );
	dict.SetInt( "int32_t", (int32_t)INT_MAX );
	dict.SetInt( "int64_t", (int64_t)LONG_MAX );
	// dict.SetInt( "compileErr", (uint32_t)7 );
	dict.SetUInt( "uint8_t", (uint8_t)UCHAR_MAX );
	dict.SetUInt( "uint16_t", (uint16_t)USHRT_MAX );
	dict.SetUInt( "uint32_t", (uint32_t)UINT_MAX );
	dict.SetUInt( "uint64_t", (uint64_t)ULONG_MAX );
	// dict.SetUInt( "compileErr", (int32_t)7 );
	dict.SetFloat( "float", 3.14f );
	dict.SetFloat( "double", 3.14 );
	dict.SetString( "string", "Hello, World!" );
	dict.SetBool( "bool", true );
	dict.SetVec2( "vec2", ae::Vec2( 1.0f, 2.0f ) );
	dict.SetVec3( "vec3", ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	dict.SetVec4( "vec4", ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );

	REQUIRE( dict.GetInt( "int8_t", 0 ) == SCHAR_MAX );
	REQUIRE( dict.GetInt( "int16_t", 0 ) == SHRT_MAX );
	REQUIRE( dict.GetInt( "int32_t", 0 ) == INT_MAX );
	REQUIRE( dict.GetInt( "int64_t", 0 ) == LONG_MAX );
	REQUIRE( dict.GetUInt( "uint8_t", 0 ) == UCHAR_MAX );
	REQUIRE( dict.GetUInt( "uint16_t", 0 ) == USHRT_MAX );
	REQUIRE( dict.GetUInt( "uint32_t", 0 ) == UINT_MAX );
	REQUIRE( dict.GetUInt( "uint64_t", 0 ) == ULONG_MAX );
	REQUIRE( dict.GetFloat( "float", 0.0f ) == Catch::Approx( 3.14f ) );
	REQUIRE( dict.GetFloat( "double", 0.0 ) == Catch::Approx( 3.14 ) );
	REQUIRE( dict.GetString( "string", "" ) == ae::Str32( "Hello, World!" ) );
	REQUIRE( dict.GetBool( "bool", false ) == true );
	REQUIRE( dict.GetVec2( "vec2", ae::Vec2( 0.0f ) ) == ae::Vec2( 1.0f, 2.0f ) );
	REQUIRE( dict.GetVec3( "vec3", ae::Vec3( 0.0f ) ) == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( dict.GetVec4( "vec4", ae::Vec4( 0.0f ) ) == ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
}
