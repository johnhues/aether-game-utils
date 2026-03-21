//------------------------------------------------------------------------------
// MetaTest.h
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

struct CustomBaseType
{
	AE_BASE_TYPE();
};

struct CustomBaseTypeTest : public ae::Inheritor< CustomBaseType, CustomBaseTypeTest >
{
	int testInt = 0;
};

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
// Enum width coverage
//------------------------------------------------------------------------------
enum class EnumInt8 : int8_t
{
	Min = INT8_MIN,
	NegativeOne = -1,
	Zero = 0,
	Max = INT8_MAX
};

enum class EnumUInt8 : uint8_t
{
	Zero = 0,
	One = 1,
	HighBit = 128,
	Max = UINT8_MAX
};

enum class EnumInt16 : int16_t
{
	Min = INT16_MIN,
	NegativeOne = -1,
	Zero = 0,
	Max = INT16_MAX
};

enum class EnumUInt16 : uint16_t
{
	Zero = 0,
	One = 1,
	HighBit = 32768,
	Max = UINT16_MAX
};

enum class EnumInt32 : int32_t
{
	Min = INT32_MIN,
	NegativeOne = -1,
	Zero = 0,
	Max = INT32_MAX
};

enum class EnumUInt32 : uint32_t
{
	Zero = 0,
	One = 1,
	HighBit = 2147483648u,
	Max = UINT32_MAX
};

enum class EnumInt64 : int64_t
{
	Min = INT64_MIN,
	NegativeOne = -1,
	Zero = 0,
	Max = INT64_MAX
};

enum class EnumUInt64 : uint64_t
{
	Zero = 0,
	One = 1,
	HighBit = 9223372036854775808ull,
	Max = UINT64_MAX
};

//------------------------------------------------------------------------------
// SomeClass
//------------------------------------------------------------------------------
class SomeClass : public ae::Inheritor< ae::Object, SomeClass >
{
public:
	int32_t intMember;
	bool boolMember;
	TestEnumClass enumTest;
	ae::UUID uuidMember;
};


//------------------------------------------------------------------------------
// NamespaceClass
//------------------------------------------------------------------------------
namespace Namespace0 {
	namespace Namespace1 {
		class NamespaceClass : public ae::Inheritor< SomeClass, NamespaceClass >
		{
		public:
			int32_t intMember;
			bool boolMember;
			TestEnumClass enumTest;
			ae::UUID uuidMember;
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

template< typename T >
class ae::TypeT< ae::Optional< T > > : public ae::OptionalType
{
public:
	const ae::Type& GetInnerVarType() const override { return *ae::TypeT< T >::Get(); }
	static ae::Type* Get() { static ae::TypeT< ae::Optional< T > > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< ae::Optional< T > >(); }

	ae::DataPointer TryGet( ae::DataPointer optional ) const override { return ae::DataPointer(); }
	ae::ConstDataPointer TryGet( ae::ConstDataPointer optional ) const override { return ae::ConstDataPointer(); }
	ae::DataPointer GetOrInsert( ae::DataPointer optional ) const override { return ae::DataPointer(); }
	void Clear( ae::DataPointer optional ) const override {}
};

template< typename T >
class ae::TypeT< std::optional< T > > : public ae::OptionalType
{
public:
	const ae::Type& GetInnerVarType() const override { return *ae::TypeT< T >::Get(); }
	static ae::Type* Get() { static ae::TypeT< std::optional< T > > s_type; return &s_type; }
	ae::TypeId GetExactVarTypeId() const override { return ae::GetTypeIdWithQualifiers< std::optional< T > >(); }

	ae::DataPointer TryGet( ae::DataPointer _opt ) const override
	{
		std::optional< T >* opt = static_cast< std::optional< T >* >( _opt.Get( this ) );
		return { GetInnerVarType(), opt->has_value() ? &opt->value() : nullptr };
	}
	ae::ConstDataPointer TryGet( ae::ConstDataPointer _opt ) const override
	{
		const std::optional< T >* opt = static_cast< const std::optional< T >* >( _opt.Get( this ) );
		return { GetInnerVarType(), opt->has_value() ? &opt->value() : nullptr };
	}
	ae::DataPointer GetOrInsert( ae::DataPointer _optional ) const override
	{
		if( std::optional< T >* optional = static_cast< std::optional< T >* >( _optional.Get( this ) ) )
		{
			optional->emplace();
			return { GetInnerVarType(), &optional->value() };
		}
		return {};
	}
	void Clear( ae::DataPointer _optional ) const override
	{
		if( std::optional< T >* optional = static_cast< std::optional< T >* >( _optional.Get( this ) ) )
		{
			optional->reset();
		}
	}
};

//------------------------------------------------------------------------------
// OptionalClass
//------------------------------------------------------------------------------
class OptionalClass : public ae::Inheritor< ae::Object, OptionalClass >
{
public:
	ae::Optional< int32_t > intOptional;
	ae::Optional < SomeClass > someClassOptional;

	std::optional< int32_t > intStdOptional;
	std::optional< SomeClass > someClassStdOptional;
};

//------------------------------------------------------------------------------
// MapClass
//------------------------------------------------------------------------------
class MapClass : public ae::Inheritor< ae::Object, MapClass >
{
public:
	ae::Map< ae::Str32, int32_t > intMap = AE_ALLOC_TAG_META_TEST;
	ae::Map< ae::Str32, int32_t, 4 > intMapFixed;
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

AE_DEFINE_BIT_FIELD_ENUM_CLASS( GamePadBitField, uint16_t,
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

//------------------------------------------------------------------------------
// SceneFlags - proper power-of-2 bit field enum for combination tests
//------------------------------------------------------------------------------
AE_DEFINE_BIT_FIELD_ENUM_CLASS( SceneFlags, uint32_t,
	None = 0,
	Player = 1 << 0,
	Camera = 1 << 1,
	Mesh = 1 << 2,
	All = Player | Camera | Mesh
);

//------------------------------------------------------------------------------
// OldBitFieldFlags - c-style bit field enum (AE_REGISTER_BIT_FIELD_ENUM)
//------------------------------------------------------------------------------
enum OldBitFieldFlags : uint32_t
{
	OldBitFieldFlags_None    = 0,
	OldBitFieldFlags_Read    = 1 << 0,
	OldBitFieldFlags_Write   = 1 << 1,
	OldBitFieldFlags_Execute = 1 << 2
};

//------------------------------------------------------------------------------
// OldBitFieldPrefixFlags - c-style bit field enum with prefix (AE_REGISTER_BIT_FIELD_ENUM_PREFIX)
//------------------------------------------------------------------------------
enum OldBitFieldPrefixFlags : uint32_t
{
	kOBPF_None    = 0,
	kOBPF_Read    = 1 << 0,
	kOBPF_Write   = 1 << 1,
	kOBPF_Execute = 1 << 2
};

//------------------------------------------------------------------------------
// NewBitFieldFlags - pre-existing enum class (AE_REGISTER_BIT_FIELD_ENUM_CLASS2)
//------------------------------------------------------------------------------
enum class NewBitFieldFlags : uint32_t
{
	None    = 0,
	Read    = 1 << 0,
	Write   = 1 << 1,
	Execute = 1 << 2
};

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
	class RefTester* ref = nullptr;
};

// RefTesterManager
class RefTesterManager
{
public:
	template< typename T >
	T* Create() { T* o = (T*)m_objectMap.Set( m_nextId, ae::New< T >( AE_ALLOC_TAG_META_TEST ) ); o->id = m_nextId; m_nextId++; return o; }
	void Destroy( RefTester* object );
	
	RefTester* GetObjectById( uint32_t id );

private:
	uint32_t m_nextId = 1;
	ae::Map< uint32_t, RefTester* > m_objectMap = AE_ALLOC_TAG_META_TEST;
};

#endif
