//------------------------------------------------------------------------------
// 02_Sprites.cpp
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
#include "ae/ae.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	aeSpriteRender spriteRender;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "sprites" );
	render.InitializeOpenGL( &window );
	input.Initialize( &window );
	spriteRender.Initialize( 16 );
	spriteRender.SetBlending( true );
  spriteRender.SetDepthTest( true );
	spriteRender.SetDepthWrite( true );
  spriteRender.SetSorting( true );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	aeTexture2D tex;
	tex.Initialize( "circle.png", aeTextureFilter::Linear, aeTextureWrap::Repeat );

	while ( !input.GetState()->exit )
	{
		input.Pump();
		int scaleFactor = input.GetState()->space ? 4 : 1;
		render.Activate();
		render.Clear( aeColor::PicoDarkPurple() );
		//render.StartFrame( window.GetWidth() / scaleFactor, window.GetHeight() / scaleFactor );
		spriteRender.Clear();

		aeFloat4x4 transform;

		// @NOTE: Notice these are rendered out of order and the transparent edges of
		//        the circles are handled correctly. Hold space to see this more clearly.

		// Front
		transform = aeFloat4x4::Translation( aeFloat3( 0.5f, 0.5f, -0.5f ) );
		transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
		spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::PicoBlue() );

		// Back
		transform = aeFloat4x4::Translation( aeFloat3( -0.5f, -0.5f, 0.5f ) );
		transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
		spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::PicoBlue() );

		// Middle
		transform = aeFloat4x4::Scaling(  aeFloat3( 1.0f, 1.0f, 0.5f ) );
		spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::PicoWhite() );

		aeFloat4x4 screenTransform = aeFloat4x4::Scaling( aeFloat3( 1.0f / 5.0f, render.GetAspectRatio() / 5.0f, 1.0f ) );
		spriteRender.Render( screenTransform );
		render.Present();
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
