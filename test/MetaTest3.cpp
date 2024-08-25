//------------------------------------------------------------------------------
// MetaTest3.cpp
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
#include "MetaTest.h"

// @NOTE: This is file exists to make sure types registered here can be used
// with registered member variables in MetaTest2.cpp

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
