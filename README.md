# aether game utils
`aether.h` is a single-header collection of cross platform C++ utilities for quickly creating small games. Platforms currently supported are Mac, Windows, Linux, and Web. Core tenants of this library are to statisfy the (often times at odds!) goals of:
* Providing useful and performant utilities tailored for game development
* Providing utilities that naturally conform to game development best practices
* Not overstepping or emposing a particular game engine architecture

Modules and utilities include:
* Allocation
* C++ reflection
* Data structures
* File IO
* Frame rate control and timing
* Geometry and collision detection
* Graphics and windowing
* Input
* Logging
* Math
* Networked object synchronization
* Serialization
* Sockets

# Example
A complete working example showing a window with a purple background:

`main.cpp` (or `main.mm` on Mac)
```
#define AE_MAIN
#define AE_USE_MODULES
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
Download the repository with:
```
git clone https://github.com/johnhues/aether-game-utils.git ~/aether-game-utils
```
Create a file called `main.mm` with the above contents. Open the `Terminal` application. Type `cd ` (with a space after) and then drag the folder containing main.mm into the terminal window and press enter. With Xcode installed run the following:
```
clang++ -std=c++17 -fmodules -fcxx-modules -I ~/aether-game-utils main.mm
```

## Building on Windows
Download the repository with:
```
git clone https://github.com/johnhues/aether-game-utils.git C:\aether-game-utils
```
Create a file called `main.cpp` with the above contents. With Visual Studio installed, right click in the containing directory and choose `Open in Terminal`. Run `"C:\Program Files\Microsoft Visual Studio\20XX\EDITION\VC\Auxiliary\Build\vcvars64.bat"`, replacing `20XX` with the year, and `EDITION` with `Community` etc. Finally build it with:
```
cl /std:c++17 -D_UNICODE -DUNICODE /I C:\aether-game-utils main.cpp
```

## Building with Emscripten
```
// todo
```

## Building on Linux
All utilities are currently supported on Linux except for windowing and graphics. Full windowing and graphics support is planned.

# Games using aether game utils
[Bella Lunaux: The Case of the Diamond Panther Tiara](https://v6p9d9t4.ssl.hwcdn.net/html/5321468/index.html)

<a href="https://v6p9d9t4.ssl.hwcdn.net/html/5321468/index.html"><img src="https://img.itch.zone/aW1hZ2UvOTQzMzU2LzUzNDY4NTMucG5n/original/%2BypGAU.png" width="250"></a>

[Play in Browser](https://v6p9d9t4.ssl.hwcdn.net/html/5321468/index.html)

[See on Itch.io](https://johnhues.itch.io/bella-lunaux-tiara)

# Dependencies
* [Catch2](https://github.com/catchorg/Catch2) (Test framework)
* [ENet](http://enet.bespin.org/) (Optional)
* [OpenAL](https://github.com/kcat/openal-soft) (Optional)
