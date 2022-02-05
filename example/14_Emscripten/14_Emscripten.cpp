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
	{ ae::Vec4( -0.5f, -0.4f, 0.0f, 1.0f ), ae::Color::PicoRed().GetLinearRGBA(), ae::Vec2( 0.0f, 0.0f ) },
	{ ae::Vec4( 0.5f, -0.4f, 0.0f, 1.0f ), ae::Color::PicoGreen().GetLinearRGBA(), ae::Vec2( 1.0f, 0.0f ) },
	{ ae::Vec4( 0.0f, 0.4f, 0.0f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA(), ae::Vec2( 0.0f, 1.0f ) },
};

uint16_t kTriangleIndices[] = { 0, 1, 2 };

typedef struct Texture
{
	// Image
	char* url;
	int w, h;

	GLuint texture;
	GLuint target;
} Texture;

#define MAX_TEXTURES 256
static Texture textures[ MAX_TEXTURES ] = {};

extern "C"
{
	void load_texture_from_url( GLuint texture, const char* url, int* outWidth, int* outHeight );
}

static Texture* find_or_cache_url( const char* url )
{
	for ( int i = 0; i < MAX_TEXTURES; ++i ) // Naive O(n) lookup for tiny code size
	{
		if ( !strcmp( textures[ i ].url, url ) )
		{
			return textures + i;
		}
		else if ( !textures[ i ].url )
		{
			textures[ i ].url = strdup( url );
			textures[ i ].target = GL_TEXTURE_2D;
			glGenTextures( 1, &textures[ i ].texture );
			glBindTexture( textures[ i ].target, textures[ i ].texture );
			glTexParameteri( textures[ i ].target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( textures[ i ].target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( textures[ i ].target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( textures[ i ].target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			load_texture_from_url( textures[ i ].texture, url, &textures[ i ].w, &textures[ i ].h );
			return textures + i;
		}
	}
	return 0; // fail
}

class Game
{
public:
	ae::Vec2 charPos = ae::Vec2( 0.0f );
	ae::Vec2 charVel = ae::Vec2( 0.0f );
	float rotation = 0.0f;
	ae::Texture2D m_texture;

	Game() : charPos( 0.0f ), charVel( 0.0f ), rotation( 0.0f ) {}

	void Initialize()
	{
		Validate();

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

		find_or_cache_url( "moon.png" );

		Validate();
	}

	void Update( float dt )
	{
		Validate();

		static bool s_first = true;
		Texture* tex = find_or_cache_url( "moon.png" );
		if ( s_first && tex->w && tex->h )
		{
			
			AE_INFO( "# w:# h:#", tex->url, tex->w, tex->h );
			m_texture.m_texture = tex->texture;
			m_texture.m_hasAlpha = true;
			m_texture.m_width = tex->w;
			m_texture.m_height = tex->h;
			m_texture.m_target = tex->target;
			s_first = false;
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

		Validate();
	}

	void Render( float dt )
	{
		Validate();
		gfx.Activate();
		gfx.Clear( ae::Color::PicoDarkPurple() );

		if ( m_texture.GetTexture() )
		{
			ae::Matrix4 transform = ae::Matrix4::Scaling( ae::Vec3( 1.0f / gfx.GetAspectRatio(), 1.0f, 1.0f ) );
			transform *= ae::Matrix4::Translation( ae::Vec3( charPos, 0.0f ) );
			transform *= ae::Matrix4::RotationY( rotation );
			ae::UniformList uniformList;
			uniformList.Set( "u_modelToNdc", transform );
			uniformList.Set( "u_tex", &m_texture );
			vertexData.Render( &shader, uniformList );
		}

		gfx.Present();
		Validate();
	}

	void Validate()
	{
		AE_ASSERT( check0 == 0xCDCDCDCD );
		AE_ASSERT( check1 == 0xCDCDCDCD );
		AE_ASSERT( check2 == 0xCDCDCDCD );
		AE_ASSERT( check3 == 0xCDCDCDCD );
	}

	uint32_t check0 = 0xCDCDCDCD;
	ae::Window window;
	uint32_t check1 = 0xCDCDCDCD;
	ae::GraphicsDevice gfx;
	uint32_t check2 = 0xCDCDCDCD;
	ae::Input input;
	uint32_t check3 = 0xCDCDCDCD;
	ae::Shader shader;
	uint32_t check4 = 0xCDCDCDCD;
	ae::VertexData vertexData;
	uint32_t check5 = 0xCDCDCDCD;
};

EM_BOOL draw_frame( double t, void* userData )
{
	static double prevT = t;
	float dt = ( t - prevT ) * 0.001f;
	prevT = t;

	Game* game = (Game*)userData;
	game->Validate();
	game->Update( dt );
	game->Render( dt );
	game->Validate();

	return EM_TRUE;
}

int main()
{
	Game* game = ae::New< Game >( "game" );
	game->Initialize();
	game->Validate();
	emscripten_request_animation_frame_loop( &draw_frame, game );
}

#define AE_MAIN
#include "ae/aether.h"
