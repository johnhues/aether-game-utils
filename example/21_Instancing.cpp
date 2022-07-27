//------------------------------------------------------------------------------
// 21_Instancing.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2022 John Hughes
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
	AE_IN_HIGHP vec3 a_position; // Per vertex
	AE_IN_HIGHP vec4 a_color; // Per vertex
	AE_IN_HIGHP vec3 a_offset; // Per instance
	AE_OUT_HIGHP vec4 v_color;
	void main()
	{
		v_color = a_color * u_color;
		gl_Position = u_worldToProj * vec4( a_position + a_offset, 1.0 );
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
const uint32_t kMaxInstancesDimm = 128;
const uint32_t kMaxInstances = kMaxInstancesDimm * kMaxInstancesDimm * kMaxInstancesDimm;

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
	ae::DebugCamera camera;
	ae::Shader shader;
	ae::VertexBuffer vertexData;
	ae::InstanceData instanceData;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "instancing" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	camera.Initialize( ae::Axis::Y, ae::Vec3( 0.0f ), ae::Vec3( 25.0f ) );

	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	shader.SetBlending( true );
	shader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );

	vertexData.Initialize( sizeof( *kCubeVerts ), sizeof( *kCubeIndices ), countof( kCubeVerts ), countof( kCubeIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	vertexData.AddAttribute( "a_position", 3, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
	vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
	vertexData.UploadVertices( 0, kCubeVerts, countof( kCubeVerts ) );
	vertexData.UploadIndices( 0, kCubeIndices, countof( kCubeIndices ) );

	ae::Vec3* offsets = ae::NewArray< ae::Vec3 >( "instance", kMaxInstances );
	for ( uint32_t z = 0 ; z < kMaxInstancesDimm; z++ )
	for ( uint32_t y = 0 ; y < kMaxInstancesDimm; y++ )
	for ( uint32_t x = 0 ; x < kMaxInstancesDimm; x++ )
	{
		ae::Vec3 offset( x, y, z );
		offset -= ae::Vec3( kMaxInstancesDimm / 2 );
		offset *= 3.0f;
		uint32_t idx = x + y * kMaxInstancesDimm + z * kMaxInstancesDimm * kMaxInstancesDimm;
		offsets[ idx ] = offset;
	}
	instanceData.Initialize( sizeof(ae::Vec3), kMaxInstances, ae::Vertex::Usage::Static );
	instanceData.AddAttribute( "a_offset", 3, ae::Vertex::Type::Float, 0 );
	instanceData.UploadData( 0, offsets, kMaxInstances );
	
	float r0 = 0.0f;
	float r1 = 0.0f;

	AE_INFO( "Run" );
	while ( !input.quit )
	{
		input.Pump();
		camera.Update( &input, timeStep.GetDt() );
		
		r0 += timeStep.GetDt() * 0.6f;
		r1 += timeStep.GetDt() * 0.75f;

		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), ae::Vec3( 0.0f, 1.0f, 0.0f ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.5f, 1000.0f );
		
		ae::UniformList uniformList;
		uniformList.Set( "u_worldToProj", viewToProj * worldToView );
		uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );

		const ae::InstanceData* datas[] = { &instanceData };
		vertexData.Bind( &shader, uniformList, datas, 1 );
		vertexData.Draw( 0, countof( kCubeIndices ) / 3, kMaxInstances );
		
		render.Present();

		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
