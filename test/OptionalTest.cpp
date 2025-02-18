//------------------------------------------------------------------------------
// OptionalTest.cpp
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

// template< typename T >
// Optional< T >::Optional( const T& value ) : m_hasValue( false )
// {
// 	Set( value );
// }

// template< typename T >
// Optional< T >::Optional( T&& value ) noexcept : m_hasValue( false )
// {
// 	Set( std::move( value ) );
// }

// template< typename T >
// Optional< T >::Optional( const Optional< T >& other ) : m_hasValue( false )
// {
// 	Set( other );
// }

// template< typename T >
// Optional< T >::Optional( Optional< T >&& other ) noexcept : m_hasValue( false )
// {
// 	Set( std::move( other ) );
// }

// template< typename T >
// Optional< T >::~Optional()
// {
// 	Clear();
// }

// // OK
// template< typename T >
// void Optional< T >::Set( const T& value )
// {
// 	if( &m_value == &value )
// 	{
// 		return;
// 	}
// 	else if( m_hasValue )
// 	{
// 		*reinterpret_cast< T* >( &m_value ) = value;
// 	}
// 	else
// 	{
// 		new( &m_value ) T( value );
// 		m_hasValue = true;
// 	}
// }

// // OK
// template< typename T >
// void Optional< T >::Set( T&& value ) noexcept
// {
// 	if( reinterpret_cast< T* >( &m_value ) == &value )
// 	{
// 		return;
// 	}
// 	else if( m_hasValue )
// 	{
// 		*reinterpret_cast< T* >( &m_value ) = std::move( value );
// 	}
// 	else
// 	{
// 		new( &m_value ) T( std::move( value ) );
// 		m_hasValue = true;
// 	}
// }

// // OK
// template< typename T >
// void Optional< T >::Set( const Optional< T >& other )
// {
// 	if( other.m_hasValue )
// 	{
// 		Set( *reinterpret_cast< T* >( &other.m_value ) );
// 	}
// 	else
// 	{
// 		Clear();
// 	}
// }

// // OK
// template< typename T >
// void Optional< T >::Set( Optional< T >&& other ) noexcept
// {
// 	if( this == &other )
// 	{
// 		return;
// 	}
// 	else if( other.m_hasValue )
// 	{
// 		Set( std::move( *reinterpret_cast< T* >( &other.m_value ) ) );
// 		other.Clear();
// 	}
// 	else
// 	{
// 		Clear();
// 	}
// }

// template< typename T >
// void Optional< T >::Clear()
// {
// 	if( m_hasValue )
// 	{
// 		reinterpret_cast< T* >( &m_value )->~T();
// 		m_hasValue = false;
// 	}
// }

// template< typename T >
// T* Optional< T >::TryGet()
// {
// 	return m_hasValue ? reinterpret_cast< T* >( &m_value ) : nullptr;
// }

// template< typename T >
// const T* Optional< T >::TryGet() const
// {
// 	return m_hasValue ? reinterpret_cast< const T* >( &m_value ) : nullptr;
// }

//------------------------------------------------------------------------------
// ae::Optional tests
//------------------------------------------------------------------------------
TEST_CASE( "does not call ctor/dtor when default constructed", "[ae::Optional]" )
{
	ae::LifetimeTester::ClearStats();

	ae::Optional< ae::LifetimeTester > opt;
	REQUIRE( !opt.TryGet() );

	REQUIRE( ae::LifetimeTester::ctorCount == 0 );
	REQUIRE( ae::LifetimeTester::copyCount == 0 );
	REQUIRE( ae::LifetimeTester::moveCount == 0 );
	REQUIRE( ae::LifetimeTester::copyAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::moveAssignCount == 0 );
	REQUIRE( ae::LifetimeTester::dtorCount == 0 );
	REQUIRE( ae::LifetimeTester::currentCount == 0 );
}

TEST_CASE( "copy operations", "[ae::Optional]" )
{
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t move = 0;
	int32_t assign = 0;
	int32_t moveAssign = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	ae::LifetimeTester::ClearStats();

	{
		ae::LifetimeTester tester;
		ctor++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		
		ae::Optional< ae::LifetimeTester > opt0 = tester;
		copy++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt0.TryGet() );
		REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );
		
		ae::Optional< ae::LifetimeTester > opt1 = opt0;
		copy++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt1.TryGet() );
		REQUIRE( opt1.TryGet()->check == ae::LifetimeTester::kConstructed );

		ae::Optional< ae::LifetimeTester > opt2;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( !opt2.TryGet() );
		opt2 = opt1;
		copy++; // opt2 is empty, so it's copy constructed
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt1.TryGet() );
		REQUIRE( opt1.TryGet()->check == ae::LifetimeTester::kConstructed );
		REQUIRE( opt2.TryGet() );
		REQUIRE( opt2.TryGet()->check == ae::LifetimeTester::kConstructed );
		opt2 = opt1; // opt2 already has a value, so it's copy assigned
		assign++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt1.TryGet() );
		REQUIRE( opt1.TryGet()->check == ae::LifetimeTester::kConstructed );
		REQUIRE( opt2.TryGet() );
		REQUIRE( opt2.TryGet()->check == ae::LifetimeTester::kConstructed );
	}
	dtor += 4;
	current -= 4;

	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::moveCount == move );
	REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
	REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor );
	REQUIRE( ae::LifetimeTester::currentCount == current );
	REQUIRE( current == ctor + copy + move - dtor );
}

TEST_CASE( "move operations", "[ae::Optional]" )
{
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t move = 0;
	int32_t assign = 0;
	int32_t moveAssign = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	ae::LifetimeTester::ClearStats();

	{
		ae::Optional< ae::LifetimeTester > opt0 = ae::LifetimeTester();
		ctor++;
		move++;
		dtor++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt0.TryGet() );
		REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );

		ae::Optional< ae::LifetimeTester > opt1 = std::move( opt0 );
		move++;
		dtor++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt1.TryGet() );
		REQUIRE( opt1.TryGet()->check == ae::LifetimeTester::kConstructed );
		REQUIRE( !opt0.TryGet() );

		ae::Optional< ae::LifetimeTester > opt2;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( !opt2.TryGet() );
		opt2 = std::move( opt1 ); // opt2 is empty, so it's move constructed
		move++;
		dtor++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( !opt1.TryGet() );
		REQUIRE( opt2.TryGet() );
		REQUIRE( opt2.TryGet()->check == ae::LifetimeTester::kConstructed );
		
		ae::Optional< ae::LifetimeTester > opt3 = ae::LifetimeTester();
		ctor++;
		move++;
		dtor++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt2.TryGet() );
		REQUIRE( opt2.TryGet()->check == ae::LifetimeTester::kConstructed );
		REQUIRE( opt3.TryGet() );
		REQUIRE( opt3.TryGet()->check == ae::LifetimeTester::kConstructed );
		opt2 = std::move( opt3 ); // opt2 already has a value, so it's move assigned
		moveAssign++;
		dtor++;
		current--; // Assigned to another existing object, so only one object remains
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt2.TryGet() );
		REQUIRE( opt2.TryGet()->check == ae::LifetimeTester::kConstructed );
		REQUIRE( !opt3.TryGet() );
	}
	dtor++;
	current--;

	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::moveCount == move );
	REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
	REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor );
	REQUIRE( ae::LifetimeTester::currentCount == current );
	REQUIRE( current == ctor + copy + move - dtor );
}

TEST_CASE( "copy edge cases", "[ae::Optional]" )
{
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t move = 0;
	int32_t assign = 0;
	int32_t moveAssign = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	ae::LifetimeTester::ClearStats();

	{
		ae::LifetimeTester tester;
		ctor++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );

		ae::Optional< ae::LifetimeTester > opt0 = tester;
		copy++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt0.TryGet() );
		REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );

		SECTION( "assign self value" )
		{
			opt0 = *opt0.TryGet();
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::moveCount == move );
			REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
			REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor );
			REQUIRE( ae::LifetimeTester::currentCount == current );
			REQUIRE( current == ctor + copy + move - dtor );
			REQUIRE( opt0.TryGet() );
			REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );
		}

		SECTION( "assign self" )
		{
			opt0 = opt0;
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::moveCount == move );
			REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
			REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor );
			REQUIRE( ae::LifetimeTester::currentCount == current );
			REQUIRE( current == ctor + copy + move - dtor );
			REQUIRE( opt0.TryGet() );
			REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );
		}

		SECTION( "assign empty other" )
		{
			ae::Optional< ae::LifetimeTester > opt1;
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::moveCount == move );
			REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
			REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor );
			REQUIRE( ae::LifetimeTester::currentCount == current );
			REQUIRE( current == ctor + copy + move - dtor );
			REQUIRE( opt0.TryGet() );
			REQUIRE( !opt1.TryGet() );
			opt0 = opt1;
			REQUIRE( !opt0.TryGet() );
			REQUIRE( !opt1.TryGet() );
		}
	}
	dtor += 2;
	current -= 2;

	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::moveCount == move );
	REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
	REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor );
	REQUIRE( ae::LifetimeTester::currentCount == current );
	REQUIRE( current == ctor + copy + move - dtor );
}

TEST_CASE( "move edge cases", "[ae::Optional]" )
{
	int32_t ctor = 0;
	int32_t copy = 0;
	int32_t move = 0;
	int32_t assign = 0;
	int32_t moveAssign = 0;
	int32_t dtor = 0;
	int32_t current = 0;
	ae::LifetimeTester::ClearStats();

	{
		ae::Optional< ae::LifetimeTester > opt0 = ae::LifetimeTester();
		ctor++;
		move++;
		dtor++;
		current++;
		REQUIRE( ae::LifetimeTester::ctorCount == ctor );
		REQUIRE( ae::LifetimeTester::copyCount == copy );
		REQUIRE( ae::LifetimeTester::moveCount == move );
		REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
		REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
		REQUIRE( ae::LifetimeTester::dtorCount == dtor );
		REQUIRE( ae::LifetimeTester::currentCount == current );
		REQUIRE( current == ctor + copy + move - dtor );
		REQUIRE( opt0.TryGet() );
		REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );

		SECTION( "move assign self value" )
		{
			opt0 = std::move( *opt0.TryGet() );
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::moveCount == move );
			REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
			REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor );
			REQUIRE( ae::LifetimeTester::currentCount == current );
			REQUIRE( current == ctor + copy + move - dtor );
			REQUIRE( opt0.TryGet() );
			REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );
		}

		SECTION( "move assign self" )
		{
			opt0 = std::move( opt0 );
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::moveCount == move );
			REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
			REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor );
			REQUIRE( ae::LifetimeTester::currentCount == current );
			REQUIRE( current == ctor + copy + move - dtor );
			REQUIRE( opt0.TryGet() );
			REQUIRE( opt0.TryGet()->check == ae::LifetimeTester::kConstructed );
		}

		SECTION( "assign empty other" )
		{
			ae::Optional< ae::LifetimeTester > opt1;
			REQUIRE( ae::LifetimeTester::ctorCount == ctor );
			REQUIRE( ae::LifetimeTester::copyCount == copy );
			REQUIRE( ae::LifetimeTester::moveCount == move );
			REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
			REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
			REQUIRE( ae::LifetimeTester::dtorCount == dtor );
			REQUIRE( ae::LifetimeTester::currentCount == current );
			REQUIRE( current == ctor + copy + move - dtor );
			REQUIRE( opt0.TryGet() );
			REQUIRE( !opt1.TryGet() );
			opt0 = std::move( opt1 );
			REQUIRE( !opt0.TryGet() );
			REQUIRE( !opt1.TryGet() );
		}
	}
	dtor++;
	current--;

	REQUIRE( ae::LifetimeTester::ctorCount == ctor );
	REQUIRE( ae::LifetimeTester::copyCount == copy );
	REQUIRE( ae::LifetimeTester::moveCount == move );
	REQUIRE( ae::LifetimeTester::copyAssignCount == assign );
	REQUIRE( ae::LifetimeTester::moveAssignCount == moveAssign );
	REQUIRE( ae::LifetimeTester::dtorCount == dtor );
	REQUIRE( ae::LifetimeTester::currentCount == current );
	REQUIRE( current == ctor + copy + move - dtor );
}
