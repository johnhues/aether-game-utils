<p align="center">
	<img src="https://github.com/johnhues/aether-game-utils/blob/master/example/data/Icon.png?raw=true" width="250">
</p>

# aether game utils
[`aether.h`](https://github.com/johnhues/aether-game-utils/blob/master/aether.h) is a single-header collection of cross platform C++ utilities for quickly creating small games. Platforms currently supported are Mac, Windows, Linux, and Web. Core tenants of this library are to statisfy the (often times at odds!) goals of:
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
The following are instructions to get started with aether-game-utils. This example is a single source file which only includes [`aether.h`](https://github.com/johnhues/aether-game-utils/blob/master/aether.h) (which handles linking the required system libraries). It is a complete first person exploration experience with textured geometry and basic kinematic physics. Use the arrow keys to move.

`main.cpp` (or `main.mm` on Mac)
```cpp
#define AE_MAIN
#define AE_USE_MODULES
#include "aether.h"
const ae::Tag TAG_EXAMPLE = "example";
extern const char* kVertexShader;
extern const char* kFragmentShader;

int main()
{
	// Initialization
	ae::Window window;
	ae::GraphicsDevice graphicsDevice;
	ae::Input input;
	ae::TimeStep timeStep;
	window.Initialize( 640, 320, false, true );
	window.SetTitle( "Game" );
	graphicsDevice.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	// File loading
	ae::FileSystem fileSystem;
	fileSystem.Initialize( "", "ae", "Game" );
	const ae::File* geoFile = fileSystem.Read( ae::FileSystem::Root::Data, "level.obj", 2.5f );
	const ae::File* textureFile = fileSystem.Read( ae::FileSystem::Root::Data, "level.tga", 2.5f );
	while ( fileSystem.AnyPending() ) { std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) ); }
	if ( !fileSystem.AllSuccess() ) { std::exit( EXIT_FAILURE ); }
	ae::OBJFile obj = TAG_EXAMPLE;
	ae::TargaFile tga = TAG_EXAMPLE;
	tga.Load( textureFile->GetData(), textureFile->GetLength() );
	obj.Load( geoFile->GetData(), geoFile->GetLength() );

	// Resources
	ae::CollisionMesh<> collisionMesh = TAG_EXAMPLE;
	ae::Shader shader;
	ae::Texture2D tex;
	ae::VertexArray vertexData;
	obj.InitializeCollisionMesh( &collisionMesh, ae::Matrix4::Identity() );
	shader.Initialize( kVertexShader, kFragmentShader, nullptr, 0 );
	shader.SetCulling( ae::Culling::CounterclockwiseFront );
	shader.SetDepthWrite( true );
	shader.SetDepthTest( true );
	tex.Initialize( tga.textureParams );
	obj.InitializeVertexData( { &vertexData } );
	vertexData.Upload();

	// Game state
	ae::PushOutInfo player;
	player.sphere.radius = 0.7f;
	player.sphere.center = ae::Vec3( 0.0f, player.sphere.radius, 0.0f );
	float angle = 0.0f;
	float angularVel = 0.0f;
	
	while ( !input.quit )
	{
		const float dt = ae::Min( timeStep.GetDt(), 0.03f );
		const ae::Vec3 forward( -cosf( angle ), 0.0f, sinf( angle ) );

		// Input
		input.Pump();
		if ( input.Get( ae::Key::Up ) ) { player.velocity += forward * dt * 25.0f; }
		if ( input.Get( ae::Key::Down ) ) { player.velocity -= forward * dt * 25.0f; }
		if ( input.Get( ae::Key::Left ) ) { angularVel += dt * 15.0f; }
		if ( input.Get( ae::Key::Right ) ) { angularVel -= dt * 15.0f; }

		// Physics
		player.velocity.SetXZ( ae::DtSlerp( player.velocity.GetXZ(), 2.5f, dt, ae::Vec2( 0.0f ) ) );
		angularVel = ae::DtLerp( angularVel, 3.5f, dt, 0.0f );
		player.velocity.y -= dt * 20.0f;
		player.sphere.center += player.velocity * dt;
		angle += angularVel * dt;
		player = collisionMesh.PushOut( ae::PushOutParams(), player );
		ae::RaycastParams raycastParams;
		raycastParams.source = player.sphere.center;
		raycastParams.ray = ae::Vec3( 0, player.sphere.radius * -1.1f, 0 );
		ae::RaycastResult r = collisionMesh.Raycast( raycastParams );
		if ( r.hits.Length() )
		{
			player.sphere.center = r.hits[ 0 ].position +  ae::Vec3( 0, player.sphere.radius * 1.1f, 0 );
			player.velocity.y = ae::Max( 0.0f, player.velocity.y );
		}

		// Rendering
		ae::UniformList uniforms;
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( player.sphere.center, forward, ae::Vec3( 0, 1, 0 ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, graphicsDevice.GetAspectRatio(), 0.5f, 1000.0f );
		uniforms.Set( "u_worldToProj", viewToProj * worldToView );
		uniforms.Set( "u_tex", &tex );
		graphicsDevice.Activate();
		graphicsDevice.Clear( ae::Color::Black() );
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

const char* kVertexShader = R"(
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

const char* kFragmentShader = R"(
	AE_UNIFORM sampler2D u_tex;
	AE_IN_HIGHP vec2 v_uv;
	void main()
	{
		AE_COLOR = AE_TEXTURE2D( u_tex, v_uv );
	}
)";
```

## Building on Mac
Create a file called `main.mm` with the above contents and download [aether.h](https://github.com/johnhues/aether-game-utils/blob/master/aether.h), [level.obj](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.obj) and [level.tga](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.tga) to the same folder. Open the `Terminal` application. Type `cd` (with a space after it) and then drag the folder containing main.mm into the terminal window and press enter. With Xcode installed run the following:
```
clang++ -std=c++17 -fmodules -fcxx-modules main.mm && ./a.out
```

## Building on Windows
Create a file called `main.cpp` with the above contents and download [aether.h](https://github.com/johnhues/aether-game-utils/blob/master/aether.h), [level.obj](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.obj) and [level.tga](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/level.tga) to the same folder. With Visual Studio installed, right click inside the containing directory and choose `Open in Terminal`. Run `"C:\Program Files\Microsoft Visual Studio\20XX\EDITION\VC\Auxiliary\Build\vcvars64.bat"`, replacing `20XX` with the year, and `EDITION` with `Community` etc. Finally build it with:
```
cl /std:c++17 -D_UNICODE -DUNICODE main.cpp
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
