#define AE_MAIN
#define AE_USE_MODULES
#include "aether.h"
const ae::Tag TAG_RESOURCE = "resource";
extern const char* kVertexShader;
extern const char* kFragmentShader;
#ifndef DATA_DIR
	#define DATA_DIR ""
#endif

int main()
{
	ae::Window window;
	ae::GraphicsDevice graphicsDevice;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::FileSystem fileSystem;
	window.Initialize( 640, 320, false, true );
	window.SetTitle( "Game" );
	graphicsDevice.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( DATA_DIR, "ae", "Game" );

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

	ae::PushOutInfo player;
	player.sphere.radius = 0.7f;
	player.sphere.center = ae::Vec3( 0.0f, player.sphere.radius, 0.0f );
	float angle = 0.0f;
	float angularVel = 0.0f;

	auto Update = [&]()
	{
		const float dt = ae::Min( timeStep.GetDt(), 0.03f );
		input.Pump();

		// Async load for web
		if ( geoFile && geoFile->GetStatus() == ae::File::Status::Success )
		{
			ae::OBJFile obj = TAG_RESOURCE;
			obj.Load( geoFile->GetData(), geoFile->GetLength() );
			obj.InitializeVertexData( { &vertexData } );
			obj.InitializeCollisionMesh( &collisionMesh, ae::Matrix4::Identity() );
			geoFile = nullptr;
		}
		if ( textureFile && textureFile->GetStatus() == ae::File::Status::Success )
		{
			ae::TargaFile tga = TAG_RESOURCE;
			tga.Load( textureFile->GetData(), textureFile->GetLength() );
			tex.Initialize( tga.textureParams );
			textureFile = nullptr;
		}

		graphicsDevice.Activate();
		graphicsDevice.Clear( ae::Color::Black() );
		if ( fileSystem.GetFileStatusCount( ae::File::Status::Success ) == fileSystem.GetFileCount() )
		{
			// Input
			const ae::Vec3 forward( -cosf( angle ), 0.0f, sinf( angle ) );
			const ae::Vec2 mouse = ( ae::Vec2( input.mouse.position ) / window.GetWidth() ) - ae::Vec2( 0.5f, 0.15f );
			if ( input.Get( ae::Key::Up ) || ( input.mouse.leftButton && mouse.y > 0.1f ) ) { player.velocity += forward * dt * 20.0f; }
			if ( input.Get( ae::Key::Down ) || ( input.mouse.leftButton && mouse.y < -0.1f ) ) { player.velocity -= forward * dt * 20.0f; }
			if ( input.Get( ae::Key::Left ) || ( input.mouse.leftButton && mouse.x < -0.2f ) ) { angularVel += dt * 10.0f; }
			if ( input.Get( ae::Key::Right ) || ( input.mouse.leftButton && mouse.x > 0.2f ) ) { angularVel -= dt * 10.0f; }
			if ( input.Get( ae::Key::Meta ) && input.GetPress( ae::Key::F ) ) { window.SetFullScreen( !window.GetFullScreen() ); }

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
			vertexData.Bind( &shader, uniforms );
			vertexData.Draw();
		}
		graphicsDevice.Present();
		timeStep.Tick();

		return !input.quit;
	};

#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while ( Update() ) {}
#endif

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
