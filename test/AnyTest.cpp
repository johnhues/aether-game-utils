//------------------------------------------------------------------------------
// AnyTest.cpp
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

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
struct AnyTestPod
{
	int32_t x;
	int32_t y;
};

struct TestAnyBase { AE_BASE_TYPE; };
AE_REGISTER_CLASS( TestAnyBase );
struct TestAnyDerived : public ae::Inheritor< TestAnyBase, TestAnyDerived > {};
AE_REGISTER_CLASS( TestAnyDerived );
struct TestAnyUnrelated : public ae::Inheritor< ae::Object, TestAnyUnrelated > {};
AE_REGISTER_CLASS( TestAnyUnrelated );

using AnySmall = ae::Any< sizeof( int32_t ), alignof( int32_t ) >;
using AnyPtr = ae::Any< sizeof( void* ), alignof( void* ) >;

//------------------------------------------------------------------------------
// ae::Any basic value tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any default construction is empty", "[ae::Any]" )
{
	const AnySmall a;
	REQUIRE( a.GetTypeId() == ae::TypeId() );
	REQUIRE( a.TryGet< int32_t >() == nullptr );
}

TEST_CASE( "ae::Any construct from value", "[ae::Any]" )
{
	const AnySmall a( (int32_t)42 );
	REQUIRE( a.GetTypeId() == ae::GetTypeIdWithQualifiers< int32_t >() );
	REQUIRE( a.TryGet< int32_t >() != nullptr );
	REQUIRE( *a.TryGet< int32_t >() == 42 );
}

TEST_CASE( "ae::Any operator bool", "[ae::Any]" )
{
	AnySmall a;
	REQUIRE_FALSE( a );
	a = (int32_t)0; // zero value, but type is set
	REQUIRE( a );
}

TEST_CASE( "ae::Any assign value", "[ae::Any]" )
{
	AnySmall a;
	a = (int32_t)7;
	REQUIRE( *a.TryGet< int32_t >() == 7 );
}

TEST_CASE( "ae::Any Get returns value on match", "[ae::Any]" )
{
	AnySmall a( (int32_t)99 );
	REQUIRE( a.Get< int32_t >() == 99 );
}

TEST_CASE( "ae::Any Get returns default on type mismatch", "[ae::Any]" )
{
	AnySmall a( (int32_t)99 );
	REQUIRE( a.Get< float >( 3.14f ) == 3.14f );
}

TEST_CASE( "ae::Any TryGet returns nullptr on type mismatch", "[ae::Any]" )
{
	const AnySmall a( (int32_t)5 );
	REQUIRE( a.TryGet< float >() == nullptr );
}

TEST_CASE( "ae::Any reassignment changes stored type", "[ae::Any]" )
{
	AnySmall a( (int32_t)1 );
	REQUIRE( a.TryGet< int32_t >() != nullptr );
	a = 1.5f;
	REQUIRE( a.TryGet< int32_t >() == nullptr );
	REQUIRE( a.TryGet< float >() != nullptr );
	REQUIRE( *a.TryGet< float >() == 1.5f );
}

TEST_CASE( "ae::Any non-const TryGet returns mutable pointer", "[ae::Any]" )
{
	AnySmall a( (int32_t)10 );
	int32_t* ptr = a.TryGet< int32_t >();
	REQUIRE( ptr != nullptr );
	*ptr = 20;
	REQUIRE( a.Get< int32_t >() == 20 );
}

//------------------------------------------------------------------------------
// ae::Any pointer tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any store and retrieve pointer", "[ae::Any]" )
{
	AnyTestPod pod{ 1, 2 };
	AnyPtr a( &pod );
	REQUIRE( a.TryGet< AnyTestPod* >() != nullptr );
	REQUIRE( *a.TryGet< AnyTestPod* >() == &pod );
}

TEST_CASE( "ae::Any retrieve as const pointer when non-const pointer stored", "[ae::Any]" )
{
	AnyTestPod pod{ 3, 4 };
	AnyPtr a( &pod );
	REQUIRE( a.TryGet< const AnyTestPod* >() != nullptr );
	REQUIRE( *a.TryGet< const AnyTestPod* >() == &pod );
}

TEST_CASE( "ae::Any cannot retrieve as non-const pointer when const pointer stored", "[ae::Any]" )
{
	const AnyTestPod pod{ 5, 6 };
	AnyPtr a( &pod );
	REQUIRE( a.TryGet< const AnyTestPod* >() != nullptr );
	REQUIRE( a.TryGet< AnyTestPod* >() == nullptr );
}

TEST_CASE( "ae::Any store and retrieve null pointer", "[ae::Any]" )
{
	AnyTestPod* null = nullptr;
	AnyPtr a( null );
	REQUIRE( a.GetTypeId() == ae::GetTypeIdWithQualifiers< AnyTestPod* >() );
	REQUIRE( a.TryGet< AnyTestPod* >() != nullptr );  // pointer to the stored null
	REQUIRE( *a.TryGet< AnyTestPod* >() == nullptr ); // stored value is null
}

TEST_CASE( "ae::Any pointer type mismatch returns nullptr", "[ae::Any]" )
{
	AnyTestPod pod{ 7, 8 };
	int32_t other = 0;
	AnyPtr a( &pod );
	REQUIRE( a.TryGet< int32_t* >() == nullptr );
}

//------------------------------------------------------------------------------
// ae::BinaryStream user data tests (uses ae::Any internally)
//------------------------------------------------------------------------------
TEST_CASE( "ae::BinaryStream GetUserData returns nullptr by default", "[ae::BinaryStream][UserData]" )
{
	uint8_t buffer[ 64 ];
	ae::BinaryWriter stream( buffer, sizeof( buffer ) );
	REQUIRE( stream.GetUserData< AnyTestPod >() == nullptr );
}

TEST_CASE( "ae::BinaryStream SetUserData / GetUserData same type", "[ae::BinaryStream][UserData]" )
{
	AnyTestPod pod{ 10, 20 };
	uint8_t buffer[ 64 ];
	ae::BinaryWriter stream( buffer, sizeof( buffer ) );
	stream.SetUserData( &pod );
	REQUIRE( stream.GetUserData< AnyTestPod >() == &pod );
}

TEST_CASE( "ae::BinaryStream GetUserData as const when non-const stored", "[ae::BinaryStream][UserData]" )
{
	AnyTestPod pod{ 30, 40 };
	uint8_t buffer[ 64 ];
	ae::BinaryWriter stream( buffer, sizeof( buffer ) );
	stream.SetUserData( &pod );
	const AnyTestPod* result = stream.GetUserData< const AnyTestPod >();
	REQUIRE( result == &pod );
}

TEST_CASE( "ae::BinaryStream SetUserData const pointer allows const retrieval", "[ae::BinaryStream][UserData]" )
{
	const AnyTestPod pod{ 50, 60 };
	uint8_t buffer[ 64 ];
	ae::BinaryWriter stream( buffer, sizeof( buffer ) );
	stream.SetUserData( &pod );
	REQUIRE( stream.GetUserData< const AnyTestPod >() == &pod );
}

TEST_CASE( "ae::BinaryStream cannot retrieve non-const when const stored", "[ae::BinaryStream][UserData]" )
{
	const AnyTestPod pod{ 70, 80 };
	uint8_t buffer[ 64 ];
	ae::BinaryWriter stream( buffer, sizeof( buffer ) );
	stream.SetUserData( &pod );
	REQUIRE( stream.GetUserData< AnyTestPod >() == nullptr );
}

TEST_CASE( "ae::BinaryStream GetUserData wrong type returns nullptr", "[ae::BinaryStream][UserData]" )
{
	AnyTestPod pod{ 1, 2 };
	int32_t other = 0;
	uint8_t buffer[ 64 ];
	ae::BinaryWriter stream( buffer, sizeof( buffer ) );
	stream.SetUserData( &pod );
	REQUIRE( stream.GetUserData< int32_t >() == nullptr );
}

TEST_CASE( "ae::BinaryStream SetUserData overwrites previous", "[ae::BinaryStream][UserData]" )
{
	AnyTestPod pod{ 1, 2 };
	int32_t value = 99;
	uint8_t buffer[ 64 ];
	ae::BinaryWriter stream( buffer, sizeof( buffer ) );
	stream.SetUserData( &pod );
	stream.SetUserData( &value );
	REQUIRE( stream.GetUserData< AnyTestPod >() == nullptr );
	REQUIRE( stream.GetUserData< int32_t >() == &value );
}

//------------------------------------------------------------------------------
// ae::Any const value type tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any TryGet<const T> succeeds when T is stored", "[ae::Any]" )
{
	AnySmall a( (int32_t)7 );
	REQUIRE( a.TryGet< const int32_t >() != nullptr );
	REQUIRE( *a.TryGet< const int32_t >() == 7 );
}

TEST_CASE( "ae::Any TryGet<T> succeeds when T is stored (const value source)", "[ae::Any]" )
{
	const int32_t v = 7;
	AnySmall a( v );
	REQUIRE( a.TryGet< int32_t >() != nullptr );
	REQUIRE( *a.TryGet< int32_t >() == 7 );
}

TEST_CASE( "ae::Any TryGet<const T> does not widen across types", "[ae::Any]" )
{
	AnySmall a( (int32_t)7 );
	REQUIRE( a.TryGet< const float >() == nullptr );
}

//------------------------------------------------------------------------------
// ae::Any ClassType registered pointer tests (non-const stored)
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any store Derived* retrieve as Derived*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	AnyPtr a( &obj );
	REQUIRE( a.TryGet< TestAnyDerived* >() != nullptr );
	REQUIRE( *a.TryGet< TestAnyDerived* >() == &obj );
}

TEST_CASE( "ae::Any store Derived* retrieve as Base*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	AnyPtr a( &obj );
	REQUIRE( a.TryGet< TestAnyBase* >() != nullptr );
	REQUIRE( *a.TryGet< TestAnyBase* >() == &obj );
}

TEST_CASE( "ae::Any store Derived* cannot retrieve as unrelated type", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	AnyPtr a( &obj );
	REQUIRE( a.TryGet< TestAnyUnrelated* >() == nullptr );
}

TEST_CASE( "ae::Any store Derived* retrieve as const Derived*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	AnyPtr a( &obj );
	REQUIRE( a.TryGet< const TestAnyDerived* >() != nullptr );
	REQUIRE( *a.TryGet< const TestAnyDerived* >() == &obj );
}

TEST_CASE( "ae::Any store Derived* retrieve as const Base*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	AnyPtr a( &obj );
	REQUIRE( a.TryGet< const TestAnyBase* >() != nullptr );
	REQUIRE( *a.TryGet< const TestAnyBase* >() == &obj );
}

//------------------------------------------------------------------------------
// ae::Any ClassType registered pointer tests (const stored)
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any store const Derived* retrieve as const Derived*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	const TestAnyDerived* ptr = &obj;
	AnyPtr a( ptr );
	REQUIRE( a.TryGet< const TestAnyDerived* >() != nullptr );
	REQUIRE( *a.TryGet< const TestAnyDerived* >() == ptr );
}

TEST_CASE( "ae::Any store const Derived* retrieve as const Base*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	const TestAnyDerived* ptr = &obj;
	AnyPtr a( ptr );
	REQUIRE( a.TryGet< const TestAnyBase* >() != nullptr );
	REQUIRE( *a.TryGet< const TestAnyBase* >() == ptr );
}

TEST_CASE( "ae::Any store const Derived* cannot retrieve as non-const Derived*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	const TestAnyDerived* ptr = &obj;
	AnyPtr a( ptr );
	REQUIRE( a.TryGet< TestAnyDerived* >() == nullptr );
}

TEST_CASE( "ae::Any store const Derived* cannot retrieve as non-const Base*", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	const TestAnyDerived* ptr = &obj;
	AnyPtr a( ptr );
	REQUIRE( a.TryGet< TestAnyBase* >() == nullptr );
}

//------------------------------------------------------------------------------
// ae::Any GetTypeId returns ClassType TypeId for registered pointers
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any GetTypeId returns ClassType TypeId for registered pointer", "[ae::Any][ClassType]" )
{
	TestAnyDerived obj;
	AnyPtr a( &obj );
	REQUIRE( a.GetTypeId() == ae::GetClassType< TestAnyDerived >()->GetTypeId() );
}

//------------------------------------------------------------------------------
// ae::Any unregistered pointer const-widening regression
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any store int32_t* retrieve as const int32_t*", "[ae::Any]" )
{
	int32_t v = 5;
	AnyPtr a( &v );
	REQUIRE( a.TryGet< const int32_t* >() != nullptr );
	REQUIRE( *a.TryGet< const int32_t* >() == &v );
}

TEST_CASE( "ae::Any store const int32_t* cannot retrieve as int32_t*", "[ae::Any]" )
{
	const int32_t v = 5;
	AnyPtr a( &v );
	REQUIRE( a.TryGet< int32_t* >() == nullptr );
}

//------------------------------------------------------------------------------
// ae::Any registered type by value (TypeId collision regression)
//------------------------------------------------------------------------------
TEST_CASE( "ae::Any store TestAnyBase by value: value retrieval succeeds, pointer retrieval fails", "[ae::Any][ClassType]" )
{
	TestAnyBase b;
	AnySmall a( b );
	REQUIRE( a.TryGet< TestAnyBase >() != nullptr );
	REQUIRE( a.TryGet< TestAnyBase* >() == nullptr );
	REQUIRE( a.TryGet< TestAnyDerived* >() == nullptr );
}

TEST_CASE( "ae::Any store TestAnyDerived by value: value retrieval succeeds, pointer and upcast retrieval fail", "[ae::Any][ClassType]" )
{
	TestAnyDerived d;
	AnySmall a( d );
	REQUIRE( a.TryGet< TestAnyDerived >() != nullptr );
	REQUIRE( a.TryGet< TestAnyDerived* >() == nullptr );
	REQUIRE( a.TryGet< TestAnyBase* >() == nullptr );
	// Value storage must NOT upcast (not a pointer)
	REQUIRE( a.TryGet< TestAnyBase >() == nullptr );
}
