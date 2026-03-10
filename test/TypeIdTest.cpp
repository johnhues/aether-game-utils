//------------------------------------------------------------------------------
// TypeIdTest.cpp
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

//------------------------------------------------------------------------------
// ae::Hash32 constexpr tests
//------------------------------------------------------------------------------
TEST_CASE( "Hash32 can be used in constexpr context", "[ae::Hash32]" )
{
	// Default construction and Get() are constexpr
	constexpr uint32_t basis = ae::Hash32().Get();
	static_assert( basis == 0x811c9dc5u, "FNV1a-32 offset basis mismatch" );
	REQUIRE( basis == 0x811c9dc5u );

	// HashString is constexpr — result is identical to runtime
	constexpr uint32_t compileTime = ae::Hash32().HashString( "hello" ).Get();
	const uint32_t runTime = ae::Hash32().HashString( "hello" ).Get();
	static_assert( compileTime == ae::Hash32().HashString( "hello" ).Get() );
	REQUIRE( compileTime == runTime );

	// Empty string produces the offset basis unchanged
	constexpr uint32_t emptyHash = ae::Hash32().HashString( "" ).Get();
	REQUIRE( emptyHash == 0x811c9dc5u );
}

//------------------------------------------------------------------------------
// ae::TypeId tests
//------------------------------------------------------------------------------
TEST_CASE( "TypeId default construction equals kInvalidTypeId", "[ae::TypeId]" )
{
	ae::TypeId id;
	REQUIRE( id == ae::kInvalidTypeId );
	REQUIRE( (uint32_t)id == 0u );
}

TEST_CASE( "TypeId constructed from string literal matches GetTypeIdFromName", "[ae::TypeId]" )
{
	// Construction from a string literal produces the same hash as GetTypeIdFromName
	ae::TypeId fromLiteral( "MyType" );
	ae::TypeId fromFunction = ae::GetTypeIdFromName( "MyType" );
	REQUIRE( fromLiteral == fromFunction );

	// Value equals a direct FNV1a-32 hash
	REQUIRE( fromLiteral == ae::TypeId( ae::Hash32().HashString( "MyType" ).Get() ) );
}

TEST_CASE( "TypeId from empty string equals kInvalidTypeId", "[ae::TypeId]" )
{
	ae::TypeId fromEmpty( "" );
	REQUIRE( fromEmpty == ae::kInvalidTypeId );
}

TEST_CASE( "TypeId can be implicitly constructed from string literal at call site", "[ae::TypeId]" )
{
	// A function accepting ae::TypeId can be called with a string literal directly.
	// Verify by constructing and comparing inline.
	auto accept = []( ae::TypeId id ) { return (uint32_t)id; };
	REQUIRE( accept( "MyType" ) == ae::Hash32().HashString( "MyType" ).Get() );
}

TEST_CASE( "TypeId equality and inequality operators", "[ae::TypeId]" )
{
	ae::TypeId a( "TypeA" );
	ae::TypeId b( "TypeB" );
	ae::TypeId a2( "TypeA" );

	REQUIRE( a == a2 );
	REQUIRE( a != b );
	REQUIRE( !( a == b ) );
	REQUIRE( !( a != a2 ) );
}

TEST_CASE( "TypeId is usable as a constexpr value", "[ae::TypeId]" )
{
	constexpr ae::TypeId id( "ConstexprType" );
	constexpr ae::TypeId same( "ConstexprType" );
	constexpr ae::TypeId diff( "OtherType" );
	static_assert( id == same, "Same string should produce same TypeId" );
	static_assert( id != diff, "Different strings should produce different TypeIds" );
	static_assert( ae::kInvalidTypeId == ae::TypeId(), "kInvalidTypeId should equal default TypeId" );

	// Runtime checks to satisfy Catch2
	REQUIRE( id == same );
	REQUIRE( id != diff );
	REQUIRE( ae::kInvalidTypeId == ae::TypeId() );
}

TEST_CASE( "TypeId operator uint32_t returns underlying value", "[ae::TypeId]" )
{
	ae::TypeId id( "SomeType" );
	uint32_t raw = (uint32_t)id;
	REQUIRE( raw == ae::Hash32().HashString( "SomeType" ).Get() );
	REQUIRE( raw != 0u );
}

TEST_CASE( "GetTypeIdFromName returns kInvalidTypeId for empty string", "[ae::TypeId]" )
{
	REQUIRE( ae::GetTypeIdFromName( "" ) == ae::kInvalidTypeId );
}

TEST_CASE( "GetTypeIdFromName is consistent with direct FNV1a-32 hash", "[ae::TypeId]" )
{
	const char* name = "ae::SomeRegisteredType";
	ae::TypeId id = ae::GetTypeIdFromName( name );
	REQUIRE( id == ae::TypeId( ae::Hash32().HashString( name ).Get() ) );
}
