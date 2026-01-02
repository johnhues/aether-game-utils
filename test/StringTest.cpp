//------------------------------------------------------------------------------
// StringTest.cpp
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

#if _AE_WINDOWS_
	// @NOTE: Disable a few warnings caused by catch2 that should not affect correctness
	#pragma warning( disable : 6319 )
	#pragma warning( disable : 6237 )
#endif

//------------------------------------------------------------------------------
// aeString tests
//------------------------------------------------------------------------------
TEST_CASE( "string construction should set length correctly", "[aeString]" )
{
	REQUIRE( ( ae::Str16().Length() == 0 ) );
	REQUIRE( ( ae::Str16( "abc" ).Length() == 3 ) );

	char str[ 32 ] = {};
	memset( str, 'q', ae::Str16::MaxLength() );
	const uint32_t maxLength = ae::Str16::MaxLength();
	REQUIRE( ae::Str16( str ).Length() == maxLength );

	const char* strPath = "/var/log/system.log.1.gz";
	const char* fileName = ae::FileSystem::GetFileNameFromPath( strPath );
	const char* fileExt = ae::FileSystem::GetFileExtFromPath( strPath, true );
	REQUIRE( strcmp( fileName, "system.log.1.gz" ) == 0 );
	REQUIRE( strcmp( fileExt, ".log.1.gz" ) == 0 );
	REQUIRE( ae::Str16( fileName, fileExt ).Length() == strlen("system") );
	REQUIRE( ae::Str16( fileName, fileExt ) == "system" );
}

TEST_CASE( "strings of the same length can be compared alphabetically", "[aeString]" )
{
	ae::Str32 str0 = "abc";
	ae::Str32 str1 = "xyz";
	const char* str2 = "abc";
	ae::Str32 str3 = "xyz";
	ae::Str32 str4 = "abc";
	const char* str5 = "xyz";

	SECTION( "equivalency comparison" )
	{
		REQUIRE( str0 == str0 );
		REQUIRE( str0 == str2 );
		REQUIRE( str2 == str4 );
		REQUIRE( !( str0 == str1 ) );
		REQUIRE( !( str0 == str3 ) );
		REQUIRE( !( str5 == str0 ) );

		REQUIRE( !( str0 != str0 ) );
		REQUIRE( !( str0 != str2 ) );
		REQUIRE( !( str2 != str4 ) );
		REQUIRE( str0 != str1 );
		REQUIRE( str0 != str3 );
		REQUIRE( str5 != str0 );
	}

	SECTION( "less than comparison" )
	{
		REQUIRE( str0 < str1 );
		REQUIRE( str2 < str3 );
		REQUIRE( str4 < str5 );
		REQUIRE( !( str1 < str0 ) );
		REQUIRE( !( str3 < str2 ) );
		REQUIRE( !( str5 < str4 ) );
	}

	SECTION( "greater than comparison" )
	{
		REQUIRE( !( str0 > str1 ) );
		REQUIRE( !( str2 > str3 ) );
		REQUIRE( !( str4 > str5 ) );
		REQUIRE( str1 > str0 );
		REQUIRE( str3 > str2 );
		REQUIRE( str5 > str4 );
	}

	SECTION( "less than or equal comparison" )
	{
		REQUIRE( str0 <= str1 );
		REQUIRE( str2 <= str3 );
		REQUIRE( str4 <= str5 );
		REQUIRE( !( str1 <= str0 ) );
		REQUIRE( !( str3 <= str2 ) );
		REQUIRE( !( str5 <= str4 ) );

		REQUIRE( str0 <= str0 );
		REQUIRE( str0 <= str2 );
		REQUIRE( str2 <= str4 );
	}

	SECTION( "greater than or equal comparison" )
	{
		REQUIRE( !( str0 >= str1 ) );
		REQUIRE( !( str2 >= str3 ) );
		REQUIRE( !( str4 >= str5 ) );
		REQUIRE( str1 >= str0 );
		REQUIRE( str3 >= str2 );
		REQUIRE( str5 >= str4 );

		REQUIRE( str0 >= str0 );
		REQUIRE( str0 >= str2 );
		REQUIRE( str2 >= str4 );
	}
}

TEST_CASE( "strings of different lengths can be compared alphabetically", "[aeString]" )
{
	ae::Str32 str0 = "abcd";
	ae::Str32 str1 = "xyz";
	ae::Str32 str2 = "abcd";
	const char* str3 = "xyz";
	const char* str4 = "abcd";
	ae::Str32 str5 = "xyz";

	SECTION( "equivalency comparison" )
	{
		REQUIRE( str0 == str0 );
		REQUIRE( str0 == str2 );
		REQUIRE( str2 == str4 );
		REQUIRE( !( str0 == str1 ) );
		REQUIRE( !( str0 == str3 ) );
		REQUIRE( !( str5 == str0 ) );

		REQUIRE( !( str0 != str0 ) );
		REQUIRE( !( str0 != str2 ) );
		REQUIRE( !( str2 != str4 ) );
		REQUIRE( str0 != str1 );
		REQUIRE( str0 != str3 );
		REQUIRE( str5 != str0 );
	}

	SECTION( "less than comparison" )
	{
		REQUIRE( str0 < str1 );
		REQUIRE( str2 < str3 );
		REQUIRE( str4 < str5 );
		REQUIRE( !(str1 < str0) );
		REQUIRE( !(str3 < str2) );
		REQUIRE( !( str5 < str4 ) );
	}

	SECTION( "greater than comparison" )
	{
		REQUIRE( !( str0 > str1 ) );
		REQUIRE( !( str2 > str3 ) );
		REQUIRE( !( str4 > str5 ) );
		REQUIRE( str1 > str0 );
		REQUIRE( str3 > str2 );
		REQUIRE( str5 > str4 );
	}

	SECTION( "less than or equal comparison" )
	{
		REQUIRE( str0 <= str1 );
		REQUIRE( str2 <= str3 );
		REQUIRE( str4 <= str5 );
		REQUIRE( !( str1 <= str0 ) );
		REQUIRE( !( str3 <= str2 ) );
		REQUIRE( !( str5 <= str4 ) );

		REQUIRE( str0 <= str0 );
		REQUIRE( str0 <= str2 );
		REQUIRE( str2 <= str4 );
	}

	SECTION( "greater than or equal comparison" )
	{
		REQUIRE( !( str0 >= str1 ) );
		REQUIRE( !( str2 >= str3 ) );
		REQUIRE( !( str4 >= str5 ) );
		REQUIRE( str1 >= str0 );
		REQUIRE( str3 >= str2 );
		REQUIRE( str5 >= str4 );

		REQUIRE( str0 >= str0 );
		REQUIRE( str0 >= str2 );
		REQUIRE( str2 >= str4 );
	}
}

TEST_CASE( "ToString", "[aeString]" )
{
	REQUIRE( ae::ToString( ae::Vec2( 1.0f, 2.0f ) ) == "1.000 2.000" );
	REQUIRE( ae::ToString( ae::Vec3( 1.0f, 2.0f, 3.0f ) ) == "1.000 2.000 3.000" );
	REQUIRE( ae::ToString( ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) ) == "1.000 2.000 3.000 4.000" );
	REQUIRE( ae::ToString( ae::Matrix4::Identity() ) == "1.000 0.000 0.000 0.000 0.000 1.000 0.000 0.000 0.000 0.000 1.000 0.000 0.000 0.000 0.000 1.000" );
	REQUIRE( ae::ToString( ae::Color::Red() ) == "1.000 0.000 0.000 1.000" );
	REQUIRE( ae::ToString( true ) == "true" );
	REQUIRE( ae::ToString( false ) == "false" );
}

TEST_CASE( "FromString success", "[aeString]" )
{
	REQUIRE( ae::FromString( "1 2", ae::Vec2( 0.0f ) ) == ae::Vec2( 1.0f, 2.0f ) );
	REQUIRE( ae::FromString( "1 2 3", ae::Vec2( 0.0f ) ) == ae::Vec2( 1.0f, 2.0f ) );
	REQUIRE( ae::FromString( "1 2 3 4", ae::Vec2( 0.0f ) ) == ae::Vec2( 1.0f, 2.0f ) );
	REQUIRE( ae::FromString( "1 2 3", ae::Vec3( 0.0f ) ) == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( ae::FromString( "1 2 3 4", ae::Vec3( 0.0f ) ) == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( ae::FromString( "1 2 3 4", ae::Vec4( 0.0f ) ) == ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	REQUIRE( ae::FromString( "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1", ae::Matrix4::Scaling( 0.0f ) ) == ae::Matrix4::Identity() );
	REQUIRE( ae::FromString( "1 0 0 1", ae::Color::Black() ) == ae::Color::Red() );
	
	REQUIRE( ae::FromString( "true", false ) == true );
	REQUIRE( ae::FromString( "false", true ) == false );
	REQUIRE( ae::FromString( "TRUE", false ) == true );
	REQUIRE( ae::FromString( "FALSE", true ) == false );
	REQUIRE( ae::FromString( "1", false ) == true );
	REQUIRE( ae::FromString( "123", false ) == true );
	REQUIRE( ae::FromString( "-123", false ) == true );
	REQUIRE( ae::FromString( "10.04", false ) == true );
	REQUIRE( ae::FromString( "-10.04", false ) == true );
	REQUIRE( ae::FromString( "0", true ) == false );
	REQUIRE( ae::FromString( "0.0", true ) == false );
	REQUIRE( ae::FromString( ".0", true ) == false );
	REQUIRE( ae::FromString( "-0", true ) == false );
	REQUIRE( ae::FromString( "-0.0", true ) == false );
	REQUIRE( ae::FromString( "-.0", true ) == false );
	REQUIRE( ae::FromString( "INF", false ) == true );
	REQUIRE( ae::FromString( "-INF", false ) == true );
	REQUIRE( ae::FromString( "NAN", false ) == true );
}

TEST_CASE( "FromString fail", "[aeString]" )
{
	REQUIRE( ae::FromString( "1", ae::Vec2( 1.0f, 2.0f ) ) == ae::Vec2( 1.0f, 2.0f ) );
	REQUIRE( ae::FromString( "a b", ae::Vec2( 1.0f, 2.0f ) ) == ae::Vec2( 1.0f, 2.0f ) );

	REQUIRE( ae::FromString( "1 2", ae::Vec3( 1.0f, 2.0f, 3.0f ) ) == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( ae::FromString( "1 2 3", ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) ) == ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	REQUIRE( ae::FromString( "1 0 0 0 0 1 0 0 4 0 1 0 0 0 2", ae::Matrix4::Identity() ) == ae::Matrix4::Identity() );
	REQUIRE( ae::FromString( "1 0 0", ae::Color::Red() ) == ae::Color::Red() );
	
	REQUIRE( ae::FromString( "tru", false ) == false );
	REQUIRE( ae::FromString( "fals", true ) == true );
	REQUIRE( ae::FromString( "truex", false ) == false );
	REQUIRE( ae::FromString( "falsex", true ) == true );
	REQUIRE( ae::FromString( "a", false ) == false );
	REQUIRE( ae::FromString( "a", true ) == true );
	REQUIRE( ae::FromString( "", false ) == false );
	REQUIRE( ae::FromString( "", true ) == true );
	REQUIRE( ae::FromString( ".", false ) == false );
	REQUIRE( ae::FromString( ".", true ) == true );
}

TEST_CASE( "Str::Append", "[aeString]" )
{
	SECTION( "append const char*" )
	{
		ae::Str32 str = "Hello";
		str.Append( " World" );
		REQUIRE( str == "Hello World" );
		REQUIRE( str.Length() == 11 );
	}

	SECTION( "append Str" )
	{
		ae::Str32 str = "Hello";
		ae::Str16 suffix = " World";
		str.Append( suffix );
		REQUIRE( str == "Hello World" );
		REQUIRE( str.Length() == 11 );
	}

	SECTION( "append empty string" )
	{
		ae::Str32 str = "Hello";
		str.Append( "" );
		REQUIRE( str == "Hello" );
		REQUIRE( str.Length() == 5 );
	}
}

TEST_CASE( "Str::Insert basic functionality", "[aeString]" )
{
	SECTION( "insert at beginning" )
	{
		ae::Str32 str = "World";
		str.Insert( 0, "Hello " );
		REQUIRE( str == "Hello World" );
		REQUIRE( str.Length() == 11 );
	}

	SECTION( "insert at end" )
	{
		ae::Str32 str = "Hello";
		str.Insert( 5, " World" );
		REQUIRE( str == "Hello World" );
		REQUIRE( str.Length() == 11 );
	}

	SECTION( "insert in middle" )
	{
		ae::Str32 str = "HellWorld";
		str.Insert( 4, "o " );
		REQUIRE( str == "Hello World" );
		REQUIRE( str.Length() == 11 );
	}

	SECTION( "insert empty string" )
	{
		ae::Str32 str = "Hello";
		str.Insert( 2, "" );
		REQUIRE( str == "Hello" );
		REQUIRE( str.Length() == 5 );
	}

	SECTION( "insert Str object" )
	{
		ae::Str32 str = "Hello";
		ae::Str16 insert = " World";
		str.Insert( 5, insert );
		REQUIRE( str == "Hello World" );
		REQUIRE( str.Length() == 11 );
	}
}

TEST_CASE( "Str::Insert multiple operations", "[aeString]" )
{
	ae::Str64 str = "";
	str.Insert( 0, "fox" );
	REQUIRE( str == "fox" );
	REQUIRE( str.Length() == 3 );
	
	str.Insert( 0, "The " );
	REQUIRE( str == "The fox" );
	REQUIRE( str.Length() == 7 );
	
	str.Insert( 4, "quick " );
	REQUIRE( str == "The quick fox" );
	REQUIRE( str.Length() == 13 );
	
	str.Insert( 10, "brown " );
	REQUIRE( str == "The quick brown fox" );
	REQUIRE( str.Length() == 19 );
	
	str.Insert( 19, " jumps" );
	REQUIRE( str == "The quick brown fox jumps" );
	REQUIRE( str.Length() == 25 );
}

TEST_CASE( "Str self-append handling", "[aeString]" )
{
	SECTION( "append string to itself using c_str" )
	{
		ae::Str32 str = "Hello";
		str += str.c_str();
		REQUIRE( str == "HelloHello" );
		REQUIRE( str.Length() == 10 );
	}

	SECTION( "append string to itself using operator" )
	{
		ae::Str32 str = "Hello";
		str += str;
		REQUIRE( str == "HelloHello" );
		REQUIRE( str.Length() == 10 );
	}

	SECTION( "append substring to itself" )
	{
		ae::Str32 str = "Hello";
		str += &str.c_str()[1]; // Append "ello"
		REQUIRE( str == "Helloello" );
		REQUIRE( str.Length() == 9 );
	}

	SECTION( "multiple self-appends" )
	{
		ae::Str32 str = "A";
		str += str; // "AA"
		REQUIRE( str == "AA" );
		str += str; // "AAAA"
		REQUIRE( str == "AAAA" );
		str += str; // "AAAAAAAA"
		REQUIRE( str == "AAAAAAAA" );
		REQUIRE( str.Length() == 8 );
	}
}

TEST_CASE( "Str self-insert handling", "[aeString]" )
{
	SECTION( "insert string into itself using c_str at beginning" )
	{
		ae::Str32 str = "Hello";
		str.Insert( 0, str.c_str() );
		REQUIRE( str == "HelloHello" );
		REQUIRE( str.Length() == 10 );
	}

	SECTION( "insert string into itself using c_str in middle" )
	{
		ae::Str32 str = "Hello";
		str.Insert( 2, str.c_str() );
		REQUIRE( str == "HeHellollo" );
		REQUIRE( str.Length() == 10 );
	}

	SECTION( "insert string into itself using c_str at end" )
	{
		ae::Str32 str = "Hello";
		str.Insert( 5, str.c_str() );
		REQUIRE( str == "HelloHello" );
		REQUIRE( str.Length() == 10 );
	}

	SECTION( "insert substring into itself" )
	{
		ae::Str32 str = "Hello";
		str.Insert( 2, &str.c_str()[1] ); // Insert "ello" at position 2
		REQUIRE( str == "Heellollo" );
		REQUIRE( str.Length() == 9 );
	}

	SECTION( "insert Str object into itself" )
	{
		ae::Str32 str = "Hello";
		str.Insert( 2, str );
		REQUIRE( str == "HeHellollo" );
		REQUIRE( str.Length() == 10 );
	}
}
