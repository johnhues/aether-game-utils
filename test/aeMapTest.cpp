//------------------------------------------------------------------------------
// aeMapTest.cpp
// Copyright (c) John Hughes on 4/20/21. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "catch2/catch.hpp"
#include "ae/aether.h"

//------------------------------------------------------------------------------
// aeMap tests
//------------------------------------------------------------------------------
TEST_CASE( "map elements can be set and retrieved", "[aeMap]" )
{
  ae::Map< int, char > map;
  for ( uint32_t i = 0; i < 10; i++ )
  {
    map.Set( i, 'a' + i );
  }
  REQUIRE( map.Length() == 10 );

  SECTION( "can retrieve previously set values" )
  {
    REQUIRE( map.Get( 0 )  == 'a' );
    REQUIRE( map.Get( 1 )  == 'b' );
    REQUIRE( map.Get( 2 )  == 'c' );
    REQUIRE( map.Get( 3 )  == 'd' );
    REQUIRE( map.Get( 4 )  == 'e' );
    REQUIRE( map.Get( 5 )  == 'f' );
    REQUIRE( map.Get( 6 )  == 'g' );
    REQUIRE( map.Get( 7 )  == 'h' );
    REQUIRE( map.Get( 8 )  == 'i' );
    REQUIRE( map.Get( 9 )  == 'j' );
  }

  SECTION( "existing values are correctly set" )
  {
    for ( uint32_t i = 0; i < 10; i++ )
    {
      map.Set( i, 'A' + i );
    }
    REQUIRE( map.Length() == 10 );
    
    REQUIRE( map.Get( 0 )  == 'A' );
    REQUIRE( map.Get( 1 )  == 'B' );
    REQUIRE( map.Get( 2 )  == 'C' );
    REQUIRE( map.Get( 3 )  == 'D' );
    REQUIRE( map.Get( 4 )  == 'E' );
    REQUIRE( map.Get( 5 )  == 'F' );
    REQUIRE( map.Get( 6 )  == 'G' );
    REQUIRE( map.Get( 7 )  == 'H' );
    REQUIRE( map.Get( 8 )  == 'I' );
    REQUIRE( map.Get( 9 )  == 'J' );
  }
}
