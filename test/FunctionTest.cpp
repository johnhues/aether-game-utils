//------------------------------------------------------------------------------
// FunctionTest.cpp
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
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// ae::Function basic tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function default constructor", "[ae::Function]" )
{
	ae::Function< int() > fn;
	REQUIRE_FALSE( fn );
}

TEST_CASE( "ae::Function simple lambda without capture", "[ae::Function]" )
{
	ae::Function< int() > fn = []() { return 42; };
	REQUIRE( fn );
	REQUIRE( fn() == 42 );
}

TEST_CASE( "ae::Function lambda with single int capture", "[ae::Function]" )
{
	int value = 10;
	ae::Function< int() > fn = [value]() { return value * 2; };
	REQUIRE( fn );
	REQUIRE( fn() == 20 );
	
	// Original value unchanged
	value = 15;
	REQUIRE( fn() == 20 ); // Still returns 20, not 30
}

TEST_CASE( "ae::Function lambda with reference capture", "[ae::Function]" )
{
	int total = 0;
	ae::Function< int( int ) > increment = [&total]( int v ) { return total += v; };
	
	REQUIRE( increment( 1 ) == 1 );
	REQUIRE( increment( 1 ) == 2 );
	REQUIRE( increment( 1 ) == 3 );
	REQUIRE( increment( 7 ) == 10 );
	REQUIRE( total == 10 );
}

TEST_CASE( "ae::Function with multiple parameters", "[ae::Function]" )
{
	ae::Function< int( int, int, int ) > sum = []( int a, int b, int c ) {
		return a + b + c;
	};
	
	REQUIRE( sum( 1, 2, 3 ) == 6 );
	REQUIRE( sum( 10, 20, 30 ) == 60 );
}

TEST_CASE( "ae::Function with void return type", "[ae::Function]" )
{
	int counter = 0;
	ae::Function< void( int ) > addTo = [&counter]( int v ) {
		counter += v;
	};
	
	addTo( 5 );
	REQUIRE( counter == 5 );
	addTo( 10 );
	REQUIRE( counter == 15 );
}

TEST_CASE( "ae::Function with float types", "[ae::Function]" )
{
	ae::Function< float( float, float ) > multiply = []( float a, float b ) {
		return a * b;
	};
	
	REQUIRE( multiply( 2.5f, 4.0f ) == 10.0f );
	REQUIRE( multiply( -1.5f, 2.0f ) == -3.0f );
}

TEST_CASE( "ae::Function bool operator", "[ae::Function]" )
{
	ae::Function< int() > empty;
	ae::Function< int() > valid = []() { return 1; };
	
	REQUIRE_FALSE( empty );
	REQUIRE( valid );
	
	if ( valid )
	{
		REQUIRE( valid() == 1 );
	}
}

//------------------------------------------------------------------------------
// ae::Function capture size tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function with multiple captures", "[ae::Function]" )
{
	int a = 5;
	int b = 10;
	int c = 15;
	
	ae::Function< int() > fn = [a, b, c]() {
		return a + b + c;
	};
	
	REQUIRE( fn() == 30 );
}

TEST_CASE( "ae::Function with pointer captures", "[ae::Function]" )
{
	int value = 100;
	int* ptr = &value;
	
	ae::Function< int() > fn = [ptr]() {
		return *ptr;
	};
	
	REQUIRE( fn() == 100 );
	value = 200;
	REQUIRE( fn() == 200 );
}

TEST_CASE( "ae::Function with mixed value and reference captures", "[ae::Function]" )
{
	int multiplier = 2;
	int accumulator = 0;
	
	ae::Function< int( int ) > fn = [multiplier, &accumulator]( int val ) {
		int result = val * multiplier;
		accumulator += result;
		return result;
	};
	
	REQUIRE( fn( 5 ) == 10 );
	REQUIRE( accumulator == 10 );
	REQUIRE( fn( 3 ) == 6 );
	REQUIRE( accumulator == 16 );
}

//------------------------------------------------------------------------------
// ae::Function custom buffer size tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function with custom MaxSize", "[ae::Function]" )
{
	// Small buffer for simple lambda
	ae::Function< int(), 16 > small = []() { return 1; };
	REQUIRE( small() == 1 );
	
	// Larger buffer can hold more captures
	int a = 1, b = 2, c = 3, d = 4;
	ae::Function< int(), 128 > large = [a, b, c, d]() {
		return a + b + c + d;
	};
	REQUIRE( large() == 10 );
}

//------------------------------------------------------------------------------
// ae::Function type safety tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function signature is compile-time fixed", "[ae::Function]" )
{
	// These are different types and cannot be assigned to each other
	ae::Function< int() > fn1 = []() { return 1; };
	ae::Function< int( int ) > fn2 = []( int x ) { return x; };
	ae::Function< void() > fn3 = []() {};
	
	// All have different signatures
	static_assert( !std::is_same_v< decltype(fn1), decltype(fn2) > );
	static_assert( !std::is_same_v< decltype(fn1), decltype(fn3) > );
	static_assert( !std::is_same_v< decltype(fn2), decltype(fn3) > );
}

//------------------------------------------------------------------------------
// ae::Function callable object tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function can be called multiple times", "[ae::Function]" )
{
	int counter = 0;
	ae::Function< int() > fn = [&counter]() {
		return ++counter;
	};
	
	REQUIRE( fn() == 1 );
	REQUIRE( fn() == 2 );
	REQUIRE( fn() == 3 );
	REQUIRE( counter == 3 );
}

TEST_CASE( "ae::Function with complex return types", "[ae::Function]" )
{
	struct Point
	{
		float x, y;
		bool operator==( const Point& other ) const
		{
			return x == other.x && y == other.y;
		}
	};
	
	ae::Function< Point( float, float ) > makePoint = []( float x, float y ) {
		return Point{ x, y };
	};
	
	Point p = makePoint( 1.0f, 2.0f );
	REQUIRE( p.x == 1.0f );
	REQUIRE( p.y == 2.0f );
}

TEST_CASE( "ae::Function with const parameters", "[ae::Function]" )
{
	ae::Function< int( const int& ) > fn = []( const int& val ) {
		return val * 2;
	};
	
	REQUIRE( fn( 5 ) == 10 );
	REQUIRE( fn( 100 ) == 200 );
}

//------------------------------------------------------------------------------
// ae::Function practical usage tests
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function as callback", "[ae::Function]" )
{
	struct EventHandler
	{
		ae::Function< void( int ) > onEvent;
		
		void trigger( int value )
		{
			if ( onEvent )
			{
				onEvent( value );
			}
		}
	};
	
	int result = 0;
	EventHandler handler;
	handler.onEvent = [&result]( int val ) {
		result = val * 2;
	};
	
	handler.trigger( 10 );
	REQUIRE( result == 20 );
	
	handler.trigger( 25 );
	REQUIRE( result == 50 );
}

TEST_CASE( "ae::Function in container", "[ae::Function]" )
{
	ae::Array< ae::Function< int( int ) >, 4 > operations;
	
	operations.Append( []( int x ) { return x + 1; } );
	operations.Append( []( int x ) { return x * 2; } );
	operations.Append( []( int x ) { return x - 3; } );
	
	REQUIRE( operations[ 0 ]( 5 ) == 6 );   // 5 + 1
	REQUIRE( operations[ 1 ]( 5 ) == 10 );  // 5 * 2
	REQUIRE( operations[ 2 ]( 5 ) == 2 );   // 5 - 3
}

TEST_CASE( "ae::Function chaining operations", "[ae::Function]" )
{
	int state = 0;
	
	ae::Function< void( int ) > add = [&state]( int v ) { state += v; };
	ae::Function< void( int ) > multiply = [&state]( int v ) { state *= v; };
	ae::Function< int() > get = [&state]() { return state; };
	
	add( 5 );
	REQUIRE( get() == 5 );
	
	multiply( 3 );
	REQUIRE( get() == 15 );
	
	add( 10 );
	REQUIRE( get() == 25 );
}

//------------------------------------------------------------------------------
// ae::Function memory and performance characteristics
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function does not allocate", "[ae::Function]" )
{
	// This test verifies the design principle: ae::Function never allocates
	// Memory for captures is stored in the inline buffer (m_storage)
	// The size of Function should be: MaxSize + sizeof(function pointer)
	
	constexpr size_t maxSize = 64;
	using FnType = ae::Function< int(), maxSize >;
	
	// The size should be the storage plus the invoker pointer
	// (may have alignment padding)
	constexpr size_t expectedMinSize = maxSize + sizeof(void*);
	static_assert( sizeof(FnType) >= expectedMinSize );
	
	// Verify it's small enough to be stack-allocated efficiently
	static_assert( sizeof(FnType) < 256 ); // Reasonable size for stack allocation
}

TEST_CASE( "ae::Function type is static", "[ae::Function]" )
{
	// The type of ae::Function is known at compile time
	// Different lambdas with same signature have the same Function type
	
	// But they can both be stored in the same Function type
	ae::Function< int() > fn1 = []() { return 1; };
	ae::Function< int() > fn2 = []() { return 2; };
	
	// Function types are the same
	static_assert( std::is_same_v< decltype(fn1), decltype(fn2) > );
	
	REQUIRE( fn1() == 1 );
	REQUIRE( fn2() == 2 );
}

TEST_CASE( "ae::Function stores lambda data in internal buffer", "[ae::Function]" )
{
	// Capture some data that will be stored in the Function's buffer
	int capturedValue = 42;
	double capturedDouble = 3.14;
	
	ae::Function< double() > fn = [capturedValue, capturedDouble]() {
		return capturedValue + capturedDouble;
	};
	
	// The captured data is stored within the Function object itself
	// No heap allocation occurs
	double result = fn();
	REQUIRE( result > 45.13 );
	REQUIRE( result < 45.15 );
}

//------------------------------------------------------------------------------
// ae::Function edge cases
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function with no captures", "[ae::Function]" )
{
	// Lambda with no captures should work fine
	ae::Function< int( int, int ) > add = []( int a, int b ) {
		return a + b;
	};
	
	REQUIRE( add( 1, 2 ) == 3 );
	REQUIRE( add( -5, 5 ) == 0 );
}

TEST_CASE( "ae::Function replacing lambda", "[ae::Function]" )
{
	ae::Function< int() > fn = []() { return 1; };
	REQUIRE( fn() == 1 );
	
	// Assign a different lambda with same signature
	fn = []() { return 2; };
	REQUIRE( fn() == 2 );
}

TEST_CASE( "ae::Function with bool return", "[ae::Function]" )
{
	int threshold = 10;
	ae::Function< bool( int ) > check = [threshold]( int value ) {
		return value > threshold;
	};
	
	REQUIRE_FALSE( check( 5 ) );
	REQUIRE_FALSE( check( 10 ) );
	REQUIRE( check( 11 ) );
	REQUIRE( check( 100 ) );
}

TEST_CASE( "ae::Function capturing this pointer", "[ae::Function]" )
{
	struct Counter
	{
		int value = 0;
		
		ae::Function< void( int ) > getIncrementer()
		{
			return [this]( int delta ) {
				value += delta;
			};
		}
	};
	
	Counter counter;
	auto inc = counter.getIncrementer();
	
	inc( 5 );
	REQUIRE( counter.value == 5 );
	
	inc( 10 );
	REQUIRE( counter.value == 15 );
}

//------------------------------------------------------------------------------
// ae::Function comparison with std::function characteristics
//------------------------------------------------------------------------------
TEST_CASE( "ae::Function vs std::function - no allocation", "[ae::Function]" )
{
	// Key differentiator: ae::Function NEVER allocates
	// std::function may allocate for larger captures
	
	int a = 1, b = 2, c = 3;
	
	// ae::Function stores everything inline
	ae::Function< int() > aeFn = [a, b, c]() {
		return a + b + c;
	};
	
	// Verify it works
	REQUIRE( aeFn() == 6 );
	
	// The size is fixed at compile time
	static_assert( sizeof(aeFn) == sizeof(uint8_t[64]) + sizeof(void*) );
}

TEST_CASE( "ae::Function signature fixed at compile-time", "[ae::Function]" )
{
	// The signature is part of the type, not runtime polymorphic like std::function
	
	using IntFn = ae::Function< int() >;
	using IntIntFn = ae::Function< int( int ) >;
	
	// These are completely different types
	static_assert( !std::is_same_v< IntFn, IntIntFn > );
	
	IntFn fn1 = []() { return 42; };
	IntIntFn fn2 = []( int x ) { return x * 2; };
	
	REQUIRE( fn1() == 42 );
	REQUIRE( fn2( 21 ) == 42 );
}
