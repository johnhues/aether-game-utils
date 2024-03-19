//------------------------------------------------------------------------------
// SerializeTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2023 John Hughes
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

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_TEST = "test";

enum class SerializeEnum
{
	Zero,
	One,
	Two,
	Three
};

//------------------------------------------------------------------------------
// TestStruct
//------------------------------------------------------------------------------
AE_PACK( struct TestStruct
{
	uint8_t uint8 = 0;
	uint16_t uint16 = 0;
	uint32_t uint32 = 0;
	uint64_t uint64 = 0;
	int8_t int8 = 0;
	int16_t int16 = 0;
	int32_t int32 = 0;
	int64_t int64 = 0;
	float f = 0.0f;
	double d = 0.0;
	bool b = false;
	SerializeEnum e = SerializeEnum::Zero;
	uint32_t rawData[ 6 ] = { 0 };

	void Initialize()
	{
		uint8 = 0x01;
		uint16 = 0x0203;
		uint32 = 0x04050607;
		uint64 = 0x08090A0B0C0D0E0F;
		int8 = 0x11;
		int16 = 0x1213;
		int32 = 0x14151617;
		int64 = 0x18191A1B1C1D1E1F;
		f = 1.0f;
		d = 2.0;
		b = true;
		e = SerializeEnum::Two;
		rawData[ 0 ] = 0x31323334;
		rawData[ 1 ] = 0x35363738;
		rawData[ 2 ] = 0x393A3B3C;
		rawData[ 3 ] = 0x3D3E3F40;
		rawData[ 4 ] = 0x41424344;
		rawData[ 5 ] = 0x45464748;
	}

	void DoSerialize( ae::BinaryStream* stream )
	{
		stream->SerializeUint8( uint8 );
		stream->SerializeUint16( uint16 );
		stream->SerializeUint32( uint32 );
		stream->SerializeUint64( uint64 );
		stream->SerializeInt8( int8 );
		stream->SerializeInt16( int16 );
		stream->SerializeInt32( int32 );
		stream->SerializeInt64( int64 );
		stream->SerializeFloat( f );
		stream->SerializeDouble( d );
		stream->SerializeBool( b );
		stream->SerializeEnum( e );
		stream->SerializeRaw( rawData, sizeof(rawData) );
	}

	void DoSerialize( ae::BinaryStream* stream ) const
	{
		stream->SerializeUint8( uint8 );
		stream->SerializeUint16( uint16 );
		stream->SerializeUint32( uint32 );
		stream->SerializeUint64( uint64 );
		stream->SerializeInt8( int8 );
		stream->SerializeInt16( int16 );
		stream->SerializeInt32( int32 );
		stream->SerializeInt64( int64 );
		stream->SerializeFloat( f );
		stream->SerializeDouble( d );
		stream->SerializeBool( b );
		stream->SerializeEnum( e );
		stream->SerializeRaw( rawData, sizeof(rawData) );
	}
} );

//------------------------------------------------------------------------------
// MemberSerializeClass
//------------------------------------------------------------------------------
class MemberSerializeClass
{
public:
	void Serialize( ae::BinaryStream* stream )
	{
		stream->SerializeUint32( data );
	}
	void Serialize( ae::BinaryStream* stream ) const
	{
		stream->SerializeUint32( data );
	}

	uint32_t data = 1;
};

//------------------------------------------------------------------------------
// StaticSerializeClass
//------------------------------------------------------------------------------
class StaticSerializeClass
{
public:
	uint32_t data = 2;
};
void Serialize( ae::BinaryStream* stream, StaticSerializeClass* obj )
{
	stream->SerializeUint32( obj->data );
}
void Serialize( ae::BinaryStream* stream, const StaticSerializeClass* obj )
{
	stream->SerializeUint32( obj->data );
}

//------------------------------------------------------------------------------
// ConstMemberSerializeClass
//------------------------------------------------------------------------------
class ConstMemberSerializeClass
{
public:
	void Serialize( ae::BinaryStream* stream ) const
	{
		stream->SerializeUint32( data );
	}

	uint32_t data = 3;
};

//------------------------------------------------------------------------------
// ae::BinaryStream
//------------------------------------------------------------------------------
TEST_CASE( "BinaryStream::Writer( void* data, uint32_t length )", "[ae::BinaryStream]" )
{
	SECTION( "Create writer with a valid data pointer and length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryStream writer = ae::BinaryStream::Writer( buffer, sizeof(buffer) );
		REQUIRE( writer.IsWriter() );
		REQUIRE( writer.IsValid() );
		REQUIRE( writer.GetData() == buffer );
		REQUIRE( writer.GetOffset() == 0 );
		REQUIRE( writer.GetRemainingBytes() == sizeof(buffer) );
		REQUIRE( writer.GetSize() == sizeof(buffer) );
		REQUIRE_THROWS( writer.PeekReadData() ); // Stream is writer
		REQUIRE_THROWS( writer.DiscardReadData( 1 ) ); // Stream is writer
	}
	
	SECTION( "Create writer with a null data pointer with length 0" )
	{
		ae::BinaryStream writer = ae::BinaryStream::Writer( nullptr, 0 );
		REQUIRE( writer.IsWriter() );
		REQUIRE( !writer.IsValid() );
		REQUIRE( writer.GetData() == nullptr );
		REQUIRE( writer.GetOffset() == 0 );
		REQUIRE( writer.GetRemainingBytes() == 0 );
		REQUIRE( writer.GetSize() == 0 );
		REQUIRE_THROWS( writer.PeekReadData() ); // Stream is writer
		REQUIRE_THROWS( writer.DiscardReadData( 1 ) ); // Stream is writer
	}
	
	SECTION( "Writing with a valid data pointer and 0 length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryStream writer = ae::BinaryStream::Writer( buffer, 0 );
		REQUIRE( writer.IsWriter() );
		REQUIRE( !writer.IsValid() );
		REQUIRE( writer.GetData() == buffer );
		REQUIRE( writer.GetOffset() == 0 );
		REQUIRE( writer.GetRemainingBytes() == 0 ); // Stream is invalid
		REQUIRE( writer.GetSize() == 0 );
		REQUIRE_THROWS( writer.PeekReadData() ); // Stream is writer
		REQUIRE_THROWS( writer.DiscardReadData( 1 ) ); // Stream is writer
	}
}

TEST_CASE( "BinaryStream::Writer( Array< uint8_t >* array )", "[ae::BinaryStream]" )
{
	SECTION( "Create writer with a valid empty array" )
	{
		ae::Array< uint8_t > buffer = TAG_TEST;
		ae::BinaryStream writer = ae::BinaryStream::Writer( &buffer );
		REQUIRE( writer.IsWriter() );
		REQUIRE( writer.IsValid() );
		REQUIRE( writer.GetData() == buffer.Data() );
		REQUIRE( writer.GetOffset() == 0 );
		REQUIRE( writer.GetRemainingBytes() == 0 ); // Stream storage is dynamic
		REQUIRE( writer.GetSize() == 0 ); // Stream storage is dynamic
		REQUIRE( writer.GetSize() == buffer.Size() ); // Stream storage is dynamic
		REQUIRE_THROWS( writer.PeekReadData() ); // Stream is writer
		REQUIRE_THROWS( writer.DiscardReadData( 1 ) ); // Stream is writer
	}

	SECTION( "Create writer with a valid non-zero length array" )
	{
		ae::Array< uint8_t > buffer = TAG_TEST;
		buffer.Append( 1 );
		buffer.Append( 2 );
		buffer.Append( 3 );
		ae::BinaryStream writer = ae::BinaryStream::Writer( &buffer );
		REQUIRE( writer.IsWriter() );
		REQUIRE( writer.IsValid() );
		REQUIRE( writer.GetData() == buffer.Data() ); // Stream storage is dynamic
		REQUIRE( writer.GetOffset() == buffer.Length() ); // Stream storage is dynamic
		REQUIRE( writer.GetRemainingBytes() == buffer.Size() - buffer.Length() ); // Stream storage is dynamic
		REQUIRE( writer.GetSize() == buffer.Size() ); // Stream storage is dynamic
		REQUIRE_THROWS( writer.PeekReadData() ); // Stream is writer
		REQUIRE_THROWS( writer.DiscardReadData( 1 ) ); // Stream is writer
	}
}

TEST_CASE( "BinaryStream::Reader( const void* data, uint32_t length )", "[ae::BinaryStream]" )
{
	SECTION( "Create reader with a valid data pointer and length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryStream reader = ae::BinaryStream::Reader( buffer, sizeof(buffer) );
		REQUIRE( reader.IsReader() );
		REQUIRE( reader.IsValid() );
		REQUIRE( reader.GetData() == buffer );
		REQUIRE( reader.GetOffset() == 0 );
		REQUIRE( reader.GetSize() == sizeof(buffer) );
		REQUIRE( reader.PeekReadData() == buffer );
		REQUIRE( reader.GetRemainingBytes() == sizeof(buffer) );
	}
	
	SECTION( "Create reader with a null data pointer with length 0" )
	{
		ae::BinaryStream reader = ae::BinaryStream::Reader( nullptr, 0 );
		REQUIRE( reader.IsReader() );
		REQUIRE( !reader.IsValid() );
		REQUIRE( reader.GetData() == nullptr );
		REQUIRE( reader.GetOffset() == 0 );
		REQUIRE( reader.GetSize() == 0 );
		REQUIRE( reader.PeekReadData() == nullptr );
		REQUIRE( reader.GetRemainingBytes() == 0 );
	}
	
	SECTION( "Reading with a valid data pointer and 0 length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryStream reader = ae::BinaryStream::Reader( buffer, 0 );
		REQUIRE( reader.IsReader() );
		REQUIRE( !reader.IsValid() );
		REQUIRE( reader.GetData() == buffer );
		REQUIRE( reader.GetOffset() == 0 );
		REQUIRE( reader.GetSize() == 0 );
		REQUIRE( reader.PeekReadData() == nullptr );
		REQUIRE( reader.GetRemainingBytes() == 0 );
	}
}

TEST_CASE( "BinaryStream::Reader( const Array< uint8_t >& data )", "[ae::BinaryStream]" )
{
	SECTION( "Create reader with a valid empty array" )
	{
		ae::Array< uint8_t > buffer = TAG_TEST;
		ae::BinaryStream reader = ae::BinaryStream::Reader( buffer );
		REQUIRE( reader.IsReader() );
		REQUIRE( !reader.IsValid() );
		REQUIRE( reader.GetData() == buffer.Data() );
		REQUIRE( reader.GetOffset() == 0 );
		REQUIRE( reader.GetSize() == buffer.Size() );
		REQUIRE( reader.PeekReadData() == nullptr );
		REQUIRE( reader.GetRemainingBytes() == 0 );
	}

	SECTION( "Create reader with a valid non-zero length array" )
	{
		ae::Array< uint8_t > _buffer = TAG_TEST;
		_buffer.Append( 1 );
		_buffer.Append( 2 );
		_buffer.Append( 3 );
		const ae::Array< uint8_t >& buffer = _buffer;
		ae::BinaryStream reader = ae::BinaryStream::Reader( buffer );
		REQUIRE( reader.IsReader() );
		REQUIRE( reader.IsValid() );
		REQUIRE( reader.GetData() == buffer.Data() );
		REQUIRE( reader.GetOffset() == 0 );
		REQUIRE( reader.GetSize() == buffer.Length() );
		REQUIRE( reader.PeekReadData() == buffer.Data() );
		REQUIRE( reader.GetRemainingBytes() == buffer.Length() );
	}
}

TEST_CASE( "BinaryStream writer serialize basic values", "ae::BinaryStream" )
{

	TestStruct _test;
	_test.Initialize();

	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryStream writer = ae::BinaryStream::Writer( &buffer );
	const TestStruct& test = _test;
	test.DoSerialize( &writer );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == sizeof(TestStruct) );
	REQUIRE( buffer.Data() == writer.GetData() );
	REQUIRE( buffer.Length() == sizeof(TestStruct) );

	for( uint32_t i = 0; i < sizeof(TestStruct); i++ )
	{
		REQUIRE( buffer[ i ] == reinterpret_cast< const uint8_t* >( &test )[ i ] );
	}
}

TEST_CASE( "BinaryStream reader serialize basic values", "ae::BinaryStream" )
{
	TestStruct test0;
	test0.Initialize();
	ae::Array< uint8_t > buffer = TAG_TEST;
	buffer.AppendArray( reinterpret_cast< const uint8_t* >( &test0 ), sizeof(TestStruct) );

	TestStruct test1;
	ae::BinaryStream reader = ae::BinaryStream::Reader( buffer );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetData() == buffer.Data() );
	REQUIRE( reader.GetOffset() == 0 );
	REQUIRE( reader.GetSize() == buffer.Size() );
	REQUIRE( reader.PeekReadData() == buffer.Data() );
	REQUIRE( reader.GetRemainingBytes() == buffer.Length() );
	test1.DoSerialize( &reader );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetOffset() == sizeof(TestStruct) );
	REQUIRE( reader.PeekReadData() == buffer.Data() + sizeof(TestStruct) );
	REQUIRE( reader.GetRemainingBytes() == 0 );
	REQUIRE( memcmp( &test0, &test1, sizeof(TestStruct) ) == 0 );
}

TEST_CASE( "Try to write past end of data buffer", "ae::BinaryStream" )
{
	uint8_t buffer[ 10 ] = { 0 };
	ae::BinaryStream writer = ae::BinaryStream::Writer( buffer, sizeof(buffer) );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == 0 );
	writer.SerializeUint32( 0x12345678u );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == sizeof(uint32_t) );
	writer.SerializeUint32( 0x12345678u );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == sizeof(uint32_t) * 2 );
	writer.SerializeUint32( 0x12345678u );
	REQUIRE( !writer.IsValid() );
	REQUIRE( writer.GetOffset() == sizeof(uint32_t) * 2 );

	REQUIRE( buffer[ 0 ] == 0x78 );
	REQUIRE( buffer[ 1 ] == 0x56 );
	REQUIRE( buffer[ 2 ] == 0x34 );
	REQUIRE( buffer[ 3 ] == 0x12 );
	REQUIRE( buffer[ 4 ] == 0x78 );
	REQUIRE( buffer[ 5 ] == 0x56 );
	REQUIRE( buffer[ 6 ] == 0x34 );
	REQUIRE( buffer[ 7 ] == 0x12 );
	REQUIRE( buffer[ 8 ] == 0 );
	REQUIRE( buffer[ 9 ] == 0 );
}

TEST_CASE( "Try to read past end of data buffer", "ae::BinaryStream" )
{
	uint8_t buffer[ 10 ];
	memset( buffer, 0x22, sizeof(buffer) );
	ae::BinaryStream reader = ae::BinaryStream::Reader( buffer, sizeof(buffer) );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetOffset() == 0 );

	uint32_t check = 0xEFEFEFEF;
	reader.SerializeUint32( check );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetOffset() == sizeof(uint32_t) );
	REQUIRE( check == 0x22222222 );

	check = 0xEFEFEFEF;
	reader.SerializeUint32( check );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( check == 0x22222222 );

	check = 0xEFEFEFEF;
	reader.SerializeUint32( check );
	REQUIRE( !reader.IsValid() );
	REQUIRE( reader.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( check == 0xEFEFEFEF );
}

TEST_CASE( "C string test", "ae::BinaryStream" )
{
	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryStream writer = ae::BinaryStream::Writer( &buffer );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == 0 );
	writer.SerializeString( "Hello, World!" );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == buffer.Length() );
	REQUIRE( writer.GetOffset() == sizeof("Hello, World!") ); // Length of string + null terminator

	ae::BinaryStream reader = ae::BinaryStream::Reader( buffer );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetOffset() == 0 );
	REQUIRE( reader.GetRemainingBytes() == buffer.Length() );
	char str[ sizeof("Hello, World!") ];
	memset( str, 0xAB, sizeof(str) );
	reader.SerializeString( str, sizeof(str) );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetOffset() == sizeof("Hello, World!") );
	REQUIRE( strcmp( str, "Hello, World!" ) == 0 );
}

TEST_CASE( "C string test with buffer length 0", "ae::BinaryStream" )
{
	ae::Array< uint8_t > buffer = TAG_TEST;
	char str[ sizeof("Hello, World!") ];
	strcpy( str, "Hello, World!" );
	ae::BinaryStream writer = ae::BinaryStream::Writer( &buffer );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == 0 );
	REQUIRE_THROWS( writer.SerializeString( str, 0 ) );

	ae::BinaryStream reader = ae::BinaryStream::Reader( buffer );
	REQUIRE_THROWS( reader.SerializeString( str, 0 ) );
}

TEST_CASE( "C string target buffer too small", "ae::BinaryStream" )
{
	char str[ 5 ];
	ae::BinaryStream writer = ae::BinaryStream::Writer( str, sizeof(str) );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == 0 );
	writer.SerializeString( "Hello, World!" );
	REQUIRE( !writer.IsValid() );
}

TEST_CASE( "C string write buffer too small", "ae::BinaryStream" )
{
	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryStream writer = ae::BinaryStream::Writer( &buffer );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == 0 );
	char str[ sizeof("Hello, World!") ];
	strcpy( str, "Hello, World!" );
	writer.SerializeString( str, 5 );
	REQUIRE( !writer.IsValid() );
}

TEST_CASE( "C string read buffer too small", "ae::BinaryStream" )
{
	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryStream writer = ae::BinaryStream::Writer( &buffer );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == 0 );
	writer.SerializeString( "Hello, World!" );
	REQUIRE( writer.IsValid() );
	REQUIRE( writer.GetOffset() == buffer.Length() );
	REQUIRE( writer.GetOffset() == sizeof("Hello, World!") ); // Length of string + null terminator

	ae::BinaryStream reader = ae::BinaryStream::Reader( buffer );
	REQUIRE( reader.IsValid() );
	REQUIRE( reader.GetOffset() == 0 );
	REQUIRE( reader.GetRemainingBytes() == buffer.Length() );
	char str[ sizeof("Hello, World!") ];
	memset( str, 0xAB, sizeof(str) );
	reader.SerializeString( str, 5 ); // Too small
	REQUIRE( !reader.IsValid() );
	REQUIRE( str[ 0 ] == 0 );
}

#if 0

template< uint32_t N > void SerializeString( Str< N >& str );
//! Writer mode only
template< uint32_t N > void SerializeString( const Str< N >& str );
//! Writer mode only
void SerializeString( const char* str );

//! Use SerializeObjectConditional() when an object may not be available for
//! serialization when writing or reading. This function correctly updates
//! read/write offsets when skipping serialization. Sends slightly more data
//! than SerializeObject().
template< typename T > void SerializeObjectConditional( T* obj );

template< uint32_t N > void SerializeArray( char (&str)[ N ] );
template< uint32_t N > void SerializeArray( const char (&str)[ N ] );
void SerializeArray( Array< uint8_t >& array, uint32_t maxLength = 65535 );
void SerializeArray( const Array< uint8_t >& array, uint32_t maxLength = 65535 );

void SerializeRaw( Array< uint8_t >& array );
void SerializeRaw( const Array< uint8_t >& array );

// Once the stream is invalid serialization calls will result in silent no-ops
void Invalidate() { m_isValid = false; }

//! Returns the data at the current read head. One use case is with
//! ae::BinaryStream::GetRemainingBytes() and ae::BinaryStream::DiscardReadData()
//! to read chunks of data into another binary stream, so individual chunks
//! can be invalidated separately. Returns null if called on an invalid or
//! writer stream.
const uint8_t* PeekReadData() const;
//! Returns the number of bytes remaining in the buffer. Returns 0 if called
//! on an invalid or writer stream.
uint32_t GetRemainingBytes() const;
//! Advances the read/write head by \p length bytes. If the end of the buffer
//! is reached the stream is invalidated. Has no effect if called on an
//! invalid or writer stream.
void DiscardReadData( uint32_t length );

#endif

TEST_CASE( "Member serialize function should be successfully detected", "[ae::BinaryStream]" )
{
	REQUIRE( ae::HasSerializeMethod< MemberSerializeClass >::value );
	REQUIRE( ae::HasConstSerializeMethod< MemberSerializeClass >::value );
	
	REQUIRE( !ae::HasSerializeMethod< StaticSerializeClass >::value );
	REQUIRE( !ae::HasConstSerializeMethod< StaticSerializeClass >::value );
	
	REQUIRE( !ae::HasSerializeMethod< ConstMemberSerializeClass >::value );
	REQUIRE( ae::HasConstSerializeMethod< ConstMemberSerializeClass >::value );
}

TEST_CASE( "Writer: Member and static function Serialize() should be called by SerializeObject()", "[ae::BinaryStream]" )
{
	MemberSerializeClass m;
	StaticSerializeClass s;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer, sizeof(buffer) );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetSize() == sizeof(buffer) );

	wStream.SerializeObject( m );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	wStream.SerializeObject( s );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

	ae::BinaryStream rStream = ae::BinaryStream::Reader( wStream.GetData(), wStream.GetOffset() );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == 0 );

	uint32_t check = 0;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) );
	REQUIRE( check == 1 );
	check = 0;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( check == 2 );

	REQUIRE( rStream.GetOffset() == rStream.GetSize() );
}

TEST_CASE( "Writer: Const member and static function Serialize() should be called by SerializeObject()", "[ae::BinaryStream]" )
{
	MemberSerializeClass m;
	StaticSerializeClass s;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer, sizeof(buffer) );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetSize() == sizeof(buffer) );

	wStream.SerializeObject( *const_cast< const MemberSerializeClass* >( &m ) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	wStream.SerializeObject( *const_cast< const StaticSerializeClass* >( &s ) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

	ae::BinaryStream rStream = ae::BinaryStream::Reader( wStream.GetData(), wStream.GetOffset() );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == 0 );

	uint32_t check = 0;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) );
	REQUIRE( check == 1 );
	check = 0;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( check == 2 );

	REQUIRE( rStream.GetOffset() == rStream.GetSize() );
}

TEST_CASE( "Writer: Support only having a const Serialize() that is called by SerializeObject()", "[ae::BinaryStream]" )
{
	ConstMemberSerializeClass c;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer, sizeof(buffer) );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetSize() == sizeof(buffer) );

	wStream.SerializeObject( *const_cast< const ConstMemberSerializeClass* >( &c ) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	wStream.SerializeObject( c );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

	ae::BinaryStream rStream = ae::BinaryStream::Reader( wStream.GetData(), wStream.GetOffset() );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == 0 );

	uint32_t check = 0;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) );
	REQUIRE( check == 3 );
	check = 0;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( check == 3 );

	REQUIRE( rStream.GetOffset() == rStream.GetSize() );
}

TEST_CASE( "Reader: Member and static function Serialize() should be called by SerializeObject()", "[ae::BinaryStream]" )
{
	MemberSerializeClass m;
	StaticSerializeClass s;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer, sizeof(buffer) );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetSize() == sizeof(buffer) );

	uint32_t check = 666;
	wStream.SerializeUint32( check );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	check = 777;
	wStream.SerializeUint32( check );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

	ae::BinaryStream rStream = ae::BinaryStream::Reader( wStream.GetData(), wStream.GetOffset() );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == 0 );

	rStream.SerializeObject( m );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) );
	REQUIRE( m.data == 666 );
	rStream.SerializeObject( s );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( s.data == 777 );

	REQUIRE( rStream.GetOffset() == rStream.GetSize() );
}
