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
// ae::Hash64 constexpr tests
//------------------------------------------------------------------------------
TEST_CASE( "Hash64 can be used in constexpr context", "[ae::Hash64]" )
{
	// Default construction and Get() are constexpr
	constexpr uint64_t basis = ae::Hash64().Get();
	static_assert( basis == 0xCBF29CE484222325ull, "FNV1a-64 offset basis mismatch" );
	REQUIRE( basis == 0xCBF29CE484222325ull );

	// HashString is constexpr — result is identical to runtime
	constexpr uint64_t compileTime = ae::Hash64().HashString( "hello" ).Get();
	const uint64_t runTime = ae::Hash64().HashString( "hello" ).Get();
	static_assert( compileTime == ae::Hash64().HashString( "hello" ).Get() );
	REQUIRE( compileTime == runTime );

	// Two different strings produce different hashes
	constexpr uint64_t hashA = ae::Hash64().HashString( "hello" ).Get();
	constexpr uint64_t hashB = ae::Hash64().HashString( "world" ).Get();
	static_assert( hashA != hashB );
	REQUIRE( hashA != hashB );

	// Empty string produces the offset basis unchanged
	constexpr uint64_t emptyHash = ae::Hash64().HashString( "" ).Get();
	REQUIRE( emptyHash == 0xCBF29CE484222325ull );
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
	constexpr const char* objectName = ae::GetTypeName< ae::Object >();
	constexpr ae::TypeId objectTypeId = ae::GetTypeIdFromName( objectName );
	constexpr ae::TypeId objectTypedHelperId = ae::GetTypeIdWithQualifiers< ae::Object >();
	constexpr ae::TypeId objectUnqualifiedHelperId = ae::GetTypeIdWithoutQualifiers< const ae::Object* >();
	static_assert( id == same, "Same string should produce same TypeId" );
	static_assert( id != diff, "Different strings should produce different TypeIds" );
	static_assert( ae::kInvalidTypeId == ae::TypeId(), "kInvalidTypeId should equal default TypeId" );
	static_assert( objectTypeId == ae::TypeId( "ae::Object" ), "TypeId should be constexpr when generated from a constexpr type name." );
	static_assert( objectTypedHelperId == objectTypeId, "GetTypeIdWithQualifiers<T>() should be constexpr." );
	static_assert( objectUnqualifiedHelperId == ae::TypeId( "ae::Object" ), "GetTypeIdWithoutQualifiers<T>() should be constexpr." );

	// Runtime checks to satisfy Catch2
	REQUIRE( id == same );
	REQUIRE( id != diff );
	REQUIRE( ae::kInvalidTypeId == ae::TypeId() );
	REQUIRE( objectTypeId == ae::TypeId( "ae::Object" ) );
	REQUIRE( objectTypeId == objectTypedHelperId );
	REQUIRE( objectUnqualifiedHelperId == ae::TypeId( "ae::Object" ) );
}

TEST_CASE( "TypeId from runtime type name is validated at runtime", "[ae::TypeId]" )
{
	constexpr ae::TypeId constexprTypeId = ae::GetTypeIdFromName( ae::GetTypeName< ae::Object >() );
	static_assert( constexprTypeId == ae::TypeId( "ae::Object" ) );

	const std::string runtimeTypeName = ae::GetTypeName< ae::Object >();
	const ae::TypeId runtimeTypeId = ae::GetTypeIdFromName( runtimeTypeName.c_str() );

	// This path must still work for runtime-provided names, but it cannot be
	// verified with static_assert because the input string is no longer constexpr.
	REQUIRE( runtimeTypeId == constexprTypeId );
	REQUIRE( runtimeTypeId == ae::GetTypeIdWithQualifiers< ae::Object >() );
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

//------------------------------------------------------------------------------
// ae::TypeId hashing tests
//------------------------------------------------------------------------------
TEST_CASE( "TypeId GetHash32 returns underlying uint32_t value", "[ae::TypeId]" )
{
	ae::TypeId id( "HashMe" );
	REQUIRE( ae::GetHash32( id ) == (uint32_t)id );
	REQUIRE( ae::GetHash32( id ) == ae::Hash32().HashString( "HashMe" ).Get() );

	// Invalid TypeId hashes to 0
	REQUIRE( ae::GetHash32( ae::kInvalidTypeId ) == 0u );
}

TEST_CASE( "TypeId GetHash64 returns underlying value widened to uint64_t", "[ae::TypeId]" )
{
	ae::TypeId id( "HashMe64" );
	REQUIRE( ae::GetHash64( id ) == (uint64_t)(uint32_t)id );

	// Two distinct TypeIds produce distinct hash64 values
	ae::TypeId other( "Other" );
	REQUIRE( ae::GetHash64( id ) != ae::GetHash64( other ) );

	// Invalid TypeId hashes to 0
	REQUIRE( ae::GetHash64( ae::kInvalidTypeId ) == 0ull );
}

//------------------------------------------------------------------------------
// ae::TypeId BinaryStream serialization tests
//------------------------------------------------------------------------------
TEST_CASE( "TypeId round-trips through BinaryStream", "[ae::TypeId]" )
{
	ae::TypeId original( "Serializable" );

	// Write via BinaryWriter (use const pointer to resolve overload unambiguously)
	uint8_t buffer[ 64 ] = {};
	ae::BinaryWriter writer( buffer, sizeof(buffer) );
	const ae::TypeId* writePtr = &original;
	Serialize( &writer, writePtr );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == sizeof( uint32_t ) );

	// Read back via BinaryReader (BinaryReader* → BinaryStream*, unambiguous)
	ae::BinaryReader reader( buffer, writer.GetOffset() );
	ae::TypeId restored;
	Serialize( &reader, &restored );
	REQUIRE( reader.IsValid() );
	REQUIRE( restored == original );
}

TEST_CASE( "TypeId BinaryWriter const write path works", "[ae::TypeId]" )
{
	ae::TypeId id( "ConstWrite" );

	uint8_t buffer[ 64 ] = {};
	ae::BinaryWriter writer( buffer, sizeof(buffer) );
	const ae::TypeId* writePtr = &id;
	Serialize( &writer, writePtr );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == sizeof( uint32_t ) );

	// Verify the raw bytes equal the underlying uint32_t in native byte order
	uint32_t raw = 0;
	memcpy( &raw, buffer, sizeof( raw ) );
	REQUIRE( raw == (uint32_t)id );
}

TEST_CASE( "TypeId kInvalidTypeId round-trips through BinaryStream", "[ae::TypeId]" )
{
	uint8_t buffer[ 64 ] = {};
	ae::BinaryWriter writer( buffer, sizeof(buffer) );
	Serialize( &writer, &ae::kInvalidTypeId );
	REQUIRE( writer.IsValid() );

	ae::BinaryReader reader( buffer, writer.GetOffset() );
	ae::TypeId restored( "NotInvalid" );
	Serialize( &reader, &restored );
	REQUIRE( reader.IsValid() );
	REQUIRE( restored == ae::kInvalidTypeId );
}
