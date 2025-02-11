//------------------------------------------------------------------------------
// 05_TextInput.cpp
//------------------------------------------------------------------------------
// Copyright(c) 2024 John Hughes
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
	ae::TextRender textRender = TAG_EXAMPLE;
	ae::TimeStep timeStep;
	
	fileSystem.Initialize( DATA_DIR, "ae", "text_input" );
	window.Initialize( 1280, 720, false, true, true );
	window.SetTitle( "example" );
	render.Initialize( &window );
	input.Initialize( &window );
	input.SetTextMode( true );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	
	const ae::File* fontFile = fileSystem.Read( ae::FileSystem::Root::Data, "font.tga", 2.5f );
	ae::Texture2D fontTexture;

	float textZoom = 4.0f;
	const char* defaultText = "Try typing.\nCopy and paste should also work.\nYou can zoom by scrolling.\nPress 'ESC' to reset.";
	input.SetText( defaultText );

	auto Update = [&]()
	{
		input.Pump();
		if( fontFile && fontFile->GetStatus() != ae::File::Status::Pending )
		{
			AE_ASSERT_MSG( fontFile->GetLength(), "Could not load #", fontFile->GetUrl() );
			ae::TargaFile targa = TAG_EXAMPLE;
			targa.Load( fontFile->GetData(), fontFile->GetLength() );
			targa.textureParams.filter = ae::Texture::Filter::Nearest;
			fontTexture.Initialize( targa.textureParams );
			textRender.Initialize( 1, 512, &fontTexture, 8, 1.0f );
			fileSystem.Destroy( fontFile );
			fontFile = nullptr;
		}
		
		if( input.Get( ae::Key::Escape ) && !input.GetPrev( ae::Key::Escape ) )
		{
			textZoom = 4.0f;
			input.SetText( defaultText );
		}
		textZoom += input.mouse.scroll.y * 0.01f;

		// UI units in pixels, origin in bottom left
		ae::Matrix4 textToNdc = ae::Matrix4::Scaling( ae::Vec3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
		textToNdc *= ae::Matrix4::Translation( ae::Vec3( render.GetWidth() / -2.0f, render.GetHeight() / 2.0f, 0.0f ) );
		textToNdc *= ae::Matrix4::Scaling( ae::Vec3( textZoom, textZoom, 1.0f ) );

		// Format input text buffer
		ae::Str512 displayText( ">" );
		displayText.Append( input.GetText() );
		if( input.Get( ae::Key::Meta ) && input.GetPress( ae::Key::C ) )
		{
			ae::SetClipboardText( input.GetText() );
		}
		if( input.Get( ae::Key::Meta ) && input.GetPress( ae::Key::V ) )
		{
			std::string inputText = input.GetText();
			inputText.append( ae::GetClipboardText() );
			input.SetText( inputText.c_str() );
		}
		
		static int s_blink = 0;
		static int s_prevLen = 0;
		if( s_prevLen != displayText.Length() )
		{
			s_prevLen = displayText.Length();
			s_blink = 0;
		}
		else if( s_blink >= 60 )
		{
			s_blink -= 60;
		}
		displayText.Append( s_blink < 30 ? "_" : " " );
		s_blink++;
		
		render.Activate();
		render.Clear( ae::Color::Green().ScaleRGB( 0.01f ) );
		if( textRender.GetFontSize() )
		{
			// Render text in top left corner
			int maxLineLength = render.GetWidth() / textRender.GetFontSize() - 2;
			ae::Vec3 textPos( textRender.GetFontSize() / 2.0f, textRender.GetFontSize() / -2.0f, 0.0f );
			textRender.Add( textPos, ae::Vec2( (float)textRender.GetFontSize() ), displayText.c_str(), ae::Color::Green(), maxLineLength, 0 );
			textRender.Render( textToNdc );
		}
		render.Present();

		timeStep.Tick();
		return !input.quit;
	};
#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while ( Update() ) {}
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
