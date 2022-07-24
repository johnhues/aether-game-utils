//------------------------------------------------------------------------------
// SignalTest.cpp
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
#include "aeSignal.h"
#include "catch2/catch.hpp"

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

namespace ae
{
  template <>
  uint32_t GetHash< aeId< Thing > >( aeId< Thing > key )
  {
    return key.GetInternalId();
  }
}

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
    Thing* thingP = ae::New< Thing >( AE_ALLOC_TAG_FIXME );
    aeRef< Thing > thingRef( thingP );
    
    signal.Add( thingRef, &Thing::Fn );

    ae::Delete( thingP );
    signal.Send();

    REQUIRE( signal.Length() == 0 );
  }
}
