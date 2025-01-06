//------------------------------------------------------------------------------
// MetaTest.h
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
#ifndef METATEST_H
#define METATEST_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag AE_ALLOC_TAG_META_TEST = "MetaTest";

//------------------------------------------------------------------------------
// TestEnumClass
//------------------------------------------------------------------------------
AE_DEFINE_ENUM_CLASS( TestEnumClass, int32_t,
	NegativeOne = -1,
	Zero,
	One,
	Two = 0x02,
	Three = 0x03,
	Four,
	Five
);

//------------------------------------------------------------------------------
// PlayerState
//------------------------------------------------------------------------------
AE_DEFINE_ENUM_CLASS( PlayerState, uint16_t,
	Idle,
	Run,
	Jump
);

//------------------------------------------------------------------------------
// SomeClass
//------------------------------------------------------------------------------
class SomeClass : public ae::Inheritor< ae::Object, SomeClass >
{
public:
	int32_t intMember;
	bool boolMember;
	TestEnumClass enumTest;
};

//------------------------------------------------------------------------------
// NamespaceClass
//------------------------------------------------------------------------------
namespace Namespace0 {
	namespace Namespace1 {
		class NamespaceClass : public ae::Inheritor< SomeClass, NamespaceClass >
		{
		public:
			bool boolMember;
		};
	}
}

//------------------------------------------------------------------------------
// AggregateClass
//------------------------------------------------------------------------------
class AggregateClass : public ae::Inheritor< ae::Object, AggregateClass >
{
public:
	SomeClass someClass;
	SomeClass someClass1;
};

//------------------------------------------------------------------------------
// ArrayClass
//------------------------------------------------------------------------------
class ArrayClass : public ae::Inheritor< ae::Object, ArrayClass >
{
public:
	int32_t intArray[ 3 ];
	ae::Array< int32_t, 4 > intArray2;
	ae::Array< int32_t > intArray3 = AE_ALLOC_TAG_META_TEST;

	SomeClass someClassArray[ 3 ];
	ae::Array< SomeClass, 4 > someClassArray2;
	ae::Array< SomeClass > someClassArray3 = AE_ALLOC_TAG_META_TEST;
};

template < typename T >
class ae::VarType< ae::Optional< T > > : public ae::VarTypeOptional
{
public:
	uint32_t GetSize() const override { return sizeof(T); }
	ae::BasicType GetType() const override { return ae::VarType< T >::Get()->GetType(); }
	const char* GetName() const override { return ae::VarType< T >::Get()->GetName(); }
	const char* GetPrefix() const override { return ae::VarType< T >::Get()->GetPrefix(); }
	bool SetRef( void* varData, const char* value, const ae::Var* var ) const override { AE_FAIL(); return false; }
	bool SetRef( void* varData, ae::Object* value ) const override{ AE_FAIL(); return false; }
	std::string GetStringFromRef( const void* ref ) const override { AE_FAIL(); return ""; }
	const char* GetSubTypeName() const override { return ae::VarType< T >::Get()->GetSubTypeName(); }
	static ae::VarTypeBase* Get() { static ae::VarType< ae::Optional< T > > s_type; return &s_type; }

	void* TryGetValue( void* opt ) const override { return static_cast< ae::Optional< T >* >( opt )->TryGet(); }
	const void* TryGetValue( const void* opt ) const override { return static_cast< const ae::Optional< T >* >( opt )->TryGet(); }
};

//------------------------------------------------------------------------------
// OptionalClass
//------------------------------------------------------------------------------
class OptionalClass : public ae::Inheritor< ae::Object, OptionalClass >
{
public:
	ae::Optional< int32_t > intOptional;
	ae::Optional < SomeClass > someClassOptional;
};

//------------------------------------------------------------------------------
// SomeOldEnum
//------------------------------------------------------------------------------
enum SomeOldEnum
{
	Bleep = 4,
	Bloop,
	Blop = 7
};

//------------------------------------------------------------------------------
// SomeOldPrefixEnum
//------------------------------------------------------------------------------
enum SomeOldPrefixEnum
{
	kSomeOldPrefixEnum_Bleep = 4,
	kSomeOldPrefixEnum_Bloop,
	kSomeOldPrefixEnum_Blop = 7
};

//------------------------------------------------------------------------------
// SomeOldRenamedEnum
//------------------------------------------------------------------------------
enum SomeOldRenamedEnum
{
	BLEEP = 4,
	BLOOP,
	BLOP = 7
};

//------------------------------------------------------------------------------
// SomeNewEnum
//------------------------------------------------------------------------------
enum class SomeNewEnum
{
	Bleep = 4,
	Bloop,
	Blop = 7
};

//------------------------------------------------------------------------------
// A::B::SomeNewEnum
//------------------------------------------------------------------------------
namespace A
{
	namespace B
	{
		enum class SomeNewEnum
		{
			Bleep = 4,
			Bloop,
			Blop = 7
		};
	}
}

//------------------------------------------------------------------------------
// PlayerState
//------------------------------------------------------------------------------
// #define AE_DEFINE_ENUM_BITFIELD( E )\
// 	typedef std::underlying_type< E >::type _ae_##E;\
// 	inline E operator | ( E a, E b ) { return (E)( (_ae_##E)a | (_ae_##E)b ); }\
// 	inline E operator & ( E a, E b ) { return (E)( (_ae_##E)a & (_ae_##E)b ); }\
// 	inline E operator ^ ( E a, E b ) { return (E)( (_ae_##E)a ^ (_ae_##E)b ); }\
// 	inline E operator ~ ( E e ) { return (E)( ~(_ae_##E)e ); }

#define AE_DEFINE_ENUM_BITFIELD( E )\
	typedef std::underlying_type< E >::type _ae_##E;\
	inline _ae_##E operator | ( E a, E b ) { return ( (_ae_##E)a | (_ae_##E)b ); }\
	inline _ae_##E operator | ( _ae_##E a, E b ) { return ( a | (_ae_##E)b ); }\
	inline _ae_##E operator | ( E a, _ae_##E b ) { return ( (_ae_##E)a | b ); }\
	inline _ae_##E operator & ( E a, E b ) { return ( (_ae_##E)a & (_ae_##E)b ); }\
	inline _ae_##E operator ^ ( E a, E b ) { return ( (_ae_##E)a ^ (_ae_##E)b ); }\
	inline _ae_##E operator ~ ( E e ) { return ~(_ae_##E)e; }

AE_DEFINE_ENUM_CLASS( GamePadBitField, uint16_t,
	None,
	A,
	B,
	X,
	Y,
	L,
	R,
	Up,
	Down,
	Left,
	Right
);
AE_DEFINE_ENUM_BITFIELD( GamePadBitField );

//------------------------------------------------------------------------------
// Reference testing
//------------------------------------------------------------------------------
// RefTester
class RefTester : public ae::Inheritor< ae::Object, RefTester >
{
public:
	
	static std::string GetIdString( const RefTester* obj );
	static bool StringToId( const char* str, uint32_t* idOut );
	
	uint32_t id = 0;
};

// RefTesterA
class RefTesterA : public ae::Inheritor< RefTester, RefTesterA >
{
public:
	int notRef = 0xfdfdfdfd;
	class RefTesterA* refA = nullptr;
	class RefTesterB* refB = nullptr;
};

// RefTesterB
class RefTesterB : public ae::Inheritor< RefTester, RefTesterB >
{
public:
	class RefTesterA* refA = nullptr;
};

// RefTesterManager
class RefTesterManager
{
public:
	template < typename T >
	T* Create() { T* o = (T*)m_objectMap.Set( m_nextId, ae::New< T >( AE_ALLOC_TAG_META_TEST ) ); o->id = m_nextId; m_nextId++; return o; }
	void Destroy( RefTester* object );
	
	RefTester* GetObjectById( uint32_t id );

private:
	uint32_t m_nextId = 1;
	ae::Map< uint32_t, RefTester* > m_objectMap = AE_ALLOC_TAG_META_TEST;
};

#endif
