//------------------------------------------------------------------------------
// MetaTest.cpp
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
#include "aeMeta.h"

//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------
AE_ENUM( TestEnumClass, int32_t,
  NegativeOne = -1,
  Zero,
  One,
  Two = 0x02,
  Three = 0x03,
  Four,
  Five
);

AE_ENUM( PlayerState, uint16_t,
  Idle,
  Run,
  Jump
);

class SomeClass : public aeInheritor< aeObject, SomeClass >
{
public:
  int32_t intMember;
  TestEnumClass enumTest;
};
AE_META_CLASS( SomeClass );
AE_META_VAR( SomeClass, intMember );
AE_META_VAR( SomeClass, enumTest );

//------------------------------------------------------------------------------
// Test
//------------------------------------------------------------------------------
TEST_CASE( "enum registration", "[aeMeta]" )
{
  const aeMeta::Enum* playerStateEnum = aeMeta::GetEnum( "PlayerState" );
  
  REQUIRE( playerStateEnum->GetName() == aeStr32( "PlayerState" ) );
  
  REQUIRE( playerStateEnum->TypeSize() == 2 );
  REQUIRE( playerStateEnum->TypeIsSigned() == false );
  
  REQUIRE( playerStateEnum->Length() == 3 );
  
  REQUIRE( playerStateEnum->GetNameByIndex( 0 ) == "Idle" );
  REQUIRE( playerStateEnum->GetValueByIndex( 0 ) == 0 );
  
  REQUIRE( playerStateEnum->GetNameByIndex( 1 ) == "Run" );
  REQUIRE( playerStateEnum->GetValueByIndex( 1 ) == 1 );
  
  REQUIRE( playerStateEnum->GetNameByIndex( 2 ) == "Jump" );
  REQUIRE( playerStateEnum->GetValueByIndex( 2 ) == 2 );
}

TEST_CASE( "can read enum values from object using meta definition", "[aeMeta]" )
{
  SomeClass c0;
  const aeMeta::Type* type = aeMeta::GetTypeFromObject( &c0 );
  const aeMeta::Var* enumTestVar = type->GetVarByName( "enumTest" );
  
  c0.enumTest = TestEnumClass::Five;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "Five" );
  c0.enumTest = TestEnumClass::Four;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "Four" );
  c0.enumTest = TestEnumClass::Three;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "Three" );
  c0.enumTest = TestEnumClass::Two;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "Two" );
  c0.enumTest = TestEnumClass::One;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "One" );
  c0.enumTest = TestEnumClass::Zero;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "Zero" );
  c0.enumTest = TestEnumClass::NegativeOne;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "NegativeOne" );
  
  c0.enumTest = (TestEnumClass)6;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c0 ) == "" );
}

TEST_CASE( "can set enum values on object using meta definition", "[aeMeta]" )
{
  SomeClass c0;
  const aeMeta::Type* type = aeMeta::GetTypeFromObject( &c0 );
  const aeMeta::Var* enumTestVar = type->GetVarByName( "enumTest" );
  
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "Five" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Five );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "Four" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Four );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "Three" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Three );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "Two" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Two );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "One" ) );
  REQUIRE( c0.enumTest == TestEnumClass::One );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "Zero" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Zero );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "NegativeOne" ) );
  REQUIRE( c0.enumTest == TestEnumClass::NegativeOne );
  
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "5" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Five );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "4" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Four );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "3" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Three );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "2" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Two );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "1" ) );
  REQUIRE( c0.enumTest == TestEnumClass::One );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "0" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Zero );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c0, "-1" ) );
  REQUIRE( c0.enumTest == TestEnumClass::NegativeOne );
  
  c0.enumTest = TestEnumClass::Four;
  REQUIRE( !enumTestVar->SetObjectValueFromString( &c0, "Six" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Four );
  REQUIRE( !enumTestVar->SetObjectValueFromString( &c0, "6" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Four );
  REQUIRE( !enumTestVar->SetObjectValueFromString( &c0, "" ) );
  REQUIRE( c0.enumTest == TestEnumClass::Four );
}
