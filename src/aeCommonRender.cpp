//------------------------------------------------------------------------------
// aeRenderCommon.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
#include "aeCommonRender.h"
#include "aeRender.h"
#include "aeWindow.h"

//------------------------------------------------------------------------------
// aeRender constants
//------------------------------------------------------------------------------
const aeFloat3 aeQuadVertPos[ aeQuadVertCount ] = {
  aeFloat3( -0.5f, -0.5f, 0.0f ),
  aeFloat3( 0.5f, -0.5f, 0.0f ),
  aeFloat3( 0.5f, 0.5f, 0.0f ),
  aeFloat3( -0.5f, 0.5f, 0.0f )
};

const aeFloat2 aeQuadVertUvs[ aeQuadVertCount ] = {
  aeFloat2( 0.0f, 0.0f ),
  aeFloat2( 1.0f, 0.0f ),
  aeFloat2( 1.0f, 1.0f ),
  aeFloat2( 0.0f, 1.0f )
};

const aeQuadIndex aeQuadIndices[ aeQuadIndexCount ] = {
  3, 0, 1,
  3, 1, 2
};

//------------------------------------------------------------------------------
// aeUniformList class
//------------------------------------------------------------------------------
void aeUniformList::Set( const char* name, float value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 1;
  uniform.value.data[ 0 ] = value;
}

void aeUniformList::Set( const char* name, aeFloat2 value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 2;
  uniform.value.data[ 0 ] = value.x;
  uniform.value.data[ 1 ] = value.y;
}

void aeUniformList::Set( const char* name, aeFloat3 value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 3;
  uniform.value.data[ 0 ] = value.x;
  uniform.value.data[ 1 ] = value.y;
  uniform.value.data[ 2 ] = value.z;
}

void aeUniformList::Set( const char* name, aeFloat4 value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 4;
  uniform.value.data[ 0 ] = value.x;
  uniform.value.data[ 1 ] = value.y;
  uniform.value.data[ 2 ] = value.z;
  uniform.value.data[ 3 ] = value.w;
}

void aeUniformList::Set( const char* name, const aeFloat4x4& value )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.size = 16;
  uniform.value = value;
}

void aeUniformList::Set( const char* name, const aeTexture* tex )
{
  AE_ASSERT( name );
  AE_ASSERT( name[ 0 ] );
  Value& uniform = m_uniforms.Set( name, Value() );
  uniform.sampler = tex->GetTexture();
  uniform.target = tex->GetTarget();
}

const aeUniformList::Value* aeUniformList::Get( const char* name ) const
{
  return m_uniforms.TryGet( name );
}

//------------------------------------------------------------------------------
// aeSpriteRender member functions
//------------------------------------------------------------------------------
aeSpriteRender::aeSpriteRender()
{
  m_count = 0;
  m_maxCount = 0;
  m_sprites = nullptr;
  
  m_shaderAll = nullptr;
  m_shaderOpaque = nullptr;
  m_shaderTransparent = nullptr;
  
  m_blending = false;
  m_depthTest = false;
  m_depthWrite = false;
  m_sorting = false;
}

void aeSpriteRender::Initialize( uint32_t maxCount )
{
  AE_ASSERT( maxCount );

  m_maxCount = maxCount;
  m_count = 0;
  m_sprites = ae::NewArray< Sprite >( AE_ALLOC_TAG_RENDER, m_maxCount );

  m_vertexData.Initialize( sizeof(Vertex), sizeof(uint16_t), aeQuadVertCount * maxCount, aeQuadIndexCount * maxCount, aeVertexPrimitive::Triangle, aeVertexUsage::Dynamic, aeVertexUsage::Static );
  m_vertexData.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof(Vertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, aeVertexDataType::Float, offsetof(Vertex, color) );
  m_vertexData.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof(Vertex, uv) );

  ae::Scratch< uint16_t > scratch( m_maxCount * aeQuadIndexCount );
  uint16_t* indices = scratch.Data();
  for ( uint32_t i = 0; i < m_maxCount; i++ )
  {
    indices[ i * aeQuadIndexCount + 0 ] = i * aeQuadVertCount + aeQuadIndices[ 0 ];
    indices[ i * aeQuadIndexCount + 1 ] = i * aeQuadVertCount + aeQuadIndices[ 1 ];
    indices[ i * aeQuadIndexCount + 2 ] = i * aeQuadVertCount + aeQuadIndices[ 2 ];
    indices[ i * aeQuadIndexCount + 3 ] = i * aeQuadVertCount + aeQuadIndices[ 3 ];
    indices[ i * aeQuadIndexCount + 4 ] = i * aeQuadVertCount + aeQuadIndices[ 4 ];
    indices[ i * aeQuadIndexCount + 5 ] = i * aeQuadVertCount + aeQuadIndices[ 5 ];
  }
  m_vertexData.SetIndices( indices, m_maxCount * aeQuadIndexCount );
}

void aeSpriteRender::Destroy()
{
  if ( m_shaderAll )
  {
    ae::Delete( m_shaderAll );
    m_shaderAll = nullptr;
  }
  
  if ( m_shaderOpaque )
  {
    ae::Delete( m_shaderOpaque );
    m_shaderOpaque = nullptr;
  }
  
  if ( m_shaderTransparent )
  {
    ae::Delete( m_shaderTransparent );
    m_shaderTransparent = nullptr;
  }
  
  m_vertexData.Destroy();
  
  ae::Delete( m_sprites );
  m_sprites = nullptr;
}

void aeSpriteRender::Render( const aeFloat4x4& worldToScreen )
{
  if ( m_count == 0 )
  {
    return;
  }
  
  if ( m_sorting )
  {
    aeFloat3 cameraView = worldToScreen.GetRowVector( 2 ).GetXYZ();
    for ( uint32_t i = 0; i < m_count; i++ )
    {
      m_sprites[ i ].sort = cameraView.Dot( m_sprites[ i ].transform.GetTranslation() );
    }

    auto sortFn = []( const Sprite& a, const Sprite& b ) -> bool
    {
      return a.sort > b.sort;
    };
    std::sort( m_sprites, m_sprites + m_count, sortFn );
  }
  
  if ( m_depthWrite && m_blending )
  {
    if ( !m_shaderOpaque )
    {
      m_LoadShaderOpaque();
      m_shaderOpaque->SetDepthWrite( true );
    }
    if ( !m_shaderTransparent )
    {
      m_LoadShaderTransparent();
      m_shaderTransparent->SetBlending( true );
    }
    
    m_shaderOpaque->SetDepthTest( m_depthTest );
    m_shaderTransparent->SetDepthTest( m_depthTest );
    
    m_Render( worldToScreen, m_shaderOpaque );
    m_Render( worldToScreen, m_shaderTransparent );
  }
  else
  {
    if ( !m_shaderAll )
    {
      m_LoadShaderAll();
    }
    
    m_shaderAll->SetDepthTest( m_depthTest );
    m_shaderAll->SetBlending( m_blending );
    
    m_Render( worldToScreen, m_shaderAll );
  }
  
  Clear();
}

void aeSpriteRender::m_Render( const aeFloat4x4& worldToScreen, aeShader* shader )
{
  for ( uint32_t i = 0; i < m_textures.Length(); i++ )
  {
    const aeTexture2D* texture = m_textures.GetKey( i );
    if ( !texture )
    {
      continue;
    }
    uint32_t textureId = texture->GetTexture();

    ae::Scratch< Vertex > scratch( m_count * aeQuadVertCount );
    Vertex* vertices = scratch.Data();
    for ( uint32_t j = 0; j < m_count; j++ )
    {
      Sprite* sprite = &m_sprites[ j ];
      if ( sprite->textureId != textureId )
      {
        continue;
      }
    
      vertices[ j * aeQuadVertCount + 0 ].pos = aeFloat3( sprite->transform * aeFloat4( aeQuadVertPos[ 0 ], 1.0f ) );
      vertices[ j * aeQuadVertCount + 1 ].pos = aeFloat3( sprite->transform * aeFloat4( aeQuadVertPos[ 1 ], 1.0f ) );
      vertices[ j * aeQuadVertCount + 2 ].pos = aeFloat3( sprite->transform * aeFloat4( aeQuadVertPos[ 2 ], 1.0f ) );
      vertices[ j * aeQuadVertCount + 3 ].pos = aeFloat3( sprite->transform * aeFloat4( aeQuadVertPos[ 3 ], 1.0f ) );

      vertices[ j * aeQuadVertCount + 0 ].uv = aeFloat2( sprite->uvMin.x, sprite->uvMin.y );
      vertices[ j * aeQuadVertCount + 1 ].uv = aeFloat2( sprite->uvMax.x, sprite->uvMin.y );
      vertices[ j * aeQuadVertCount + 2 ].uv = aeFloat2( sprite->uvMax.x, sprite->uvMax.y );
      vertices[ j * aeQuadVertCount + 3 ].uv = aeFloat2( sprite->uvMin.x, sprite->uvMax.y );

      vertices[ j * aeQuadVertCount + 0 ].color = sprite->color.GetLinearRGBA();
      vertices[ j * aeQuadVertCount + 1 ].color = sprite->color.GetLinearRGBA();
      vertices[ j * aeQuadVertCount + 2 ].color = sprite->color.GetLinearRGBA();
      vertices[ j * aeQuadVertCount + 3 ].color = sprite->color.GetLinearRGBA();
    }
    // @TODO: Should set all vertices first then render multiple times
    m_vertexData.SetVertices( vertices, scratch.Length() );

    aeUniformList uniforms;
    uniforms.Set( "u_worldToScreen", worldToScreen );
    uniforms.Set( "u_tex", texture );

    m_vertexData.Render( shader, m_count * 2, uniforms );
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

void aeSpriteRender::AddSprite( const aeTexture2D* texture, aeFloat4x4 transform, aeFloat2 uvMin, aeFloat2 uvMax, aeColor color )
{
  AE_ASSERT_MSG( m_maxCount, "aeSpriteRender is not initialized" );

  if ( !texture )
  {
    return;
  }

  if ( m_count < m_maxCount )
  {
    Sprite* sprite = &m_sprites[ m_count ];
    sprite->transform = transform;
    sprite->uvMin = uvMin;
    sprite->uvMax = uvMax;
    sprite->color = color;
    sprite->textureId = texture->GetTexture();
    sprite->sort = 0.0f;
    m_count++;

    m_textures.Set( texture, 0 );
  }
}

void aeSpriteRender::Clear()
{
  m_count = 0;
  m_textures.Clear();
}

void aeSpriteRender::m_LoadShaderAll()
{
  if ( m_shaderAll )
  {
    return;
  }
  
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_worldToScreen;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec2 v_uv;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_uv = a_uv;\
      v_color = a_color;\
      gl_Position = u_worldToScreen * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
    }";
  
  m_shaderAll = ae::New< aeShader >( AE_ALLOC_TAG_RENDER );
  m_shaderAll->Initialize( vertexStr, fragStr, nullptr, 0 );
}

void aeSpriteRender::m_LoadShaderOpaque()
{
  if ( m_shaderOpaque )
  {
    return;
  }
  
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_worldToScreen;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec2 v_uv;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_uv = a_uv;\
      v_color = a_color;\
      gl_Position = u_worldToScreen * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      vec4 color = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
      if ( color.a < 0.99 ) { discard; }\
      AE_COLOR = color;\
    }";
  
  m_shaderOpaque = ae::New< aeShader >( AE_ALLOC_TAG_RENDER );
  m_shaderOpaque->Initialize( vertexStr, fragStr, nullptr, 0 );
}

void aeSpriteRender::m_LoadShaderTransparent()
{
  if ( m_shaderTransparent )
  {
    return;
  }
  
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_worldToScreen;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec2 v_uv;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_uv = a_uv;\
      v_color = a_color;\
      gl_Position = u_worldToScreen * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      vec4 color = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
      if ( color.a >= 0.99 ) { discard; }\
      AE_COLOR = color;\
    }";
  
  m_shaderTransparent = ae::New< aeShader >( AE_ALLOC_TAG_RENDER );
  m_shaderTransparent->Initialize( vertexStr, fragStr, nullptr, 0 );
}

//------------------------------------------------------------------------------
// Text constants
//------------------------------------------------------------------------------
const uint32_t kCharsPerString = 64;

//------------------------------------------------------------------------------
// aeTextRender member functions
//------------------------------------------------------------------------------
void aeTextRender::Initialize( const char* imagePath, aeTextureFilter::Type filterType, uint32_t fontSize )
{
  m_fontSize = fontSize;
  m_rectCount = 0;

  m_vertexData.Initialize( sizeof( Vertex ), sizeof( uint16_t ), kMaxTextRects * m_rects[ 0 ].text.Size() * aeQuadVertCount, kMaxTextRects * kCharsPerString * aeQuadIndexCount, aeVertexPrimitive::Triangle, aeVertexUsage::Dynamic, aeVertexUsage::Dynamic );
  m_vertexData.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof( Vertex, pos ) );
  m_vertexData.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof( Vertex, uv ) );
  m_vertexData.AddAttribute( "a_color", 4, aeVertexDataType::Float, offsetof( Vertex, color ) );

  // Load shader
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_uiToScreen;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec2 v_uv;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_uv = a_uv;\
      v_color = a_color;\
      gl_Position = u_uiToScreen * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      if ( AE_TEXTURE2D( u_tex, v_uv ).r < 0.5 ) { discard; };\
      AE_COLOR = v_color;\
    }";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );

  m_texture.Initialize( imagePath, filterType, aeTextureWrap::Clamp );
}

void aeTextRender::Terminate()
{
  m_texture.Destroy();
  m_shader.Destroy();
  m_vertexData.Destroy();
}

void aeTextRender::Render( const aeFloat4x4& uiToScreen )
{
  uint32_t vertCount = 0;
  uint32_t indexCount = 0;
  ae::Scratch< Vertex > verts( m_vertexData.GetMaxVertexCount() );
  ae::Scratch< uint16_t > indices( m_vertexData.GetMaxIndexCount() );

  for ( uint32_t i = 0; i < m_rectCount; i++ )
  {
    const TextRect& rect = m_rects[ i ];
    aeFloat3 pos = rect.pos;
    pos.y -= rect.size.y;

    const char* start = rect.text.c_str();
    const char* str = start;
    while ( str[ 0 ] )
    {
      if ( !isspace( str[ 0 ] ) )
      {
        int32_t index = str[ 0 ];
        uint32_t columns = m_texture.GetWidth() / m_fontSize;
        aeFloat2 offset( index % columns, columns - index / columns - 1 ); // @HACK: Assume same number of columns and rows

        for ( uint32_t j = 0; j < aeQuadIndexCount; j++ )
        {
          indices.GetSafe( indexCount ) = aeQuadIndices[ j ] + vertCount;
          indexCount++;
        }

        AE_ASSERT( vertCount + aeQuadVertCount <= verts.Length() );
        // Bottom Left
        verts[ vertCount ].pos = pos;
        verts[ vertCount ].uv = ( aeQuadVertUvs[ 0 ] + offset ) / columns;
        verts[ vertCount ].color = rect.color;
        vertCount++;
        // Bottom Right
        verts[ vertCount ].pos = pos + aeFloat3( rect.size.x, 0.0f, 0.0f );
        verts[ vertCount ].uv = ( aeQuadVertUvs[ 1 ] + offset ) / columns;
        verts[ vertCount ].color = rect.color;
        vertCount++;
        // Top Right
        verts[ vertCount ].pos = pos + aeFloat3( rect.size.x, rect.size.y, 0.0f );
        verts[ vertCount ].uv = ( aeQuadVertUvs[ 2 ] + offset ) / columns;
        verts[ vertCount ].color = rect.color;
        vertCount++;
        // Top Left
        verts[ vertCount ].pos = pos + aeFloat3( 0.0f, rect.size.y, 0.0f );
        verts[ vertCount ].uv = ( aeQuadVertUvs[ 3 ] + offset ) / columns;
        verts[ vertCount ].color = rect.color;
        vertCount++;
      }

      if ( str[ 0 ] == '\n' || str[ 0 ] == '\r' )
      {
        pos.x = rect.pos.x;
        pos.y -= rect.size.y;
      }
      else
      {
        pos.x += rect.size.x;
      }
      str++;
    }
  }

  m_vertexData.SetVertices( verts.Data(), vertCount );
  m_vertexData.SetIndices( indices.Data(), indexCount );

  aeUniformList uniforms;
  uniforms.Set( "u_uiToScreen", uiToScreen );
  uniforms.Set( "u_tex", &m_texture );
  m_vertexData.Render( &m_shader, uniforms );

  m_rectCount = 0;
}

void aeTextRender::Add( aeFloat3 pos, aeFloat2 size, const char* str, aeColor color, uint32_t lineLength, uint32_t charLimit )
{
  if ( m_rectCount >= kMaxTextRects )
  {
    return;
  }

  TextRect* rect = &m_rects[ m_rectCount ];
  m_rectCount++;

  rect->pos = pos;
  rect->size = size;
  m_ParseText( str, lineLength, charLimit, &rect->text );
  rect->color = color;
}

uint32_t aeTextRender::GetLineCount( const char* str, uint32_t lineLength, uint32_t charLimit ) const
{
  return m_ParseText( str, lineLength, charLimit, nullptr );
}

uint32_t aeTextRender::m_ParseText( const char* str, uint32_t lineLength, uint32_t charLimit, aeStr512* outText ) const
{
  if ( outText )
  {
    *outText = "";
  }

  uint32_t lineCount = 1;
  const char* start = str;
  uint32_t lineChars = 0;
  while ( str[ 0 ] )
  {
    // Truncate displayed string based on param
    if ( charLimit && (uint32_t)( str - start ) >= charLimit )
    {
      break;
    }

    bool isNewlineChar = ( str[ 0 ] == '\n' || str[ 0 ] == '\r' );

    if ( lineLength && !isNewlineChar && isspace( str[ 0 ] ) )
    {
      // Prevent words from being split across lines
      uint32_t wordRemainder = 1;
      while ( str[ wordRemainder ] && !isspace( str[ wordRemainder ] ) )
      {
        wordRemainder++;
      }

      if ( lineChars + wordRemainder > lineLength )
      {
        if ( outText )
        {
          outText->Append( "\n" );
        }
        lineCount++;
        lineChars = 0;
      }
    }

    // Skip non-newline whitespace at the beginning of a line
    if ( lineChars || isNewlineChar || !isspace( str[ 0 ] ) )
    {
      if ( outText )
      {
        // @TODO: aeStr should support appending chars
        char hack[] = { str[ 0 ], 0 };
        outText->Append( hack );
      }

      lineChars = isNewlineChar ? 0 : lineChars + 1;
    }
    if ( isNewlineChar )
    {
      lineCount++;
    }

    str++;
  }

  return lineCount;
}

//------------------------------------------------------------------------------
// aeDebugRender constants
//------------------------------------------------------------------------------
const uint32_t kDebugVertexCountPerObject = 32;

//------------------------------------------------------------------------------
// aeDebugRender member functions
//------------------------------------------------------------------------------
void aeDebugRender::Initialize( uint32_t maxObjects )
{
  m_objs = ae::Array< DebugObject >( AE_ALLOC_TAG_RENDER, maxObjects );

  // @HACK: Should handle vert count in a safer way
  m_vertexData.Initialize( sizeof(DebugVertex), sizeof(uint16_t), m_objs.Size() * kDebugVertexCountPerObject, 0, aeVertexPrimitive::Line, aeVertexUsage::Dynamic, aeVertexUsage::Static );
  m_vertexData.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof(DebugVertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, aeVertexDataType::Float, offsetof(DebugVertex, color) );

  // Load shader
  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_worldToScreen;\
    AE_UNIFORM float u_saturation;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      float bw = (min(a_color.r, min(a_color.g, a_color.b)) + max(a_color.r, max(a_color.g, a_color.b))) * 0.5;\
      v_color = vec4(mix(vec3(bw), a_color.rgb, u_saturation), 1.0);\
      gl_Position = u_worldToScreen * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      AE_COLOR = v_color;\
    }";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );
  m_shader.SetBlending( true );
  m_shader.SetDepthTest( true );
}

void aeDebugRender::Destroy()
{
  m_shader.Destroy();
  m_vertexData.Destroy();
}

void aeDebugRender::Render( const aeFloat4x4& worldToScreen )
{
  if ( !m_objs.Length() )
  {
    return;
  }

  const uint16_t kQuadIndices[] = {
    3, 1, 0,
    3, 1, 2
  };

  m_verts.Clear();
  m_verts.Reserve( m_objs.Size() * kDebugVertexCountPerObject );

  for ( uint32_t i = 0; i < m_objs.Length(); i++ )
  {
    DebugObject obj = m_objs[ i ];
    if ( obj.type == DebugType::Rect )
    {
      aeFloat3 halfSize = obj.size * 0.5f;

      DebugVertex verts[ 4 ];
      
      verts[ 0 ].pos = obj.pos + obj.rotation.Rotate( aeFloat3( -halfSize.x, 0.0f, -halfSize.y ) ); // Bottom Left
      verts[ 1 ].pos = obj.pos + obj.rotation.Rotate( aeFloat3( halfSize.x, 0.0f, -halfSize.y ) ); // Bottom Right
      verts[ 2 ].pos = obj.pos + obj.rotation.Rotate( aeFloat3( halfSize.x, 0.0f, halfSize.y ) ); // Top Right
      verts[ 3 ].pos = obj.pos + obj.rotation.Rotate( aeFloat3( -halfSize.x, 0.0f, halfSize.y ) ); // Top Left

      verts[ 0 ].color = obj.color;
      verts[ 1 ].color = obj.color;
      verts[ 2 ].color = obj.color;
      verts[ 3 ].color = obj.color;

      m_verts.Append( verts[ 0 ] );
      m_verts.Append( verts[ 1 ] );
      m_verts.Append( verts[ 1 ] );
      m_verts.Append( verts[ 2 ] );
      m_verts.Append( verts[ 2 ] );
      m_verts.Append( verts[ 3 ] );
      m_verts.Append( verts[ 3 ] );
      m_verts.Append( verts[ 0 ] );
    }
    else if ( obj.type == DebugType::Circle )
    {
      float angleInc = aeMath::PI * 2.0f / obj.pointCount;
      for ( uint32_t i = 0; i < obj.pointCount; i++ )
      {
        float angle0 = angleInc * i;
        float angle1 = angleInc * ( i + 1 );

        DebugVertex verts[ 2 ];

        verts[ 0 ].pos = aeFloat3( cosf( angle0 ) * obj.radius, 0.0f, sinf( angle0 ) * obj.radius );
        verts[ 1 ].pos = aeFloat3( cosf( angle1 ) * obj.radius, 0.0f, sinf( angle1 ) * obj.radius );
        verts[ 0 ].pos = obj.rotation.Rotate( verts[ 0 ].pos );
        verts[ 1 ].pos = obj.rotation.Rotate( verts[ 1 ].pos );
        verts[ 0 ].pos += obj.pos;
        verts[ 1 ].pos += obj.pos;

        verts[ 0 ].color = obj.color;
        verts[ 1 ].color = obj.color;

        m_verts.Append( verts, countof( verts ) );
      }
    }
    else if ( obj.type == DebugType::Line )
    {
      DebugVertex verts[ 2 ];
      verts[ 0 ].pos = obj.pos;
      verts[ 0 ].color = obj.color;
      verts[ 1 ].pos = obj.end;
      verts[ 1 ].color = obj.color;

      m_verts.Append( verts, countof( verts ) );
    }
    else if ( obj.type == DebugType::AABB )
    {
      aeFloat3 s = obj.size;
      aeFloat3 c[] =
      {
        obj.pos + aeFloat3( -s.x, s.y, s.z ),
        obj.pos + s,
        obj.pos + aeFloat3( s.x, -s.y, s.z ),
        obj.pos + aeFloat3( -s.x, -s.y, s.z ),
        obj.pos + aeFloat3( -s.x, s.y, -s.z ),
        obj.pos + aeFloat3( s.x, s.y, -s.z ),
        obj.pos + aeFloat3( s.x, -s.y, -s.z ),
        obj.pos + aeFloat3( -s.x, -s.y, -s.z )
      };
      AE_STATIC_ASSERT( countof( c ) == 8 );

      DebugVertex verts[] =
      {
        // Top
        { c[ 0 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 0 ], obj.color },
        // Sides
        { c[ 0 ], obj.color },
        { c[ 4 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 7 ], obj.color },
        //Bottom
        { c[ 4 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 4 ], obj.color },
      };
      AE_STATIC_ASSERT( countof( c ) * 3 == countof( verts ) );
      
      m_verts.Append( verts, countof( verts ) );
    }
    else if ( obj.type == DebugType::Cube )
    {
      aeFloat3 c[] =
      {
        ( obj.transform * aeFloat4( -0.5f, 0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * aeFloat4( 0.5f, 0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * aeFloat4( 0.5f, -0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * aeFloat4( -0.5f, -0.5f, 0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * aeFloat4( -0.5f, 0.5f, -0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * aeFloat4( 0.5f, 0.5f, -0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * aeFloat4( 0.5f, -0.5f, -0.5f, 1.0f ) ).GetXYZ(),
        ( obj.transform * aeFloat4( -0.5f, -0.5f, -0.5f, 1.0f ) ).GetXYZ()
      };
      AE_STATIC_ASSERT( countof( c ) == 8 );

      DebugVertex verts[] =
      {
        // Top
        { c[ 0 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 0 ], obj.color },
        // Sides
        { c[ 0 ], obj.color },
        { c[ 4 ], obj.color },
        { c[ 1 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 2 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 3 ], obj.color },
        { c[ 7 ], obj.color },
        //Bottom
        { c[ 4 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 5 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 6 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 7 ], obj.color },
        { c[ 4 ], obj.color },
      };
      AE_STATIC_ASSERT( countof( c ) * 3 == countof( verts ) );

      m_verts.Append( verts, countof( verts ) );
    }
  }

  if ( m_verts.Length() )
  {
    m_vertexData.SetVertices( &m_verts[ 0 ], aeMath::Min( m_verts.Length(), m_vertexData.GetMaxVertexCount() ) );

    aeUniformList uniforms;
    uniforms.Set( "u_worldToScreen", worldToScreen );

    if ( m_xray )
    {
      m_shader.SetDepthTest( false );
      m_shader.SetDepthWrite( false );
      uniforms.Set( "u_saturation", 0.1f );
      m_vertexData.Render( &m_shader, uniforms );
    }

    m_shader.SetDepthTest( true );
    m_shader.SetDepthWrite( true );
    uniforms.Set( "u_saturation", 1.0f );
    m_vertexData.Render( &m_shader, uniforms );
  }

  m_objs.Clear();
}

void aeDebugRender::Clear()
{
  m_objs.Clear();
}

void aeDebugRender::AddLine( aeFloat3 p0, aeFloat3 p1, aeColor color )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Line;
    obj->pos = p0;
    obj->end = p1;
    obj->color = color;
  }
}

void aeDebugRender::AddDistanceCheck( aeFloat3 p0, aeFloat3 p1, float distance )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Line;
    obj->pos = p0;
    obj->end = p1;
    obj->color = ( ( p1 - p0 ).Length() <= distance ) ? aeColor::Green() : aeColor::Red();
  }
}

void aeDebugRender::AddRect( aeFloat3 pos, aeFloat3 up, aeFloat3 normal, aeFloat2 size, aeColor color )
{
  if ( m_objs.Length() < m_objs.Size()
    && up.LengthSquared() > 0.001f
    && normal.LengthSquared() > 0.001f )
  {
    up.SafeNormalize();
    normal.SafeNormalize();
    if ( normal.Dot( up ) < 0.999f )
    {
      DebugObject* obj = &m_objs.Append( DebugObject() );
      obj->type = DebugType::Rect;
      obj->pos = pos;
      obj->rotation = aeQuat( normal, up );
      obj->size = aeFloat3( size );
      obj->color = color;
      obj->pointCount = 0;
    }
  }
}

void aeDebugRender::AddCircle( aeFloat3 pos, aeFloat3 normal, float radius, aeColor color, uint32_t pointCount )
{
  if ( m_objs.Length() < m_objs.Size() && normal.LengthSquared() > 0.001f )
  {
    normal.SafeNormalize();
    float dot = normal.Dot( aeFloat3::Up );
    
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Circle;
    obj->pos = pos;
    obj->rotation = aeQuat( normal, ( dot < 0.99f && dot > -0.99f ) ? aeFloat3::Up : aeFloat3::Right );
    obj->radius = radius;
    obj->color = color;
    obj->pointCount = pointCount;
  }
}

void aeDebugRender::AddSphere( aeFloat3 pos, float radius, aeColor color, uint32_t pointCount )
{
  if ( m_objs.Length() + 3 <= m_objs.Size() )
  {
    AddCircle( pos, aeFloat3::Up, radius, color, pointCount );
    AddCircle( pos, aeFloat3::Right, radius, color, pointCount );
    AddCircle( pos, aeFloat3::Forward, radius, color, pointCount );
  }
}

void aeDebugRender::AddAABB( aeFloat3 pos, aeFloat3 halfSize, aeColor color )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::AABB;
    obj->pos = pos;
    obj->rotation = aeQuat::Identity();
    obj->size = halfSize;
    obj->color = color;
    obj->pointCount = 0;
  }
}

void aeDebugRender::AddCube( aeFloat4x4 transform, aeColor color )
{
  if ( m_objs.Length() < m_objs.Size() )
  {
    DebugObject* obj = &m_objs.Append( DebugObject() );
    obj->type = DebugType::Cube;
    obj->transform = transform;
    obj->color = color;
  }
}
