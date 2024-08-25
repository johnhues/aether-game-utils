//------------------------------------------------------------------------------
// SerializeTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2024 John Hughes
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

	void DoSerialize( ae::BinaryWriter* stream ) const
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
	void Serialize( ae::BinaryWriter* stream ) const
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
void Serialize( ae::BinaryWriter* stream, const StaticSerializeClass* obj )
{
	stream->SerializeUint32( obj->data );
}

//------------------------------------------------------------------------------
// ConstMemberSerializeClass
//------------------------------------------------------------------------------
class ConstMemberSerializeClass
{
public:
	void Serialize( ae::BinaryWriter* stream ) const
	{
		stream->SerializeUint32( data );
	}

	uint32_t data = 3;
};

//------------------------------------------------------------------------------
// ae::BinaryStream
//------------------------------------------------------------------------------
TEST_CASE( "BinaryWriter( void* data, uint32_t length )", "[ae::BinaryStream]" )
{
	SECTION( "Create writer with a valid data pointer and length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryWriter wStream( buffer, sizeof(buffer) );
		REQUIRE( wStream.AsWriter() == &wStream );
		REQUIRE( !wStream.AsReader() );
		REQUIRE( wStream.IsValid() );
		REQUIRE( wStream.GetData() == buffer );
		REQUIRE( wStream.GetOffset() == 0 );
		REQUIRE( wStream.GetRemainingBytes() == sizeof(buffer) );
		REQUIRE( wStream.GetSize() == sizeof(buffer) );
	}
	
	SECTION( "Create writer with a null data pointer with length 0" )
	{
		ae::BinaryWriter wStream( nullptr, 0 );
		REQUIRE( wStream.AsWriter() == &wStream );
		REQUIRE( !wStream.AsReader() );
		REQUIRE( !wStream.IsValid() );
		REQUIRE( wStream.GetData() == nullptr );
		REQUIRE( wStream.GetOffset() == 0 );
		REQUIRE( wStream.GetRemainingBytes() == 0 );
		REQUIRE( wStream.GetSize() == 0 );
	}
	
	SECTION( "Writing with a valid data pointer and 0 length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryWriter wStream( buffer, 0 );
		REQUIRE( wStream.AsWriter() == &wStream );
		REQUIRE( !wStream.AsReader() );
		REQUIRE( !wStream.IsValid() );
		REQUIRE( wStream.GetData() == buffer );
		REQUIRE( wStream.GetOffset() == 0 );
		REQUIRE( wStream.GetRemainingBytes() == 0 ); // Stream is invalid
		REQUIRE( wStream.GetSize() == 0 );
	}
}

TEST_CASE( "BinaryWriter( Array< uint8_t >* array )", "[ae::BinaryStream]" )
{
	SECTION( "Create writer with a valid empty array" )
	{
		ae::Array< uint8_t > buffer = TAG_TEST;
		ae::BinaryWriter wStream( &buffer );
		REQUIRE( wStream.AsWriter() == &wStream );
		REQUIRE( !wStream.AsReader() );
		REQUIRE( wStream.IsValid() );
		REQUIRE( wStream.GetData() == buffer.Data() );
		REQUIRE( wStream.GetOffset() == 0 );
		REQUIRE( wStream.GetRemainingBytes() == 0 ); // Stream storage is dynamic
		REQUIRE( wStream.GetSize() == 0 ); // Stream storage is dynamic
		REQUIRE( wStream.GetSize() == buffer.Size() ); // Stream storage is dynamic
	}

	SECTION( "Create writer with a valid non-zero length array" )
	{
		ae::Array< uint8_t > buffer = TAG_TEST;
		buffer.Append( 1 );
		buffer.Append( 2 );
		buffer.Append( 3 );
		ae::BinaryWriter wStream( &buffer );
		REQUIRE( wStream.AsWriter() == &wStream );
		REQUIRE( !wStream.AsReader() );
		REQUIRE( wStream.IsValid() );
		REQUIRE( wStream.GetData() == buffer.Data() ); // Stream storage is dynamic
		REQUIRE( wStream.GetOffset() == buffer.Length() ); // Stream storage is dynamic
		REQUIRE( wStream.GetRemainingBytes() == buffer.Size() - buffer.Length() ); // Stream storage is dynamic
		REQUIRE( wStream.GetSize() == buffer.Size() ); // Stream storage is dynamic
	}

	SECTION( "Create writer with a null array" )
	{
		ae::BinaryWriter wStream( nullptr );
		REQUIRE( wStream.AsWriter() == &wStream );
		REQUIRE( !wStream.AsReader() );
		REQUIRE( !wStream.IsValid() );
		REQUIRE( wStream.GetData() == nullptr ); // Stream storage is dynamic
		REQUIRE( wStream.GetOffset() == 0 ); // Stream storage is dynamic
		REQUIRE( wStream.GetRemainingBytes() == 0 ); // Stream storage is dynamic
		REQUIRE( wStream.GetSize() == 0 ); // Stream storage is dynamic
	}
}

TEST_CASE( "BinaryReader( const void* data, uint32_t length )", "[ae::BinaryStream]" )
{
	SECTION( "Create reader with a valid data pointer and length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryReader rStream( buffer, sizeof(buffer) );
		REQUIRE( rStream.AsReader() == &rStream );
		REQUIRE( !rStream.AsWriter() );
		REQUIRE( rStream.IsValid() );
		REQUIRE( rStream.GetData() == buffer );
		REQUIRE( rStream.GetOffset() == 0 );
		REQUIRE( rStream.GetSize() == sizeof(buffer) );
		REQUIRE( rStream.PeekReadData() == buffer );
		REQUIRE( rStream.GetRemainingBytes() == sizeof(buffer) );
	}
	
	SECTION( "Create reader with a null data pointer with length 0" )
	{
		ae::BinaryReader rStream( nullptr, 0 );
		REQUIRE( rStream.AsReader() == &rStream );
		REQUIRE( !rStream.AsWriter() );
		REQUIRE( !rStream.IsValid() );
		REQUIRE( rStream.GetData() == nullptr );
		REQUIRE( rStream.GetOffset() == 0 );
		REQUIRE( rStream.GetSize() == 0 );
		REQUIRE( rStream.PeekReadData() == nullptr );
		REQUIRE( rStream.GetRemainingBytes() == 0 );
	}
	
	SECTION( "Reading with a valid data pointer and 0 length" )
	{
		uint8_t buffer[ 10 ];
		ae::BinaryReader rStream( buffer, 0 );
		REQUIRE( rStream.AsReader() == &rStream );
		REQUIRE( !rStream.AsWriter() );
		REQUIRE( !rStream.IsValid() );
		REQUIRE( rStream.GetData() == buffer );
		REQUIRE( rStream.GetOffset() == 0 );
		REQUIRE( rStream.GetSize() == 0 );
		REQUIRE( rStream.PeekReadData() == nullptr );
		REQUIRE( rStream.GetRemainingBytes() == 0 );
	}
}

TEST_CASE( "BinaryReader( const Array< uint8_t >& data )", "[ae::BinaryStream]" )
{
	SECTION( "Create reader with a valid empty array" )
	{
		ae::Array< uint8_t > buffer = TAG_TEST;
		ae::BinaryReader rStream( buffer );
		REQUIRE( rStream.AsReader() == &rStream );
		REQUIRE( !rStream.AsWriter() );
		REQUIRE( !rStream.IsValid() );
		REQUIRE( rStream.GetData() == buffer.Data() );
		REQUIRE( rStream.GetOffset() == 0 );
		REQUIRE( rStream.GetSize() == buffer.Size() );
		REQUIRE( rStream.PeekReadData() == nullptr );
		REQUIRE( rStream.GetRemainingBytes() == 0 );
	}

	SECTION( "Create reader with a valid non-zero length array" )
	{
		ae::Array< uint8_t > _buffer = TAG_TEST;
		_buffer.Append( 1 );
		_buffer.Append( 2 );
		_buffer.Append( 3 );
		const ae::Array< uint8_t >& buffer = _buffer;
		ae::BinaryReader rStream( buffer );
		REQUIRE( rStream.AsReader() == &rStream );
		REQUIRE( !rStream.AsWriter() );
		REQUIRE( rStream.IsValid() );
		REQUIRE( rStream.GetData() == buffer.Data() );
		REQUIRE( rStream.GetOffset() == 0 );
		REQUIRE( rStream.GetSize() == buffer.Length() );
		REQUIRE( rStream.PeekReadData() == buffer.Data() );
		REQUIRE( rStream.GetRemainingBytes() == buffer.Length() );
	}
}

TEST_CASE( "BinaryStream writer serialize basic values", "ae::BinaryStream" )
{
	TestStruct _test;
	_test.Initialize();

	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryWriter wStream( &buffer );
	const TestStruct& test = _test;
	test.DoSerialize( &wStream );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(TestStruct) );
	REQUIRE( buffer.Data() == wStream.GetData() );
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
	ae::BinaryReader rStream( buffer );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetData() == buffer.Data() );
	REQUIRE( rStream.GetOffset() == 0 );
	REQUIRE( rStream.GetSize() == buffer.Size() );
	REQUIRE( rStream.PeekReadData() == buffer.Data() );
	REQUIRE( rStream.GetRemainingBytes() == buffer.Length() );
	test1.DoSerialize( &rStream );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(TestStruct) );
	REQUIRE( rStream.PeekReadData() == buffer.Data() + sizeof(TestStruct) );
	REQUIRE( rStream.GetRemainingBytes() == 0 );
	REQUIRE( memcmp( &test0, &test1, sizeof(TestStruct) ) == 0 );
}

TEST_CASE( "Try to write past end of data buffer", "ae::BinaryStream" )
{
	uint8_t buffer[ 10 ] = { 0 };
	ae::BinaryWriter wStream( buffer, sizeof(buffer) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	wStream.SerializeUint32( 0x12345678u );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	wStream.SerializeUint32( 0x12345678u );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );
	wStream.SerializeUint32( 0x12345678u );
	REQUIRE( !wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

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
	ae::BinaryReader rStream( buffer, sizeof(buffer) );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == 0 );

	uint32_t check = 0xEFEFEFEF;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) );
	REQUIRE( check == 0x22222222 );

	check = 0xEFEFEFEF;
	rStream.SerializeUint32( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( check == 0x22222222 );

	check = 0xEFEFEFEF;
	rStream.SerializeUint32( check );
	REQUIRE( !rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof(uint32_t) * 2 );
	REQUIRE( check == 0xEFEFEFEF );
}

TEST_CASE( "C string test", "ae::BinaryStream" )
{
	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryWriter wStream( &buffer );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	wStream.SerializeString( "Hello, World!" );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == buffer.Length() );
	REQUIRE( wStream.GetOffset() == sizeof("Hello, World!") ); // Length of string + null terminator

	ae::BinaryReader rStream( buffer );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == 0 );
	REQUIRE( rStream.GetRemainingBytes() == buffer.Length() );
	char str[ sizeof("Hello, World!") ];
	memset( str, 0xAB, sizeof(str) );
	rStream.SerializeString( str, sizeof(str) );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == sizeof("Hello, World!") );
	REQUIRE( strcmp( str, "Hello, World!" ) == 0 );
}

TEST_CASE( "C string test with buffer length 0", "ae::BinaryStream" )
{
	ae::Array< uint8_t > buffer = TAG_TEST;
	char str[ sizeof("Hello, World!") ];
	strcpy( str, "Hello, World!" );
	ae::BinaryWriter wStream( &buffer );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE_THROWS( wStream.SerializeString( str, 0 ) );

	ae::BinaryReader rStream( buffer );
	REQUIRE_THROWS( rStream.SerializeString( str, 0 ) );
}

TEST_CASE( "C string target buffer too small", "ae::BinaryStream" )
{
	char str[ 5 ];
	ae::BinaryWriter wStream( str, sizeof(str) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	wStream.SerializeString( "Hello, World!" );
	REQUIRE( !wStream.IsValid() );
}

TEST_CASE( "C string write buffer too small", "ae::BinaryStream" )
{
	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryWriter wStream( &buffer );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	char str[ sizeof("Hello, World!") ];
	strcpy( str, "Hello, World!" );
	wStream.SerializeString( str, 5 );
	REQUIRE( !wStream.IsValid() );
}

TEST_CASE( "C string read buffer too small", "ae::BinaryStream" )
{
	char str[ sizeof("Hello, World!") ];
	ae::Array< uint8_t > buffer = TAG_TEST;
	ae::BinaryWriter wStream( &buffer );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	wStream.SerializeString( "Hello, World!" );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == buffer.Length() );
	REQUIRE( wStream.GetOffset() == sizeof("Hello, World!") );
	strcpy( str, "Hello, World!" );
	wStream.SerializeString( str, sizeof(str) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == buffer.Length() );
	REQUIRE( wStream.GetOffset() == sizeof("Hello, World!") * 2 );

	SECTION( "Success" )
	{
		ae::BinaryReader rStream( buffer );
		ae::Str64 check;
		rStream.SerializeString( check );
		REQUIRE( rStream.IsValid() );
		REQUIRE( rStream.GetOffset() == countof(str) );
		REQUIRE( check == "Hello, World!" );
		check = "";
		rStream.SerializeString( check );
		REQUIRE( rStream.IsValid() );
		REQUIRE( rStream.GetOffset() == countof(str) * 2 );
		REQUIRE( check == "Hello, World!" );
	}

	SECTION( "Out buffer too small" )
	{
		ae::BinaryReader rStream( buffer );
		REQUIRE( rStream.IsValid() );
		REQUIRE( rStream.GetOffset() == 0 );
		REQUIRE( rStream.GetRemainingBytes() == buffer.Length() );
		memset( str, 0xAB, sizeof(str) );
		rStream.SerializeString( str, 5 ); // Too small
		REQUIRE( !rStream.IsValid() );
		REQUIRE( str[ 0 ] == 0 );
	}
}

TEST_CASE( "BinaryStream ae::Str serialization", "ae::BinaryStream" )
{
	ae::Str64 str = "Hello, World!";
	uint8_t buffer[ 32 ];
	ae::BinaryWriter wStream( buffer, sizeof(buffer) );
	wStream.BinaryStream::SerializeString( str );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == ( str.Length() + 1 ) );
	wStream.SerializeString( *const_cast< const ae::Str64* >( &str ) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == ( str.Length() + 1 ) * 2 );

	ae::BinaryReader rStream( wStream.GetData(), wStream.GetOffset() );
	ae::Str64 check;
	rStream.SerializeString( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == ( str.Length() + 1 ) );
	REQUIRE( check == str );
	check = "";
	rStream.SerializeString( check );
	REQUIRE( rStream.IsValid() );
	REQUIRE( rStream.GetOffset() == ( str.Length() + 1 ) * 2 );
	REQUIRE( check == str );
}

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
	ae::BinaryWriter wStream( buffer, sizeof(buffer) );
	REQUIRE( wStream.AsWriter() == &wStream );
	REQUIRE( !wStream.AsReader() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetSize() == sizeof(buffer) );

	wStream.SerializeObject( m );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	wStream.BinaryStream::SerializeObject( s );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

	ae::BinaryReader rStream( wStream.GetData(), wStream.GetOffset() );
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
	ae::BinaryWriter wStream( buffer, sizeof(buffer) );
	REQUIRE( wStream.AsWriter() == &wStream );
	REQUIRE( !wStream.AsReader() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetSize() == sizeof(buffer) );

	wStream.SerializeObject( *const_cast< const MemberSerializeClass* >( &m ) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	wStream.SerializeObject( *const_cast< const StaticSerializeClass* >( &s ) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

	ae::BinaryReader rStream( wStream.GetData(), wStream.GetOffset() );
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
	ae::BinaryWriter wStream( buffer, sizeof(buffer) );
	REQUIRE( wStream.AsWriter() == &wStream );
	REQUIRE( !wStream.AsReader() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetSize() == sizeof(buffer) );

	wStream.SerializeObject( *const_cast< const ConstMemberSerializeClass* >( &c ) );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) );
	wStream.SerializeObject( c );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == sizeof(uint32_t) * 2 );

	ae::BinaryReader rStream( wStream.GetData(), wStream.GetOffset() );
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
	ae::BinaryWriter wStream( buffer, sizeof(buffer) );
	REQUIRE( wStream.AsWriter() == &wStream );
	REQUIRE( !wStream.AsReader() );
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

	ae::BinaryReader rStream( wStream.GetData(), wStream.GetOffset() );
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

//! Use SerializeObjectConditional() when an object may not be available for
//! serialization when writing or reading. This function correctly updates
//! read/write offsets when skipping serialization. Sends slightly more data
//! than SerializeObject().
template< typename T > void SerializeObjectConditional( T* objInOut );
