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
  3, 1, 0,
  3, 1, 2
};

//------------------------------------------------------------------------------
// aeColor
//------------------------------------------------------------------------------
const aeColor aeColor::White( 1.0f, 1.0f, 1.0f );
const aeColor aeColor::Red( 1.0f, 0.0f, 0.0f );
const aeColor aeColor::Green( 0.0f, 1.0f, 0.0f );
const aeColor aeColor::Blue( 0.0f, 0.0f, 1.0f );
const aeColor aeColor::Orange( 1.0f, 0.5f, 0.0f );
const aeColor aeColor::Gray( 0.5f, 0.5f, 0.5f );
const aeColor aeColor::Black( 0.0f, 0.0f, 0.0f );

aeColor::aeColor( float rgb )
  : r( rgb ), g( rgb ), b( rgb ), a( 1.0f )
{}

aeColor::aeColor( float r, float g, float b )
  : r( r ), g( g ), b( b ), a( 1.0f )
{}

aeColor::aeColor( float r, float g, float b, float a )
  : r( r ), g( g ), b( b ), a( a )
{}

aeColor::aeColor( aeColor c, float a )
  : r( c.r ), g( c.g ), b( c.b ), a( a )
{}

aeColor aeColor::SRGB( float r, float g, float b )
{
  return aeColor(
    SRGBToRGB( r ),
    SRGBToRGB( g ),
    SRGBToRGB( b ),
    1.0f
  );
}

aeColor aeColor::SRGBA( float r, float g, float b, float a )
{
  return aeColor(
    SRGBToRGB( r ),
    SRGBToRGB( g ),
    SRGBToRGB( b ),
    a
  );
}

aeColor aeColor::PS( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
  return aeColor(
    SRGBToRGB( r / 255.0f ),
    SRGBToRGB( g / 255.0f ),
    SRGBToRGB( b / 255.0f ),
    a / 255.0f
  );
}

aeFloat3 aeColor::GetSRGB() const
{
  return aeFloat3(
    RGBToSRGB( r ),
    RGBToSRGB( g ),
    RGBToSRGB( b )
  );
}

aeFloat4 aeColor::GetSRGBA() const
{
  return aeFloat4( GetSRGB(), a );
}

aeColor aeColor::Lerp( const aeColor& end, float t ) const
{
  return aeColor(
    aeMath::Lerp( r, end.r, t ),
    aeMath::Lerp( g, end.g, t ),
    aeMath::Lerp( b, end.b, t ),
    aeMath::Lerp( a, end.a, t )
  );
}

float aeColor::SRGBToRGB( float x )
{
  return pow( x , 2.2 );
}

float aeColor::RGBToSRGB( float x )
{
  return pow( x, 1.0 / 2.2 );
}

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
}

void aeSpriteRender::Initialize( uint32_t maxCount )
{
  AE_ASSERT( maxCount );

  m_maxCount = maxCount;
  m_count = 0;
  m_sprites = aeAlloc::AllocateArray< Sprite >( m_maxCount );

  m_vertexData.Initialize( sizeof(Vertex), sizeof(uint16_t), aeQuadVertCount * maxCount, aeQuadIndexCount * maxCount, aeVertexPrimitive::Triangle, aeVertexUsage::Dynamic, aeVertexUsage::Static );
  m_vertexData.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof(Vertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, aeVertexDataType::Float, offsetof(Vertex, color) );
  m_vertexData.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof(Vertex, uv) );

  aeAlloc::Scratch< uint16_t > scratch( m_maxCount * aeQuadIndexCount );
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
      AE_COLOR = AE_RGBA_TO_SRGBA( AE_SRGBA_TO_RGBA( AE_TEXTURE2D( u_tex, v_uv ) ) * v_color );\
    }";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );
}

void aeSpriteRender::Destroy()
{
  m_shader.Destroy();
  m_vertexData.Destroy();
  
  aeAlloc::Release( m_sprites );
  m_sprites = nullptr;
}

void aeSpriteRender::Render( const aeFloat4x4& worldToScreen )
{
  for ( uint32_t i = 0; i < m_textures.Length(); i++ )
  {
    const aeTexture2D* texture = m_textures.GetKey( i );
    if ( !texture )
    {
      continue;
    }
    uint32_t textureId = texture->GetTexture();

    aeAlloc::Scratch< Vertex > scratch( m_count * aeQuadVertCount );
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

    m_vertexData.Render( &m_shader, m_count * 2, uniforms );
  }

  Clear();
}

void aeSpriteRender::SetBlending( bool enabled )
{
  m_shader.SetBlending( enabled );
}
void aeSpriteRender::SetDepthTest( bool enabled )
{
  m_shader.SetDepthTest( enabled );
}
void aeSpriteRender::SetDepthWrite( bool enabled )
{
  m_shader.SetDepthWrite( enabled );
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
    m_count++;

    m_textures.Set( texture, 0 );
  }
}

void aeSpriteRender::Clear()
{
  m_count = 0;
  m_textures.Clear();
}

//------------------------------------------------------------------------------
// Text constants
//------------------------------------------------------------------------------
const uint32_t kCharsPerString = 64;

//------------------------------------------------------------------------------
// aeTextRender member functions
//------------------------------------------------------------------------------
void aeTextRender::Initialize( const char* imagePath, aeTextureFilter::Type filterType, uint32_t charSize )
{
  m_charSize = charSize;
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
      if ( AE_SRGB_TO_RGB( AE_TEXTURE2D( u_tex, v_uv ).r ) < 0.5 ) { discard; };\
      AE_COLOR = v_color;\
      AE_COLOR = AE_RGBA_TO_SRGBA( AE_COLOR );\
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
  aeAlloc::Scratch< Vertex > verts( m_vertexData.GetMaxVertexCount() );
  aeAlloc::Scratch< uint16_t > indices( m_vertexData.GetMaxIndexCount() );

  for ( uint32_t i = 0; i < m_rectCount; i++ )
  {
    const TextRect& rect = m_rects[ i ];
    aeFloat2 pos = rect.pos;
    pos.y -= rect.size.y;

    const char* start = rect.text.c_str();
    const char* str = start;
    while ( str[ 0 ] )
    {
      if ( !isspace( str[ 0 ] ) )
      {
        int32_t index = str[ 0 ];
        uint32_t columns = m_texture.GetWidth() / m_charSize;
        aeFloat2 offset( index % columns, columns - index / columns - 1 ); // @HACK: Assume same number of columns and rows

        for ( uint32_t j = 0; j < aeQuadIndexCount; j++ )
        {
          indices.GetSafe( indexCount ) = aeQuadIndices[ j ] + vertCount;
          indexCount++;
        }

        AE_ASSERT( vertCount + aeQuadVertCount <= verts.Length() );
        // Bottom Left
        verts[ vertCount ].pos = aeFloat3( pos.x, pos.y, 0.0f );
        verts[ vertCount ].uv = ( aeQuadVertUvs[ 0 ] + offset ) / columns;
        verts[ vertCount ].color = rect.color;
        vertCount++;
        // Bottom Right
        verts[ vertCount ].pos = aeFloat3( pos.x + rect.size.x, pos.y, 0.0f );
        verts[ vertCount ].uv = ( aeQuadVertUvs[ 1 ] + offset ) / columns;
        verts[ vertCount ].color = rect.color;
        vertCount++;
        // Top Right
        verts[ vertCount ].pos = aeFloat3( pos.x + rect.size.x, pos.y + rect.size.y, 0.0f );
        verts[ vertCount ].uv = ( aeQuadVertUvs[ 2 ] + offset ) / columns;
        verts[ vertCount ].color = rect.color;
        vertCount++;
        // Top Left
        verts[ vertCount ].pos = aeFloat3( pos.x, pos.y + rect.size.y, 0.0f );
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

void aeTextRender::Add( aeFloat2 pos, aeFloat2 size, const char* str, aeColor color, uint32_t lineLength, uint32_t charLimit )
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
// aeRender member functions
//------------------------------------------------------------------------------
aeRender::aeRender()
{
  m_renderInternal = nullptr;

  m_window = nullptr;
  m_targetWidth = 0;
  m_targetHeight = 0;

  m_clearColor = aeColor::Black;
}

aeRender::~aeRender()
{
  Terminate();
}

void aeRender::InitializeOpenGL( class aeWindow* window, uint32_t width, uint32_t height )
{
  AE_ASSERT( !m_renderInternal );

  m_targetWidth = width;
  m_targetHeight = height;

  m_window = window;

  m_renderInternal = aeAlloc::Allocate< aeOpenGLRender >();
  m_renderInternal->Initialize( this );
}

void aeRender::Terminate()
{
  if ( m_renderInternal )
  {
    m_renderInternal->Terminate( this );
    aeAlloc::Release( m_renderInternal );
    m_renderInternal = nullptr;
  }
}

void aeRender::StartFrame()
{
  AE_ASSERT( m_renderInternal );

  if ( m_targetWidth != m_canvas.GetWidth() || m_targetHeight != m_canvas.GetHeight() )
  {
    m_canvas.Destroy();
    m_canvas.Initialize( m_targetWidth, m_targetHeight, aeTextureFilter::Nearest, aeTextureWrap::Clamp );
  }
  m_canvas.Activate();

  m_renderInternal->StartFrame( this );
}

void aeRender::EndFrame()
{
  AE_ASSERT( m_renderInternal );
  m_renderInternal->EndFrame( this );
}


void aeRender::Resize( uint32_t width, uint32_t height )
{
  m_targetWidth = width;
  m_targetHeight = height;
}

void aeRender::SetClearColor( aeColor color )
{
  m_clearColor = color;
}

aeColor aeRender::GetClearColor() const
{
  return m_clearColor;
}

aeFloat4x4 aeRender::GetWindowToRenderTransform()
{
  aeFloat4x4 windowToNDC = aeFloat4x4::Translation( aeFloat3( -1.0f, -1.0f, 0.0f ) );
  windowToNDC.Scale( aeFloat3( 2.0f / m_window->GetWidth(), 2.0f / m_window->GetHeight(), 1.0f ) );
  
  aeFloat4x4 ndcToQuad = aeRenderTexture::GetQuadToNDCTransform( GetNDCRect(), 0.0f );
  ndcToQuad.Invert();
  
  aeFloat4x4 quadToRender = aeFloat4x4::Scaling( aeFloat3( m_canvas.GetWidth(), m_canvas.GetHeight(), 1.0f ) );
  quadToRender.Translate( aeFloat3( 0.5f, 0.5f, 0.0f ) );
  
  return ( quadToRender * ndcToQuad * windowToNDC );
}

aeRect aeRender::GetNDCRect() const
{
  float canvasAspect = m_canvas.GetWidth() / (float)m_canvas.GetHeight();
  float windowAspect = m_window->GetWidth() / (float)m_window->GetHeight();
  if ( canvasAspect >= windowAspect )
  {
    // Fit width
    float height = windowAspect / canvasAspect;
    return aeRect( -1.0f, -height, 2.0f, height * 2.0f );
  }
  else
  {
    // Fit height
    float width = canvasAspect / windowAspect;
    return aeRect( -width, -1.0f, width * 2.0f, 2.0f );
  }
}
