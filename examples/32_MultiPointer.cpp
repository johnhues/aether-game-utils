//------------------------------------------------------------------------------
// 32_MultiPointer.cpp
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
// Draws every ae::InputPointer with its sample trail and properties, along
// with the hover position. Tap the mode button to
// switch between claiming with ae::Input::PumpPointers(),
// ae::Input::ClaimPointer(), or leaving pointers unclaimed.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";
const float kEndedDisplayTime = 1.0f;

enum class ClaimMode
{
	Pump,
	Claim,
	Unclaimed
};
const char* kClaimModeNames[] = { "PumpPointers()", "ClaimPointer()", "Unclaimed" };

struct EndedPointer
{
	ae::InputPointer pointer;
	double endTime = 0.0;
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::FileSystem fileSystem;
	ae::DebugLines debug = TAG_EXAMPLE;
	ae::TextRender text = TAG_EXAMPLE;
	ae::Texture2D fontTexture;
	const ae::File* fontFile = nullptr;
	ClaimMode claimMode = ClaimMode::Pump;
	ae::Array< EndedPointer, 16 > endedPointers;
	uint32_t pumpedCount = 0;
	bool textReady = false;

	auto Initialize = [&]()
	{
		AE_INFO( "Initialize" );
		window.Initialize( 800, 500, false, true, "" );
		window.SetTitle( "multi pointer" );
		render.Initialize( &window );
		input.Initialize( &window );
		timeStep.SetTimeStep( 1.0f / 60.0f );
		fileSystem.Initialize( "data", "ae", "multi_pointer" );
		debug.Initialize( 16384 );
		fontFile = fileSystem.Read( ae::FileSystem::Root::Data, "font.tga", 0.0f );
		AE_INFO( "Run" );
		return true;
	};

	auto Update = [&]() -> bool
	{
		input.Pump();
		if( fontFile && fontFile->GetStatus() != ae::File::Status::Pending )
		{
			ae::TargaFile targa = TAG_EXAMPLE;
			if( targa.Load( fontFile->GetData(), fontFile->GetLength() ) )
			{
				fontTexture.Initialize( targa.textureParams );
				text.Initialize( 64, 4096, &fontTexture, 8, 1.0f );
				textReady = true;
			}
			fileSystem.Destroy( fontFile );
			fontFile = nullptr;
		}

		const float width = (float)window.GetWidth();
		const float height = (float)window.GetHeight();
		const float fontSize = 16.0f;
		const ae::Rect modeButton = ae::Rect::FromPoints( ae::Vec2( width - 260.0f, 10.0f ), ae::Vec2( width - 10.0f, 50.0f ) );
		const double time = ae::GetTime();

		// Claim
		auto OnEnded = [&]( const ae::InputPointer* pointer )
		{
			if( modeButton.Contains( pointer->StartPosition() ) && modeButton.Contains( pointer->Position() ) )
			{
				claimMode = (ClaimMode)( ( (uint32_t)claimMode + 1 ) % countof( kClaimModeNames ) );
			}
			if( endedPointers.Full() )
			{
				endedPointers.Remove( 0 );
			}
			endedPointers.Append( { *pointer, time } );
		};
		if( input.GetPress( ae::Key::Space ) )
		{
			claimMode = (ClaimMode)( ( (uint32_t)claimMode + 1 ) % countof( kClaimModeNames ) );
		}
		if( claimMode == ClaimMode::Pump )
		{
			while( input.PumpPointers() )
			{
				pumpedCount++;
			}
		}
		else if( claimMode == ClaimMode::Claim )
		{
			for( const ae::InputPointer* pointer : input.GetUnclaimedPointers() )
			{
				input.ClaimPointer( pointer );
			}
		}
		const ae::InputPointerArray claimed = input.GetPointers();
		const ae::InputPointerArray unclaimed = input.GetUnclaimedPointers();
		for( const ae::InputPointer* pointer : claimed )
		{
			if( pointer->Ended() )
			{
				OnEnded( pointer );
				input.ReleasePointer( pointer );
			}
			else if( claimMode == ClaimMode::Unclaimed )
			{
				input.ReleasePointer( pointer );
			}
		}
		for( const ae::InputPointer* pointer : unclaimed )
		{
			if( pointer->Ended() )
			{
				OnEnded( pointer );
			}
		}
		while( endedPointers.Length() && time - endedPointers[ 0 ].endTime > kEndedDisplayTime )
		{
			endedPointers.Remove( 0 );
		}

		// Draw
		const ae::Color colors[] = { ae::Color::AetherRed(), ae::Color::AetherOrange(), ae::Color::AetherYellow(), ae::Color::AetherGreen(), ae::Color::AetherBlue(), ae::Color::AetherWhite() };
		const ae::Color background = ae::Color::AetherDarkPurple();
		float listY = height - 10.0f - fontSize * 6.0f;
		auto DrawPointer = [&]( const ae::InputPointer* pointer, bool isClaimed, float fade )
		{
			const ae::Color color = colors[ pointer->id % countof( colors ) ].Lerp( background, fade );
			const ae::Vec3 start( pointer->StartPosition(), 0.0f );
			const ae::Vec3 position( pointer->Position(), 0.0f );
			for( uint32_t i = 1; i < pointer->samples.Length(); i++ )
			{
				debug.AddLine( ae::Vec3( pointer->samples[ i - 1 ].position, 0.0f ), ae::Vec3( pointer->samples[ i ].position, 0.0f ), color );
			}
			for( const ae::InputPointer::Sample& sample : pointer->samples )
			{
				debug.AddCircle( ae::Vec3( sample.position, 0.0f ), ae::Vec3( 0, 0, 1 ), 2.0f, color, 6 );
			}
			debug.AddCircle( start, ae::Vec3( 0, 0, 1 ), 8.0f, color, 16 );
			debug.AddCircle( start, ae::Vec3( 0, 0, 1 ), pointer->MaxStartDistance(), color.ScaleA( 0.5f ).Lerp( background, 0.5f ), 32 );
			debug.AddLine( start, position, color.ScaleA( 0.5f ).Lerp( background, 0.5f ) );
			debug.AddCircle( position, ae::Vec3( 0, 0, 1 ), pointer->Ended() ? 20.0f : 40.0f, color, 32 );
			if( textReady && fade == 0.0f )
			{
				const ae::Vec2 delta = pointer->StartDelta();
				const char* const kSourceNames[] = { "mouse", "touch" };
				const char* const kButtonNames[] = { "primary", "secondary", "tertiary" };
				const ae::Str32 source = ae::Str32::Format( "# #", kSourceNames[ (uint32_t)pointer->source ], kButtonNames[ (uint32_t)pointer->button ] );
				const ae::Str256 info = ae::Str256::Format(
					"id # # #\n  pos #,#  delta #,#  max #  #s  samples #/#",
					pointer->id,
					source.c_str(),
					pointer->Ended() ? "ended" : ( isClaimed ? "claimed" : "unclaimed" ),
					(int32_t)pointer->Position().x, (int32_t)pointer->Position().y,
					(int32_t)delta.x, (int32_t)delta.y,
					(int32_t)pointer->MaxStartDistance(),
					(float)pointer->Lifetime(),
					pointer->samples.Length(), ae::kMaxInputPointerSamples );
				text.Add( ae::Vec3( 10.0f, listY, 0.0f ), ae::Vec2( fontSize ), info.c_str(), color, 0, 0 );
				text.Add( position + ae::Vec3( 44.0f, 8.0f, 0.0f ), ae::Vec2( fontSize ), ae::Str16::Format( "id #", pointer->id ).c_str(), color, 0, 0 );
				listY -= fontSize * 2.0f;
			}
		};
		for( const EndedPointer& ended : endedPointers )
		{
			DrawPointer( &ended.pointer, false, ae::Clip01( (float)( time - ended.endTime ) / kEndedDisplayTime ) );
		}
		for( const ae::InputPointer* pointer : input.GetPointers() )
		{
			DrawPointer( pointer, true, 0.0f );
		}
		for( const ae::InputPointer* pointer : input.GetUnclaimedPointers() )
		{
			DrawPointer( pointer, false, 0.0f );
		}
		if( const ae::Vec2* hover = input.hoverPos.TryGet() )
		{
			const ae::Vec3 h( *hover, 0.0f );
			debug.AddLine( h - ae::Vec3( 12.0f, 0.0f, 0.0f ), h + ae::Vec3( 12.0f, 0.0f, 0.0f ), ae::Color::White() );
			debug.AddLine( h - ae::Vec3( 0.0f, 12.0f, 0.0f ), h + ae::Vec3( 0.0f, 12.0f, 0.0f ), ae::Color::White() );
		}
		debug.AddRect( ae::Vec3( modeButton.GetCenter(), 0.0f ), ae::Vec3( 0, 1, 0 ), ae::Vec3( 0, 0, 1 ), modeButton.GetSize() * 0.5f, 6.0f, 4, ae::Color::White() );

		const ae::Str512 header = ae::Str512::Format(
			"Claim mode: # (Space or tap the button)\nClaimed: #  Unclaimed: #  Pumped: #\nHover: #",
			kClaimModeNames[ (uint32_t)claimMode ],
			input.GetPointers().Length(), input.GetUnclaimedPointers().Length(), pumpedCount,
			input.hoverPos.TryGet() ? ae::Str32::Format( "#, #", (int32_t)input.hoverPos.TryGet()->x, (int32_t)input.hoverPos.TryGet()->y ).c_str() : "none" );
		if( textReady )
		{
			text.Add( ae::Vec3( 10.0f, height - 10.0f, 0.0f ), ae::Vec2( fontSize ), header.c_str(), ae::Color::White(), 0, 0 );
			text.Add( ae::Vec3( modeButton.GetMin().x + 10.0f, modeButton.GetMax().y - 12.0f, 0.0f ), ae::Vec2( fontSize ), kClaimModeNames[ (uint32_t)claimMode ], ae::Color::White(), 0, 0 );
		}

		// Window units, origin in the bottom left
		ae::Matrix4 windowToNdc = ae::Matrix4::Scaling( ae::Vec3( 2.0f / width, 2.0f / height, 1.0f ) );
		windowToNdc *= ae::Matrix4::Translation( ae::Vec3( width / -2.0f, height / -2.0f, 0.0f ) );
		render.Activate();
		render.Clear( background );
		debug.Render( windowToNdc );
		if( textReady )
		{
			text.Render( windowToNdc );
		}
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};

	auto Terminate = [&]() -> int32_t
	{
		AE_INFO( "Terminate" );
		for( const ae::InputPointer* pointer : input.GetPointers() )
		{
			input.ReleasePointer( pointer );
		}
		input.Terminate();
		render.Terminate();
		window.Terminate();
		fileSystem.DestroyAll();
		return 0;
	};

	return ae::Application( argc, argv, Initialize, Update, Terminate );
}
