//------------------------------------------------------------------------------
// MetaTest2.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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

//------------------------------------------------------------------------------
// PlayerState
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS( PlayerState );

//------------------------------------------------------------------------------
// SomeClass + TestEnumClass
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( SomeClass );

AE_REGISTER_CLASS_PROPERTY( SomeClass, someProp0 );

AE_REGISTER_CLASS_PROPERTY_VALUE( SomeClass, someProp1, v0 );
AE_REGISTER_CLASS_PROPERTY_VALUE( SomeClass, someProp1, v1 );

AE_REGISTER_CLASS_PROPERTY( SomeClass, someProp2 );
AE_REGISTER_CLASS_PROPERTY_VALUE( SomeClass, someProp2, v0 );
AE_REGISTER_CLASS_PROPERTY_VALUE( SomeClass, someProp2, v1 );
AE_REGISTER_CLASS_PROPERTY_VALUE( SomeClass, someProp2, v2 );

AE_REGISTER_CLASS_VAR( SomeClass, intMember );
AE_REGISTER_CLASS_VAR_PROPERTY( SomeClass, intMember, intProp );

AE_REGISTER_CLASS_VAR( SomeClass, boolMember );
AE_REGISTER_CLASS_VAR_PROPERTY( SomeClass, boolMember, boolProp );
AE_REGISTER_CLASS_VAR_PROPERTY_VALUE( SomeClass, boolMember, boolProp, val );

AE_REGISTER_CLASS_VAR( SomeClass, enumTest );
AE_REGISTER_CLASS_VAR_PROPERTY_VALUE( SomeClass, enumTest, prop0, val0 );
AE_REGISTER_CLASS_VAR_PROPERTY_VALUE( SomeClass, enumTest, prop1, val0 );
AE_REGISTER_CLASS_VAR_PROPERTY_VALUE( SomeClass, enumTest, prop1, val1 );

AE_REGISTER_ENUM_CLASS( TestEnumClass );

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
// GamePadBitField
//------------------------------------------------------------------------------
AE_REGISTER_ENUM_CLASS( GamePadBitField );

//------------------------------------------------------------------------------
// Reference testing
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( RefTester );

AE_REGISTER_CLASS( RefTesterA );
AE_REGISTER_CLASS_VAR( RefTesterA, notRef );
AE_REGISTER_CLASS_VAR( RefTesterA, refA );
AE_REGISTER_CLASS_VAR( RefTesterA, refB ); // @NOTE: RefTesterB is registered after this, but it has to still work

AE_REGISTER_CLASS( RefTesterB );
AE_REGISTER_CLASS_VAR( RefTesterB, refA );
