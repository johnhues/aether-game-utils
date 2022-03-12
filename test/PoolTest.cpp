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

//------------------------------------------------------------------------------
// ae::ObjectPool tests
//------------------------------------------------------------------------------
TEST_CASE( "Objects can be allocated and deallocated", "[aeObjectPool]" )
{
	ae::LifetimeTester::ClearStats();

	const uint32_t kNumObjects = 10;
	ae::LifetimeTester* objects[ kNumObjects ];
	ae::ObjectPool< ae::LifetimeTester, kNumObjects > pool;

	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );

	for ( uint32_t i = 0; i < kNumObjects; i++ )
	{
		objects[ i ] = pool.Allocate();
		REQUIRE( objects[ i ] );
		objects[ i ]->value = 'a' + i;
	}
	REQUIRE( pool.Length() == kNumObjects );
	REQUIRE( !pool.HasFree() );

	REQUIRE( ae::LifetimeTester::ctorCount == 10 );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 10 );

	SECTION( "can free objects" )
	{
		for ( uint32_t i = 0; i < kNumObjects; i++ )
		{
			auto* p = objects[ i ];
			uint32_t remaining = kNumObjects - ( i + 1 );
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			REQUIRE( pool.Free( p ) );
			REQUIRE( pool.Length() == remaining );

			REQUIRE( ae::LifetimeTester::ctorCount == 10 );
			REQUIRE( ae::LifetimeTester::copyCount == 0 );
			REQUIRE( ae::LifetimeTester::moveCount == 0 );
			REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
			REQUIRE( ae::LifetimeTester::dtorCount == i + 1 );
			REQUIRE( ae::LifetimeTester::currentCount == remaining );
		}
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

		REQUIRE( ae::LifetimeTester::ctorCount == 10 );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == 10 );
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

		REQUIRE( ae::LifetimeTester::ctorCount == 10 );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == 10 );
	}

	SECTION( "can iterate over allocated objects after freeing some" )
	{
		const auto* constPool = &pool;
		uint32_t i = 0;
		for ( auto* p = constPool->GetFirst(); p; p = constPool->GetNext( p ) )
		{
			REQUIRE( p->check == ae::LifetimeTester::kConstructed );
			REQUIRE( p->value == 'a' + i );
			i++;
		}

		REQUIRE( ae::LifetimeTester::ctorCount == 10 );
		REQUIRE( ae::LifetimeTester::copyCount == 0 );
		REQUIRE( ae::LifetimeTester::moveCount == 0 );
		REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
		REQUIRE( ae::LifetimeTester::dtorCount == 0 );
		REQUIRE( ae::LifetimeTester::currentCount == 10 );
	}
}
