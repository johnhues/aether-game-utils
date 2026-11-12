//------------------------------------------------------------------------------
// Triangle.cpp
//------------------------------------------------------------------------------
// aether as a guest of an Emscripten program, which owns the page, the WebGL
// context and the main loop. Built by test/emscripten/build.sh.
//------------------------------------------------------------------------------
#include "aether.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 color;
};

const Vertex kTriangleVerts[] = {
	{ ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), ae::Color::AetherRed().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Color::AetherGreen().GetLinearRGBA() },
	{ ae::Vec4( 0.0f, 0.5f, 0.0f, 1.0f ), ae::Color::AetherBlue().GetLinearRGBA() },
};

const uint16_t kTriangleIndices[] = { 0, 1, 2 };

struct Triangle
{
	ae::Shader shader;
	ae::VertexBuffer vertexData;
	float rotation = 0.0f;
	double prevTime = 0.0;
	uint32_t frames = 0;
};

void Frame( void* userData )
{
	Triangle* triangle = (Triangle*)userData;
	int32_t width = 0;
	int32_t height = 0;
	emscripten_get_canvas_element_size( "#canvas", &width, &height );
	const double time = ae::GetTime();
	triangle->rotation += (float)( time - triangle->prevTime );
	triangle->prevTime = time;

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, width, height );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	const float aspect = height ? width / (float)height : 1.0f;
	const ae::Matrix4 transform = ae::Matrix4::Scaling( 1.0f / aspect, 1.0f, 1.0f ) * ae::Matrix4::RotationZ( triangle->rotation );
	ae::UniformList uniformList;
	uniformList.Set( "u_modelToNdc", transform );
	triangle->vertexData.Bind( &triangle->shader, uniformList );
	triangle->vertexData.Draw();

	if( ++triangle->frames == 60 )
	{
		AE_INFO( "Rendered # frames", triangle->frames );
	}
}

int main()
{
	EmscriptenWebGLContextAttributes attributes;
	emscripten_webgl_init_context_attributes( &attributes );
	attributes.alpha = false;
	attributes.majorVersion = 2;
	const EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context( "#canvas", &attributes );
	AE_ASSERT_MSG( context > 0, "WebGL 2 is not available" );
	emscripten_webgl_make_context_current( context );

	static Triangle s_triangle;
	const char* vertShader = R"(
		AE_UNIFORM mat4 u_modelToNdc;
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec4 a_color;
		AE_OUT_HIGHP vec4 v_color;
		void main()
		{
			v_color = a_color;
			gl_Position = u_modelToNdc * a_position;
		})";
	const char* fragShader = R"(
		AE_IN_HIGHP vec4 v_color;
		void main()
		{
			AE_COLOR = v_color;
		})";
	s_triangle.shader.Initialize( vertShader, fragShader, nullptr, 0 );
	ae::VertexBuffer& vertexData = s_triangle.vertexData;
	vertexData.Initialize( sizeof( *kTriangleVerts ), sizeof( *kTriangleIndices ), countof( kTriangleVerts ), countof( kTriangleIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	vertexData.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
	vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
	vertexData.UploadVertices( 0, kTriangleVerts, countof( kTriangleVerts ) );
	vertexData.UploadIndices( 0, kTriangleIndices, countof( kTriangleIndices ) );
	s_triangle.prevTime = ae::GetTime();

	emscripten_set_main_loop_arg( Frame, &s_triangle, 0, false );
	return 0;
}
