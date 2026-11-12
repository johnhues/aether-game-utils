//------------------------------------------------------------------------------
// CompactingAllocatorTest.cpp
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
#include "ae/aeCompactingAllocator.h"
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// aeCompactingAllocator tests
//------------------------------------------------------------------------------
TEST_CASE( "aeCompactingAllocator allocation is writable and freeing clears the pointer", "[aeCompactingAllocator]" )
{
	aeCompactingAllocator allocator( 1024 );
	uint32_t* a = nullptr;
	allocator.Allocate( &a, sizeof( uint32_t ) * 4 );
	REQUIRE( a );
	for( uint32_t i = 0; i < 4; i++ ) { a[ i ] = i; }
	for( uint32_t i = 0; i < 4; i++ ) { REQUIRE( a[ i ] == i ); }
	allocator.Free( &a );
	REQUIRE( !a );
}

TEST_CASE( "aeCompactingAllocator compaction rewrites pointers and preserves contents", "[aeCompactingAllocator]" )
{
	aeCompactingAllocator allocator( 1024 );
	uint32_t* a = nullptr;
	uint32_t* b = nullptr;
	uint32_t* c = nullptr;
	allocator.Allocate( &a, sizeof( uint32_t ) );
	allocator.Allocate( &b, sizeof( uint32_t ) );
	allocator.Allocate( &c, sizeof( uint32_t ) );
	*a = 1;
	*b = 2;
	*c = 3;
	const uint32_t* const bBefore = b;

	allocator.Free( &a );
	uint32_t* d = nullptr;
	allocator.Allocate( &d, sizeof( uint32_t ) );
	*d = 4;
	REQUIRE( b != bBefore );
	REQUIRE( *b == 2 );
	REQUIRE( *c == 3 );
	REQUIRE( *d == 4 );
	REQUIRE( b < c );
	REQUIRE( c < d );

	allocator.Free( &b );
	allocator.Free( &c );
	allocator.Free( &d );
}

TEST_CASE( "aeCompactingAllocator allocations are aligned", "[aeCompactingAllocator]" )
{
	aeCompactingAllocator allocator( 1024 );
	uint8_t* a = nullptr;
	uint8_t* b = nullptr;
	uint8_t* c = nullptr;
	allocator.Allocate( &a, 1 );
	allocator.Allocate( &b, 3 );
	allocator.Allocate( &c, 17 );
	REQUIRE( (uintptr_t)a % 16 == 0 );
	REQUIRE( (uintptr_t)b % 16 == 0 );
	REQUIRE( (uintptr_t)c % 16 == 0 );
	allocator.Free( &a );
	allocator.Free( &b );
	allocator.Free( &c );
}

TEST_CASE( "aeCompactingAllocator reuses space freed after an unmoved allocation", "[aeCompactingAllocator]" )
{
	aeCompactingAllocator allocator( 1024 );
	uint32_t* a = nullptr;
	uint32_t* b = nullptr;
	allocator.Allocate( &a, sizeof( uint32_t ) );
	allocator.Allocate( &b, sizeof( uint32_t ) );
	*a = 1;
	const uint32_t* const bBefore = b;

	allocator.Free( &b );
	uint32_t* c = nullptr;
	allocator.Allocate( &c, sizeof( uint32_t ) );
	*c = 3;
	REQUIRE( c == bBefore );
	REQUIRE( *a == 1 );

	allocator.Free( &a );
	allocator.Free( &c );
}
