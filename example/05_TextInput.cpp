//------------------------------------------------------------------------------
// 05_TextInput.cpp
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
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	aeTextRender textRender;
	
	window.Initialize( 1280, 720, false, true );
	window.SetTitle( "example" );
	render.InitializeOpenGL( &window, window.GetWidth() / 4, window.GetHeight() / 4 );
	render.SetClearColor( aeColor::Green().ScaleRGB( 0.01f ) );
	input.Initialize( &window, &render );
	input.SetTextMode( true );
	input.SetText( "Try typing. Copy and paste should also work." );
	textRender.Initialize( "font.png", aeTextureFilter::Nearest, 8 );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.GetState()->exit )
	{
		input.Pump();
		render.Resize( window.GetWidth() / 4, window.GetHeight() / 4 );
		render.StartFrame();

		// UI units in pixels, origin in bottom left
		aeFloat4x4 textToNdc = aeFloat4x4::Scaling( aeFloat3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
		textToNdc *= aeFloat4x4::Translation( aeFloat3( render.GetWidth() / -2.0f, render.GetHeight() / -2.0f, 0.0f ) );

		// Format input text buffer
		aeStr512 displayText( ">" );
		displayText.Append( input.GetText() );
		displayText.Append( "_" );

		// Render text in top left corner
		int maxLineLength = render.GetWidth() / textRender.GetFontSize() - 2;
		aeFloat3 textPos( textRender.GetFontSize(), render.GetHeight() - textRender.GetFontSize(), 0.0f );
		textRender.Add( textPos, aeFloat2( (float)textRender.GetFontSize() ), displayText.c_str(), aeColor::Green(), maxLineLength, 0 );
		textRender.Render( textToNdc );

		render.EndFrame();
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	textRender.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
