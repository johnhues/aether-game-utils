//------------------------------------------------------------------------------
// MetaTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2024 John Hughes
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
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
TEST_CASE( "Can get base type by name", "[aeMeta]" )
{
	const ae::Type* objType = ae::GetType< ae::Object >();
	const ae::Str32 objTypeName = objType->GetName();
	REQUIRE( objType );
	REQUIRE( objTypeName == "ae::Object" );
	REQUIRE( ae::GetTypeByName( "ae::Object" ) == objType );

	REQUIRE( ae::GetTypeName< ae::Object >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::Object* >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::Object& >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::Object[] >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< const ae::Object >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< const ae::Object* >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< const ae::Object& >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< const ae::Object[] >() == ae::Str32( "ae::Object" ) );
}

TEST_CASE( "Can get base type with templates", "[aeMeta]" )
{
	REQUIRE( ae::GetType< ae::Object >() );
}

TEST_CASE( "Class registration", "[aeMeta]" )
{
	REQUIRE( ae::GetType< SomeClass >() );
	REQUIRE( ae::GetTypeByName( "SomeClass" ) );
	REQUIRE( ae::GetType< SomeClass >() == ae::GetTypeByName( "SomeClass" ) );
}

TEST_CASE( "Class properties", "[aeMeta]" )
{
	const ae::Type* type = ae::GetType< SomeClass >();
	REQUIRE( type );
	REQUIRE( type->GetPropertyCount() == 3 );
	
	REQUIRE( type->HasProperty( "someProp0" ) );
	REQUIRE( type->GetPropertyIndex( "someProp0" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyName( 0 ), "someProp0" ) == 0 );
	REQUIRE( type->GetPropertyValueCount( 0 ) == 0 );
	REQUIRE( type->GetPropertyValueCount( "someProp0" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( 0, 0 ), "" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp0", 0 ), "" ) == 0 );

	REQUIRE( type->HasProperty( "someProp1" ) );
	REQUIRE( type->GetPropertyIndex( "someProp1" ) == 1 );
	REQUIRE( strcmp( type->GetPropertyName( 1 ), "someProp1" ) == 0 );
	REQUIRE( type->GetPropertyValueCount( 1 ) == 2 );
	REQUIRE( type->GetPropertyValueCount( "someProp1" ) == 2 );
	REQUIRE( strcmp( type->GetPropertyValue( 1, 0 ), "v0" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp1", 0 ), "v0" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( 1, 1 ), "v1" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp1", 1 ), "v1" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( 1, 2 ), "" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp1", 2 ), "" ) == 0 );

	REQUIRE( type->HasProperty( "someProp2" ) );
	REQUIRE( type->GetPropertyIndex( "someProp2" ) == 2 );
	REQUIRE( strcmp( type->GetPropertyName( 2 ), "someProp2" ) == 0 );
	REQUIRE( type->GetPropertyValueCount( 2 ) == 3 );
	REQUIRE( type->GetPropertyValueCount( "someProp2" ) == 3 );
	REQUIRE( strcmp( type->GetPropertyValue( 2, 0 ), "v0" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp2", 0 ), "v0" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( 2, 1 ), "v1" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp2", 1 ), "v1" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( 2, 2 ), "v2" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp2", 2 ), "v2" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( 2, 3 ), "" ) == 0 );
	REQUIRE( strcmp( type->GetPropertyValue( "someProp2", 3 ), "" ) == 0 );

	REQUIRE( !type->HasProperty( "someProp3" ) );
	REQUIRE( type->GetPropertyIndex( "someProp3" ) == -1 );
	REQUIRE_THROWS( type->GetPropertyName( 3 ) );
	REQUIRE_THROWS( type->GetPropertyValueCount( 3 ) );
}

TEST_CASE( "Class vars", "[aeMeta]" )
{
	const ae::Type* type = ae::GetType< SomeClass >();
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) );

	REQUIRE( type->GetVarByName( "intMember", false ) );
	REQUIRE( type->GetVarByIndex( 0, false ) );
	REQUIRE( type->GetVarByName( "intMember", false ) == type->GetVarByIndex( 0, false ) );
	const ae::Var* intVar = type->GetVarByName( "intMember", false );
	REQUIRE( intVar->HasProperty( "intProp" ) );
	REQUIRE( intVar->GetPropertyIndex( "intProp" ) == 0 );
	REQUIRE( intVar->GetPropertyCount() == 1 );
	REQUIRE( strcmp( intVar->GetPropertyName( 0 ), "intProp" ) == 0 );
	REQUIRE( intVar->GetPropertyValueCount( 0 ) == 0 );
	REQUIRE( intVar->GetPropertyValueCount( "intProp" ) == 0 );

	REQUIRE( type->GetVarByName( "boolMember", false ) );
	REQUIRE( type->GetVarByIndex( 1, false ) );
	REQUIRE( type->GetVarByName( "boolMember", false ) == type->GetVarByIndex( 1, false ) );
	const ae::Var* boolVar = type->GetVarByName( "boolMember", false );
	REQUIRE( boolVar->HasProperty( "boolProp" ) );
	REQUIRE( boolVar->GetPropertyIndex( "boolProp" ) == 0 );
	REQUIRE( boolVar->GetPropertyCount() == 1 );
	REQUIRE( strcmp( boolVar->GetPropertyName( 0 ), "boolProp" ) == 0 );
	REQUIRE( boolVar->GetPropertyValueCount( 0 ) == 1 );
	REQUIRE( boolVar->GetPropertyValueCount( "boolProp" ) == 1 );
	REQUIRE( strcmp( boolVar->GetPropertyValue( 0, 0 ), "val" ) == 0 );
	REQUIRE( strcmp( boolVar->GetPropertyValue( "boolProp", 0 ), "val" ) == 0 );

	REQUIRE( type->GetVarByName( "enumTest", false ) );
	REQUIRE( type->GetVarByIndex( 2, false ) );
	REQUIRE( type->GetVarByName( "enumTest", false ) == type->GetVarByIndex( 2, false ) );
	const ae::Var* enumVar = type->GetVarByName( "enumTest", false );
	REQUIRE( enumVar->GetPropertyCount() == 2 );
	
	// prop0
	REQUIRE( enumVar->HasProperty( "prop0" ) );
	REQUIRE( enumVar->GetPropertyIndex( "prop0" ) == 0 );
	REQUIRE( strcmp( enumVar->GetPropertyName( 0 ), "prop0" ) == 0 );
	REQUIRE( enumVar->GetPropertyValueCount( 0 ) == 1 );
	REQUIRE( enumVar->GetPropertyValueCount( "prop0" ) == 1 );

	REQUIRE( strcmp( enumVar->GetPropertyValue( 0, 0 ), "val0" ) == 0 );
	REQUIRE( strcmp( enumVar->GetPropertyValue( "prop0", 0 ), "val0" ) == 0 );

	// prop1
	REQUIRE( enumVar->HasProperty( "prop1" ) );
	REQUIRE( enumVar->GetPropertyIndex( "prop1" ) == 1 );
	REQUIRE( strcmp( enumVar->GetPropertyName( 1 ), "prop1" ) == 0 );
	REQUIRE( enumVar->GetPropertyValueCount( 1 ) == 2 );
	REQUIRE( enumVar->GetPropertyValueCount( "prop1" ) == 2 );

	REQUIRE( strcmp( enumVar->GetPropertyValue( 1, 0 ), "val0" ) == 0 );
	REQUIRE( strcmp( enumVar->GetPropertyValue( "prop1", 0 ), "val0" ) == 0 );

	REQUIRE( strcmp( enumVar->GetPropertyValue( 1, 1 ), "val1" ) == 0 );
	REQUIRE( strcmp( enumVar->GetPropertyValue( "prop1", 1 ), "val1" ) == 0 );
}

//------------------------------------------------------------------------------
// NamespaceClass
//------------------------------------------------------------------------------
TEST_CASE( "Class registration in namespaces", "[aeMeta]" )
{
	REQUIRE( ae::GetType< Namespace0::Namespace1::NamespaceClass >() );
	REQUIRE( ae::GetTypeByName( "Namespace0::Namespace1::NamespaceClass" ) );
	REQUIRE( ae::GetType< Namespace0::Namespace1::NamespaceClass >() == ae::GetTypeByName( "Namespace0::Namespace1::NamespaceClass" ) );
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

TEST_CASE( "Aggregate vars", "[aeMeta]" )
{
	const ae::Type* type = ae::GetType< AggregateClass >();
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) == 2 );

	AggregateClass c;

	{
		const ae::Var* someClass = type->GetVarByName( "someClass", false );
		REQUIRE( someClass );
		const ae::ClassVarType* someClassVarType = someClass->GetOuterVarType< ae::ClassVarType >();
		REQUIRE( someClassVarType );
		REQUIRE( someClassVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( someClassVarType->GetSize() == sizeof(SomeClass) );
		REQUIRE( someClass->GetSubType() == ae::GetType< SomeClass >() );
		SomeClass* someClassPtr = someClass->GetPointer< SomeClass >( &c );
		REQUIRE( someClassPtr == &c.someClass );
		REQUIRE( someClass->GetPointer< SomeClass >( &c, 0 ) == nullptr );
		REQUIRE( someClass->GetPointer< SomeClass >( &c, 1 ) == nullptr );
		const ae::Type* varType = ae::GetTypeFromObject( someClassPtr );
		REQUIRE( varType );
		REQUIRE( varType == ae::GetType< SomeClass >() );
	}
	{
		const ae::Var* someClass1 = type->GetVarByName( "someClass1", false );
		REQUIRE( someClass1 );
		const ae::ClassVarType* someClass1Type = someClass1->GetOuterVarType< ae::ClassVarType >();
		REQUIRE( someClass1Type );
		REQUIRE( someClass1Type->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( someClass1Type->GetSize() == sizeof(SomeClass) );
		REQUIRE( someClass1->GetSubType() == ae::GetType< SomeClass >() );
		SomeClass* someClass1Ptr = someClass1->GetPointer< SomeClass >( &c );
		REQUIRE( someClass1Ptr == &c.someClass1 );
		REQUIRE( someClass1->GetPointer< SomeClass >( &c, 0 ) == nullptr );
		REQUIRE( someClass1->GetPointer< SomeClass >( &c, 1 ) == nullptr );
		const ae::Type* varType = ae::GetTypeFromObject( someClass1Ptr );
		REQUIRE( varType );
		REQUIRE( varType == ae::GetType< SomeClass >() );
	}
}

TEST_CASE( "Var::GetObjectValue()", "[aeMeta]" )
{
	SomeClass _c;
	_c.intMember = 123;
	_c.boolMember = true;
	_c.enumTest = TestEnumClass::Five;
	const SomeClass& c = _c;
	const ae::Type* type = ae::GetTypeFromObject( &c );
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) == 3 );

	{
		int32_t intMember = 0;
		const ae::Var* intVar = type->GetVarByName( "intMember", false );
		REQUIRE( intVar );
		const ae::BasicVarType* intVarType = intVar->GetOuterVarType< ae::BasicVarType >();
		REQUIRE( intVarType );
		REQUIRE( intVarType->GetType() == ae::BasicType::Int32 );
		REQUIRE( intVar->GetObjectValue< int32_t >( &c, &intMember ) );
		REQUIRE( intMember == 123 );
	}
	{
		bool boolMember = false;
		const ae::Var* boolVar = type->GetVarByName( "boolMember", false );
		REQUIRE( boolVar );
		const ae::BasicVarType* boolVarType = boolVar->GetOuterVarType< ae::BasicVarType >();
		REQUIRE( boolVarType );
		REQUIRE( boolVarType->GetType() == ae::BasicType::Bool );
		REQUIRE( boolVar->GetObjectValue< bool >( &c, &boolMember ) );
		REQUIRE( boolMember == true );
	}
	{
		TestEnumClass enumTest = TestEnumClass::Zero;
		const ae::Var* enumVar = type->GetVarByName( "enumTest", false );
		REQUIRE( enumVar );
		const ae::EnumVarType* enumVarType = enumVar->GetOuterVarType< ae::EnumVarType >();
		REQUIRE( enumVarType );
		REQUIRE( ae::Str32( "TestEnumClass" ) == enumVarType->GetName() );
		REQUIRE( enumVar->GetObjectValue< TestEnumClass >( &c, &enumTest ) );
		REQUIRE( enumTest == TestEnumClass::Five );
	}
}

TEST_CASE( "Array vars", "[aeMeta]" )
{
	const ae::Type* type = ae::GetType< ArrayClass >();
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) == 6 );

	ArrayClass c;
	ae::VarData varData = &c;

	// int32_t intArray[ 3 ];
	{
		const ae::Var* intArrayVar = type->GetVarByName( "intArray", false );
		REQUIRE( intArrayVar );

		ae::VarData intArray( intArrayVar, &c );
		ae::VarData _intArray = &c.intArray;
		REQUIRE( intArray );
		REQUIRE( _intArray );
		REQUIRE( intArray == _intArray );
		REQUIRE( intArray.GetType() == intArrayVar->GetOuterVarType() );
		const ae::ArrayVarType* arrayVarType = intArrayVar->GetOuterVarType< ae::ArrayVarType >();
		REQUIRE( arrayVarType );

		REQUIRE( arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( intArray ) == 3 );
		REQUIRE( arrayVarType->GetMaxLength() == 3 );
		const ae::BasicVarType* basicVarType = arrayVarType->GetInnerVarType< ae::BasicVarType >();
		REQUIRE( basicVarType );
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );
		REQUIRE( basicVarType->GetSize() == sizeof(int32_t) );
		
		// @TODO: Old, replace with VarType functions
		REQUIRE( intArrayVar->IsArray() );
		REQUIRE( intArrayVar->IsArrayFixedLength() );
		REQUIRE( intArrayVar->GetArrayLength( &c ) == 3 );
		REQUIRE( intArrayVar->GetArrayMaxLength() == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 0 ) == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 4 ) == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 5 ) == 3 );
	}
	// ae::Array< int32_t, 4 > intArray2;
	{
		const ae::Var* intArray2Var = type->GetVarByName( "intArray2", false );
		REQUIRE( intArray2Var );

		ae::VarData intArray2( intArray2Var, &c );
		ae::VarData _intArray2 = &c.intArray2;
		REQUIRE( intArray2 );
		REQUIRE( _intArray2 );
		REQUIRE( intArray2 == _intArray2 );
		REQUIRE( intArray2.GetType() == intArray2Var->GetOuterVarType() );
		const ae::ArrayVarType* arrayVarType = intArray2Var->GetOuterVarType< ae::ArrayVarType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( intArray2 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == 4 );
		const ae::BasicVarType* basicVarType = arrayVarType->GetInnerVarType< ae::BasicVarType >();
		REQUIRE( basicVarType );
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );
		REQUIRE( basicVarType->GetSize() == sizeof(int32_t) );

		// @TODO: Old, replace with VarType functions
		REQUIRE( intArray2Var->IsArray() );
		REQUIRE( !intArray2Var->IsArrayFixedLength() );
		REQUIRE( intArray2Var->GetArrayLength( &c ) == 0 );
		REQUIRE( intArray2Var->GetArrayMaxLength() == 4 );
		REQUIRE( intArray2Var->SetArrayLength( &c, 4 ) == 4 );
		REQUIRE( c.intArray2.Length() == 4 );
		REQUIRE( intArray2Var->SetArrayLength( &c, 2 ) == 2 );
		REQUIRE( c.intArray2.Length() == 2 );
		REQUIRE( intArray2Var->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( c.intArray2.Length() == 3 );
		REQUIRE( intArray2Var->SetArrayLength( &c, 0 ) == 0 );
		REQUIRE( c.intArray2.Length() == 0 );
		REQUIRE( intArray2Var->SetArrayLength( &c, 1 ) == 1 );
		REQUIRE( c.intArray2.Length() == 1 );
	}
	// ae::Array< int32_t > intArray3 = AE_ALLOC_TAG_META_TEST;
	{
		const ae::Var* intArray3Var = type->GetVarByName( "intArray3", false );
		REQUIRE( intArray3Var );
		
		ae::VarData intArray3( intArray3Var, &c );
		ae::VarData _intArray3 = &c.intArray3;
		REQUIRE( intArray3 );
		REQUIRE( _intArray3 );
		REQUIRE( intArray3 == _intArray3 );
		REQUIRE( intArray3.GetType() == intArray3Var->GetOuterVarType() );
		const ae::ArrayVarType* arrayVarType = intArray3Var->GetOuterVarType< ae::ArrayVarType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( intArray3 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == ae::MaxValue< uint32_t >() );
		const ae::BasicVarType* basicVarType = arrayVarType->GetInnerVarType< ae::BasicVarType >();
		REQUIRE( basicVarType );
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );
		REQUIRE( basicVarType->GetSize() == sizeof(int32_t) );

		// @TODO: Old, replace with VarType functions
		REQUIRE( intArray3Var->IsArray() );
		REQUIRE( !intArray3Var->IsArrayFixedLength() );
		REQUIRE( intArray3Var->GetArrayLength( &c ) == 0 );
		REQUIRE( intArray3Var->GetArrayMaxLength() == ae::MaxValue< uint32_t >() );
		REQUIRE( intArray3Var->SetArrayLength( &c, 4 ) == 4 );
		REQUIRE( c.intArray3.Length() == 4 );
		REQUIRE( intArray3Var->SetArrayLength( &c, 2 ) == 2 );
		REQUIRE( c.intArray3.Length() == 2 );
		REQUIRE( intArray3Var->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( c.intArray3.Length() == 3 );
		REQUIRE( intArray3Var->SetArrayLength( &c, 0 ) == 0 );
		REQUIRE( c.intArray3.Length() == 0 );
		REQUIRE( intArray3Var->SetArrayLength( &c, 1 ) == 1 );
		REQUIRE( c.intArray3.Length() == 1 );
		REQUIRE( intArray3Var->SetArrayLength( &c, 10020 ) == 10020 );
		REQUIRE( c.intArray3.Length() == 10020 );
		REQUIRE( intArray3Var->SetArrayLength( &c, 0 ) == 0 );
		REQUIRE( c.intArray3.Length() == 0 );
	}
	// SomeClass someClassArray[ 3 ];
	{
		const ae::Var* someClassArrayVar = type->GetVarByName( "someClassArray", false );
		REQUIRE( someClassArrayVar );

		ae::VarData someClassArray( someClassArrayVar, &c );
		ae::VarData _someClassArray = &c.someClassArray;
		REQUIRE( someClassArray );
		REQUIRE( _someClassArray );
		REQUIRE( someClassArray == _someClassArray );
		REQUIRE( someClassArray.GetType() == someClassArrayVar->GetOuterVarType() );
		const ae::ArrayVarType* arrayVarType = someClassArrayVar->GetOuterVarType< ae::ArrayVarType >();
		REQUIRE( arrayVarType );

		REQUIRE( arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( someClassArray ) == 3 );
		REQUIRE( arrayVarType->GetMaxLength() == 3 );
		const ae::ClassVarType* classVarType = arrayVarType->GetInnerVarType< ae::ClassVarType >();
		REQUIRE( classVarType );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeId< SomeClass >() );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// @TODO: Old, replace with VarType functions
		REQUIRE( someClassArrayVar->GetSubType() == ae::GetType< SomeClass >() );
		REQUIRE( someClassArrayVar->IsArrayFixedLength() );
		REQUIRE( someClassArrayVar->GetArrayLength( &c ) == 3 );
		REQUIRE( someClassArrayVar->GetArrayMaxLength() == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 0 ) == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 4 ) == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 5 ) == 3 );
	}
	// ae::Array< SomeClass, 4 > someClassArray2;
	{
		const ae::Var* someClassArray2Var = type->GetVarByName( "someClassArray2", false );
		REQUIRE( someClassArray2Var );

		ae::VarData someClassArray2( someClassArray2Var, &c );
		ae::VarData _someClassArray2 = &c.someClassArray2;
		REQUIRE( someClassArray2 );
		REQUIRE( _someClassArray2 );
		REQUIRE( someClassArray2 == _someClassArray2 );
		REQUIRE( someClassArray2.GetType() == someClassArray2Var->GetOuterVarType() );
		const ae::ArrayVarType* arrayVarType = someClassArray2Var->GetOuterVarType< ae::ArrayVarType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( someClassArray2 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == 4 );
		const ae::ClassVarType* classVarType = arrayVarType->GetInnerVarType< ae::ClassVarType >();
		REQUIRE( classVarType );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeId< SomeClass >() );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// @TODO: Old, replace with VarType functions
		REQUIRE( someClassArray2Var->IsArray() );
		REQUIRE( someClassArray2Var->GetSubType() == ae::GetType< SomeClass >() );
		REQUIRE( !someClassArray2Var->IsArrayFixedLength() );
		REQUIRE( someClassArray2Var->GetArrayLength( &c ) == 0 );
		REQUIRE( someClassArray2Var->GetArrayMaxLength() == 4 );
		REQUIRE( someClassArray2Var->SetArrayLength( &c, 4 ) == 4 );
		REQUIRE( c.someClassArray2.Length() == 4 );
		REQUIRE( someClassArray2Var->SetArrayLength( &c, 2 ) == 2 );
		REQUIRE( c.someClassArray2.Length() == 2 );
		REQUIRE( someClassArray2Var->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( c.someClassArray2.Length() == 3 );
		REQUIRE( someClassArray2Var->SetArrayLength( &c, 0 ) == 0 );
		REQUIRE( c.someClassArray2.Length() == 0 );
		REQUIRE( someClassArray2Var->SetArrayLength( &c, 1 ) == 1 );
		REQUIRE( c.someClassArray2.Length() == 1 );
	}
	// ae::Array< SomeClass > someClassArray3 = AE_ALLOC_TAG_META_TEST;
	{
		const ae::Var* someClassArray3Var = type->GetVarByName( "someClassArray3", false );
		REQUIRE( someClassArray3Var );

		ae::VarData someClassArray3( someClassArray3Var, &c );
		ae::VarData _someClassArray3 = &c.someClassArray3;
		REQUIRE( someClassArray3 );
		REQUIRE( _someClassArray3 );
		REQUIRE( someClassArray3 == _someClassArray3 );
		REQUIRE( someClassArray3.GetType() == someClassArray3Var->GetOuterVarType() );
		const ae::ArrayVarType* arrayVarType = someClassArray3Var->GetOuterVarType< ae::ArrayVarType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( someClassArray3 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == ae::MaxValue< uint32_t >() );
		const ae::ClassVarType* classVarType = arrayVarType->GetInnerVarType< ae::ClassVarType >();
		REQUIRE( classVarType );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeId< SomeClass >() );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// @TODO: Old, replace with VarType functions
		REQUIRE( someClassArray3Var->IsArray() );
		REQUIRE( someClassArray3Var->GetSubType() == ae::GetType< SomeClass >() );
		REQUIRE( !someClassArray3Var->IsArrayFixedLength() );
		REQUIRE( someClassArray3Var->GetArrayLength( &c ) == 0 );
		REQUIRE( someClassArray3Var->GetArrayMaxLength() == ae::MaxValue< uint32_t >() );
		REQUIRE( someClassArray3Var->SetArrayLength( &c, 4 ) == 4 );
		REQUIRE( c.someClassArray3.Length() == 4 );
		REQUIRE( someClassArray3Var->SetArrayLength( &c, 2 ) == 2 );
		REQUIRE( c.someClassArray3.Length() == 2 );
		REQUIRE( someClassArray3Var->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( c.someClassArray3.Length() == 3 );
		REQUIRE( someClassArray3Var->SetArrayLength( &c, 0 ) == 0 );
		REQUIRE( c.someClassArray3.Length() == 0 );
		REQUIRE( someClassArray3Var->SetArrayLength( &c, 1 ) == 1 );
		REQUIRE( c.someClassArray3.Length() == 1 );
		REQUIRE( someClassArray3Var->SetArrayLength( &c, 10020 ) == 10020 );
		REQUIRE( c.someClassArray3.Length() == 10020 );
		REQUIRE( someClassArray3Var->SetArrayLength( &c, 0 ) == 0 );
		REQUIRE( c.someClassArray3.Length() == 0 );

		REQUIRE( someClassArray3Var->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( someClassArray3Var->GetPointer< SomeClass >( &c, -1 ) == nullptr );
		REQUIRE( someClassArray3Var->GetPointer< SomeClass >( &c, 0 ) == &c.someClassArray3[ 0 ] );
		REQUIRE( someClassArray3Var->GetPointer< SomeClass >( &c, 1 ) == &c.someClassArray3[ 1 ] );
		REQUIRE( someClassArray3Var->GetPointer< SomeClass >( &c, 2 ) == &c.someClassArray3[ 2 ] );
	}
}

TEST_CASE("Optional test present", "[aeMeta]")
{
	const ae::Type* type = ae::GetType< OptionalClass >();
	REQUIRE( type );

	OptionalClass c;
	c.intOptional = 123;
	// c.someClassOptional = {}; // @TODO: Fix more than one operator matches operands
	c.intStdOptional = 456;
	c.someClassStdOptional = SomeClass();
	c.someClassStdOptional->intMember = 456;
	c.someClassStdOptional->boolMember = true;
	c.someClassStdOptional->enumTest = TestEnumClass::Five;

	// ae::Optional< int32_t > intOptional;
	{
		const ae::Var* intOptional = type->GetVarByName( "intOptional", false );
		REQUIRE( intOptional );
		const ae::OptionalVarType* optionalType = intOptional->GetOuterVarType< ae::OptionalVarType >();
		REQUIRE( optionalType );
		const ae::BasicVarType* basicVarType = optionalType->GetInnerVarType< ae::BasicVarType >();
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

		// int32_t value = 0;
		// REQUIRE( c.intOptional->TryGet() );
		// REQUIRE( value == 123 );
	}
	// ae::Optional < SomeClass > someClassOptional;
	{
		const ae::Var* someClassOptional = type->GetVarByName( "someClassOptional", false );
		REQUIRE( someClassOptional );
		const ae::OptionalVarType* optionalType = someClassOptional->GetOuterVarType< ae::OptionalVarType >();
		REQUIRE( optionalType );
		const ae::ClassVarType* classVarType = optionalType->GetInnerVarType< ae::ClassVarType >();
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeId< SomeClass >() );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// SomeClass value;
		// REQUIRE( someClassOptional->GetObjectValue( &c, &value ) );
		// REQUIRE( value.intMember == 123 );
		// REQUIRE( value.boolMember == true );
		// REQUIRE( value.enumTest == TestEnumClass::Five );
	}
	// // std::optional< int32_t > intStdOptional;
	// {
	// 	const ae::Var* intStdOptional = type->GetVarByName( "intStdOptional", false );
	// 	REQUIRE( intStdOptional );
	// 	const ae::OptionalVarType* optionalType = intStdOptional->GetOuterVarType< ae::OptionalVarType >();
	// 	REQUIRE( optionalType );
	// 	const ae::BasicVarType* basicVarType = optionalType->GetInnerVarType< ae::BasicVarType >();
	// 	REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

	// 	int32_t value = 0;
	// 	const ae::ConstVarData optional = intStdOptional->GetVarData( &c );
	// 	REQUIRE( optional );
	// 	ae::ConstVarData basic = optionalType->TryGet( optional );
	// 	REQUIRE( basicVarType->GetVarData( basic, &value ) );
	// 	REQUIRE( value == 456 );
	// }
	// // std::optional< SomeClass > someClassStdOptional;
	// {
	// 	const ae::Var* someClassStdOptional = type->GetVarByName( "someClassStdOptional", false );
	// 	REQUIRE( someClassStdOptional );
	// 	const ae::OptionalVarType* optionalType = someClassStdOptional->GetOuterVarType< ae::OptionalVarType >();
	// 	REQUIRE( optionalType );
	// 	const ae::ClassVarType* classVarType = optionalType->GetInnerVarType< ae::ClassVarType >();
	// 	REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
	// 	REQUIRE( classVarType->GetTypeId() == ae::GetTypeId< SomeClass >() );
	// 	REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

	// 	ae::ConstVarData optional = someClassStdOptional->GetVarData( &c );
	// 	REQUIRE( optional );
	// 	ae::ConstVarData _class = optionalType->TryGet( optional );
	// 	REQUIRE( _class );
	// 	const ae::Type* type = classVarType->GetType();
	// 	const SomeClass* someClass = classVarType->TryGet< SomeClass >( _class );
	// 	REQUIRE( someClass );
	// 	REQUIRE( someClass->intMember == 456 );
	// 	REQUIRE( someClass->boolMember == true );
	// 	REQUIRE( someClass->enumTest == TestEnumClass::Five );
	// }
}

TEST_CASE("Optional test no value", "[aeMeta]")
{
	const ae::Type* type = ae::GetType< OptionalClass >();
	REQUIRE( type );

	OptionalClass c;

	// ae::Optional< int32_t > intOptional;
	{
		const ae::Var* intOptional = type->GetVarByName( "intOptional", false );
		REQUIRE( intOptional );
		const ae::OptionalVarType* optionalType = intOptional->GetOuterVarType< ae::OptionalVarType >();
		REQUIRE( optionalType );
		const ae::BasicVarType* basicVarType = optionalType->GetInnerVarType< ae::BasicVarType >();
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

		ae::ConstVarData optional( intOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
	// ae::Optional < SomeClass > someClassOptional;
	{
		const ae::Var* someClassOptional = type->GetVarByName( "someClassOptional", false );
		REQUIRE( someClassOptional );
		const ae::OptionalVarType* optionalType = someClassOptional->GetOuterVarType< ae::OptionalVarType >();
		REQUIRE( optionalType );
		const ae::ClassVarType* classVarType = optionalType->GetInnerVarType< ae::ClassVarType >();
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeId< SomeClass >() );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		ae::ConstVarData optional( someClassOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
	// std::optional< int32_t > intStdOptional;
	{
		const ae::Var* intStdOptional = type->GetVarByName( "intStdOptional", false );
		REQUIRE( intStdOptional );
		const ae::OptionalVarType* optionalType = intStdOptional->GetOuterVarType< ae::OptionalVarType >();
		REQUIRE( optionalType );
		const ae::BasicVarType* basicVarType = optionalType->GetInnerVarType< ae::BasicVarType >();
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

		ae::ConstVarData optional( intStdOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
	// std::optional< SomeClass > someClassStdOptional;
	{
		const ae::Var* someClassStdOptional = type->GetVarByName( "someClassStdOptional", false );
		REQUIRE( someClassStdOptional );
		const ae::OptionalVarType* optionalType = someClassStdOptional->GetOuterVarType< ae::OptionalVarType >();
		REQUIRE( optionalType );
		const ae::ClassVarType* classVarType = optionalType->GetInnerVarType< ae::ClassVarType >();
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeId< SomeClass >() );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		ae::ConstVarData optional( someClassStdOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
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
	REQUIRE( enumTestVar );
	const ae::EnumVarType* enumVarType = enumTestVar->GetOuterVarType< ae::EnumVarType >();
	REQUIRE( enumVarType );
	
	ae::ConstVarData varData( enumTestVar, &c );
	
	c.enumTest = TestEnumClass::Five;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Five" );
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Five" ); // @TODO: Remove
	c.enumTest = TestEnumClass::Four;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Four" );
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Four" ); // @TODO: Remove
	c.enumTest = TestEnumClass::Three;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Three" );
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Three" ); // @TODO: Remove
	c.enumTest = TestEnumClass::Two;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Two" );
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Two" ); // @TODO: Remove
	c.enumTest = TestEnumClass::One;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "One" );
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "One" ); // @TODO: Remove
	c.enumTest = TestEnumClass::Zero;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Zero" );
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Zero" ); // @TODO: Remove
	c.enumTest = TestEnumClass::NegativeOne;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "NegativeOne" );
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "NegativeOne" ); // @TODO: Remove
}

TEST_CASE( "can't read invalid enum values from object using meta definition", "[aeMeta]" )
{
	SomeClass c;
	const ae::Type* type = ae::GetTypeFromObject( &c );
	const ae::Var* enumTestVar = type->GetVarByName( "enumTest", false );
	const ae::EnumVarType* enumVarType = enumTestVar->GetOuterVarType< ae::EnumVarType >();
	REQUIRE( enumVarType );
	
	c.enumTest = (TestEnumClass)6;
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "" );
}

TEST_CASE( "can set enum values on object using meta definition", "[aeMeta]" )
{
	SomeClass c;
	const ae::Type* type = ae::GetTypeFromObject( &c );
	const ae::Var* enumTestVar = type->GetVarByName( "enumTest", false );
	const ae::EnumVarType* enumVarType = enumTestVar->GetOuterVarType< ae::EnumVarType >();
	REQUIRE( enumVarType );
	
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
	const ae::EnumVarType* enumVarType = enumTestVar->GetOuterVarType< ae::EnumVarType >();
	REQUIRE( enumVarType );
	
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
	const ae::Type* type_RefTester = ae::GetType< RefTester >();
	REQUIRE( type_RefTester );

	const ae::Type* type_RefTesterA = ae::GetType< RefTesterA >();
	REQUIRE( type_RefTesterA );
	const ae::Var* var_RefTesterA_notRef = type_RefTesterA->GetVarByName( "notRef", false );
	const ae::Var* var_RefTesterA_refA = type_RefTesterA->GetVarByName( "refA", false );
	const ae::Var* var_RefTesterA_refB = type_RefTesterA->GetVarByName( "refB", false );
	REQUIRE( var_RefTesterA_notRef );
	REQUIRE( var_RefTesterA_refA );
	REQUIRE( var_RefTesterA_refB );
	
	const ae::Type* type_RefTesterB = ae::GetType< RefTesterB >();
	REQUIRE( type_RefTesterB );
	const ae::Var* var_RefTesterB_ref = type_RefTesterB->GetVarByName( "ref", false );
	REQUIRE( var_RefTesterB_ref );
	
	REQUIRE( !var_RefTesterA_notRef->GetSubType() );
	REQUIRE( var_RefTesterA_refA->GetSubType() == type_RefTesterA );
	REQUIRE( var_RefTesterA_refB->GetSubType() == type_RefTesterB );
	REQUIRE( var_RefTesterB_ref->GetSubType() == type_RefTester );
	
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
	
	RefTesterA* refTesterA1 = manager.Create< RefTesterA >();
	RefTesterA* refTesterA2 = manager.Create< RefTesterA >();
	RefTesterB* refTesterB3 = manager.Create< RefTesterB >();
	
	// Validate ids
	REQUIRE( refTesterA1->id == 1 );
	REQUIRE( refTesterA2->id == 2 );
	REQUIRE( refTesterB3->id == 3 );
	REQUIRE( RefTester::GetIdString( refTesterA1 ) == "1" );
	REQUIRE( RefTester::GetIdString( refTesterA2 ) == "2" );
	REQUIRE( RefTester::GetIdString( refTesterB3 ) == "3" );
	
	// Validate initial reference values
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == nullptr );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == nullptr );
	REQUIRE( refTesterB3->ref == nullptr );
	REQUIRE( var_RefTesterA_refA->GetObjectValueAsString( refTesterA1 ) == "0" );
	REQUIRE( var_RefTesterA_refA->GetObjectValueAsString( refTesterA2 ) == "0" );
	REQUIRE( var_RefTesterB_ref->GetObjectValueAsString( refTesterB3 ) == "0" );
	
	// Set type A's ref to type A
	REQUIRE( var_RefTesterA_refA->SetObjectValueFromString( refTesterA1, "2" ) );
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == refTesterA2 );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == nullptr );
	REQUIRE( refTesterB3->ref == nullptr );
	
	// Set type B's ref to type A
	REQUIRE( var_RefTesterB_ref->SetObjectValueFromString( refTesterB3, "2" ) );
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == refTesterA2 );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == nullptr );
	REQUIRE( refTesterB3->ref == refTesterA2 );

	// Set type B's ref to type B
	REQUIRE( var_RefTesterB_ref->SetObjectValueFromString( refTesterB3, "3" ) );
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == refTesterA2 );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == nullptr );
	REQUIRE( refTesterB3->ref == refTesterB3 );

	// Set type B's ref to type B
	REQUIRE( var_RefTesterB_ref->SetObjectValue( refTesterB3, nullptr ) );
	REQUIRE( refTesterB3->ref == nullptr );
	REQUIRE( var_RefTesterB_ref->SetObjectValueFromString( refTesterB3, "3" ) );
	REQUIRE( refTesterB3->ref == refTesterB3 );
	REQUIRE( var_RefTesterB_ref->SetObjectValue( refTesterB3, nullptr ) );
	REQUIRE( refTesterB3->ref == nullptr );
	REQUIRE( var_RefTesterB_ref->SetObjectValue( refTesterB3, refTesterB3 ) );
	REQUIRE( refTesterB3->ref == refTesterB3 );

	// Set type B's ref to type A
	REQUIRE( var_RefTesterB_ref->SetObjectValue( refTesterB3, nullptr ) );
	REQUIRE( refTesterB3->ref == nullptr );
	REQUIRE( var_RefTesterB_ref->SetObjectValueFromString( refTesterB3, "2" ) );
	REQUIRE( refTesterB3->ref == refTesterA2 );
	REQUIRE( var_RefTesterB_ref->SetObjectValue( refTesterB3, nullptr ) );
	REQUIRE( refTesterB3->ref == nullptr );
	REQUIRE( var_RefTesterB_ref->SetObjectValue( refTesterB3, refTesterA2 ) );
	REQUIRE( refTesterB3->ref == refTesterA2 );
	
	REQUIRE( var_RefTesterA_refA->GetObjectValueAsString( refTesterA1 ) == "2" );
	REQUIRE( var_RefTesterA_refA->GetObjectValueAsString( refTesterA2 ) == "0" );
	REQUIRE( var_RefTesterB_ref->GetObjectValueAsString( refTesterB3 ) == "2" );
	
	// Set type A's ref B to type B
	REQUIRE( var_RefTesterA_refB->SetObjectValueFromString( refTesterA2, "3" ) );
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == refTesterA2 );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == refTesterB3 );
	REQUIRE( refTesterB3->ref == refTesterA2 );
	
	// Setting type A ref A to type B by string should do nothing
	REQUIRE( !var_RefTesterA_refA->SetObjectValueFromString( refTesterA1, "3" ) );
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == refTesterA2 );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == refTesterB3 );
	REQUIRE( refTesterB3->ref == refTesterA2 );
	
	// Setting type A ref A to type B by value should do nothing
	REQUIRE( !var_RefTesterA_refA->SetObjectValue( refTesterA1, refTesterB3 ) );
	REQUIRE( refTesterA1->refA == refTesterA2 );
	
	// Setting ref from random string value does nothing
	REQUIRE( !var_RefTesterA_refA->SetObjectValueFromString( refTesterA1, "qwerty" ) );
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == refTesterA2 );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == refTesterB3 );
	REQUIRE( refTesterB3->ref == refTesterA2 );
	
	// Setting ref to null value succeeds and clears ref
	REQUIRE( var_RefTesterA_refA->SetObjectValueFromString( refTesterA1, "0" ) );
	REQUIRE( refTesterA1->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA1->refA == nullptr );
	REQUIRE( refTesterA1->refB == nullptr );
	REQUIRE( refTesterA2->notRef == 0xfdfdfdfd );
	REQUIRE( refTesterA2->refA == nullptr );
	REQUIRE( refTesterA2->refB == refTesterB3 );
	REQUIRE( refTesterB3->ref == refTesterA2 );

	manager.Destroy( refTesterA1 );
	manager.Destroy( refTesterA2 );
	manager.Destroy( refTesterB3 );
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
