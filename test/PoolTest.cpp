//------------------------------------------------------------------------------
// PoolTest.cpp
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
#include "TestUtils.h"

const ae::Tag TAG_POOL = "pool";

//------------------------------------------------------------------------------
// ae::FreeList (static) tests
//------------------------------------------------------------------------------
TEST_CASE( "Static indices can be allocated and deallocated", "[ae::FreeList (static)]" )
{
	const uint32_t kNumElements = 10;
	int32_t objects[ kNumElements ];
	ae::FreeList< kNumElements > freeList;
	REQUIRE( freeList.Size() == kNumElements );
	REQUIRE( freeList.GetFirst() < 0 );

	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		objects[ i ] = freeList.Allocate();
		REQUIRE( objects[ i ] >= 0 );
	}
	REQUIRE( freeList.Length() == kNumElements );
	REQUIRE( !freeList.HasFree() );

	SECTION( "excess Allocate()'s return negative values" )
	{
		REQUIRE( freeList.Allocate() < 0 );
	}

	SECTION( "can free objects" )
	{
		for ( uint32_t i = 0; i < kNumElements; i++ )
		{
			int32_t idx = objects[ i ];
			uint32_t remaining = kNumElements - ( i + 1 );
			freeList.Free( idx );
			REQUIRE( freeList.Length() == remaining );
		}
	}

	SECTION( "negative indices are handled gracefully" )
	{
		freeList.Free( -1 );
		REQUIRE( freeList.Length() == kNumElements );
		REQUIRE( !freeList.HasFree() );

		REQUIRE( freeList.GetNext( -1 ) < 0 );
	}

	SECTION( "can iterate over allocated objects" )
	{
		uint32_t count = 0;
		for ( int32_t idx = freeList.GetFirst(); idx >= 0; idx = freeList.GetNext( idx ) )
		{
			REQUIRE( 0 <= idx );
			REQUIRE( idx < kNumElements );
			REQUIRE( freeList.IsAllocated( idx ) );
			count++;
		}
		REQUIRE( count == kNumElements );
	}

	SECTION( "can iterate over allocated objects after freeing some" )
	{
		freeList.Free( objects[ 0 ] );
		freeList.Free( objects[ kNumElements / 2 ] );
		freeList.Free( objects[ kNumElements - 1 ] );
		REQUIRE( !freeList.IsAllocated( objects[ 0 ] ) );
		REQUIRE( !freeList.IsAllocated( objects[ kNumElements / 2 ] ) );
		REQUIRE( !freeList.IsAllocated( objects[ kNumElements - 1 ] ) );

		uint32_t count = 0;
		for ( int32_t idx = freeList.GetFirst(); idx >= 0; idx = freeList.GetNext( idx ) )
		{
			REQUIRE( 0 <= idx );
			REQUIRE( idx < kNumElements );
			REQUIRE( freeList.IsAllocated( idx ) );
			count++;
		}
		REQUIRE( count == kNumElements - 3 );
	}
}

TEST_CASE( "can allocate objects after FreeAll", "[ae::FreeList (static)]" )
{
	const uint32_t kNumElements = 10;
	ae::FreeList< kNumElements > freeList;
	REQUIRE( freeList.Size() == kNumElements );
	REQUIRE( freeList.GetFirst() < 0 );
	REQUIRE( freeList.Length() == 0 );

	freeList.FreeAll();
	REQUIRE( freeList.GetFirst() < 0 );
	REQUIRE( freeList.Length() == 0 );

	int32_t idx = freeList.Allocate();
	REQUIRE( idx >= 0 );
	REQUIRE( freeList.Length() == 1 );
	freeList.Free( idx );
	REQUIRE( freeList.Length() == 0 );

	int32_t objects[ kNumElements ];
	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		objects[ i ] = freeList.Allocate();
		REQUIRE( objects[ i ] >= 0 );
		REQUIRE( freeList.Length() == i + 1 );
	}
	REQUIRE( freeList.Length() == kNumElements );
	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		freeList.Free( objects[ i ] );
		REQUIRE( freeList.Length() == kNumElements - ( i + 1 ) );
	}
	REQUIRE( freeList.Length() == 0 );
}

//------------------------------------------------------------------------------
// ae::FreeList (dynamic) tests
//------------------------------------------------------------------------------
TEST_CASE( "Dynamic indices can be allocated and deallocated", "[ae::FreeList (dynamic)]" )
{
	const uint32_t kNumElements = 10;
	int32_t objects[ kNumElements ];
	ae::FreeList<> freeList( TAG_POOL, kNumElements );
	REQUIRE( freeList.Size() == kNumElements );
	REQUIRE( freeList.GetFirst() < 0 );

	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		objects[ i ] = freeList.Allocate();
		REQUIRE( objects[ i ] >= 0 );
	}
	REQUIRE( freeList.Length() == kNumElements );
	REQUIRE( !freeList.HasFree() );

	SECTION( "excess Allocate()'s return negative values" )
	{
		REQUIRE( freeList.Allocate() < 0 );
	}

	SECTION( "can free objects" )
	{
		for ( uint32_t i = 0; i < kNumElements; i++ )
		{
			int32_t idx = objects[ i ];
			uint32_t remaining = kNumElements - ( i + 1 );
			freeList.Free( idx );
			REQUIRE( freeList.Length() == remaining );
		}
	}

	SECTION( "negative indices are handled gracefully" )
	{
		freeList.Free( -1 );
		REQUIRE( freeList.Length() == kNumElements );
		REQUIRE( !freeList.HasFree() );

		REQUIRE( freeList.GetNext( -1 ) < 0 );
	}

	SECTION( "can iterate over allocated objects" )
	{
		uint32_t count = 0;
		for ( int32_t idx = freeList.GetFirst(); idx >= 0; idx = freeList.GetNext( idx ) )
		{
			REQUIRE( 0 <= idx );
			REQUIRE( idx < kNumElements );
			REQUIRE( freeList.IsAllocated( idx ) );
			count++;
		}
		REQUIRE( count == kNumElements );
	}

	SECTION( "can iterate over allocated objects after freeing some" )
	{
		freeList.Free( objects[ 0 ] );
		freeList.Free( objects[ kNumElements / 2 ] );
		freeList.Free( objects[ kNumElements - 1 ] );
		REQUIRE( !freeList.IsAllocated( objects[ 0 ] ) );
		REQUIRE( !freeList.IsAllocated( objects[ kNumElements / 2 ] ) );
		REQUIRE( !freeList.IsAllocated( objects[ kNumElements - 1 ] ) );

		uint32_t count = 0;
		for ( int32_t idx = freeList.GetFirst(); idx >= 0; idx = freeList.GetNext( idx ) )
		{
			REQUIRE( 0 <= idx );
			REQUIRE( idx < kNumElements );
			REQUIRE( freeList.IsAllocated( idx ) );
			count++;
		}
		REQUIRE( count == kNumElements - 3 );
	}
}

//------------------------------------------------------------------------------
// ae::ObjectPool tests
//------------------------------------------------------------------------------
TEST_CASE( "Objects can be allocated and deallocated", "[aeObjectPool]" )
{
	ae::LifetimeTester::ClearStats();

	const uint32_t kNumElements = 10;
	ae::LifetimeTester* objects[ kNumElements ];
	ae::ObjectPool< ae::LifetimeTester, kNumElements > pool;
	REQUIRE( pool.Size() == kNumElements );
	REQUIRE( !pool.GetFirst() );

	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );

	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		objects[ i ] = pool.New();
		REQUIRE( objects[ i ] );
		objects[ i ]->value = 'a' + i;
	}
	REQUIRE( pool.Length() == kNumElements );
	REQUIRE( !pool.HasFree() );

	REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == kNumElements );

	SECTION( "excess New()'s return null" )
	{
		REQUIRE( !pool.New() );

		// No changes
		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can free objects" )
	{
		REQUIRE( pool.Length() == pool.Size() );
		REQUIRE( pool.Length() == kNumElements );
		for ( uint32_t i = 0; i < kNumElements; i++ )
		{
			auto* p = objects[ i ];
			uint32_t remaining = kNumElements - ( i + 1 );
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			pool.Delete( p );
			REQUIRE( pool.Length() == remaining );

			REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
			REQUIRE( ae::LifetimeTester::copyCount == 0 );
			REQUIRE( ae::LifetimeTester::moveCount == 0 );
			REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::dtorCount == i + 1 );
			REQUIRE( ae::LifetimeTester::currentCount == remaining );
		}
		REQUIRE( pool.Length() == 0 );
		auto* p = pool.New();
		REQUIRE( p );
		REQUIRE( pool.Length() == 1 );
		pool.Delete( p );
		REQUIRE( pool.Length() == 0 );
	}

	SECTION( "null pointers are handled gracefully" )
	{
		pool.Delete( nullptr );
		REQUIRE( pool.Length() == kNumElements );
		REQUIRE( !pool.HasFree() );
		REQUIRE( !pool.GetNext( nullptr ) );
	}

	SECTION( "can iterate over allocated objects" )
	{
		uint32_t i = 0;
		for ( auto* p = pool.GetFirst(); p; p = pool.GetNext( p ) )
		{
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			REQUIRE( p->value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects (const)" )
	{
		const auto* constPool = &pool;
		uint32_t i = 0;
		for ( auto* p = constPool->GetFirst(); p; p = constPool->GetNext( p ) )
		{
			REQUIRE( std::is_const_v< std::remove_pointer_t< decltype( p ) > > );
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			REQUIRE( p->value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects after freeing some" )
	{
		pool.Delete( objects[ 0 ] );
		pool.Delete( objects[ kNumElements / 2 ] );
		pool.Delete( objects[ kNumElements - 1 ] );

		uint32_t count = 0;
		for ( const auto* obj = pool.GetFirst(); obj; obj = pool.GetNext( obj ) )
		{
			count++;
		}
		REQUIRE( count == kNumElements - 3 );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 3 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements - 3 );
	}

	pool.DeleteAll();
	REQUIRE( ae::LifetimeTester::ctorCount == ae::LifetimeTester::dtorCount );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

TEST_CASE( "Pages can be checked for objects", "[aeObjectPool]" )
{
	ae::LifetimeTester::ClearStats();

	const uint32_t kNumElements = 10;
	ae::ObjectPool< int32_t, kNumElements > pool;
	REQUIRE( pool.Size() == kNumElements );
	REQUIRE( !pool.GetFirst() );

	// for ( uint32_t i = 0; i < kNumElements; i++ )
	// {
	// 	int32_t* p = pool.New();
	// 	REQUIRE( pool.IsInPool( p ) );
	// 	REQUIRE( !pool.IsInPool( (int32_t*)( (uint8_t*)p + 1 ) ) );
	// }
	
	// int32_t something = 0;
	//REQUIRE( !pool.IsInPool( &something ) );

	pool.DeleteAll();
}

TEST_CASE( "can allocate objects after clear", "[aeObjectPool]" )
{
	const uint32_t kNumElements = 10;
	ae::ObjectPool< int32_t, kNumElements > pool;
	REQUIRE( pool.Size() == kNumElements );
	REQUIRE( !pool.GetFirst() );
	REQUIRE( pool.Length() == 0 );

	pool.DeleteAll();
	auto* p = pool.New();
	REQUIRE( p );
	REQUIRE( pool.Length() == 1 );
	pool.Delete( p );
	REQUIRE( pool.Length() == 0 );
}

//------------------------------------------------------------------------------
// ae::ObjectPool tests
//------------------------------------------------------------------------------
TEST_CASE( "Paged pool objects can be allocated and deallocated", "[aePagedObjectPool]" )
{
	ae::LifetimeTester::ClearStats();

	const uint32_t kPageSize = 3;
	const uint32_t kNumElements = 10;
	ae::LifetimeTester* objects[ kNumElements ];
	ae::ObjectPool< ae::LifetimeTester, kPageSize, true > pool = TAG_POOL;
	REQUIRE( !pool.Length() );
	REQUIRE( !pool.GetFirst() );

	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );

	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		objects[ i ] = pool.New();
		REQUIRE( objects[ i ] );
		objects[ i ]->value = 'a' + i;
	}
	REQUIRE( pool.Length() == kNumElements );

	REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == kNumElements );

	SECTION( "can free objects" )
	{
		for ( uint32_t i = 0; i < kNumElements; i++ )
		{
			auto* p = objects[ i ];
			uint32_t remaining = kNumElements - ( i + 1 );
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			pool.Delete( p );
			REQUIRE( pool.Length() == remaining );

			REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
			REQUIRE( ae::LifetimeTester::copyCount == 0 );
			REQUIRE( ae::LifetimeTester::moveCount == 0 );
			REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::dtorCount == i + 1 );
			REQUIRE( ae::LifetimeTester::currentCount == remaining );
		}
	}

	SECTION( "null pointers are handled gracefully" )
	{
		pool.Delete( nullptr );
		REQUIRE( pool.Length() == kNumElements );
		REQUIRE( !pool.GetNext( nullptr ) );
	}

	SECTION( "can iterate over allocated objects" )
	{
		uint32_t i = 0;
		for ( auto* p = pool.GetFirst(); p; p = pool.GetNext( p ) )
		{
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			REQUIRE( p->value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects (const)" )
	{
		const auto* constPool = &pool;
		uint32_t i = 0;
		for ( auto* p = constPool->GetFirst(); p; p = constPool->GetNext( p ) )
		{
			REQUIRE( std::is_const_v< std::remove_pointer_t< decltype( p ) > > );
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			REQUIRE( p->value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects after freeing some" )
	{
		pool.Delete( objects[ 0 ] );
		pool.Delete( objects[ kNumElements / 2 ] );
		pool.Delete( objects[ kNumElements - 1 ] );

		uint32_t count = 0;
		for ( const auto* obj = pool.GetFirst(); obj; obj = pool.GetNext( obj ) )
		{
			count++;
		}
		REQUIRE( count == kNumElements - 3 );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 3 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements - 3 );
	}

	SECTION( "can create const iterator from non-const iterator" )
	{
		decltype(pool)::Iterator< const ae::LifetimeTester > constIter = pool.begin();
	}

	SECTION( "can iterate over allocated objects with a range-based for loop" )
	{
		uint32_t i = 0;
		for ( ae::LifetimeTester& obj : pool )
		{
			REQUIRE( obj.check == ae::LifetimeTester::kConstructed );
			REQUIRE( obj.value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects with a range-based for loop (const)" )
	{
		const auto& constPool = pool;
		uint32_t i = 0;
		for ( const ae::LifetimeTester& obj : constPool )
		{
			REQUIRE( std::is_const_v< std::remove_reference_t< decltype( obj ) > > );
			REQUIRE( obj.check == ae::LifetimeTester::kConstructed );
			REQUIRE( obj.value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects with a range-based for loop after freeing some" )
	{
		pool.Delete( objects[ 0 ] );
		pool.Delete( objects[ kNumElements / 2 ] );
		pool.Delete( objects[ kNumElements - 1 ] );

		uint32_t count = 0;
		for ( const ae::LifetimeTester& obj : pool )
		{
			count++;
		}
		REQUIRE( count == kNumElements - 3 );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 3 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements - 3 );
	}

	pool.DeleteAll();
	REQUIRE( ae::LifetimeTester::ctorCount == ae::LifetimeTester::dtorCount );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

//------------------------------------------------------------------------------
// ae::OpaquePool tests
//------------------------------------------------------------------------------
TEST_CASE( "aeOpaquePool Objects can be allocated and deallocated", "[aeOpaquePool]" )
{
	ae::LifetimeTester::ClearStats();

	const uint32_t kNumElements = 10;
	ae::LifetimeTester* objects[ kNumElements ];
	ae::OpaquePool pool( TAG_POOL, sizeof(ae::LifetimeTester), alignof(ae::LifetimeTester), kNumElements, false );
	REQUIRE( pool.PageSize() == kNumElements );
	REQUIRE( pool.Size() == kNumElements );
	// REQUIRE( pool.Iterate< int32_t >().begin() == pool.Iterate< int32_t >().end() ); // @TODO: Catch can't convert this to a string
	REQUIRE( pool.Length() == 0 );

	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );

	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		objects[ i ] = pool.New< ae::LifetimeTester >();
		REQUIRE( objects[ i ] );
		objects[ i ]->value = 'a' + i;
	}
	REQUIRE( pool.Length() == kNumElements );
	REQUIRE( !pool.HasFree() );

	REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == kNumElements );

	SECTION( "excess New()'s return null" )
	{
		REQUIRE( !pool.New< ae::LifetimeTester >() );

		// No changes
		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can free objects" )
	{
		REQUIRE( pool.Length() == pool.Size() );
		REQUIRE( pool.Length() == kNumElements );
		for ( uint32_t i = 0; i < kNumElements; i++ )
		{
			auto* p = objects[ i ];
			uint32_t remaining = kNumElements - ( i + 1 );
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			pool.Delete( p );
			REQUIRE( pool.Length() == remaining );

			REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
			REQUIRE( ae::LifetimeTester::copyCount == 0 );
			REQUIRE( ae::LifetimeTester::moveCount == 0 );
			REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::dtorCount == i + 1 );
			REQUIRE( ae::LifetimeTester::currentCount == remaining );
		}
		REQUIRE( pool.Length() == 0 );
		auto* p = pool.New< ae::LifetimeTester >();
		REQUIRE( p );
		REQUIRE( pool.Length() == 1 );
		pool.Delete( p );
		REQUIRE( pool.Length() == 0 );
	}

	SECTION( "null pointers are handled gracefully" )
	{
		ae::LifetimeTester* nullObj = nullptr;
		pool.Delete( nullObj );
		REQUIRE( pool.Length() == kNumElements );
		REQUIRE( !pool.HasFree() );
	}

	SECTION( "can iterate over allocated objects" )
	{
		uint32_t i = 0;
		for ( ae::LifetimeTester& obj : pool.Iterate< ae::LifetimeTester >() )
		{
			REQUIRE( obj.check == ae::LifetimeTester::kConstructed );
			REQUIRE( obj.value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects (const)" )
	{
		const auto* constPool = &pool;
		uint32_t i = 0;
		for ( const ae::LifetimeTester& obj : constPool->Iterate< ae::LifetimeTester >() )
		{
			REQUIRE( std::is_const_v< std::remove_reference_t< decltype( obj ) > > );
			REQUIRE( obj.check == ae::LifetimeTester::kConstructed );
			REQUIRE( obj.value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects after freeing some" )
	{
		pool.Delete( objects[ 0 ] );
		pool.Delete( objects[ kNumElements / 2 ] );
		pool.Delete( objects[ kNumElements - 1 ] );

		uint32_t count = 0;
		for ( const ae::LifetimeTester& obj : pool.Iterate< ae::LifetimeTester >() )
		{
			count++;
		}
		REQUIRE( count == kNumElements - 3 );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 3 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements - 3 );
	}

	pool.DeleteAll< ae::LifetimeTester >();
	REQUIRE( ae::LifetimeTester::ctorCount == ae::LifetimeTester::dtorCount );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

TEST_CASE( "aeOpaquePool Pages can be checked for objects", "[aeOpaquePool]" )
{
	ae::LifetimeTester::ClearStats();

	const uint32_t kNumElements = 10;
	ae::OpaquePool pool( TAG_POOL, sizeof(int32_t), alignof(int32_t), kNumElements, false );
	REQUIRE( pool.PageSize() == kNumElements );
	REQUIRE( pool.Size() == kNumElements );
	// REQUIRE( pool.Iterate< int32_t >().begin() == pool.Iterate< int32_t >().end() ); // @TODO: Catch can't convert this to a string
	REQUIRE( pool.Length() == 0 );

	// for ( uint32_t i = 0; i < kNumElements; i++ )
	// {
	// 	int32_t* p = pool.New< int32_t >();
	// 	REQUIRE( pool.IsInPool( p ) );
	// 	REQUIRE( !pool.IsInPool( (int32_t*)( (uint8_t*)p + 1 ) ) );
	// }
	
	// int32_t something = 0;
	// REQUIRE( !pool.IsInPool( &something ) );

	pool.DeleteAll< int32_t >();
}

TEST_CASE( "aeOpaquePool can allocate objects after clear", "[aeOpaquePool]" )
{
	const uint32_t kNumElements = 10;
	ae::OpaquePool pool( TAG_POOL, sizeof(int32_t), alignof(int32_t), kNumElements, false );
	REQUIRE( pool.PageSize() == kNumElements );
	REQUIRE( pool.Size() == kNumElements );
	// REQUIRE( pool.Iterate< int32_t >().begin() == pool.Iterate< int32_t >().end() ); // @TODO: Catch can't convert this to a string
	REQUIRE( pool.Length() == 0 );

	pool.DeleteAll< int32_t >();
	auto* p = pool.New< int32_t >();
	REQUIRE( p );
	REQUIRE( pool.Length() == 1 );
	pool.Delete( p );
	REQUIRE( pool.Length() == 0 );
}

//------------------------------------------------------------------------------
// ae::OpaquePool tests
//------------------------------------------------------------------------------
TEST_CASE( "Paged aeOpaquePool pool objects can be allocated and deallocated", "[aeOpaquePool]" )
{
	ae::LifetimeTester::ClearStats();

	const uint32_t kPageSize = 3;
	const uint32_t kNumElements = 10;
	ae::LifetimeTester* objects[ kNumElements ];
	ae::OpaquePool pool( TAG_POOL, sizeof(ae::LifetimeTester), alignof(ae::LifetimeTester), kPageSize, true );
	REQUIRE( pool.PageSize() == kPageSize );
	REQUIRE( pool.Size() == INT32_MAX );
	// REQUIRE( pool.Iterate< ae::LifetimeTester >().begin() == pool.Iterate< ae::LifetimeTester >().end() ); // @TODO: Catch can't convert this to a string
	REQUIRE( pool.Length() == 0 );

	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );

	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		objects[ i ] = pool.New< ae::LifetimeTester >();
		REQUIRE( objects[ i ] );
		objects[ i ]->value = 'a' + i;
	}
	REQUIRE( pool.Length() == kNumElements );

	REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == kNumElements );

	SECTION( "can free objects" )
	{
		for ( uint32_t i = 0; i < kNumElements; i++ )
		{
			auto* p = objects[ i ];
			uint32_t remaining = kNumElements - ( i + 1 );
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			pool.Delete( p );
			REQUIRE( pool.Length() == remaining );

			REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
			REQUIRE( ae::LifetimeTester::copyCount == 0 );
			REQUIRE( ae::LifetimeTester::moveCount == 0 );
			REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::dtorCount == i + 1 );
			REQUIRE( ae::LifetimeTester::currentCount == remaining );
		}
	}

	SECTION( "null pointers are handled gracefully" )
	{
		ae::LifetimeTester* nullObj = nullptr;
		pool.Delete( nullObj );
		REQUIRE( pool.Length() == kNumElements );
	}

	SECTION( "can iterate over allocated objects" )
	{
		uint32_t i = 0;
		for ( auto& obj : pool.Iterate< ae::LifetimeTester >() )
		{
			REQUIRE( obj.check == ae::LifetimeTester::kConstructed );
			REQUIRE( obj.value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects (const)" )
	{
		const auto* constPool = &pool;
		uint32_t i = 0;
		for ( auto& obj : constPool->Iterate< ae::LifetimeTester >() )
		{
			REQUIRE( std::is_const_v< std::remove_reference_t< decltype( obj ) > > );
			REQUIRE( obj.check == ae::LifetimeTester::kConstructed );
			REQUIRE( obj.value == 'a' + i );
			i++;
		}
		REQUIRE( i == kNumElements );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements );
	}

	SECTION( "can iterate over allocated objects after freeing some" )
	{
		pool.Delete( objects[ 0 ] );
		pool.Delete( objects[ kNumElements / 2 ] );
		pool.Delete( objects[ kNumElements - 1 ] );

		uint32_t count = 0;
		for ( const auto& _obj : pool.Iterate< ae::LifetimeTester >() )
		{
			count++;
		}
		REQUIRE( count == kNumElements - 3 );

		REQUIRE( ae::LifetimeTester::ctorCount == kNumElements );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 3 );
		REQUIRE( ae::LifetimeTester::currentCount == kNumElements - 3 );
	}

	pool.DeleteAll< ae::LifetimeTester >();
	REQUIRE( ae::LifetimeTester::ctorCount == ae::LifetimeTester::dtorCount );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

TEST_CASE( "aeOpaquePool can iterate after modification", "[aeOpaquePool]" )
{
	const uint32_t kPageSize = 3;
	ae::OpaquePool pool( TAG_POOL, sizeof(ae::LifetimeTester), alignof(ae::LifetimeTester), kPageSize, true );
	REQUIRE( pool.PageSize() == kPageSize );
	REQUIRE( pool.Length() == 0 );

	for( uint32_t i = 0; i < kPageSize; i++ )
	{
		pool.New< ae::LifetimeTester >()->value = i;
	}
	REQUIRE( pool.Length() == kPageSize );

	uint32_t idx = 0;
	for ( auto& obj : pool.Iterate< ae::LifetimeTester >() )
	{
		REQUIRE( obj.value == idx );
		idx++;
	}
	REQUIRE( idx == kPageSize );

	idx = 0;
	for ( auto& obj : pool.Iterate< ae::LifetimeTester >() )
	{
		REQUIRE( obj.value == idx );
		idx++;
		if( idx == kPageSize )
		{
			// Allocate at end of page
			pool.New< ae::LifetimeTester >()->value = kPageSize;
		}
	}
	REQUIRE( idx == kPageSize + 1 );

	pool.DeleteAll< ae::LifetimeTester >();
}
