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
TEST_CASE( "VarData", "[aeMeta]" )
{
	REQUIRE( ae::VarData() == ae::VarData() );
	REQUIRE( ae::ConstVarData() == ae::ConstVarData() );
}

TEST_CASE( "ClassType", "[aeMeta]" )
{
	SomeClass someClass;
	ae::VarData someClassPointer( &someClass );
	REQUIRE( someClassPointer.GetVarType().AsVarType< ae::ClassVarType >()->GetType() == ae::GetType< SomeClass >() );
	REQUIRE( someClassPointer.GetVarType().GetExactVarTypeId() == ae::GetType< SomeClass >()->GetId() );
}

TEST_CASE( "StaticArrayVarType", "[aeMeta]" )
{
	ae::Array< int32_t, 5 > _array;
	ae::VarData array( &_array );
	REQUIRE( array.GetVarType().IsSameBaseVarType< ae::ArrayVarType >() );
	const ae::ArrayVarType* varType = array.GetVarType().AsVarType< ae::ArrayVarType >();
	REQUIRE( varType );
	REQUIRE( varType->GetInnerVarType().IsSameBaseVarType< ae::BasicVarType >() );
	const ae::BasicVarType* innerVarType = varType->GetInnerVarType().AsVarType< ae::BasicVarType >();
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
	REQUIRE( varType->GetElement( array, -1 ) == ae::VarData() );
	ae::VarData elem0 = varType->GetElement( array, 0 );
	ae::VarData elem1 = varType->GetElement( array, 1 );
	REQUIRE( varType->GetElement( array, 2 ) == ae::VarData() );
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

TEST_CASE( "MapVarType", "[aeMeta]" )
{
	ae::Map< ae::Str32, int32_t, 2 > _map;
	ae::VarData map( &_map );

	REQUIRE( map.GetVarType().IsSameBaseVarType< ae::MapVarType >() );
	const ae::MapVarType* mapVarType = map.GetVarType().AsVarType< ae::MapVarType >();
	REQUIRE( mapVarType );
	REQUIRE( mapVarType->GetMaxLength() == 2 );
	
	const ae::BasicVarType* keyVarType = mapVarType->GetKeyVarType().AsVarType< ae::BasicVarType >();
	const ae::BasicVarType* valueVarType = mapVarType->GetValueVarType().AsVarType< ae::BasicVarType >();
	REQUIRE( keyVarType );
	REQUIRE( valueVarType );
	REQUIRE( keyVarType->GetType() == ae::BasicType::String );
	REQUIRE( valueVarType->GetType() == ae::BasicType::Int32 );

	const ae::Str32 keyStr0 = "Something1000";
	{
		ae::VarData valueVarData = mapVarType->Get( map, ae::ConstVarData( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1000 ) );
		REQUIRE( _map.Get( keyStr0.c_str() ) == 1000 );
	}
	{
		int32_t value = -777;
		ae::VarData valueVarData = mapVarType->TryGet( map, ae::ConstVarData( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == 1000 );
	}
	{
		int32_t value = -777;
		ae::ConstVarData valueVarData = mapVarType->TryGet( ae::ConstVarData( map ), ae::ConstVarData( &keyStr0 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == 1000 );
	}

	const ae::Str32 keyStr1 = "Something1001";
	{
		int32_t value = -777;
		ae::ConstVarData valueVarData = mapVarType->TryGet( ae::ConstVarData( map ), ae::ConstVarData( &keyStr1 ) );
		REQUIRE( _map.Length() == 1 );
		REQUIRE( !valueVarData );
		REQUIRE( !valueVarType->GetVarData( valueVarData, &value ) );
		REQUIRE( value == -777 );
	}
	{
		ae::VarData valueVarData = mapVarType->Get( map, ae::ConstVarData( &keyStr1 ) );
		REQUIRE( _map.Length() == 2 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1001 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1001 );
	}
	{
		ae::VarData valueVarData = mapVarType->TryGet( map, ae::ConstVarData( &keyStr1 ) );
		REQUIRE( _map.Length() == 2 );
		REQUIRE( valueVarData );
		REQUIRE( valueVarType->SetVarData( valueVarData, 1101 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
	}

	const ae::Str32 keyStr2 = "Something1002";
	{
		ae::VarData valueVarData = mapVarType->Get( map, ae::ConstVarData( &keyStr2 ) );
		REQUIRE( _map.Length() == 2 );
		REQUIRE( !valueVarData );
		REQUIRE( !valueVarType->SetVarData( valueVarData, 1002 ) );
		REQUIRE( _map.Get( "Something1000" ) == 1000 );
		REQUIRE( _map.Get( "Something1001" ) == 1101 );
	}
}
