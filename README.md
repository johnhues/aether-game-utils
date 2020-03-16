# aether game utils
A collection of cross platform utilities for quickly creating small game prototypes in C++.

Modules and utilities include:
* Graphics and windowing
* Input
* Math
* C++ reflection
* Data structures
* Allocation
* Frame rate control and timing
* Logging

Dependencies:
* [GLEW](http://glew.sourceforge.net/) (Windows only)
* [SDL2](https://www.libsdl.org/)
* [Assimp](https://github.com/assimp/assimp)
* [ENet](http://enet.bespin.org/)
* [zlib](https://github.com/madler/zlib)
* [OSSP uuid](http://www.ossp.org/pkg/lib/uuid/)
* [Catch2](https://github.com/catchorg/Catch2) (Test framework)

## Example
A complete working example showing a window with a red background:
```
#include "aeClock.h"
#include "aeInput.h"
#include "aeRender.h"
#include "aeWindow.h"

int main()
{
	aeWindow window;
	aeRender render;
	aeInput input;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "example" );
	render.InitializeOpenGL( &window, 400, 300 );
	render.SetClearColor( aeColor::Red );
	input.Initialize( &window, &render );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.GetState()->exit )
	{
		input.Pump();
		render.StartFrame();
		render.EndFrame();
		timeStep.Wait();
	}

	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
```

## Linux Setup
Installing dependencies:
```
sudo apt install libsdl2-dev libassimp-dev libenet-dev uuid-dev
```
Building:
```
mkdir ~/aether-game-utils && cd ~/aether-game-utils
git clone https://github.com/johnhues/aether-game-utils.git .
mkdir ./build && cd ./build
cmake ..
make
sudo make install
```

## MacOSX Setup
Installing dependencies:
```
brew install sdl2 assimp enet ossp-uuid
```
Building:
```
mkdir ~/aether-game-utils && cd ~/aether-game-utils
git clone https://github.com/johnhues/aether-game-utils.git .
mkdir ./build && cd ./build
cmake ..
make
sudo make install
```

## Windows Setup
```
mkdir C:\Library
cd C:\Library
git clone https://github.com/johnhues/aether-game-utils.git aether-game-utils

Use cmake to generate VS projects:
Source should point to 'C:\Library\aether-game-utils'
Build should point to 'C:\Library\aether-game-utils/build'
```
