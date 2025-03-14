//------------------------------------------------------------------------------
// ReferenceTest.cpp
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
#include "aether.h"
#include <catch2/catch_test_macros.hpp>
#include "ae/Reference.h"

//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------
struct SomethingA
{
	virtual ~SomethingA() = default;
	ae::ReferenceCounted< SomethingA > ref = this;
};

struct SomethingB : public SomethingA
{
};

struct SomethingAA
{
	virtual ~SomethingAA() = default;
};

struct SomethingBB : public SomethingAA
{
	ae::ReferenceCounted< SomethingBB > ref = this;
};

TEST_CASE( "Reference basics", "[ae::Reference]" )
{
	uint32_t refCountA = 0;
	uint32_t refCountB = 0;
	SomethingA* rawPointerA = new SomethingA();
	SomethingB* rawPointerB = new SomethingB();
	
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );

	ae::Reference< SomethingA > ref = rawPointerA->ref;
	refCountA++;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( ref.Get() == rawPointerA );

	ref = ref;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( ref.Get() == rawPointerA );

	ae::Reference< const SomethingA > constRef = rawPointerA->ref;
	refCountA++;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( constRef.Get() == rawPointerA );
	
	constRef = constRef;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( constRef.Get() == rawPointerA );

	constRef = ref;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( constRef.Get() == rawPointerA );

	ref = rawPointerB->ref;
	refCountA--;
	refCountB++;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( ref.Get() == rawPointerB );

	// Assignment
	ref = rawPointerA->ref;
	constRef = rawPointerB->ref;
	refCountA = 1;
	refCountB = 1;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( ref.Get() == rawPointerA );
	REQUIRE( constRef.Get() == rawPointerB );
	constRef = ref;
	refCountA++;
	refCountB--;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( ref.Get() == rawPointerA );
	REQUIRE( constRef.Get() == rawPointerA );

	// Clear
	ref.Clear();
	constRef.Clear();
	refCountA = 0;
	refCountB = 0;
	REQUIRE( rawPointerA->ref.GetReferenceCount() == refCountA );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == refCountB );
	REQUIRE( ref.Get() == nullptr );
	REQUIRE( constRef.Get() == nullptr );

	delete rawPointerA;
	delete rawPointerB;
}

TEST_CASE( "Reference inheritance", "[ae::Reference]" )
{
	uint32_t refCount = 0;
	SomethingBB* rawPointer = ae::New< SomethingBB >( AE_ALLOC_TAG_META_TEST );
	
	REQUIRE( rawPointer->ref.GetReferenceCount() == refCount );

	ae::Reference< SomethingAA > ref = rawPointer->ref;
	refCount++;
	REQUIRE( rawPointer->ref.GetReferenceCount() == refCount );
	REQUIRE( ref.Get() == rawPointer );

	ref = ref;
	REQUIRE( rawPointer->ref.GetReferenceCount() == refCount );
	REQUIRE( ref.Get() == rawPointer );

	ae::Reference< const SomethingAA > constRef = rawPointer->ref;
	refCount++;
	REQUIRE( rawPointer->ref.GetReferenceCount() == refCount );
	REQUIRE( constRef.Get() == rawPointer );
	
	constRef = constRef;
	REQUIRE( rawPointer->ref.GetReferenceCount() == refCount );
	REQUIRE( constRef.Get() == rawPointer );

	constRef = ref;
	REQUIRE( rawPointer->ref.GetReferenceCount() == refCount );
	REQUIRE( constRef.Get() == rawPointer );

	ref.Clear();
	constRef.Clear();
	refCount = 0;
	REQUIRE( rawPointer->ref.GetReferenceCount() == refCount );
	REQUIRE( ref.Get() == nullptr );
	REQUIRE( constRef.Get() == nullptr );

	ae::Delete( rawPointer );
}

TEST_CASE( "Reference cast", "[ae::Reference]" )
{
	SomethingB* rawPointerB = ae::New< SomethingB >( AE_ALLOC_TAG_META_TEST );
	
	ae::Reference< SomethingA > refA = rawPointerB->ref;
	REQUIRE( rawPointerB->ref.GetReferenceCount() == 1 );
	REQUIRE( refA.Get() == rawPointerB );
	ae::Reference< SomethingB > refB = refA.Cast< SomethingB >( rawPointerB );
	REQUIRE( rawPointerB->ref.GetReferenceCount() == 2 );
	REQUIRE( refB.Get() == rawPointerB );

	refA.Clear();
	refB.Clear();
	
	ae::Delete( rawPointerB );
}
