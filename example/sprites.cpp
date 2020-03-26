//------------------------------------------------------------------------------
// sprites.cpp
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
// main
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
	render.InitializeOpenGL( &window, 400, 300 );
	render.SetClearColor( aeColor::Red );
	input.Initialize( &window, &render );
	spriteRender.Initialize( 16 );
	spriteRender.SetDepthTest( true );
	spriteRender.SetDepthWrite( true );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	aeTexture2D tex;
	uint8_t texData[] = { 255, 255, 255 };
	tex.Initialize( texData, 1, 1, 3, aeTextureFilter::Linear, aeTextureWrap::Repeat );

	float scale = 5.0f;
	aeFloat4x4 screenTransform = aeFloat4x4::Scaling( aeFloat3( 1.0f / scale, render.GetAspectRatio() / scale, 1.0f ) );

	while ( !input.GetState()->exit )
	{
		input.Pump();
		render.StartFrame();
		spriteRender.Clear();

		aeFloat4x4 transform;

		transform = aeFloat4x4::Translation( aeFloat3( 0.5f, 0.5f, -0.5f ) );
		transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
		spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Blue );

		transform = aeFloat4x4::Translation( aeFloat3( -0.5f, -0.5f, 0.5f ) );
		transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
		spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Blue );

		transform = aeFloat4x4::Scaling(  aeFloat3( 1.0f, 1.0f, 0.5f ) );
		spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::White );

		spriteRender.Render( screenTransform );
		render.EndFrame();
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
