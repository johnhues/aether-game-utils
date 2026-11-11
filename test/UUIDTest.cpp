//------------------------------------------------------------------------------
// UUIDTest.cpp
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
#include <thread>
#include <set>
#include <chrono>

//------------------------------------------------------------------------------
// UUID v7 Tests - RFC 9562 Compliance
//------------------------------------------------------------------------------

TEST_CASE( "UUID v7 version field is correct", "[uuid][uuid7]" )
{
	ae::UUID uuid = ae::UUID::Generate();
	// Version is in byte 6, high nibble (bits 48-51)
	// Version should be 0111 (7)
	uint8_t version = ( uuid.data[ 6 ] >> 4 ) & 0x0F;
	REQUIRE( version == 0x07 );
}

TEST_CASE( "UUID v7 variant field is correct", "[uuid][uuid7]" )
{
	ae::UUID uuid = ae::UUID::Generate();
	// Variant is in byte 8, high 2 bits
	// Should be 10 binary (RFC 9562 variant)
	uint8_t variantBits = ( uuid.data[ 8 ] >> 6 ) & 0x03;
	REQUIRE( variantBits == 0x02 ); // 10 in binary = 2 in decimal
}

TEST_CASE( "UUID v7 format validation", "[uuid][uuid7]" )
{
	ae::UUID uuid = ae::UUID::Generate();
	
	// Check version field (byte 6, high nibble)
	uint8_t version = ( uuid.data[ 6 ] >> 4 ) & 0x0F;
	REQUIRE( version == 0x07 );
	
	// Check variant field (byte 8, high 2 bits = 10b)
	uint8_t variantBits = ( uuid.data[ 8 ] >> 6 ) & 0x03;
	REQUIRE( variantBits == 0x02 );
}

TEST_CASE( "UUID v7 timestamp increases monotonically", "[uuid][uuid7]" )
{
	ae::UUID uuid1 = ae::UUID::Generate();
	
	// Small delay to ensure different timestamp
	std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
	
	ae::UUID uuid2 = ae::UUID::Generate();
	
	// Extract 48-bit timestamps (first 6 bytes, big-endian)
	uint64_t ts1 = 0;
	uint64_t ts2 = 0;
	
	for( int i = 0; i < 6; i++ )
	{
		ts1 = ( ts1 << 8 ) | uuid1.data[ i ];
		ts2 = ( ts2 << 8 ) | uuid2.data[ i ];
	}
	
	REQUIRE( ts2 >= ts1 );
}

TEST_CASE( "UUID v7 uniqueness", "[uuid][uuid7]" )
{
	const int numUUIDs = 10000;
	std::set< std::string > uuidSet;
	
	for( int i = 0; i < numUUIDs; i++ )
	{
		ae::UUID uuid = ae::UUID::Generate();
		std::string str = ae::ToString( uuid );
		uuidSet.insert( str );
	}
	
	// All UUIDs should be unique
	REQUIRE( uuidSet.size() == numUUIDs );
}

TEST_CASE( "UUID v7 string format", "[uuid][uuid7]" )
{
	ae::UUID uuid = ae::UUID::Generate();
	std::string str = ae::ToString( uuid );
	
	// Should be 36 characters: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
	REQUIRE( str.length() == 36 );
	
	// Check hyphens at correct positions
	REQUIRE( str[ 8 ] == '-' );
	REQUIRE( str[ 13 ] == '-' );
	REQUIRE( str[ 18 ] == '-' );
	REQUIRE( str[ 23 ] == '-' );
	
	// Check that version nibble in string is '7' (position 14 in string, byte 6 high nibble)
	REQUIRE( str[ 14 ] == '7' );
	
	// Check all characters are valid hex or hyphens
	for( size_t i = 0; i < str.length(); i++ )
	{
		if( i == 8 || i == 13 || i == 18 || i == 23 )
		{
			REQUIRE( str[ i ] == '-' );
		}
		else
		{
			bool isHex = ( str[ i ] >= '0' && str[ i ] <= '9' ) || 
			             ( str[ i ] >= 'a' && str[ i ] <= 'f' ) ||
			             ( str[ i ] >= 'A' && str[ i ] <= 'F' );
			REQUIRE( isHex );
		}
	}
}

TEST_CASE( "UUID v7 timestamp is reasonable", "[uuid][uuid7]" )
{
	ae::UUID uuid = ae::UUID::Generate();
	
	// Extract 48-bit timestamp (first 6 bytes, big-endian)
	uint64_t timestamp = 0;
	for( int i = 0; i < 6; i++ )
	{
		timestamp = ( timestamp << 8 ) | uuid.data[ i ];
	}
	
	// Get current time in milliseconds since epoch
	auto now = std::chrono::system_clock::now();
	uint64_t currentMs = std::chrono::duration_cast< std::chrono::milliseconds >( 
		now.time_since_epoch() 
	).count();
	
	// UUID timestamp should be within 1 second of current time
	uint64_t diff = ( timestamp > currentMs ) ? ( timestamp - currentMs ) : ( currentMs - timestamp );
	REQUIRE( diff < 1000 ); // Less than 1 second difference
}

TEST_CASE( "UUID v7 equality operators", "[uuid][uuid7]" )
{
	ae::UUID uuid1 = ae::UUID::Generate();
	ae::UUID uuid2 = ae::UUID::Generate();
	
	// Two different UUIDs should not be equal
	REQUIRE( uuid1 != uuid2 );
	REQUIRE_FALSE( uuid1 == uuid2 );
	
	// Same UUID should be equal to itself
	REQUIRE( uuid1 == uuid1 );
	REQUIRE_FALSE( uuid1 != uuid1 );
	
	// Copy should be equal
	ae::UUID uuid3 = uuid1;
	REQUIRE( uuid3 == uuid1 );
	REQUIRE_FALSE( uuid3 != uuid1 );
}

TEST_CASE( "UUID v7 default constructor creates zero UUID", "[uuid][uuid7]" )
{
	ae::UUID uuid;
	
	// All bytes should be zero
	for( int i = 0; i < 16; i++ )
	{
		REQUIRE( uuid.data[ i ] == 0 );
	}
}

TEST_CASE( "UUID v7 randomness distribution", "[uuid][uuid7]" )
{
	// Generate multiple UUIDs rapidly to check that random bits vary
	const int numUUIDs = 100;
	std::vector< ae::UUID > uuids;
	
	for( int i = 0; i < numUUIDs; i++ )
	{
		uuids.push_back( ae::UUID::Generate() );
	}
	
	// Check that the random portion (rand_a and rand_b) varies
	// rand_a is in byte 6 (low nibble) and byte 7
	// rand_b is in bytes 8-15 (with byte 8 having variant in top 2 bits)
	
	std::set< uint16_t > randAValues;
	std::set< uint64_t > randBValues;
	
	for( const auto& uuid : uuids )
	{
		// Extract rand_a (12 bits)
		uint16_t randA = static_cast<uint16_t>( ( ( uuid.data[ 6 ] & 0x0F ) << 8 ) | uuid.data[ 7 ] );
		randAValues.insert( randA );
		
		// Extract rand_b (62 bits, from bytes 8-15)
		uint64_t randB = 0;
		for( int i = 8; i < 16; i++ )
		{
			randB = ( randB << 8 ) | uuid.data[ i ];
		}
		randB &= 0x3FFFFFFFFFFFFFFFULL; // Mask to 62 bits
		randBValues.insert( randB );
	}
	
	// We expect good randomness - at least 90% unique values for rand_a
	REQUIRE( randAValues.size() >= numUUIDs * 0.9 );
	
	// rand_b should be nearly 100% unique (62 bits is a huge space)
	REQUIRE( randBValues.size() >= numUUIDs * 0.99 );
}

TEST_CASE( "UUID v7 concurrent generation", "[uuid][uuid7]" )
{
	const int numThreads = 4;
	const int uuidsPerThread = 1000;
	std::vector< std::thread > threads;
	std::vector< std::vector< ae::UUID > > results( numThreads );
	
	// Generate UUIDs concurrently from multiple threads
	for( int t = 0; t < numThreads; t++ )
	{
		threads.emplace_back( [t, &results, uuidsPerThread]() {
			for( int i = 0; i < uuidsPerThread; i++ )
			{
				results[ t ].push_back( ae::UUID::Generate() );
			}
		} );
	}
	
	// Wait for all threads
	for( auto& thread : threads )
	{
		thread.join();
	}
	
	// Collect all UUIDs and verify uniqueness
	std::set< std::string > allUUIDs;
	for( const auto& threadResults : results )
	{
		for( const auto& uuid : threadResults )
		{
			allUUIDs.insert( ae::ToString( uuid ) );
		}
	}
	
	REQUIRE( allUUIDs.size() == numThreads * uuidsPerThread );
}

TEST_CASE( "UUID v7 sortability by timestamp", "[uuid][uuid7]" )
{
	std::vector< ae::UUID > uuids;
	
	// Generate UUIDs with small delays
	for( int i = 0; i < 10; i++ )
	{
		uuids.push_back( ae::UUID::Generate() );
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}
	
	// Extract timestamps and verify they're monotonically increasing
	std::vector< uint64_t > timestamps;
	for( const auto& uuid : uuids )
	{
		uint64_t ts = 0;
		for( int i = 0; i < 6; i++ )
		{
			ts = ( ts << 8 ) | uuid.data[ i ];
		}
		timestamps.push_back( ts );
	}
	
	// Check monotonicity
	for( size_t i = 1; i < timestamps.size(); i++ )
	{
		REQUIRE( timestamps[ i ] >= timestamps[ i - 1 ] );
	}
}

