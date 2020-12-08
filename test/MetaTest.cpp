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
// SomeClass + TestEnumClass
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Reference testing
//------------------------------------------------------------------------------
std::string RefTester::GetIdString( const RefTester* obj )
{
  return obj ? std::to_string( obj->id ) : std::string( "0" );
}

bool RefTester::StringToId( const char* str, uint32_t* idOut )
{
  char* endPtr = nullptr;
  uint32_t id = strtoul( str, &endPtr, 10 );
  if ( endPtr != str )
  {
    *idOut = id;
    return true;
  }
  return false;
}
void RefTesterManager::Destroy( RefTester* object )
{
  m_objectMap.Remove( object->id );
  aeAlloc::Release( object );
}

RefTester* RefTesterManager::GetObjectById( uint32_t id )
{
  return id ? m_objectMap.Get( id, nullptr ) : nullptr;
}

TEST_CASE( "meta system can manipulate registered reference vars", "[aeMeta]" )
{
  const aeMeta::Type* typeA = aeMeta::GetType< RefTesterA >();
  const aeMeta::Var* varA = typeA->GetVarByName( "ref" );
  
  const aeMeta::Type* typeB = aeMeta::GetType< RefTesterB >();
  const aeMeta::Var* varB = typeB->GetVarByName( "ref" );
  
  RefTesterManager manager;
  auto refStrToValFn = [&]( const char* str, aeObject** objOut )
  {
    uint32_t id = 0;
    if ( RefTester::StringToId( str, &id ) )
    {
      *objOut = manager.GetObjectById( id );
      return true;
    }
    return false;
  };
  auto refValToStrFn = []( const aeObject* o )
  {
    return RefTester::GetIdString( aeCast< RefTester >( o ) );
  };
  
  RefTesterA* testerA1 = manager.Create< RefTesterA >();
  RefTesterA* testerA2 = manager.Create< RefTesterA >();
  RefTesterB* testerB3 = manager.Create< RefTesterB >();
  
  // Validate ids
  REQUIRE( testerA1->id == 1 );
  REQUIRE( testerA2->id == 2 );
  REQUIRE( testerB3->id == 3 );
  REQUIRE( RefTester::GetIdString( testerA1 ) == "1" );
  REQUIRE( RefTester::GetIdString( testerA2 ) == "2" );
  REQUIRE( RefTester::GetIdString( testerB3 ) == "3" );
  
  // Validate initial reference values
  REQUIRE( testerA1->ref == nullptr );
  REQUIRE( testerA2->ref == nullptr );
  REQUIRE( testerB3->ref == nullptr );
  REQUIRE( varA->GetObjectValueAsString( testerA1, refValToStrFn ) == "0" );
  REQUIRE( varA->GetObjectValueAsString( testerA2, refValToStrFn ) == "0" );
  REQUIRE( varB->GetObjectValueAsString( testerB3, refValToStrFn ) == "0" );
  
  // Set ref of type A to type A
  REQUIRE( varA->SetObjectValueFromString( testerA1, "2", refStrToValFn ) );
  REQUIRE( testerA1->ref == testerA2 );
  REQUIRE( testerA2->ref == nullptr );
  REQUIRE( testerB3->ref == nullptr );
  
  // Set ref of type B to type A
  REQUIRE( varB->SetObjectValueFromString( testerB3, "2", refStrToValFn ) );
  REQUIRE( testerA1->ref == testerA2 );
  REQUIRE( testerA2->ref == nullptr );
  REQUIRE( testerB3->ref == testerA2 );
  
  REQUIRE( varA->GetObjectValueAsString( testerA1, refValToStrFn ) == "2" );
  REQUIRE( varA->GetObjectValueAsString( testerA2, refValToStrFn ) == "0" );
  REQUIRE( varB->GetObjectValueAsString( testerB3, refValToStrFn ) == "2" );
  
  // Setting ref to object of wrong type fails and does nothing
  REQUIRE( !varB->SetObjectValueFromString( testerA1, "3", refStrToValFn ) );
  REQUIRE( testerA1->ref == testerA2 );
  REQUIRE( testerA2->ref == nullptr );
  REQUIRE( testerB3->ref == testerA2 );
  
  // Setting ref from random string value does nothing
  REQUIRE( !varB->SetObjectValueFromString( testerA1, "qwerty", refStrToValFn ) );
  REQUIRE( testerA1->ref == testerA2 );
  REQUIRE( testerA2->ref == nullptr );
  REQUIRE( testerB3->ref == testerA2 );
  
  // Setting ref to null value succeeds and clears ref
  REQUIRE( varB->SetObjectValueFromString( testerA1, "0", refStrToValFn ) );
  REQUIRE( testerA1->ref == nullptr );
  REQUIRE( testerA2->ref == nullptr );
  REQUIRE( testerB3->ref == testerA2 );
  
  manager.Destroy( testerA1 );
  manager.Destroy( testerA2 );
  manager.Destroy( testerB3 );
}
