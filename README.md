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
* [SDL2](https://www.libsdl.org/)
* [Assimp](https://github.com/assimp/assimp)
* [libpng](http://www.libpng.org/pub/png/)
* [Catch2](https://github.com/catchorg/Catch2)

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

	input.Terminate();
	renderer.Terminate();
	window.Terminate();

	return 0;
}
```

## Linux Setup
Installing dependencies:
```
sudo apt install libpng-dev libsdl2-dev libassimp-dev libenet-dev uuid-dev
```
Building:
```
mkdir ~/aether-game-utils && cd ~/aether-game-utils
git clone https://github.com/johnhues/aether-game-utils.git .
mkdir ./build && cd ./build
cmake ..
make
```

## MacOSX Setup
Installing dependencies:
```
brew install libpng sdl2 assimp enet ossp-uuid
```
Building:
```
mkdir ~/aether-game-utils && cd ~/aether-game-utils
git clone https://github.com/johnhues/aether-game-utils.git .
mkdir ./build && cd ./build
cmake ..
make
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
