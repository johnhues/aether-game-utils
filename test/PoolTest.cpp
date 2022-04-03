//------------------------------------------------------------------------------
// PoolTest.cpp
// Copyright (c) John Hughes on 3/12/22. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "catch2/catch.hpp"
#include "ae/aether.h"
#include "ae/aeObjectPool.h" // @TODO: Move to aether.h
#include "ae/aeTesting.h"

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

	for ( uint32_t i = 0; i < kNumElements; i++ )
	{
		int32_t* p = pool.New();
		//REQUIRE( pool.IsInPool( p ) );
		//REQUIRE( !pool.IsInPool( (int32_t*)( (uint8_t*)p + 1 ) ) );
	}
	
	int32_t something = 0;
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
