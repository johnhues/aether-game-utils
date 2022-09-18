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
```cpp
#define AE_MAIN
#define AE_USE_MODULES
#include "aether.h"
const ae::Tag TAG_EXAMPLE = "example";

const char kVertexShader[] = R"(
	AE_UNIFORM_HIGHP mat4 u_worldToProj;
	AE_IN_HIGHP vec3 a_position;
	AE_IN_HIGHP vec2 a_uv;
	AE_OUT_HIGHP vec2 v_uv;
	void main()
	{
		v_uv = a_uv;
		gl_Position = u_worldToProj * vec4( a_position, 1.0 );
	}
)";

const char kFragmentShader[] = R"(
	AE_UNIFORM sampler2D u_tex;
	AE_IN_HIGHP vec2 v_uv;
	void main()
	{
		AE_COLOR = AE_TEXTURE2D( u_tex, v_uv );
	}
)";

int main()
{
	// Initialization
	ae::Window window;
	ae::GraphicsDevice graphicsDevice;
	ae::Input input;
	ae::FileSystem fileSystem;
	ae::TimeStep timeStep;
	ae::OBJFile obj = TAG_EXAMPLE;
	ae::TargaFile checkerTarga = TAG_EXAMPLE;
	ae::Shader shader;
	ae::VertexArray vertexData;
	ae::Texture2D tex;
	ae::CollisionMesh<> collisionMesh = TAG_EXAMPLE;
	window.Initialize( 1280, 800, false, true );
	window.SetTitle( "Game" );
	graphicsDevice.Initialize( &window );
	input.Initialize( &window );
	fileSystem.Initialize( "", "ae", "Game" );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	// Load resources
	const ae::File* geoFile = fileSystem.Read( ae::FileSystem::Root::Data, "level.obj", 2.5f );
	const ae::File* textureFile = fileSystem.Read( ae::FileSystem::Root::Data, "level.tga", 2.5f );
	while ( fileSystem.AnyPending() ) { std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) ); }
	if ( !fileSystem.AllSuccess() ) { std::exit( EXIT_FAILURE ); }
	obj.Load( geoFile->GetData(), geoFile->GetLength() );
	obj.InitializeCollisionMesh( &collisionMesh, ae::Matrix4::Identity() );
	obj.InitializeVertexData( { &vertexData } );
	vertexData.Upload();
	checkerTarga.Load( textureFile->GetData(), textureFile->GetLength() );
	tex.Initialize( checkerTarga.textureParams );
	shader.Initialize( kVertexShader, kFragmentShader, nullptr, 0 );
	shader.SetCulling( ae::Culling::CounterclockwiseFront );
	shader.SetDepthWrite( true );
	shader.SetDepthTest( true );

	// Game state
	ae::PushOutInfo player;
	player.sphere.radius = 0.75f;
	player.sphere.center = ae::Vec3( 0.0f, player.sphere.radius, 0.0f );
	float angle = 0.0f;
	float angularVel = 0.0f;
	while ( !input.quit )
	{
		// Update input and physics
		input.Pump();
		player.velocity.SetXZ( ae::DtSlerp( player.velocity.GetXZ(), 2.5f, timeStep.GetDt(), ae::Vec2( 0.0f ) ) );
		angularVel = ae::DtLerp( angularVel, 3.5f, timeStep.GetDt(), 0.0f );
		ae::Vec3 forward( -cosf( angle ), 0.0f, sinf( angle ) );
		if ( input.Get( ae::Key::Up ) ) { player.velocity += forward * timeStep.GetDt() * 25.0f; }
		if ( input.Get( ae::Key::Down ) ) { player.velocity -= forward * timeStep.GetDt() * 25.0f; }
		if ( input.Get( ae::Key::Left ) ) { angularVel += timeStep.GetDt() * 15.0f; }
		if ( input.Get( ae::Key::Right ) ) { angularVel -= timeStep.GetDt() * 15.0f; }
		player.velocity.y -= timeStep.GetDt() * 20.0f;
		player.sphere.center += player.velocity * timeStep.GetDt();
		angle += angularVel * timeStep.GetDt();
		player = collisionMesh.PushOut( ae::PushOutParams(), player );

		// Render frame
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( player.sphere.center, forward, ae::Vec3( 0, 1, 0 ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, graphicsDevice.GetAspectRatio(), 0.5f, 1000.0f );
		graphicsDevice.Activate();
		graphicsDevice.Clear( ae::Color::Black() );
		ae::UniformList uniforms;
		uniforms.Set( "u_worldToProj", viewToProj * worldToView );
		uniforms.Set( "u_tex", &tex );
		vertexData.Draw( &shader, uniforms );
		graphicsDevice.Present();
		timeStep.Wait();
	}

	// Terminate
	fileSystem.DestroyAll();
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
Create a file called `main.mm` with the above contents and download [level.obj](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.obj) and [level.tga](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.tga), placing them in the same folder. Open the `Terminal` application. Type `cd ` (with a space after) and then drag the folder containing main.mm into the terminal window and press enter. With Xcode installed run the following:
```
clang++ -std=c++17 -fmodules -fcxx-modules -I ~/aether-game-utils main.mm
```

## Building on Windows
Download the repository with:
```
git clone https://github.com/johnhues/aether-game-utils.git C:\aether-game-utils
```
Create a file called `main.cpp` with the above contents and download [level.obj](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.obj) and [level.tga](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.tga), placing them in the same folder. With Visual Studio installed, right click in the containing directory and choose `Open in Terminal`. Run `"C:\Program Files\Microsoft Visual Studio\20XX\EDITION\VC\Auxiliary\Build\vcvars64.bat"`, replacing `20XX` with the year, and `EDITION` with `Community` etc. Finally build it with:
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
