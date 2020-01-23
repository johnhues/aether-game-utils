//------------------------------------------------------------------------------
// aeRender.cpp
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
#define _AE_GRAPHICS_
#include "aeRender.h"
#include "aeAlloc.h"
#include "aeLog.h"
#include "aePlatform.h"
#include "aeVfs.h"
#include "aeWindow.h"

#if _AE_EMSCRIPTEN_
#define GL_SAMPLER_3D 0x0
#define GL_TEXTURE_3D 0x0
void glGenVertexArrays( GLsizei n, GLuint *arrays ) {}
void glDeleteVertexArrays( GLsizei n, const GLuint *arrays ) {}
void glBindVertexArray( GLuint array ) {}
void glBindFragDataLocation( GLuint program, GLuint colorNumber, const char* name ) {}
void glClearDepth( float depth ) { glClearDepthf( depth ); }
#endif

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
GLenum aeVertexDataTypeToGL( aeVertexDataType::Type type )
{
  switch ( type )
  {
    case aeVertexDataType::UInt8:
      return GL_UNSIGNED_BYTE;
    case aeVertexDataType::UInt16:
      return GL_UNSIGNED_SHORT;
    case aeVertexDataType::UInt32:
      return GL_UNSIGNED_INT;
    case aeVertexDataType::Float:
      return GL_FLOAT;
    default:
      AE_FAIL();
      return 0;
  }
}

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
    aeMath::Pow( r, 2.2f ),
    aeMath::Pow( g, 2.2f ),
    aeMath::Pow( b, 2.2f ),
    1.0f
  );
}

aeColor aeColor::SRGBA( float r, float g, float b, float a )
{
  return aeColor(
    aeMath::Pow( r, 2.2f ),
    aeMath::Pow( g, 2.2f ),
    aeMath::Pow( b, 2.2f ),
    a
  );
}

aeColor aeColor::PS( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
  return aeColor(
    aeMath::Pow( r / 255.0f, 2.2f ),
    aeMath::Pow( g / 255.0f, 2.2f ),
    aeMath::Pow( b / 255.0f, 2.2f ),
    a / 255.0f
  );
}

aeFloat3 aeColor::GetSRGB() const
{
  return aeFloat3(
    aeMath::Pow( r, 1.0f / 2.2f ),
    aeMath::Pow( g, 1.0f / 2.2f ),
    aeMath::Pow( b, 1.0f / 2.2f )
  );
}

aeFloat4 aeColor::GetSRGBA() const
{
  return aeFloat4(
    aeMath::Pow( r, 1.0f / 2.2f ),
    aeMath::Pow( g, 1.0f / 2.2f ),
    aeMath::Pow( b, 1.0f / 2.2f ),
    a
  );
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
// aeVertexData member functions
//------------------------------------------------------------------------------
aeVertexData::aeVertexData()
{
  memset( this, 0, sizeof(aeVertexData) );
  m_primitive = (aeVertexPrimitive::Type)-1;
  m_vertexUsage = (aeVertexUsage::Type)-1;
  m_indexUsage = (aeVertexUsage::Type)-1;
  m_vertices = ~0;
  m_indices = ~0;
}

void aeVertexData::Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, aeVertexPrimitive::Type primitive, aeVertexUsage::Type vertexUsage, aeVertexUsage::Type indexUsage )
{
  AE_ASSERT( m_vertexSize == 0 );
  AE_ASSERT( vertexSize );
  AE_ASSERT( m_indexSize == 0 );
  AE_ASSERT( indexSize == sizeof(uint8_t) || indexSize == sizeof(uint16_t) || indexSize == sizeof(uint32_t) );

  memset( this, 0, sizeof(aeVertexData) );
  m_maxVertexCount = maxVertexCount;
  m_maxIndexCount = maxIndexCount;
  m_primitive = primitive;
  m_vertexUsage = vertexUsage;
  m_indexUsage = indexUsage;
  m_vertices = ~0;
  m_indices = ~0;
  m_vertexSize = vertexSize;
  m_indexSize = indexSize;
  
  glGenVertexArrays( 1, &m_array );
  glBindVertexArray( m_array );
}

void aeVertexData::Destroy()
{
  if ( m_vertexReadable ) { aeAlloc::Release( (uint8_t*)m_vertexReadable ); }
  if ( m_indexReadable ) { aeAlloc::Release( (uint8_t*)m_indexReadable ); }
  
  glDeleteVertexArrays( 1, &m_array );
  if ( m_vertices != ~0 ) { glDeleteBuffers( 1, &m_vertices ); }
  if ( m_indices != ~0 ) { glDeleteBuffers( 1, &m_indices ); }
  
  memset( this, 0, sizeof(aeVertexData) );
  m_primitive = (aeVertexPrimitive::Type)-1;
  m_vertexUsage = (aeVertexUsage::Type)-1;
  m_indexUsage = (aeVertexUsage::Type)-1;
  m_vertices = ~0;
  m_indices = ~0;
}

void aeVertexData::AddAttribute( const char *name, uint32_t componentCount, aeVertexDataType::Type type, uint32_t offset )
{
  AE_ASSERT( m_vertices == ~0 && m_indices == ~0 );
  
  AE_ASSERT( m_attributeCount < countof(m_attributes) );
  aeVertexAttribute* attribute = &m_attributes[ m_attributeCount ];
  m_attributeCount++;
  
  size_t length = strlen( name );
  AE_ASSERT( length < kMaxShaderAttributeNameLength );
  strcpy( attribute->name, name );
  attribute->componentCount = componentCount;
  attribute->type = aeVertexDataTypeToGL( type );
  attribute->offset = offset;
}

void aeVertexData::m_SetVertices( const void* vertices, uint32_t count )
{
  AE_ASSERT( m_vertexSize );
  AE_ASSERT_MSG( count <= m_maxVertexCount, "# #", count, m_maxVertexCount );

  if ( m_indices != ~0 )
  {
    AE_ASSERT( m_indexSize != 0 );
  }
  if ( m_indexSize )
  {
    AE_ASSERT_MSG( count <= (uint64_t)1 << ( m_indexSize * 8 ), "Vertex count (#) too high for index of size #", count, m_indexSize );
  }
  
  if( m_vertexUsage == aeVertexUsage::Static )
  {
    AE_ASSERT( count );
    AE_ASSERT_MSG( !m_vertexCount, "Cannot re-set vertices, buffer was created as static!" );
    AE_ASSERT( m_vertices == ~0 );

    m_vertexCount = count;

    glGenBuffers( 1, &m_vertices );
    glBindVertexArray( m_array );
    glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
    glBufferData( GL_ARRAY_BUFFER, count * m_vertexSize, vertices, GL_STATIC_DRAW );
    return;
  }
  
  if( m_vertexUsage == aeVertexUsage::Dynamic )
  {
    m_vertexCount = count;

    if ( !m_vertexCount )
    {
      return;
    }
    
    if( m_vertices == ~0 )
    {
      glGenBuffers( 1, &m_vertices );
      glBindVertexArray( m_array );
      glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
      glBufferData( GL_ARRAY_BUFFER, m_vertexSize * m_maxVertexCount, nullptr, GL_DYNAMIC_DRAW );
    }
    
    glBindVertexArray( m_array );
    glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
    glBufferSubData( GL_ARRAY_BUFFER, 0, count * m_vertexSize, vertices );
    return;
  }
  
  AE_FAIL();
}

void aeVertexData::m_SetIndices( const void* indices, uint32_t count )
{
  AE_ASSERT( m_indexSize );
  AE_ASSERT( count % 3 == 0 );
  AE_ASSERT( count <= m_maxIndexCount );
  
  if( m_indexUsage == aeVertexUsage::Static )
  {
    AE_ASSERT( count );
    AE_ASSERT( !m_indexCount );
    AE_ASSERT( m_indices == ~0 );

    m_indexCount = count;

    glGenBuffers( 1, &m_indices );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indexCount * m_indexSize, indices, GL_STATIC_DRAW );
    return;
  }
  
  if( m_indexUsage == aeVertexUsage::Dynamic )
  {
    m_indexCount = count;

    if ( !m_indexCount )
    {
      return;
    }
    
    if( m_indices == ~0 )
    {
      glGenBuffers( 1, &m_indices );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indexSize * m_maxIndexCount, nullptr, GL_DYNAMIC_DRAW );
    }
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, m_indexCount * m_indexSize, indices );
    return;
  }
  
  AE_FAIL();
}

void aeVertexData::SetVertices( const void *vertices, uint32_t count )
{
  AE_ASSERT( m_vertexSize );
  if ( m_vertexUsage == aeVertexUsage::Static )
  {
    m_SetVertices( vertices, count );
    AE_ASSERT( !m_vertexReadable );
    m_vertexReadable = aeAlloc::AllocateArray< uint8_t >( count * m_vertexSize );
    memcpy( m_vertexReadable, vertices, count * m_vertexSize );
  }
  else if ( m_vertexUsage == aeVertexUsage::Dynamic )
  {
    m_SetVertices( vertices, count );
    if ( !m_vertexReadable ) { m_vertexReadable = aeAlloc::AllocateArray< uint8_t >( m_maxVertexCount * m_vertexSize ); }
    memcpy( m_vertexReadable, vertices, count * m_vertexSize );
  }
  else
  {
    AE_FAIL_MSG( "Invalid vertex usage" );
  }
}

void aeVertexData::SetIndices( const void* indices, uint32_t count )
{
  AE_ASSERT( m_indexSize );

  if ( count && _AE_DEBUG_ )
  {
    uint32_t badIndex = 0;
    
    if ( m_indexSize == 1 )
    {
      uint8_t* indicesCheck = (uint8_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        if ( indicesCheck[ i ] >= m_maxVertexCount )
        {
          badIndex = indicesCheck[ i ];
        }
      }
    }
    else if ( m_indexSize == 2 )
    {
      uint16_t* indicesCheck = (uint16_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        if ( indicesCheck[ i ] >= m_maxVertexCount )
        {
          badIndex = indicesCheck[ i ];
        }
      }
    }
    else if ( m_indexSize == 4 )
    {
      uint32_t* indicesCheck = (uint32_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        if ( indicesCheck[ i ] >= m_maxVertexCount )
        {
          badIndex = indicesCheck[ i ];
        }
      }
    }

    if ( badIndex )
    {
      AE_FAIL_MSG( "Out of range index detected #", badIndex );
    }
  }

  if ( m_indexUsage == aeVertexUsage::Static )
  {
    m_SetIndices( indices, count );
    AE_ASSERT( !m_indexReadable );
    m_indexReadable = aeAlloc::AllocateArray< uint8_t >( count * m_indexSize );
    memcpy( m_indexReadable, indices, count * m_indexSize );
  }
  else if ( m_indexUsage == aeVertexUsage::Dynamic )
  {
    m_SetIndices( indices, count );
    if ( !m_indexReadable ) { m_indexReadable = aeAlloc::AllocateArray< uint8_t >( m_maxIndexCount * m_indexSize ); }
    memcpy( m_indexReadable, indices, count * m_indexSize );
  }
  else
  {
    AE_FAIL_MSG( "Invalid index usage" );
  }
}

const void* aeVertexData::GetVertices() const
{
  AE_ASSERT( m_vertexReadable != nullptr );
  return m_vertexReadable;
}

const void* aeVertexData::GetIndices() const
{
  AE_ASSERT( m_indexReadable != nullptr );
  return m_indexReadable;
}

void aeVertexData::Render( const aeShader* shader, const aeUniformList& uniforms )
{
  Render( shader, 0, uniforms ); // Draw all
}

void aeVertexData::Render( const aeShader* shader, uint32_t primitiveCount, const aeUniformList& uniforms )
{
  if ( m_vertices == ~0 || !m_vertexCount || ( m_indices != ~0 && !m_indexCount ) )
  {
    return;
  }

  shader->Activate( uniforms );

  glBindVertexArray( m_array );
  AE_ASSERT( glGetError() == GL_NO_ERROR );

  glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
  AE_ASSERT( glGetError() == GL_NO_ERROR );

  if ( m_indexCount && m_primitive != aeVertexPrimitive::Point )
  {
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    AE_ASSERT( glGetError() == GL_NO_ERROR );
  }

  for ( uint32_t i = 0; i < shader->GetAttributeCount(); i++ )
  {
    const aeShaderAttribute* shaderAttribute = shader->GetAttributeByIndex( i );
    const aeVertexAttribute* vertexAttribute = m_GetAttributeByName( shaderAttribute->name );

    AE_ASSERT_MSG( vertexAttribute, "No vertex attribute named '#'", shaderAttribute->name );
    // @TODO: Verify attribute type and size match

    GLint location = shaderAttribute->location;
    AE_ASSERT( location != -1 );
    glEnableVertexAttribArray( location );
    AE_ASSERT( glGetError() == GL_NO_ERROR );

    uint32_t componentCount = vertexAttribute->componentCount;
    uint64_t attribOffset = vertexAttribute->offset;
    glVertexAttribPointer( location, componentCount, vertexAttribute->type, GL_FALSE, m_vertexSize, (void*)attribOffset );
    AE_ASSERT( glGetError() == GL_NO_ERROR );
  }

  int64_t start = 0; // TODO: Add support to start drawing at non-zero index
  int32_t count = 0;

  // Draw
  GLenum mode;
  if( m_primitive == aeVertexPrimitive::Triangle )
  {
    count = primitiveCount ? primitiveCount * 3 : m_indexCount;
    mode = GL_TRIANGLES;
  }
  else if( m_primitive == aeVertexPrimitive::Line )
  {
    count = primitiveCount ? primitiveCount * 2 : m_indexCount;
    mode = GL_LINES;
  }
  else if( m_primitive == aeVertexPrimitive::Point )
  {
    count = primitiveCount ? primitiveCount : m_indexCount;
    mode = GL_POINTS;
  }
  else
  {
    AE_FAIL();
    return;
  }
  
  if ( m_indexCount && mode != GL_POINTS )
  {
    if ( count == 0 ) { count = m_indexCount; }
    AE_ASSERT( start + count <= m_indexCount );
    if ( mode == GL_TRIANGLES ) { AE_ASSERT( count % 3 == 0 && start % 3 == 0 ); }
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    AE_ASSERT( glGetError() == GL_NO_ERROR );
    GLenum type = 0;
    if ( m_indexSize == sizeof(uint8_t) ) { type = GL_UNSIGNED_BYTE; }
    else if ( m_indexSize == sizeof(uint16_t) ) { type = GL_UNSIGNED_SHORT; }
    else if ( m_indexSize == sizeof(uint32_t) ) { type = GL_UNSIGNED_INT; }
    glDrawElements( mode, count, type, (void*)start );
    AE_ASSERT( glGetError() == GL_NO_ERROR );
  }
  else
  {
    if ( count == 0 ) { count = m_vertexCount; }
    AE_ASSERT( start + count <= m_vertexCount );
    if ( mode == GL_TRIANGLES ) { AE_ASSERT( count % 3 == 0 && start % 3 == 0 ); }
    
    glDrawArrays( mode, start, count );
    AE_ASSERT( glGetError() == GL_NO_ERROR );
  }
}

const aeVertexAttribute* aeVertexData::m_GetAttributeByName( const char* name ) const
{
  for ( uint32_t i = 0; i < m_attributeCount; i++ )
  {
    if ( strcmp( m_attributes[ i ].name, name ) == 0 )
    {
      return &m_attributes[ i ];
    }
  }

  return nullptr;
}

//------------------------------------------------------------------------------
// aeShader member functions
//------------------------------------------------------------------------------
void aeShader::Initialize( const char* vertexStr, const char* fragStr, const char* const* defines, int32_t defineCount )
{
  m_fragmentShader = 0;
  m_vertexShader = 0;
  m_blending = false;
  m_depthTest = false;
  m_depthWrite = false;
  m_attributeCount = 0;

  m_program = glCreateProgram();
  
  m_vertexShader = m_LoadShader( vertexStr, aeShaderType::Vertex, defines, defineCount );
  m_fragmentShader = m_LoadShader( fragStr, aeShaderType::Fragment, defines, defineCount );
  
  if ( !m_vertexShader || !m_fragmentShader )
  {
    AE_LOG( "Failed to load shader!" );
    AE_FAIL();
  }
  
  glAttachShader( m_program, m_vertexShader );
  glAttachShader( m_program, m_fragmentShader );
  
  glLinkProgram( m_program );
  
  GLint status;
  glGetProgramiv( m_program, GL_LINK_STATUS, &status );
  if( status == GL_FALSE )
  {
    GLint logLength;
    glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &logLength );
    
    if( logLength > 0 )
    {
      unsigned char *log = new unsigned char[ logLength ];
      glGetProgramInfoLog( m_program, logLength, NULL, (GLchar*)log );
      AE_LOG( "#", log );
      delete [] log;
    }
    
    Destroy();
    AE_FAIL();
  }
  
  GLint attribCount = 0;
  glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTES, &attribCount );
  AE_ASSERT( 0 < attribCount && attribCount <= kMaxShaderAttributeCount );
  GLint maxLen = 0;
  glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen );
  AE_ASSERT( 0 < maxLen && maxLen <= kMaxShaderAttributeNameLength );
  for ( int32_t i = 0; i < attribCount; i++ )
  {
    AE_ASSERT( m_attributeCount < countof(m_attributes) );
    aeShaderAttribute* attribute = &m_attributes[ m_attributeCount ];
    m_attributeCount++;

    GLsizei length;
    GLint size;
    glGetActiveAttrib( m_program, i, kMaxShaderAttributeNameLength, &length, &size, (GLenum*)&attribute->type, (GLchar*)attribute->name );
    
    attribute->location = glGetAttribLocation( m_program, attribute->name );
    AE_ASSERT( attribute->location != -1 );
  }
  
  GLint uniformCount = 0;
  maxLen = 0;
  glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &uniformCount );
  glGetProgramiv( m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );
  AE_ASSERT( maxLen <= aeStr32::kMaxLength ); // @TODO: Read from aeShaderUniform

  for( int32_t i = 0; i < uniformCount; i++ )
  {
    aeShaderUniform uniform;

    GLint size = 0;
    char name[ aeStr32::kMaxLength ]; // @TODO: Read from aeShaderUniform
    glGetActiveUniform( m_program, i, sizeof(name), nullptr, &size, (GLenum*)&uniform.type, (GLchar*)name );
    AE_ASSERT( size == 1 );
    
    switch ( uniform.type )
    {
      case GL_SAMPLER_2D:
      case GL_SAMPLER_3D:
      case GL_FLOAT:
      case GL_FLOAT_VEC2:
      case GL_FLOAT_VEC3:
      case GL_FLOAT_VEC4:
      case GL_FLOAT_MAT4:
        break;
      default:
        AE_FAIL_MSG( "Unsupported uniform '#' type #", name, uniform.type );
        break;
    }

    uniform.name = name;
    uniform.location = glGetUniformLocation( m_program, name );
    AE_ASSERT( uniform.location != -1 );

    m_uniforms.Set( name, uniform );
  }
}

void aeShader::Destroy(void)
{
  if( m_fragmentShader != 0 )
  {
    glDeleteShader( m_fragmentShader );
    m_fragmentShader = 0;
  }
  
  if( m_vertexShader != 0 )
  {
    glDeleteShader( m_vertexShader );
    m_vertexShader = 0;
  }
  
  if( m_program != 0 )
  {
    glDeleteProgram( m_program );
    m_program = 0;
  }
}

void aeShader::Activate( const aeUniformList& uniforms ) const
{
  glUseProgram( m_program );

  // Set rendering params
  if ( m_blending )
  {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  else
  {
    glDisable( GL_BLEND );
  }

  if ( m_depthTest )
  {
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
  }
  else
  {
    glDisable( GL_DEPTH_TEST );
  }
  
  glDepthMask( m_depthWrite ? GL_TRUE : GL_FALSE );

  // Set shader uniforms
  uint32_t textureIndex = 0;
  for ( uint32_t i = 0; i < m_uniforms.Length(); i++ )
  {
    const char* uniformVarName = m_uniforms.GetKey( i ).c_str();
    const aeShaderUniform* uniformVar = &m_uniforms.GetValue( i );
    const aeUniformList::Value* uniformValue = uniforms.Get( uniformVarName );
    
    // Start validation
    AE_ASSERT_MSG( uniformValue, "Shader uniform '#' value is not set", uniformVarName );
    uint32_t typeSize = 0;
    switch ( uniformVar->type )
    {
      case GL_SAMPLER_2D:
        typeSize = 0;
        break;
      case GL_SAMPLER_3D:
        typeSize = 0;
        break;
      case GL_FLOAT:
        typeSize = 1;
        break;
      case GL_FLOAT_VEC2:
        typeSize = 2;
        break;
      case GL_FLOAT_VEC3:
        typeSize = 3;
        break;
      case GL_FLOAT_VEC4:
        typeSize = 4;
        break;
      case GL_FLOAT_MAT4:
        typeSize = 16;
        break;
      default:
        AE_FAIL_MSG( "Unsupported uniform '#' type #", uniformVarName, uniformVar->type );
        break;
    }
    AE_ASSERT_MSG( uniformValue->size == typeSize, "Uniform type mismatch '#' type:# size:#", uniformVarName, uniformVar->type, uniformValue->size );
    // End validation

    if ( uniformVar->type == GL_SAMPLER_2D )
    {
      AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid #", uniformVarName, uniformValue->sampler );
      glActiveTexture( GL_TEXTURE0 + textureIndex );
      glBindTexture( GL_TEXTURE_2D, uniformValue->sampler );
      glUniform1i( uniformVar->location, textureIndex );
      textureIndex++;
    }
    else if ( uniformVar->type == GL_SAMPLER_3D )
    {
      AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid #", uniformVarName, uniformValue->sampler );
      glActiveTexture( GL_TEXTURE0 + textureIndex );
      glBindTexture( GL_TEXTURE_3D, uniformValue->sampler );
      glUniform1i( uniformVar->location, textureIndex );
      textureIndex++;
    }
    else if ( uniformVar->type == GL_FLOAT )
    {
      glUniform1fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_VEC2 )
    {
      glUniform2fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_VEC3 )
    {
      glUniform3fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_VEC4 )
    {
      glUniform4fv( uniformVar->location, 1, uniformValue->value.data );
    }
    else if ( uniformVar->type == GL_FLOAT_MAT4 )
    {
#if _AE_EMSCRIPTEN_
      // WebGL/Emscripten doesn't support glUniformMatrix4fv auto-transpose
      aeFloat4x4 transposedTransform = uniformValue->value.GetTransposeCopy();
      glUniformMatrix4fv( uniformVar->location, 1, GL_FALSE, transposedTransform.data );
#else
      glUniformMatrix4fv( uniformVar->location, 1, GL_TRUE, uniformValue->value.data );
#endif
    }
    else
    {
      AE_ASSERT_MSG( false, "Invalid uniform type '#': #", uniformVarName, uniformVar->type );
    }
  }

  AE_ASSERT( glGetError() == GL_NO_ERROR );
}

const aeShaderAttribute* aeShader::GetAttributeByIndex( uint32_t index ) const
{
  AE_ASSERT( index < m_attributeCount );
  return &m_attributes[ index ];
}

int aeShader::m_LoadShader( const char* shaderStr, aeShaderType::Type type, const char* const* defines, int32_t defineCount )
{
  GLenum glType = -1;
  if ( type == aeShaderType::Vertex )
  {
    glType = GL_VERTEX_SHADER;
  }
  if ( type == aeShaderType::Fragment )
  {
    glType = GL_FRAGMENT_SHADER;
  }
  
  const uint32_t kPrependMax = 16;
  uint32_t sourceCount = 0;
  const char* shaderSource[ kPrependMax + kMaxShaderDefines * 2 + 1 ]; // x2 max defines to make room for newlines. Plus one for actual shader.

  // Version
#if _AE_IOS_
  // shaderSource[ 0 ] = "#version 300 es\n";
#elif _AE_EMSCRIPTEN_
  // No version specified
#else
  shaderSource[ sourceCount++ ] = "#version 330 core\n";
#endif

  // Utility
  shaderSource[ sourceCount++ ] = "vec3 AE_SRGB_TO_RGB( vec3 _x) { return _x * _x; }\n";
  shaderSource[ sourceCount++ ] = "vec3 AE_RGB_TO_SRGB( vec3 _x ) { return sqrt( _x ); }\n";
  shaderSource[ sourceCount++ ] = "vec4 AE_SRGBA_TO_RGBA( vec4 _x ) { return vec4( _x.rgb * _x.rgb, _x.a ); }\n";
  shaderSource[ sourceCount++ ] = "vec4 AE_RGBA_TO_SRGBA( vec4 _x ) { return vec4( sqrt( _x.rgb ), _x.a ); }\n";

  // Input/output
#if _AE_IOS_
  // shaderSource[ 1 ] = "precision highp float;\n";
  // shaderSource[ 2 ] = "precision mediump sampler3D;\n";
#elif _AE_EMSCRIPTEN_
  shaderSource[ sourceCount++ ] = "precision highp float;\n";
  shaderSource[ sourceCount++ ] = "#define AE_COLOR gl_FragColor\n";
  shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture2d\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
  // shaderSource[ sourceCount++ ] = "#define AE_FLOAT_HIGHP highp float\n";
  if ( type == aeShaderType::Vertex )
  {
    shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP attribute highp\n";
    shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP varying highp\n";
  }
  else if ( type == aeShaderType::Fragment )
  {
    shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP varying highp\n";
    shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
  }
#else
  shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform\n";
  shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP in\n";
  shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP out\n";
  if ( type == aeShaderType::Fragment )
  {
    shaderSource[ sourceCount++ ] = "out vec4 AE_COLOR;\n";
  }
#endif
  AE_ASSERT( sourceCount <= kPrependMax );

  for ( int32_t i = 0; i < defineCount; i++ )
  {
    shaderSource[ sourceCount ] = defines[ i ];
    sourceCount++;
    shaderSource[ sourceCount ] = "\n";
    sourceCount++;
  }

  shaderSource[ sourceCount ] = shaderStr;
  sourceCount++;
  
  GLuint shader = glCreateShader( glType );
  glShaderSource( shader, sourceCount, shaderSource, nullptr );
  glCompileShader( shader );
  
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(status == GL_FALSE)
  {
    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    
    if (logLength > 0)
    {
      unsigned char* log = new unsigned char[ logLength ];
      glGetShaderInfoLog(shader, logLength, NULL, (GLchar*)log);
      AE_LOG( "Error compiling shader #", log );
      delete[] log;
    }
    
    return 0;
  }
  
  return shader;
}

//------------------------------------------------------------------------------
// aeTexture
//------------------------------------------------------------------------------
#include <png.h>

namespace
{
  struct PngData
  {
    const uint8_t* data;
    uint32_t current;
    uint32_t length;
  };

  void read_data_fn( png_structp png_ptr, png_bytep buffer, png_size_t count )
  {
    PngData *data = (PngData*)png_get_io_ptr( png_ptr );
    AE_ASSERT( data->current + count <= data->length );
    memcpy( buffer, data->data + data->current, count );
    data->current += count;
  }
}

bool get_png_info( const uint8_t* data, uint32_t length, uint32_t* widthOut, uint32_t* heightOut, uint32_t* depthOut )
{
  png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
  png_infop info_ptr = png_create_info_struct( png_ptr );
  png_infop end_info = png_create_info_struct( png_ptr );
  
  PngData pngData;
  pngData.data = data;
  pngData.current = 0;
  pngData.length = length;
  png_set_read_fn( png_ptr, &pngData, read_data_fn );
  
  png_read_info(png_ptr, info_ptr);
  
  int bit_depth;
  int color_type;
  int interlace_method;
  int compression_method;
  int filter_method;
  uint32_t png_width;
  uint32_t png_height;
  png_get_IHDR( png_ptr, info_ptr, &png_width, &png_height,
    &bit_depth, &color_type, &interlace_method,
    &compression_method, &filter_method );

  AE_ASSERT( bit_depth == 8 );

  int32_t perPixel = 0;
  switch( color_type )
  {
    case 2:
      perPixel = 3;
      break;
    case 6:
      perPixel = 4;
      break;
    default:
      return false;
  }

  png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
  
  *widthOut = png_width;
  *heightOut = png_height;
  *depthOut = perPixel;

  return true;
}

bool load_png( const uint8_t* data, uint32_t length, uint8_t* dataOut, uint32_t maxOut )
{
  png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
  png_infop info_ptr = png_create_info_struct( png_ptr );
  png_infop end_info = png_create_info_struct( png_ptr );
  
  PngData pngData;
  pngData.data = data;
  pngData.current = 0;
  pngData.length = length;
  png_set_read_fn( png_ptr, &pngData, read_data_fn );
  
  png_read_info( png_ptr, info_ptr );
  
  int bit_depth;
  int color_type;
  int interlace_method;
  int compression_method;
  int filter_method;
  uint32_t png_width;
  uint32_t png_height;
  png_get_IHDR( png_ptr, info_ptr, &png_width, &png_height,
    &bit_depth, &color_type, &interlace_method,
    &compression_method, &filter_method );

  // @TODO: Get png gamma info
  // double fileGamma = 0.45455;
  // double screenGamma = 2.2;
  // if ( !png_get_gAMA( png_ptr, info_ptr, &fileGamma ) ) { fileGamma = 0.45455; }
  // png_set_gamma( png_ptr, screenGamma, fileGamma );
  // png_read_update_info( png_ptr, info_ptr );

  AE_ASSERT( bit_depth == 8 );
  
  int32_t perPixel = 0;
  switch( color_type )
  {
    case 2:
      perPixel = 3;
      break;
    case 6:
      perPixel = 4;
      break;
    default:
      return false;
  }
  
  const uint32_t byteCount = png_width * png_height * perPixel;
  if ( maxOut < byteCount )
  {
    return false;
  }

  uint8_t** rows = new uint8_t*[ png_height ];
  for ( uint32_t i = 0; i < png_height; i++ )
  {
    rows[ png_height - i - 1 ] = dataOut + i * png_width * perPixel;
  }
  png_set_rows( png_ptr, info_ptr, rows );
  
  png_read_image( png_ptr, rows );
  png_read_end( png_ptr, end_info );
  
  png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
  delete[] rows;
  
  return true;
}

aeTexture2D::aeTexture2D()
{
  m_width = 0;
  m_height = 0;
  m_hasAlpha = false;
}

void aeTexture2D::Initialize( const uint8_t* data, uint32_t width, uint32_t height, uint32_t depth, aeTextureFilter::Type filter, aeTextureWrap::Type wrap )
{
  m_width = width;
  m_height = height;

  glGenTextures( 1, &m_texture );
  glBindTexture( GL_TEXTURE_2D, m_texture );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  
  GLint internalFormat = 0;
  GLenum format = 0;
  GLint unpackAlignment;
  switch ( depth )
  {
    case 1:
      internalFormat = GL_RED;
      format = GL_RED;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
    case 3:
      internalFormat = GL_RGB;
      format = GL_RGB;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
    case 4:
      internalFormat = GL_RGBA;
      format = GL_RGBA;
      unpackAlignment = 4;
      m_hasAlpha = true;
      break;
    default:
      AE_ASSERT_MSG( false, "Invalid texture depth #", depth );
      return;
  }

  glPixelStorei( GL_UNPACK_ALIGNMENT, unpackAlignment );
  glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data );
}

void aeTexture2D::Initialize( const char* file, aeTextureFilter::Type filter, aeTextureWrap::Type wrap )
{
  uint32_t fileSize = aeVfs::GetSize( file );
  AE_ASSERT_MSG( fileSize, "Could not load #", file );
  
  uint8_t* fileBuffer = new uint8_t[ fileSize ];
  aeVfs::Read( file, fileBuffer, fileSize );

  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 0;
  get_png_info( fileBuffer, fileSize, &width, &height, &depth );
  uint8_t* imageBuf = new uint8_t[ width * height * depth ];
  load_png( fileBuffer, fileSize, imageBuf, width * height * depth );
  
  Initialize( imageBuf, width, height, depth, filter, wrap );

  delete[] imageBuf;
  delete[] fileBuffer;
}

void aeTexture2D::Destroy()
{
  glDeleteTextures( 1, &m_texture );
  *this = aeTexture2D();
}

//------------------------------------------------------------------------------
// aeRenderTexture member functions
//------------------------------------------------------------------------------
aeRenderTexture::aeRenderTexture()
{
  m_fbo = 0;

  m_width = 0;
  m_height = 0;
}

void aeRenderTexture::Initialize( uint32_t width, uint32_t height, aeTextureFilter::Type filter, aeTextureWrap::Type wrap )
{
  AE_ASSERT( m_fbo == 0 );
  AE_ASSERT( m_texture == 0 );

  AE_ASSERT( width != 0 );
  AE_ASSERT( height != 0 );

  m_width = width;
  m_height = height;

  glGenFramebuffers( 1, &m_fbo );
  AE_ASSERT( m_fbo );
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );

  glGenTextures( 1, &m_texture );
  AE_ASSERT( m_texture );
  glBindTexture( GL_TEXTURE_2D, m_texture );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr );
  glBindTexture( GL_TEXTURE_2D, 0 );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0 );

  Vertex quadVerts[] =
  {
    { aeQuadVertPos[ 0 ], aeQuadVertUvs[ 0 ] },
    { aeQuadVertPos[ 1 ], aeQuadVertUvs[ 1 ] },
    { aeQuadVertPos[ 2 ], aeQuadVertUvs[ 2 ] },
    { aeQuadVertPos[ 3 ], aeQuadVertUvs[ 3 ] }
  };
  AE_STATIC_ASSERT( countof( quadVerts ) == aeQuadVertCount );
  m_quad.Initialize( sizeof( Vertex ), sizeof( aeQuadIndex ), aeQuadVertCount, aeQuadIndexCount, aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
  m_quad.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof( Vertex, pos ) );
  m_quad.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof( Vertex, uv ) );
  m_quad.SetVertices( quadVerts, aeQuadVertCount );
  m_quad.SetIndices( aeQuadIndices, aeQuadIndexCount );

  const char* vertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_localToNdc;\
    AE_IN_HIGHP vec3 a_position;\
    AE_IN_HIGHP vec2 a_uv;\
    AE_OUT_HIGHP vec2 v_uv;\
    void main()\
    {\
      v_uv = a_uv;\
      gl_Position = u_localToNdc * vec4( a_position, 1.0 );\
    }";
  const char* fragStr = "\
    uniform sampler2D u_tex;\
    AE_IN_HIGHP vec2 v_uv;\
    void main()\
    {\
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv );\
    }";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );
}

void aeRenderTexture::Destroy()
{
  m_shader.Destroy();
  m_quad.Destroy();

  if ( m_texture )
  {
    glDeleteTextures( 1, &m_texture );
    m_texture = 0;
  }

  if ( m_fbo )
  {
    glDeleteFramebuffers( 1, &m_fbo );
    m_fbo = 0;
  }

  m_width = 0;
  m_height = 0;
}

void aeRenderTexture::Activate()
{
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  glViewport( 0, 0, GetWidth(), GetHeight() );
}

void aeRenderTexture::Render2D( aeRect ndc, float z )
{
  aeUniformList uniforms;
  uniforms.Set( "u_localToNdc", aeRenderTexture::GetQuadToNDCTransform( ndc, z ) );
  uniforms.Set( "u_tex", this );
  m_quad.Render( &m_shader, uniforms );
}

aeFloat4x4 aeRenderTexture::GetQuadToNDCTransform( aeRect ndc, float z )
{
  aeFloat4x4 localToNdc = aeFloat4x4::Translation( aeFloat3( ndc.x, ndc.y, z ) );
  localToNdc.Scale( aeFloat3( ndc.w, ndc.h, 1.0f ) );
  localToNdc.Translate( aeFloat3( 0.5f, 0.5f, 0.0f ) );
  return localToNdc;
}

//------------------------------------------------------------------------------
// aeSpriteRenderer member functions
//------------------------------------------------------------------------------
void aeSpriteRenderer::Initialize( uint32_t maxCount )
{
  m_maxCount = maxCount;
  m_count = 0;
  m_sprites = aeAlloc::AllocateArray< Sprite >( m_maxCount );

  m_vertexData.Initialize( sizeof(Vertex), sizeof(uint16_t), aeQuadVertCount * maxCount, aeQuadIndexCount * maxCount, aeVertexPrimitive::Triangle, aeVertexUsage::Dynamic, aeVertexUsage::Static );
  m_vertexData.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof(Vertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, aeVertexDataType::Float, offsetof(Vertex, color) );
  m_vertexData.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof(Vertex, uv) );

  aeAlloc::Scratch< uint16_t > scratch( m_maxCount * aeQuadIndexCount );
  uint16_t* indices = scratch.GetData();
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
  m_shader.SetBlending( true );
  m_shader.SetDepthTest( true );
}

void aeSpriteRenderer::Destroy()
{
  m_shader.Destroy();
  m_vertexData.Destroy();
  
  aeAlloc::Release( m_sprites );
  m_sprites = nullptr;
}

void aeSpriteRenderer::Render( const aeFloat4x4& worldToScreen )
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
    Vertex* vertices = scratch.GetData();
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
    m_vertexData.SetVertices( vertices, scratch.GetCount() );

    aeUniformList uniforms;
    uniforms.Set( "u_worldToScreen", worldToScreen );
    uniforms.Set( "u_tex", texture );

    m_vertexData.Render( &m_shader, m_count * 2, uniforms );
  }

  Clear();
}

void aeSpriteRenderer::AddSprite( const aeTexture2D* texture, aeFloat4x4 transform, aeFloat2 uvMin, aeFloat2 uvMax, aeColor color )
{
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

void aeSpriteRenderer::Clear()
{
  m_count = 0;
  m_textures.Clear();
}

//------------------------------------------------------------------------------
// Text constants
//------------------------------------------------------------------------------
const uint32_t kCharsPerString = 64;

//------------------------------------------------------------------------------
// aeTextRenderer member functions
//------------------------------------------------------------------------------
void aeTextRenderer::Initialize( const char* imagePath, aeTextureFilter::Type filterType, uint32_t charSize )
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
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv );\
      AE_COLOR.rgb = AE_SRGB_TO_RGB( AE_COLOR.rgb );\
      AE_COLOR *= v_color;\
      AE_COLOR.rgb = AE_RGB_TO_SRGB( AE_COLOR.rgb );\
    }";
  m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );
  m_shader.SetBlending( true );

  m_texture.Initialize( imagePath, filterType, aeTextureWrap::Clamp );
}

void aeTextRenderer::Terminate()
{
  m_texture.Destroy();
  m_shader.Destroy();
  m_vertexData.Destroy();
}

void aeTextRenderer::Render( const aeFloat4x4& uiToScreen )
{
  uint32_t vertCount = 0;
  uint32_t indexCount = 0;
  aeAlloc::Scratch< Vertex > verts( m_vertexData.GetMaxVertexCount() );
  aeAlloc::Scratch< uint16_t > indices( m_vertexData.GetMaxIndexCount() );

  for ( uint32_t i = 0; i < m_rectCount; i++ )
  {
    const TextRect& rect = m_rects[ i ];
    aeFloat2 pos = rect.pos;

    const char* start = rect.text.c_str();
    const char* str = start;
    while ( str[ 0 ] )
    {
      if ( !isspace( str[ 0 ] ) )
      {
        AE_ASSERT_MSG( str[ 0 ] >= 33 && str[ 0 ] <= 126, "Text::Render string value '#': #", (int)str[ 0 ], rect.text.c_str() );

        int32_t index = str[ 0 ] - 33;
        uint32_t columns = m_texture.GetWidth() / m_charSize;
        aeFloat2 offset( index % columns, index / columns );

        for ( uint32_t j = 0; j < aeQuadIndexCount; j++ )
        {
          indices.GetSafe( indexCount ) = aeQuadIndices[ j ] + vertCount;
          indexCount++;
        }

        AE_ASSERT( vertCount + aeQuadVertCount <= verts.GetCount() );
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

  m_vertexData.SetVertices( verts.GetData(), vertCount );
  m_vertexData.SetIndices( indices.GetData(), indexCount );

  aeUniformList uniforms;
  uniforms.Set( "u_uiToScreen", uiToScreen );
  uniforms.Set( "u_tex", &m_texture );
  m_vertexData.Render( &m_shader, uniforms );

  m_rectCount = 0;
}

void aeTextRenderer::Add( aeFloat2 pos, aeFloat2 size, const char* str, aeColor color, uint32_t lineLength, uint32_t charLimit )
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

uint32_t aeTextRenderer::GetLineCount( const char* str, uint32_t lineLength, uint32_t charLimit ) const
{
  return m_ParseText( str, lineLength, charLimit, nullptr );
}

uint32_t aeTextRenderer::m_ParseText( const char* str, uint32_t lineLength, uint32_t charLimit, aeStr512* outText ) const
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
// aeRenderer member functions
//------------------------------------------------------------------------------
aeRenderer::aeRenderer()
{
  m_window = nullptr;
  m_context = nullptr;

  m_targetWidth = 0;
  m_targetHeight = 0;

  m_clearColor = aeColor::Black;
  
  m_defaultFbo = 0;
}

void aeRenderer::Initialize( aeWindow* window, uint32_t width, uint32_t height )
{
  m_targetWidth = width;
  m_targetHeight = height;

  m_window = window;

  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 16 );

  // @TODO: Add aeWindow to avoid assumed type and SDL_WINDOW cast
  m_context = SDL_GL_CreateContext( (SDL_Window*)m_window->window );
  SDL_GL_MakeCurrent( (SDL_Window*)m_window->window, m_context );

  SDL_GL_SetSwapInterval( 1 );

#if _AE_WINDOWS_
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  glGetError(); // Glew currently has an issue which causes a GL_INVALID_ENUM on init
  AE_ASSERT_MSG( err == GLEW_OK, "Could not initialize glew" );
#endif

  // @HACK: No other place to do this outside of game modules currently
  SDL_JoystickEventState( SDL_ENABLE );

  glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFbo );
}

void aeRenderer::Terminate()
{
  SDL_GL_DeleteContext( m_context );
}

void aeRenderer::StartFrame()
{
  if ( m_targetWidth != m_canvas.GetWidth() || m_targetHeight != m_canvas.GetHeight() )
  {
    m_canvas.Destroy();
    m_canvas.Initialize( m_targetWidth, m_targetHeight, aeTextureFilter::Nearest, aeTextureWrap::Clamp );
  }

  m_canvas.Activate();

  aeFloat3 clearColor = m_clearColor.GetSRGB(); // Unclear why glClearColor() expects srgb. Maybe because of framebuffer type.
  glClearColor( clearColor.x, clearColor.y, clearColor.z, 1.0f );
  glClearDepth( 1.0f );
  glDepthMask( GL_TRUE ); // Must be true for glClear to work
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glDisable( GL_CULL_FACE ); // @TODO: Should probably be a VertexData parameter
  glEnable( GL_MULTISAMPLE ); // @TODO: Should probably be a VertexData parameter
}

void aeRenderer::EndFrame()
{
  glBindFramebuffer( GL_FRAMEBUFFER, m_defaultFbo );

  glViewport( 0, 0, m_window->GetWidth(), m_window->GetHeight() );

  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glClearDepth( 1.0f );
  glDepthMask( GL_TRUE ); // Must be true for glClear to work
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glDisable( GL_CULL_FACE );
  glEnable( GL_MULTISAMPLE ); // @TODO: Probably not needed

  m_canvas.Render2D( m_GetNDCRect(), 0.5f );

#if !_AE_EMSCRIPTEN_
  SDL_GL_SwapWindow( (SDL_Window*)m_window->window );
#endif
}

void aeRenderer::Resize( uint32_t width, uint32_t height )
{
  m_targetWidth = width;
  m_targetHeight = height;
}

void aeRenderer::SetClearColor( aeColor color )
{
  m_clearColor = color;
}

aeColor aeRenderer::GetClearColor() const
{
  return m_clearColor;
}

aeFloat4x4 aeRenderer::GetWindowToRenderTransform()
{
  aeFloat4x4 windowToNDC = aeFloat4x4::Translation( aeFloat3( -1.0f, -1.0f, 0.0f ) );
  windowToNDC.Scale( aeFloat3( 2.0f / m_window->GetWidth(), 2.0f / m_window->GetHeight(), 1.0f ) );
  
  aeFloat4x4 ndcToQuad = aeRenderTexture::GetQuadToNDCTransform( m_GetNDCRect(), 0.0f );
  ndcToQuad.Invert();
  
  aeFloat4x4 quadToRender = aeFloat4x4::Scaling( aeFloat3( m_canvas.GetWidth(), m_canvas.GetHeight(), 1.0f ) );
  quadToRender.Translate( aeFloat3( 0.5f, 0.5f, 0.0f ) );
  
  return ( quadToRender * ndcToQuad * windowToNDC );
}

aeRect aeRenderer::m_GetNDCRect() const
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
