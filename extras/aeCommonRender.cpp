//------------------------------------------------------------------------------
// aeRenderCommon.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#include "ae/aeRender.h"

//------------------------------------------------------------------------------
// aeRender constants
//------------------------------------------------------------------------------
const ae::Vec3 aeQuadVertPos[ aeQuadVertCount ] = {
  ae::Vec3( -0.5f, -0.5f, 0.0f ),
  ae::Vec3( 0.5f, -0.5f, 0.0f ),
  ae::Vec3( 0.5f, 0.5f, 0.0f ),
  ae::Vec3( -0.5f, 0.5f, 0.0f )
};

const ae::Vec2 aeQuadVertUvs[ aeQuadVertCount ] = {
  ae::Vec2( 0.0f, 0.0f ),
  ae::Vec2( 1.0f, 0.0f ),
  ae::Vec2( 1.0f, 1.0f ),
  ae::Vec2( 0.0f, 1.0f )
};

const aeQuadIndex aeQuadIndices[ aeQuadIndexCount ] = {
  3, 0, 1,
  3, 1, 2
};
 
//------------------------------------------------------------------------------
// aeSpriteRender member functions
//------------------------------------------------------------------------------
aeSpriteRender::aeSpriteRender()
{
  m_count = 0;
  m_maxCount = 0;
  m_sprites = nullptr;
  m_vertices = nullptr;
  
  m_shaderAll = nullptr;
  m_shaderOpaque = nullptr;
  m_shaderTransparent = nullptr;
  
  m_blending = false;
  m_depthTest = false;
  m_depthWrite = false;
  m_sorting = false;
}

aeSpriteRender::~aeSpriteRender()
{
	Destroy();
}

void aeSpriteRender::Initialize( uint32_t maxCount )
{
  AE_ASSERT( maxCount );

  m_maxCount = maxCount;
  m_count = 0;
  m_sprites = ae::NewArray< Sprite >( AE_ALLOC_TAG_RENDER, m_maxCount );
  m_vertices = ae::NewArray< Vertex >( AE_ALLOC_TAG_RENDER, m_maxCount * aeQuadVertCount );

  m_vertexData.Initialize( sizeof(Vertex), sizeof(uint16_t), aeQuadVertCount * maxCount, aeQuadIndexCount * maxCount, ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Dynamic, ae::Vertex::Usage::Static );
  m_vertexData.AddAttribute( "a_position", 3, ae::Vertex::Type::Float, offsetof(Vertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof(Vertex, color) );
  m_vertexData.AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof(Vertex, uv) );

  ae::Scratch< uint16_t > scratch( m_maxCount * aeQuadIndexCount );
  uint16_t* indices = scratch.Data();
  for( uint32_t i = 0; i < m_maxCount; i++ )
  {
    indices[ i * aeQuadIndexCount + 0 ] = (uint16_t)( i * aeQuadVertCount + aeQuadIndices[ 0 ] );
    indices[ i * aeQuadIndexCount + 1 ] = (uint16_t)( i * aeQuadVertCount + aeQuadIndices[ 1 ] );
    indices[ i * aeQuadIndexCount + 2 ] = (uint16_t)( i * aeQuadVertCount + aeQuadIndices[ 2 ] );
    indices[ i * aeQuadIndexCount + 3 ] = (uint16_t)( i * aeQuadVertCount + aeQuadIndices[ 3 ] );
    indices[ i * aeQuadIndexCount + 4 ] = (uint16_t)( i * aeQuadVertCount + aeQuadIndices[ 4 ] );
    indices[ i * aeQuadIndexCount + 5 ] = (uint16_t)( i * aeQuadVertCount + aeQuadIndices[ 5 ] );
  }
  m_vertexData.UploadIndices( 0, indices, m_maxCount * aeQuadIndexCount );
}

void aeSpriteRender::Destroy()
{
  if( m_shaderAll )
  {
    ae::Delete( m_shaderAll );
    m_shaderAll = nullptr;
  }
  
  if( m_shaderOpaque )
  {
    ae::Delete( m_shaderOpaque );
    m_shaderOpaque = nullptr;
  }
  
  if( m_shaderTransparent )
  {
    ae::Delete( m_shaderTransparent );
    m_shaderTransparent = nullptr;
  }
  
  m_vertexData.Terminate();
  
  ae::Delete( m_vertices );
  ae::Delete( m_sprites );
  m_vertices = nullptr;
  m_sprites = nullptr;
}

void aeSpriteRender::Render( const ae::Matrix4& localToProjection )
{
  if( m_count == 0 )
  {
    return;
  }
  
  if( m_sorting )
  {
    ae::Vec3 cameraView = localToProjection.GetRow( 2 ).GetXYZ();
    for( uint32_t i = 0; i < m_count; i++ )
    {
      m_sprites[ i ].sort = cameraView.Dot( m_sprites[ i ].transform.GetTranslation() );
    }

    auto sortFn = []( const Sprite& a, const Sprite& b ) -> bool
    {
      return a.sort > b.sort;
    };
    std::stable_sort( m_sprites, m_sprites + m_count, sortFn );
  }
  
  if( m_depthWrite && m_blending )
  {
    if( !m_shaderOpaque )
    {
      m_LoadShaderOpaque();
      m_shaderOpaque->SetDepthWrite( true );
    }
    if( !m_shaderTransparent )
    {
      m_LoadShaderTransparent();
      m_shaderTransparent->SetBlending( true );
    }
    
    m_shaderOpaque->SetDepthTest( m_depthTest );
    m_shaderTransparent->SetDepthTest( m_depthTest );
    
    m_Render( localToProjection, m_shaderOpaque );
    m_Render( localToProjection, m_shaderTransparent );
  }
  else
  {
    if( !m_shaderAll )
    {
      m_LoadShaderAll();
    }
    
    m_shaderAll->SetDepthTest( m_depthTest );
    m_shaderAll->SetBlending( m_blending );
    
    m_Render( localToProjection, m_shaderAll );
  }
  
  Clear();
}

void aeSpriteRender::m_Render( const ae::Matrix4& localToProjection, ae::Shader* shader )
{
  for( uint32_t i = 0; i < m_count; i++ )
  {
    Sprite* sprite = &m_sprites[ i ];

    uint32_t idx0 = i * aeQuadVertCount + 0;
    uint32_t idx1 = i * aeQuadVertCount + 1;
    uint32_t idx2 = i * aeQuadVertCount + 2;
    uint32_t idx3 = i * aeQuadVertCount + 3;
    
    m_vertices[ idx0 ].pos = ae::Vec3( sprite->transform * ae::Vec4( aeQuadVertPos[ 0 ], 1.0f ) );
    m_vertices[ idx1 ].pos = ae::Vec3( sprite->transform * ae::Vec4( aeQuadVertPos[ 1 ], 1.0f ) );
    m_vertices[ idx2 ].pos = ae::Vec3( sprite->transform * ae::Vec4( aeQuadVertPos[ 2 ], 1.0f ) );
    m_vertices[ idx3 ].pos = ae::Vec3( sprite->transform * ae::Vec4( aeQuadVertPos[ 3 ], 1.0f ) );

    m_vertices[ idx0 ].uv = ae::Vec2( sprite->uvMin.x, sprite->uvMin.y );
    m_vertices[ idx1 ].uv = ae::Vec2( sprite->uvMax.x, sprite->uvMin.y );
    m_vertices[ idx2 ].uv = ae::Vec2( sprite->uvMax.x, sprite->uvMax.y );
    m_vertices[ idx3 ].uv = ae::Vec2( sprite->uvMin.x, sprite->uvMax.y );

    m_vertices[ idx0 ].color = sprite->color.GetLinearRGBA();
    m_vertices[ idx1 ].color = sprite->color.GetLinearRGBA();
    m_vertices[ idx2 ].color = sprite->color.GetLinearRGBA();
    m_vertices[ idx3 ].color = sprite->color.GetLinearRGBA();
  }
  m_vertexData.UploadVertices( 0, m_vertices, m_count * 4 );

  for( uint32_t i = 0; i < m_count; i++ )
  {
    uint32_t start = i;
    const ae::Texture2D* currentTexture = m_sprites[ start ].texture;
    for(; i + 1 < m_count && m_sprites[ i + 1 ].texture == currentTexture; i++ ) {}
    uint32_t count = 1 + ( i - start );

    ae::UniformList uniforms;
    uniforms.Set( "u_localToProjection", localToProjection );
    uniforms.Set( "u_tex", currentTexture );

	m_vertexData.Bind( shader, uniforms );
    m_vertexData.Draw( start * 2, count * 2 );
  }
}

void aeSpriteRender::SetBlending( bool enabled )
{
  m_blending = enabled;
}
void aeSpriteRender::SetDepthTest( bool enabled )
{
  m_depthTest = enabled;
}
void aeSpriteRender::SetDepthWrite( bool enabled )
{
  m_depthWrite = enabled;
}

void aeSpriteRender::SetSorting( bool enabled )
{
  m_sorting = enabled;
}

void aeSpriteRender::AddSprite( const ae::Texture2D* texture, ae::Matrix4 transform, ae::Vec2 uvMin, ae::Vec2 uvMax, ae::Color color )
{
  AE_ASSERT_MSG( m_maxCount, "aeSpriteRender is not initialized" );

  if( !texture )
  {
    return;
  }

  if( m_count < m_maxCount )
  {
    Sprite* sprite = &m_sprites[ m_count ];
    sprite->transform = transform;
    sprite->uvMin = uvMin;
    sprite->uvMax = uvMax;
    sprite->color = color;
    sprite->texture = texture;
    sprite->sort = 0.0f;
    m_count++;
  }
}

void aeSpriteRender::Clear()
{
  m_count = 0;
}

void aeSpriteRender::m_LoadShaderAll()
{
  if( m_shaderAll )
  {
    return;
  }
  
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_localToProjection;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec2 v_uv;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_uv = a_uv;\
      v_color = a_color;\
      gl_Position = u_localToProjection * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
    }";
  
  m_shaderAll = ae::New< ae::Shader >( AE_ALLOC_TAG_RENDER );
  m_shaderAll->Initialize( vertexStr, fragStr, nullptr, 0 );
}

void aeSpriteRender::m_LoadShaderOpaque()
{
  if( m_shaderOpaque )
  {
    return;
  }
  
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_localToProjection;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec2 v_uv;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_uv = a_uv;\
      v_color = a_color;\
      gl_Position = u_localToProjection * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      vec4 color = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
      if( color.a < 0.99 ) { discard; }\
      AE_COLOR = color;\
    }";
  
  m_shaderOpaque = ae::New< ae::Shader >( AE_ALLOC_TAG_RENDER );
  m_shaderOpaque->Initialize( vertexStr, fragStr, nullptr, 0 );
}

void aeSpriteRender::m_LoadShaderTransparent()
{
  if( m_shaderTransparent )
  {
    return;
  }
  
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_localToProjection;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec2 v_uv;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_uv = a_uv;\
      v_color = a_color;\
      gl_Position = u_localToProjection * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      vec4 color = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
      if( color.a >= 0.99 ) { discard; }\
      AE_COLOR = color;\
    }";
  
  m_shaderTransparent = ae::New< ae::Shader >( AE_ALLOC_TAG_RENDER );
  m_shaderTransparent->Initialize( vertexStr, fragStr, nullptr, 0 );
}
