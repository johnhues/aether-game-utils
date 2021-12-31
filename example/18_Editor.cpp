//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/aether.h"
#include "ae/Editor.h"
#include "ae/Entity.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ALL = "all";

//------------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------------
class Object : public ae::Inheritor< ae::Component, Object >
{
public:
	virtual void Initialize( class Game* game ) {}
	virtual void Terminate( class Game* game ) {}
	virtual void Update( class Game* game ) {}
	virtual void Render( class Game* game ) {}
	bool initialized = false;
};
AE_REGISTER_CLASS( Object );

//------------------------------------------------------------------------------
// Avatar
//------------------------------------------------------------------------------
class Avatar : public ae::Inheritor< Object, Avatar >
{
public:
	void Initialize( class Game* game ) override;
	void Terminate( class Game* game ) override;
	void Update( class Game* game ) override;
	void Render( class Game* game ) override;
	
	ae::Vec3 position = ae::Vec3( 0.0f );
	ae::Vec3 velocity = ae::Vec3( 0.0f );
	float facingAngle = 0.0f;
	float radius = 0.5f;
	double lastTouchedGround = 0.0;
	bool onGround = false;
	ae::Vec3 initialPos = ae::Vec3( 0.0f );
	ae::Vec3 bottomPos = ae::Vec3( 0.0f );
	ae::Vec3 groundPos = ae::Vec3( 0.0f, 0.0f, -INFINITY );
	ae::Vec3 groundNormal = ae::Vec3( 0.0f );
	ae::Vec3 inputDir = ae::Vec3( 0.0f );
};
AE_REGISTER_CLASS( Avatar );
AE_REGISTER_CLASS_PROPERTY( Avatar, ae_editor_type );
AE_REGISTER_CLASS_VAR( Avatar, position );
AE_REGISTER_CLASS_VAR( Avatar, radius );

//------------------------------------------------------------------------------
// Mesh
//------------------------------------------------------------------------------
class Mesh : public ae::Inheritor< Object, Mesh >
{
public:
	ae::Str32 name;
	ae::Matrix4 transform;
};
AE_REGISTER_CLASS( Mesh );
AE_REGISTER_CLASS_PROPERTY( Mesh, ae_editor_type );
AE_REGISTER_CLASS_PROPERTY_VALUE( Mesh, ae_mesh_resource, name );
AE_REGISTER_CLASS_VAR( Mesh, name );
AE_REGISTER_CLASS_VAR( Mesh, transform );

//------------------------------------------------------------------------------
// Game
//------------------------------------------------------------------------------
class Game
{
public:
	void Initialize( int argc, char* argv[] );
	void Run();
	
	float GetDt() const { return m_dt; }
	void GetUniforms( ae::UniformList* uniformList );
	
	// System
	ae::Editor editor = TAG_ALL;
	ae::Window window;
	ae::Input input;
	ae::GraphicsDevice gfx;
	ae::TimeStep timeStep;
	ae::FileSystem fs;
	ae::DebugLines debugLines;
	ae::Registry registry = TAG_ALL;

	// Resources
	ae::VertexData bunnyVertexData;
	ae::VertexData avatarVertexData;
	ae::CollisionMesh bunnyCollision = TAG_ALL;
	ae::Shader meshShader;
	ae::Shader avatarShader;
	ae::Texture2D spacesuitTex;

	// Gameplay
	bool slowDt = false;
	ae::Vec3 cameraPos = ae::Vec3( 10.0f );
	ae::Vec3 cameraDir = ae::Vec3( -1.0f ).SafeNormalizeCopy();
	ae::Color skyColor = ae::Color::PicoBlue().ScaleRGB( 1.0f );
	ae::Matrix4 worldToView = ae::Matrix4::Identity();
	ae::Matrix4 viewToProj = ae::Matrix4::Identity();
	ae::Matrix4 worldToProj = ae::Matrix4::Identity();
	class Avatar* avatar = nullptr;
	
private:
	float m_dt = 0.0f;
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
		light += u_ambLight;
		AE_COLOR.rgb = v_color.rgb * light;
#ifdef DIFFUSE
		AE_COLOR.rgb *= AE_TEXTURE2D( u_tex, v_uv ).rgb;
#endif
		AE_COLOR.a = v_color.a;
	})";

void LoadOBj( const char* fileName, const ae::FileSystem* fs, ae::VertexData* vertexDataOut, ae::CollisionMesh* collisionOut, ae::EditorMesh* editorMeshOut )
{
	ae::OBJFile objFile = TAG_ALL;
	uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( TAG_ALL, fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	objFile.Load( fileBuffer.Data(), fileBuffer.Length() );
	
	if ( objFile.vertices.Length() )
	{
		if ( vertexDataOut )
		{
			vertexDataOut->Initialize(
				sizeof(*objFile.vertices.Begin()), sizeof(*objFile.indices.Begin()),
				objFile.vertices.Length(), objFile.indices.Length(),
				ae::VertexData::Primitive::Triangle,
				ae::VertexData::Usage::Static, ae::VertexData::Usage::Static
			);
			vertexDataOut->AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( ae::OBJFile::Vertex, position ) );
			vertexDataOut->AddAttribute( "a_normal", 4, ae::VertexData::Type::Float, offsetof( ae::OBJFile::Vertex, normal ) );
			vertexDataOut->AddAttribute( "a_uv", 2, ae::VertexData::Type::Float, offsetof( ae::OBJFile::Vertex, texture ) );
			vertexDataOut->AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof( ae::OBJFile::Vertex, color ) );
			vertexDataOut->SetVertices( objFile.vertices.Begin(), objFile.vertices.Length() );
			vertexDataOut->SetIndices( objFile.indices.Begin(), objFile.indices.Length() );
		}
		
		if ( collisionOut )
		{
			collisionOut->Clear();
			ae::CollisionMesh::Params params;
			params.positions = objFile.vertices.Begin()->position.data;
			params.positionCount = objFile.vertices.Length();
			params.positionStride = sizeof( *objFile.vertices.Begin() );
			params.indices = objFile.indices.Begin();
			params.indexCount = objFile.indices.Length();
			params.indexSize = sizeof( *objFile.indices.Begin() );
			collisionOut->Load( params );
		}
		
		if ( editorMeshOut )
		{
			editorMeshOut->verts.Reserve( objFile.vertices.Length() );
			for ( uint32_t i = 0; i < objFile.vertices.Length(); i++ )
			{
				editorMeshOut->verts.Append( objFile.vertices[ i ].position.GetXYZ() );
			}
			editorMeshOut->indices.Append( objFile.indices.Begin(), objFile.indices.Length() );
		}
	}
}

void LoadTarga( const char* fileName, const ae::FileSystem* fs, ae::Texture2D* tex )
{
	ae::TargaFile tgaFile = TAG_ALL;
	uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( TAG_ALL, fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	tgaFile.Load( fileBuffer.Data(), fileBuffer.Length() );
	tex->Initialize( tgaFile.textureParams );
}

//------------------------------------------------------------------------------
// Game member functions
//------------------------------------------------------------------------------
void Game::Initialize( int argc, char* argv[] )
{
	fs.Initialize( "data", "johnhues", "test" );
	
	ae::EditorParams editorParams;
	editorParams.argc = argc;
	editorParams.argv = argv;
	editorParams.loadMeshFn = [&]( const char* resourceId )
	{
		ae::EditorMesh result = TAG_ALL;
		LoadOBj( resourceId, &fs, nullptr, nullptr, &result );
		return result;
	};

	editor.Initialize( editorParams );
	window.Initialize( 1280, 720, false, true );
	input.Initialize( &window );
	gfx.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debugLines.Initialize( 10 * 1024 );
	
	LoadOBj( "bunny.obj", &fs, &bunnyVertexData, &bunnyCollision, nullptr );
	meshShader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	meshShader.SetDepthTest( true );
	meshShader.SetDepthWrite( true );
	meshShader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );
	
	LoadOBj( "character.obj", &fs, &avatarVertexData, nullptr, nullptr );
	const char* avatarShaderDefines[] = { "#define DIFFUSE 1" };
	avatarShader.Initialize( kVertShader, kFragShader, avatarShaderDefines, countof(avatarShaderDefines) );
	avatarShader.SetDepthTest( true );
	avatarShader.SetDepthWrite( true );
	avatarShader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );
	LoadTarga( "character.tga", &fs, &spacesuitTex );
	
	editor.Read( "example.level" );
	registry.Load( &editor );
}

void Game::Run()
{
	while ( !input.quit )
	{
		m_dt = timeStep.GetDt();
		if ( m_dt > timeStep.GetTimeStep() * 5.0f )
		{
			m_dt = 0.00001f;
		}
		if ( slowDt )
		{
			m_dt *= 0.1f;
		}
		
		input.Pump();
		editor.Update();
		
		if ( editor.levelDidChange )
		{
			registry.CallFn< Object >( [&]( Object* o ){ o->Terminate( this ); } );
			registry.Load( &editor );
		}
		
		if ( input.Get( ae::Key::Tilde ) && !input.GetPrev( ae::Key::Tilde ) )
		{
			editor.Launch();
		}
		if ( input.Get( ae::Key::Q ) && !input.GetPrev( ae::Key::Q ) )
		{
			slowDt = !slowDt;
		}
		
		registry.CallFn< Object >( [&]( Object* o )
		{
			if ( !o->initialized )
			{
				o->Initialize( this );
				o->initialized = true;
			}
		} );
		registry.CallFn< Object >( [&]( Object* o ){ o->Update( this ); } );
		
		gfx.Activate();
		gfx.Clear( skyColor );
		
		if ( avatar && ( avatar->inputDir.Length() > 0.1f || avatar->velocity.Length() > 0.5f ) )
		{
			ae::Vec3 camOffset = ae::Vec3( 6.0f, 6.0f, 4.0f );
			ae::Vec3 camTarget = avatar->position + ae::Vec3( 0.0f, 0.0f, 2.0f );
			cameraPos = ae::DtLerp( cameraPos, 1.0f, GetDt(), camTarget + camOffset );
			cameraDir = ae::DtLerp( cameraDir, 1.0f, GetDt(), camTarget - cameraPos );
			cameraDir.SafeNormalize();
		}
		
		worldToView = ae::Matrix4::WorldToView( cameraPos, cameraDir, ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		viewToProj = ae::Matrix4::ViewToProjection( 0.9f, gfx.GetAspectRatio(), 1.0f, 1000.0f );
		worldToProj = viewToProj * worldToView;
		
		ae::UniformList uniformList;
		GetUniforms( &uniformList );
		uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
		registry.CallFn< Mesh >( [&]( Mesh* m )
		{
			uniformList.Set( "u_worldToProj", worldToProj * m->transform );
			uniformList.Set( "u_normalToWorld", m->transform.GetNormalMatrix() );
			bunnyVertexData.Render( &meshShader, uniformList );
		} );
		
		registry.CallFn< Object >( [&]( Object* o ){ o->Render( this ); } );
		
		debugLines.Render( worldToProj );
		gfx.Present();
		timeStep.Wait();
	}
}

void Game::GetUniforms( ae::UniformList* uniformList )
{
	uniformList->Set( "u_lightColor", ae::Color::PicoPeach().ScaleRGB( 1.0f ).GetLinearRGB() );
	uniformList->Set( "u_lightDir", ae::Vec3( -7.0f, 5.0f, -3.0f ).NormalizeCopy() );
	uniformList->Set( "u_ambLight", skyColor.GetLinearRGB() );
}

//------------------------------------------------------------------------------
// Avatar member functions
//------------------------------------------------------------------------------
void Avatar::Initialize( Game* game )
{
	initialPos = position;
	game->avatar = this;
}

void Avatar::Terminate( Game* game )
{
	game->avatar = nullptr;
}

void Avatar::Update( Game* game )
{
	if ( game->input.Get( ae::Key::R ) && !game->input.GetPrev( ae::Key::R ) )
	{
		position = initialPos;
		velocity = ae::Vec3( 0.0f );
	}

	inputDir = ae::Vec3( 0.0f );
	inputDir.x -= ( game->input.Get( ae::Key::A ) || game->input.Get( ae::Key::Left ) ) ? 1.0f : 0.0f;
	inputDir.x += ( game->input.Get( ae::Key::D ) || game->input.Get( ae::Key::Right ) ) ? 1.0f : 0.0f;
	inputDir.y += ( game->input.Get( ae::Key::W ) || game->input.Get( ae::Key::Up ) ) ? 1.0f : 0.0f;
	inputDir.y -= ( game->input.Get( ae::Key::S ) || game->input.Get( ae::Key::Down ) ) ? 1.0f : 0.0f;
	inputDir.SafeNormalize();
	ae::Vec3 cameraFlat = ae::Vec3( game->cameraDir.x, game->cameraDir.y, 0.0f );
	inputDir.AddRotationXY( cameraFlat.GetAngleBetween( ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
	
	if ( inputDir.Length() > 0.01f )
	{
		float targetFacingAngle = inputDir.GetXY().GetAngle() + ae::HALF_PI;
		facingAngle = ae::DtLerpAngle( facingAngle, 2.5f, game->GetDt(), targetFacingAngle );
	}
	
	if ( onGround )
	{
		onGround = ( ae::GetTime() - lastTouchedGround ) < 0.15;
		if ( onGround && game->input.Get( ae::Key::Space ) )
		{
			velocity.z += 7.5f;
			onGround = false;
		}
	}
	
	float rayLength = 1.0f;
	float speed = onGround ? 20.0f : 10.0f;
	ae::Vec3 accel( inputDir.x * speed, inputDir.y * speed, -10.0f );
	velocity += game->GetDt() * accel;
	float friction;
	if ( !onGround )
	{
		friction = 0.2f;
	}
	else if ( ( inputDir.Length() < 0.1f || inputDir.GetXY().Dot( velocity.GetXY() ) < 0.5f ) )
	{
		friction = 3.5f;
	}
	else
	{
		friction = 1.0f;
	}
	velocity.x = ae::DtLerp( velocity.x, friction, game->GetDt(), 0.0f );
	velocity.y = ae::DtLerp( velocity.y, friction, game->GetDt(), 0.0f );
	position += game->GetDt() * velocity;
	
	ae::CollisionMesh::RaycastParams rayParams;
	rayParams.source = ae::Vec3( position );
	rayParams.direction = ae::Vec3( 0.0f, 0.0f, -1.0f );
	ae::CollisionMesh::RaycastResult outResult;
	
	ae::CollisionMesh::PushOutParams pushOutParams;
	ae::CollisionMesh::PushOutInfo pushOutInfo;
	pushOutInfo.sphere.center = position;
	pushOutInfo.sphere.radius = radius;
	pushOutInfo.velocity = velocity;
	
	game->registry.CallFn< Mesh >( [&]( Mesh* m )
	{
		rayParams.transform = m->transform;
		outResult = game->bunnyCollision.Raycast( rayParams, outResult );

		pushOutParams.transform = m->transform;
		pushOutInfo = game->bunnyCollision.PushOut( pushOutParams, pushOutInfo );
	} );
	
	ae::Vec4 finalPos( 0.0f );
	if ( pushOutInfo.hits.Length() )
	{
		for ( const auto& hit : pushOutInfo.hits )
		{
			if ( hit.normal.Dot( ae::Vec3( 0.0f, 0.0f, 1.0f ) ) > 0.75f )
			{
				lastTouchedGround = ae::GetTime();
				onGround = true;
			}
		}
		finalPos += ae::Vec4( pushOutInfo.sphere.center, 1.0f );
		velocity = pushOutInfo.velocity;
	}
	if ( outResult.hits.Length() )
	{
		if ( outResult.hits[ 0 ].distance < rayLength )
		{
			finalPos += ae::Vec4( outResult.hits[ 0 ].position + ae::Vec3( 0.0f, 0.0f, rayLength ), 1.0f );
			velocity.z = ae::Max( 0.0f, velocity.z );
			lastTouchedGround = ae::GetTime();
			onGround = true;
		}
		
		groundPos = outResult.hits[ 0 ].position;
		groundNormal = outResult.hits[ 0 ].normal;
	}
	else
	{
		groundPos = position;
		groundPos.z = -INFINITY;
	}
	if ( finalPos.w )
	{
		position = finalPos.GetXYZ() / finalPos.w;
	}
	
	bottomPos = position - ae::Vec3( 0.0f, 0.0f, rayLength );
	
	game->debugLines.AddSphere( position, radius, ae::Color::Red(), 16 );
	game->debugLines.AddLine( position, position - ae::Vec3( 0.0f, 0.0f, rayLength ), ae::Color::Red() );
	game->debugLines.AddCircle( groundPos, groundNormal, radius, ae::Color::Black(), 16 );
}

void Avatar::Render( Game* game )
{
	ae::Matrix4 transform = ae::Matrix4::Translation( bottomPos );
	transform *= ae::Matrix4::RotationZ( facingAngle );
	transform *= ae::Matrix4::RotationX( ae::HALF_PI );
	transform *= ae::Matrix4::Scaling( 0.015f );
	ae::UniformList uniformList;
	game->GetUniforms( &uniformList );
	uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
	uniformList.Set( "u_worldToProj", game->worldToProj * transform );
	uniformList.Set( "u_normalToWorld", transform.GetNormalMatrix() );
	uniformList.Set( "u_tex", &game->spacesuitTex );
	game->avatarVertexData.Render( &game->avatarShader, uniformList );
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	AE_INFO( "Init" );

	Game game;
	game.Initialize( argc, argv );
	game.Run();

	return 0;
}
