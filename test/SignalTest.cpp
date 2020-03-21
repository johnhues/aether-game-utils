//------------------------------------------------------------------------------
// SignalTest.cpp
// Copyright (c) John Hughes on 12/2/19. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "catch2/catch.hpp"
#include "aeSignal.h"

//------------------------------------------------------------------------------
// Test helpers
//------------------------------------------------------------------------------
class Thing
{
public:
  Thing() : m_refable( this ) {}
  AE_REFABLE( Thing );

  void Fn();
  void FnInt( int i );

  int val = 0;
  int callCount = 0;
};

void Thing::Fn()
{
  val = 1;
  callCount++;
}

void Thing::FnInt( int i )
{
  val = i;
  callCount++;
}

//------------------------------------------------------------------------------
// aeSignalList tests
//------------------------------------------------------------------------------
TEST_CASE( "signal send should result in the correct functions being called", "[aeSignal]" )
{
  aeSignalList< int > signal;
  REQUIRE( signal.Length() == 0 );

  Thing thing;
  Thing thingInt;
  REQUIRE( thing.val == 0 );
  REQUIRE( thingInt.val == 0 );

  SECTION( "sending without value should call correct function" )
  {
    signal.Add( &thing, &Thing::Fn );
    signal.Send();
    REQUIRE( thing.val == 1 );
  }

  SECTION( "sending with value should pass value to function" )
  {
    signal.Add( &thing, &Thing::Fn );
    signal.Add( &thingInt, &Thing::FnInt );
    signal.Send( 2 );
    REQUIRE( thing.val == 1 );
    REQUIRE( thingInt.val == 2 );
  }

  SECTION( "adding object twice should result in only single function call" )
  {
    REQUIRE( thing.callCount == 0 );
    signal.Add( &thing, &Thing::Fn );
    signal.Add( &thing, &Thing::Fn );
    REQUIRE( signal.Length() == 1 );
    signal.Send();
    REQUIRE( thing.callCount == 1 );
  }

  // @TODO: Add two objects by reference
  // @TODO: Add one object by reference the other by pointer

  SECTION( "removing pointer object from signal should result in no function call" )
  {
    REQUIRE( thing.callCount == 0 );
    signal.Add( &thing, &Thing::Fn );
    signal.Send();
    REQUIRE( thing.callCount == 1 );
    signal.Remove( &thing );
    signal.Send();
    REQUIRE( thing.callCount == 1 );
  }

  // @TODO: Remove reference by reference test
  // @TODO: Remove reference by pointer test

  SECTION( "null pointers to objects should not be added" )
  {
    Thing* thingP = nullptr;
    REQUIRE( signal.Length() == 0 );
    signal.Add( thingP, &Thing::Fn );
    REQUIRE( signal.Length() == 0 );
  }

  SECTION( "empty references to objects should not be added" )
  {
    aeRef< Thing > thingRef;
    REQUIRE( signal.Length() == 0 );
    signal.Add( thingRef, &Thing::Fn );
    REQUIRE( signal.Length() == 0 );
  }

  SECTION( "destroying referenced object should result in removal on send" )
  {
    Thing* thingP = aeAlloc::Allocate< Thing >();
    aeRef< Thing > thingRef( thingP );
    
    signal.Add( thingRef, &Thing::Fn );

    aeAlloc::Release( thingP );
    signal.Send();

    REQUIRE( signal.Length() == 0 );
  }
}
