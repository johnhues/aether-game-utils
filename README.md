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
	// AE_UNIFORM sampler2D u_tex;
	AE_IN_HIGHP vec2 v_uv;
	void main()
	{
		AE_COLOR = vec4( 1, 0, 0, 1 );
	}
)";

int main()
{
	ae::Window window;
	ae::GraphicsDevice graphicsDevice;
	ae::Input input;
	ae::FileSystem fileSystem;
	ae::TimeStep timeStep;
	ae::OBJFile obj = TAG_EXAMPLE;
	ae::Shader shader;
	ae::VertexArray vertexData;
	ae::CollisionMesh<> collisionMesh = TAG_EXAMPLE;

	window.Initialize( 1280, 800, false, true );
	window.SetTitle( "Game" );
	graphicsDevice.Initialize( &window );
	input.Initialize( &window );
	fileSystem.Initialize( "", "ae", "Game" );
	timeStep.SetTimeStep( 1.0f / 60.0f );

	const ae::File* file = fileSystem.Read( ae::FileSystem::Root::Data, "level.obj", 2.5f );
	while ( file->GetStatus() == ae::File::Status::Pending ) { sleep( 0 ); }
	if ( file->GetStatus() != ae::File::Status::Success )
	{
		ae::ShowMessage( "Could not load 'level.obj'" );
		std::exit( EXIT_FAILURE );
	}
	obj.Load( file->GetData(), file->GetLength() );
	obj.InitializeVertexData( { &vertexData } );
	vertexData.Upload();
	shader.Initialize( kVertexShader, kFragmentShader, nullptr, 0 );
	shader.SetCulling( ae::Culling::CounterclockwiseFront );
	shader.SetDepthWrite( true );
	shader.SetDepthTest( true );
	obj.InitializeCollisionMesh( &collisionMesh, ae::Matrix4::Identity() );

	ae::Vec3 pos = ae::Vec3( 0.0f );
	ae::Vec3 vel = ae::Vec3( 0.0f );
	float angle = 0.0f;
	float angularVel = 0.0f;
	while ( !input.quit )
	{
		input.Pump();
		graphicsDevice.Activate();
		graphicsDevice.Clear( ae::Color::Black() );

		vel = ae::DtLerp( vel, 2.0f, timeStep.GetDt(), ae::Vec3( 0.0f ) );
		angularVel = ae::DtLerp( angularVel, 3.0f, timeStep.GetDt(), 0.0f );
		ae::Vec3 forward( -cosf( angle ), 0.0f, sinf( angle ) );
		if ( input.Get( ae::Key::Up ) ) { vel += forward * timeStep.GetDt() * 20.0f; }
		if ( input.Get( ae::Key::Down ) ) { vel -= forward * timeStep.GetDt() * 20.0f; }
		if ( input.Get( ae::Key::Left ) ) { angularVel += timeStep.GetDt() * 10.0f; }
		if ( input.Get( ae::Key::Right ) ) { angularVel -= timeStep.GetDt() * 10.0f; }
		pos += vel * timeStep.GetDt();
		angle += angularVel * timeStep.GetDt();

		ae::PushOutInfo info;
		info.sphere.center = pos;
		info.sphere.radius = 0.75f;
		info.velocity = vel;
		info = collisionMesh.PushOut( ae::PushOutParams(), info );
		pos = info.sphere.center;
		vel = info.velocity;

		ae::UniformList uniforms;
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( pos, forward, ae::Vec3( 0, 1, 0 ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, graphicsDevice.GetAspectRatio(), 0.5f, 1000.0f );
		uniforms.Set( "u_worldToProj", viewToProj * worldToView );
		vertexData.Draw( &shader, uniforms );

		graphicsDevice.Present();
		timeStep.Wait();
	}

	fileSystem.Destroy( file );
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
Create a file called `main.mm` with the above contents and download [level.obj](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/bunny.obj), placing them in the same folder. Open the `Terminal` application. Type `cd ` (with a space after) and then drag the folder containing main.mm into the terminal window and press enter. With Xcode installed run the following:
```
clang++ -std=c++17 -fmodules -fcxx-modules -I ~/aether-game-utils main.mm
```

## Building on Windows
Download the repository with:
```
git clone https://github.com/johnhues/aether-game-utils.git C:\aether-game-utils
```
Create a file called `main.cpp` with the above contents and download [level.obj](https://raw.githubusercontent.com/johnhues/aether-game-utils/master/example/data/bunny.obj), placing them in the same folder. With Visual Studio installed, right click in the containing directory and choose `Open in Terminal`. Run `"C:\Program Files\Microsoft Visual Studio\20XX\EDITION\VC\Auxiliary\Build\vcvars64.bat"`, replacing `20XX` with the year, and `EDITION` with `Community` etc. Finally build it with:
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
