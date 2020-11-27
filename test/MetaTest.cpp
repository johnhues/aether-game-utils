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
#include "MetaTest.h"

//------------------------------------------------------------------------------
// PlayerState
//------------------------------------------------------------------------------

AE_ENUM_REGISTER( PlayerState );

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

TEST_CASE( "static enum helpers", "[aeMeta]" )
{
  REQUIRE( aeMeta::Enum::GetNameFromValue( PlayerState::Idle ) == "Idle" );
  REQUIRE( aeMeta::Enum::GetValueFromString( "Idle", (PlayerState)666 ) == PlayerState::Idle );
  REQUIRE( aeMeta::Enum::GetValueFromString( "0", (PlayerState)666 ) == PlayerState::Idle );

  REQUIRE( aeMeta::Enum::GetNameFromValue( PlayerState::Run ) == "Run" );
  REQUIRE( aeMeta::Enum::GetValueFromString( "Run", (PlayerState)666 ) == PlayerState::Run );
  REQUIRE( aeMeta::Enum::GetValueFromString( "1", (PlayerState)666 ) == PlayerState::Run );

  REQUIRE( aeMeta::Enum::GetNameFromValue( PlayerState::Jump ) == "Jump" );
  REQUIRE( aeMeta::Enum::GetValueFromString( "Jump", (PlayerState)666 ) == PlayerState::Jump );
  REQUIRE( aeMeta::Enum::GetValueFromString( "2", (PlayerState)666 ) == PlayerState::Jump );
}

TEST_CASE( "static enum helpers using missing values", "[aeMeta]" )
{
  REQUIRE( aeMeta::Enum::GetNameFromValue( (PlayerState)666 ) == "" );
  REQUIRE( aeMeta::Enum::GetValueFromString( "3", PlayerState::Jump ) == PlayerState::Jump );
  REQUIRE( aeMeta::Enum::GetValueFromString( "", PlayerState::Jump ) == PlayerState::Jump );
}

//------------------------------------------------------------------------------
// SomeClass
//------------------------------------------------------------------------------
AE_META_CLASS( SomeClass );
AE_META_VAR( SomeClass, intMember );
AE_META_VAR( SomeClass, enumTest );

AE_ENUM_REGISTER( TestEnumClass );

TEST_CASE( "can read enum values from object using meta definition", "[aeMeta]" )
{
  SomeClass c;
  const aeMeta::Type* type = aeMeta::GetTypeFromObject( &c );
  const aeMeta::Var* enumTestVar = type->GetVarByName( "enumTest" );
  
  c.enumTest = TestEnumClass::Five;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Five" );
  c.enumTest = TestEnumClass::Four;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Four" );
  c.enumTest = TestEnumClass::Three;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Three" );
  c.enumTest = TestEnumClass::Two;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Two" );
  c.enumTest = TestEnumClass::One;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "One" );
  c.enumTest = TestEnumClass::Zero;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Zero" );
  c.enumTest = TestEnumClass::NegativeOne;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "NegativeOne" );
}

TEST_CASE( "can't read invalid enum values from object using meta definition", "[aeMeta]" )
{
  SomeClass c;
  const aeMeta::Type* type = aeMeta::GetTypeFromObject( &c );
  const aeMeta::Var* enumTestVar = type->GetVarByName( "enumTest" );
  
  c.enumTest = (TestEnumClass)6;
  REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "" );
}

TEST_CASE( "can set enum values on object using meta definition", "[aeMeta]" )
{
  SomeClass c;
  const aeMeta::Type* type = aeMeta::GetTypeFromObject( &c );
  const aeMeta::Var* enumTestVar = type->GetVarByName( "enumTest" );
  
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "Five" ) );
  REQUIRE( c.enumTest == TestEnumClass::Five );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "Four" ) );
  REQUIRE( c.enumTest == TestEnumClass::Four );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "Three" ) );
  REQUIRE( c.enumTest == TestEnumClass::Three );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "Two" ) );
  REQUIRE( c.enumTest == TestEnumClass::Two );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "One" ) );
  REQUIRE( c.enumTest == TestEnumClass::One );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "Zero" ) );
  REQUIRE( c.enumTest == TestEnumClass::Zero );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "NegativeOne" ) );
  REQUIRE( c.enumTest == TestEnumClass::NegativeOne );
  
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "5" ) );
  REQUIRE( c.enumTest == TestEnumClass::Five );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "4" ) );
  REQUIRE( c.enumTest == TestEnumClass::Four );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "3" ) );
  REQUIRE( c.enumTest == TestEnumClass::Three );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "2" ) );
  REQUIRE( c.enumTest == TestEnumClass::Two );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "1" ) );
  REQUIRE( c.enumTest == TestEnumClass::One );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "0" ) );
  REQUIRE( c.enumTest == TestEnumClass::Zero );
  REQUIRE( enumTestVar->SetObjectValueFromString( &c, "-1" ) );
  REQUIRE( c.enumTest == TestEnumClass::NegativeOne );
}

TEST_CASE( "can't set invalid enum values on object using meta definition", "[aeMeta]" )
{
  SomeClass c;
  const aeMeta::Type* type = aeMeta::GetTypeFromObject( &c );
  const aeMeta::Var* enumTestVar = type->GetVarByName( "enumTest" );
  
  c.enumTest = TestEnumClass::Four;
  REQUIRE( !enumTestVar->SetObjectValueFromString( &c, "Six" ) );
  REQUIRE( c.enumTest == TestEnumClass::Four );
  REQUIRE( !enumTestVar->SetObjectValueFromString( &c, "6" ) );
  REQUIRE( c.enumTest == TestEnumClass::Four );
  REQUIRE( !enumTestVar->SetObjectValueFromString( &c, "" ) );
  REQUIRE( c.enumTest == TestEnumClass::Four );
}

//------------------------------------------------------------------------------
// SomeOldEnum
//------------------------------------------------------------------------------
AE_META_ENUM( SomeOldEnum );
AE_META_ENUM_VALUE( SomeOldEnum, Bleep );
AE_META_ENUM_VALUE( SomeOldEnum, Bloop );
AE_META_ENUM_VALUE( SomeOldEnum, Blop );

TEST_CASE( "can register an already existing c-style enum", "[aeMeta]" )
{
  const aeMeta::Enum* enumType = aeMeta::GetEnum< SomeOldEnum >();
  REQUIRE( enumType == aeMeta::GetEnum( "SomeOldEnum" ) );
  REQUIRE( enumType->Length() == 3 );
  REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
  REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
  REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
  REQUIRE( enumType->GetValueByIndex( 0 ) == 4 );
  REQUIRE( enumType->GetValueByIndex( 1 ) == 5 );
  REQUIRE( enumType->GetValueByIndex( 2 ) == 7 );
}

//------------------------------------------------------------------------------
// SomeOldPrefixEnum
//------------------------------------------------------------------------------
AE_META_ENUM_PREFIX( SomeOldPrefixEnum, kSomeOldPrefixEnum_ );
AE_META_ENUM_VALUE( SomeOldPrefixEnum, kSomeOldPrefixEnum_Bleep );
AE_META_ENUM_VALUE( SomeOldPrefixEnum, kSomeOldPrefixEnum_Bloop );
AE_META_ENUM_VALUE( SomeOldPrefixEnum, kSomeOldPrefixEnum_Blop );

TEST_CASE( "can register an already existing c-style enum where each value has a prefix", "[aeMeta]" )
{
  const aeMeta::Enum* enumType = aeMeta::GetEnum< SomeOldPrefixEnum >();
  REQUIRE( enumType == aeMeta::GetEnum( "SomeOldPrefixEnum" ) );
  REQUIRE( enumType->Length() == 3 );
  REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
  REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
  REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
  REQUIRE( enumType->GetValueByIndex( 0 ) == 4 );
  REQUIRE( enumType->GetValueByIndex( 1 ) == 5 );
  REQUIRE( enumType->GetValueByIndex( 2 ) == 7 );
}

//------------------------------------------------------------------------------
// SomeOldRenamedEnum
//------------------------------------------------------------------------------
AE_META_ENUM( SomeOldRenamedEnum );
AE_META_ENUM_VALUE_NAME( SomeOldRenamedEnum, BLEEP, Bleep );
AE_META_ENUM_VALUE_NAME( SomeOldRenamedEnum, BLOOP, Bloop );
AE_META_ENUM_VALUE_NAME( SomeOldRenamedEnum, BLOP, Blop );

TEST_CASE( "can register an already existing c-style enum where each value has a manually specified name", "[aeMeta]" )
{
  const aeMeta::Enum* enumType = aeMeta::GetEnum< SomeOldRenamedEnum >();
  REQUIRE( enumType == aeMeta::GetEnum( "SomeOldRenamedEnum" ) );
  REQUIRE( enumType->Length() == 3 );
  REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
  REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
  REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
  REQUIRE( enumType->GetValueByIndex( 0 ) == 4 );
  REQUIRE( enumType->GetValueByIndex( 1 ) == 5 );
  REQUIRE( enumType->GetValueByIndex( 2 ) == 7 );
}

//------------------------------------------------------------------------------
// SomeNewEnum
//------------------------------------------------------------------------------
AE_META_ENUM_CLASS( SomeNewEnum );
AE_META_ENUM_CLASS_VALUE( SomeNewEnum, Bleep );
AE_META_ENUM_CLASS_VALUE( SomeNewEnum, Bloop );
AE_META_ENUM_CLASS_VALUE( SomeNewEnum, Blop );

TEST_CASE( "can register an already existing enum class", "[aeMeta]" )
{
  const aeMeta::Enum* enumType = aeMeta::GetEnum< SomeNewEnum >();
  REQUIRE( enumType == aeMeta::GetEnum( "SomeNewEnum" ) );
  REQUIRE( enumType->Length() == 3 );
  REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
  REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
  REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
  REQUIRE( enumType->GetValueByIndex( 0 ) == 4 );
  REQUIRE( enumType->GetValueByIndex( 1 ) == 5 );
  REQUIRE( enumType->GetValueByIndex( 2 ) == 7 );
}

//------------------------------------------------------------------------------
// A::B::SomeNewEnum
//------------------------------------------------------------------------------
AE_META_ENUM_CLASS( A::B::SomeNewEnum );
AE_META_ENUM_CLASS_VALUE( A::B::SomeNewEnum, Bleep );
AE_META_ENUM_CLASS_VALUE( A::B::SomeNewEnum, Bloop );
AE_META_ENUM_CLASS_VALUE( A::B::SomeNewEnum, Blop );

TEST_CASE( "can register an already existing enum class in a nested namespace", "[aeMeta]" )
{
  const aeMeta::Enum* enumType = aeMeta::GetEnum< A::B::SomeNewEnum >();
  REQUIRE( enumType == aeMeta::GetEnum( "A::B::SomeNewEnum" ) );
  REQUIRE( enumType->Length() == 3 );
  REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
  REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
  REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
  REQUIRE( enumType->GetValueByIndex( 0 ) == 4 );
  REQUIRE( enumType->GetValueByIndex( 1 ) == 5 );
  REQUIRE( enumType->GetValueByIndex( 2 ) == 7 );
}
