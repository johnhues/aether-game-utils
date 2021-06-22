//------------------------------------------------------------------------------
// 07_Cube.cpp
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
#include "ae/aether.h"
//#include "ae/aetherEXT.h"

//------------------------------------------------------------------------------
// Shaders
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
//const char* kVertShader = "\
//	AE_UNIFORM mat4 u_worldToProj;\
//	AE_IN_HIGHP vec4 a_position;\
//	AE_IN_HIGHP vec4 a_color;\
//	AE_OUT_HIGHP vec4 v_color;\
//	AE_OUT_HIGHP vec4 v_pos;\
//	void main()\
//	{\
//		v_color = a_color;\
//		v_pos = u_worldToProj * a_position;\
//		gl_Position = v_pos;\
//	}";
//
//const char* kFragShader = "\
//	AE_IN_HIGHP vec4 v_color;\
//	AE_IN_HIGHP vec4 v_pos;\
//	layout(location = 0) out vec4 diffuseColor;\
//	layout(location = 1) out vec4 depth;\
//	void main()\
//	{\
//		diffuseColor = v_color;\
//		depth = vec4( vec3( v_pos.z / v_pos.w ), 1.0 );\
//	}";

//------------------------------------------------------------------------------
// Cube
//------------------------------------------------------------------------------
struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 color;
};

Vertex kCubeVerts[] =
{
	{ ae::Vec4( -0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::PicoRed().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::PicoOrange().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::PicoYellow().GetLinearRGBA() },
	{ ae::Vec4( -0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::PicoPeach().GetLinearRGBA() },
	{ ae::Vec4( -0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::PicoGreen().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::PicoPeach().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::PicoPink().GetLinearRGBA() },
	{ ae::Vec4( -0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA() },
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

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::Shader shader;
	ae::VertexData vertexData;
//	aeEditorCamera camera;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "cube" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	shader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );

	vertexData.Initialize( sizeof( *kCubeVerts ), sizeof( *kCubeIndices ), countof( kCubeVerts ), countof( kCubeIndices ), ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Static, ae::VertexData::Usage::Static );
	vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( Vertex, pos ) );
	vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof( Vertex, color ) );
	vertexData.SetVertices( kCubeVerts, countof( kCubeVerts ) );
	vertexData.SetIndices( kCubeIndices, countof( kCubeIndices ) );

	AE_INFO( "Run" );
//	while ( !input.GetState()->exit )
  while ( !input.quit )
	{
		input.Pump();

//		if ( !input.GetPrevState()->Get( aeKey::F ) && input.GetState()->Get( aeKey::F ) )
//		{
//			camera.Refocus( ae::Vec3( 0.0f ) );
//		}
//		camera.Update( &input, timeStep.GetTimeStep() );

		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		ae::UniformList uniformList;
		//ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( ae::Vec3( 0.0f, 5.0f, 5.0f ), -ae::Vec3( 0.0f, 5.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.6f, render.GetAspectRatio(), 0.25f, 50.0f );
		uniformList.Set( "u_worldToProj", worldToView * viewToProj );
		vertexData.Render( &shader, uniformList );
		render.Present();

		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	//input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
