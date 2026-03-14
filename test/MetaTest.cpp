//------------------------------------------------------------------------------
// MetaTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#include "TestUtils.h"
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// Enum helpers
//------------------------------------------------------------------------------
template< typename T >
static void RequireEnumValueByIndex( const ae::EnumType* enumType, int32_t index, const T& expectedValue )
{
	T valueOut = enumType->GetValueByIndex< T >( index );
	REQUIRE( valueOut == expectedValue );
}

template< typename T >
struct EnumMetaCase
{
	const char* name;
	T value;
};

template< typename T, uint32_t N >
static void CheckEnumMetadata(
	const char* typeName,
	bool isSigned,
	const EnumMetaCase< T >( &cases )[ N ],
	std::underlying_type_t< T > missingValue,
	const char* outOfRangeValue )
{
	using U = std::underlying_type_t< T >;

	const ae::EnumType* enumType = ae::GetEnumType< T >();
	REQUIRE( enumType == ae::GetEnumType( typeName ) );
	REQUIRE( enumType->TypeSize() == sizeof(T) );
	REQUIRE( enumType->TypeIsSigned() == isSigned );
	REQUIRE( enumType->Length() == N );

	for( uint32_t i = 0; i < N; i++ )
	{
		const std::string numericValue = ae::ToString( static_cast< U >( cases[ i ].value ) );

		REQUIRE( enumType->GetNameByIndex( i ) == cases[ i ].name );
		RequireEnumValueByIndex( enumType, i, cases[ i ].value );

		REQUIRE( enumType->GetNameByValue( cases[ i ].value ) == cases[ i ].name );
		REQUIRE( enumType->GetNameByValue( static_cast< U >( cases[ i ].value ) ) == cases[ i ].name );
		REQUIRE( enumType->HasValue( cases[ i ].value ) );
		REQUIRE( enumType->HasValue( static_cast< U >( cases[ i ].value ) ) );

		T parsedEnum = static_cast< T >( missingValue );
		REQUIRE( enumType->GetValueFromString( cases[ i ].name, &parsedEnum ) );
		REQUIRE( parsedEnum == cases[ i ].value );

		parsedEnum = static_cast< T >( missingValue );
		REQUIRE( enumType->GetValueFromString( numericValue.c_str(), &parsedEnum ) );
		REQUIRE( parsedEnum == cases[ i ].value );

		T varValue = cases[ 0 ].value;
		ae::DataPointer varData( &varValue );
		REQUIRE( enumType->SetVarData( varData, cases[ i ].value ) );
		REQUIRE( varValue == cases[ i ].value );
		REQUIRE( enumType->GetVarDataAsString( ae::ConstDataPointer( varData ) ) == cases[ i ].name );

		U underlyingValue = 0;
		REQUIRE( enumType->GetVarData( ae::ConstDataPointer( varData ), &underlyingValue ) );
		REQUIRE( underlyingValue == static_cast< U >( cases[ i ].value ) );

		varValue = cases[ 0 ].value;
		REQUIRE( enumType->SetVarDataFromString( varData, cases[ i ].name ) );
		REQUIRE( varValue == cases[ i ].value );
		REQUIRE( enumType->SetVarDataFromString( varData, numericValue.c_str() ) );
		REQUIRE( varValue == cases[ i ].value );
	}

	const T missingEnumValue = static_cast< T >( missingValue );
	const std::string missingNumericValue = ae::ToString( missingValue );

	REQUIRE( enumType->GetNameByValue( missingEnumValue ) == "" );
	REQUIRE( !enumType->HasValue( missingEnumValue ) );

	T parsedEnum = cases[ 0 ].value;
	REQUIRE( !enumType->GetValueFromString( "MissingValue", &parsedEnum ) );
	REQUIRE( parsedEnum == cases[ 0 ].value );
	REQUIRE( !enumType->GetValueFromString( missingNumericValue.c_str(), &parsedEnum ) );
	REQUIRE( parsedEnum == cases[ 0 ].value );
	REQUIRE( !enumType->GetValueFromString( outOfRangeValue, &parsedEnum ) );
	REQUIRE( parsedEnum == cases[ 0 ].value );

	T varValue = cases[ 0 ].value;
	ae::DataPointer varData( &varValue );
	REQUIRE( !enumType->SetVarDataFromString( varData, missingNumericValue.c_str() ) );
	REQUIRE( varValue == cases[ 0 ].value );
	REQUIRE( !enumType->SetVarDataFromString( varData, outOfRangeValue ) );
	REQUIRE( varValue == cases[ 0 ].value );
}

template< std::size_t N >
constexpr bool ConstexprStringEquals( const char* lhs, const char( &rhs )[ N ] )
{
	for( std::size_t i = 0; i < N; i++ )
	{
		if( lhs[ i ] != rhs[ i ] )
		{
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
TEST_CASE( "Can get base type by name", "[aeMeta]" )
{
	const ae::ClassType* objType = ae::GetClassType< ae::Object >();
	const ae::Str32 objTypeName = objType->GetName();
	REQUIRE( objType );
	REQUIRE( objTypeName == "ae::Object" );
	REQUIRE( ae::GetClassTypeByName( "ae::Object" ) == objType );

	REQUIRE( ae::GetTypeName< ae::Object >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::Object* >() == ae::Str32( "ae::Object *" ) );
	REQUIRE( ae::GetTypeName< ae::Object& >() == ae::Str32( "ae::Object &" ) );
	REQUIRE( ae::GetTypeName< ae::Object[ 3 ] >() == ae::Str32( "ae::Object[3]" ) );
	REQUIRE( ae::GetTypeName< const ae::Object >() == ae::Str32( "const ae::Object" ) );
	REQUIRE( ae::GetTypeName< const ae::Object* >() == ae::Str32( "const ae::Object *" ) );
	REQUIRE( ae::GetTypeName< const ae::Object& >() == ae::Str32( "const ae::Object &" ) );
	REQUIRE( ae::GetTypeName< const ae::Object[ 3 ] >() == ae::Str32( "const ae::Object[3]" ) );

	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< ae::Object > >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< ae::Object* > >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< ae::Object& > >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< ae::Object[ 3 ] > >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< const ae::Object > >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< const ae::Object* > >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< const ae::Object& > >() == ae::Str32( "ae::Object" ) );
	REQUIRE( ae::GetTypeName< ae::RemoveTypeQualifiers< const ae::Object[ 3 ] > >() == ae::Str32( "ae::Object" ) );
}

TEST_CASE( "GetTypeName can be used in constexpr context", "[aeMeta]" )
{
	constexpr const char* objectName = ae::GetTypeName< ae::Object >();
	constexpr const char* objectPtrName = ae::GetTypeName< ae::Object* >();
	constexpr const char* constArrayName = ae::GetTypeName< const ae::Object[ 3 ] >();
	static_assert( ConstexprStringEquals( objectName, "ae::Object" ) );
	static_assert( ConstexprStringEquals( objectPtrName, "ae::Object *" ) );
	static_assert( ConstexprStringEquals( constArrayName, "const ae::Object[3]" ) );
	static_assert( ae::GetTypeIdFromName( objectName ) == ae::TypeId( "ae::Object" ) );
	REQUIRE( objectName == ae::Str32( "ae::Object" ) );
	REQUIRE( objectPtrName == ae::Str32( "ae::Object *" ) );
	REQUIRE( constArrayName == ae::Str32( "const ae::Object[3]" ) );
}

TEST_CASE( "Can get base type with templates", "[aeMeta]" )
{
	REQUIRE( ae::GetClassType< ae::Object >() );
}

TEST_CASE( "Class registration", "[aeMeta]" )
{
	REQUIRE( ae::GetClassType< SomeClass >() );
	REQUIRE( ae::GetClassTypeByName( "SomeClass" ) );
	REQUIRE( ae::GetClassType< SomeClass >() == ae::GetClassTypeByName( "SomeClass" ) );
	REQUIRE( ae::GetClassType< const SomeClass >() );
	REQUIRE( ae::GetClassType< const SomeClass >() == ae::GetClassType< SomeClass >() );
	REQUIRE( ae::GetClassType< SomeClass* >() );
	REQUIRE( ae::GetClassType< SomeClass* >() == ae::GetClassType< SomeClass >() );
}

#if AE_DEPRECATED
TEST_CASE( "Class properties", "[aeMeta]" )
{
	const ae::ClassType* type = ae::GetClassType< SomeClass >();
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
	AE_REQUIRE_THROWS( type->GetPropertyName( 3 ) );
	AE_REQUIRE_THROWS( type->GetPropertyValueCount( 3 ) );
}
#endif // AE_DEPRECATED

TEST_CASE( "Class vars", "[aeMeta]" )
{
	const ae::ClassType* type = ae::GetClassType< SomeClass >();
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) );

	REQUIRE( type->GetVarByName( "intMember", false ) );
	REQUIRE( type->GetVarByIndex( 0, false ) );
	REQUIRE( type->GetVarByName( "intMember", false ) == type->GetVarByIndex( 0, false ) );
#if AE_DEPRECATED
	const ae::ClassVar* intVar = type->GetVarByName( "intMember", false );
	REQUIRE( intVar->HasProperty( "intProp" ) );
	REQUIRE( intVar->GetPropertyIndex( "intProp" ) == 0 );
	REQUIRE( intVar->GetPropertyCount() == 1 );
	REQUIRE( strcmp( intVar->GetPropertyName( 0 ), "intProp" ) == 0 );
	REQUIRE( intVar->GetPropertyValueCount( 0 ) == 0 );
	REQUIRE( intVar->GetPropertyValueCount( "intProp" ) == 0 );
#endif // AE_DEPRECATED

	REQUIRE( type->GetVarByName( "boolMember", false ) );
	REQUIRE( type->GetVarByIndex( 1, false ) );
	REQUIRE( type->GetVarByName( "boolMember", false ) == type->GetVarByIndex( 1, false ) );
#if AE_DEPRECATED
	const ae::ClassVar* boolVar = type->GetVarByName( "boolMember", false );
	REQUIRE( boolVar->HasProperty( "boolProp" ) );
	REQUIRE( boolVar->GetPropertyIndex( "boolProp" ) == 0 );
	REQUIRE( boolVar->GetPropertyCount() == 1 );
	REQUIRE( strcmp( boolVar->GetPropertyName( 0 ), "boolProp" ) == 0 );
	REQUIRE( boolVar->GetPropertyValueCount( 0 ) == 1 );
	REQUIRE( boolVar->GetPropertyValueCount( "boolProp" ) == 1 );
	REQUIRE( strcmp( boolVar->GetPropertyValue( 0, 0 ), "val" ) == 0 );
	REQUIRE( strcmp( boolVar->GetPropertyValue( "boolProp", 0 ), "val" ) == 0 );
#endif // AE_DEPRECATED

	REQUIRE( type->GetVarByName( "enumTest", false ) );
	REQUIRE( type->GetVarByIndex( 2, false ) );
	REQUIRE( type->GetVarByName( "enumTest", false ) == type->GetVarByIndex( 2, false ) );
#if AE_DEPRECATED
	const ae::ClassVar* enumVar = type->GetVarByName( "enumTest", false );
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
#endif // AE_DEPRECATED
}

//------------------------------------------------------------------------------
// NamespaceClass
//------------------------------------------------------------------------------
TEST_CASE( "Class registration in namespaces", "[aeMeta]" )
{
	REQUIRE( ae::GetClassType< Namespace0::Namespace1::NamespaceClass >() );
	REQUIRE( ae::GetClassTypeByName( "Namespace0::Namespace1::NamespaceClass" ) );
	REQUIRE( ae::GetClassType< Namespace0::Namespace1::NamespaceClass >() == ae::GetClassTypeByName( "Namespace0::Namespace1::NamespaceClass" ) );
}

//------------------------------------------------------------------------------
// PlayerState
//------------------------------------------------------------------------------
TEST_CASE( "enum registration", "[aeMeta]" )
{
	const ae::EnumType* playerStateEnum = ae::GetEnumType( "PlayerState" );
	REQUIRE( playerStateEnum );
	REQUIRE( playerStateEnum->GetName() == ae::Str32( "PlayerState" ) );
	
	REQUIRE( playerStateEnum->TypeSize() == 2 );
	REQUIRE( playerStateEnum->TypeIsSigned() == false );
	
	REQUIRE( playerStateEnum->Length() == 3 );
	
	REQUIRE( playerStateEnum->GetNameByIndex( 0 ) == "Idle" );
	RequireEnumValueByIndex( playerStateEnum, 0, PlayerState::Idle );
	
	REQUIRE( playerStateEnum->GetNameByIndex( 1 ) == "Run" );
	RequireEnumValueByIndex( playerStateEnum, 1, PlayerState::Run );
	
	REQUIRE( playerStateEnum->GetNameByIndex( 2 ) == "Jump" );
	RequireEnumValueByIndex( playerStateEnum, 2, PlayerState::Jump );
}

TEST_CASE( "enum metadata supports all fixed-width underlying types", "[aeMeta]" )
{
	{
		const EnumMetaCase< EnumInt8 > cases[] =
		{
			{ "Min", EnumInt8::Min },
			{ "NegativeOne", EnumInt8::NegativeOne },
			{ "Zero", EnumInt8::Zero },
			{ "Max", EnumInt8::Max },
		};
		CheckEnumMetadata( "EnumInt8", true, cases, (int8_t)1, "128" );
	}
	{
		const EnumMetaCase< EnumUInt8 > cases[] =
		{
			{ "Zero", EnumUInt8::Zero },
			{ "One", EnumUInt8::One },
			{ "HighBit", EnumUInt8::HighBit },
			{ "Max", EnumUInt8::Max },
		};
		CheckEnumMetadata( "EnumUInt8", false, cases, (uint8_t)2, "256" );
	}
	{
		const EnumMetaCase< EnumInt16 > cases[] =
		{
			{ "Min", EnumInt16::Min },
			{ "NegativeOne", EnumInt16::NegativeOne },
			{ "Zero", EnumInt16::Zero },
			{ "Max", EnumInt16::Max },
		};
		CheckEnumMetadata( "EnumInt16", true, cases, (int16_t)1, "32768" );
	}
	{
		const EnumMetaCase< EnumUInt16 > cases[] =
		{
			{ "Zero", EnumUInt16::Zero },
			{ "One", EnumUInt16::One },
			{ "HighBit", EnumUInt16::HighBit },
			{ "Max", EnumUInt16::Max },
		};
		CheckEnumMetadata( "EnumUInt16", false, cases, (uint16_t)2, "65536" );
	}
	{
		const EnumMetaCase< EnumInt32 > cases[] =
		{
			{ "Min", EnumInt32::Min },
			{ "NegativeOne", EnumInt32::NegativeOne },
			{ "Zero", EnumInt32::Zero },
			{ "Max", EnumInt32::Max },
		};
		CheckEnumMetadata( "EnumInt32", true, cases, (int32_t)1, "2147483648" );
	}
	{
		const EnumMetaCase< EnumUInt32 > cases[] =
		{
			{ "Zero", EnumUInt32::Zero },
			{ "One", EnumUInt32::One },
			{ "HighBit", EnumUInt32::HighBit },
			{ "Max", EnumUInt32::Max },
		};
		CheckEnumMetadata( "EnumUInt32", false, cases, (uint32_t)2, "4294967296" );
	}
	{
		const EnumMetaCase< EnumInt64 > cases[] =
		{
			{ "Min", EnumInt64::Min },
			{ "NegativeOne", EnumInt64::NegativeOne },
			{ "Zero", EnumInt64::Zero },
			{ "Max", EnumInt64::Max },
		};
		CheckEnumMetadata( "EnumInt64", true, cases, (int64_t)1, "9223372036854775808" );
	}
	{
		const EnumMetaCase< EnumUInt64 > cases[] =
		{
			{ "Zero", EnumUInt64::Zero },
			{ "One", EnumUInt64::One },
			{ "HighBit", EnumUInt64::HighBit },
			{ "Max", EnumUInt64::Max },
		};
		CheckEnumMetadata( "EnumUInt64", false, cases, (uint64_t)2, "18446744073709551616" );
	}
}

TEST_CASE( "Aggregate vars", "[aeMeta]" )
{
	const ae::ClassType* type = ae::GetClassType< AggregateClass >();
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) == 2 );

	AggregateClass c;

	{
		const ae::ClassVar* someClassVar = type->GetVarByName( "someClass", false );
		REQUIRE( someClassVar );
		const ae::ClassType* someClassType = someClassVar->GetOuterVarType().AsVarType< ae::ClassType >();
		REQUIRE( someClassType );
		REQUIRE( someClassType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( someClassType->GetSize() == sizeof(SomeClass) );
#if AE_DEPRECATED
		REQUIRE( someClassVar->GetSubType() == ae::GetClassType< SomeClass >() );
		SomeClass* someClassPtr = someClassVar->GetPointer< SomeClass >( &c );
		REQUIRE( someClassPtr == &c.someClass );
		REQUIRE( someClassVar->GetPointer< SomeClass >( &c, 0 ) == nullptr );
		REQUIRE( someClassVar->GetPointer< SomeClass >( &c, 1 ) == nullptr );
		const ae::ClassType* varType = ae::GetClassTypeFromObject( someClassPtr );
		REQUIRE( varType );
		REQUIRE( varType == ae::GetClassType< SomeClass >() );
#endif // AE_DEPRECATED
	}
	{
		const ae::ClassVar* someClass1 = type->GetVarByName( "someClass1", false );
		REQUIRE( someClass1 );
		const ae::ClassType* someClass1Type = someClass1->GetOuterVarType().AsVarType< ae::ClassType >();
		REQUIRE( someClass1Type );
		REQUIRE( someClass1Type->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( someClass1Type->GetSize() == sizeof(SomeClass) );
#if AE_DEPRECATED
		REQUIRE( someClass1->GetSubType() == ae::GetClassType< SomeClass >() );
		SomeClass* someClass1Ptr = someClass1->GetPointer< SomeClass >( &c );
		REQUIRE( someClass1Ptr == &c.someClass1 );
		REQUIRE( someClass1->GetPointer< SomeClass >( &c, 0 ) == nullptr );
		REQUIRE( someClass1->GetPointer< SomeClass >( &c, 1 ) == nullptr );
		const ae::ClassType* varType = ae::GetClassTypeFromObject( someClass1Ptr );
		REQUIRE( varType );
		REQUIRE( varType == ae::GetClassType< SomeClass >() );
#endif // AE_DEPRECATED
	}
}

TEST_CASE( "Var::GetObjectValue()", "[aeMeta]" )
{
	const SomeClass c = []()
	{
		SomeClass r;
		r.intMember = 123;
		r.boolMember = true;
		r.enumTest = TestEnumClass::Five;
		for( uint32_t i = 0; i < 16; ++i )
		{
			r.uuidMember.data[ i ] = (uint8_t)i;
		}
		return r;
	}();
	const ae::ClassType* type = ae::GetClassTypeFromObject( &c );
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) == 4 );

	{
		int32_t intMember = 0;
		const ae::ClassVar* intVar = type->GetVarByName( "intMember", false );
		REQUIRE( intVar );
		const ae::BasicType* intVarType = intVar->GetOuterVarType().AsVarType< ae::BasicType >();
		REQUIRE( intVarType );
		REQUIRE( intVarType->GetType() == ae::BasicType::Int32 );
#if AE_DEPRECATED
		REQUIRE( intVar->GetObjectValue< int32_t >( &c, &intMember ) );
		REQUIRE( intMember == c.intMember );
#endif // AE_DEPRECATED
	}
	{
		bool boolMember = false;
		const ae::ClassVar* boolVar = type->GetVarByName( "boolMember", false );
		REQUIRE( boolVar );
		const ae::BasicType* boolVarType = boolVar->GetOuterVarType().AsVarType< ae::BasicType >();
		REQUIRE( boolVarType );
		REQUIRE( boolVarType->GetType() == ae::BasicType::Bool );
#if AE_DEPRECATED
		REQUIRE( boolVar->GetObjectValue< bool >( &c, &boolMember ) );
		REQUIRE( boolMember == c.boolMember );
#endif // AE_DEPRECATED
	}
	{
		TestEnumClass enumTest = TestEnumClass::Zero;
		const ae::ClassVar* enumVar = type->GetVarByName( "enumTest", false );
		REQUIRE( enumVar );
		const ae::EnumType* enumVarType = enumVar->GetOuterVarType().AsVarType< ae::EnumType >();
		REQUIRE( enumVarType );
		REQUIRE( ae::Str32( "TestEnumClass" ) == enumVarType->GetName() );
#if AE_DEPRECATED
		REQUIRE( enumVar->GetObjectValue< TestEnumClass >( &c, &enumTest ) );
		REQUIRE( enumTest == c.enumTest );
#endif // AE_DEPRECATED
	}
	{
		ae::UUID uuid;
		const ae::ClassVar* uuidVar = type->GetVarByName( "uuidMember", false );
		REQUIRE( uuidVar );
		const ae::BasicType* uuidVarType = uuidVar->GetOuterVarType().AsVarType< ae::BasicType >();
		REQUIRE( uuidVarType );
		REQUIRE( uuidVarType->GetType() == ae::BasicType::UUID );
#if AE_DEPRECATED
		REQUIRE( uuidVar->GetObjectValue< ae::UUID >( &c, &uuid ) );
		REQUIRE( uuid == c.uuidMember );
		const ae::Str64 uuidStrExpected = "00010203-0405-0607-0809-0a0b0c0d0e0f";
		REQUIRE( ae::ToString( uuid ) == uuidStrExpected );
		const auto strActual = uuidVar->GetObjectValueAsString( &c );
		REQUIRE( strActual == uuidStrExpected );
#endif // AE_DEPRECATED
	}
}

TEST_CASE( "Null DataPointer", "[aeMeta]" )
{
	const ae::ClassType* type = ae::GetClassType< ArrayClass >();
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) == 6 );
	const ae::ClassVar* classVar = type->GetVarByIndex( 0, false );
	ArrayClass c;

	ae::DataPointer dataDefault;
	ae::DataPointer dataTemplate( (ArrayClass*)nullptr );
	ae::DataPointer dataVarType( *type, nullptr );
	ae::DataPointer dataClassVar0( classVar, nullptr );
	ae::DataPointer dataClassVar1( nullptr, &c );
	REQUIRE( !dataDefault );
	REQUIRE( !dataTemplate );
	REQUIRE( !dataVarType );
	REQUIRE( !dataClassVar0 );
	REQUIRE( !dataClassVar1 );
	AE_REQUIRE_THROWS( dataDefault.GetVarType() );
	AE_REQUIRE_THROWS( dataTemplate.GetVarType() );
	AE_REQUIRE_THROWS( dataVarType.GetVarType() );
	AE_REQUIRE_THROWS( dataClassVar0.GetVarType() );
	AE_REQUIRE_THROWS( dataClassVar1.GetVarType() );
	
	ae::ConstDataPointer constDataDefault;
	ae::ConstDataPointer constDataTemplate( (ArrayClass*)nullptr );
	ae::ConstDataPointer constDataVarType( *type, nullptr );
	ae::ConstDataPointer constDataClassVar0( classVar, nullptr );
	ae::ConstDataPointer constDataClassVar1( nullptr, &c );
	REQUIRE( !constDataDefault );
	REQUIRE( !constDataTemplate );
	REQUIRE( !constDataVarType );
	REQUIRE( !constDataClassVar0 );
	REQUIRE( !constDataClassVar1 );
	AE_REQUIRE_THROWS( constDataDefault.GetVarType() );
	AE_REQUIRE_THROWS( constDataTemplate.GetVarType() );
	AE_REQUIRE_THROWS( constDataVarType.GetVarType() );
	AE_REQUIRE_THROWS( constDataClassVar0.GetVarType() );
	AE_REQUIRE_THROWS( constDataClassVar1.GetVarType() );
}

TEST_CASE( "Array vars", "[aeMeta]" )
{
	const ae::ClassType* type = ae::GetClassType< ArrayClass >();
	REQUIRE( type );
	REQUIRE( type->GetVarCount( false ) == 6 );

	ArrayClass c;
	ae::DataPointer varData( &c );

	// int32_t intArray[ 3 ];
	{
		const ae::ClassVar* intArrayVar = type->GetVarByName( "intArray", false );
		REQUIRE( intArrayVar );

		ae::DataPointer intArray( intArrayVar, &c );
		ae::DataPointer _intArray( &c.intArray );
		REQUIRE( intArray );
		REQUIRE( _intArray );
		REQUIRE( intArray == _intArray );
		REQUIRE( &intArray.GetVarType() == &intArrayVar->GetOuterVarType() );
		const ae::ArrayType* arrayVarType = intArrayVar->GetOuterVarType().AsVarType< ae::ArrayType >();
		REQUIRE( arrayVarType );
		REQUIRE( arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( intArray ) == 3 );
		REQUIRE( arrayVarType->GetMaxLength() == 3 );

		const ae::BasicType* basicVarType = arrayVarType->GetInnerVarType().AsVarType< ae::BasicType >();
		REQUIRE( basicVarType );
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );
		REQUIRE( basicVarType->GetSize() == sizeof(int32_t) );

#if AE_DEPRECATED
		// @TODO: Old, replace with Type functions
		REQUIRE( intArrayVar->IsArray() );
		REQUIRE( intArrayVar->IsArrayFixedLength() );
		REQUIRE( intArrayVar->GetArrayLength( &c ) == 3 );
		REQUIRE( intArrayVar->GetArrayMaxLength() == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 0 ) == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 4 ) == 3 );
		REQUIRE( intArrayVar->SetArrayLength( &c, 5 ) == 3 );
#endif // AE_DEPRECATED
	}
	// ae::Array< int32_t, 4 > intArray2;
	{
		const ae::ClassVar* intArray2Var = type->GetVarByName( "intArray2", false );
		REQUIRE( intArray2Var );

		ae::DataPointer intArray2( intArray2Var, &c );
		ae::DataPointer _intArray2( &c.intArray2 );
		REQUIRE( intArray2 );
		REQUIRE( _intArray2 );
		REQUIRE( intArray2 == _intArray2 );
		REQUIRE( &intArray2.GetVarType() == &intArray2Var->GetOuterVarType() );
		const ae::ArrayType* arrayVarType = intArray2Var->GetOuterVarType().AsVarType< ae::ArrayType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( intArray2 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == 4 );
		const ae::BasicType* basicVarType = arrayVarType->GetInnerVarType().AsVarType< ae::BasicType >();
		REQUIRE( basicVarType );
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );
		REQUIRE( basicVarType->GetSize() == sizeof(int32_t) );

		// @TODO: Old, replace with Type functions
#if AE_DEPRECATED
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
#endif // AE_DEPRECATED
	}
	// ae::Array< int32_t > intArray3 = AE_ALLOC_TAG_META_TEST;
	{
		const ae::ClassVar* intArray3Var = type->GetVarByName( "intArray3", false );
		REQUIRE( intArray3Var );
		
		ae::DataPointer intArray3( intArray3Var, &c );
		ae::DataPointer _intArray3( &c.intArray3 );
		REQUIRE( intArray3 );
		REQUIRE( _intArray3 );
		REQUIRE( intArray3 == _intArray3 );
		REQUIRE( &intArray3.GetVarType() == &intArray3Var->GetOuterVarType() );
		const ae::ArrayType* arrayVarType = intArray3Var->GetOuterVarType().AsVarType< ae::ArrayType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( intArray3 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == ae::MaxValue< uint32_t >() );
		const ae::BasicType* basicVarType = arrayVarType->GetInnerVarType().AsVarType< ae::BasicType >();
		REQUIRE( basicVarType );
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );
		REQUIRE( basicVarType->GetSize() == sizeof(int32_t) );

		// @TODO: Old, replace with Type functions
#if AE_DEPRECATED
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
#endif // AE_DEPRECATED
	}
	// SomeClass someClassArray[ 3 ];
	{
		const ae::ClassVar* someClassArrayVar = type->GetVarByName( "someClassArray", false );
		REQUIRE( someClassArrayVar );

		ae::DataPointer someClassArray( someClassArrayVar, &c );
		ae::DataPointer _someClassArray( &c.someClassArray );
		REQUIRE( someClassArray );
		REQUIRE( _someClassArray );
		REQUIRE( someClassArray == _someClassArray );
		REQUIRE( &someClassArray.GetVarType() == &someClassArrayVar->GetOuterVarType() );
		const ae::ArrayType* arrayVarType = someClassArrayVar->GetOuterVarType().AsVarType< ae::ArrayType >();
		REQUIRE( arrayVarType );

		REQUIRE( arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( someClassArray ) == 3 );
		REQUIRE( arrayVarType->GetMaxLength() == 3 );
		const ae::ClassType* classVarType = arrayVarType->GetInnerVarType().AsVarType< ae::ClassType >();
		REQUIRE( classVarType );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeIdWithoutQualifiers< SomeClass >() );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// @TODO: Old, replace with Type functions
#if AE_DEPRECATED
		REQUIRE( someClassArrayVar->GetSubType() == ae::GetClassType< SomeClass >() );
		REQUIRE( someClassArrayVar->IsArrayFixedLength() );
		REQUIRE( someClassArrayVar->GetArrayLength( &c ) == 3 );
		REQUIRE( someClassArrayVar->GetArrayMaxLength() == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 0 ) == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 3 ) == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 4 ) == 3 );
		REQUIRE( someClassArrayVar->SetArrayLength( &c, 5 ) == 3 );
#endif // AE_DEPRECATED
	}
	// ae::Array< SomeClass, 4 > someClassArray2;
	{
		const ae::ClassVar* someClassArray2Var = type->GetVarByName( "someClassArray2", false );
		REQUIRE( someClassArray2Var );

		ae::DataPointer someClassArray2( someClassArray2Var, &c );
		ae::DataPointer _someClassArray2( &c.someClassArray2 );
		REQUIRE( someClassArray2 );
		REQUIRE( _someClassArray2 );
		REQUIRE( someClassArray2 == _someClassArray2 );
		REQUIRE( &someClassArray2.GetVarType() == &someClassArray2Var->GetOuterVarType() );
		const ae::ArrayType* arrayVarType = someClassArray2Var->GetOuterVarType().AsVarType< ae::ArrayType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( someClassArray2 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == 4 );
		const ae::ClassType* classVarType = arrayVarType->GetInnerVarType().AsVarType< ae::ClassType >();
		REQUIRE( classVarType );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeIdWithoutQualifiers< SomeClass >() );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// @TODO: Old, replace with Type functions
#if AE_DEPRECATED
		REQUIRE( someClassArray2Var->IsArray() );
		REQUIRE( someClassArray2Var->GetSubType() == ae::GetClassType< SomeClass >() );
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
#endif // AE_DEPRECATED
	}
	// ae::Array< SomeClass > someClassArray3 = AE_ALLOC_TAG_META_TEST;
	{
		const ae::ClassVar* someClassArray3Var = type->GetVarByName( "someClassArray3", false );
		REQUIRE( someClassArray3Var );

		ae::DataPointer someClassArray3( someClassArray3Var, &c );
		ae::DataPointer _someClassArray3( &c.someClassArray3 );
		REQUIRE( someClassArray3 );
		REQUIRE( _someClassArray3 );
		REQUIRE( someClassArray3 == _someClassArray3 );
		REQUIRE( &someClassArray3.GetVarType() == &someClassArray3Var->GetOuterVarType() );
		const ae::ArrayType* arrayVarType = someClassArray3Var->GetOuterVarType().AsVarType< ae::ArrayType >();
		REQUIRE( arrayVarType );

		REQUIRE( !arrayVarType->IsFixedLength() );
		REQUIRE( arrayVarType->GetLength( someClassArray3 ) == 0 );
		REQUIRE( arrayVarType->GetMaxLength() == ae::MaxValue< uint32_t >() );
		const ae::ClassType* classVarType = arrayVarType->GetInnerVarType().AsVarType< ae::ClassType >();
		REQUIRE( classVarType );
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeIdWithoutQualifiers< SomeClass >() );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// @TODO: Old, replace with Type functions
#if AE_DEPRECATED
		REQUIRE( someClassArray3Var->IsArray() );
		REQUIRE( someClassArray3Var->GetSubType() == ae::GetClassType< SomeClass >() );
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
#endif // AE_DEPRECATED
	}
}

TEST_CASE("Optional test present", "[aeMeta]")
{
	const ae::ClassType* type = ae::GetClassType< OptionalClass >();
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
		const ae::ClassVar* intOptional = type->GetVarByName( "intOptional", false );
		REQUIRE( intOptional );
		const ae::OptionalType* optionalType = intOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
		REQUIRE( optionalType );
		const ae::BasicType* basicVarType = optionalType->GetInnerVarType().AsVarType< ae::BasicType >();
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

		// int32_t value = 0;
		// REQUIRE( c.intOptional->TryGet() );
		// REQUIRE( value == 123 );
	}
	// ae::Optional < SomeClass > someClassOptional;
	{
		const ae::ClassVar* someClassOptional = type->GetVarByName( "someClassOptional", false );
		REQUIRE( someClassOptional );
		const ae::OptionalType* optionalType = someClassOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
		REQUIRE( optionalType );
		const ae::ClassType* classVarType = optionalType->GetInnerVarType().AsVarType< ae::ClassType >();
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeIdWithoutQualifiers< SomeClass >() );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		// SomeClass value;
		// REQUIRE( someClassOptional->GetObjectValue( &c, &value ) );
		// REQUIRE( value.intMember == 123 );
		// REQUIRE( value.boolMember == true );
		// REQUIRE( value.enumTest == TestEnumClass::Five );
	}
	// // std::optional< int32_t > intStdOptional;
	// {
	// 	const ae::ClassVar* intStdOptional = type->GetVarByName( "intStdOptional", false );
	// 	REQUIRE( intStdOptional );
	// 	const ae::OptionalType* optionalType = intStdOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
	// 	REQUIRE( optionalType );
	// 	const ae::BasicType* basicVarType = optionalType->GetInnerVarType().AsVarType< ae::BasicType >();
	// 	REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

	// 	int32_t value = 0;
	// 	const ae::ConstDataPointer optional = intStdOptional->GetVarData( &c );
	// 	REQUIRE( optional );
	// 	ae::ConstDataPointer basic = optionalType->TryGet( optional );
	// 	REQUIRE( basicVarType->GetVarData( basic, &value ) );
	// 	REQUIRE( value == 456 );
	// }
	// // std::optional< SomeClass > someClassStdOptional;
	// {
	// 	const ae::ClassVar* someClassStdOptional = type->GetVarByName( "someClassStdOptional", false );
	// 	REQUIRE( someClassStdOptional );
	// 	const ae::OptionalType* optionalType = someClassStdOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
	// 	REQUIRE( optionalType );
	// 	const ae::ClassType* classVarType = optionalType->GetInnerVarType().AsVarType< ae::ClassType >();
	// 	REQUIRE( classVarType->GetTypeId() == ae::GetTypeIdWithoutQualifiers< SomeClass >() );
	// 	REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
	// 	REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

	// 	ae::ConstDataPointer optional = someClassStdOptional->GetVarData( &c );
	// 	REQUIRE( optional );
	// 	ae::ConstDataPointer _class = optionalType->TryGet( optional );
	// 	REQUIRE( _class );
	// 	const ae::ClassType* type = classVarType->GetType();
	// 	const SomeClass* someClass = classVarType->TryGet< SomeClass >( _class );
	// 	REQUIRE( someClass );
	// 	REQUIRE( someClass->intMember == 456 );
	// 	REQUIRE( someClass->boolMember == true );
	// 	REQUIRE( someClass->enumTest == TestEnumClass::Five );
	// }
}

TEST_CASE("Optional test no value", "[aeMeta]")
{
	const ae::ClassType* type = ae::GetClassType< OptionalClass >();
	REQUIRE( type );

	OptionalClass c;

	// ae::Optional< int32_t > intOptional;
	{
		const ae::ClassVar* intOptional = type->GetVarByName( "intOptional", false );
		REQUIRE( intOptional );
		const ae::OptionalType* optionalType = intOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
		REQUIRE( optionalType );
		const ae::BasicType* basicVarType = optionalType->GetInnerVarType().AsVarType< ae::BasicType >();
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

		ae::ConstDataPointer optional( intOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
	// ae::Optional < SomeClass > someClassOptional;
	{
		const ae::ClassVar* someClassOptional = type->GetVarByName( "someClassOptional", false );
		REQUIRE( someClassOptional );
		const ae::OptionalType* optionalType = someClassOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
		REQUIRE( optionalType );
		const ae::ClassType* classVarType = optionalType->GetInnerVarType().AsVarType< ae::ClassType >();
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeIdWithoutQualifiers< SomeClass >() );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		ae::ConstDataPointer optional( someClassOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
	// std::optional< int32_t > intStdOptional;
	{
		const ae::ClassVar* intStdOptional = type->GetVarByName( "intStdOptional", false );
		REQUIRE( intStdOptional );
		const ae::OptionalType* optionalType = intStdOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
		REQUIRE( optionalType );
		const ae::BasicType* basicVarType = optionalType->GetInnerVarType().AsVarType< ae::BasicType >();
		REQUIRE( basicVarType->GetType() == ae::BasicType::Int32 );

		ae::ConstDataPointer optional( intStdOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
	// std::optional< SomeClass > someClassStdOptional;
	{
		const ae::ClassVar* someClassStdOptional = type->GetVarByName( "someClassStdOptional", false );
		REQUIRE( someClassStdOptional );
		const ae::OptionalType* optionalType = someClassStdOptional->GetOuterVarType().AsVarType< ae::OptionalType >();
		REQUIRE( optionalType );
		const ae::ClassType* classVarType = optionalType->GetInnerVarType().AsVarType< ae::ClassType >();
		REQUIRE( classVarType->GetTypeId() == ae::GetTypeIdWithoutQualifiers< SomeClass >() );
		REQUIRE( classVarType->GetName() == ae::Str32( "SomeClass" ) );
		REQUIRE( classVarType->GetSize() == sizeof(SomeClass) );

		ae::ConstDataPointer optional( someClassStdOptional, &c );
		REQUIRE( optional );
		REQUIRE( !optionalType->TryGet( optional ) );
	}
}

TEST_CASE( "enum string conversions", "[aeMeta]" )
{
	const ae::EnumType* playerStateEnum = ae::GetEnumType( "PlayerState" );
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
	const ae::EnumType* playerStateEnum = ae::GetEnumType( "PlayerState" );
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
	const ae::ClassType* type = ae::GetClassTypeFromObject( &c );
	const ae::ClassVar* enumTestVar = type->GetVarByName( "enumTest", false );
	REQUIRE( enumTestVar );
	const ae::EnumType* enumVarType = enumTestVar->GetOuterVarType().AsVarType< ae::EnumType >();
	REQUIRE( enumVarType );
	
	ae::ConstDataPointer varData( enumTestVar, &c );
	
	c.enumTest = TestEnumClass::Five;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Five" );
#if AE_DEPRECATED
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Five" ); // @TODO: Remove
#endif // AE_DEPRECATED
	c.enumTest = TestEnumClass::Four;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Four" );
#if AE_DEPRECATED
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Four" ); // @TODO: Remove
#endif // AE_DEPRECATED
	c.enumTest = TestEnumClass::Three;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Three" );
#if AE_DEPRECATED
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Three" ); // @TODO: Remove
#endif // AE_DEPRECATED
	c.enumTest = TestEnumClass::Two;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Two" );
#if AE_DEPRECATED
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Two" ); // @TODO: Remove
#endif // AE_DEPRECATED
	c.enumTest = TestEnumClass::One;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "One" );
#if AE_DEPRECATED
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "One" ); // @TODO: Remove
#endif // AE_DEPRECATED
	c.enumTest = TestEnumClass::Zero;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "Zero" );
#if AE_DEPRECATED
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "Zero" ); // @TODO: Remove
#endif // AE_DEPRECATED
	c.enumTest = TestEnumClass::NegativeOne;
	REQUIRE( enumVarType->GetVarDataAsString( varData ) == "NegativeOne" );
#if AE_DEPRECATED
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "NegativeOne" ); // @TODO: Remove
#endif // AE_DEPRECATED
}

TEST_CASE( "can't read invalid enum values from object using meta definition", "[aeMeta]" )
{
	SomeClass c;
	const ae::ClassType* type = ae::GetClassTypeFromObject( &c );
	const ae::ClassVar* enumTestVar = type->GetVarByName( "enumTest", false );
	const ae::EnumType* enumVarType = enumTestVar->GetOuterVarType().AsVarType< ae::EnumType >();
	REQUIRE( enumVarType );

#if AE_DEPRECATED
	c.enumTest = (TestEnumClass)6;
	REQUIRE( enumTestVar->GetObjectValueAsString( &c ) == "" );
#endif // AE_DEPRECATED
}

TEST_CASE( "can set enum values on object using meta definition", "[aeMeta]" )
{
	SomeClass c;
	const ae::ClassType* type = ae::GetClassTypeFromObject( &c );
	const ae::ClassVar* enumTestVar = type->GetVarByName( "enumTest", false );
	const ae::EnumType* enumVarType = enumTestVar->GetOuterVarType().AsVarType< ae::EnumType >();
	REQUIRE( enumVarType );

#if AE_DEPRECATED
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
#endif // AE_DEPRECATED
}

TEST_CASE( "can't set invalid enum values on object using meta definition", "[aeMeta]" )
{
	SomeClass c;
	const ae::ClassType* type = ae::GetClassTypeFromObject( &c );
	const ae::ClassVar* enumTestVar = type->GetVarByName( "enumTest", false );
	const ae::EnumType* enumVarType = enumTestVar->GetOuterVarType().AsVarType< ae::EnumType >();
	REQUIRE( enumVarType );

#if AE_DEPRECATED
	c.enumTest = TestEnumClass::Four;
	REQUIRE( !enumTestVar->SetObjectValueFromString( &c, "Six" ) );
	REQUIRE( c.enumTest == TestEnumClass::Four );
	REQUIRE( !enumTestVar->SetObjectValueFromString( &c, "6" ) );
	REQUIRE( c.enumTest == TestEnumClass::Four );
	REQUIRE( !enumTestVar->SetObjectValueFromString( &c, "" ) );
	REQUIRE( c.enumTest == TestEnumClass::Four );
#endif // AE_DEPRECATED
}

//------------------------------------------------------------------------------
// SomeOldEnum
//------------------------------------------------------------------------------
TEST_CASE( "can register an already existing c-style enum", "[aeMeta]" )
{
	const ae::EnumType* enumType = ae::GetEnumType< SomeOldEnum >();
	REQUIRE( enumType == ae::GetEnumType( "SomeOldEnum" ) );
	REQUIRE( enumType->Length() == 3 );
	REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
	REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
	REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
	RequireEnumValueByIndex( enumType, 0, SomeOldEnum::Bleep );
	RequireEnumValueByIndex( enumType, 1, SomeOldEnum::Bloop );
	RequireEnumValueByIndex( enumType, 2, SomeOldEnum::Blop );
}

TEST_CASE( "existing c-style enum string conversions", "[aeMeta]" )
{
	const ae::EnumType* enumType = ae::GetEnumType< SomeOldEnum >();
	REQUIRE( enumType == ae::GetEnumType( "SomeOldEnum" ) );

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
	const ae::EnumType* enumType = ae::GetEnumType< SomeOldPrefixEnum >();
	REQUIRE( enumType == ae::GetEnumType( "SomeOldPrefixEnum" ) );
	REQUIRE( enumType->Length() == 3 );
	REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
	REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
	REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
	RequireEnumValueByIndex( enumType, 0, SomeOldPrefixEnum::kSomeOldPrefixEnum_Bleep );
	RequireEnumValueByIndex( enumType, 1, SomeOldPrefixEnum::kSomeOldPrefixEnum_Bloop );
	RequireEnumValueByIndex( enumType, 2, SomeOldPrefixEnum::kSomeOldPrefixEnum_Blop );
}

//------------------------------------------------------------------------------
// SomeOldRenamedEnum
//------------------------------------------------------------------------------
TEST_CASE( "can register an already existing c-style enum where each value has a manually specified name", "[aeMeta]" )
{
	const ae::EnumType* enumType = ae::GetEnumType< SomeOldRenamedEnum >();
	REQUIRE( enumType == ae::GetEnumType( "SomeOldRenamedEnum" ) );
	REQUIRE( enumType->Length() == 3 );
	REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
	REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
	REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
	RequireEnumValueByIndex( enumType, 0, SomeOldRenamedEnum::BLEEP );
	RequireEnumValueByIndex( enumType, 1, SomeOldRenamedEnum::BLOOP );
	RequireEnumValueByIndex( enumType, 2, SomeOldRenamedEnum::BLOP );
}

//------------------------------------------------------------------------------
// SomeNewEnum
//------------------------------------------------------------------------------
TEST_CASE( "can register an already existing enum class", "[aeMeta]" )
{
	const ae::EnumType* enumType = ae::GetEnumType< SomeNewEnum >();
	REQUIRE( enumType == ae::GetEnumType( "SomeNewEnum" ) );
	REQUIRE( enumType->Length() == 3 );
	REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
	REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
	REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
	RequireEnumValueByIndex( enumType, 0, SomeNewEnum::Bleep );
	RequireEnumValueByIndex( enumType, 1, SomeNewEnum::Bloop );
	RequireEnumValueByIndex( enumType, 2, SomeNewEnum::Blop );
}

//------------------------------------------------------------------------------
// A::B::SomeNewEnum
//------------------------------------------------------------------------------
TEST_CASE( "can register an already existing enum class in a nested namespace", "[aeMeta]" )
{
	const ae::EnumType* enumType = ae::GetEnumType< A::B::SomeNewEnum >();
	REQUIRE( enumType == ae::GetEnumType( "A::B::SomeNewEnum" ) );
	REQUIRE( enumType->Length() == 3 );
	REQUIRE( enumType->GetNameByIndex( 0 ) == "Bleep" );
	REQUIRE( enumType->GetNameByIndex( 1 ) == "Bloop" );
	REQUIRE( enumType->GetNameByIndex( 2 ) == "Blop" );
	RequireEnumValueByIndex( enumType, 0, A::B::SomeNewEnum::Bleep );
	RequireEnumValueByIndex( enumType, 1, A::B::SomeNewEnum::Bloop );
	RequireEnumValueByIndex( enumType, 2, A::B::SomeNewEnum::Blop );
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
	const uint32_t id = (uint32_t)strtoul( str, &endPtr, 10 );
	if( endPtr != str )
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
	const ae::ClassType* type_RefTester = ae::GetClassType< RefTester >();
	REQUIRE( type_RefTester );

	const ae::ClassType* type_RefTesterA = ae::GetClassType< RefTesterA >();
	REQUIRE( type_RefTesterA );
	const ae::ClassVar* var_RefTesterA_notRef = type_RefTesterA->GetVarByName( "notRef", false );
	const ae::ClassVar* var_RefTesterA_refA = type_RefTesterA->GetVarByName( "refA", false );
	const ae::ClassVar* var_RefTesterA_refB = type_RefTesterA->GetVarByName( "refB", false );
	REQUIRE( var_RefTesterA_notRef );
	REQUIRE( var_RefTesterA_refA );
	REQUIRE( var_RefTesterA_refB );
	
	const ae::ClassType* type_RefTesterB = ae::GetClassType< RefTesterB >();
	REQUIRE( type_RefTesterB );
	const ae::ClassVar* var_RefTesterB_ref = type_RefTesterB->GetVarByName( "ref", false );
	REQUIRE( var_RefTesterB_ref );
	
#if AE_DEPRECATED
	REQUIRE( !var_RefTesterA_notRef->GetSubType() );
	REQUIRE( var_RefTesterA_refA->GetSubType() == type_RefTesterA );
	REQUIRE( var_RefTesterA_refB->GetSubType() == type_RefTesterB );
	REQUIRE( var_RefTesterB_ref->GetSubType() == type_RefTester );
	class RefSerializer : public ae::ClassVar::Serializer
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
			if( RefTester::StringToId( pointerVal, &id ) )
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
	ae::ClassVar::SetSerializer( &refSerializer );
	
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
#endif // AE_DEPRECATED
}

TEST_CASE( "BasicType can read and write values via DataPointer", "[aeMeta]" )
{
	SomeClass c;
	c.intMember = 42;
	c.boolMember = true;

	const ae::ClassType* type = ae::GetClassType< SomeClass >();
	const ae::ClassVar* intVar = type->GetVarByName( "intMember", false );
	const ae::ClassVar* boolVar = type->GetVarByName( "boolMember", false );

	const ae::BasicType* intVarType = intVar->GetOuterVarType().AsVarType< ae::BasicType >();
	const ae::BasicType* boolVarType = boolVar->GetOuterVarType().AsVarType< ae::BasicType >();
	REQUIRE( intVarType );
	REQUIRE( boolVarType );

	ae::DataPointer intData( intVar, &c );
	ae::DataPointer boolData( boolVar, &c );

	// GetVarData reads the correct int value
	int32_t intOut = 0;
	REQUIRE( intVarType->GetVarData( ae::ConstDataPointer( intData ), &intOut ) );
	REQUIRE( intOut == 42 );

	// Type mismatch returns false
	bool boolOut = false;
	REQUIRE( !intVarType->GetVarData( ae::ConstDataPointer( intData ), &boolOut ) );

	// SetVarData writes correctly
	REQUIRE( intVarType->SetVarData( intData, (int32_t)99 ) );
	REQUIRE( c.intMember == 99 );

	// Type mismatch SetVarData returns false
	REQUIRE( !intVarType->SetVarData( intData, true ) );

	// GetVarDataAsString returns decimal string for int
	REQUIRE( intVarType->GetVarDataAsString( ae::ConstDataPointer( intData ) ) == "99" );

	// SetVarDataFromString parses decimal string
	REQUIRE( intVarType->SetVarDataFromString( intData, "77" ) );
	REQUIRE( c.intMember == 77 );

	// Non-numeric string: SetVarDataFromString returns false, member unchanged
	REQUIRE( !intVarType->SetVarDataFromString( intData, "hello" ) );
	REQUIRE( c.intMember == 77 );

	// bool: GetVarData reads the correct value
	REQUIRE( boolVarType->GetVarData( ae::ConstDataPointer( boolData ), &boolOut ) );
	REQUIRE( boolOut == true );

	REQUIRE( boolVarType->SetVarData( boolData, false ) );
	REQUIRE( c.boolMember == false );

	// GetVarDataAsString returns "false"/"true" for bool
	REQUIRE( boolVarType->GetVarDataAsString( ae::ConstDataPointer( boolData ) ) == "false" );
	REQUIRE( boolVarType->SetVarData( boolData, true ) );
	REQUIRE( boolVarType->GetVarDataAsString( ae::ConstDataPointer( boolData ) ) == "true" );

	// SetVarDataFromString parses "false" and "true"
	REQUIRE( boolVarType->SetVarDataFromString( boolData, "false" ) );
	REQUIRE( c.boolMember == false );
	REQUIRE( boolVarType->SetVarDataFromString( boolData, "true" ) );
	REQUIRE( c.boolMember == true );

	// Null DataPointer returns false / empty string for all ops
	ae::DataPointer nullData;
	REQUIRE( !intVarType->GetVarData( ae::ConstDataPointer( nullData ), &intOut ) );
	REQUIRE( !intVarType->SetVarData( nullData, (int32_t)1 ) );
	REQUIRE( intVarType->GetVarDataAsString( ae::ConstDataPointer( nullData ) ) == "" );
	REQUIRE( !intVarType->SetVarDataFromString( nullData, "1" ) );
}

TEST_CASE( "EnumType can read and write values via DataPointer", "[aeMeta]" )
{
	SomeClass c;
	c.enumTest = TestEnumClass::Two;

	const ae::ClassType* type = ae::GetClassType< SomeClass >();
	const ae::ClassVar* enumVar = type->GetVarByName( "enumTest", false );
	const ae::EnumType* enumVarType = enumVar->GetOuterVarType().AsVarType< ae::EnumType >();
	REQUIRE( enumVarType );

	ae::DataPointer enumData( enumVar, &c );

	// GetVarData reads correctly
	TestEnumClass enumOut = TestEnumClass::Zero;
	REQUIRE( enumVarType->GetVarData( ae::ConstDataPointer( enumData ), &enumOut ) );
	REQUIRE( enumOut == TestEnumClass::Two );

	// SetVarData writes correctly
	REQUIRE( enumVarType->SetVarData( enumData, TestEnumClass::Five ) );
	REQUIRE( c.enumTest == TestEnumClass::Five );

	// GetVarData succeeds for any integral T (int32_t reads the underlying value)
	int32_t intOut = 0;
	REQUIRE( enumVarType->GetVarData( ae::ConstDataPointer( enumData ), &intOut ) );
	REQUIRE( intOut == (int32_t)TestEnumClass::Five );

	// Type mismatch: wrong enum type returns false (PlayerState != TestEnumClass)
	PlayerState playerStateOut = PlayerState::Idle;
	REQUIRE( !enumVarType->GetVarData( ae::ConstDataPointer( enumData ), &playerStateOut ) );
	REQUIRE( !enumVarType->SetVarData( enumData, PlayerState::Run ) );

	// SetVarDataFromString with valid name succeeds
	REQUIRE( enumVarType->SetVarDataFromString( enumData, "Three" ) );
	REQUIRE( c.enumTest == TestEnumClass::Three );

	// SetVarDataFromString with numeric string succeeds
	REQUIRE( enumVarType->SetVarDataFromString( enumData, "2" ) );
	REQUIRE( c.enumTest == TestEnumClass::Two );

	// Unknown name returns false and leaves value unchanged
	REQUIRE( !enumVarType->SetVarDataFromString( enumData, "Unknown" ) );
	REQUIRE( c.enumTest == TestEnumClass::Two );

	// Permissive numeric parsing: leading whitespace and + prefix
	REQUIRE( enumVarType->SetVarDataFromString( enumData, " 1" ) );
	REQUIRE( c.enumTest == TestEnumClass::One );
	REQUIRE( enumVarType->SetVarDataFromString( enumData, "+2" ) );
	REQUIRE( c.enumTest == TestEnumClass::Two );

	// GetVarDataAsString returns empty string for value not in the enum
	c.enumTest = (TestEnumClass)99;
	REQUIRE( enumVarType->GetVarDataAsString( ae::ConstDataPointer( enumData ) ) == "" );
}

TEST_CASE( "ArrayType can resize and access elements via DataPointer", "[aeMeta]" )
{
	ArrayClass c;

	const ae::ClassType* type = ae::GetClassType< ArrayClass >();
	const ae::ClassVar* intArrayVar = type->GetVarByName( "intArray", false );
	const ae::ClassVar* intArray3Var = type->GetVarByName( "intArray3", false );
	REQUIRE( intArrayVar );
	REQUIRE( intArray3Var );

	const ae::ArrayType* fixedArrayType = intArrayVar->GetOuterVarType().AsVarType< ae::ArrayType >();
	const ae::ArrayType* dynArrayType = intArray3Var->GetOuterVarType().AsVarType< ae::ArrayType >();
	REQUIRE( fixedArrayType );
	REQUIRE( dynArrayType );

	ae::DataPointer fixedData( intArrayVar, &c );
	ae::DataPointer dynData( intArray3Var, &c );

	// Resize dynamic array: grow to 3
	REQUIRE( dynArrayType->Resize( dynData, 3 ) == 3 );
	REQUIRE( dynArrayType->GetLength( ae::ConstDataPointer( dynData ) ) == 3 );

	// Shrink to 1
	REQUIRE( dynArrayType->Resize( dynData, 1 ) == 1 );
	REQUIRE( dynArrayType->GetLength( ae::ConstDataPointer( dynData ) ) == 1 );

	// Resize to 0
	REQUIRE( dynArrayType->Resize( dynData, 0 ) == 0 );
	REQUIRE( dynArrayType->GetLength( ae::ConstDataPointer( dynData ) ) == 0 );

	// Fixed C-array resize to max (3) returns 3; length always equals max
	REQUIRE( fixedArrayType->Resize( fixedData, 3 ) == 3 );
	REQUIRE( fixedArrayType->GetLength( ae::ConstDataPointer( fixedData ) ) == 3 );

	// Resize beyond capacity clamps to 3
	REQUIRE( fixedArrayType->Resize( fixedData, 5 ) == 3 );

	// GetElement returns valid DataPointer for a valid index
	REQUIRE( dynArrayType->Resize( dynData, 3 ) == 3 );
	ae::DataPointer elem0 = dynArrayType->GetElement( dynData, 0 );
	REQUIRE( elem0 );

	// Out-of-bounds index returns null DataPointer
	ae::DataPointer elemOob = dynArrayType->GetElement( dynData, 10 );
	REQUIRE( !elemOob );

	// Write through mutable GetElement pointer and verify via object member
	const ae::BasicType* intBasicType = dynArrayType->GetInnerVarType().AsVarType< ae::BasicType >();
	REQUIRE( intBasicType );
	REQUIRE( intBasicType->SetVarData( elem0, (int32_t)777 ) );
	REQUIRE( c.intArray3[ 0 ] == 777 );
}

TEST_CASE( "OptionalType can insert and clear values via DataPointer", "[aeMeta]" )
{
	// ae::Optional TypeT is a stub; test std::optional only
	OptionalClass c;

	const ae::ClassType* type = ae::GetClassType< OptionalClass >();
	const ae::ClassVar* intStdOptionalVar = type->GetVarByName( "intStdOptional", false );
	const ae::ClassVar* someClassStdOptionalVar = type->GetVarByName( "someClassStdOptional", false );
	REQUIRE( intStdOptionalVar );
	REQUIRE( someClassStdOptionalVar );

	const ae::OptionalType* intOptType =
		intStdOptionalVar->GetOuterVarType().AsVarType< ae::OptionalType >();
	const ae::OptionalType* classOptType =
		someClassStdOptionalVar->GetOuterVarType().AsVarType< ae::OptionalType >();
	REQUIRE( intOptType );
	REQUIRE( classOptType );

	ae::DataPointer intOptData( intStdOptionalVar, &c );
	ae::DataPointer classOptData( someClassStdOptionalVar, &c );

	// GetOrInsert on empty optional constructs a value and returns non-null
	REQUIRE( !c.intStdOptional.has_value() );
	ae::DataPointer intInserted = intOptType->GetOrInsert( intOptData );
	REQUIRE( intInserted );
	REQUIRE( c.intStdOptional.has_value() );

	// TryGet on the now-present value returns non-null
	REQUIRE( intOptType->TryGet( intOptData ) );

	// Clear destroys the value; TryGet returns null afterward
	intOptType->Clear( intOptData );
	REQUIRE( !c.intStdOptional.has_value() );
	REQUIRE( !intOptType->TryGet( intOptData ) );

	// Same for std::optional< SomeClass >
	REQUIRE( !c.someClassStdOptional.has_value() );
	ae::DataPointer classInserted = classOptType->GetOrInsert( classOptData );
	REQUIRE( classInserted );
	REQUIRE( c.someClassStdOptional.has_value() );
	REQUIRE( classOptType->TryGet( classOptData ) );

	classOptType->Clear( classOptData );
	REQUIRE( !c.someClassStdOptional.has_value() );
	REQUIRE( !classOptType->TryGet( classOptData ) );
}

TEST_CASE( "PointerType can read and write pointer values via DataPointer", "[aeMeta]" )
{
	RefTesterA objA;
	RefTesterA objA2;
	RefTesterB objB;

	const ae::ClassType* typeRefTesterA = ae::GetClassType< RefTesterA >();
	const ae::ClassType* typeRefTesterB = ae::GetClassType< RefTesterB >();
	const ae::ClassType* typeRefTester  = ae::GetClassType< RefTester >();
	REQUIRE( typeRefTesterA );
	REQUIRE( typeRefTesterB );
	REQUIRE( typeRefTester );

	const ae::ClassVar* varRefA = typeRefTesterA->GetVarByName( "refA", false );
	const ae::ClassVar* varRef  = typeRefTesterB->GetVarByName( "ref", false );
	REQUIRE( varRefA );
	REQUIRE( varRef );

	// Pointer vars expose PointerType as outer var type
	const ae::PointerType* refAType = varRefA->GetOuterVarType().AsVarType< ae::PointerType >();
	const ae::PointerType* refType  = varRef->GetOuterVarType().AsVarType< ae::PointerType >();
	REQUIRE( refAType );
	REQUIRE( refType );

	// GetInnerVarType for refA matches RefTesterA
	REQUIRE( refAType->GetInnerVarType().AsVarType< ae::ClassType >() == typeRefTesterA );
	// GetInnerVarType for ref matches RefTester (base class)
	REQUIRE( refType->GetInnerVarType().AsVarType< ae::ClassType >() == typeRefTester );

	ae::DataPointer ptrRefA( varRefA, &objA );
	ae::DataPointer ptrRef( varRef, &objB );

	// SetRef sets the pointer
	REQUIRE( refAType->Set( ptrRefA, &objA2 ) );
	REQUIRE( objA.refA == &objA2 );

	// SetRef with nullptr clears the pointer
	REQUIRE( refAType->Set( ptrRefA, nullptr ) );
	REQUIRE( objA.refA == nullptr );

	// SetRef stores the pointer; verify via direct member access
	REQUIRE( refAType->Set( ptrRefA, &objA2 ) );
	REQUIRE( objA.refA == &objA2 );

	// Dereference on a non-null pointer returns a non-null DataPointer
	ae::DataPointer derefed = refAType->Dereference( ae::ConstDataPointer( ptrRefA ) );
	REQUIRE( derefed );

	// Dereference on a null pointer returns a null DataPointer
	REQUIRE( refAType->Set( ptrRefA, nullptr ) );
	ae::DataPointer derefedNull = refAType->Dereference( ae::ConstDataPointer( ptrRefA ) );
	REQUIRE( !derefedNull );

	// Get<T> returns T** when T matches the inner pointer type
	REQUIRE( refAType->Set( ptrRefA, &objA2 ) );
	RefTesterA** rawRef = refAType->Get< RefTesterA >( ptrRefA );
	REQUIRE( rawRef );
	REQUIRE( *rawRef == &objA2 );

	// Get<T> const variant also returns the stored pointer
	RefTesterA* const* rawRefConst = refAType->Get< RefTesterA >( ae::ConstDataPointer( ptrRefA ) );
	REQUIRE( rawRefConst );
	REQUIRE( *rawRefConst == &objA2 );

	// Get<T> returns null when T does not match the inner pointer type
	REQUIRE( refAType->Get< RefTesterB >( ptrRefA ) == nullptr );
}

TEST_CASE( "MapType can read and write map values via DataPointer", "[aeMeta]" )
{
	MapClass c;

	const ae::ClassType* type = ae::GetClassType< MapClass >();
	REQUIRE( type );

	const ae::ClassVar* intMapVar      = type->GetVarByName( "intMap", false );
	const ae::ClassVar* intMapFixedVar = type->GetVarByName( "intMapFixed", false );
	REQUIRE( intMapVar );
	REQUIRE( intMapFixedVar );

	// Outer var type is MapType
	const ae::MapType* mapType      = intMapVar->GetOuterVarType().AsVarType< ae::MapType >();
	const ae::MapType* fixedMapType = intMapFixedVar->GetOuterVarType().AsVarType< ae::MapType >();
	REQUIRE( mapType );
	REQUIRE( fixedMapType );

	// Key type is BasicType::String (ae::Str32)
	const ae::BasicType* keyType = mapType->GetKeyVarType().AsVarType< ae::BasicType >();
	REQUIRE( keyType );
	REQUIRE( keyType->GetType() == ae::BasicType::String );

	// Value type is BasicType::Int32
	const ae::BasicType* valueType = mapType->GetValueVarType().AsVarType< ae::BasicType >();
	REQUIRE( valueType );
	REQUIRE( valueType->GetType() == ae::BasicType::Int32 );

	ae::DataPointer mapData( intMapVar, &c );
	ae::DataPointer fixedMapData( intMapFixedVar, &c );

	// Initial length is 0
	REQUIRE( mapType->GetLength( ae::ConstDataPointer( mapData ) ) == 0 );

	// Dynamic map has no hard length limit
	REQUIRE( mapType->GetMaxLength() == ae::MaxValue< uint32_t >() );
	// Fixed map capacity is 4
	REQUIRE( fixedMapType->GetMaxLength() == 4 );

	// Get() inserts key if absent and returns non-null value DataPointer
	const ae::Str32 keyHello( "hello" );
	const ae::ConstDataPointer keyData( &keyHello );
	ae::DataPointer valPtr = mapType->Get( mapData, keyData );
	REQUIRE( valPtr );

	// Write a value through the returned DataPointer and verify via member
	REQUIRE( valueType->SetVarData( valPtr, (int32_t)42 ) );
	REQUIRE( c.intMap.Get( keyHello, 0 ) == 42 );

	// TryGet for an existing key returns non-null
	REQUIRE( mapType->TryGet( mapData, keyData ) );

	// TryGet for a missing key returns null
	const ae::Str32 keyMissing( "missing" );
	REQUIRE( !mapType->TryGet( mapData, ae::ConstDataPointer( &keyMissing ) ) );

	// Insert a second entry and verify index-based accessors
	const ae::Str32 keyWorld( "world" );
	ae::DataPointer valPtr2 = mapType->Get( mapData, ae::ConstDataPointer( &keyWorld ) );
	REQUIRE( valueType->SetVarData( valPtr2, (int32_t)99 ) );

	// Length increases after inserts
	REQUIRE( mapType->GetLength( ae::ConstDataPointer( mapData ) ) == 2 );

	// GetKey/GetValue by index return valid DataPointers
	REQUIRE( mapType->GetKey( ae::ConstDataPointer( mapData ), 0 ) );
	REQUIRE( mapType->GetValue( ae::ConstDataPointer( mapData ), 0 ) );
}

TEST_CASE( "ClassType hierarchy and DataPointer operations", "[aeMeta]" )
{
	const ae::ClassType* typeObject       = ae::GetClassType< ae::Object >();
	const ae::ClassType* typeSomeClass    = ae::GetClassType< SomeClass >();
	const ae::ClassType* typeNamespace    =
		ae::GetClassTypeByName( "Namespace0::Namespace1::NamespaceClass" );
	const ae::ClassType* typeRefTester    = ae::GetClassType< RefTester >();
	const ae::ClassType* typeRefTesterA   = ae::GetClassType< RefTesterA >();
	REQUIRE( typeObject );
	REQUIRE( typeSomeClass );
	REQUIRE( typeNamespace );
	REQUIRE( typeRefTester );
	REQUIRE( typeRefTesterA );

	// GetId() equals FNV1a-32 hash of the registered type name
	REQUIRE( typeSomeClass->GetId() ==
		ae::TypeId( ae::Hash32().HashString( typeSomeClass->GetName() ).Get() ) );
	// GetId() matches GetTypeIdFromName()
	REQUIRE( typeSomeClass->GetId() == ae::GetTypeIdFromName( typeSomeClass->GetName() ) );

	// GetParentTypeName for SomeClass is "ae::Object"
	REQUIRE( ae::Str32( typeSomeClass->GetParentTypeName() ) == "ae::Object" );

	// GetParentType for SomeClass returns ae::Object's ClassType
	REQUIRE( typeSomeClass->GetParentType() == typeObject );

	// GetParentType for ae::Object returns nullptr (no registered parent)
	REQUIRE( typeObject->GetParentType() == nullptr );

	// IsType with a base class returns true
	REQUIRE( typeSomeClass->IsType( typeObject ) );

	// IsType<SomeClass> is true for SomeClass itself
	REQUIRE( typeSomeClass->IsType< SomeClass >() );
	// IsType<SomeClass> is true for a derived type
	REQUIRE( typeNamespace->IsType< SomeClass >() );
	// IsType<SomeClass> is false for an unrelated base type
	REQUIRE( !typeObject->IsType< SomeClass >() );

	// TryGet<SomeClass> on a DataPointer holding a NamespaceClass succeeds
	Namespace0::Namespace1::NamespaceClass nsObj;
	ae::DataPointer nsData( &nsObj );
	REQUIRE( typeNamespace->TryGet< SomeClass >( nsData ) );

	// TryGet<NamespaceClass> on a DataPointer holding a SomeClass fails
	SomeClass scObj;
	ae::DataPointer scData( &scObj );
	REQUIRE( !typeSomeClass->TryGet< Namespace0::Namespace1::NamespaceClass >( scData ) );

	// GetVarData returns a DataPointer addressing the member; read it back
	const ae::ClassVar* intVar = typeSomeClass->GetVarByName( "intMember", false );
	REQUIRE( intVar );
	scObj.intMember = 55;
	ae::ConstDataPointer memberData = typeSomeClass->GetVarData( intVar, ae::ConstDataPointer( scData ) );
	REQUIRE( memberData );
	const ae::BasicType* intType = intVar->GetOuterVarType().AsVarType< ae::BasicType >();
	int32_t readBack = 0;
	REQUIRE( intType->GetVarData( memberData, &readBack ) );
	REQUIRE( readBack == 55 );

	// New(): allocate aligned storage, placement-construct, verify type
	void* storage = ae::Allocate( AE_ALLOC_TAG_META_TEST, typeSomeClass->GetSize(),
		typeSomeClass->GetAlignment() );
	SomeClass* newObj = typeSomeClass->New< SomeClass >( storage );
	REQUIRE( newObj );
	REQUIRE( typeSomeClass->IsType( ae::GetClassTypeFromObject( newObj ) ) );
	newObj->~SomeClass();
	ae::Free( storage );
}

TEST_CASE( "bitfield registration", "[aeMeta]" )
{
	const ae::EnumType* gamePadBitFieldEnum = ae::GetEnumType( "GamePadBitField" );

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
