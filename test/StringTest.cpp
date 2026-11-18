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
#include "TestUtils.h"
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
	const char* fileExt = ae::FileSystem::GetFileExtensionFromPath( strPath, true );
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

TEST_CASE( "Str equality with other string types", "[aeString]" )
{
	const ae::Str32 str32 = "abc";
	const ae::Str16 str16 = "abc";
	const ae::Str16 other16 = "xyz";
	const std::string stdStr = "abc";
	const std::string otherStdStr = "xyz";

	REQUIRE( str32 == str16 );
	REQUIRE( str16 == str32 );
	REQUIRE( str32 != other16 );
	REQUIRE( other16 != str32 );
	REQUIRE_FALSE( str16 != str32 );

	REQUIRE( stdStr == str32 );
	REQUIRE( str32 == stdStr );
	REQUIRE( otherStdStr != str32 );
	REQUIRE( str32 != otherStdStr );
	REQUIRE_FALSE( stdStr != str32 );
	REQUIRE_FALSE( str32 != stdStr );
}

TEST_CASE( "Str chained assignment", "[aeString]" )
{
	SECTION( "same size" )
	{
		ae::Str16 a = "a";
		ae::Str16 b = "b";
		const ae::Str16 c = "abc";
		a = b = c;
		REQUIRE( a == "abc" );
		REQUIRE( b == "abc" );
	}

	SECTION( "different sizes" )
	{
		ae::Str16 a = "a";
		ae::Str32 b = "b";
		ae::Str64 c = "abc";
		a = b = c;
		REQUIRE( a == "abc" );
		REQUIRE( b == "abc" );
	}

	SECTION( "mixed string types" )
	{
		ae::Str16 a = "a";
		ae::Str32 b = "b";
		a = b = std::string( "abc" );
		REQUIRE( a == "abc" );
		REQUIRE( b == "abc" );
		a = b = "xyz";
		REQUIRE( a == "xyz" );
		REQUIRE( a.Length() == 3 );
	}

	SECTION( "assign substring of itself" )
	{
		ae::Str16 a = "Hello";
		REQUIRE( ( a = &a.c_str()[ 1 ] ) == "ello" );
		REQUIRE( a.Length() == 4 );
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

TEST_CASE( "TryFromString success", "[aeString]" )
{
	// integer types
	int8_t i8 = 0; REQUIRE( ae::TryFromString( "-128", &i8 ) ); REQUIRE( i8 == INT8_MIN );
	int8_t i8b = 0; REQUIRE( ae::TryFromString( "127", &i8b ) ); REQUIRE( i8b == INT8_MAX );
	int16_t i16 = 0; REQUIRE( ae::TryFromString( "-32768", &i16 ) ); REQUIRE( i16 == INT16_MIN );
	int16_t i16b = 0; REQUIRE( ae::TryFromString( "32767", &i16b ) ); REQUIRE( i16b == INT16_MAX );
	int32_t i32 = 0; REQUIRE( ae::TryFromString( "-2147483648", &i32 ) ); REQUIRE( i32 == INT32_MIN );
	int32_t i32b = 0; REQUIRE( ae::TryFromString( "2147483647", &i32b ) ); REQUIRE( i32b == INT32_MAX );
	int64_t i64 = 0; REQUIRE( ae::TryFromString( "-9223372036854775808", &i64 ) ); REQUIRE( i64 == INT64_MIN );
	int64_t i64b = 0; REQUIRE( ae::TryFromString( "9223372036854775807", &i64b ) ); REQUIRE( i64b == INT64_MAX );
	uint8_t u8 = 0; REQUIRE( ae::TryFromString( "255", &u8 ) ); REQUIRE( u8 == UINT8_MAX );
	uint16_t u16 = 0; REQUIRE( ae::TryFromString( "65535", &u16 ) ); REQUIRE( u16 == UINT16_MAX );
	uint32_t u32 = 0; REQUIRE( ae::TryFromString( "4294967295", &u32 ) ); REQUIRE( u32 == UINT32_MAX );
	uint64_t u64 = 0; REQUIRE( ae::TryFromString( "18446744073709551615", &u64 ) ); REQUIRE( u64 == UINT64_MAX );
	// float / double
	float f = 0.0f; REQUIRE( ae::TryFromString( "1.5", &f ) ); REQUIRE( f == 1.5f );
	double d = 0.0; REQUIRE( ae::TryFromString( "1.5", &d ) ); REQUIRE( d == 1.5 );
	// composite
	ae::Vec2 v2; REQUIRE( ae::TryFromString( "1 2", &v2 ) ); REQUIRE( v2 == ae::Vec2( 1.0f, 2.0f ) );
	ae::Vec3 v3; REQUIRE( ae::TryFromString( "1 2 3", &v3 ) ); REQUIRE( v3 == ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	ae::Vec4 v4; REQUIRE( ae::TryFromString( "1 2 3 4", &v4 ) ); REQUIRE( v4 == ae::Vec4( 1.0f, 2.0f, 3.0f, 4.0f ) );
	bool b = false; REQUIRE( ae::TryFromString( "true", &b ) ); REQUIRE( b == true );
	bool b2 = true; REQUIRE( ae::TryFromString( "false", &b2 ) ); REQUIRE( b2 == false );
	// permissive: leading whitespace, + prefix, trailing garbage
	int32_t perm = 0;
	REQUIRE( ae::TryFromString( " 5", &perm ) ); REQUIRE( perm == 5 );
	REQUIRE( ae::TryFromString( "+5", &perm ) ); REQUIRE( perm == 5 );
	REQUIRE( ae::TryFromString( "5abc", &perm ) ); REQUIRE( perm == 5 );
	// out param unchanged on failure
	int32_t unchanged = 99;
	REQUIRE( !ae::TryFromString( "abc", &unchanged ) ); REQUIRE( unchanged == 99 );
}

TEST_CASE( "TryFromString fail", "[aeString]" )
{
	int32_t i = 0;
	REQUIRE( !ae::TryFromString( "", &i ) );
	REQUIRE( !ae::TryFromString( "abc", &i ) );
	// 64-bit overflow
	int64_t i64 = 0; REQUIRE( !ae::TryFromString( "9223372036854775808", &i64 ) );
	uint64_t u64 = 0; REQUIRE( !ae::TryFromString( "18446744073709551616", &u64 ) );
	// incomplete component count
	ae::Vec3 v3 = ae::Vec3( 1.0f ); REQUIRE( !ae::TryFromString( "1 2", &v3 ) ); REQUIRE( v3 == ae::Vec3( 1.0f ) );
	ae::Matrix4 m = ae::Matrix4::Identity();
	REQUIRE( !ae::TryFromString( "1 0 0 0 0 1 0 0 0 0 1 0 0 0 2", &m ) );
	REQUIRE( m == ae::Matrix4::Identity() );
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

//------------------------------------------------------------------------------
// ae::Str coverage
//------------------------------------------------------------------------------
namespace
{
	//! Only provides c_str(), measured with strlen.
	struct NameTag
	{
		const char* c_str() const { return name; }
		const char* name;
	};

	//! Implicitly convertible to const char*.
	struct Label
	{
		operator const char*() const { return text; }
		const char* text;
	};

	//! Only convertible to const char* when non-const, which ae::Str rejects.
	struct MutableLabel
	{
		operator const char*() { return text; }
		const char* text;
	};

	template< typename A, typename B >
	concept StrComparable = requires( const A& a, const B& b ) { a == b; a < b; };
}

TEST_CASE( "Str type constraints", "[aeString]" )
{
	static_assert( sizeof( ae::Str16 ) == 16 );
	static_assert( sizeof( ae::Str256 ) == 256 );
	static_assert( ae::Str16::MaxLength() == 13 );

	static_assert( std::is_convertible_v< const char*, ae::Str16 > );
	static_assert( std::is_convertible_v< char*, ae::Str16 > );
	static_assert( std::is_convertible_v< const char( & )[ 4 ], ae::Str16 > );
	static_assert( std::is_convertible_v< ae::Str64, ae::Str16 > );
	static_assert( std::is_convertible_v< std::string, ae::Str16 > );
	static_assert( std::is_convertible_v< NameTag, ae::Str16 > );
	static_assert( std::is_convertible_v< Label, ae::Str16 > );

	static_assert( !std::is_constructible_v< ae::Str16, std::nullptr_t > );
	static_assert( !std::is_constructible_v< ae::Str16, int > );
	static_assert( !std::is_constructible_v< ae::Str16, const wchar_t* > );
	static_assert( !std::is_constructible_v< ae::Str16, std::wstring > );
	static_assert( !std::is_constructible_v< ae::Str16, MutableLabel > );

	static_assert( !std::is_convertible_v< ae::Str16, const char* > );
	static_assert( std::is_constructible_v< const char*, ae::Str16 > );

	static_assert( StrComparable< ae::Str16, ae::Str32 > );
	static_assert( StrComparable< const char*, ae::Str16 > );
	static_assert( StrComparable< std::string, ae::Str16 > );
	static_assert( StrComparable< ae::Str16, NameTag > );
	static_assert( !StrComparable< ae::Str16, std::nullptr_t > );
	static_assert( !StrComparable< ae::Str16, int > );
	static_assert( !StrComparable< ae::Str16, std::wstring > );

	static_assert( std::is_same_v< decltype( ae::Str16() + ae::Str64() ), ae::Str16 > );
	static_assert( std::is_same_v< decltype( ae::Str64() + ae::Str16() ), ae::Str64 > );
	static_assert( std::is_same_v< decltype( ae::Str16() = "a" ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16() = ae::Str32() ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16() += "a" ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16() += ae::Str32() ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16().Append( "a" ) ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16().Append( ae::Str32() ) ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16().Insert( 0, "a" ) ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16().Insert( 0, ae::Str32() ) ), ae::Str16& > );
	static_assert( std::is_same_v< decltype( ae::Str16() <=> "a" ), std::strong_ordering > );
}

TEST_CASE( "Str construction from string types", "[aeString]" )
{
	char buffer[ 8 ] = "abc";
	char* const mutablePtr = buffer;
	const char* const constPtr = "abc";
	const NameTag tag = { "abc" };
	const Label label = { "abc" };
	const std::string stdStr = "abc";

	SECTION( "default is empty" )
	{
		const ae::Str16 str;
		REQUIRE( str.Empty() );
		REQUIRE( str.Length() == 0 );
		REQUIRE( str == "" );
		REQUIRE( strcmp( str.c_str(), "" ) == 0 );
	}

	SECTION( "every accepted source type" )
	{
		const ae::Str16 strs[] =
		{
			ae::Str16( "abc" ),
			ae::Str16( buffer ),
			ae::Str16( mutablePtr ),
			ae::Str16( constPtr ),
			ae::Str16( ae::Str256( "abc" ) ),
			ae::Str16( stdStr ),
			ae::Str16( tag ),
			ae::Str16( label ),
		};
		for( const ae::Str16& str : strs )
		{
			REQUIRE( str == "abc" );
			REQUIRE( str.Length() == 3 );
			REQUIRE( strlen( str.c_str() ) == 3 );
		}
	}

	SECTION( "implicit conversion to function arguments" )
	{
		const auto length = []( const ae::Str16& str ) { return str.Length(); };
		REQUIRE( length( "abcd" ) == 4 );
		REQUIRE( length( std::string( "abcd" ) ) == 4 );
		REQUIRE( length( ae::Str64( "abcd" ) ) == 4 );
		REQUIRE( length( tag ) == 3 );
		REQUIRE( length( label ) == 3 );
	}

	SECTION( "length and character constructors" )
	{
		REQUIRE( ae::Str16( 3u, "abcdef" ) == "abc" );
		REQUIRE( ae::Str16( 3u, "abcdef" ).Length() == 3 );
		REQUIRE( ae::Str16( 0u, "abcdef" ).Empty() );
		REQUIRE( ae::Str16( 4u, 'x' ) == "xxxx" );
		REQUIRE( ae::Str16( 4u, 'x' ).Length() == 4 );
		REQUIRE( ae::Str16( 0u, 'x' ).Empty() );
		const char* const range = "abcdef";
		REQUIRE( ae::Str16( range + 1, range + 4 ) == "bcd" );
		REQUIRE( ae::Str16( range, range ).Empty() );
	}

	SECTION( "max length" )
	{
		const ae::Str16 str( ae::Str16::MaxLength(), 'q' );
		REQUIRE( str.Length() == ae::Str16::MaxLength() );
		REQUIRE( ae::Str16( str.c_str() ) == str );
		REQUIRE( ae::Str16( std::string( ae::Str16::MaxLength(), 'q' ) ) == str );
	}
}

TEST_CASE( "Str assignment", "[aeString]" )
{
	SECTION( "every accepted source type" )
	{
		const NameTag tag = { "tag" };
		const Label label = { "label" };
		ae::Str16 str = "start";
		str = "literal"; REQUIRE( str == "literal" ); REQUIRE( str.Length() == 7 );
		str = std::string( "std" ); REQUIRE( str == "std" ); REQUIRE( str.Length() == 3 );
		str = ae::Str64( "str64" ); REQUIRE( str == "str64" ); REQUIRE( str.Length() == 5 );
		str = ae::Str16( "str16" ); REQUIRE( str == "str16" ); REQUIRE( str.Length() == 5 );
		str = tag; REQUIRE( str == "tag" ); REQUIRE( str.Length() == 3 );
		str = label; REQUIRE( str == "label" ); REQUIRE( str.Length() == 5 );
		str = ""; REQUIRE( str.Empty() ); REQUIRE( strlen( str.c_str() ) == 0 );
	}

	SECTION( "shorter value terminates" )
	{
		ae::Str16 str = "abcdefgh";
		str = "xy";
		REQUIRE( str == "xy" );
		REQUIRE( strlen( str.c_str() ) == 2 );
	}

	SECTION( "returns itself" )
	{
		ae::Str16 str;
		REQUIRE( &( str = "a" ) == &str );
		REQUIRE( &( str = ae::Str32( "b" ) ) == &str );
		REQUIRE( &( str = ae::Str16( "c" ) ) == &str );
		REQUIRE( &( str = std::string( "d" ) ) == &str );
	}

	SECTION( "self assignment" )
	{
		ae::Str16 str = "abc";
		const ae::Str16& alias = str;
		str = alias;
		REQUIRE( str == "abc" );
		str = str.c_str();
		REQUIRE( str == "abc" );
		REQUIRE( str.Length() == 3 );
	}
}

TEST_CASE( "Str concatenation", "[aeString]" )
{
	const NameTag tag = { "tag" };
	const Label label = { "label" };

	SECTION( "operator+ with every accepted source type" )
	{
		const ae::Str32 str = "a";
		REQUIRE( str + "b" == "ab" );
		REQUIRE( str + std::string( "b" ) == "ab" );
		REQUIRE( str + ae::Str16( "b" ) == "ab" );
		REQUIRE( str + ae::Str64( "b" ) == "ab" );
		REQUIRE( str + tag == "atag" );
		REQUIRE( str + label == "alabel" );
		REQUIRE( ( str + "bcd" ).Length() == 4 );
		REQUIRE( str == "a" );
	}

	SECTION( "operator+ chain" )
	{
		const ae::Str32 str = ae::Str32( "a" ) + "b" + std::string( "c" ) + ae::Str16( "d" );
		REQUIRE( str == "abcd" );
		REQUIRE( str.Length() == 4 );
	}

	SECTION( "operator+= with every accepted source type" )
	{
		ae::Str32 str = "a";
		str += "b";
		str += std::string( "c" );
		str += ae::Str16( "d" );
		str += ae::Str64( "e" );
		str += tag;
		str += label;
		str += "";
		REQUIRE( str == "abcdetaglabel" );
		REQUIRE( str.Length() == 13 );
	}

	SECTION( "Append with every accepted source type" )
	{
		ae::Str32 str = "a";
		str.Append( std::string( "b" ) );
		str.Append( ae::Str64( "c" ) );
		str.Append( tag );
		str.Append( label );
		REQUIRE( str == "abctaglabel" );
		REQUIRE( str.Length() == 11 );
	}

	SECTION( "operator+= returns itself" )
	{
		ae::Str32 str = "a";
		REQUIRE( &( str += "b" ) == &str );
		REQUIRE( &( str += ae::Str16( "c" ) ) == &str );
		REQUIRE( &( str += std::string( "d" ) ) == &str );
		( str += "e" ) += "f";
		REQUIRE( str == "abcdef" );
		REQUIRE( ( str += str ) == "abcdefabcdef" );
		REQUIRE( str.Length() == 12 );
	}

	SECTION( "Append returns itself" )
	{
		ae::Str32 str = "a";
		REQUIRE( &str.Append( "b" ) == &str );
		REQUIRE( &str.Append( ae::Str16( "c" ) ) == &str );
		REQUIRE( &str.Append( std::string( "d" ) ) == &str );
		str.Append( "e" ).Append( "f" );
		REQUIRE( str == "abcdef" );
		REQUIRE( str.Append( str ) == "abcdefabcdef" );
		REQUIRE( str.Length() == 12 );
	}

	SECTION( "fill to max length" )
	{
		ae::Str16 str = "abc";
		str += "defghijklm";
		REQUIRE( str.Length() == ae::Str16::MaxLength() );
		REQUIRE( str == "abcdefghijklm" );
	}
}

TEST_CASE( "Str::Insert with string types", "[aeString]" )
{
	SECTION( "into empty" )
	{
		ae::Str16 str;
		str.Insert( 0, "abc" );
		REQUIRE( str == "abc" );
		REQUIRE( str.Length() == 3 );
	}

	SECTION( "every accepted source type" )
	{
		const NameTag tag = { "T" };
		const Label label = { "L" };
		ae::Str32 str = "ad";
		str.Insert( 1, std::string( "b" ) );
		str.Insert( 2, ae::Str64( "c" ) );
		str.Insert( 0, tag );
		str.Insert( 5, label );
		REQUIRE( str == "TabcdL" );
		REQUIRE( str.Length() == 6 );
	}

	SECTION( "returns itself" )
	{
		ae::Str32 str = "ad";
		REQUIRE( &str.Insert( 1, "c" ) == &str );
		REQUIRE( &str.Insert( 1, ae::Str16( "b" ) ) == &str );
		REQUIRE( &str.Insert( 4, std::string( "f" ) ) == &str );
		str.Insert( 4, "e" ).Insert( 0, "_" );
		REQUIRE( str == "_abcdef" );
		REQUIRE( str.Insert( 1, str ) == "__abcdefabcdef" );
		REQUIRE( str.Length() == 14 );
	}

	SECTION( "fill to max length" )
	{
		ae::Str16 str = "am";
		str.Insert( 1, "bcdefghijkl" );
		REQUIRE( str == "abcdefghijklm" );
		REQUIRE( str.Length() == ae::Str16::MaxLength() );
	}

	SECTION( "self insert at every index" )
	{
		for( uint32_t i = 0; i <= 4; i++ )
		{
			ae::Str32 str = "abcd";
			std::string expected = "abcd";
			expected.insert( i, expected );
			str.Insert( i, str );
			REQUIRE( str == expected );
			REQUIRE( str.Length() == 8 );
		}
	}

	SECTION( "self substring insert matches std::string" )
	{
		for( uint32_t index = 0; index <= 4; index++ )
		{
			for( uint32_t offset = 0; offset < 4; offset++ )
			{
				ae::Str32 str = "abcd";
				std::string expected = "abcd";
				expected.insert( index, expected.c_str() + offset );
				str.Insert( index, str.c_str() + offset );
				REQUIRE( str == expected );
				REQUIRE( str.Length() == expected.size() );
			}
		}
	}
}

TEST_CASE( "Str comparison", "[aeString]" )
{
	SECTION( "matches strcmp ordering" )
	{
		const char* const strs[] = { "", "a", "ab", "abc", "abd", "b", "B", "a\x7f", "a\xff", "\xff" };
		for( const char* a : strs )
		{
			for( const char* b : strs )
			{
				const int32_t expected = strcmp( a, b );
				const ae::Str16 strA = a;
				const std::strong_ordering order = strA <=> b;
				REQUIRE( ( order < 0 ) == ( expected < 0 ) );
				REQUIRE( ( order == 0 ) == ( expected == 0 ) );
				REQUIRE( ( order > 0 ) == ( expected > 0 ) );
				REQUIRE( ( strA == ae::Str32( b ) ) == ( expected == 0 ) );
				REQUIRE( ( strA < std::string( b ) ) == ( expected < 0 ) );
				REQUIRE( ( b < strA ) == ( expected > 0 ) );
				REQUIRE( ( std::string( b ) >= strA ) == ( expected <= 0 ) );
			}
		}
	}

	SECTION( "prefixes" )
	{
		const ae::Str16 ab = "ab";
		const ae::Str32 abc = "abc";
		REQUIRE( ab != abc );
		REQUIRE( ab != "abc" );
		REQUIRE( "abc" != ab );
		REQUIRE( ab < abc );
		REQUIRE( abc > ab );
		REQUIRE( "ab" < abc );
		REQUIRE( ( ab <=> abc ) == std::strong_ordering::less );
		REQUIRE( ( abc <=> ab ) == std::strong_ordering::greater );
		REQUIRE( ( "abc" <=> ab ) == std::strong_ordering::greater );
		REQUIRE( ( ab <=> "ab" ) == std::strong_ordering::equal );
	}

	SECTION( "empty" )
	{
		const ae::Str16 empty;
		REQUIRE( empty == "" );
		REQUIRE( "" == empty );
		REQUIRE( empty == ae::Str64() );
		REQUIRE( empty == std::string() );
		REQUIRE( empty < "a" );
		REQUIRE( ( empty <=> "" ) == std::strong_ordering::equal );
	}

	SECTION( "custom string types" )
	{
		const NameTag tag = { "abc" };
		const Label label = { "abd" };
		const ae::Str16 str = "abc";
		REQUIRE( str == tag );
		REQUIRE( tag == str );
		REQUIRE( str != label );
		REQUIRE( str < label );
		REQUIRE( label > str );
	}

	SECTION( "after modification" )
	{
		ae::Str16 str = "abcdef";
		str.Trim( 3 );
		REQUIRE( str == "abc" );
		REQUIRE( str < "abcd" );
		str[ 1 ] = 'z';
		REQUIRE( str == "azc" );
		REQUIRE( str > "ab" );
		str += "d";
		REQUIRE( str == "azcd" );
	}

	SECTION( "hash matches across sizes" )
	{
		REQUIRE( ae::GetHash32( ae::Str16( "abc" ) ) == ae::GetHash32( ae::Str64( "abc" ) ) );
		REQUIRE( ae::GetHash64( ae::Str16( "abc" ) ) == ae::GetHash64( ae::Str64( "abc" ) ) );
	}
}

TEST_CASE( "Str element access and size", "[aeString]" )
{
	SECTION( "operator[]" )
	{
		ae::Str16 str = "abc";
		REQUIRE( str[ 0 ] == 'a' );
		REQUIRE( str[ 2 ] == 'c' );
		REQUIRE( str[ 3 ] == 0 );
		str[ 0 ] = 'x';
		const ae::Str16& constStr = str;
		REQUIRE( constStr[ 0 ] == 'x' );
		REQUIRE( constStr == "xbc" );
	}

	SECTION( "Trim" )
	{
		ae::Str16 str = "abcdef";
		str.Trim( 10 );
		REQUIRE( str == "abcdef" );
		str.Trim( 6 );
		REQUIRE( str == "abcdef" );
		str.Trim( 3 );
		REQUIRE( str == "abc" );
		REQUIRE( str.Length() == 3 );
		REQUIRE( strlen( str.c_str() ) == 3 );
		str.Trim( 0 );
		REQUIRE( str.Empty() );
	}

	SECTION( "Capacity and Empty" )
	{
		REQUIRE( ae::Str16().Capacity() == ae::Str16::MaxLength() );
		REQUIRE( ae::Str256().Capacity() == ae::Str256::MaxLength() );
		REQUIRE( ae::Str16().Empty() );
		REQUIRE( !ae::Str16( "a" ).Empty() );
	}

	SECTION( "explicit const char*" )
	{
		const ae::Str16 str = "abc";
		REQUIRE( static_cast< const char* >( str ) == str.c_str() );
	}
}

TEST_CASE( "Str::Format", "[aeString]" )
{
	REQUIRE( ae::Str32::Format( "abc" ) == "abc" );
	REQUIRE( ae::Str32::Format( "" ).Empty() );
	REQUIRE( ae::Str32::Format( "#", 1 ) == "1" );
	REQUIRE( ae::Str32::Format( "a#b#c", 1, "x" ) == "a1bxc" );
	REQUIRE( ae::Str32::Format( "##", 1, 2 ) == "12" );
	REQUIRE( ae::Str32::Format( "# #", true, false ) == "true false" );
	REQUIRE( ae::Str32::Format( "#", 1.23456f ) == "1.235" );
	REQUIRE( ae::Str32::Format( "#", ae::Str16( "str" ) ) == "str" );
	REQUIRE( ae::Str32::Format( "#", std::string( "std" ) ) == "std" );
	REQUIRE( ae::Str32::Format( "a#b" ) == "a#b" );
	REQUIRE( ae::Str32::Format( "#", 1, 2 ) == "1" );
	REQUIRE( ae::Str32::Format( "#-#", 12, 345 ).Length() == 6 );
}

TEST_CASE( "Str streams", "[aeString]" )
{
	SECTION( "operator<<" )
	{
		std::ostringstream out;
		out << ae::Str16( "abc" ) << ae::Str64( "def" );
		REQUIRE( out.str() == "abcdef" );
	}

	SECTION( "operator>> reads lines" )
	{
		std::istringstream in( "abc\n\nde f\nlast" );
		ae::Str16 str = "start";
		in >> str;
		REQUIRE( str == "abc" );
		REQUIRE( str.Length() == 3 );
		in >> str;
		REQUIRE( str.Empty() );
		in >> str;
		REQUIRE( str == "de f" );
		REQUIRE( str.Length() == 4 );
		in >> str;
		REQUIRE( str == "last" );
		REQUIRE( !in.fail() );
	}

	SECTION( "operator>> max length line" )
	{
		std::istringstream in( "abcdefghijklm\nnext\n" );
		ae::Str16 str;
		in >> str;
		REQUIRE( str == "abcdefghijklm" );
		REQUIRE( str.Length() == ae::Str16::MaxLength() );
		REQUIRE( in.good() );
		in >> str;
		REQUIRE( str == "next" );
	}

	SECTION( "operator>> truncates long line" )
	{
		std::istringstream in( "abcdefghijklmn\n" );
		ae::Str16 str;
		in >> str;
		REQUIRE( in.fail() );
		REQUIRE( str == "abcdefghijklm" );
		REQUIRE( str.Length() == ae::Str16::MaxLength() );
	}
}

TEST_CASE( "Str length asserts", "[aeString]" )
{
	const char* const tooLong = "abcdefghijklmn";
	ae::Str16 str = "abc";
	AE_REQUIRE_THROWS( ae::Str16( tooLong ) );
	AE_REQUIRE_THROWS( ae::Str16( std::string( tooLong ) ) );
	AE_REQUIRE_THROWS( ae::Str16( ae::Str32( tooLong ) ) );
	AE_REQUIRE_THROWS( ae::Str16( 14u, tooLong ) );
	AE_REQUIRE_THROWS( ae::Str16( 14u, 'x' ) );
	AE_REQUIRE_THROWS( ae::Str16( tooLong, tooLong + 14 ) );
	AE_REQUIRE_THROWS( str = tooLong );
	AE_REQUIRE_THROWS( str += "defghijklmn" );
	AE_REQUIRE_THROWS( str + "defghijklmn" );
	AE_REQUIRE_THROWS( str.Append( std::string( "defghijklmn" ) ) );
	AE_REQUIRE_THROWS( str.Insert( 1, "defghijklmn" ) );
	AE_REQUIRE_THROWS( str.Insert( 4, "d" ) );
	AE_REQUIRE_THROWS( str[ 4 ] );
	REQUIRE_NOTHROW( str[ 3 ] );
	REQUIRE_NOTHROW( str += "defghijklm" );
	REQUIRE( str.Length() == ae::Str16::MaxLength() );
}
