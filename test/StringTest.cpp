//------------------------------------------------------------------------------
// StringTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
#include "catch2/catch.hpp"
#include "aeString.h"

#if _AE_WINDOWS_
  // @NOTE: Disable a few warnings caused by catch2 that should not affect correctness
  #pragma warning( disable : 6319 )
  #pragma warning( disable : 6237 )
#endif

//------------------------------------------------------------------------------
// aeString tests
//------------------------------------------------------------------------------
TEST_CASE( "strings of the same length can be compared alphabetically", "[aeString]" )
{
  aeStr32 str0 = "abc";
  aeStr32 str1 = "xyz";
  const char* str2 = "abc";
  aeStr32 str3 = "xyz";
  aeStr32 str4 = "abc";
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
  aeStr32 str0 = "xyz";
  aeStr32 str1 = "abcd";
  const char* str2 = "xyz";
  aeStr32 str3 = "abcd";
  aeStr32 str4 = "xyz";
  const char* str5 = "abcd";

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
