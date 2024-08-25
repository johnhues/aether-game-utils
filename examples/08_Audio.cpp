//------------------------------------------------------------------------------
// 08_Audio.cpp
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

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );
	ae::FileSystem fs;
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::Audio audio;
	ae::TimeStep timeStep;
	fs.Initialize( "data", "ae", "audio" );
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "audio" );
	render.Initialize( &window );
	input.Initialize( &window );
	audio.Initialize( 1, 3, 0, 2 );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	auto LoadAudio = [&fs, &audio]( const ae::File*& file, const ae::AudioData*& audioData )
	{
		if( file && file->GetStatus() != ae::File::Status::Pending )
		{
			audioData = audio.LoadWavFile( file->GetData(), file->GetLength() );
			AE_INFO( audioData ? "Loaded audio file: '#'" : "Failed to load audio file: '#'", file->GetUrl() );
			fs.Destroy( file );
			file = nullptr;
		}
	};
	const ae::File* musicFile = fs.Read( ae::FileSystem::Root::Data, "music.wav", 5.0f );
	const ae::File* sfxFile = fs.Read( ae::FileSystem::Root::Data, "sound.wav", 5.0f );
	const ae::AudioData* music = nullptr;
	const ae::AudioData* sfx = nullptr;

	bool musicPlaying = false;
	float musicTime = 0.0f;
	float hitFade = 0.0f;

	auto Update = [&]()
	{
		// Update
		input.Pump();
		LoadAudio( musicFile, music );
		LoadAudio( sfxFile, sfx );
		if( musicPlaying )
		{
			musicTime += timeStep.GetDt();
		}
		if( hitFade > 0.0f )
		{
			hitFade = ae::Max( 0.0f, hitFade - timeStep.GetDt() / 0.2f );
		}
		
		// Input
		if( ( !input.mouse.rightButton && input.mousePrev.rightButton )
			|| ( !input.Get( ae::Key::Space ) && input.GetPrev( ae::Key::Space ) )
			|| ( !input.gamepads[ 0 ].y && input.gamepadsPrev[ 0 ].y ) )
		{
			if( musicPlaying )
			{
				audio.StopMusic( 0 );
				musicPlaying = false;
			}
			else if( music )
			{
				audio.PlayMusic( music, 0.5f, 0 );
				musicPlaying = true;
				musicTime = 0.0f;
			}
		}
		if( ( input.mouse.leftButton && !input.mousePrev.leftButton )
			|| ( input.gamepads[ 0 ].a && !input.gamepadsPrev[ 0 ].a ) )
		{
			audio.PlaySfx( sfx, 1.0f, 0 );
			hitFade = 1.0f;
		}

		// Render
		render.Activate();
		ae::Color color = ae::Color::PicoDarkGray();
		if( musicPlaying )
		{
			ae::Color beatColors[] =
			{
				ae::Color::PicoBlue(),
				ae::Color::PicoRed(),
				ae::Color::PicoOrange(),
				ae::Color::PicoDarkPurple()
			};
			const uint32_t beat = musicTime / 0.75f;
			color = beatColors[ beat % countof( beatColors ) ];
		}
		float hitOpacity = ae::Min( 1.0f, hitFade / 0.8f );
		hitOpacity *= hitOpacity;
		render.Clear( color.Lerp( ae::Color::PicoWhite(), hitOpacity * 0.8f ) );
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
	audio.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();
	fs.DestroyAll();
	return 0;
}
