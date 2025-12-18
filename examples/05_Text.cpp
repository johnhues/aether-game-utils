//------------------------------------------------------------------------------
// 05_Text.cpp
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
const ae::Tag TAG_EXAMPLE = "example";
#ifndef DATA_DIR
	#define DATA_DIR "data"
#endif
const uint32_t kCharWidth = 8;
const uint32_t kCharHeight = 8;
const uint32_t kCharScale = 4;
const char* kVertShader =
R"(
	AE_UNIFORM mat4 u_textToNdc;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_color;
	AE_IN_HIGHP vec2 a_uv;
	AE_OUT_HIGHP vec4 v_color;
	AE_OUT_HIGHP vec2 v_uv;
	void main()
	{
		v_color = a_color;
		v_uv = a_uv;
		gl_Position = u_textToNdc * a_position;
	}
)";
const char* kFragShader =
R"(
	AE_UNIFORM sampler2D u_tex;
	AE_IN_HIGHP vec4 v_color;
	AE_IN_HIGHP vec2 v_uv;
	void main()
	{
		AE_COLOR.rgb = v_color.rgb;
		AE_COLOR.a = v_color.a * AE_TEXTURE2D( u_tex, v_uv ).r;
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
	ae::SpriteRenderer textRender = TAG_EXAMPLE;
	ae::Texture2D fontTexture;
	ae::SpriteFont font;
	ae::Shader fontShader;
	fileSystem.Initialize( DATA_DIR, "ae", "text_input" );
	window.Initialize( 1280, 720, false, true, true );
	window.SetTitle( "example" );
	render.Initialize( &window );
	input.Initialize( &window );
	input.SetTextMode( true );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	textRender.Initialize( 1, 1024 * 8 );
	fontShader.Initialize( kVertShader, kFragShader );
	fontShader.SetBlending( true );

	const ae::File* fontFile = fileSystem.Read( ae::FileSystem::Root::Data, "font.tga", 2.5f );
	std::string displayText;
	input.SetText( "Try typing.\nCopy and paste should also work.\nResizing the window will rearrange the text.\n" );
	auto Update = [&]()
	{
		// File loading
		if( fontFile && fontFile->GetStatus() != ae::File::Status::Pending )
		{
			AE_ASSERT_MSG( fontFile->GetLength(), "Could not load #", fontFile->GetUrl() );
			ae::TargaFile targa = TAG_EXAMPLE;
			targa.Load( fontFile->GetData(), fontFile->GetLength() );
			targa.textureParams.filter = ae::Texture::Filter::Nearest;
			fontTexture.Initialize( targa.textureParams );
			font.SetGlyphsASCIISpriteSheet( fontTexture.GetWidth(), fontTexture.GetHeight(), kCharWidth, kCharHeight, ' ', '~', ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ) );
			fileSystem.Destroy( fontFile );
			fontFile = nullptr;
		}

		// Input
		input.Pump();
		if( input.Get( ae::Key::Meta ) && input.GetPress( ae::Key::C ) )
		{
			ae::SetClipboardText( input.GetText() );
		}
		if( input.Get( ae::Key::Meta ) && input.GetPress( ae::Key::V ) )
		{
			input.AppendText( ae::GetClipboardText().c_str() );
		}
		displayText = input.GetText();
		displayText += '_';
		
		// Render
		ae::Matrix4 textToNdc = ae::Matrix4::Translation( -1.0f, -1.0f, 0.0f ) * ae::Matrix4::Scaling( ae::Vec3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
		render.Activate();
		render.Clear( ae::Color::Green().ScaleRGB( 0.01f ) );
		if( fontTexture.GetTexture() )
		{
			const ae::Rect region = ae::Rect::FromPoints( ae::Vec2( kCharWidth * kCharScale ), ae::Vec2( render.GetWidth() - kCharWidth * kCharScale, render.GetHeight() - kCharWidth * kCharScale ) );
			textRender.AddText( 0, displayText.c_str(), &font, region, kCharHeight * kCharScale, ( kCharHeight + 1 ) * kCharScale, ae::Color::Green() );
		}
		ae::UniformList uniforms;
		uniforms.Set( "u_textToNdc", textToNdc );
		uniforms.Set( "u_tex", &fontTexture );
		textRender.SetParams( 0, &fontShader, uniforms );
		textRender.Render();
		render.Present();

		timeStep.Tick();
		return !input.quit;
	};
#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while( Update() ) {}
#endif

	AE_LOG( "Terminate" );
	textRender.Terminate();
	fontTexture.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();
	fileSystem.DestroyAll();
	return 0;
}
