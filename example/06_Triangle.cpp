//------------------------------------------------------------------------------
// 06_Triangle.cpp
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
#include "aeInput.h"
#include "aeLog.h"
#include "aeRender.h"
#include "aeWindow.h"

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = "\
	AE_UNIFORM mat4 u_modelToNdc;\
	AE_IN_HIGHP vec4 a_position;\
	AE_IN_HIGHP vec4 a_color;\
	AE_OUT_HIGHP vec4 v_color;\
	void main()\
	{\
		v_color = a_color;\
		gl_Position = u_modelToNdc * a_position;\
	}";

const char* kFragShader = "\
	AE_IN_HIGHP vec4 v_color;\
	void main()\
	{\
		AE_COLOR = v_color;\
	}";

//------------------------------------------------------------------------------
// Triangle
//------------------------------------------------------------------------------
struct Vertex
{
	aeFloat4 pos;
	aeFloat4 color;
};

Vertex kTriangleVerts[] =
{
	{ aeFloat4( -0.5f, -0.4f, 0.0f, 1.0f ), aeColor::PicoRed().GetLinearRGBA() },
	{ aeFloat4( 0.5f, -0.4f, 0.0f, 1.0f ), aeColor::PicoGreen().GetLinearRGBA() },
	{ aeFloat4( 0.0f, 0.4f, 0.0f, 1.0f ), aeColor::PicoBlue().GetLinearRGBA() },
};

uint16_t kTriangleIndices[] =
{
	0, 1, 2
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	aeFixedTimeStep timeStep;
	aeShader shader;
	aeVertexData vertexData;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "triangle" );
	render.InitializeOpenGL( &window );
	render.SetClearColor( aeColor::PicoDarkPurple() );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );

	vertexData.Initialize( sizeof( *kTriangleVerts ), sizeof( *kTriangleIndices ), countof( kTriangleVerts ), countof( kTriangleIndices ), aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
	vertexData.AddAttribute( "a_position", 4, aeVertexDataType::Float, offsetof( Vertex, pos ) );
	vertexData.AddAttribute( "a_color", 4, aeVertexDataType::Float, offsetof( Vertex, color ) );
	vertexData.SetVertices( kTriangleVerts, countof( kTriangleVerts ) );
	vertexData.SetIndices( kTriangleIndices, countof( kTriangleIndices ) );

	AE_LOG( "Run" );
	while ( !input.GetState()->exit )
	{
		input.Pump();
		render.Activate();

		aeUniformList uniformList;
		uniformList.Set( "u_modelToNdc", aeFloat4x4::Scaling( aeFloat3( 1.0f / render.GetAspectRatio() , 1.0f, 1.0f ) ) );
		vertexData.Render( &shader, uniformList );
		
		render.Present();
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
