//------------------------------------------------------------------------------
// AttributeTest.h
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

//------------------------------------------------------------------------------
// Attribute
//------------------------------------------------------------------------------
struct Attribute : public ae::Inheritor< ae::Attribute, Attribute >
{
	Attribute() = default;
	ae::Str128 fieldPath;
};

//------------------------------------------------------------------------------
// EmptyAttrib
//------------------------------------------------------------------------------
struct EmptyAttrib final : public ae::Inheritor< Attribute, EmptyAttrib >
{
};

//------------------------------------------------------------------------------
// CategoryInfoAttribute
//------------------------------------------------------------------------------
struct CategoryInfoAttribute final : public ae::Inheritor< ae::Attribute, CategoryInfoAttribute >
{
	int sortOrder;
	ae::Str128 name;
};

//------------------------------------------------------------------------------
// DisplayName
//------------------------------------------------------------------------------
struct DisplayName final : public ae::Inheritor< Attribute, DisplayName >
{
	ae::Str128 name;
};

//------------------------------------------------------------------------------
// RequiresAttrib
//------------------------------------------------------------------------------
struct RequiresAttrib final : public ae::Inheritor< Attribute, RequiresAttrib >
{
	RequiresAttrib( const char* name ) : name( name ) {}
	ae::Str128 name;
};

//------------------------------------------------------------------------------
// GameObject
//------------------------------------------------------------------------------
class GameObject : public ae::Inheritor< ae::Object, GameObject >
{
public:
	uint32_t id = 0;
};

//------------------------------------------------------------------------------
// xyz::Util
//------------------------------------------------------------------------------
namespace xyz
{
	class Util : public ae::Inheritor< ae::Object, Util >
	{
	public:
		uint32_t id = 0;
	};
}

//------------------------------------------------------------------------------
// xyz::EnumLabel
//------------------------------------------------------------------------------
namespace xyz
{
	struct EnumLabel final : public ae::Inheritor< ae::Attribute, EnumLabel >
	{
		ae::Str128 name;
	};
}

//------------------------------------------------------------------------------
// AttribEnumClass - AE_REGISTER_ENUM_CLASS
//------------------------------------------------------------------------------
AE_DEFINE_ENUM_CLASS( AttribEnumClass, uint8_t,
	Idle,
	Walk
);

//------------------------------------------------------------------------------
// AttribCStyleEnum - AE_REGISTER_ENUM
//------------------------------------------------------------------------------
enum AttribCStyleEnum : uint8_t
{
	AttribCStyleIdle,
	AttribCStyleWalk
};

//------------------------------------------------------------------------------
// AttribPrefixEnum - AE_REGISTER_ENUM_PREFIX
//------------------------------------------------------------------------------
enum AttribPrefixEnum : uint8_t
{
	kAttribPrefixEnum_Idle,
	kAttribPrefixEnum_Walk
};

//------------------------------------------------------------------------------
// AttribBitFieldEnum - AE_REGISTER_BIT_FIELD_ENUM
//------------------------------------------------------------------------------
enum AttribBitFieldEnum : uint32_t
{
	AttribBitFieldNone  = 0,
	AttribBitFieldRead  = 1 << 0,
	AttribBitFieldWrite = 1 << 1
};

//------------------------------------------------------------------------------
// AttribBitFieldPrefixEnum - AE_REGISTER_BIT_FIELD_ENUM_PREFIX
//------------------------------------------------------------------------------
enum AttribBitFieldPrefixEnum : uint32_t
{
	kABFPE_None  = 0,
	kABFPE_Read  = 1 << 0,
	kABFPE_Write = 1 << 1
};

//------------------------------------------------------------------------------
// AttribEnumClass2 - AE_REGISTER_ENUM_CLASS2
//------------------------------------------------------------------------------
enum class AttribEnumClass2 : uint8_t
{
	Idle,
	Walk
};

//------------------------------------------------------------------------------
// AttribBitFieldEnumClass2 - AE_REGISTER_BIT_FIELD_ENUM_CLASS2
//------------------------------------------------------------------------------
enum class AttribBitFieldEnumClass2 : uint32_t
{
	None  = 0,
	Read  = 1 << 0,
	Write = 1 << 1
};

//------------------------------------------------------------------------------
// xyz::AttribNamespacedEnum - AE_REGISTER_ENUM_CLASS2 with a qualified name
//------------------------------------------------------------------------------
namespace xyz
{
	enum class AttribNamespacedEnum : uint8_t
	{
		Idle,
		Walk
	};
}

//------------------------------------------------------------------------------
// AttribUnattributedEnum - registered without attributes
//------------------------------------------------------------------------------
enum class AttribUnattributedEnum : uint8_t
{
	Idle,
	Walk
};
