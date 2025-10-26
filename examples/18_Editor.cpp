//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "ae/Editor.h"
#include "ae/Entity.h"
#include "ae/MeshEditorPlugin.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ALL = "all";

//------------------------------------------------------------------------------
// Component
//------------------------------------------------------------------------------
class Component : public ae::Inheritor< ae::Component, Component >
{
public:
	virtual void Initialize( class Game* game ) {}
	virtual void Terminate( class Game* game ) {}
	virtual void Update( class Game* game ) {}
	virtual void Render( class Game* game ) {}
	bool initialized = false;
};
AE_REGISTER_CLASS( Component );

//------------------------------------------------------------------------------
// Avatar
//------------------------------------------------------------------------------
class Avatar : public ae::Inheritor< Component, Avatar >
{
public:
	void Update( class Game* game ) override;

	ae::Vec3 position = ae::Vec3( 0.0f );
	ae::Vec3 velocity = ae::Vec3( 0.0f );
	float yaw = 0.0f;
	float pitch = 0.0f;
	uint32_t moveTouchId = 0;
	uint32_t lookTouchId = 0;

	ae::Vec3 m_src = ae::Vec3( 0.0f );
	ae::Vec3 m_ray = ae::Vec3( 0.0f );
	ae::Optional< ae::RaycastResult::Hit > m_hit;
};
AE_REGISTER_CLASS( Avatar );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (Avatar), (ae, EditorTypeAttribute), {} );
AE_REGISTER_CLASS_VAR( Avatar, position );

//------------------------------------------------------------------------------
// Mesh
//------------------------------------------------------------------------------
class Mesh : public ae::Inheritor< Component, Mesh >
{
public:
	void Render( class Game* game ) override;
	ae::Str32 name;
	ae::Matrix4 transform = ae::Matrix4::Identity();
};
AE_REGISTER_CLASS( Mesh );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (Mesh), (ae, EditorTypeAttribute), {} );
AE_REGISTER_CLASS_VAR( Mesh, name );
AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( (Mesh), name, (ae, EditorMeshResourceAttribute), {} );
AE_REGISTER_CLASS_VAR( Mesh, transform );

//------------------------------------------------------------------------------
// Game
//------------------------------------------------------------------------------
class Game
{
public:
	bool Initialize( int argc, char* argv[] );
	void Run();
	void GetUniforms( ae::UniformList* uniformList );
	
	// System
	ae::Registry registry = TAG_ALL;
	ae::Editor editor = TAG_ALL;
	ae::Window window;
	ae::Input input;
	ae::GraphicsDevice gfx;
	ae::TimeStep timeStep;
	ae::FileSystem fs;
	ae::DebugLines debugLines = TAG_ALL;

	// Resources
	ae::VertexBuffer bunnyVertexData;
	ae::CollisionMesh<> bunnyCollision = TAG_ALL;
	ae::Shader meshShader;

	// Gameplay
	ae::Vec3 cameraPos = ae::Vec3( 10.0f );
	ae::Vec3 cameraDir = ae::Vec3( -1.0f ).SafeNormalizeCopy();
	ae::Color skyColor = ae::Color::PicoBlue();
	ae::Matrix4 worldToView = ae::Matrix4::Identity();
	ae::Matrix4 viewToProj = ae::Matrix4::Identity();
	ae::Matrix4 worldToProj = ae::Matrix4::Identity();
};

//------------------------------------------------------------------------------
// GameEditorPlugin
//------------------------------------------------------------------------------
class GameEditorPlugin : public ae::MeshEditorPlugin
{
public:
	GameEditorPlugin( Game* game ) : ae::MeshEditorPlugin( TAG_ALL ), m_game( game ) {}
	void OnEvent( const ae::EditorEvent& event ) override;
	ae::Optional< ae::EditorMesh > TryLoad( const char* resourceStr ) override;
private:
	Game* m_game = nullptr;
};

//------------------------------------------------------------------------------
// Resources
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_worldToProj;
	AE_UNIFORM mat4 u_normalToWorld;
	AE_UNIFORM vec4 u_color;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_normal;
	AE_IN_HIGHP vec2 a_uv;
	AE_IN_HIGHP vec4 a_color;
	AE_OUT_HIGHP vec3 v_normal;
	AE_OUT_HIGHP vec2 v_uv;
	AE_OUT_HIGHP vec4 v_color;
	void main()
	{
		v_color = a_color * u_color;
		v_normal = ( u_normalToWorld * a_normal ).xyz;
		v_uv = a_uv;
		gl_Position = u_worldToProj * a_position;
	})";

const char* kFragShader = R"(
	AE_UNIFORM vec3 u_ambLight;
	AE_UNIFORM vec3 u_lightColor;
	AE_UNIFORM vec3 u_lightDir;
#ifdef DIFFUSE
	uniform sampler2D u_tex;
#endif
	AE_IN_HIGHP vec3 v_normal;
	AE_IN_HIGHP vec2 v_uv;
	AE_IN_HIGHP vec4 v_color;
	void main()
	{
		vec3 light = u_lightColor * max( 0.0, dot( -u_lightDir, normalize( v_normal ) ) );
		light += u_ambLight + u_lightColor * 0.2;
		AE_COLOR.rgb = v_color.rgb * light;
#ifdef DIFFUSE
		AE_COLOR.rgb *= AE_TEXTURE2D( u_tex, v_uv ).rgb;
#endif
		AE_COLOR.a = v_color.a;
	})";

void LoadObj( const char* fileName, const ae::FileSystem* fs, ae::VertexBuffer* vertexDataOut, ae::CollisionMesh<>* collisionOut, ae::EditorMesh* editorMeshOut )
{
	ae::OBJLoader objFile = TAG_ALL;
	const uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	objFile.Load( { fileBuffer.Data(), fileBuffer.Length() } );
	if( objFile.vertices.Length() )
	{
		if( vertexDataOut )
		{
			vertexDataOut->Initialize(
				sizeof(*objFile.vertices.Data()), sizeof(*objFile.indices.Data()),
				objFile.vertices.Length(), objFile.indices.Length(),
				ae::Vertex::Primitive::Triangle,
				ae::Vertex::Usage::Static, ae::Vertex::Usage::Static
			);
			vertexDataOut->AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, position ) );
			vertexDataOut->AddAttribute( "a_normal", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, normal ) );
			vertexDataOut->AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, texture ) );
			vertexDataOut->AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, color ) );
			vertexDataOut->UploadVertices( 0, objFile.vertices.Data(), objFile.vertices.Length() );
			vertexDataOut->UploadIndices( 0, objFile.indices.Data(), objFile.indices.Length() );
		}
		
		objFile.InitializeCollisionMesh( collisionOut );
		
		if( editorMeshOut )
		{
			editorMeshOut->verts.Reserve( objFile.vertices.Length() );
			for( uint32_t i = 0; i < objFile.vertices.Length(); i++ )
			{
				editorMeshOut->verts.Append( objFile.vertices[ i ].position.GetXYZ() );
			}
			editorMeshOut->indices.AppendArray( objFile.indices.Data(), objFile.indices.Length() );
		}
	}
}

void LoadTarga( const char* fileName, const ae::FileSystem* fs, ae::Texture2D* tex )
{
	ae::TargaFile tgaFile = TAG_ALL;
	const uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	tgaFile.Load( fileBuffer.Data(), fileBuffer.Length() );
	tex->Initialize( tgaFile.textureParams );
}

//------------------------------------------------------------------------------
// Game member functions
//------------------------------------------------------------------------------
bool Game::Initialize( int argc, char* argv[] )
{
	fs.Initialize( "data", "ae", "editor" );
	
	ae::EditorParams editorParams( argc, argv, &registry );
	// editorParams.run = true;
	editor.AddPlugin< GameEditorPlugin >( this );
	if( editor.Initialize( editorParams ) )
	{
		return false; // Exit, the editor has forked, ran, closed, and returned gracefully
	}
	window.Initialize( 1280, 720, false, true, true );
	window.SetTitle( "Press '~' to Open the Editor" );
	input.Initialize( &window );
	gfx.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debugLines.Initialize( 10 * 1024 );
	
	LoadObj( "bunny.obj", &fs, &bunnyVertexData, &bunnyCollision, nullptr );
	meshShader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	meshShader.SetDepthTest( true );
	meshShader.SetDepthWrite( true );
	meshShader.SetCulling( ae::Culling::CounterclockwiseFront );

	ae::Str256 levelPath;
	if( fs.GetRootDir( ae::FileSystem::Root::Data, &levelPath ) )
	{
		ae::FileSystem::AppendToPath( &levelPath, "example.level" );
		editor.QueueRead( levelPath.c_str() );
	}
	return true;
}

void Game::Run()
{
	while( !input.quit )
	{
		// Update
		input.Pump();
		editor.Update();
		if( input.GetMousePressLeft() ) { input.SetMouseCaptured( true ); }
		if( input.GetPress( ae::Key::F ) ) { window.SetFullScreen( !window.GetFullScreen() ); input.SetMouseCaptured( window.GetFullScreen() ); }
		if( input.GetPress( ae::Key::Escape ) ) { input.SetMouseCaptured( false ); window.SetFullScreen( false ); }
		if( input.Get( ae::Key::Tilde ) && !input.GetPrev( ae::Key::Tilde ) ) { editor.Launch(); }
		registry.CallFn< Component >( [&]( Component* component )
		{
			if( !component->initialized )
			{
				component->Initialize( this );
				component->initialized = true;
			}
		} );
		registry.CallFn< Component >( [&]( Component* c ){ c->Update( this ); } );
		
		// Render
		gfx.Activate();
		gfx.Clear( skyColor );
		worldToView = ae::Matrix4::WorldToView( cameraPos, cameraDir, ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		viewToProj = ae::Matrix4::ViewToProjection( 1.1f, gfx.GetAspectRatio(), 0.5f, 500.0f );
		worldToProj = viewToProj * worldToView;
		registry.CallFn< Component >( [&]( Component* c ){ c->Render( this ); } );
		debugLines.Render( worldToProj );
		gfx.Present();
		timeStep.Tick();
	}
}

void Game::GetUniforms( ae::UniformList* uniformList )
{
	uniformList->Set( "u_lightColor", ae::Color::PicoPeach().ScaleRGB( 1.0f ).GetLinearRGB() );
	uniformList->Set( "u_lightDir", ae::Vec3( -7.0f, 5.0f, -3.0f ).NormalizeCopy() );
	uniformList->Set( "u_ambLight", skyColor.GetLinearRGB() );
}

//------------------------------------------------------------------------------
// GameEditorPlugin member functions
//------------------------------------------------------------------------------
void GameEditorPlugin::OnEvent( const ae::EditorEvent& event )
{
	ae::MeshEditorPlugin::OnEvent( event );
	if( event.type == ae::EditorEventType::LevelUnload )
	{
		m_game->registry.CallFn< Component >( [&]( Component* c ){ c->Terminate( m_game ); } );
		m_game->registry.Clear();
	}
}

ae::Optional< ae::EditorMesh > GameEditorPlugin::TryLoad( const char* resourceStr )
{
	ae::EditorMesh result = TAG_ALL;
	LoadObj( resourceStr, &m_game->fs, nullptr, nullptr, &result );
	return result;
}

//------------------------------------------------------------------------------
// Avatar member functions
//------------------------------------------------------------------------------
void Avatar::Update( Game* game )
{
	const float dt = game->timeStep.GetDt();
	ae::Input& input = game->input;
	ae::Window& window = game->window;

	// Camera Input
	const ae::Array< ae::Touch, ae::kMaxTouches > newTouches = input.GetNewTouches();
	const float displaySize = ae::Min( window.GetWidth(), window.GetHeight() );
	const ae::Vec3 forward( cosf( yaw ) * cosf( pitch ), sinf( yaw ) * cosf( pitch ), sinf( pitch ) );
	const ae::Vec3 right( forward.y, -forward.x, 0.0f );
	const ae::Touch* lookTouch = input.GetTouchById( lookTouchId );
	const ae::Touch* moveTouch = input.GetTouchById( moveTouchId );
	if( input.GetMouseCaptured() ) { yaw -= input.mouse.movement.x * 0.001f; pitch += input.mouse.movement.y * 0.001f; }
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
	dir += ( forward * ( input.Get( ae::Key::W ) - input.Get( ae::Key::S ) ) + right * ( input.Get( ae::Key::D ) - input.Get( ae::Key::A ) ) ).SafeNormalizeCopy() * ( input.Get( ae::Key::LeftShift ) ? 0.333f : 1.0f );
	dir += ( forward * input.gamepads[ 0 ].leftAnalog.y - right * input.gamepads[ 0 ].leftAnalog.x );
	if( moveTouch )
	{
		const ae::Vec2 touchDir = ( ae::Vec2( moveTouch->position - moveTouch->startPosition ) / ( displaySize * 0.15f ) ).TrimCopy( 1.0f );
		dir += ( forward * touchDir.y - right * touchDir.x );
	}

	// Physics
	velocity += ae::Vec3( dir.x, dir.y, 0.0f ).TrimCopy( 1.0f ) * dt * 15.0f;
	velocity.SetXY( ae::DtSlerp( velocity.GetXY(), 2.5f, dt, ae::Vec2( 0.0f ) ) );
	position += velocity * dt;
	ae::RaycastResult raycastResult;
	game->registry.CallFn< Mesh >( [ & ]( Mesh* m ) { raycastResult = game->bunnyCollision.Raycast( ae::RaycastParams{ .transform = m->transform, .source = position, .ray = ae::Vec3( 0.0f, 0.0f, -0.9f ) }, raycastResult ); } );
	if( raycastResult.hits.Length() )
	{
		position.z = raycastResult.hits[ 0 ].position.z + 0.8f;
		velocity.z = ae::Max( 0.0f, velocity.z );
	}
	else
	{
		velocity.z -= dt * 10.0f;
	}

	ae::PushOutInfo pushOutInfo = { .sphere = ae::Sphere( position, 0.6f ), .velocity = velocity };
	game->registry.CallFn< Mesh >( [ & ]( Mesh* m ) { pushOutInfo = game->bunnyCollision.PushOut( ae::PushOutParams{ .transform = m->transform }, pushOutInfo ); } );
	position = pushOutInfo.sphere.center;

	// Update game camera
	game->cameraPos = position + ae::Vec3( 0.0f, 0.0f, 0.02f * cos( ae::GetTime() * 15.0 ) * velocity.GetXY().Length() );
	game->cameraDir = forward;
}

void Mesh::Render( Game* game )
{
	ae::UniformList uniformList;
	game->GetUniforms( &uniformList );
	uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
	uniformList.Set( "u_worldToProj", game->worldToProj * transform );
	uniformList.Set( "u_normalToWorld", transform.GetNormalMatrix() );
	game->bunnyVertexData.Bind( &game->meshShader, uniformList );
	game->bunnyVertexData.Draw();
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	Game game;
	if( game.Initialize( argc, argv ) )
	{
		game.Run();
	}
	return 0;
}
