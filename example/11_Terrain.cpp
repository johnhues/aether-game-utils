//------------------------------------------------------------------------------
// 11_Terrain.cpp
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
#include "aeClock.h"
#include "aeEditorCamera.h"
#include "aeInput.h"
#include "aeLog.h"
#include "aeRender.h"
#include "aeTerrain.h"
#include "aeWindow.h"

//------------------------------------------------------------------------------
// Cube Shader
//------------------------------------------------------------------------------
const char* kVertShader = "\
	AE_UNIFORM mat4 u_worldToProj;\
	AE_IN_HIGHP vec4 a_position;\
	AE_IN_HIGHP vec4 a_color;\
	AE_OUT_HIGHP vec4 v_color;\
	void main()\
	{\
		v_color = a_color;\
		gl_Position = u_worldToProj * a_position;\
	}";

const char* kFragShader = "\
	AE_IN_HIGHP vec4 v_color;\
	void main()\
	{\
		AE_COLOR = v_color;\
	}";

//------------------------------------------------------------------------------
// Terrain Shader
//------------------------------------------------------------------------------
const char* kTerrainVertShader = "\
	AE_UNIFORM mat4 u_worldToProj;\
	AE_IN_HIGHP vec3 a_position;\
	AE_IN_HIGHP vec3 a_normal;\
	AE_OUT_HIGHP vec4 v_color;\
	void main()\
	{\
		v_color = vec4( a_normal, 1.0 );\
		gl_Position = u_worldToProj * vec4( a_position, 1.0 );\
	}";

const char* kTerrainFragShader = "\
	AE_IN_HIGHP vec4 v_color;\
	void main()\
	{\
		AE_COLOR = v_color;\
	}";

//------------------------------------------------------------------------------
// Grid Shader
//------------------------------------------------------------------------------
const char* kGridVertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_screenToWorld;\
    AE_IN_HIGHP vec4 a_position;\
    AE_OUT_HIGHP vec3 v_worldPos;\
    void main()\
    {\
      v_worldPos = vec3( u_screenToWorld * a_position );\
      gl_Position = a_position;\
    }";

const char* kGridFragStr = "\
    AE_IN_HIGHP vec3 v_worldPos;\
    void main()\
    {\
      int x = int( floor( v_worldPos.x ) ) % 2;\
      int y = int( floor( v_worldPos.y ) ) % 2;\
      AE_COLOR.rgb = mix( vec3( 0.3 ), vec3( 0.35 ), int( x != y ) );\
      float gridX = mod( v_worldPos.x + 16.0, 32.0 ) - 16.0;\
      float gridY = mod( v_worldPos.y + 16.0, 32.0 ) - 16.0;\
      if ( abs( gridX ) < 0.05 || abs( gridY ) < 0.05 ) { AE_COLOR.rgb = vec3( 0.25 ); } \
      AE_COLOR.a = 1.0;\
    }";

class Grid
{
public:
	void Initialize()
	{
		struct BGVertex
		{
			aeFloat4 pos;
		};
		
		BGVertex bgVertices[ aeQuadVertCount ];
		uint8_t bgIndices[ aeQuadIndexCount ];
		for ( uint32_t i = 0; i < aeQuadVertCount; i++ )
		{
			bgVertices[ i ].pos = aeFloat4( aeQuadVertPos[ i ] * 2.0f, 1.0f );
		}
		for ( uint32_t i = 0; i < aeQuadIndexCount; i++ )
		{
			bgIndices[ i ] = aeQuadIndices[ i ];
		}

		m_bgVertexData.Initialize( sizeof( BGVertex ), sizeof( uint8_t ), countof( bgVertices ), countof( bgIndices ), aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
		m_bgVertexData.AddAttribute( "a_position", 4, aeVertexDataType::Float, offsetof( BGVertex, pos ) );
		m_bgVertexData.SetVertices( bgVertices, countof( bgVertices ) );
		m_bgVertexData.SetIndices( bgIndices, countof( bgIndices ) );

		m_gridShader.Initialize( kGridVertexStr, kGridFragStr, nullptr, 0 );
	}

	void Render( aeFloat4x4 worldToProj )
	{
		aeUniformList uniforms;
		uniforms.Set( "u_screenToWorld", worldToProj.Inverse() );
		m_bgVertexData.Render( &m_gridShader, uniforms );
	}

private:
	aeShader m_gridShader;
	aeVertexData m_bgVertexData;
};

//------------------------------------------------------------------------------
// Cube
//------------------------------------------------------------------------------
struct Vertex
{
	aeFloat4 pos;
	aeFloat4 color;
};

Vertex kCubeVerts[] =
{
	{ aeFloat4( -0.5f, -0.5f, -0.5f, 1.0f ), aeColor::PicoRed().GetLinearRGBA() },
	{ aeFloat4( 0.5f, -0.5f, -0.5f, 1.0f ), aeColor::PicoOrange().GetLinearRGBA() },
	{ aeFloat4( 0.5f, 0.5f, -0.5f, 1.0f ), aeColor::PicoYellow().GetLinearRGBA() },
	{ aeFloat4( -0.5f, 0.5f, -0.5f, 1.0f ), aeColor::PicoPeach().GetLinearRGBA() },
	{ aeFloat4( -0.5f, -0.5f, 0.5f, 1.0f ), aeColor::PicoGreen().GetLinearRGBA() },
	{ aeFloat4( 0.5f, -0.5f, 0.5f, 1.0f ), aeColor::PicoPeach().GetLinearRGBA() },
	{ aeFloat4( 0.5f, 0.5f, 0.5f, 1.0f ), aeColor::PicoPink().GetLinearRGBA() },
	{ aeFloat4( -0.5f, 0.5f, 0.5f, 1.0f ), aeColor::PicoBlue().GetLinearRGBA() },
};

uint16_t kCubeIndices[] =
{
	4, 5, 6, 4, 6, 7, // Top
	0, 3, 1, 1, 3, 2, // Bottom
	0, 4, 3, 3, 4, 7, // Left
	1, 2, 6, 6, 5, 1, // Right
	6, 2, 3, 6, 3, 7, // Back
	0, 1, 5, 0, 5, 4 // Front
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_INFO( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	aeFixedTimeStep timeStep;
	aeShader shader, terrainShader;
	aeVertexData vertexData;
	aeEditorCamera camera;
	//Grid grid;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "terrain" );
	render.InitializeOpenGL( &window );
	render.SetClearColor( aeColor::PicoDarkPurple() );
	input.Initialize( &window, &render );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	//grid.Initialize();

	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	shader.SetCulling( aeShaderCulling::CounterclockwiseFront );

	terrainShader.Initialize( kTerrainVertShader, kTerrainFragShader, nullptr, 0 );
	terrainShader.SetDepthTest( true );
	terrainShader.SetDepthWrite( true );
	terrainShader.SetCulling( aeShaderCulling::CounterclockwiseFront );

	vertexData.Initialize( sizeof( *kCubeVerts ), sizeof( *kCubeIndices ), countof( kCubeVerts ), countof( kCubeIndices ), aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
	vertexData.AddAttribute( "a_position", 4, aeVertexDataType::Float, offsetof( Vertex, pos ) );
	vertexData.AddAttribute( "a_color", 4, aeVertexDataType::Float, offsetof( Vertex, color ) );
	vertexData.SetVertices( kCubeVerts, countof( kCubeVerts ) );
	vertexData.SetIndices( kCubeIndices, countof( kCubeIndices ) );

	aeTerrain* terrain = aeAlloc::Allocate< aeTerrain >();
	terrain->Initialize();

	AE_INFO( "Run" );
	while ( !input.GetState()->exit )
	{
		input.Pump();

		terrain->Update();

		if ( !input.GetPrevState()->Get( aeKey::F ) && input.GetState()->Get( aeKey::F ) )
		{
			camera.Refocus( aeFloat3( 0.0f ) );
		}
		camera.Update( &input, timeStep.GetTimeStep() );

		render.StartFrame( window.GetWidth(), window.GetHeight() );

		
		aeUniformList uniformList;
		aeFloat4x4 worldToView = aeFloat4x4::WorldToView( camera.GetPosition(), camera.GetForward(), aeFloat3( 0.0f, 0.0f, 1.0f ) );
		aeFloat4x4 viewToProj = aeFloat4x4::ViewToProjection( 0.6f, render.GetAspectRatio(), 0.25f, 50.0f );
		aeFloat4x4 worldToProj = viewToProj * worldToView;

		//grid.Render( worldToProj );

		uniformList.Set( "u_worldToProj", viewToProj * worldToView );
		vertexData.Render( &shader, uniformList );

		terrain->Render( aeFloat3( 0.0f ), &terrainShader, uniformList );

		render.EndFrame();

		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
