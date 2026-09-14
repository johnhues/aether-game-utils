//------------------------------------------------------------------------------
// AttributeTest.cpp
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
#include "AttributesTest.h"

//------------------------------------------------------------------------------
// Attribute
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( Attribute );

//------------------------------------------------------------------------------
// EmptyAttrib
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( EmptyAttrib );

//------------------------------------------------------------------------------
// CategoryInfoAttribute
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( CategoryInfoAttribute );
AE_REGISTER_CLASS_VAR( CategoryInfoAttribute, sortOrder );
AE_REGISTER_CLASS_VAR( CategoryInfoAttribute, name );

//------------------------------------------------------------------------------
// DisplayName
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( DisplayName );

//------------------------------------------------------------------------------
// RequiresAttrib
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( RequiresAttrib );

//------------------------------------------------------------------------------
// GameObject
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( GameObject );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, EmptyAttrib, {} );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "Something" ) );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "SomethingElse" ) );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "SomethingSomethingElse" ) );
AE_REGISTER_CLASS_ATTRIBUTE( GameObject, RequiresAttrib, ( "SomethingSomethingElseElse" ) );
AE_REGISTER_CLASS_VAR( GameObject, id );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, CategoryInfoAttribute, ({ .sortOrder = 1, .name = "General" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, DisplayName, ({ .name = "ID" }) );
AE_REGISTER_CLASS_VAR_ATTRIBUTE( GameObject, id, DisplayName, ({ .name = "ID2" }) );

//------------------------------------------------------------------------------
// xyz::Util (namespace class)
//------------------------------------------------------------------------------
AE_REGISTER_NAMESPACECLASS( (xyz, Util) );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (xyz, Util), (EmptyAttrib), {} );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (xyz, Util), (RequiresAttrib), ( "Something" ) );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (xyz, Util), (RequiresAttrib), ( "SomethingElse" ) );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (xyz, Util), (RequiresAttrib), ( "SomethingSomethingElse" ) );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (xyz, Util), (RequiresAttrib), ( "SomethingSomethingElseElse" ) );
AE_REGISTER_NAMESPACECLASS_VAR( (xyz, Util), id );
AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( (xyz, Util), id, (CategoryInfoAttribute), ({ .sortOrder = 1, .name = "General" }) );
AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( (xyz, Util), id, (DisplayName), ({ .name = "ID" }) );
AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( (xyz, Util), id, (DisplayName), ({ .name = "ID2" }) );

//------------------------------------------------------------------------------
// xyz::EnumLabel
//------------------------------------------------------------------------------
AE_REGISTER_NAMESPACECLASS( (xyz, EnumLabel) );

//------------------------------------------------------------------------------
// AttribEnumClass
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS( AttribEnumClass );
AE_REGISTER_ENUM_ATTRIBUTE( AttribEnumClass, EmptyAttrib, {} );
AE_REGISTER_ENUM_ATTRIBUTE( AttribEnumClass, RequiresAttrib, ( "Something" ) );
AE_REGISTER_ENUM_ATTRIBUTE( AttribEnumClass, RequiresAttrib, ( "SomethingElse" ) );
AE_REGISTER_ENUM_ATTRIBUTE( AttribEnumClass, DisplayName, ({ .name = "Enum Class" }) );
AE_REGISTER_ENUM_ATTRIBUTE( AttribEnumClass, xyz::EnumLabel, ({ .name = "Qualified" }) );

//------------------------------------------------------------------------------
// AttribCStyleEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM( AttribCStyleEnum );
AE_REGISTER_ENUM_VALUE( AttribCStyleEnum, AttribCStyleIdle );
AE_REGISTER_ENUM_VALUE( AttribCStyleEnum, AttribCStyleWalk );
AE_REGISTER_ENUM_ATTRIBUTE( AttribCStyleEnum, DisplayName, ({ .name = "C Style" }) );

//------------------------------------------------------------------------------
// AttribPrefixEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_PREFIX( AttribPrefixEnum, kAttribPrefixEnum_ );
AE_REGISTER_ENUM_VALUE( AttribPrefixEnum, kAttribPrefixEnum_Idle );
AE_REGISTER_ENUM_VALUE( AttribPrefixEnum, kAttribPrefixEnum_Walk );
AE_REGISTER_ENUM_ATTRIBUTE( AttribPrefixEnum, DisplayName, ({ .name = "Prefix" }) );

//------------------------------------------------------------------------------
// AttribBitFieldEnum
//------------------------------------------------------------------------------
AE_REGISTER_BIT_FIELD_ENUM( AttribBitFieldEnum );
AE_REGISTER_ENUM_VALUE_NAME( AttribBitFieldEnum, AttribBitFieldNone, None );
AE_REGISTER_ENUM_VALUE_NAME( AttribBitFieldEnum, AttribBitFieldRead, Read );
AE_REGISTER_ENUM_VALUE_NAME( AttribBitFieldEnum, AttribBitFieldWrite, Write );
AE_REGISTER_ENUM_ATTRIBUTE( AttribBitFieldEnum, DisplayName, ({ .name = "Bit Field" }) );

//------------------------------------------------------------------------------
// AttribBitFieldPrefixEnum
//------------------------------------------------------------------------------
AE_REGISTER_BIT_FIELD_ENUM_PREFIX( AttribBitFieldPrefixEnum, kABFPE_ );
AE_REGISTER_ENUM_VALUE( AttribBitFieldPrefixEnum, kABFPE_None );
AE_REGISTER_ENUM_VALUE( AttribBitFieldPrefixEnum, kABFPE_Read );
AE_REGISTER_ENUM_VALUE( AttribBitFieldPrefixEnum, kABFPE_Write );
AE_REGISTER_ENUM_ATTRIBUTE( AttribBitFieldPrefixEnum, DisplayName, ({ .name = "Bit Field Prefix" }) );

//------------------------------------------------------------------------------
// AttribEnumClass2
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS2( AttribEnumClass2 );
AE_REGISTER_ENUM_CLASS2_VALUE( AttribEnumClass2, Idle );
AE_REGISTER_ENUM_CLASS2_VALUE( AttribEnumClass2, Walk );
AE_REGISTER_ENUM_ATTRIBUTE( AttribEnumClass2, DisplayName, ({ .name = "Enum Class 2" }) );

//------------------------------------------------------------------------------
// AttribBitFieldEnumClass2
//------------------------------------------------------------------------------
AE_REGISTER_BIT_FIELD_ENUM_CLASS2( AttribBitFieldEnumClass2 );
AE_REGISTER_ENUM_CLASS2_VALUE( AttribBitFieldEnumClass2, None );
AE_REGISTER_ENUM_CLASS2_VALUE( AttribBitFieldEnumClass2, Read );
AE_REGISTER_ENUM_CLASS2_VALUE( AttribBitFieldEnumClass2, Write );
AE_REGISTER_ENUM_ATTRIBUTE( AttribBitFieldEnumClass2, DisplayName, ({ .name = "Bit Field Enum Class 2" }) );

//------------------------------------------------------------------------------
// xyz::AttribNamespacedEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS2( xyz::AttribNamespacedEnum );
AE_REGISTER_ENUM_CLASS2_VALUE( xyz::AttribNamespacedEnum, Idle );
AE_REGISTER_ENUM_CLASS2_VALUE( xyz::AttribNamespacedEnum, Walk );
AE_REGISTER_ENUM_ATTRIBUTE( xyz::AttribNamespacedEnum, DisplayName, ({ .name = "Namespaced" }) );

//------------------------------------------------------------------------------
// AttribUnattributedEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS2( AttribUnattributedEnum );
AE_REGISTER_ENUM_CLASS2_VALUE( AttribUnattributedEnum, Idle );
AE_REGISTER_ENUM_CLASS2_VALUE( AttribUnattributedEnum, Walk );
