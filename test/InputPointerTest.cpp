//------------------------------------------------------------------------------
// InputPointerTest.cpp
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
// Helpers
//------------------------------------------------------------------------------
// Simulates one platform mouse event
static void MouseEvent( ae::Input* input, bool left, bool middle, bool right, ae::Vec2 pos )
{
	input->m_leftButton = left;
	input->m_middleButton = middle;
	input->m_rightButton = right;
	input->m_UpdateMousePointer( pos );
}

static void ReleaseAll( ae::Input* input )
{
	while( input->PumpPointers() ) {}
	for( const ae::InputPointer* pointer : input->GetPointers() )
	{
		input->ReleasePointer( pointer );
	}
}

//------------------------------------------------------------------------------
// ae::InputPointer mouse tests
//------------------------------------------------------------------------------
TEST_CASE( "Each mouse button is a pointer from its press to its release", "[ae::InputPointer]" )
{
	ae::Input input;
	MouseEvent( &input, false, false, false, ae::Vec2( 1.0f, 1.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 0 );

	MouseEvent( &input, true, false, false, ae::Vec2( 1.0f, 1.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 1 );
	const ae::InputPointer* left = input.GetUnclaimedPointers()[ 0 ];
	REQUIRE( left->id != 0 );
	REQUIRE( left->button == ae::InputPointerButton::Primary );

	// A right click during the left drag is a separate pointer
	MouseEvent( &input, true, false, true, ae::Vec2( 2.0f, 3.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 2 );
	const ae::InputPointer* right = input.GetUnclaimedPointers()[ 1 ];
	REQUIRE( right->button == ae::InputPointerButton::Secondary );
	REQUIRE( left->Position() == ae::Vec2( 2.0f, 3.0f ) );
	REQUIRE( right->StartPosition() == ae::Vec2( 2.0f, 3.0f ) );

	MouseEvent( &input, true, false, false, ae::Vec2( 4.0f, 5.0f ) );
	REQUIRE( right->Ended() );
	REQUIRE( !left->Ended() );
	REQUIRE( right->Position() == ae::Vec2( 4.0f, 5.0f ) );

	MouseEvent( &input, false, false, false, ae::Vec2( 6.0f, 7.0f ) );
	REQUIRE( left->Ended() );
	REQUIRE( left->Position() == ae::Vec2( 6.0f, 7.0f ) );
	REQUIRE( left->StartDelta() == ae::Vec2( 5.0f, 6.0f ) );
	REQUIRE( right->Position() == ae::Vec2( 4.0f, 5.0f ) ); // Unchanged once ended

	MouseEvent( &input, false, false, false, ae::Vec2( 8.0f, 9.0f ) );
	REQUIRE( left->Position() == ae::Vec2( 6.0f, 7.0f ) );
	ReleaseAll( &input );
}

TEST_CASE( "Mouse buttons pressed together are a pointer each, oldest first", "[ae::InputPointer]" )
{
	ae::Input input;
	MouseEvent( &input, true, true, true, ae::Vec2( 1.0f, 1.0f ) );
	const ae::InputPointerArray pointers = input.GetUnclaimedPointers();
	REQUIRE( pointers.Length() == 3 );
	REQUIRE( pointers[ 0 ]->button == ae::InputPointerButton::Primary );
	REQUIRE( pointers[ 1 ]->button == ae::InputPointerButton::Secondary );
	REQUIRE( pointers[ 2 ]->button == ae::InputPointerButton::Tertiary );
	for( const ae::InputPointer* pointer : pointers ) { REQUIRE( pointer->source == ae::InputPointerSource::Mouse ); }
	MouseEvent( &input, false, false, false, ae::Vec2( 1.0f, 1.0f ) );
	for( const ae::InputPointer* pointer : pointers ) { REQUIRE( pointer->Ended() ); }
	ReleaseAll( &input );
}

TEST_CASE( "Mouse click within one frame is pumped already ended", "[ae::InputPointer]" )
{
	ae::Input input;
	MouseEvent( &input, true, false, false, ae::Vec2( 1.0f, 1.0f ) );
	MouseEvent( &input, false, false, false, ae::Vec2( 1.0f, 1.0f ) );
	const ae::InputPointer* pointer = input.PumpPointers();
	REQUIRE( pointer );
	REQUIRE( pointer->Ended() );
	REQUIRE( pointer->button == ae::InputPointerButton::Primary );
	input.ReleasePointer( pointer );
}

TEST_CASE( "A mouse button pointer released by its claimant is not remade mid-press", "[ae::InputPointer]" )
{
	ae::Input input;
	MouseEvent( &input, true, false, false, ae::Vec2( 1.0f, 1.0f ) );
	const ae::InputPointer* pointer = input.PumpPointers();
	REQUIRE( pointer );
	input.ReleasePointer( pointer );

	MouseEvent( &input, true, false, false, ae::Vec2( 2.0f, 2.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 0 );
	REQUIRE( input.GetPointers().Length() == 0 );

	// Other buttons are unaffected
	MouseEvent( &input, true, true, false, ae::Vec2( 3.0f, 3.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 1 );
	REQUIRE( input.GetUnclaimedPointers()[ 0 ]->button == ae::InputPointerButton::Tertiary );

	MouseEvent( &input, false, false, false, ae::Vec2( 3.0f, 3.0f ) );
	ReleaseAll( &input );
	MouseEvent( &input, true, false, false, ae::Vec2( 4.0f, 4.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 1 );
	REQUIRE( input.GetUnclaimedPointers()[ 0 ]->StartPosition() == ae::Vec2( 4.0f, 4.0f ) );
	ReleaseAll( &input );
}

TEST_CASE( "A pointer that returns to its start keeps its furthest distance", "[ae::InputPointer]" )
{
	ae::Input input;
	MouseEvent( &input, true, false, false, ae::Vec2( 1.0f, 1.0f ) );
	const ae::InputPointer* pointer = input.GetUnclaimedPointers()[ 0 ];
	REQUIRE( pointer->MaxStartDistance() == 0.0f );
	MouseEvent( &input, true, false, false, ae::Vec2( 4.0f, 5.0f ) );
	MouseEvent( &input, true, false, false, ae::Vec2( 2.0f, 1.0f ) );
	MouseEvent( &input, false, false, false, ae::Vec2( 1.0f, 1.0f ) );
	REQUIRE( pointer->StartDelta() == ae::Vec2( 0.0f ) );
	REQUIRE( pointer->MaxStartDistance() == 5.0f );
	ReleaseAll( &input );
}

TEST_CASE( "Ended unclaimed pointers are discarded on the next frame", "[ae::InputPointer]" )
{
	ae::Input input;
	MouseEvent( &input, true, false, false, ae::Vec2( 1.0f, 1.0f ) );
	MouseEvent( &input, false, false, false, ae::Vec2( 1.0f, 1.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 1 );
	input.m_TryNewFrame();
	REQUIRE( input.GetUnclaimedPointers().Length() == 0 );
}

//------------------------------------------------------------------------------
// ae::InputPointer id and limit tests
//------------------------------------------------------------------------------
TEST_CASE( "Pointer ids are unique across touches and the mouse", "[ae::InputPointer]" )
{
	ae::Input input;
	ae::InputPointer* touch = input.m_NewPointer( ae::Vec2( 0.0f ) );
	REQUIRE( touch );
	touch->source = ae::InputPointerSource::Touch;
	touch->_platformId = 1;
	MouseEvent( &input, true, false, false, ae::Vec2( 1.0f, 1.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 2 );
	const ae::InputPointer* mouse = input.GetUnclaimedPointers()[ 1 ];
	REQUIRE( mouse->id != touch->id );
	REQUIRE( input.m_FindPointer( touch->id ) == touch );
	REQUIRE( input.m_FindPointer( mouse->id ) == mouse );
	REQUIRE( input.m_FindPointer( 0 ) == nullptr );
	MouseEvent( &input, false, false, false, ae::Vec2( 1.0f, 1.0f ) );
	touch->ended = true;
	ReleaseAll( &input );
}

TEST_CASE( "Pointers are discarded while claimed pointers fill every slot", "[ae::InputPointer]" )
{
	ae::Input input;
	for( uint32_t i = 0; i < ae::kMaxInputPointers; i++ )
	{
		REQUIRE( input.m_NewPointer( ae::Vec2( 0.0f ) ) );
		REQUIRE( input.PumpPointers() );
	}
	REQUIRE( !input.m_NewPointer( ae::Vec2( 0.0f ) ) );

	// A press that couldn't make a pointer doesn't make one once space frees up
	MouseEvent( &input, true, false, false, ae::Vec2( 1.0f, 1.0f ) );
	input.ReleasePointer( input.GetPointers()[ 0 ] );
	MouseEvent( &input, true, false, false, ae::Vec2( 2.0f, 2.0f ) );
	REQUIRE( input.GetUnclaimedPointers().Length() == 0 );
	MouseEvent( &input, false, false, false, ae::Vec2( 2.0f, 2.0f ) );
	ReleaseAll( &input );
}
