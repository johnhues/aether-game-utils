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
	ae::VarData array = &_array;
	const ae::VarType* varType = array.GetType();

	REQUIRE( varType->IsVarType< ae::ArrayVarType >() );
	REQUIRE( varType->AsVarType< ae::ArrayVarType >() );
	REQUIRE( varType->AsVarType< ae::ArrayVarType >()->IsFixedLength() == false );
	REQUIRE( varType->AsVarType< ae::ArrayVarType >()->GetMaxLength() == 5 );
	REQUIRE( varType->GetInnerVarType< ae::BasicVarType >() );
	REQUIRE( varType->GetInnerVarType< ae::BasicVarType >()->GetType() == ae::BasicType::Int32 );

	// Set array length
	REQUIRE( _array.Length() == 0 );
	REQUIRE( varType->AsVarType< ae::ArrayVarType >()->Resize( array, 2 ) == 2 );
	REQUIRE( _array.Length() == 2 );

	// Access elements
	REQUIRE( _array[ 0 ] == 0 );
	REQUIRE( _array[ 1 ] == 0 );
	REQUIRE_THROWS( varType->AsVarType< ae::ArrayVarType >()->GetElement( array, -1 ) );
	ae::VarData elem0 = varType->AsVarType< ae::ArrayVarType >()->GetElement( array, 0 );
	ae::VarData elem1 = varType->AsVarType< ae::ArrayVarType >()->GetElement( array, 1 );
	REQUIRE_THROWS( varType->AsVarType< ae::ArrayVarType >()->GetElement( array, 2 ) );
	REQUIRE( elem0 );
	REQUIRE( elem1 );
	// Set elements
	REQUIRE( varType->GetInnerVarType< ae::BasicVarType >()->SetVarData( elem0, 1000 ) );
	REQUIRE( varType->GetInnerVarType< ae::BasicVarType >()->SetVarDataFromString( elem1, "1001" ) );
	REQUIRE( _array[ 0 ] == 1000 );
	REQUIRE( _array[ 1 ] == 1001 );
	// Get elements
	int32_t val0 = 0;
	REQUIRE( varType->GetInnerVarType< ae::BasicVarType >()->GetVarData( elem0, &val0 ) );
	REQUIRE( val0 == 1000 );
	REQUIRE( varType->GetInnerVarType< ae::BasicVarType >()->GetVarDataAsString( elem1 ) == "1001" );
}
