//------------------------------------------------------------------------------
// MetaTest2.cpp
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
#include "MetaTest.h"

// @NOTE: This is file exists for two reasons:
// 1) To make sure that no aeMeta utilities  produce duplicate symbol errors.
// 2) To make sure aeMeta registered types are available outside of the module
// they were registered in.

AE_REGISTER_CLASS( CustomBaseType );
AE_REGISTER_CLASS( CustomBaseTypeTest );
AE_REGISTER_CLASS_VAR( CustomBaseTypeTest, testInt );

//------------------------------------------------------------------------------
// PlayerState
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS( PlayerState );

//------------------------------------------------------------------------------
// TestEnumClass
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS( TestEnumClass );

//------------------------------------------------------------------------------
// Bit field enums
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS( GamePadBitField );
AE_REGISTER_ENUM_CLASS( SceneFlags );

//------------------------------------------------------------------------------
// Enum width coverage
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS2( EnumInt8 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt8, Min );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt8, NegativeOne );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt8, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt8, Max );

AE_REGISTER_ENUM_CLASS2( EnumUInt8 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt8, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt8, One );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt8, HighBit );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt8, Max );

AE_REGISTER_ENUM_CLASS2( EnumInt16 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt16, Min );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt16, NegativeOne );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt16, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt16, Max );

AE_REGISTER_ENUM_CLASS2( EnumUInt16 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt16, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt16, One );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt16, HighBit );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt16, Max );

AE_REGISTER_ENUM_CLASS2( EnumInt32 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt32, Min );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt32, NegativeOne );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt32, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt32, Max );

AE_REGISTER_ENUM_CLASS2( EnumUInt32 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt32, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt32, One );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt32, HighBit );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt32, Max );

AE_REGISTER_ENUM_CLASS2( EnumInt64 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt64, Min );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt64, NegativeOne );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt64, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumInt64, Max );

AE_REGISTER_ENUM_CLASS2( EnumUInt64 );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt64, Zero );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt64, One );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt64, HighBit );
AE_REGISTER_ENUM_CLASS2_VALUE( EnumUInt64, Max );

//------------------------------------------------------------------------------
// Namespace0::Namespace1::NamespaceClass registration
//------------------------------------------------------------------------------
AE_FORCE_LINK_CLASS( Namespace0, Namespace1, NamespaceClass );
AE_REGISTER_NAMESPACECLASS( (Namespace0, Namespace1, NamespaceClass) );

#if AE_DEPRECATED
AE_REGISTER_NAMESPACECLASS_PROPERTY( (Namespace0, Namespace1, NamespaceClass), someProp0 );

AE_REGISTER_NAMESPACECLASS_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), someProp1, v0 );
AE_REGISTER_NAMESPACECLASS_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), someProp1, v1 );

AE_REGISTER_NAMESPACECLASS_PROPERTY( (Namespace0, Namespace1, NamespaceClass), someProp2 );
AE_REGISTER_NAMESPACECLASS_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), someProp2, v0 );
AE_REGISTER_NAMESPACECLASS_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), someProp2, v1 );
AE_REGISTER_NAMESPACECLASS_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), someProp2, v2 );

AE_REGISTER_NAMESPACECLASS_VAR( (Namespace0, Namespace1, NamespaceClass), intMember );
AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY( (Namespace0, Namespace1, NamespaceClass), intMember, intProp );

AE_REGISTER_NAMESPACECLASS_VAR( (Namespace0, Namespace1, NamespaceClass), boolMember );
AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY( (Namespace0, Namespace1, NamespaceClass), boolMember, boolProp );
AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), boolMember, boolProp, val );

AE_REGISTER_NAMESPACECLASS_VAR( (Namespace0, Namespace1, NamespaceClass), enumTest );
AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), enumTest, prop0, val0 );
AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), enumTest, prop1, val0 );
AE_REGISTER_NAMESPACECLASS_VAR_PROPERTY_VALUE( (Namespace0, Namespace1, NamespaceClass), enumTest, prop1, val1 );
#endif // AE_DEPRECATED

AE_REGISTER_NAMESPACECLASS_VAR( (Namespace0, Namespace1, NamespaceClass), uuidMember );

//------------------------------------------------------------------------------
// AggregateClass
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( AggregateClass );
AE_REGISTER_CLASS_VAR( AggregateClass, someClass );
AE_REGISTER_CLASS_VAR( AggregateClass, someClass1 );

//------------------------------------------------------------------------------
// ArrayClass
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( ArrayClass );
AE_REGISTER_CLASS_VAR( ArrayClass, intArray );
AE_REGISTER_CLASS_VAR( ArrayClass, intArray2 );
AE_REGISTER_CLASS_VAR( ArrayClass, intArray3 );
AE_REGISTER_CLASS_VAR( ArrayClass, someClassArray );
AE_REGISTER_CLASS_VAR( ArrayClass, someClassArray2 );
AE_REGISTER_CLASS_VAR( ArrayClass, someClassArray3 );

//------------------------------------------------------------------------------
// OptionalClass
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( OptionalClass );
AE_REGISTER_CLASS_VAR( OptionalClass, intOptional );
AE_REGISTER_CLASS_VAR( OptionalClass, someClassOptional );
AE_REGISTER_CLASS_VAR( OptionalClass, intStdOptional );
AE_REGISTER_CLASS_VAR( OptionalClass, someClassStdOptional );

//------------------------------------------------------------------------------
// MapClass
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( MapClass );
AE_REGISTER_CLASS_VAR( MapClass, intMap );
AE_REGISTER_CLASS_VAR( MapClass, intMapFixed );

//------------------------------------------------------------------------------
// SomeOldEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM( SomeOldEnum );
AE_REGISTER_ENUM_VALUE( SomeOldEnum, Bleep );
AE_REGISTER_ENUM_VALUE( SomeOldEnum, Bloop );
AE_REGISTER_ENUM_VALUE( SomeOldEnum, Blop );

//------------------------------------------------------------------------------
// SomeOldPrefixEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_PREFIX( SomeOldPrefixEnum, kSomeOldPrefixEnum_ );
AE_REGISTER_ENUM_VALUE( SomeOldPrefixEnum, kSomeOldPrefixEnum_Bleep );
AE_REGISTER_ENUM_VALUE( SomeOldPrefixEnum, kSomeOldPrefixEnum_Bloop );
AE_REGISTER_ENUM_VALUE( SomeOldPrefixEnum, kSomeOldPrefixEnum_Blop );

//------------------------------------------------------------------------------
// SomeOldRenamedEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM( SomeOldRenamedEnum );
AE_REGISTER_ENUM_VALUE_NAME( SomeOldRenamedEnum, BLEEP, Bleep );
AE_REGISTER_ENUM_VALUE_NAME( SomeOldRenamedEnum, BLOOP, Bloop );
AE_REGISTER_ENUM_VALUE_NAME( SomeOldRenamedEnum, BLOP, Blop );

//------------------------------------------------------------------------------
// SomeNewEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS2( SomeNewEnum );
AE_REGISTER_ENUM_CLASS2_VALUE( SomeNewEnum, Bleep );
AE_REGISTER_ENUM_CLASS2_VALUE( SomeNewEnum, Bloop );
AE_REGISTER_ENUM_CLASS2_VALUE( SomeNewEnum, Blop );

//------------------------------------------------------------------------------
// A::B::SomeNewEnum
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS2( A::B::SomeNewEnum );
AE_REGISTER_ENUM_CLASS2_VALUE( A::B::SomeNewEnum, Bleep );
AE_REGISTER_ENUM_CLASS2_VALUE( A::B::SomeNewEnum, Bloop );
AE_REGISTER_ENUM_CLASS2_VALUE( A::B::SomeNewEnum, Blop );

//------------------------------------------------------------------------------
// OldBitFieldFlags
//------------------------------------------------------------------------------
AE_REGISTER_BIT_FIELD_ENUM( OldBitFieldFlags );
AE_REGISTER_ENUM_VALUE_NAME( OldBitFieldFlags, OldBitFieldFlags_None, None );
AE_REGISTER_ENUM_VALUE_NAME( OldBitFieldFlags, OldBitFieldFlags_Read, Read );
AE_REGISTER_ENUM_VALUE_NAME( OldBitFieldFlags, OldBitFieldFlags_Write, Write );
AE_REGISTER_ENUM_VALUE_NAME( OldBitFieldFlags, OldBitFieldFlags_Execute, Execute );

//------------------------------------------------------------------------------
// OldBitFieldPrefixFlags
//------------------------------------------------------------------------------
AE_REGISTER_BIT_FIELD_ENUM_PREFIX( OldBitFieldPrefixFlags, kOBPF_ );
AE_REGISTER_ENUM_VALUE( OldBitFieldPrefixFlags, kOBPF_None );
AE_REGISTER_ENUM_VALUE( OldBitFieldPrefixFlags, kOBPF_Read );
AE_REGISTER_ENUM_VALUE( OldBitFieldPrefixFlags, kOBPF_Write );
AE_REGISTER_ENUM_VALUE( OldBitFieldPrefixFlags, kOBPF_Execute );

//------------------------------------------------------------------------------
// NewBitFieldFlags
//------------------------------------------------------------------------------
AE_REGISTER_BIT_FIELD_ENUM_CLASS2( NewBitFieldFlags );
AE_REGISTER_ENUM_CLASS2_VALUE( NewBitFieldFlags, None );
AE_REGISTER_ENUM_CLASS2_VALUE( NewBitFieldFlags, Read );
AE_REGISTER_ENUM_CLASS2_VALUE( NewBitFieldFlags, Write );
AE_REGISTER_ENUM_CLASS2_VALUE( NewBitFieldFlags, Execute );

//------------------------------------------------------------------------------
// Reference testing
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( RefTester );

AE_REGISTER_CLASS( RefTesterA );
AE_REGISTER_CLASS_VAR( RefTesterA, notRef );
AE_REGISTER_CLASS_VAR( RefTesterA, refA );
AE_REGISTER_CLASS_VAR( RefTesterA, refB ); // @NOTE: RefTesterB is registered after this, but it has to still work

AE_REGISTER_CLASS( RefTesterB );
AE_REGISTER_CLASS_VAR( RefTesterB, ref );
