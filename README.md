# aether game utils
`aether.h` is a single-header collection of cross platform C++ utilities for quickly creating small games. Platforms currently supported are Mac, Windows, Linux, and Web. Core tenants of this library are to statisfy the (often times at odds!) goals of:
* Providing useful and performant utilities tailored for game development
* Providing utilities that naturally conform to game development best practices
* Not overstepping or emposing a particular game engine architecture

Modules and utilities include:
* Allocation
* C++ reflection
* Data structures
* Frame rate control and timing
* Geometry and collision detection
* Graphics and windowing
* Input
* Logging
* Math
* Networked object synchronization
* Serialization
* Sockets

## Example Source
A complete working example showing a window with a purple background:

`main.cpp`
```
#include "aether.h"

int main()
{
	AE_LOG( "Initialize" );
	ae::Window window;
	ae::GraphicsDevice graphicsDevice;
	ae::Input input;
	ae::TimeStep timeStep;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "example" );
	graphicsDevice.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.quit )
	{
		input.Pump();
		graphicsDevice.Activate();
		graphicsDevice.Clear( ae::Color::PicoDarkPurple() );
		graphicsDevice.Present();
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );
	input.Terminate();
	graphicsDevice.Terminate();
	window.Terminate();

	return 0;
}
```

## Building on Mac
TODO:
```
// todo
```

## Building on Windows
TODO:
```
// todo
```

## Building with Emscripten
TODO:
```
// todo
```

## Building on Linux
All utilities are currently supported on Linux except for windowing and graphics. Full windowing and graphics support is planned.

## Games using aether game utils
[Bella Lunaux: The Case of the Diamond Panther Tiara](https://v6p9d9t4.ssl.hwcdn.net/html/5321468/index.html)

<a href="https://v6p9d9t4.ssl.hwcdn.net/html/5321468/index.html"><img src="https://img.itch.zone/aW1hZ2UvOTQzMzU2LzUzNDY4NTMucG5n/original/%2BypGAU.png" width="250"></a>

[Play in Browser](https://v6p9d9t4.ssl.hwcdn.net/html/5321468/index.html)

[See on Itch.io](https://johnhues.itch.io/bella-lunaux-tiara)

## Dependencies
* [Catch2](https://github.com/catchorg/Catch2) (Test framework)
* [ENet](http://enet.bespin.org/) (Optional)
* [OpenAL](https://github.com/kcat/openal-soft) (Optional)
