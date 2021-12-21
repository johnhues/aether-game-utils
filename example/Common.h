//------------------------------------------------------------------------------
// Common.h
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/aether.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
inline void LoadPng( ae::Texture2D* texture, const char* file, ae::Texture::Filter filter, ae::Texture::Wrap wrap, bool autoGenerateMipmaps, bool isSRGB )
{
  uint32_t fileSize = ae::FileSystem::GetSize( file );
  AE_ASSERT_MSG( fileSize, "Could not load #", file );
  
  uint8_t* fileBuffer = (uint8_t*)malloc( fileSize );
  ae::FileSystem::Read( file, fileBuffer, fileSize );

  int32_t width = 0;
  int32_t height = 0;
  int32_t channels = 0;
  stbi_set_flip_vertically_on_load( 1 );
#if _AE_IOS_
  stbi_convert_iphone_png_to_rgb( 1 );
#endif
  bool is16BitImage = stbi_is_16_bit_from_memory( fileBuffer, fileSize );

  uint8_t* image;
  if (is16BitImage)
  {
     image = (uint8_t*)stbi_load_16_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  }
  else
  {
    image = stbi_load_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  }
  AE_ASSERT( image );

  ae::Texture::Format format;
  auto type = ae::Texture::Type::Uint8;
  switch ( channels )
  {
    case STBI_grey:
      format = ae::Texture::Format::R8;
        
      // for now only support R16Unorm
      if (is16BitImage)
      {
        format = ae::Texture::Format::R16_UNORM;
        type = ae::Texture::Type::Uint16;
      }
      break;
    case STBI_grey_alpha:
      AE_FAIL();
      break;
    case STBI_rgb:
      format = isSRGB ? ae::Texture::Format::RGB8_SRGB : ae::Texture::Format::RGB8;
      break;
    case STBI_rgb_alpha:
      format = isSRGB ? ae::Texture::Format::RGBA8_SRGB : ae::Texture::Format::RGBA8;
      break;
  }
  
  texture->Initialize( image, width, height, format, type, filter, wrap, autoGenerateMipmaps );
  
  stbi_image_free( image );
  free( fileBuffer );
}

//------------------------------------------------------------------------------
// SpriteRenderer example class
//------------------------------------------------------------------------------
class SpriteRenderer
{
public:
  void Initialize( uint32_t maxCount );
  void Terminate();
  void AddSprite( const ae::Matrix4& localToWorld, ae::Rect uvs, ae::Color color );
  void Clear();
  void Render( const ae::Matrix4& worldToProj, const ae::Texture2D* texture );
private:
  struct Vertex
  {
    ae::Vec4 pos;
    ae::Vec4 color;
    ae::Vec2 uv;
  };
  uint32_t m_count = 0;
  uint32_t m_maxCount = 0;
  ae::Shader m_shader;
  ae::VertexData m_vertexData;
};

//------------------------------------------------------------------------------
// SpriteRenderer member functions
//------------------------------------------------------------------------------
void SpriteRenderer::Initialize( uint32_t maxCount )
{
  m_maxCount = maxCount;
  m_count = 0;
  
  const char* vertShader = R"(
    AE_UNIFORM mat4 u_worldToProj;

    AE_IN_HIGHP vec4 a_position;
    AE_IN_HIGHP vec4 a_color;
    AE_IN_HIGHP vec2 a_uv;

    AE_OUT_HIGHP vec4 v_color;
    AE_OUT_HIGHP vec2 v_uv;

    void main()
    {
      v_color = a_color;
      v_uv = a_uv;
      gl_Position = u_worldToProj * a_position;
    }
  )";
  const char* fragShader = R"(
    AE_UNIFORM sampler2D u_tex;

    AE_IN_HIGHP vec4 v_color;
    AE_IN_HIGHP vec2 v_uv;

    void main()
    {
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;
    }
  )";
  m_shader.Initialize( vertShader, fragShader, nullptr, 0 );
  m_shader.SetBlending( true );
  
  m_vertexData.Initialize( sizeof(Vertex), 2, 4 * m_maxCount, 6 * m_maxCount, ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Dynamic, ae::VertexData::Usage::Dynamic );
  m_vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof(Vertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof(Vertex, color) );
  m_vertexData.AddAttribute( "a_uv", 2, ae::VertexData::Type::Float, offsetof(Vertex, uv) );
}

void SpriteRenderer::Terminate()
{
  m_count = 0;
  m_maxCount = 0;
  m_vertexData.Terminate();
  m_shader.Terminate();
}

void SpriteRenderer::AddSprite( const ae::Matrix4& localToWorld, ae::Rect uvs, ae::Color color )
{
  if ( m_count >= m_maxCount )
  {
    return;
  }
  
  const uint16_t indices[] = { 3, 0, 1, 3, 1, 2 };
  m_vertexData.AppendIndices( indices, countof(indices), m_vertexData.GetVertexCount() );

  ae::Vec2 min = uvs.GetMin();
  ae::Vec2 max = uvs.GetMax();
  Vertex verts[] =
  {
    { localToWorld * ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, min.y ) },
    { localToWorld * ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, min.y ) },
    { localToWorld * ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, max.y ) },
    { localToWorld * ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, max.y ) }
  };
  m_vertexData.AppendVertices( verts, countof(verts) );
  
  m_count++;
}

void SpriteRenderer::Clear()
{
  m_vertexData.ClearVertices();
  m_vertexData.ClearIndices();
  m_count = 0;
}

void SpriteRenderer::Render( const ae::Matrix4& worldToProj, const ae::Texture2D* texture )
{
  if ( !m_count )
  {
    return;
  }
  ae::UniformList uniforms;
  uniforms.Set( "u_worldToProj", worldToProj );
  uniforms.Set( "u_tex", texture );
  m_vertexData.Upload();
  m_vertexData.Render( &m_shader, uniforms, 0, m_count * 2 );
}
