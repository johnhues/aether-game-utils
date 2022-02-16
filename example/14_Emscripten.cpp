#include "ae/aether.h"
#include <assert.h> // For assert()
#include <emscripten.h> // For emscripten_get_device_pixel_ratio()
#include <emscripten.h>
#include <emscripten/html5.h> // For Emscripten HTML5 WebGL context creation API
#include <emscripten/html5.h>
#include <math.h>
#include <memory.h>
#include <string.h> // For NULL and strcmp()
#include <webgl/webgl1.h> // For Emscripten WebGL API headers (see also webgl/webgl1_ext.h and webgl/webgl2.h)

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
// File forward declarations
//------------------------------------------------------------------------------
extern "C" {
void _AsyncFileLoader_LoadSuccess( void* arg, void* data, uint32_t length );
void _AsyncFileLoader_LoadFail( void* arg, uint32_t code, bool timeout );
}

//------------------------------------------------------------------------------
// LoadFileInfo class
//------------------------------------------------------------------------------
class LoadFileInfo
{
public:
	enum class Status
	{
		Success,
		Pending,
		NotFound,
		Timeout,
		Error
	};

	const char* GetUrl() const;
	Status GetStatus() const;
	//! Platform specific error code eg. 200, 404, etc. for http
	uint32_t GetCode() const;
	//! Null terminated for convenience
	const void* GetData() const;
	uint32_t GetLength() const;
	float GetElapsedTime() const;

private:
	friend void _AsyncFileLoader_LoadSuccess( void* arg, void* data, uint32_t length );
	friend void _AsyncFileLoader_LoadFail( void* arg, uint32_t code, bool timeout );
	friend class AsyncFileLoader;
	ae::Str256 m_url;
	uint8_t* m_data = nullptr;
	uint32_t m_length = 0;
	Status m_status = Status::Pending;
	uint32_t m_code = 0;
	double m_startTime = 0.0;
	double m_finishTime = 0.0;
};

//------------------------------------------------------------------------------
// AsyncFileLoader class
//------------------------------------------------------------------------------
class AsyncFileLoader
{
public:
	~AsyncFileLoader();
	//! Loads a file asynchronously. Returns a LoadFileInfo object to be freed
	//! later with AsyncFileLoader::Destroy(). A zero or negative /p timeoutSec
	//! value will disable the timeout.
	const LoadFileInfo* Load( const char* url, float timeoutSec );
	//! Destroys the given LoadFileInfo object returned by AsyncFileLoader::Load().
	void Destroy( const LoadFileInfo* info );
	//! Frees all existing LoadFileInfo objects. It is not safe to access any
	//! LoadFileInfo objects returned earlier by AsyncFileLoader::Load() after
	//! calling this.
	void DestroyAll();

	const LoadFileInfo* GetFile( uint32_t idx ) const;
	uint32_t GetFileCount() const;

private:
	ae::Array< LoadFileInfo* > m_files = AE_ALLOC_TAG_FILE;
};

//------------------------------------------------------------------------------
// LoadFileInfo member functions
//------------------------------------------------------------------------------
const char* LoadFileInfo::GetUrl() const
{
	return m_url.c_str();
}
LoadFileInfo::Status LoadFileInfo::GetStatus() const
{
	return m_status;
}
uint32_t LoadFileInfo::GetCode() const
{
	return m_code;
}
const void* LoadFileInfo::GetData() const
{
	return m_data;
}
uint32_t LoadFileInfo::GetLength() const
{
	return m_length;
}
float LoadFileInfo::GetElapsedTime() const
{
	return m_finishTime ? ( m_finishTime - m_startTime ) : ( ae::GetTime() - m_startTime );
}

//------------------------------------------------------------------------------
// AsyncFileLoader implementation
//------------------------------------------------------------------------------
extern "C" void EMSCRIPTEN_KEEPALIVE _AsyncFileLoader_LoadSuccess( void* arg, void* data, uint32_t length )
{
	LoadFileInfo* info = (LoadFileInfo*)arg;
	info->m_finishTime = ae::GetTime();
	info->m_data = (uint8_t*)ae::Allocate( TAG_ALL, length + 1, 8 );
	memcpy( info->m_data, data, length );
	info->m_data[ length ] = 0;
	info->m_length = length;

	info->m_status = LoadFileInfo::Status::Success;
	info->m_code = 200;
}

extern "C" void EMSCRIPTEN_KEEPALIVE _AsyncFileLoader_LoadFail( void* arg, uint32_t code, bool timeout )
{
	LoadFileInfo* info = (LoadFileInfo*)arg;
	info->m_finishTime = ae::GetTime();
	info->m_code = code;
	if ( timeout )
	{
		info->m_status = LoadFileInfo::Status::Timeout;
	}
	else
	{
		switch ( code )
		{
			case 404:
				info->m_status = LoadFileInfo::Status::NotFound;
				break;
			default:
				info->m_status = LoadFileInfo::Status::Error;
				break;
		}
	}
}

EM_JS( void, _AsyncFileLoader_LoadImpl, ( const char* url, void* arg, uint32_t timeoutMs ),
{
	var xhr = new XMLHttpRequest();
	xhr.timeout = timeoutMs;
	xhr.open('GET', UTF8ToString(url), true);
	xhr.responseType = 'arraybuffer';
	xhr.ontimeout = function xhr_ontimeout() {
		__AsyncFileLoader_LoadFail(arg, xhr.status, true);
	};
	xhr.onload = function xhr_onload() {
		if (xhr.status == 200) {
			if (xhr.response) {
				var byteArray = new Uint8Array(xhr.response);
				var buffer = _malloc(byteArray.length);
				HEAPU8.set(byteArray, buffer);
				__AsyncFileLoader_LoadSuccess(arg, buffer, byteArray.length);
				_free(buffer);
			}
			else {
				__AsyncFileLoader_LoadSuccess(arg, 0, 0); // Empty response but request succeeded
			}
			
		}
	};
	xhr.onerror = function xhrError() {
		__AsyncFileLoader_LoadFail(arg, xhr.status, false);
	};
	xhr.send(null);
} );

AsyncFileLoader::~AsyncFileLoader()
{
	AE_ASSERT_MSG( !m_files.Length(), "All files must be destroyed before destroying the loader" );
}

const LoadFileInfo* AsyncFileLoader::Load( const char* url, float timeoutSec )
{
	double t = ae::GetTime();
	LoadFileInfo* info = ae::New< LoadFileInfo >( TAG_ALL );
	info->m_url = url;
	info->m_startTime = t;
	uint32_t timeoutMs;
	if ( timeoutSec <= 0.0f )
	{
		timeoutMs = 0.0f;
	}
	else
	{
		timeoutMs = timeoutSec * 1000.0f;
		timeoutMs = ae::Max( 1u, timeoutMs ); // Prevent rounding down to infinite timeout
	}
	m_files.Append( info );
	_AsyncFileLoader_LoadImpl( url, info, timeoutMs );
	return info;
}

void AsyncFileLoader::Destroy( const LoadFileInfo* info )
{
	if ( info )
	{
		m_files.Remove( m_files.Find( info ) );
		ae::Free( info->m_data );
		ae::Delete( info );
	}
}

void AsyncFileLoader::DestroyAll()
{
	for ( auto info : m_files )
	{
		ae::Free( info->m_data );
		ae::Delete( info );
	}
	m_files.Clear();
}

const LoadFileInfo* AsyncFileLoader::GetFile( uint32_t idx ) const
{
	return m_files[ idx ];
}

uint32_t AsyncFileLoader::GetFileCount() const
{
	return m_files.Length();
}

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

	AsyncFileLoader fileLoader;
	const LoadFileInfo* m_moonFile = nullptr;
	const LoadFileInfo* m_testFile = nullptr;
	const LoadFileInfo* m_licenseFile = nullptr;

	Game() : charPos( 0.0f ), charVel( 0.0f ), rotation( 0.0f ) {}

	void Initialize()
	{
		printf( "Initialize\n" );
		window.Initialize( 0, 0, "WebGL Demo", true );
		gfx.Initialize( &window );
		input.Initialize( &window );
		shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
		vertexData.Initialize( sizeof( *kTriangleVerts ), sizeof( *kTriangleIndices ), countof( kTriangleVerts ), countof( kTriangleIndices ), ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Static, ae::VertexData::Usage::Static );
		vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( Vertex, pos ) );
		vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof( Vertex, color ) );
		vertexData.AddAttribute( "a_uv", 2, ae::VertexData::Type::Float, offsetof( Vertex, uv ) );
		vertexData.SetVertices( kTriangleVerts, countof( kTriangleVerts ) );
		vertexData.SetIndices( kTriangleIndices, countof( kTriangleIndices ) );

		uint8_t data[] = { 255, 255, 255 };
		m_texture.Initialize( &data, 1, 1, ae::Texture::Format::RGB8, ae::Texture::Type::Uint8, ae::Texture::Filter::Nearest, ae::Texture::Wrap::Repeat, false );

		m_moonFile = fileLoader.Load( "moon.tga", 2.5f );
		m_testFile = fileLoader.Load( "test.txt", 2.5f );
		m_licenseFile = fileLoader.Load( "https://raw.githubusercontent.com/johnhues/aether-game-utils/master/LICENSE", 2.5f );
	}

	void Update( float dt )
	{
		if ( m_moonFile && m_moonFile->GetStatus() != LoadFileInfo::Status::Pending )
		{
			if ( m_moonFile->GetStatus() == LoadFileInfo::Status::Success )
			{
				ae::TargaFile targa = TAG_ALL;
				if ( targa.Load( (const uint8_t*)m_moonFile->GetData(), m_moonFile->GetLength() ) )
				{
					AE_INFO( "Loaded '#' (#x#)", m_moonFile->GetUrl(), targa.textureParams.width, targa.textureParams.height );
					m_texture.Initialize( targa.textureParams );
				}
			}
			fileLoader.Destroy( m_moonFile );
			m_moonFile = nullptr;
		}

		const LoadFileInfo** files[] = { &m_moonFile, &m_testFile, &m_licenseFile };
		for ( const LoadFileInfo** _file : files )
		{
			const LoadFileInfo*& file = *_file;
			if ( file && file->GetStatus() != LoadFileInfo::Status::Pending )
			{
				switch ( file->GetStatus() )
				{
					case LoadFileInfo::Status::Success:
						AE_INFO( "Loaded file: '#' Length: # (#) (#sec)", file->GetUrl(), file->GetLength(), file->GetCode(), file->GetElapsedTime() );
						if ( strcmp( ae::FileSystem::GetFileExtFromPath( file->GetUrl() ), "tga" ) != 0 )
						{
							printf( "%s\n", (const char*)file->GetData() );
						}
						break;
					case LoadFileInfo::Status::NotFound:
						AE_ERR( "File not found: '#' (#) (#sec)", file->GetUrl(), file->GetCode(), file->GetElapsedTime() );
						break;
					case LoadFileInfo::Status::Timeout:
						AE_ERR( "File load timeout: '#' (#) (#sec)", file->GetUrl(), file->GetCode(), file->GetElapsedTime() );
						break;
					default:
						AE_ERR( "Error loading file: '#' (#) (#sec)", file->GetUrl(), file->GetCode(), file->GetElapsedTime() );
						break;
				}
				fileLoader.Destroy( file );
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

	void Render( float dt )
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
};

EM_BOOL draw_frame( double t, void* userData )
{
	static double prevT = t;
	float dt = ( t - prevT ) * 0.001f;
	prevT = t;

	Game* game = (Game*)userData;
	game->Update( dt );
	game->Render( dt );

	return EM_TRUE;
}

int main()
{
	Game* game = ae::New< Game >( "game" );
	game->Initialize();
	emscripten_request_animation_frame_loop( &draw_frame, game );
}
