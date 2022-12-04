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
#include "aether.h"

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_worldToProj;
	AE_UNIFORM vec4 u_color;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_color;
	AE_OUT_HIGHP vec4 v_color;
	void main()
	{
		v_color = a_color * u_color;
		gl_Position = u_worldToProj * a_position;
	})";

const char* kFragShader = R"(
	AE_IN_HIGHP vec4 v_color;
	void main()
	{
		AE_COLOR = v_color;
	})";

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
// Example
//------------------------------------------------------------------------------
struct Example
{
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::Shader shader;
	ae::VertexArray vertexData;
	float r0 = 0.0f;
	float r1 = 0.0f;

	void Initialize()
	{
		AE_INFO( "Initialize" );

		window.Initialize( 800, 600, false, true );
		window.SetTitle( "cube" );
		render.Initialize( &window );
		input.Initialize( &window );
		timeStep.SetTimeStep( 1.0f / 60.0f );

		shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
		shader.SetDepthTest( true );
		shader.SetDepthWrite( true );
		shader.SetBlending( true );
		shader.SetCulling( ae::Culling::CounterclockwiseFront );

		vertexData.Initialize( sizeof( *kCubeVerts ), sizeof( *kCubeIndices ), countof( kCubeVerts ), countof( kCubeIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
		vertexData.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
		vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
		vertexData.SetVertices( kCubeVerts, countof( kCubeVerts ) );
		vertexData.SetIndices( kCubeIndices, countof( kCubeIndices ) );
		vertexData.Upload();
	}

	bool Tick()
	{
		input.Pump();
		
		r0 += timeStep.GetDt() * 0.6f;
		r1 += timeStep.GetDt() * 0.75f;

		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		ae::UniformList uniformList;
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( ae::Vec3( 0.0f, 3.5f, -0.4f ), -ae::Vec3( 0.0f, 3.5f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.25f, 50.0f );
		
		// Cube
		ae::Matrix4 modelToWorld = ae::Matrix4::RotationX( r0 ) * ae::Matrix4::RotationZ( r1 );
		uniformList.Set( "u_worldToProj", viewToProj * worldToView * modelToWorld );
		uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
		vertexData.Draw( &shader, uniformList );
		
		// Shadow
		ae::Matrix4 flat = ae::Matrix4::Translation( ae::Vec3( 0.0f, 0.0f, -1.25f ) )
			* ae::Matrix4::Scaling( ae::Vec3( 1.0f, 1.0f, 0.0f ) );
		uniformList.Set( "u_worldToProj", viewToProj * worldToView * flat * modelToWorld );
		uniformList.Set( "u_color", ae::Color::Black().ScaleA( 0.1f ).GetLinearRGBA() );
		vertexData.Draw( &shader, uniformList );
		
		render.Present();
		timeStep.Tick();

		return !input.quit;
	}

	void Terminate()
	{
		AE_INFO( "Terminate" );
		input.Terminate();
		render.Terminate();
		window.Terminate();
	}
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	Example example;
	example.Initialize();
#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* example ) { ((Example*)example)->Tick(); }, &example, 0, 1 );
#else
	while ( example.Tick() ) {}
#endif
	example.Terminate();
	return 0;
}
