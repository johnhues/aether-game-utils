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
#include "MetaTest.h"
#include "catch2/catch.hpp"

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
TEST_CASE( "Can get base type by name", "[aeMeta]" )
{
	REQUIRE( ae::GetTypeByName( "ae::Object" ) );
}

TEST_CASE( "Can get base type with templates", "[aeMeta]" )
{
	REQUIRE( ae::GetType< ae::Object >() );
}

//------------------------------------------------------------------------------
// PlayerState
//------------------------------------------------------------------------------
TEST_CASE( "enum registration", "[aeMeta]" )
{
	const ae::Enum* playerStateEnum = ae::GetEnum( "PlayerState" );
	REQUIRE( playerStateEnum->GetName() == ae::Str32( "PlayerState" ) );
	
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

TEST_CASE( "enum string conversions", "[aeMeta]" )
{
	const ae::Enum* playerStateEnum = ae::GetEnum( "PlayerState" );
	REQUIRE( playerStateEnum->GetName() == ae::Str32( "PlayerState" ) );

	REQUIRE( playerStateEnum->GetNameByValue( PlayerState::Idle ) == "Idle" );
	REQUIRE( playerStateEnum->GetValueFromString( "Idle", (PlayerState)666 ) == PlayerState::Idle );
	REQUIRE( playerStateEnum->GetValueFromString( "0", (PlayerState)666 ) == PlayerState::Idle );

	REQUIRE( playerStateEnum->GetNameByValue( PlayerState::Run ) == "Run" );
	REQUIRE( playerStateEnum->GetValueFromString( "Run", (PlayerState)666 ) == PlayerState::Run );
	REQUIRE( playerStateEnum->GetValueFromString( "1", (PlayerState)666 ) == PlayerState::Run );

	REQUIRE( playerStateEnum->GetNameByValue( PlayerState::Jump ) == "Jump" );
	REQUIRE( playerStateEnum->GetValueFromString( "Jump", (PlayerState)666 ) == PlayerState::Jump );
	REQUIRE( playerStateEnum->GetValueFromString( "2", (PlayerState)666 ) == PlayerState::Jump );

	REQUIRE( ae::ToString( PlayerState::Idle ) == "Idle" );
	REQUIRE( ae::FromString< PlayerState >( "Idle", (PlayerState)666 ) == PlayerState::Idle );
	REQUIRE( ae::FromString< PlayerState >( "0", (PlayerState)666 ) == PlayerState::Idle );

	REQUIRE( ae::ToString( PlayerState::Run ) == "Run" );
	REQUIRE( ae::FromString< PlayerState >( "Run", (PlayerState)666 ) == PlayerState::Run );
	REQUIRE( ae::FromString< PlayerState >( "1", (PlayerState)666 ) == PlayerState::Run );

	REQUIRE( ae::ToString( PlayerState::Jump ) == "Jump" );
	REQUIRE( ae::FromString< PlayerState >( "Jump", (PlayerState)666 ) == PlayerState::Jump );
	REQUIRE( ae::FromString< PlayerState >( "2", (PlayerState)666 ) == PlayerState::Jump );
}

TEST_CASE( "enum string conversions using missing values", "[aeMeta]" )
{
	const ae::Enum* playerStateEnum = ae::GetEnum( "PlayerState" );
	REQUIRE( playerStateEnum->GetName() == ae::Str32( "PlayerState" ) );

	REQUIRE( playerStateEnum->GetNameByValue( (PlayerState)666 ) == "" );
	REQUIRE( playerStateEnum->GetValueFromString( "3", PlayerState::Jump ) == PlayerState::Jump );
	REQUIRE( playerStateEnum->GetValueFromString( "", PlayerState::Jump ) == PlayerState::Jump );

	REQUIRE( ae::ToString( (PlayerState)666 ) == "" );
	REQUIRE( ae::FromString< PlayerState >( "3", PlayerState::Jump ) == PlayerState::Jump );
	REQUIRE( ae::FromString< PlayerState >( "", PlayerState::Jump ) == PlayerState::Jump );
}

//------------------------------------------------------------------------------
// SomeClass + TestEnumClass
//------------------------------------------------------------------------------
TEST_CASE( "can read enum values from object using meta definition", "[aeMeta]" )
{
	SomeClass c;
	const ae::Type* type = ae::GetTypeFromObject( &c );
	const ae::Var* enumTestVar = type->GetVarByName( "enumTest", false );
	
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
	const ae::Type* type = ae::GetTypeFromObject( &c );
	const ae::Var* enumTestVar = type->GetVarByName( "enumTest", false );
	
	c.enumTest = (TestEnumClass)6;
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "" );
}

TEST_CASE( "can set enum values on object using meta definition", "[aeMeta]" )
{
	SomeClass c;
	const ae::Type* type = ae::GetTypeFromObject( &c );
	const ae::Var* enumTestVar = type->GetVarByName( "enumTest", false );
	
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
	const ae::Type* type = ae::GetTypeFromObject( &c );
	const ae::Var* enumTestVar = type->GetVarByName( "enumTest", false );
	
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
	const ae::Enum* enumType = ae::GetEnum< SomeOldEnum >();
	REQUIRE( enumType == ae::GetEnum( "SomeOldEnum" ) );
	REQUIRE( enumType->Length() == 3 );
	REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
	REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
	REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
	REQUIRE( enumType->GetValueByIndex( 0 ) == 4 );
	REQUIRE( enumType->GetValueByIndex( 1 ) == 5 );
	REQUIRE( enumType->GetValueByIndex( 2 ) == 7 );
}

TEST_CASE( "existing c-style enum string conversions", "[aeMeta]" )
{
	const ae::Enum* enumType = ae::GetEnum< SomeOldEnum >();
	REQUIRE( enumType == ae::GetEnum( "SomeOldEnum" ) );

	REQUIRE( enumType->GetNameByValue( SomeOldEnum::Bleep ) == "Bleep" );
	REQUIRE( enumType->GetValueFromString( "Bleep", (SomeOldEnum)666 ) == SomeOldEnum::Bleep );
	REQUIRE( enumType->GetValueFromString( "4", (SomeOldEnum)666 ) == SomeOldEnum::Bleep );

	REQUIRE( enumType->GetNameByValue( SomeOldEnum::Bloop ) == "Bloop" );
	REQUIRE( enumType->GetValueFromString( "Bloop", (SomeOldEnum)666 ) == SomeOldEnum::Bloop );
	REQUIRE( enumType->GetValueFromString( "5", (SomeOldEnum)666 ) == SomeOldEnum::Bloop );

	REQUIRE( enumType->GetNameByValue( SomeOldEnum::Blop ) == "Blop" );
	REQUIRE( enumType->GetValueFromString( "Blop", (SomeOldEnum)666 ) == SomeOldEnum::Blop );
	REQUIRE( enumType->GetValueFromString( "7", (SomeOldEnum)666 ) == SomeOldEnum::Blop );
	
	REQUIRE( ae::ToString( SomeOldEnum::Bleep ) == "Bleep" );
	REQUIRE( ae::FromString< SomeOldEnum >( "Bleep", (SomeOldEnum)666 ) == SomeOldEnum::Bleep );
	REQUIRE( ae::FromString< SomeOldEnum >( "4", (SomeOldEnum)666 ) == SomeOldEnum::Bleep );

	REQUIRE( ae::ToString( SomeOldEnum::Bloop ) == "Bloop" );
	REQUIRE( ae::FromString< SomeOldEnum >( "Bloop", (SomeOldEnum)666 ) == SomeOldEnum::Bloop );
	REQUIRE( ae::FromString< SomeOldEnum >( "5", (SomeOldEnum)666 ) == SomeOldEnum::Bloop );

	REQUIRE( ae::ToString( SomeOldEnum::Blop ) == "Blop" );
	REQUIRE( ae::FromString< SomeOldEnum >( "Blop", (SomeOldEnum)666 ) == SomeOldEnum::Blop );
	REQUIRE( ae::FromString< SomeOldEnum >( "7", (SomeOldEnum)666 ) == SomeOldEnum::Blop );

	REQUIRE( enumType->GetNameByValue( (SomeOldEnum)666 ) == "" );
	REQUIRE( enumType->GetValueFromString( "3", SomeOldEnum::Bleep ) == SomeOldEnum::Bleep );
	REQUIRE( enumType->GetValueFromString( "", SomeOldEnum::Bleep ) == SomeOldEnum::Bleep );

	REQUIRE( ae::ToString( (SomeOldEnum)666 ) == "" );
	REQUIRE( ae::FromString< SomeOldEnum >( "3", SomeOldEnum::Bleep ) == SomeOldEnum::Bleep );
	REQUIRE( ae::FromString< SomeOldEnum >( "", SomeOldEnum::Bleep ) == SomeOldEnum::Bleep );
}

//------------------------------------------------------------------------------
// SomeOldPrefixEnum
//------------------------------------------------------------------------------
TEST_CASE( "can register an already existing c-style enum where each value has a prefix", "[aeMeta]" )
{
	const ae::Enum* enumType = ae::GetEnum< SomeOldPrefixEnum >();
	REQUIRE( enumType == ae::GetEnum( "SomeOldPrefixEnum" ) );
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
	const ae::Enum* enumType = ae::GetEnum< SomeOldRenamedEnum >();
	REQUIRE( enumType == ae::GetEnum( "SomeOldRenamedEnum" ) );
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
	const ae::Enum* enumType = ae::GetEnum< SomeNewEnum >();
	REQUIRE( enumType == ae::GetEnum( "SomeNewEnum" ) );
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
	const ae::Enum* enumType = ae::GetEnum< A::B::SomeNewEnum >();
	REQUIRE( enumType == ae::GetEnum( "A::B::SomeNewEnum" ) );
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
	ae::Delete( object );
}

RefTester* RefTesterManager::GetObjectById( uint32_t id )
{
	return id ? m_objectMap.Get( id, nullptr ) : nullptr;
}

TEST_CASE( "meta system can manipulate registered reference vars", "[aeMeta]" )
{
	const ae::Type* typeA = ae::GetType< RefTesterA >();
	REQUIRE( typeA );
	const ae::Var* typeA_notRef = typeA->GetVarByName( "notRef", false );
	const ae::Var* typeA_varA = typeA->GetVarByName( "refA", false );
	const ae::Var* typeA_varB = typeA->GetVarByName( "refB", false );
	REQUIRE( typeA_notRef );
	REQUIRE( typeA_varA );
	REQUIRE( typeA_varB );
	
	const ae::Type* typeB = ae::GetType< RefTesterB >();
	REQUIRE( typeB );
	const ae::Var* typeB_varA = typeB->GetVarByName( "refA", false );
	REQUIRE( typeB_varA );
	
	REQUIRE( !typeA_notRef->GetSubType() );
	REQUIRE( typeA_varA->GetSubType() == typeA );
	REQUIRE( typeA_varB->GetSubType() == typeB );
	REQUIRE( typeB_varA->GetSubType() == typeA );
	
	class RefSerializer : public ae::Var::Serializer
	{
	public:
		RefSerializer( RefTesterManager* manager ) : m_manager( manager ) {}
		std::string ObjectPointerToString( const ae::Object* obj ) const override
		{
			return RefTester::GetIdString( ae::Cast< RefTester >( obj ) );
		}
		bool StringToObjectPointer( const char* pointerVal, ae::Object** objOut ) const override
		{
			uint32_t id = 0;
			if ( RefTester::StringToId( pointerVal, &id ) )
			{
				*objOut = m_manager->GetObjectById( id );
				return true;
			}
			return false;
		}
	private:
		RefTesterManager* m_manager;
	};
	RefTesterManager manager;
	RefSerializer refSerializer = &manager;
	ae::Var::SetSerializer( &refSerializer );
	
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
	REQUIRE( testerA1->notRef == 0xfdfdfdfd );
	REQUIRE( testerA1->refA == nullptr );
	REQUIRE( testerA1->refB == nullptr );
	REQUIRE( testerA2->notRef == 0xfdfdfdfd );
	REQUIRE( testerA2->refA == nullptr );
	REQUIRE( testerA2->refB == nullptr );
	REQUIRE( testerB3->refA == nullptr );
	REQUIRE( typeA_varA->GetObjectValueAsString( testerA1 ) == "0" );
	REQUIRE( typeA_varA->GetObjectValueAsString( testerA2 ) == "0" );
	REQUIRE( typeB_varA->GetObjectValueAsString( testerB3 ) == "0" );
	
	// Set type A's ref to type A
	REQUIRE( typeA_varA->SetObjectValueFromString( testerA1, "2" ) );
	REQUIRE( testerA1->notRef == 0xfdfdfdfd );
	REQUIRE( testerA1->refA == testerA2 );
	REQUIRE( testerA1->refB == nullptr );
	REQUIRE( testerA2->notRef == 0xfdfdfdfd );
	REQUIRE( testerA2->refA == nullptr );
	REQUIRE( testerA2->refB == nullptr );
	REQUIRE( testerB3->refA == nullptr );
	
	// Set type B's ref to type A
	REQUIRE( typeB_varA->SetObjectValueFromString( testerB3, "2" ) );
	REQUIRE( testerA1->notRef == 0xfdfdfdfd );
	REQUIRE( testerA1->refA == testerA2 );
	REQUIRE( testerA1->refB == nullptr );
	REQUIRE( testerA2->notRef == 0xfdfdfdfd );
	REQUIRE( testerA2->refA == nullptr );
	REQUIRE( testerA2->refB == nullptr );
	REQUIRE( testerB3->refA == testerA2 );
	
	REQUIRE( typeA_varA->GetObjectValueAsString( testerA1 ) == "2" );
	REQUIRE( typeA_varA->GetObjectValueAsString( testerA2 ) == "0" );
	REQUIRE( typeB_varA->GetObjectValueAsString( testerB3 ) == "2" );
	
	// Set type A's ref B to type B
	REQUIRE( typeA_varB->SetObjectValueFromString( testerA2, "3" ) );
	REQUIRE( testerA1->notRef == 0xfdfdfdfd );
	REQUIRE( testerA1->refA == testerA2 );
	REQUIRE( testerA1->refB == nullptr );
	REQUIRE( testerA2->notRef == 0xfdfdfdfd );
	REQUIRE( testerA2->refA == nullptr );
	REQUIRE( testerA2->refB == testerB3 );
	REQUIRE( testerB3->refA == testerA2 );
	
	// Setting type A ref A to type B should do nothing
	REQUIRE( !typeA_varA->SetObjectValueFromString( testerA1, "3" ) );
	REQUIRE( testerA1->notRef == 0xfdfdfdfd );
	REQUIRE( testerA1->refA == testerA2 );
	REQUIRE( testerA1->refB == nullptr );
	REQUIRE( testerA2->notRef == 0xfdfdfdfd );
	REQUIRE( testerA2->refA == nullptr );
	REQUIRE( testerA2->refB == testerB3 );
	REQUIRE( testerB3->refA == testerA2 );
	
	// Setting ref from random string value does nothing
	REQUIRE( !typeA_varA->SetObjectValueFromString( testerA1, "qwerty" ) );
	REQUIRE( testerA1->notRef == 0xfdfdfdfd );
	REQUIRE( testerA1->refA == testerA2 );
	REQUIRE( testerA1->refB == nullptr );
	REQUIRE( testerA2->notRef == 0xfdfdfdfd );
	REQUIRE( testerA2->refA == nullptr );
	REQUIRE( testerA2->refB == testerB3 );
	REQUIRE( testerB3->refA == testerA2 );
	
	// Setting ref to null value succeeds and clears ref
	REQUIRE( typeA_varA->SetObjectValueFromString( testerA1, "0" ) );
	REQUIRE( testerA1->notRef == 0xfdfdfdfd );
	REQUIRE( testerA1->refA == nullptr );
	REQUIRE( testerA1->refB == nullptr );
	REQUIRE( testerA2->notRef == 0xfdfdfdfd );
	REQUIRE( testerA2->refA == nullptr );
	REQUIRE( testerA2->refB == testerB3 );
	REQUIRE( testerB3->refA == testerA2 );
	
	manager.Destroy( testerA1 );
	manager.Destroy( testerA2 );
	manager.Destroy( testerB3 );
}

TEST_CASE( "bitfield registration", "[aeMeta]" )
{
	const ae::Enum* gamePadBitFieldEnum = ae::GetEnum( "GamePadBitField" );

	REQUIRE( gamePadBitFieldEnum->GetName() == ae::Str32( "GamePadBitField" ) );
	REQUIRE( gamePadBitFieldEnum->GetValueFromString( "A", (GamePadBitField)666 ) == GamePadBitField::A );
	REQUIRE( gamePadBitFieldEnum->GetValueFromString( "1", (GamePadBitField)666 ) == GamePadBitField::A );

	uint16_t something = GamePadBitField::Y | GamePadBitField::X;
	something = something | GamePadBitField::A;
	something = GamePadBitField::B | something;
	//something |= GamePadBitField::B;
	
	// REQUIRE( playerStateEnum->TypeSize() == 2 );
	// REQUIRE( playerStateEnum->TypeIsSigned() == false );
	
	// REQUIRE( playerStateEnum->Length() == 3 );
	
	// REQUIRE( playerStateEnum->GetNameByIndex( 0 ) == "Idle" );
	// REQUIRE( playerStateEnum->GetValueByIndex( 0 ) == 0 );
	
	// REQUIRE( playerStateEnum->GetNameByIndex( 1 ) == "Run" );
	// REQUIRE( playerStateEnum->GetValueByIndex( 1 ) == 1 );
	
	// REQUIRE( playerStateEnum->GetNameByIndex( 2 ) == "Jump" );
	// REQUIRE( playerStateEnum->GetValueByIndex( 2 ) == 2 );
}
