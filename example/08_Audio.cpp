//------------------------------------------------------------------------------
// 08_Audio.cpp
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
#include "ae/aetherEXT.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	aeAudio audio;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "audio" );
	render.InitializeOpenGL( &window );
	input.Initialize( &window );
	audio.Initialize( 1, 3 );
	
	ae::TimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	aeAudioData music, sfx;
	music.Initialize( "music.wav" );
	sfx.Initialize( "sound.wav" );

	bool musicPlaying = false;
	float musicTime = 0.0f;
	float hitFade = 0.0f;

	while ( !input.GetState()->exit )
	{
		input.Pump();

		if ( musicPlaying )
		{
			musicTime += timeStep.GetTimeStep();
		}

		if ( hitFade > 0.0f )
		{
			hitFade = aeMath::Max( 0.0f, hitFade - timeStep.GetTimeStep() / 0.2f );
		}
		
		if ( ( !input.GetState()->mouseRight && input.GetPrevState()->mouseRight )
			|| ( !input.GetState()->space && input.GetPrevState()->space ) )
		{
			if ( musicPlaying )
			{
				audio.StopMusic( 0 );
				musicPlaying = false;
			}
			else
			{
				audio.PlayMusic( &music, 0.5f, 0 );
				musicPlaying = true;
				musicTime = 0.0f;
			}
		}

		if ( !input.GetState()->mouseLeft && input.GetPrevState()->mouseLeft )
		{
			audio.PlaySfx( &sfx, 1.0f, 0 );
			hitFade = 1.0f;
		}

		aeColor color = aeColor::PicoDarkGray();
		if ( musicPlaying )
		{
			aeColor beatColors[] =
			{
				aeColor::PicoBlue(),
				aeColor::PicoRed(),
				aeColor::PicoOrange(),
				aeColor::PicoDarkPurple()
			};

			uint32_t beat = musicTime / 0.75f;
			color = beatColors[ beat % countof( beatColors ) ];
		}

		float hitOpacity = aeMath::Min( 1.0f, hitFade / 0.8f );
		hitOpacity *= hitOpacity;
		render.Activate();
		render.Clear( color.Lerp( aeColor::PicoWhite(), hitOpacity * 0.8f ) );
		render.Present();
		
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	audio.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
