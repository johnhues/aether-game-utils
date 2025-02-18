//------------------------------------------------------------------------------
// 25_AStar.cpp
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

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
using AStarNode = ae::AStarNode<>;
const ae::Tag TAG_EXAMPLE = ae::Tag( "example" );
const float kNodeRadius = 0.05f;
const float kPathMaxRadius = 0.8f;
const float kStartRadius = 0.9f;

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
void BuildGraph( ae::Array< AStarNode >& nodes, ae::Array< const AStarNode* >& goals )
{
	nodes.Clear();
	goals.Clear();

	const uint32_t nodeCount = 12;
	const uint32_t goalCount = 3;
	const uint32_t neighborCount = 3;
	for( uint32_t i = 0; i < nodeCount; i++ )
	{
		const float angle = ae::Random( 0.0f, ae::TWO_PI );
		float t = ae::Sqrt( ae::Random( 0.0f, 1.0f ) ) * kPathMaxRadius;
		AStarNode& node = nodes.Append( AStarNode( TAG_EXAMPLE ) );
		node.pos = ae::Vec3( ae::Cos( angle ) * t, ae::Sin( angle ) * t, 0.0f );
	}
	for( uint32_t i = 0; i < goalCount; i++ )
	{
		goals.Append( &nodes[ ae::Random( 0, nodes.Length() - 1 ) ] );
	}
	auto Pair = []( AStarNode* a, AStarNode* b )
	{
		if( a == b ) { return; }
		if( a->next.Find( b ) >= 0 ) { return; }
		AE_ASSERT( b->next.Find( a ) < 0 );
		a->next.Append( b );
		b->next.Append( a );
	};
	for( AStarNode& node : nodes )
	{
		struct AStarNodePair
		{
			AStarNode* node;
			float dist;
		};
		ae::Array< AStarNodePair, nodeCount > neighbors;
		for( AStarNode& neighbor : nodes )
		{
			if( &node == &neighbor ) { continue; }
			neighbors.Append( { &neighbor, ( node.pos - neighbor.pos ).Length() } );
		}
		std::partial_sort( neighbors.begin(), neighbors.begin() + neighborCount, neighbors.end(), []( const AStarNodePair& a, const AStarNodePair& b ) { return a.dist < b.dist; } );
		for( uint32_t i = 0; i < neighborCount; i++ )
		{
			Pair( &node, neighbors[ i ].node );
		}
	}
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	// Setup
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::DebugLines debugLines = TAG_EXAMPLE;
	ae::TimeStep timeStep;
	window.Initialize( 1280, 1280, false, true, true );
	window.SetTitle( "A*: Press arrow keys to move, space to randomize." );
	render.Initialize( &window );
	input.Initialize( &window );
	debugLines.Initialize( 2048 );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	auto DrawLine = [&]( const ae::Vec3& a, const ae::Vec3& b, const ae::Color& color )
	{
		ae::Vec3 dir = ( b - a );
		const float dist = dir.SafeNormalize();
		debugLines.AddLine( a + dir * kNodeRadius, a + dir * ( dist - kNodeRadius ), color );
	};

	// State
	float startTheta = 0.0f;
	ae::Array< AStarNode > nodes = TAG_EXAMPLE;
	ae::Array< const AStarNode* > goals = TAG_EXAMPLE;

	BuildGraph( nodes, goals );
	auto Update = [&]() -> bool
	{
		// Input
		input.Pump();
		if( input.GetPress( ae::Key::Space ) ) { BuildGraph( nodes, goals ); }
		if( input.Get( ae::Key::Left ) ) { startTheta -= timeStep.GetDt(); }
		if( input.Get( ae::Key::Right ) ) { startTheta += timeStep.GetDt(); }

		// Start position
		const ae::Vec3 startPos( ae::Cos( startTheta ) * kStartRadius, ae::Sin( startTheta ) * kStartRadius, 0.0f );
		const AStarNode* startNode = [&]()
		{
			float minDist = ae::MaxValue< float >();
			AStarNode* closest = nullptr;
			for( AStarNode& node : nodes )
			{
				const float dist = ( node.pos - startPos ).Length();
				if( dist < minDist )
				{
					minDist = dist;
					closest = &node;
				}
			}
			return closest;
		}();

		// Generate path
		const AStarNode* path[ 32 ];
		const uint32_t pathLength = ae::AStar( startNode, nodes.Data(), nodes.Length(), goals.Data(), goals.Length(), path, countof( path ) );

		render.Activate();
		render.Clear( ae::Color::AetherBlack() );

		// Draw graph and path
		debugLines.AddCircle( startPos, ae::Vec3( 0.0f, 0.0f, 1.0f ), kNodeRadius, ( pathLength ? ae::Color::AetherGreen() : ae::Color::AetherBlue() ), 32 );
		DrawLine( startPos, startNode->pos, ( pathLength ? ae::Color::AetherGreen() : ae::Color::AetherBlue() ) );
		for( const AStarNode& node : nodes )
		{
			ae::Color color = ae::Color::AetherGray();
			if( std::find( path, path + pathLength, &node ) != ( path + pathLength ) )
			{
				color = ae::Color::AetherGreen();
			}
			if( goals.Find( &node ) >= 0 )
			{
				debugLines.AddCircle( node.pos, ae::Vec3( 0.0f, 0.0f, 1.0f ), kNodeRadius * 0.7f, color, 32 );
			}
			debugLines.AddCircle( node.pos, ae::Vec3( 0.0f, 0.0f, 1.0f ), kNodeRadius, color, 32 );
			for( const AStarNode* next : node.next )
			{
				if( next > &node )
				{
					DrawLine( node.pos, next->pos, ae::Color::AetherGray() );
				}
			}
		}
		for( uint32_t i = 1; i < pathLength; i++ )
		{
			DrawLine( path[ i - 1 ]->pos, path[ i ]->pos, ae::Color::AetherGreen() );
		}

		const ae::Matrix4 worldToNDC = ae::Matrix4::Scaling( ( render.GetAspectRatio() > 1.0f ) ? ae::Vec3( 1.0f / render.GetAspectRatio(), 1.0f, 1.0f ) : ae::Vec3( 1.0f, render.GetAspectRatio(), 1.0f ) );
		debugLines.Render( worldToNDC );
		render.Present();
		timeStep.Tick();
		
		return !input.quit;
	};

#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while ( Update() ) {}
#endif
	return 0;
}
