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
#include "catch2/catch.hpp"

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
// Serialization tests
//------------------------------------------------------------------------------
TEST_CASE( "Member serialize function should be successfully detected", "[BinaryStream]" )
{
	REQUIRE( ae::HasSerializeMethod< MemberSerializeClass >::value );
	REQUIRE( ae::HasConstSerializeMethod< MemberSerializeClass >::value );
	
	REQUIRE( !ae::HasSerializeMethod< StaticSerializeClass >::value );
	REQUIRE( !ae::HasConstSerializeMethod< StaticSerializeClass >::value );
	
	REQUIRE( !ae::HasSerializeMethod< ConstMemberSerializeClass >::value );
	REQUIRE( ae::HasConstSerializeMethod< ConstMemberSerializeClass >::value );
}

TEST_CASE( "Writer: Member and static function Serialize() should be called by SerializeObject()", "[BinaryStream]" )
{
	MemberSerializeClass m;
	StaticSerializeClass s;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetLength() == sizeof(buffer) );

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

	REQUIRE( rStream.GetOffset() == rStream.GetLength() );
}

TEST_CASE( "Writer: Const member and static function Serialize() should be called by SerializeObject()", "[BinaryStream]" )
{
	MemberSerializeClass m;
	StaticSerializeClass s;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetLength() == sizeof(buffer) );

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

	REQUIRE( rStream.GetOffset() == rStream.GetLength() );
}

TEST_CASE( "Writer: Support only having a const Serialize() that is called by SerializeObject()", "[BinaryStream]" )
{
	ConstMemberSerializeClass c;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetLength() == sizeof(buffer) );

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

	REQUIRE( rStream.GetOffset() == rStream.GetLength() );
}

TEST_CASE( "Reader: Member and static function Serialize() should be called by SerializeObject()", "[BinaryStream]" )
{
	MemberSerializeClass m;
	StaticSerializeClass s;

	uint8_t buffer[ 32 ];
	ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer );
	REQUIRE( wStream.IsWriter() );
	REQUIRE( wStream.IsValid() );
	REQUIRE( wStream.GetOffset() == 0 );
	REQUIRE( wStream.GetLength() == sizeof(buffer) );

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

	REQUIRE( rStream.GetOffset() == rStream.GetLength() );
}
