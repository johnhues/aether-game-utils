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
