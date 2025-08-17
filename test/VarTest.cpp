//------------------------------------------------------------------------------
// VarTest.cpp
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
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
TEST_CASE( "DataPointer", "[aeMeta]" )
{
	REQUIRE( ae::DataPointer() == ae::DataPointer() );
	REQUIRE( ae::ConstDataPointer() == ae::ConstDataPointer() );
}

TEST_CASE( "ClassType", "[aeMeta]" )
{
	SomeClass someClass;
	ae::DataPointer someClassPointer( &someClass );
	REQUIRE( &someClassPointer.GetVarType() == ae::GetClassType< SomeClass >() );
	REQUIRE( someClassPointer.GetVarType().AsVarType< ae::ClassType >() == ae::GetClassType< SomeClass >() );
	REQUIRE( someClassPointer.GetVarType().GetExactVarTypeId().id == ae::GetClassType< SomeClass >()->GetId().id );
}

TEST_CASE( "StaticArrayVarType", "[aeMeta]" )
{
	ae::Array< int32_t, 5 > _array;
	ae::DataPointer array( &_array );
	REQUIRE( array.GetVarType().IsSameBaseVarType< ae::ArrayType >() );
	const ae::ArrayType* varType = array.GetVarType().AsVarType< ae::ArrayType >();
	REQUIRE( varType );
	REQUIRE( varType->GetInnerVarType().IsSameBaseVarType< ae::BasicType >() );
	const ae::BasicType* innerVarType = varType->GetInnerVarType().AsVarType< ae::BasicType >();
	REQUIRE( innerVarType );

	REQUIRE( varType->IsFixedLength() == false );
	REQUIRE( varType->GetMaxLength() == 5 );
	REQUIRE( innerVarType->GetType() == ae::BasicType::Int32 );

	// Set array length
	REQUIRE( _array.Length() == 0 );
	REQUIRE( varType->Resize( array, 2 ) == 2 );
	REQUIRE( _array.Length() == 2 );

	// Access elements
	REQUIRE( _array[ 0 ] == 0 );
	REQUIRE( _array[ 1 ] == 0 );
	REQUIRE( varType->GetElement( array, -1 ) == ae::DataPointer() );
	ae::DataPointer elem0 = varType->GetElement( array, 0 );
	ae::DataPointer elem1 = varType->GetElement( array, 1 );
	REQUIRE( varType->GetElement( array, 2 ) == ae::DataPointer() );
	REQUIRE( elem0 );
	REQUIRE( elem1 );
	// Set elements
	REQUIRE( innerVarType->SetVarData( elem0, 1000 ) );
	REQUIRE( innerVarType->SetVarDataFromString( elem1, "1001" ) );
	REQUIRE( _array[ 0 ] == 1000 );
	REQUIRE( _array[ 1 ] == 1001 );
	// Get elements
	int32_t val0 = 0;
	REQUIRE( innerVarType->GetVarData( elem0, &val0 ) );
	REQUIRE( val0 == 1000 );
	REQUIRE( innerVarType->GetVarDataAsString( elem1 ) == "1001" );
}

TEST_CASE( "MapType Static", "[aeMeta]" )
{
	ae::Map< ae::Str32, int32_t, 3 > _map;
	ae::DataPointer map( &_map );

	REQUIRE( map.GetVarType().IsSameBaseVarType< ae::MapType >() );
	const ae::MapType* mapVarType = map.GetVarType().AsVarType< ae::MapType >();
	REQUIRE( mapVarType );
	REQUIRE( mapVarType->GetMaxLength() == 3 );
	
	const ae::BasicType* keyVarType = mapVarType->GetKeyVarType().AsVarType< ae::BasicType >();
	const ae::BasicType* valueVarType = mapVarType->GetValueVarType().AsVarType< ae::BasicType >();
	REQUIRE( keyVarType );
	REQUIRE( valueVarType );
	REQUIRE( keyVarType->GetType() == ae::BasicType::String );
	REQUIRE( valueVarType->GetType() == ae::BasicType::Int32 );

	const ae::Str32 keyStr0 = "Something1000";
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1000 ) );
		REQUIRE( _map.Get( keyStr0.c_str() ) == 1000 );
	}
	{
		int32_t value = -777;
		ae::DataPointer valueVarData = mapVarType->TryGet( map, ae::ConstDataPointer( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == 1000 );
	}
	{
		int32_t value = -777;
		ae::ConstDataPointer valueVarData = mapVarType->TryGet( ae::ConstDataPointer( map ), ae::ConstDataPointer( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == 1000 );
	}

	const ae::Str32 keyStr1 = "Something1001";
	{
		int32_t value = -777;
		ae::ConstDataPointer valueVarData = mapVarType->TryGet( ae::ConstDataPointer( map ), ae::ConstDataPointer( &keyStr1 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( !valueVarData );
		REQUIRE( !valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == -777 );
	}
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr1 ) );
		REQUIRE( _map.Length() == 2 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1001 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1001 );
	}
	{
		ae::DataPointer valueVarData = mapVarType->TryGet( map, ae::ConstDataPointer( &keyStr1 ) );
		REQUIRE( _map.Length() == 2 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1101 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
	}

	const ae::Str32 keyStr2 = "Something1002";
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr2 ) );
		REQUIRE( _map.Length() == 3 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1002 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
		REQUIRE( _map.Get( "Something1002" ) == 1002 );
	}

	const ae::Str32 keyStr3 = "Something1003";
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr3 ) );
		REQUIRE( _map.Length() == 3 );
		REQUIRE( !valueVarData );
		REQUIRE( !valueVarType->SetVarData( valueVarData, 1003 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
		REQUIRE( _map.Get( "Something1002" ) == 1002 );
	}
}

TEST_CASE( "MapType Dynamic", "[aeMeta]" )
{
	ae::Map< ae::Str32, int32_t > _map = AE_ALLOC_TAG_META_TEST;
	_map.Reserve( 2 );
	ae::DataPointer map( &_map );

	REQUIRE( map.GetVarType().IsSameBaseVarType< ae::MapType >() );
	const ae::MapType* mapVarType = map.GetVarType().AsVarType< ae::MapType >();
	REQUIRE( mapVarType );
	REQUIRE( mapVarType->GetMaxLength() == UINT32_MAX );
	REQUIRE( _map.Size() == 2 );
	
	const ae::BasicType* keyVarType = mapVarType->GetKeyVarType().AsVarType< ae::BasicType >();
	const ae::BasicType* valueVarType = mapVarType->GetValueVarType().AsVarType< ae::BasicType >();
	REQUIRE( keyVarType );
	REQUIRE( valueVarType );
	REQUIRE( keyVarType->GetType() == ae::BasicType::String );
	REQUIRE( valueVarType->GetType() == ae::BasicType::Int32 );

	const ae::Str32 keyStr0 = "Something1000";
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1000 ) );
		REQUIRE( _map.Get( keyStr0.c_str() ) == 1000 );
	}
	{
		int32_t value = -777;
		ae::DataPointer valueVarData = mapVarType->TryGet( map, ae::ConstDataPointer( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == 1000 );
	}
	{
		int32_t value = -777;
		ae::ConstDataPointer valueVarData = mapVarType->TryGet( ae::ConstDataPointer( map ), ae::ConstDataPointer( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == 1000 );
	}
	REQUIRE( _map.Size() == 2 );

	const ae::Str32 keyStr1 = "Something1001";
	{
		int32_t value = -777;
		ae::ConstDataPointer valueVarData = mapVarType->TryGet( ae::ConstDataPointer( map ), ae::ConstDataPointer( &keyStr1 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( !valueVarData );
		REQUIRE( !valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == -777 );
	}
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr1 ) );
		REQUIRE( _map.Length() == 2 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1001 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1001 );
	}
	{
		ae::DataPointer valueVarData = mapVarType->TryGet( map, ae::ConstDataPointer( &keyStr1 ) );
		REQUIRE( _map.Length() == 2 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1101 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
	}
	REQUIRE( _map.Size() == 2 );

	const ae::Str32 keyStr2 = "Something1002";
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr2 ) );
		REQUIRE( _map.Length() == 3 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1002 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
		REQUIRE( _map.Get( "Something1002" ) == 1002 );
	}
	REQUIRE( _map.Size() >= 3 );

	const ae::Str32 keyStr3 = "Something1003";
	{
		ae::DataPointer valueVarData = mapVarType->Get( map, ae::ConstDataPointer( &keyStr3 ) );
		REQUIRE( _map.Length() == 4 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1003 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
		REQUIRE( _map.Get( "Something1002" ) == 1002 );
		REQUIRE( _map.Get( "Something1003" ) == 1003 );
	}
	REQUIRE( _map.Size() >= 4 );
}

TEST_CASE( "MapType Iteration", "[aeMeta]" )
{
	ae::Map< ae::Str32, int32_t, 4 > _map;
	_map.Set( "Something1000", 1000 );
	_map.Set( "Something1001", 1001 );
	_map.Set( "Something1002", 1002 );
	_map.Set( "Something1003", 1003 );

	ae::DataPointer map( &_map );
	ae::ConstDataPointer constMap( &_map );
	const ae::MapType* mapVarType = map.GetVarType().AsVarType< ae::MapType >();
	REQUIRE( mapVarType );

	const int32_t mapLength = mapVarType->GetLength( map );
	REQUIRE( mapLength == 4 );

	for( int32_t i = 0; i < mapLength; i++ )
	{
		const ae::Str32 actualKey = _map.GetKey( i );
		const int32_t actualValue = _map.GetValue( i );
		
		ae::ConstDataPointer constKey = mapVarType->GetKey( constMap, i );
		ae::ConstDataPointer constValue = mapVarType->GetValue( constMap, i );
		REQUIRE( *reinterpret_cast< const ae::Str32* >( constKey.Get() ) == actualKey );
		REQUIRE( *reinterpret_cast< const int32_t* >( constValue.Get() ) == actualValue );
		
		ae::DataPointer value = mapVarType->GetValue( map, i );
		REQUIRE( *reinterpret_cast< int32_t* >( value.Get() ) == actualValue );
	}

	REQUIRE( mapVarType->GetKey( constMap, -1 ) == ae::ConstDataPointer() );
	REQUIRE( mapVarType->GetValue( constMap, -1 ) == ae::ConstDataPointer() );
	REQUIRE( mapVarType->GetValue( map, -1 ) == ae::DataPointer() );
	
	REQUIRE( mapVarType->GetKey( constMap, mapLength ) == ae::ConstDataPointer() );
	REQUIRE( mapVarType->GetValue( constMap, mapLength ) == ae::ConstDataPointer() );
	REQUIRE( mapVarType->GetValue( map, mapLength ) == ae::DataPointer() );
}
