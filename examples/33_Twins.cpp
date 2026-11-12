//------------------------------------------------------------------------------
// 33_Twins.cpp
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
// Two instances of the same wasm module on one page, each drawing its own
// canvas with its own camera. Dragging a ball in either canvas moves it in
// both, and each draws where its twin's camera is looking from.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// Twin calls
//------------------------------------------------------------------------------
// Provided by the page as the 'twin' import module, see 33_Twins.html.
// TwinCall() copies \p size bytes into the twin instance's memory and calls
// \p fn there with them. Function pointers are table indices, identical in
// both instances.
extern "C" __attribute__(( import_module( "twin" ), import_name( "Call" ) )) void _TwinCall( void* fn, const void* args, uint32_t size );
extern "C" __attribute__(( import_module( "twin" ), import_name( "Index" ) )) int32_t _TwinIndex();

// Called by the twin instance's page
extern "C" __attribute__(( export_name( "TwinInvoke" ) )) void TwinInvoke( void* fn, const void* args )
{
	( (void(*)( const void* ))fn )( args );
}
extern "C" __attribute__(( export_name( "TwinAlloc" ) )) void* TwinAlloc( uint32_t size ) { return malloc( size ); }
extern "C" __attribute__(( export_name( "TwinFree" ) )) void TwinFree( void* p ) { free( p ); }

template< auto Fn, typename T >
void _TwinTrampoline( const void* data )
{
	T msg;
	memcpy( &msg, data, sizeof( T ) );
	Fn( msg );
}

//! Calls \p Fn with a copy of \p msg in the twin instance before returning.
//! Does nothing until the page links the twins.
template< auto Fn, typename T >
void TwinCall( const T& msg )
{
	static_assert( std::is_trivially_copyable_v< T >, "Twin messages are copied between heaps" );
	_TwinCall( (void*)&_TwinTrampoline< Fn, T >, &msg, sizeof( T ) );
}

//------------------------------------------------------------------------------
// Scene
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";
const float kYFov = ae::QUARTER_PI;
const float kBallRadius = 0.5f;
const uint32_t kBallCount = 5;
const ae::Color kBallColors[ kBallCount ] = {
	ae::Color::AetherRed(),
	ae::Color::AetherOrange(),
	ae::Color::AetherGreen(),
	ae::Color::AetherBlue(),
	ae::Color::AetherYellow(),
};

struct BallMoved
{
	uint32_t index;
	ae::Vec3 position;
};

struct TwinCamera
{
	ae::Vec3 position;
	ae::Vec3 forward;
	ae::Vec3 right;
	ae::Vec3 up;
	float aspect;
};

ae::Vec3 g_balls[ kBallCount ];
ae::Optional< TwinCamera > g_twinCamera;

void OnBallMoved( const BallMoved& msg )
{
	if( msg.index < kBallCount )
	{
		g_balls[ msg.index ] = msg.position;
	}
}

void OnTwinCamera( const TwinCamera& msg )
{
	g_twinCamera = msg;
}

// Direction from the camera through \p pos, in window units
ae::Vec3 GetPointerRay( const ae::DebugCamera& camera, ae::Vec2 pos, ae::Vec2 size )
{
	const ae::Vec2 ndc = ( pos / size ) * 2.0f - ae::Vec2( 1.0f );
	const float tanY = ae::Tan( kYFov * 0.5f );
	const float tanX = tanY * ( size.x / size.y );
	const ae::Vec3 ray = camera.GetForward() + camera.GetRight() * ( ndc.x * tanX ) + camera.GetUp() * ( ndc.y * tanY );
	return ray.NormalizeCopy();
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	const int32_t twinIndex = _TwinIndex();
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::DebugLines debug = TAG_EXAMPLE;
	ae::DebugCamera camera = ae::Axis::Z;
	const ae::InputPointer* dragPointer = nullptr;
	uint32_t dragBall = 0;
	ae::Vec3 dragOffset( 0.0f );
	const ae::Plane ground( ae::Vec3( 0.0f, 0.0f, kBallRadius ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );

	auto Initialize = [ & ]()
	{
		window.Initialize( 640, 480, false, true, "" );
		render.Initialize( &window );
		input.Initialize( &window );
		timeStep.SetTimeStep( 1.0f / 60.0f );
		debug.Initialize( 8192 );
		camera.SetModifierKey( {} );
		camera.Reset( ae::Vec3( 0.0f ), twinIndex ? ae::Vec3( -2.0f, -0.5f, 12.0f ) : ae::Vec3( 7.0f, -7.0f, 5.0f ) );
		for( uint32_t i = 0; i < kBallCount; i++ )
		{
			const float angle = ae::TWO_PI * i / kBallCount;
			g_balls[ i ] = ae::Vec3( ae::Cos( angle ) * 3.0f, ae::Sin( angle ) * 3.0f, kBallRadius );
		}
		return true;
	};

	auto Update = [ & ]() -> bool
	{
		const float dt = timeStep.GetDt();
		input.Pump();
		const ae::Vec2 windowSize( (float)window.GetWidth(), (float)window.GetHeight() );

		// Claim pointers that start on a ball before the camera sees them
		for( const ae::InputPointer* pointer : input.GetUnclaimedPointers() )
		{
			if( dragPointer || pointer->Ended() || pointer->button != ae::InputPointerButton::Primary )
			{
				continue;
			}
			const ae::Vec3 ray = GetPointerRay( camera, pointer->StartPosition(), windowSize );
			float closest = ae::MaxValue< float >();
			for( uint32_t i = 0; i < kBallCount; i++ )
			{
				ae::Vec3 hit;
				float distance = 0.0f;
				if( ae::Sphere( g_balls[ i ], kBallRadius ).Raycast( camera.GetPosition(), ray * 100.0f, &hit, nullptr, &distance ) && distance < closest )
				{
					closest = distance;
					dragBall = i;
				}
			}
			ae::Vec3 groundHit;
			if( closest < ae::MaxValue< float >() && ground.Raycast( camera.GetPosition(), ray * 100.0f, &groundHit ) && input.ClaimPointer( pointer ) )
			{
				dragPointer = pointer;
				dragOffset = g_balls[ dragBall ] - groundHit;
			}
		}
		if( dragPointer )
		{
			const ae::Vec3 ray = GetPointerRay( camera, dragPointer->Position(), windowSize );
			ae::Vec3 groundHit;
			if( ground.Raycast( camera.GetPosition(), ray * 100.0f, &groundHit ) )
			{
				g_balls[ dragBall ] = groundHit + dragOffset;
				TwinCall< OnBallMoved >( BallMoved{ dragBall, g_balls[ dragBall ] } );
			}
			if( dragPointer->Ended() )
			{
				input.ReleasePointer( dragPointer );
				dragPointer = nullptr;
			}
		}
		camera.Update( { .input = &input, .dt = dt, .yFov = kYFov, .viewport = window.GetArea() } );
		TwinCall< OnTwinCamera >( TwinCamera{ camera.GetPosition(), camera.GetForward(), camera.GetRight(), camera.GetUp(), render.GetAspectRatio() } );

		// Grid
		for( int32_t i = -5; i <= 5; i++ )
		{
			const ae::Color color = i ? ae::Color::AetherDarkBlue() : ae::Color::AetherBlue();
			debug.AddLine( ae::Vec3( (float)i, -5.0f, 0.0f ), ae::Vec3( (float)i, 5.0f, 0.0f ), color );
			debug.AddLine( ae::Vec3( -5.0f, (float)i, 0.0f ), ae::Vec3( 5.0f, (float)i, 0.0f ), color );
		}
		// Balls
		for( uint32_t i = 0; i < kBallCount; i++ )
		{
			const bool dragging = dragPointer && dragBall == i;
			debug.AddSphere( g_balls[ i ], dragging ? kBallRadius * 1.1f : kBallRadius, kBallColors[ i ], 16 );
			debug.AddLine( g_balls[ i ], ae::Vec3( g_balls[ i ].x, g_balls[ i ].y, 0.0f ), kBallColors[ i ] );
		}
		// Twin camera, as its position and where the corners of its view meet the floor
		if( const TwinCamera* twin = g_twinCamera.TryGet() )
		{
			const ae::Plane floor( ae::Vec3( 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
			const float tanY = ae::Tan( kYFov * 0.5f );
			const ae::Vec3 x = twin->right * ( tanY * twin->aspect );
			const ae::Vec3 y = twin->up * tanY;
			const ae::Vec3 cornerRays[ 4 ] = {
				twin->forward + x + y,
				twin->forward - x + y,
				twin->forward - x - y,
				twin->forward + x - y,
			};
			ae::Vec3 floorHits[ 4 ];
			bool hasFloorHit[ 4 ];
			for( uint32_t i = 0; i < 4; i++ )
			{
				hasFloorHit[ i ] = floor.Raycast( twin->position, cornerRays[ i ] * 100.0f, &floorHits[ i ] );
			}
			const ae::Color color = ae::Color::AetherWhite();
			debug.AddSphere( twin->position, 0.15f, color, 8 );
			for( uint32_t i = 0; i < 4; i++ )
			{
				const uint32_t next = ( i + 1 ) % 4;
				debug.AddLine( twin->position, hasFloorHit[ i ] ? floorHits[ i ] : twin->position + cornerRays[ i ], color );
				if( hasFloorHit[ i ] && hasFloorHit[ next ] )
				{
					debug.AddLine( floorHits[ i ], floorHits[ next ], color );
				}
			}
		}

		render.Activate();
		render.Clear( ae::Color::AetherBlack() );
		const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
		const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( kYFov, render.GetAspectRatio(), 0.25f, 100.0f );
		debug.Render( viewToProj * worldToView );
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};

	auto Terminate = [ & ]() -> int32_t
	{
		debug.Terminate();
		input.Terminate();
		render.Terminate();
		window.Terminate();
		return 0;
	};
	return ae::Application( argc, argv, {
		.initialize = Initialize,
		.update = Update,
		.terminate = Terminate,
	} );
}
