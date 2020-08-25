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
#include "aeSpline.h"
#include "aeTerrain.h"
#include "aeWindow.h"
#include "aeVfs.h"

//------------------------------------------------------------------------------
// Terrain Shader
//------------------------------------------------------------------------------
const char* kTerrainVertShader = "\
	AE_UNIFORM mat4 u_worldToProj;\
	AE_UNIFORM vec3 u_topColor;\
	AE_UNIFORM vec3 u_sideColor;\
	AE_IN_HIGHP vec3 a_position;\
	AE_IN_HIGHP vec3 a_normal;\
	AE_OUT_HIGHP vec3 v_color;\
	AE_OUT_HIGHP vec3 v_normal;\
	void main()\
	{\
		float top = max(0.0, a_normal.z);\
		top *= top;\
		top *= top;\
		v_color = mix(u_sideColor, u_topColor, top);\
		v_normal = a_normal;\
		gl_Position = u_worldToProj * vec4( a_position, 1.0 );\
	}";

const char* kTerrainFragShader = "\
	AE_IN_HIGHP vec3 v_color;\
	AE_IN_HIGHP vec3 v_normal;\
	void main()\
	{\
		float light = dot( normalize( v_normal ), normalize( vec3( 1.0 ) ) );\
		light = max(0.0, light);\
		light = mix( 0.8, 4.0, light );\
		AE_COLOR = vec4( AE_RGB_TO_SRGB( v_color * vec3( light ) ), 1.0 );\
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
// Main
//------------------------------------------------------------------------------
int main()
{
	{
	AE_INFO( "Initialize" );

	bool headless = _AE_LINUX_;

	aeWindow window;
	aeRender render;
	aeInput input;
	aeDebugRender debug;
	aeFixedTimeStep timeStep;
	aeShader terrainShader;
	aeEditorCamera camera;
	////Grid grid;
	ae::Image terrainHeightMap;

	if ( !headless )
	{
		window.Initialize( 800, 600, false, true );
		window.SetTitle( "terrain" );
		render.InitializeOpenGL( &window );
		render.SetClearColor( aeColor::PicoDarkPurple() );
		debug.Initialize();
	}

	input.Initialize( headless ? nullptr : &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	////grid.Initialize();
	camera.SetPosition( aeFloat3( 150.0f, 150.f, 60.0f ) );

	if ( !headless )
	{
		terrainShader.Initialize( kTerrainVertShader, kTerrainFragShader, nullptr, 0 );
		terrainShader.SetDepthTest( true );
		terrainShader.SetDepthWrite( true );
		terrainShader.SetCulling( aeShaderCulling::CounterclockwiseFront );
	}

	{
		aeAlloc::Scratch< uint8_t > fileBuffer( aeVfs::GetSize( "terrain.png" ) );
		if ( aeVfs::Read( "terrain.png", fileBuffer.Data(), fileBuffer.Length() ) )
		{
			terrainHeightMap.LoadFile( fileBuffer.Data(), fileBuffer.Length(), ae::Image::Extension::PNG, ae::Image::Format::R );
		}
	}

	uint32_t terrainThreads = aeMath::Max( 1u, (uint32_t)( aeGetMaxConcurrentThreads() * 0.75f ) );
	aeTerrain* terrain = aeAlloc::Allocate< aeTerrain >();
	terrain->Initialize( terrainThreads, !headless );

	struct TerrainGen
	{
		ae::Image* heightMap;
		aeSpline spline;
	} terrainGen;

	terrainGen.heightMap = &terrainHeightMap;

	aeFloat3 splineCenter( 100.0f, 100.0f, 40.0f );
	for ( uint32_t i = 0; i < 5; i++ )
	{
		float angle = aeMath::TWO_PI * i / 5.0f; 
		aeFloat3 p( aeMath::Cos( angle ), aeMath::Sin( angle ), 0.0f );
		p *= 12.0f;
		p += splineCenter;
		terrainGen.spline.AppendControlPoint( p );
	}
	terrainGen.spline.SetLooping( true );

	terrain->SetCallback( &terrainGen, []( void* userdata, aeFloat3 p )
	{
		TerrainGen* terrain = (TerrainGen*)userdata;

		float height = p.z - terrain->heightMap->Get( p.GetXY() * 5.0f, ae::Image::Interpolation::Cosine ).r * 20.0f;
		float sphere = ( p - aeFloat3( 100.0f, 100.0f, 40.0f ) ).Length() - 7.0f;
		
		float spline = aeMath::MaxValue< float >();
		if ( terrain->spline.GetAABB().GetMinDistance( p ) <= 2.5f )
		{
			spline = terrain->spline.GetMinDistance( p ) - 2.5f;
		}

		return aeMath::Min( height, sphere, spline );
	} );

	AE_INFO( "Run" );
	while ( !input.GetState()->exit )
	{
		input.Pump();

		terrain->Update( camera.GetPosition(), 1250.0f );

		RaycastResult result = terrain->Raycast( camera.GetPosition(), camera.GetForward() * 1000.0f );

		camera.Update( &input, timeStep.GetTimeStep() );
		if ( result.hit && !input.GetPrevState()->Get( aeKey::F ) && input.GetState()->Get( aeKey::F ) )
		{
			camera.Refocus( result.posf );
		}

		if ( !headless )
		{
			render.StartFrame( window.GetWidth(), window.GetHeight() );

			aeFloat4x4 worldToView = aeFloat4x4::WorldToView( camera.GetPosition(), camera.GetForward(), aeFloat3( 0.0f, 0.0f, 1.0f ) );
			aeFloat4x4 viewToProj = aeFloat4x4::ViewToProjection( 0.4f, render.GetAspectRatio(), 0.5f, 1000.0f );
			aeFloat4x4 worldToProj = viewToProj * worldToView;

			//grid.Render( worldToProj );

			aeColor top = aeColor::PS( 46, 65, 35 );
			aeColor side = aeColor::PS( 84, 84, 74 );
			aeUniformList uniformList;
			uniformList.Set( "u_worldToProj", worldToProj );
			uniformList.Set( "u_topColor", top.GetLinearRGB() );
			uniformList.Set( "u_sideColor", side.GetLinearRGB() );
			terrain->Render( &terrainShader, uniformList );

			if ( result.hit )
			{
				debug.AddLine( result.posf, result.posf + result.normal, aeColor::Red() );
				debug.AddSphere( result.posf, 0.5f, aeColor::Red(), 24 );
			}

			for ( uint32_t i = 0; i < terrainGen.spline.GetControlPointCount(); i++ )
			{
				debug.AddSphere( terrainGen.spline.GetControlPoint( i ), 0.5f, aeColor::Red(), 24 );
			}

			debug.Render( worldToProj );

			render.EndFrame();
		}

			timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	terrain->Terminate();
	aeAlloc::Release( terrain );
	input.Terminate();
	if ( !headless )
	{
		render.Terminate();
		window.Terminate();
	}
}
	return 0;
}
