//------------------------------------------------------------------------------
// ListTest.cpp
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
#include "aether.h"
#include <catch2/catch_test_macros.hpp>

const ae::Tag TAG_LIST = "list";

struct TestObject
{
	TestObject() = default;
	TestObject( uint32_t value ) : value( value ) {}
	ae::ListNode< TestObject > node = this;
	uint32_t value = -1;
	bool inList = false;
};

//------------------------------------------------------------------------------
// ae::List tests
//------------------------------------------------------------------------------
TEST_CASE( "Basic", "[ae::List]" )
{
	ae::List< TestObject > list;
	TestObject objs[ 5 ] = { 0, 1, 2, 3, 4 };
	for( TestObject& obj : objs )
	{
		list.Append( obj.node );
	}

	uint32_t i = 0;
	for( TestObject* obj = list.GetFirst(); obj; obj = obj->node.GetNext() )
	{
		REQUIRE( obj->value == i );
		++i;
	}

	i = 4;
	for( TestObject* obj = list.GetLast(); obj; obj = obj->node.GetPrev() )
	{
		REQUIRE( obj->value == i );
		--i;
	}

	i = 0;
	for( TestObject* obj = list.GetFirst(); obj; obj = obj->node.GetNext() )
	{
		obj->node.Remove();
		++i;
		uint32_t j = i;
		for( TestObject* obj2 = list.GetFirst(); obj2; obj2 = obj2->node.GetNext() )
		{
			REQUIRE( obj2->value == j );
			++j;
		}
		j = 4;
		for( TestObject* obj2 = list.GetLast(); obj2; obj2 = obj2->node.GetPrev() )
		{
			REQUIRE( obj2->value == j );
			--j;
		}
	}
}

TEST_CASE( "Stress", "[ae::List]" )
{
	ae::List< TestObject > list;
	ae::Array< TestObject* > objs = TAG_LIST;
	for( uint32_t i = 0; i < 1000; ++i )
	{
		objs.Append( ae::New< TestObject >( TAG_LIST, i ) )->value = i;
	}
	uint32_t listLength = 0;
	uint32_t objectCount = objs.Length();
	uint32_t i = 0;
	while( objectCount )
	{
		i++;
		const uint32_t index = ae::Random( 0, objs.Length() );
		if( TestObject* obj = objs[ index ] )
		{
			if( ae::Random( 0, 10 ) == 0 )
			{
				if( obj->inList )
				{
					listLength--;
				}
				ae::Delete( objs[ index ] );
				objs[ index ] = nullptr;
				objectCount--;
			}
			else if( obj->inList )
			{
				obj->node.Remove();
				obj->inList = false;
				listLength--;
			}
			else
			{
				list.Append( obj->node );
				obj->inList = true;
				listLength++;
			}
			REQUIRE( list.Length() == listLength );
			for( TestObject* obj = list.GetFirst(); obj; obj = obj->node.GetNext() )
			{
				REQUIRE( obj->inList );
			}
			for( TestObject* obj : objs )
			{
				if( obj )
				{
					if( obj->inList )
					{
						REQUIRE( obj->node.GetList() == &list );
					}
					else
					{
						REQUIRE( !obj->node.GetList() );
					}
				}
			}
		}
	}
	AE_INFO( "List iterations: #", i );
	REQUIRE( listLength == 0 );
	REQUIRE( objectCount == 0 );
}
