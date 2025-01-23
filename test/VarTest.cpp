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
TEST_CASE( "ArrayVarType", "[aeMeta]" )
{
	ae::Array< int32_t, 5 > _array;
	ae::VarData array( &_array );
	REQUIRE( array.GetType().IsVarType< ae::ArrayVarType >() );
	// @TODO: Remove template fn, make return type a const ref
	const ae::ArrayVarType* varType = array.GetType().AsVarType< ae::ArrayVarType >();
	REQUIRE( varType );
	REQUIRE( varType->GetInnerVarType().IsVarType< ae::BasicVarType >() );
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
	REQUIRE_THROWS( varType->GetElement( array, -1 ) );
	ae::VarData elem0 = varType->GetElement( array, 0 );
	ae::VarData elem1 = varType->GetElement( array, 1 );
	REQUIRE_THROWS( varType->GetElement( array, 2 ) );
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
