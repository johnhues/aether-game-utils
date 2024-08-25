//------------------------------------------------------------------------------
// 02_Sprites.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2024 John Hughes
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
#include "ae/SpriteRenderer.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";

const char* kVertexShader = R"(
	AE_UNIFORM_HIGHP mat4 u_worldToProj;

	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_color;
	AE_IN_HIGHP vec2 a_uv;

	AE_OUT_HIGHP vec4 v_color;
	AE_OUT_HIGHP vec2 v_uv;

	void main()
	{
		v_color = a_color;
		v_uv = a_uv;
		gl_Position = u_worldToProj * a_position;
	}
)";
const char* kFragmentShader = R"(
	AE_UNIFORM sampler2D u_tex;

	AE_IN_HIGHP vec4 v_color;
	AE_IN_HIGHP vec2 v_uv;

	void main()
	{
		AE_COLOR = v_color * AE_TEXTURE2D( u_tex, v_uv );
	}
)";

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );
	ae::FileSystem fileSystem;
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::SpriteRenderer spriteRenderer = TAG_EXAMPLE;
	ae::Shader spriteShader;

	fileSystem.Initialize( "data", "ae", "sprites" );
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "sprites" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	spriteRenderer.Initialize( 1, 16 );
	spriteShader.Initialize( kVertexShader, kFragmentShader, nullptr, 0 );
	
	// Sprites
	ae::Texture2D spriteTex;
	{
		ae::Scratch< uint8_t > fileData( fileSystem.GetSize( ae::FileSystem::Root::Data, "circle.tga" ) );
		fileSystem.Read( ae::FileSystem::Root::Data, "circle.tga", fileData.Data(), fileData.Length() );
		ae::TargaFile targa = TAG_EXAMPLE;
		targa.Load( fileData.Data(), fileData.Length() );
		spriteTex.Initialize( targa.textureParams );
	}
	
	while ( !input.quit )
	{
		input.Pump();
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		spriteRenderer.Clear();
		
		// Red
		ae::Matrix4 localToWorld = ae::Matrix4::Translation( ae::Vec3( -0.5f, -0.5f, 0.5f ) );
		localToWorld *= ae::Matrix4::Scaling( ae::Vec3( 1.0f, 1.0f, 0.0f ) );
		spriteRenderer.AddSprite( 0, localToWorld, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::PicoRed() );

		// Green
		localToWorld = ae::Matrix4::Scaling(  ae::Vec3( 1.0f, 1.0f, 0.5f ) );
		spriteRenderer.AddSprite( 0, localToWorld, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::PicoGreen() );

		// Blue
		localToWorld = ae::Matrix4::Translation( ae::Vec3( 0.5f, 0.5f, -0.5f ) );
		localToWorld *= ae::Matrix4::Scaling( ae::Vec3( 1.0f, 1.0f, 0.0f ) );
		spriteRenderer.AddSprite( 0, localToWorld, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::PicoBlue() );

		ae::UniformList uniforms;
		uniforms.Set( "u_worldToProj", ae::Matrix4::Scaling( ae::Vec3( 0.5f / render.GetAspectRatio(), 0.5f, 1.0f ) ) );
		uniforms.Set( "u_tex", &spriteTex );
		spriteRenderer.SetParams( 0, &spriteShader, uniforms );
		spriteRenderer.Render();
		render.Present();
		timeStep.Tick();
	}

	AE_LOG( "Terminate" );
	spriteRenderer.Terminate();
	spriteTex.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
