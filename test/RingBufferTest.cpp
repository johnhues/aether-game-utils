//------------------------------------------------------------------------------
// RingBufferTest.cpp
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
#include "TestUtils.h"
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// Consants
//------------------------------------------------------------------------------
const ae::Tag TAG_TEST = "test";

//------------------------------------------------------------------------------
// ae::RingBuffer tests
//------------------------------------------------------------------------------
TEST_CASE( "Static RingBuffer", "[aeRingBuffer]" )
{
	ae::RingBuffer< ae::LifetimeTester, 4 > ringBuffer;
	REQUIRE( ringBuffer.Length() == 0 );
	REQUIRE( ringBuffer.Size() == 4 );
}

TEST_CASE( "Can append and clear static RingBuffer", "[aeRingBuffer]" )
{
	ae::RingBuffer< ae::LifetimeTester, 4 > ringBuffer;
	REQUIRE( ringBuffer.Length() == 0 );
	REQUIRE( ringBuffer.Size() == 4 );
	
	ringBuffer.Append( {} ).value = 1000;
	REQUIRE( ringBuffer.Length() == 1 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	
	ringBuffer.Append( {} ).value = 1001;
	REQUIRE( ringBuffer.Length() == 2 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	
	ringBuffer.Append( {} ).value = 1002;
	REQUIRE( ringBuffer.Length() == 3 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	
	ringBuffer.Append( {} ).value = 1003;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1003 );
	
	ringBuffer.Append( {} ).value = 1004;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1002 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1003 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1004 );
	
	ringBuffer.Clear();
	REQUIRE( ringBuffer.Length() == 0 );
	REQUIRE( ringBuffer.Size() == 4 );
	
	ringBuffer.Append( {} ).value = 1005;
	REQUIRE( ringBuffer.Length() == 1 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	
	ringBuffer.Append( {} ).value = 1006;
	REQUIRE( ringBuffer.Length() == 2 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1006 );
	
	ringBuffer.Append( {} ).value = 1007;
	REQUIRE( ringBuffer.Length() == 3 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1006 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1007 );
	
	ringBuffer.Append( {} ).value = 1008;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1006 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1007 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1008 );
	
	ringBuffer.Append( {} ).value = 1009;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1006 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1007 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1008 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1009 );
}

TEST_CASE( "Dynamic RingBuffer", "[aeRingBuffer]" )
{
	ae::RingBuffer< ae::LifetimeTester > ringBuffer( TAG_TEST, 4 );
	REQUIRE( ringBuffer.Length() == 0 );
	REQUIRE( ringBuffer.Size() == 4 );
}

TEST_CASE( "Can append and clear dynamic RingBuffer", "[aeRingBuffer]" )
{
	ae::RingBuffer< ae::LifetimeTester > ringBuffer( TAG_TEST, 4 );
	REQUIRE( ringBuffer.Length() == 0 );
	REQUIRE( ringBuffer.Size() == 4 );
	
	ringBuffer.Append( {} ).value = 1000;
	REQUIRE( ringBuffer.Length() == 1 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	
	ringBuffer.Append( {} ).value = 1001;
	REQUIRE( ringBuffer.Length() == 2 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	
	ringBuffer.Append( {} ).value = 1002;
	REQUIRE( ringBuffer.Length() == 3 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	
	ringBuffer.Append( {} ).value = 1003;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1003 );
	
	ringBuffer.Append( {} ).value = 1004;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1002 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1003 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1004 );
	
	ringBuffer.Clear();
	REQUIRE( ringBuffer.Length() == 0 );
	REQUIRE( ringBuffer.Size() == 4 );
	
	ringBuffer.Append( {} ).value = 1005;
	REQUIRE( ringBuffer.Length() == 1 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	
	ringBuffer.Append( {} ).value = 1006;
	REQUIRE( ringBuffer.Length() == 2 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1006 );
	
	ringBuffer.Append( {} ).value = 1007;
	REQUIRE( ringBuffer.Length() == 3 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1006 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1007 );
	
	ringBuffer.Append( {} ).value = 1008;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1005 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1006 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1007 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1008 );
	
	ringBuffer.Append( {} ).value = 1009;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1006 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1007 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1008 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1009 );
}

TEST_CASE( "Can set ref with Get()", "[aeRingBuffer]" )
{
	ae::RingBuffer< ae::LifetimeTester, 4 > ringBuffer;
	REQUIRE( ringBuffer.Length() == 0 );
	REQUIRE( ringBuffer.Size() == 4 );
	
	ringBuffer.Append( {} ).value = 1000;
	REQUIRE( ringBuffer.Length() == 1 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	
	ringBuffer.Append( {} ).value = 1001;
	REQUIRE( ringBuffer.Length() == 2 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	
	ringBuffer.Append( {} ).value = 1002;
	REQUIRE( ringBuffer.Length() == 3 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	
	ringBuffer.Append( {} ).value = 1003;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 1000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1003 );
	
	ringBuffer.Get( 0 ).value = 2000;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 2000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 1001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1003 );
	
	ringBuffer.Get( 1 ).value = 2001;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 2000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 2001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 1002 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1003 );
	
	ringBuffer.Get( 2 ).value = 2002;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 2000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 2001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 2002 );
	REQUIRE( ringBuffer.Get( 3 ).value == 1003 );
	
	ringBuffer.Get( 3 ).value = 2003;
	REQUIRE( ringBuffer.Length() == 4 );
	REQUIRE( ringBuffer.Size() == 4 );
	REQUIRE( ringBuffer.Get( 0 ).value == 2000 );
	REQUIRE( ringBuffer.Get( 1 ).value == 2001 );
	REQUIRE( ringBuffer.Get( 2 ).value == 2002 );
	REQUIRE( ringBuffer.Get( 3 ).value == 2003 );
}
