# A single-header C++ collection of cross platform utilities

For quickly creating small games or for supplementing existing projects. It provides a largely complete set of small carefully designed and optimized building blocks for game development. Platforms currently supported are Mac, Windows, Linux, and Web. Core tenants of this library are to statisfy the (often times at odds!) goals of:
* Providing useful and performant utilities tailored for game development
* Providing utilities that naturally conform to game development best practices
* Not overstepping or imposing a particular game engine architecture

[`aether.h`](https://github.com/johnhues/aether-game-utils/blob/main/aether.h) is a single-header which contains all of the functionality provided by the library. Only system libraries are included, no other external dependencies are required. A CMakeLists.txt file is provided for building examples and tests, but is not needed or recommended for including in your own project. Instead [`aether.h`](https://github.com/johnhues/aether-game-utils/blob/main/aether.h) is designed to be included as a single header file. C++17 or later is required. Neither exceptions nor RTTI are used.

---
# Playable Example

---

<div class="ae-demo" data-demo="examples/22_read_me" align="center">
	<a href="https://johnhues.github.io/aether-game-utils/examples/22_read_me/" style="display: block;"><img src="https://github.com/johnhues/aether-game-utils/blob/main/examples/data/example.png?raw=true" width="75%" style="display: block; margin: 0 auto;"></a>
	<strong><a href="https://johnhues.github.io/aether-game-utils/examples/22_read_me/" style="display: block; text-align: center; font-size: 1.25em;">Click to play</a></strong>
</div>

---
# Example Source

---

```cpp
//------------------------------------------------------------------------------
// main.cpp (or main.mm on Apple platforms)
//------------------------------------------------------------------------------
#define AE_MAIN
#define AE_USE_MODULES
#include "aether.h"

const ae::Tag TAG_RESOURCE = "resource";
extern const char* kVertexShader;
extern const char* kFragmentShader;

//------------------------------------------------------------------------------
// Main function
//------------------------------------------------------------------------------
int main()
{
	// Platform and core systems
	ae::Window window;
	ae::GraphicsDevice graphicsDevice;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::FileSystem fileSystem;
	window.Initialize( 640, 320, false, true, "" );
	window.SetTitle( "Explore Game" );
	graphicsDevice.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( DATA_DIR, "ae", "Explore Game" );

	// Graphics resources and collision
	ae::VertexBuffer vertexData;
	ae::CollisionMesh<> collisionMesh = TAG_RESOURCE;
	ae::Texture2D tex;
	ae::Shader shader;
	shader.Initialize( kVertexShader, kFragmentShader, nullptr, 0 );
	shader.SetCulling( ae::Culling::CounterclockwiseFront );
	shader.SetDepthWrite( true );
	shader.SetDepthTest( true );
	const ae::File* geoFile = fileSystem.Read( ae::FileSystem::Root::Data, "level.obj", 2.5f );
	const ae::File* textureFile = fileSystem.Read( ae::FileSystem::Root::Data, "level.tga", 2.5f );

	// Player state
	ae::PushOutInfo player;
	player.sphere.radius = 0.7f;
	player.sphere.center = ae::Vec3( 0.0f, player.sphere.radius, 0.0f );
	float yaw = 0.0f;
	float pitch = 0.0f;
	uint32_t moveTouchId = 0;
	uint32_t lookTouchId = 0;

	//--------------------------------------------------------------------------
	// Game loop
	//--------------------------------------------------------------------------
	auto Update = [&]()
	{
		const float dt = ae::Min( timeStep.GetDt(), 0.03f );
		input.Pump();
		const ae::Array< ae::Touch, ae::kMaxTouches > newTouches = input.GetNewTouches();
		// Fullscreen
		if( input.GetPress( ae::Key::F ) ) { window.SetFullScreen( !window.GetFullScreen() ); input.SetCursorState( window.GetFullScreen() ? ae::CursorState::HiddenAndCaptured : ae::CursorState::Normal ); }
		if( input.GetPress( ae::Key::Escape ) ) { input.SetCursorState( ae::CursorState::Normal ); window.SetFullScreen( false ); }
		// Mouse capture
		if( input.GetMousePressLeft() ) { input.SetCursorState( ae::CursorState::HiddenAndCaptured ); }
		if( input.GetCursorLocked() ) { yaw -= input.mouse.movement.x * 0.001f; pitch += input.mouse.movement.y * 0.001f; }

		// Resource loading
		if( geoFile && geoFile->GetStatus() == ae::File::Status::Success )
		{
			ae::OBJLoader obj = TAG_RESOURCE;
			obj.Load( { geoFile->GetData(), geoFile->GetLength() } );
			obj.InitializeVertexData( { &vertexData } );
			obj.InitializeCollisionMesh( &collisionMesh, ae::Matrix4::Identity() );
			geoFile = nullptr;
		}
		if( textureFile && textureFile->GetStatus() == ae::File::Status::Success )
		{
			ae::TargaFile tga = TAG_RESOURCE;
			tga.Load( textureFile->GetData(), textureFile->GetLength() );
			tex.Initialize( tga.textureParams );
			textureFile = nullptr;
		}
		const bool resourcesLoaded = ( !geoFile && !textureFile );

		graphicsDevice.Activate();
		graphicsDevice.Clear( ae::Color::RGB( 0.15f, 0.13f, 0.25f ) );

		// Misc input
		const ae::Vec3 forward( -cosf( yaw ) * cosf( pitch ), sinf( pitch ), sinf( yaw ) * cosf( pitch ) );
		const ae::Vec3 right( forward.z, 0.0f, -forward.x );

		// Camera input
		const float displaySize = ae::Min( window.GetWidth(), window.GetHeight() );
		const ae::Touch* lookTouch = input.GetTouchById( lookTouchId );
		const ae::Touch* moveTouch = input.GetTouchById( moveTouchId );
		if( !lookTouch ){ const int32_t idx = newTouches.FindFn( [&]( const ae::Touch& t ){ return moveTouch || ( t.startPosition.x >= window.GetWidth() / 2.0f ); } ); if( idx >= 0 ) { lookTouchId = newTouches[ idx ].id; } }
		yaw -= input.gamepads[ 0 ].rightAnalog.x * 2.0f * dt;
		pitch += input.gamepads[ 0 ].rightAnalog.y * 2.0f * dt;
		if( lookTouch )
		{
			const ae::Vec2 touchDir = ae::Vec2( lookTouch->movement ) / ( displaySize * 0.35f );
			yaw -= touchDir.x;
			pitch += touchDir.y;
		}
		pitch = ae::Clip( pitch, -1.0f, 1.0f );
		
		// Movement input
		if( !moveTouch ){ const int32_t idx = newTouches.FindFn( [&]( const ae::Touch& t ){ return t.startPosition.x < window.GetWidth() / 2.0f; } ); if( idx >= 0 ) { moveTouchId = newTouches[ idx ].id; } }
		ae::Vec3 dir = ae::Vec3( 0.0f );
		if( input.Get( ae::Key::W ) ) { dir += forward; }
		if( input.Get( ae::Key::A ) ) { dir += right; }
		if( input.Get( ae::Key::S ) ) { dir -= forward; }
		if( input.Get( ae::Key::D ) ) { dir -= right; }
		dir += forward * input.gamepads[ 0 ].leftAnalog.y;
		dir -= right * input.gamepads[ 0 ].leftAnalog.x;
		if( moveTouch )
		{
			const ae::Vec2 touchDir = ( ae::Vec2( moveTouch->position - moveTouch->startPosition ) / ( displaySize * 0.15f ) ).TrimCopy( 1.0f );
			dir += forward * touchDir.y;
			dir -= right * touchDir.x;
		}
		
		// Physics
		player.velocity += dir.TrimCopy( 1.0f ) * dt * 15.0f;
		player.velocity.SetXZ( ae::DtSlerp( player.velocity.GetXZ(), 2.5f, dt, ae::Vec2( 0.0f ) ) );
		player.velocity.y -= dt * 20.0f;
		player.sphere.center += player.velocity * dt;
		player = collisionMesh.PushOut( ae::PushOutParams(), player );
		ae::RaycastParams raycastParams;
		raycastParams.source = player.sphere.center;
		raycastParams.ray = ae::Vec3( 0, player.sphere.radius * -1.1f, 0 );
		ae::RaycastResult r = collisionMesh.Raycast( raycastParams );
		if( r.hits.Length() )
		{
			player.sphere.center = r.hits[ 0 ].position +  ae::Vec3( 0, player.sphere.radius * 1.1f, 0 );
			player.velocity.y = ae::Max( 0.0f, player.velocity.y );
		}

		// Rendering
		ae::UniformList uniforms;
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( player.sphere.center, forward, ae::Vec3( 0, 1, 0 ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, graphicsDevice.GetAspectRatio(), 0.5f, 16.0f );
		uniforms.Set( "u_worldToProj", viewToProj * worldToView );
		uniforms.Set( "u_worldCameraPosition", player.sphere.center );
		uniforms.Set( "u_tex", &tex );
		vertexData.Bind( &shader, uniforms );
		vertexData.Draw();

		graphicsDevice.Present();
		timeStep.Tick();

		return !input.quit;
	};

#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while( Update() ) {}
#endif

	// Terminate
	fileSystem.DestroyAll();
	input.Terminate();
	graphicsDevice.Terminate();
	window.Terminate();
	return 0;
}

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertexShader = R"(
	AE_UNIFORM_HIGHP mat4 u_worldToProj;
	AE_IN_HIGHP vec3 a_position;
	AE_IN_HIGHP vec2 a_uv;
	AE_OUT_HIGHP vec3 v_position;
	AE_OUT_HIGHP vec2 v_uv;
	void main()
	{
		v_uv = a_uv;
		v_position = a_position;
		gl_Position = u_worldToProj * vec4( a_position, 1.0 );
	}
)";
		
const char* kFragmentShader = R"(
	AE_UNIFORM sampler2D u_tex;
	AE_UNIFORM_HIGHP vec3 u_worldCameraPosition;
	AE_IN_HIGHP vec3 v_position;
	AE_IN_HIGHP vec2 v_uv;
	void main()
	{
		float depth = clamp( length( u_worldCameraPosition - v_position ) / 16.0, 0.0, 1.0 );
		AE_COLOR = mix( vec4( 0.15, 0.13, 0.25, 1.0 ), AE_TEXTURE2D( u_tex, v_uv ), pow( 1.0 - depth, 1.5 ) );
	}
)";
```

This ~200 line example contains everythings needed for a cross-platform first person exploration game on: Web, MacOS, Windows, and iOS (and more platforms to come!).

<!-- # Games using aether game utils
[Bella Lunaux: The Case of the Diamond Panther Tiara](https://johnhues.itch.io/bella-lunaux-tiara)

<a href="https://johnhues.itch.io/bella-lunaux-tiara"><img src="https://img.itch.zone/aW1hZ2UvOTQzMzU2LzUzNDY4NTMucG5n/original/%2BypGAU.png" width="250"></a>

# Dependencies
* [Catch2](https://github.com/catchorg/Catch2) (Test framework)
* [ENet](http://enet.bespin.org/) (Optional)
* [OpenAL](https://github.com/kcat/openal-soft) (Optional) -->
