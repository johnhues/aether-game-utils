#include "aeClock.h"
#include "aeInput.h"
#include "aeLog.h"
#include "aeRender.h"
#include "aeWindow.h"

int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRenderer renderer;
	aeInput input;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "example" );
	renderer.Initialize( &window, 400, 300 );
	renderer.SetClearColor( aeColor::Red );
	input.Initialize( &window, &renderer );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.GetState()->esc )
	{
		input.Pump();
		renderer.StartFrame();
		renderer.EndFrame();
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	input.Terminate();
	renderer.Terminate();
	window.Terminate();

	return 0;
}
