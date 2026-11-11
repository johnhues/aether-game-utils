//------------------------------------------------------------------------------
// Main.cpp
//------------------------------------------------------------------------------
// Uses ae::Window, ae::Input, ae::Application, ae::GraphicsDevice and ae::Audio
// so linking proves each is defined under every combination check.sh builds.
//------------------------------------------------------------------------------
#include "aether.h"

int main( int argc, char* argv[] )
{
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::Audio audio;
	ae::TimeStep timeStep;
	const ae::Array< ae::Screen, 16 > screens = ae::GetScreens();
	auto Initialize = [ & ]()
	{
		window.Initialize( 320, 240, false, true, "" );
		window.SetTitle( "config" );
		render.Initialize( &window );
		input.Initialize( &window );
		audio.Initialize( 1, 1, 1, 1 );
		return screens.Length() > 0;
	};
	auto Update = [ & ]()
	{
		input.Pump();
		while( const ae::InputPointer* pointer = input.PumpPointers() )
		{
			input.ReleasePointer( pointer );
		}
		input.SetCursorState( ae::CursorState::Normal );
		render.Activate();
		render.Clear( ae::Color::Black() );
		render.Present();
		timeStep.Tick();
		return !input.Get( ae::Key::Escape );
	};
	auto Terminate = [ & ]() -> int32_t
	{
		audio.Terminate();
		input.Terminate();
		render.Terminate();
		window.Terminate();
		return 0;
	};
	return ae::Application( argc, argv, {
		.initialize = Initialize,
		.update = Update,
		.terminate = Terminate,
	} );
}
