//------------------------------------------------------------------------------
// 06_Triangle.cpp
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
// Triangle
//------------------------------------------------------------------------------
struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 color;
};

const Vertex kTriangleVerts[] = {
	{ ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), ae::Color::AetherRed().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Color::AetherGreen().GetLinearRGBA() },
	{ ae::Vec4( 0.0f, 0.5f, 0.0f, 1.0f ), ae::Color::AetherBlue().GetLinearRGBA() },
};

const uint16_t kTriangleIndices[] = { 0, 1, 2 };

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::Shader shader;
	ae::VertexBuffer vertexData;
	ae::Vec2 position( 0.0f );
	ae::Vec2 velocity( 0.0f );
	float facing = ae::HalfPi;
	float rotation = 0.0f;
	float zoom = 0.25f;

	auto Initialize = [ & ]()
	{
		AE_LOG( "Initialize (debug #)", (int)_AE_DEBUG_ );
		window.Initialize( 1280, 720, false, true, true );
		window.SetTitle( "triangle" );
		render.Initialize( &window );
		input.Initialize( &window );
		timeStep.SetTimeStep( 1.0f / 60.0f );

		const char* vertShader = R"(
			AE_UNIFORM mat4 u_modelToNdc;
			AE_IN_HIGHP vec4 a_position;
			AE_IN_HIGHP vec4 a_color;
			AE_OUT_HIGHP vec4 v_color;
			void main()
			{
				v_color = a_color;
				gl_Position = u_modelToNdc * a_position;
			})";
		const char* fragShader = R"(
			AE_IN_HIGHP vec4 v_color;
			void main()
			{
				AE_COLOR = v_color;
			})";
		shader.Initialize( vertShader, fragShader, nullptr, 0 );

		vertexData.Initialize( sizeof( *kTriangleVerts ), sizeof( *kTriangleIndices ), countof( kTriangleVerts ), countof( kTriangleIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
		vertexData.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
		vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
		vertexData.UploadVertices( 0, kTriangleVerts, countof( kTriangleVerts ) );
		vertexData.UploadIndices( 0, kTriangleIndices, countof( kTriangleIndices ) );
		return true;
	};
	auto Update = [ & ]() -> bool
	{
		// Update input
		input.Pump();
		if( input.GetMousePressLeft() )
		{
			input.SetMouseCaptured( !input.GetMouseCaptured() );
		}
		if( input.GetPress( ae::Key::Escape ) )
		{
			input.SetMouseCaptured( false );
		}

		// Keyboard and mouse movement
		const ae::Vec2 keyDir = ae::Vec2( ( input.Get( ae::Key::Right ) - input.Get( ae::Key::Left ) ), ( input.Get( ae::Key::Up ) - input.Get( ae::Key::Down ) ) ).SafeNormalizeCopy();
		ae::Vec2 mouseDir( 0.0f );
		if( input.GetMouseCaptured() )
		{
			mouseDir += input.mouse.movement * 0.5f;
		}
		// Scroll wheel
		if( input.mouse.usingTouch )
		{
			mouseDir += input.mouse.scroll * ae::Vec2( 0.5f, -0.5f );
		}
		else
		{
			zoom += ( input.mouse.scroll.y + input.mouse.scrollInertia.y ) * 0.01f;
			zoom = ae::Clip( zoom, 0.1f, 2.0f );
		}
		mouseDir.Trim( 1.0f );
		const ae::Vec2 dir = ( keyDir + mouseDir ).SafeNormalizeCopy();
		velocity += dir * timeStep.GetDt() * 20.0f;
		position += velocity * timeStep.GetDt();
		velocity = ae::DtLerp( velocity, 1.5f, timeStep.GetDt(), ae::Vec2( 0.0f ) );
		
		if( velocity.LengthSquared() > 0.0001f )
		{
			facing = ae::DtLerpAngle( facing, 4.0f, timeStep.GetDt(), velocity.NormalizeCopy().GetAngle() );
		}
		rotation += timeStep.GetDt(); // Rotation effect

		// Start rendering
		render.Activate();
		render.Clear( ae::Color::AetherBlack() );
		// Transform
		const ae::Matrix4 transform = ae::Matrix4::Scaling( zoom / render.GetAspectRatio(), zoom, 1.0f ) * ae::Matrix4::Translation( position.x, position.y, 0.0f ) * ae::Matrix4::RotationZ( facing - ae::HalfPi ) * ae::Matrix4::RotationY( rotation );
		ae::UniformList uniformList;
		uniformList.Set( "u_modelToNdc", transform );
		vertexData.Bind( &shader, uniformList );
		vertexData.Draw();
		// Finish rendering
		render.Present();
		timeStep.Tick();

		return !input.quit;
	};
	auto Terminate = [ & ]() -> int32_t
	{
		vertexData.Terminate();
		shader.Terminate();
		input.Terminate();
		render.Terminate();
		window.Terminate();
		return 0;
	};
	return ae::Application( argc, argv, Initialize, Update, Terminate );
}
