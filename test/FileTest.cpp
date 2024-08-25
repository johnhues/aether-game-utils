//------------------------------------------------------------------------------
// FileTest.cpp
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
#include <catch2/catch_test_macros.hpp>
#if _AE_WINDOWS_
	// @NOTE: Disable a few warnings caused by catch2 that should not affect correctness
	#pragma warning( disable : 6319 )
	#pragma warning( disable : 6237 )
#endif

//------------------------------------------------------------------------------
// ae::FileSystem tests
//------------------------------------------------------------------------------
TEST_CASE( "Directory tests", "[ae::FileSystem]" )
{
	REQUIRE( !ae::FileSystem::IsDirectory( "" ) );
	REQUIRE( !ae::FileSystem::IsDirectory( "test" ) );
	REQUIRE( !ae::FileSystem::IsDirectory( "test/test" ) );

	REQUIRE( !ae::FileSystem::IsDirectory( ".ext" ) );
	REQUIRE( !ae::FileSystem::IsDirectory( "test.ext" ) );
	REQUIRE( !ae::FileSystem::IsDirectory( "test/test.ext" ) );

	REQUIRE( ae::FileSystem::IsDirectory( "/" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "test/" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "/test/" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "/test/test/test/" ) );

	REQUIRE( ae::FileSystem::IsDirectory( "\\" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "test\\" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "\\test\\" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "\\test\\test\\test\\" ) );

	REQUIRE( ae::FileSystem::IsDirectory( "C:\\" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "C:\\test\\" ) );
	REQUIRE( ae::FileSystem::IsDirectory( "C:\\test\\test\\test\\" ) );
}

TEST_CASE( "Replace extension", "[ae::FileSystem]" )
{
	SECTION( "Bad extension" )
	{
		ae::Str256 str = "test";
		REQUIRE( !ae::FileSystem::SetExtension( nullptr, "cpp" ) );
		REQUIRE( !ae::FileSystem::SetExtension( &str, nullptr ) );
		REQUIRE( str == "test" );
		REQUIRE( !ae::FileSystem::SetExtension( &str, "" ) );
		REQUIRE( str == "test" );
		REQUIRE( !ae::FileSystem::SetExtension( &str, ".cpp" ) );
		REQUIRE( str == "test" );
		REQUIRE( !ae::FileSystem::SetExtension( &str, "..cpp" ) );
		REQUIRE( str == "test" );
		REQUIRE( !ae::FileSystem::SetExtension( &str, "." ) );
		REQUIRE( str == "test" );
		REQUIRE( !ae::FileSystem::SetExtension( &str, ".c.pp" ) );
		REQUIRE( str == "test" );
		REQUIRE( !ae::FileSystem::SetExtension( &str, "c.pp" ) );
		REQUIRE( str == "test" );
	}

	SECTION( "File name" )
	{
		ae::Str256 noExt = "test";
		ae::Str256 dot = "test.";
		ae::Str256 shortExt = "test.a";
		ae::Str256 ext = "test.txt";
		ae::Str256 ext2Dots = "test..txt";
		ae::Str256 dir = "test/";
		REQUIRE( ae::FileSystem::SetExtension( &noExt, "cpp" ) );
		REQUIRE( noExt == "test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &dot, "cpp" ) );
		REQUIRE( dot == "test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &shortExt, "cpp" ) );
		REQUIRE( shortExt == "test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &ext, "cpp" ) );
		REQUIRE( ext == "test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &ext2Dots, "cpp" ) );
		REQUIRE( ext2Dots == "test..cpp" );
		REQUIRE( !ae::FileSystem::SetExtension( &dir, "cpp" ) );
		REQUIRE( dir == "test/" );
	}

	SECTION( "File path" )
	{
		ae::Str256 noExt = "something/1.1.1999/test";
		ae::Str256 dot = "something/1.1.1999/test.";
		ae::Str256 shortExt = "something/1.1.1999/test.a";
		ae::Str256 ext = "something/1.1.1999/test.txt";
		ae::Str256 ext2Dots = "something/1.1.1999/test..txt";
		ae::Str256 dir = "something/1.1.1999/test/";
		REQUIRE( ae::FileSystem::SetExtension( &noExt, "cpp" ) );
		REQUIRE( noExt == "something/1.1.1999/test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &dot, "cpp" ) );
		REQUIRE( dot == "something/1.1.1999/test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &shortExt, "cpp" ) );
		REQUIRE( shortExt == "something/1.1.1999/test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &ext, "cpp" ) );
		REQUIRE( ext == "something/1.1.1999/test.cpp" );
		REQUIRE( ae::FileSystem::SetExtension( &ext2Dots, "cpp" ) );
		REQUIRE( ext2Dots == "something/1.1.1999/test..cpp" );
		REQUIRE( !ae::FileSystem::SetExtension( &dir, "cpp" ) );
		REQUIRE( dir == "something/1.1.1999/test/" );
	}
}
