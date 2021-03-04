# aether game utils
A collection of cross platform utilities for quickly creating small game prototypes in C++.

Modules and utilities include:
* Allocation
* C++ reflection
* Data structures
* Frame rate control and timing
* Graphics and windowing
* Input
* Logging
* Math
* Networked object synchronization
* Serialization
* Server / client utilities
* Terrain

Dependencies:
* [GLEW](http://glew.sourceforge.net/) (Windows only)
* [SDL2](https://www.libsdl.org/)
* [Assimp](https://github.com/assimp/assimp)
* [ENet](http://enet.bespin.org/)
* [zlib](https://github.com/madler/zlib)
* [OSSP uuid](http://www.ossp.org/pkg/lib/uuid/)
* [Catch2](https://github.com/catchorg/Catch2) (Test framework)

## Games using aether game utils
[Bella Lunaux: The Case of the Diamond Panther Tiara](https://johnhues.itch.io/bella-lunaux-tiara)

<a href="https://johnhues.itch.io/bella-lunaux-tiara"><img src="https://img.itch.zone/aW1hZ2UvOTQzMzU2LzUzNDY4NTMucG5n/original/%2BypGAU.png" width="250"></a>

## Example
A complete working example showing a window with a red background.
`main.cpp`
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
This is a cmake file that will configure main.cpp to be compiled and linked with aether-game-utils. It's not necessary that your project uses cmake, but it might be easier for initial setup.
`CMakeLists.txt`
```
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(MyProject LANGUAGES CXX VERSION 0.1.0)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_UNICODE -DUNICODE)

set(CMAKE_BUILD_TYPE Debug)

find_package(ae REQUIRED)

add_executable(game main.cpp)
target_link_libraries(
	game
	ae
)
```

## Linux Setup
Installing dependencies:
```
sudo apt install libsdl2-dev libenet-dev uuid-dev libopenal-dev
```
Building:
```
mkdir ~/aether-game-utils && cd ~/aether-game-utils
git clone --recurse-submodules https://github.com/johnhues/aether-game-utils.git .
mkdir ./build && cd ./build
cmake ..
make
sudo make install
```

## Mac Setup
Installing dependencies:
```
brew install sdl2 enet ossp-uuid libpng
```
Building:
```
mkdir ~/aether-game-utils && cd ~/aether-game-utils
git clone --recurse-submodules https://github.com/johnhues/aether-game-utils.git .
mkdir ./build && cd ./build
cmake ..
make
sudo make install
```
Using the Xcode cmake generator will work for everything except the actual library install step, and so just using make is advised on OSX instead.

## Windows Setup
Make sure cmake is installed: https://cmake.org/download/
Depending on your environment you may need to append the cmake directory to your path:
```
SET PATH=%PATH%;C:\Program Files\CMake\bin;
```
Python 3 is also required to run unit tests: https://www.python.org/downloads/

These commands are intended to be run with Windows Command Prompt and may not work with cygwin or other environments. You may need to individually copy and paste the following commands as cmake can cause strange behavior when multiple commands are strung together.
```
git clone --recurse-submodules https://github.com/johnhues/aether-game-utils.git C:\temp\aether-game-utils
mkdir C:\temp\aether-game-utils\build && cd C:\temp\aether-game-utils\build
cmake -DCMAKE_INSTALL_PREFIX=C:\Library ..
cmake --build . --config Release --target INSTALL
cmake --build . --config Debug --target INSTALL
```
If successful you should see the installed library files in `C:\Library\ae`. ~~You can safely delete `C:\temp\aether-game-utils` without affecting the installed library.~~ Currently the install step does not copy dependent static libraries, and so built dependencies are referenced when linking the final executable.

Optional: CMake can automatically locate aeLib (and any other libraries you install!) with an environment variable. To accomplish this add a new environment variable `CMAKE_PREFIX_PATH` with the value `C:\Library`.
