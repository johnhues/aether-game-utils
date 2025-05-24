//------------------------------------------------------------------------------
// AttributeTest2.cpp
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
#include <catch2/catch_test_macros.hpp>
#include "aether.h"
#include "AttributesTest.h"

//------------------------------------------------------------------------------
// Attribute tests
//------------------------------------------------------------------------------
TEST_CASE( "Registered class attributes", "[Attributes]" )
{
	const ae::ClassType* type = ae::GetClassType< GameObject >();
	REQUIRE( type );
	const ae::ClassVar* var = type->GetVarByName( "id", false );
	REQUIRE( var );
	REQUIRE( type->attributes.GetCount< ae::Attribute >() == 6 );
	REQUIRE( var->attributes.GetCount< ae::Attribute >() == 4 );

	GameObject obj;
	obj.id = 1;
	REQUIRE( var->GetObjectValueAsString( &obj ) == "1" );

	REQUIRE( type->attributes.Has< ae::SourceFileAttribute >() );
	REQUIRE( type->attributes.GetCount< ae::SourceFileAttribute >() == 1 );

	REQUIRE( type->attributes.Has< EmptyAttrib >() );
	REQUIRE( type->attributes.GetCount< EmptyAttrib >() == 1 );

	REQUIRE( type->attributes.Has< RequiresAttrib >() );
	const RequiresAttrib* requiresAttrib0 = type->attributes.TryGet< RequiresAttrib >( 0 );
	const RequiresAttrib* requiresAttrib1 = type->attributes.TryGet< RequiresAttrib >( 1 );
	const RequiresAttrib* requiresAttrib2 = type->attributes.TryGet< RequiresAttrib >( 2 );
	const RequiresAttrib* requiresAttrib3 = type->attributes.TryGet< RequiresAttrib >( 3 );
	const RequiresAttrib* requiresAttrib4 = type->attributes.TryGet< RequiresAttrib >( 4 );
	REQUIRE( requiresAttrib0 );
	REQUIRE( requiresAttrib0->name == "Something" );
	REQUIRE( requiresAttrib1 );
	REQUIRE( requiresAttrib1->name == "SomethingElse" );
	REQUIRE( requiresAttrib2 );
	REQUIRE( requiresAttrib2->name == "SomethingSomethingElse" );
	REQUIRE( requiresAttrib3 );
	REQUIRE( requiresAttrib3->name == "SomethingSomethingElseElse" );
	REQUIRE( requiresAttrib4 == nullptr );

	REQUIRE( var->attributes.Has< ae::SourceFileAttribute >() );

	const CategoryInfoAttribute* categoryInfoAttrib = var->attributes.TryGet< CategoryInfoAttribute >();
	REQUIRE( categoryInfoAttrib );
	REQUIRE( categoryInfoAttrib->name == "General" );
	REQUIRE( categoryInfoAttrib->sortOrder == 1 );

	REQUIRE( var->attributes.GetCount< DisplayName >() == 2 );
	const DisplayName* displayNameAttrib0 = var->attributes.TryGet< DisplayName >( 0 );
	const DisplayName* displayNameAttrib1 = var->attributes.TryGet< DisplayName >( 1 );
	const DisplayName* displayNameAttrib2 = var->attributes.TryGet< DisplayName >( 2 );
	REQUIRE( displayNameAttrib0 );
	REQUIRE( displayNameAttrib0->name == "ID" );
	REQUIRE( displayNameAttrib1 );
	REQUIRE( displayNameAttrib1->name == "ID2" );
	REQUIRE( displayNameAttrib2 == nullptr );
}

TEST_CASE( "Registered namespace class attributes", "[Attributes]" )
{
	const ae::ClassType* type = ae::GetClassType< xyz::Util >();
	REQUIRE( type );
	const ae::ClassVar* var = type->GetVarByName( "id", false );
	REQUIRE( var );
	REQUIRE( type->attributes.GetCount< ae::Attribute >() == 6 );
	REQUIRE( var->attributes.GetCount< ae::Attribute >() == 4 );

	xyz::Util obj;
	obj.id = 1;
	REQUIRE( var->GetObjectValueAsString( &obj ) == "1" );

	REQUIRE( type->attributes.Has< ae::SourceFileAttribute >() );
	REQUIRE( type->attributes.GetCount< ae::SourceFileAttribute >() == 1 );

	REQUIRE( type->attributes.Has< EmptyAttrib >() );
	REQUIRE( type->attributes.GetCount< EmptyAttrib >() == 1 );

	REQUIRE( type->attributes.Has< RequiresAttrib >() );
	const RequiresAttrib* requiresAttrib0 = type->attributes.TryGet< RequiresAttrib >( 0 );
	const RequiresAttrib* requiresAttrib1 = type->attributes.TryGet< RequiresAttrib >( 1 );
	const RequiresAttrib* requiresAttrib2 = type->attributes.TryGet< RequiresAttrib >( 2 );
	const RequiresAttrib* requiresAttrib3 = type->attributes.TryGet< RequiresAttrib >( 3 );
	const RequiresAttrib* requiresAttrib4 = type->attributes.TryGet< RequiresAttrib >( 4 );
	REQUIRE( requiresAttrib0 );
	REQUIRE( requiresAttrib0->name == "Something" );
	REQUIRE( requiresAttrib1 );
	REQUIRE( requiresAttrib1->name == "SomethingElse" );
	REQUIRE( requiresAttrib2 );
	REQUIRE( requiresAttrib2->name == "SomethingSomethingElse" );
	REQUIRE( requiresAttrib3 );
	REQUIRE( requiresAttrib3->name == "SomethingSomethingElseElse" );
	REQUIRE( requiresAttrib4 == nullptr );

	REQUIRE( var->attributes.Has< ae::SourceFileAttribute >() );

	const CategoryInfoAttribute* categoryInfoAttrib = var->attributes.TryGet< CategoryInfoAttribute >();
	REQUIRE( categoryInfoAttrib );
	REQUIRE( categoryInfoAttrib->name == "General" );
	REQUIRE( categoryInfoAttrib->sortOrder == 1 );

	REQUIRE( var->attributes.GetCount< DisplayName >() == 2 );
	const DisplayName* displayNameAttrib0 = var->attributes.TryGet< DisplayName >( 0 );
	const DisplayName* displayNameAttrib1 = var->attributes.TryGet< DisplayName >( 1 );
	const DisplayName* displayNameAttrib2 = var->attributes.TryGet< DisplayName >( 2 );
	REQUIRE( displayNameAttrib0 );
	REQUIRE( displayNameAttrib0->name == "ID" );
	REQUIRE( displayNameAttrib1 );
	REQUIRE( displayNameAttrib1->name == "ID2" );
	REQUIRE( displayNameAttrib2 == nullptr );
}
