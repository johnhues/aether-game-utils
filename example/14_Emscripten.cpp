#include "ae/aether.h"

const ae::Tag TAG_ALL = "all";

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_modelToNdc;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_color;
	AE_IN_HIGHP vec2 a_uv;
	AE_OUT_HIGHP vec4 v_color;
	AE_OUT_HIGHP vec2 v_uv;
	void main()
	{
		v_color = a_color;
		v_uv = a_uv;
		gl_Position = u_modelToNdc * a_position;
	})";

const char* kFragShader = R"(
	uniform sampler2D u_tex;
	AE_IN_HIGHP vec4 v_color;
	AE_IN_HIGHP vec2 v_uv;
	void main()
	{
		AE_COLOR = v_color * AE_TEXTURE2D( u_tex, v_uv );
	})";

//------------------------------------------------------------------------------
// Triangle
//------------------------------------------------------------------------------
struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 color;
	ae::Vec2 uv;
};

Vertex kTriangleVerts[] = {
	{ ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), ae::Color::PicoRed().GetLinearRGBA(), ae::Vec2( 0.0f, 0.0f ) },
	{ ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Color::PicoGreen().GetLinearRGBA(), ae::Vec2( 1.0f, 0.0f ) },
	{ ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA(), ae::Vec2( 0.0f, 1.0f ) },
	{ ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA(), ae::Vec2( 1.0f, 1.0f ) },
};

uint16_t kTriangleIndices[] = { 0, 1, 2, 3, 1, 2 };

//------------------------------------------------------------------------------
// Game class
//------------------------------------------------------------------------------
class Game
{
public:
	ae::Vec2 charPos = ae::Vec2( 0.0f );
	ae::Vec2 charVel = ae::Vec2( 0.0f );
	float rotation = 0.0f;
	ae::Texture2D m_texture;

	ae::FileSystem fileSystem;
	const ae::AsyncFile* m_moonFile = nullptr;
	const ae::AsyncFile* m_testFile = nullptr;
	const ae::AsyncFile* m_licenseFile = nullptr;

	Game() : charPos( 0.0f ), charVel( 0.0f ), rotation( 0.0f ) {}

	void Initialize()
	{
		printf( "Initialize\n" );
		window.Initialize( 1280, 720, "WebGL Demo", true );
		gfx.Initialize( &window );
		input.Initialize( &window );
		shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
		vertexData.Initialize( sizeof( *kTriangleVerts ), sizeof( *kTriangleIndices ), countof( kTriangleVerts ), countof( kTriangleIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
		vertexData.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
		vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
		vertexData.AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof( Vertex, uv ) );
		vertexData.SetVertices( kTriangleVerts, countof( kTriangleVerts ) );
		vertexData.SetIndices( kTriangleIndices, countof( kTriangleIndices ) );
		timeStep.SetTimeStep( 1.0f / 60.0f );

		uint8_t data[] = { 255, 255, 255 };
		m_texture.Initialize( &data, 1, 1, ae::Texture::Format::RGB8, ae::Texture::Type::Uint8, ae::Texture::Filter::Nearest, ae::Texture::Wrap::Repeat, false );

		fileSystem.Initialize( "data", "ae", "emscripten" );
		m_moonFile = fileSystem.LoadAsyncFile( ae::FileSystem::Root::Data, "moon.tga", 2.5f );
		m_testFile = fileSystem.LoadAsyncFile( ae::FileSystem::Root::Data, "test.txt", 2.5f );
		m_licenseFile = fileSystem.LoadAsyncFile( "https://raw.githubusercontent.com/johnhues/aether-game-utils/master/LICENSE", 2.5f );
	}

	bool Tick()
	{
		Update();
		Render();
		timeStep.Wait();
		return !input.quit;
	}

	void Update()
	{
		const float dt = timeStep.GetDt();
		
		if ( m_moonFile && m_moonFile->GetStatus() != ae::AsyncFile::Status::Pending )
		{
			if ( m_moonFile->GetStatus() == ae::AsyncFile::Status::Success )
			{
				ae::TargaFile targa = TAG_ALL;
				if ( targa.Load( (const uint8_t*)m_moonFile->GetData(), m_moonFile->GetLength() ) )
				{
					AE_INFO( "Loaded '#' (#x#)", m_moonFile->GetUrl(), targa.textureParams.width, targa.textureParams.height );
					m_texture.Initialize( targa.textureParams );
				}
			}
			fileSystem.Destroy( m_moonFile );
			m_moonFile = nullptr;
		}

		const ae::AsyncFile** files[] = { &m_moonFile, &m_testFile, &m_licenseFile };
		for ( const ae::AsyncFile** _file : files )
		{
			const ae::AsyncFile*& file = *_file;
			if ( file && file->GetStatus() != ae::AsyncFile::Status::Pending )
			{
				switch ( file->GetStatus() )
				{
					case ae::AsyncFile::Status::Success:
						AE_INFO( "Loaded file: '#' Length: # (#) (#sec)", file->GetUrl(), file->GetLength(), file->GetCode(), file->GetElapsedTime() );
						if ( strcmp( ae::FileSystem::GetFileExtFromPath( file->GetUrl() ), "tga" ) != 0 )
						{
							printf( "%s\n", (const char*)file->GetData() );
						}
						break;
					case ae::AsyncFile::Status::NotFound:
						AE_ERR( "File not found: '#' (#) (#sec)", file->GetUrl(), file->GetCode(), file->GetElapsedTime() );
						break;
					case ae::AsyncFile::Status::Timeout:
						AE_ERR( "File load timeout: '#' (#) (#sec)", file->GetUrl(), file->GetCode(), file->GetElapsedTime() );
						break;
					default:
						AE_ERR( "Error loading file: '#' (#) (#sec)", file->GetUrl(), file->GetCode(), file->GetElapsedTime() );
						break;
				}
				fileSystem.Destroy( file );
				file = nullptr;
			}
		}
		
		input.Pump();

		float speed = 5.0f;

		ae::Vec2 accel( 0.0f );
		accel.x += input.Get( ae::Key::Right ) ? 1.0f : 0.0f;
		accel.x -= input.Get( ae::Key::Left ) ? 1.0f : 0.0f;
		accel.y += input.Get( ae::Key::Up ) ? 1.0f : 0.0f;
		accel.y -= input.Get( ae::Key::Down ) ? 1.0f : 0.0f;
		accel.SafeNormalize();
		accel *= speed;

		charVel = ae::DtLerp( charVel, 2.0f, dt, ae::Vec2( 0.0f ) );
		charVel += accel * dt;
		charPos += charVel * dt;

		rotation += dt;
	}

	void Render()
	{
		gfx.Activate();
		gfx.Clear( ae::Color::Black() );

		ae::Matrix4 transform = ae::Matrix4::Scaling( ae::Vec3( 1.0f / gfx.GetAspectRatio(), 1.0f, 1.0f ) );
		transform *= ae::Matrix4::Translation( ae::Vec3( charPos, 0.0f ) );
		transform *= ae::Matrix4::RotationY( rotation );
		ae::UniformList uniformList;
		uniformList.Set( "u_modelToNdc", transform );
		uniformList.Set( "u_tex", &m_texture );
		vertexData.Render( &shader, uniformList );

		gfx.Present();
	}

	ae::Window window;
	ae::GraphicsDevice gfx;
	ae::Input input;
	ae::Shader shader;
	ae::VertexData vertexData;
	ae::TimeStep timeStep;
};

int main()
{
	Game* game = ae::New< Game >( "game" );
	game->Initialize();
#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* game ) { ((Game*)game)->Tick(); }, game, 0, 1 );
#else
	while ( game->Tick() ) {}
#endif
	ae::Delete( game );
}
