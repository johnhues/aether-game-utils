//------------------------------------------------------------------------------
// LogTest.cpp
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
// Static variables
//------------------------------------------------------------------------------
static int32_t s_runningTestIndex = -1;

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void TestLogger( ae::LogSeverity severity, const char* filePath, uint32_t line, const char** tags, uint32_t tagCount, const char* message )
{
#if AE_ENABLE_SOURCE_INFO
	REQUIRE( filePath[ 0 ] );
	REQUIRE( line >= 0 );
#else
	REQUIRE( !filePath[ 0 ] );
	REQUIRE( line == 0 );
#endif
	switch( s_runningTestIndex )
	{
		case 0:
		{
			REQUIRE( ae::Str32( "Test [0]" ) == message );
			REQUIRE( severity == ae::LogSeverity::Info );
			REQUIRE( tagCount == 0 );
			break;
		}
		case 1:
		{
			REQUIRE( ae::Str32( "Test [1]" ) == message );
			REQUIRE( severity == ae::LogSeverity::Debug );
			REQUIRE( tagCount == 1 );
			REQUIRE( ae::Str32( "TestTag" ) == tags[ 0 ] );
			break;
		}
		case 2:
		{
			REQUIRE( ae::Str32( "Test [2]" ) == message );
			REQUIRE( severity == ae::LogSeverity::Warning );
			REQUIRE( tagCount == 2 );
			REQUIRE( ae::Str32( "TestTag0" ) == tags[ 0 ] );
			REQUIRE( ae::Str32( "TestTag1" ) == tags[ 1 ] );
			break;
		}
		case 3:
		{
			REQUIRE( ae::Str32( "Test [3]" ) == message );
			REQUIRE( severity == ae::LogSeverity::Error );
			REQUIRE( tagCount == 1 );
			REQUIRE( ae::Str32( "TestTag0" ) == tags[ 0 ] );
			break;
		}
		case 4:
		{
			REQUIRE( ae::Str64( "AE_ASSERT_MSG( false ) Test [4]" ) == message );
			REQUIRE( severity == ae::LogSeverity::Fatal );
			REQUIRE( tagCount == 1 );
			REQUIRE( ae::Str32( "AssertTag" ) == tags[ 0 ] );
			break;
		}
		default:
			break;
	}
}

TEST_CASE( "Log message tagging", "[ae::Log]" )
{
	s_runningTestIndex++;
	AE_INFO( "Test [#]", s_runningTestIndex );
	
	ae::PushLogTag( "TestTag" );
	s_runningTestIndex++;
	AE_DEBUG( "Test [#]", s_runningTestIndex );
	ae::PopLogTag();

	ae::PushLogTag( "TestTag0" );
	ae::PushLogTag( "TestTag1" );
	s_runningTestIndex++;
	AE_WARN( "Test [#]", s_runningTestIndex );
	ae::PopLogTag();
	s_runningTestIndex++;
	AE_ERR( "Test [#]", s_runningTestIndex );
	ae::PopLogTag();
	
	s_runningTestIndex++;
	ae::PushLogTag( "AssertTag" );
	REQUIRE_THROWS( [](){ AE_ASSERT_MSG( false, "Test [#]", s_runningTestIndex ); }() );
	ae::PopLogTag();

	s_runningTestIndex = -1;
}
