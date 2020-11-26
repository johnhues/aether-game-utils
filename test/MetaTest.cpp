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

enum SomeOldEnum
{
  Bleep = 4,
  Bloop,
  Blop = 7
};
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

enum SomeOldPrefixEnum
{
  kSomeOldPrefixEnum_Bleep = 4,
  kSomeOldPrefixEnum_Bloop,
  kSomeOldPrefixEnum_Blop = 7
};
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

enum SomeOldRenamedEnum
{
  BLEEP = 4,
  BLOOP,
  BLOP = 7
};
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

enum class SomeNewEnum
{
  Bleep = 4,
  Bloop,
  Blop = 7
};
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

namespace A
{
  namespace B
  {
    enum class SomeNewEnum
    {
      Bleep = 4,
      Bloop,
      Blop = 7
    };
  }
}
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
