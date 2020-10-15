//------------------------------------------------------------------------------
// aeOpenGLRender.cpp
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
#include "aeRender.h"
#include "aeAlloc.h"
#include "aeCommonRender.h"
#include "aeLog.h"
#include "aePlatform.h"
#include "aeVfs.h"
#include "aeWindow.h"

#include "SDL.h"
#if _AE_WINDOWS_
  #define GLEW_STATIC 1
  #include <GL\glew.h>
#elif _AE_EMSCRIPTEN_
  #include <GLES2/gl2.h>
#elif _AE_LINUX_
  #include <GL/gl.h>
  #include <GLES3/gl3.h>
#elif _AE_IOS_
  #include <OpenGLES/ES3/gl.h>
  #include <OpenGLES/ES3/glext.h>
  #define glClearDepth glClearDepthf
#else
  #include <OpenGL/gl3.h>
  #include <OpenGL/gl3ext.h> // for glTexStorage2D
#endif

#include <stb_image.h>

#if _AE_DEBUG_ && !_AE_APPLE_
  #define AE_GL_DEBUG_MODE 1
#else
  #define AE_GL_DEBUG_MODE 0
#endif

#if _AE_EMSCRIPTEN_
  #define GL_SAMPLER_3D 0x0
  #define GL_TEXTURE_3D 0x0
  void glGenVertexArrays( GLsizei n, GLuint *arrays ) {}
  void glDeleteVertexArrays( GLsizei n, const GLuint *arrays ) {}
  void glBindVertexArray( GLuint array ) {}
  void glBindFragDataLocation( GLuint program, GLuint colorNumber, const char* name ) {}
  void glClearDepth( float depth ) { glClearDepthf( depth ); }
#endif

// TODO: needed on ES2/GL/WebGL1, but not on ES3/WebGL2, only adding for OSX right now
// TODO: this will break any code using the SRGB_TO_RGB macros in shaders on this platform,
// But the colors and blends will be handled correctly.
#if _AE_OSX_ && defined(GL_ARB_framebuffer_sRGB)
  #define READ_FROM_SRGB 1
  #define WRITE_TO_SRGB  1
#else
  #define READ_FROM_SRGB 0
  #define WRITE_TO_SRGB  0
#endif

  const uint32_t kMaxFrameBufferAttachments = 16;

// Caller enables this externally.  The renderer, AEShader, math aren't tied to one another
// enough to pass this locally.  glClipControl is also no accessible in ES or GL 4.1, so
// doing this just to write the shaders for reverseZ.  In GL, this won't improve precision.
// http://www.reedbeta.com/blog/depth-precision-visualized/
bool gReverseZ = false;

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
    case aeVertexDataType::NormalizedUInt8:
      return GL_UNSIGNED_BYTE;
    case aeVertexDataType::NormalizedUInt16:
      return GL_UNSIGNED_SHORT;
    case aeVertexDataType::NormalizedUInt32:
      return GL_UNSIGNED_INT;
    case aeVertexDataType::Float:
      return GL_FLOAT;
    default:
      AE_FAIL();
      return 0;
  }
}

#define AE_CHECK_GL_ERROR() do { if ( GLenum err = glGetError() ) { AE_FAIL_MSG( "GL Error: #", err ); } } while ( 0 )

void CheckFramebufferComplete( GLuint framebuffer )
{
  GLenum fboStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
  if ( fboStatus != GL_FRAMEBUFFER_COMPLETE )
  {
    const char* errStr = "unknown";
    switch ( fboStatus )
    {
      case GL_FRAMEBUFFER_UNDEFINED:
        errStr = "GL_FRAMEBUFFER_UNDEFINED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
#endif
      case GL_FRAMEBUFFER_UNSUPPORTED:
        errStr = "GL_FRAMEBUFFER_UNSUPPORTED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        errStr = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;
#endif
      default:
        break;
    }
    AE_FAIL_MSG( "GL FBO Error: (#) #", fboStatus, errStr );
  }
}

#if AE_GL_DEBUG_MODE
void aeOpenGLDebugCallback( GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam )
{
  //std::cout << "---------------------opengl-callback-start------------" << std::endl;
  //std::cout << "message: " << message << std::endl;
  //std::cout << "type: ";
  //switch ( type )
  //{
  //  case GL_DEBUG_TYPE_ERROR:
  //    std::cout << "ERROR";
  //    break;
  //  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
  //    std::cout << "DEPRECATED_BEHAVIOR";
  //    break;
  //  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
  //    std::cout << "UNDEFINED_BEHAVIOR";
  //    break;
  //  case GL_DEBUG_TYPE_PORTABILITY:
  //    std::cout << "PORTABILITY";
  //    break;
  //  case GL_DEBUG_TYPE_PERFORMANCE:
  //    std::cout << "PERFORMANCE";
  //    break;
  //  case GL_DEBUG_TYPE_OTHER:
  //    std::cout << "OTHER";
  //    break;
  //}
  //std::cout << std::endl;

  //std::cout << "id: " << id << std::endl;
  //std::cout << "severity: ";
  switch ( severity )
  {
    case GL_DEBUG_SEVERITY_LOW:
      //std::cout << "LOW";
      //AE_INFO( message );
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      //std::cout << "MEDIUM";
      AE_WARN( message );
      break;
    case GL_DEBUG_SEVERITY_HIGH:
      //std::cout << "HIGH";
      AE_ERR( message );
      break;
  }
  //std::cout << std::endl;
  //std::cout << "---------------------opengl-callback-end--------------" << std::endl;

  if ( severity == GL_DEBUG_SEVERITY_HIGH )
  {
    AE_FAIL();
  }
}
#endif

//------------------------------------------------------------------------------
// aeVertexData member functions
//------------------------------------------------------------------------------
aeVertexData::~aeVertexData()
{
  Destroy();
}

void aeVertexData::Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, aeVertexPrimitive::Type primitive, aeVertexUsage::Type vertexUsage, aeVertexUsage::Type indexUsage )
{
  Destroy();

  AE_ASSERT( m_vertexSize == 0 );
  AE_ASSERT( vertexSize );
  AE_ASSERT( m_indexSize == 0 );
  AE_ASSERT( indexSize == sizeof(uint8_t) || indexSize == sizeof(uint16_t) || indexSize == sizeof(uint32_t) );

  m_maxVertexCount = maxVertexCount;
  m_maxIndexCount = maxIndexCount;
  m_primitive = primitive;
  m_vertexUsage = vertexUsage;
  m_indexUsage = indexUsage;
  m_vertexSize = vertexSize;
  m_indexSize = indexSize;
  
  glGenVertexArrays( 1, &m_array );
  glBindVertexArray( m_array );
}

void aeVertexData::Destroy()
{
  if ( m_vertexReadable )
  {
    aeAlloc::Release( (uint8_t*)m_vertexReadable );
  }
  if ( m_indexReadable )
  {
    aeAlloc::Release( (uint8_t*)m_indexReadable );
  }
  
  if ( m_array )
  {
    glDeleteVertexArrays( 1, &m_array );
  }
  if ( m_vertices != ~0 )
  {
    glDeleteBuffers( 1, &m_vertices );
  }
  if ( m_indices != ~0 )
  {
    glDeleteBuffers( 1, &m_indices );
  }
  
  m_array = 0;
  m_vertices = ~0;
  m_indices = ~0;
  m_vertexCount = 0;
  m_indexCount = 0;

  m_maxVertexCount = 0;
  m_maxIndexCount = 0;

  m_primitive = ( aeVertexPrimitive::Type ) - 1;
  m_vertexUsage = ( aeVertexUsage::Type ) - 1;
  m_indexUsage = ( aeVertexUsage::Type ) - 1;

  m_attributeCount = 0;
  m_vertexSize = 0;
  m_indexSize = 0;

  m_vertexReadable = nullptr;
  m_indexReadable = nullptr;
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
  attribute->normalized =
    type == aeVertexDataType::NormalizedUInt8 ||
    type == aeVertexDataType::NormalizedUInt16 ||
    type == aeVertexDataType::NormalizedUInt32;
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
    int32_t badIndex = -1;
    
    if ( m_indexSize == 1 )
    {
      uint8_t* indicesCheck = (uint8_t*)indices;
      for ( uint32_t i = 0; i < count; i++ )
      {
        if ( indicesCheck[ i ] >= m_maxVertexCount )
        {
          badIndex = indicesCheck[ i ];
          break;
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
          break;
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
          break;
        }
      }
    }

    if ( badIndex >= 0 )
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
  AE_ASSERT_MSG( m_vertexSize && m_indexSize, "Must call Initialize() before Render()" );
  AE_ASSERT( shader );
  
  if ( m_vertices == ~0 || !m_vertexCount || ( m_indices != ~0 && !m_indexCount ) )
  {
    return;
  }

  shader->Activate( uniforms );

  glBindVertexArray( m_array );
  AE_CHECK_GL_ERROR();

  glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
  AE_CHECK_GL_ERROR();

  if ( m_indexCount && m_primitive != aeVertexPrimitive::Point )
  {
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    AE_CHECK_GL_ERROR();
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
    AE_CHECK_GL_ERROR();

    uint32_t componentCount = vertexAttribute->componentCount;
    uint64_t attribOffset = vertexAttribute->offset;
    glVertexAttribPointer( location, componentCount, vertexAttribute->type, vertexAttribute->normalized, m_vertexSize, (void*)attribOffset );
    AE_CHECK_GL_ERROR();
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
    AE_CHECK_GL_ERROR();
    GLenum type = 0;
    if ( m_indexSize == sizeof(uint8_t) ) { type = GL_UNSIGNED_BYTE; }
    else if ( m_indexSize == sizeof(uint16_t) ) { type = GL_UNSIGNED_SHORT; }
    else if ( m_indexSize == sizeof(uint32_t) ) { type = GL_UNSIGNED_INT; }
    glDrawElements( mode, count, type, (void*)start );
    AE_CHECK_GL_ERROR();
  }
  else
  {
    if ( count == 0 ) { count = m_vertexCount; }
    AE_ASSERT( start + count <= m_vertexCount );
    if ( mode == GL_TRIANGLES ) { AE_ASSERT( count % 3 == 0 && start % 3 == 0 ); }
    
    glDrawArrays( mode, start, count );
    AE_CHECK_GL_ERROR();
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
aeShader::aeShader()
{
  m_fragmentShader = 0;
  m_vertexShader = 0;
  m_program = 0;
  
  m_blending = false;
  m_depthTest = false;
  m_depthWrite = false;
  m_culling = aeShaderCulling::None;
  m_wireframe = false;

  m_attributeCount = 0;
}

aeShader::~aeShader()
{
  Destroy();
}

void aeShader::Initialize( const char* vertexStr, const char* fragStr, const char* const* defines, int32_t defineCount )
{
  AE_CHECK_GL_ERROR();
  AE_ASSERT( !m_program );

  m_program = glCreateProgram();
  
  m_vertexShader = m_LoadShader( vertexStr, aeShaderType::Vertex, defines, defineCount );
  m_fragmentShader = m_LoadShader( fragStr, aeShaderType::Fragment, defines, defineCount );
  
  if ( !m_vertexShader )
  {
	AE_LOG( "Failed to load vertex shader! #", vertexStr );
  }
  if ( !m_fragmentShader )
  {
    AE_LOG( "Failed to load fragment shader! #", fragStr );
  }
	
  if ( !m_vertexShader || !m_fragmentShader )
  {
	AE_FAIL();
  }
	
  glAttachShader( m_program, m_vertexShader );
  glAttachShader( m_program, m_fragmentShader );
  
  glLinkProgram( m_program );
  
  // immediate reflection of shader can be delayed by compiler and optimizer and can stll
  GLint status;
  glGetProgramiv( m_program, GL_LINK_STATUS, &status );
  if( status == GL_FALSE )
  {
    GLint logLength = 0;
    glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &logLength );
    
    char* log = nullptr;
    if( logLength > 0 )
    {
      log = new char[ logLength ];
      glGetProgramInfoLog( m_program, logLength, NULL, (GLchar*)log );
    }
    
    if ( log )
    {
      AE_FAIL_MSG( log );
      delete[] log;
    }
    else
    {
      AE_FAIL();
    }
    Destroy();
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
  AE_ASSERT( maxLen <= (GLint)aeStr32::MaxLength() ); // @TODO: Read from aeShaderUniform

  for( int32_t i = 0; i < uniformCount; i++ )
  {
    aeShaderUniform uniform;

    GLint size = 0;
    char name[ aeStr32::MaxLength() ]; // @TODO: Read from aeShaderUniform
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

  AE_CHECK_GL_ERROR();
}

void aeShader::Destroy()
{
  m_attributeCount = 0;

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
  AE_CHECK_GL_ERROR();

  // This is really context state shadow, and that should be able to override
  // so reverseZ for example can be set without the shader knowing about that.
	
  // Blending
  if ( m_blending )
  {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  else
  {
    glDisable( GL_BLEND );
  }

  // Depth write
  glDepthMask( m_depthWrite ? GL_TRUE : GL_FALSE );

  // Depth test
  if ( m_depthTest )
  {
    glDepthFunc( gReverseZ ? GL_GEQUAL : GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
  }
  else
  {
    glDisable( GL_DEPTH_TEST );
  }

  // Culling
  if ( m_culling == aeShaderCulling::None )
  {
    glDisable( GL_CULL_FACE );
  }
  else
  {
    glEnable( GL_CULL_FACE );
    glFrontFace( ( m_culling == aeShaderCulling::ClockwiseFront ) ? GL_CW : GL_CCW );
  }

  // Wireframe
  glPolygonMode( GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL );

  // Now setup the shader
  glUseProgram( m_program );

  // Set shader uniforms
  bool missingUniforms = false;
  uint32_t textureIndex = 0;
  for ( uint32_t i = 0; i < m_uniforms.Length(); i++ )
  {
    const char* uniformVarName = m_uniforms.GetKey( i ).c_str();
    const aeShaderUniform* uniformVar = &m_uniforms.GetValue( i );
    const aeUniformList::Value* uniformValue = uniforms.Get( uniformVarName );
    
    // Start validation
    if ( !uniformValue )
    {
      AE_WARN( "Shader uniform '#' value is not set", uniformVarName );
      missingUniforms = true;
      continue;
    }
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
    AE_ASSERT_MSG( uniformValue->size == typeSize, "Uniform size mismatch '#' type:# var:# param:#", uniformVarName, uniformVar->type, typeSize, uniformValue->size );
    // End validation

    if ( uniformVar->type == GL_SAMPLER_2D )
    {
      AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid #", uniformVarName, uniformValue->sampler );
      glActiveTexture( GL_TEXTURE0 + textureIndex );
      glBindTexture( uniformValue->target, uniformValue->sampler );
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

    AE_CHECK_GL_ERROR();
  }

  AE_ASSERT_MSG( !missingUniforms, "Missing shader uniform parameters" );
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
  shaderSource[ sourceCount++ ] = "#version 300 es\n";
  shaderSource[ sourceCount++ ] = "precision highp float;\n";
#elif _AE_EMSCRIPTEN_
  // No version specified
  shaderSource[ sourceCount++ ] = "precision highp float;\n";
#else
  shaderSource[ sourceCount++ ] = "#version 330 core\n";
  // No default precision specified
#endif

  // Input/output
#if _AE_EMSCRIPTEN_
  shaderSource[ sourceCount++ ] = "#define AE_COLOR gl_FragColor\n";
  shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture2d\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
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
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM uniform\n";
  shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform\n";
  shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP in\n";
  shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP out\n";
  if ( type == aeShaderType::Fragment )
  {
    shaderSource[ sourceCount++ ] = "out vec4 AE_COLOR;\n";
  }
#endif
    
  // TODO: don't use these macros anymore.  Fix srgb writes in engine on srgb/a textures.
    // Utility
  shaderSource[ sourceCount++ ] = "float AE_SRGB_TO_RGB( float _x ) { return pow( _x, 2.2 ); }\n";
  shaderSource[ sourceCount++ ] = "float AE_RGB_TO_SRGB( float _x ) { return pow( _x, 1.0 / 2.2 ); }\n";
  shaderSource[ sourceCount++ ] = "vec3 AE_SRGB_TO_RGB( vec3 _x ) { return vec3( AE_SRGB_TO_RGB( _x.r ), AE_SRGB_TO_RGB( _x.g ), AE_SRGB_TO_RGB( _x.b ) ); }\n";
  shaderSource[ sourceCount++ ] = "vec3 AE_RGB_TO_SRGB( vec3 _x ) { return vec3( AE_RGB_TO_SRGB( _x.r ), AE_RGB_TO_SRGB( _x.g ), AE_RGB_TO_SRGB( _x.b ) ); }\n";
  shaderSource[ sourceCount++ ] = "vec4 AE_SRGBA_TO_RGBA( vec4 _x ) { return vec4( AE_SRGB_TO_RGB( _x.rgb ), _x.a ); }\n";
  shaderSource[ sourceCount++ ] = "vec4 AE_RGBA_TO_SRGBA( vec4 _x ) { return vec4( AE_RGB_TO_SRGB( _x.rgb ), _x.a ); }\n";
    
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
      const char* typeStr = ( type == aeShaderType::Vertex ? "vertex" : "fragment" );
      AE_LOG( "Error compiling # shader #", typeStr, log );
      delete[] log;
    }
    
    return 0;
  }
  
  AE_CHECK_GL_ERROR();
  return shader;
}

//------------------------------------------------------------------------------
// aeTexture member functions
//------------------------------------------------------------------------------
aeTexture::~aeTexture()
{
  // @NOTE: Only aeTexture should call virtual Destroy() so it only runs once
  Destroy();
}

void aeTexture::Initialize( uint32_t target )
{
  // @NOTE: To avoid undoing any initialization logic only aeTexture should
  //        call Destroy() on initialize, and inherited Initialize()'s should
  //        always call Base::Initialize() before any other logic.
  Destroy();

  m_target = target;

  glGenTextures( 1, &m_texture );
  AE_ASSERT( m_texture );
}

void aeTexture::Destroy()
{
  if ( m_texture )
  {
    glDeleteTextures( 1, &m_texture );
  }

  m_texture = 0;
  m_target = 0;
}

//------------------------------------------------------------------------------
// aeTexture2D member functions
//------------------------------------------------------------------------------
void aeTexture2D::Initialize( const void* data, uint32_t width, uint32_t height, aeTextureFormat::Type format, aeTextureType::Type type, aeTextureFilter::Type filter, aeTextureWrap::Type wrap, bool autoGenerateMipmaps )
{
  aeTexture::Initialize( GL_TEXTURE_2D );

  m_width = width;
  m_height = height;

  glBindTexture( GetTarget(), GetTexture() );

  glTexParameteri( GetTarget(), GL_TEXTURE_MIN_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  glTexParameteri( GetTarget(), GL_TEXTURE_MAG_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  glTexParameteri( GetTarget(), GL_TEXTURE_WRAP_S, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  glTexParameteri( GetTarget(), GL_TEXTURE_WRAP_T, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );

  // this is the type of data passed in, conflating with internal format type
  GLenum glType = 0;
  switch ( type )
  {
    case aeTextureType::Uint8:
      glType = GL_UNSIGNED_BYTE;
      break;
    case aeTextureType::HalfFloat:
      glType = GL_HALF_FLOAT;
      break;
    case aeTextureType::Float:
      glType = GL_FLOAT;
      break;
    default:
      AE_FAIL_MSG( "Invalid texture type #", type );
      return;
  }

  GLint glInternalFormat = 0;
  GLenum glFormat = 0;
  GLint unpackAlignment = 0;
  switch ( format )
  {
    case aeTextureFormat::Depth:
      glInternalFormat = GL_DEPTH_COMPONENT32F;
      glFormat = GL_DEPTH_COMPONENT;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
    case aeTextureFormat::R:
	  switch(type)
	  {
		  case aeTextureType::Uint8: glInternalFormat = GL_R8; break;
		  case aeTextureType::HalfFloat: glInternalFormat = GL_R16F; break;
		  case aeTextureType::Float: glInternalFormat = GL_R32F; break;
	  }
			
      glFormat = GL_RED;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
		  
#if _AE_OSX_
	  // RedGreen, TODO: extend to other ES but WebGL1 left those constants out IIRC
	  case aeTextureFormat::RG:
		switch(type)
		{
			case aeTextureType::Uint8: glInternalFormat = GL_RG8; break;
			case aeTextureType::HalfFloat: glInternalFormat = GL_RG16F; break;
			case aeTextureType::Float: glInternalFormat = GL_RG32F; break;
		}
			  
		glFormat = GL_RG;
		unpackAlignment = 1;
		m_hasAlpha = false;
		break;
#endif
    case aeTextureFormat::RGB:
	  switch(type)
	  {
	    case aeTextureType::Uint8: glInternalFormat = GL_RGB8; break;
	    case aeTextureType::HalfFloat: glInternalFormat = GL_RGB16F; break;
	    case aeTextureType::Float: glInternalFormat = GL_RGB32F; break;
	  }
      glFormat = GL_RGB;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
    case aeTextureFormat::RGBA:
	  switch(type)
	  {
		case aeTextureType::Uint8: glInternalFormat = GL_RGBA8; break;
		case aeTextureType::HalfFloat: glInternalFormat = GL_RGBA16F; break;
		case aeTextureType::Float: glInternalFormat = GL_RGBA32F; break;
	  }
      glFormat = GL_RGBA;
      unpackAlignment = 1;
      m_hasAlpha = true;
      break;
		  
      // TODO: fix these constants, but they differ on ES2/3 and GL
      // WebGL1 they require loading an extension (if present) to get at the constants.
#if READ_FROM_SRGB      
    case aeTextureFormat::SRGB:
	  // ignore type
      glInternalFormat = GL_SRGB8;
      glFormat = GL_SRGB;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
    case aeTextureFormat::SRGBA:
	  // ignore type
      glInternalFormat = GL_SRGB8_ALPHA8;
      glFormat = GL_SRGB_ALPHA;
      unpackAlignment = 1;
      m_hasAlpha = false;
      break;
#endif
    default:
      AE_FAIL_MSG( "Invalid texture format #", format );
      return;
  }

  if ( data )
  {
    glPixelStorei( GL_UNPACK_ALIGNMENT, unpackAlignment );
  }

    // count the mip levels
	int w = width;
	int h = height;
	
	int numberOfMipmaps = 1;
	if ( autoGenerateMipmaps )
	{
		while ( w > 1 || h > 1 )
		{
		  numberOfMipmaps++;
		  w = (w+1) / 2;
		  h = (h+1) / 2;
		}
	}
	
	// allocate mip levels
	// texStorage is GL4.2, so not on macOS.  ES emulates the call internaly.
#define USE_TEXSTORAGE 0
#if USE_TEXSTORAGE
	// TODO: enable glTexStorage on all platforms, this is in gl3ext.h for GL
	// It allocates a full mip chain all at once, and can handle formats glTexImage2D cannot
	// for compressed textures.
	glTexStorage2D( GetTarget(), numberOfMipmaps, glInternalFormat, width, height );
#else
	w = width;
	h = height;
	
	for ( int i = 0; i < numberOfMipmaps; ++i )
	{
	  glTexImage2D( GetTarget(), i, glInternalFormat, w, h, 0, glFormat, glType, NULL );
	  w = (w+1) / 2;
	  h = (h+1) / 2;
	}
#endif
	
  if ( data != nullptr )
  {
	  // upload the first mipmap
	  glTexSubImage2D( GetTarget(), 0, 0,0, width, height, glFormat, glType, data );

	  // autogen only works for uncompressed textures
	  // Also need to know if format is filterable on platform, or this will fail (f.e. R32F)
	  if ( numberOfMipmaps > 1 && autoGenerateMipmaps )
	  {
		glGenerateMipmap( GetTarget() );
	  }
  }
	
  AE_CHECK_GL_ERROR();
}

void aeTexture2D::Initialize( const char* file, aeTextureFilter::Type filter, aeTextureWrap::Type wrap, bool autoGenerateMipmaps )
{
  uint32_t fileSize = aeVfs::GetSize( file );
  AE_ASSERT_MSG( fileSize, "Could not load #", file );
  
  uint8_t* fileBuffer = (uint8_t*)malloc( fileSize );
  aeVfs::Read( file, fileBuffer, fileSize );

  int32_t width = 0;
  int32_t height = 0;
  int32_t channels = 0;
  stbi_set_flip_vertically_on_load( 1 );
#if _AE_IOS_
  stbi_convert_iphone_png_to_rgb( 1 );
#endif
  uint8_t* image = stbi_load_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  AE_ASSERT( image );

  aeTextureFormat::Type format;
  switch ( channels )
  {
    case STBI_grey:
      format = aeTextureFormat::R;
      break;
    case STBI_grey_alpha:
      AE_FAIL();
      break;
    case STBI_rgb:
      format = aeTextureFormat::RGB;
      break;
    case STBI_rgb_alpha:
      format = aeTextureFormat::RGBA;
      break;
  }
  
  Initialize( image, width, height, format, aeTextureType::Uint8, filter, wrap, autoGenerateMipmaps );
  
  stbi_image_free( image );
  free( fileBuffer );
}

void aeTexture2D::Destroy()
{
  m_width = 0;
  m_height = 0;
  m_hasAlpha = false;

  aeTexture::Destroy();
}

//------------------------------------------------------------------------------
// aeRenderTarget member functions
//------------------------------------------------------------------------------
aeRenderTarget::~aeRenderTarget()
{
  Destroy();
}

void aeRenderTarget::Initialize( uint32_t width, uint32_t height )
{
  Destroy();

  AE_ASSERT( m_fbo == 0 );

  AE_ASSERT( width != 0 );
  AE_ASSERT( height != 0 );

  m_width = width;
  m_height = height;

  glGenFramebuffers( 1, &m_fbo );
  AE_ASSERT( m_fbo );
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );

  AE_CHECK_GL_ERROR();
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
  AE_CHECK_GL_ERROR();

  // @TODO: Figure out if there are any implicit SRGB conversions happening here. Improve interface and visibility to user if there are.
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

  AE_CHECK_GL_ERROR();
}

void aeRenderTarget::Destroy()
{
  m_shader.Destroy();
  m_quad.Destroy();

  for ( uint32_t i = 0; i < m_targets.Length(); i++ )
  {
    m_targets[ i ]->Destroy();
    aeAlloc::Release( m_targets[ i ] );
  }
  m_targets.Clear();

  m_depth.Destroy();

  if ( m_fbo )
  {
    glDeleteFramebuffers( 1, &m_fbo );
    m_fbo = 0;
  }

  m_width = 0;
  m_height = 0;
}

void aeRenderTarget::AddTexture( aeTextureFilter::Type filter, aeTextureWrap::Type wrap )
{
  AE_ASSERT( m_targets.Length() < kMaxFrameBufferAttachments );

  aeTexture2D* tex = aeAlloc::Allocate< aeTexture2D >();
  tex->Initialize( nullptr, m_width, m_height, aeTextureFormat::RGBA, aeTextureType::HalfFloat, filter, wrap );

  GLenum attachement = GL_COLOR_ATTACHMENT0 + m_targets.Length();
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  glFramebufferTexture2D( GL_FRAMEBUFFER, attachement, tex->GetTarget(), tex->GetTexture(), 0 );

  m_targets.Append( tex );

  AE_CHECK_GL_ERROR();
}

void aeRenderTarget::AddDepth( aeTextureFilter::Type filter, aeTextureWrap::Type wrap )
{
  AE_ASSERT_MSG( m_depth.GetTexture() == 0, "Render target already has a depth texture" );

  m_depth.Initialize( nullptr, m_width, m_height, aeTextureFormat::Depth, aeTextureType::Float, filter, wrap );
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth.GetTarget(), m_depth.GetTexture(), 0 );

  AE_CHECK_GL_ERROR();
}

void aeRenderTarget::Activate()
{
  CheckFramebufferComplete( m_fbo );
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_fbo );
  
  GLenum buffers[] =
  {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3,
    GL_COLOR_ATTACHMENT4,
    GL_COLOR_ATTACHMENT5,
    GL_COLOR_ATTACHMENT6,
    GL_COLOR_ATTACHMENT7,
    GL_COLOR_ATTACHMENT8,
    GL_COLOR_ATTACHMENT9,
    GL_COLOR_ATTACHMENT10,
    GL_COLOR_ATTACHMENT11,
    GL_COLOR_ATTACHMENT12,
    GL_COLOR_ATTACHMENT13,
    GL_COLOR_ATTACHMENT14,
    GL_COLOR_ATTACHMENT15
  };
  AE_STATIC_ASSERT( countof( buffers ) == kMaxFrameBufferAttachments );
  glDrawBuffers( m_targets.Length(), buffers );

  glViewport( 0, 0, GetWidth(), GetHeight() );
}

void aeRenderTarget::Clear( aeColor color )
{
  Activate();

  AE_CHECK_GL_ERROR();

  aeFloat3 clearColor = color.GetSRGB(); // Unclear why glClearColor() expects srgb. Maybe because of framebuffer type.
  glClearColor( clearColor.x, clearColor.y, clearColor.z, 1.0f );
  glClearDepth( gReverseZ ? 0.0f : 1.0f );

  glDepthMask( GL_TRUE );
  glDisable( GL_DEPTH_TEST );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  AE_CHECK_GL_ERROR();
}

void aeRenderTarget::Render( const aeShader* shader, const aeUniformList& uniforms )
{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
  m_quad.Render( shader, uniforms );
}

void aeRenderTarget::Render2D( uint32_t textureIndex, aeRect ndc, float z )
{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );

  aeUniformList uniforms;
  uniforms.Set( "u_localToNdc", aeRenderTarget::GetQuadToNDCTransform( ndc, z ) );
  uniforms.Set( "u_tex", GetTexture( textureIndex ) );
  m_quad.Render( &m_shader, uniforms );
}

const aeTexture2D* aeRenderTarget::GetTexture( uint32_t index ) const
{
  return m_targets[ index ];
}

const aeTexture2D* aeRenderTarget::GetDepth() const
{
  return m_depth.GetTexture() ? &m_depth : nullptr;
}

uint32_t aeRenderTarget::GetWidth() const
{
  return m_width;
}

uint32_t aeRenderTarget::GetHeight() const
{
  return m_height;
}

aeFloat4x4 aeRenderTarget::GetQuadToNDCTransform( aeRect ndc, float z )
{
  aeFloat4x4 localToNdc = aeFloat4x4::Translation( aeFloat3( ndc.x, ndc.y, z ) );
  localToNdc.Scale( aeFloat3( ndc.w, ndc.h, 1.0f ) );
  localToNdc.Translate( aeFloat3( 0.5f, 0.5f, 0.0f ) );
  return localToNdc;
}

//------------------------------------------------------------------------------
// aeOpenGLRender member functions
//------------------------------------------------------------------------------
aeOpenGLRender::aeOpenGLRender()
{
  m_context = nullptr;
  m_defaultFbo = 0;
}

void aeOpenGLRender::Initialize( aeRender* render )
{
#if _AE_IOS_
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#else
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
#endif
	
#if WRITE_TO_SRGB
	SDL_GL_SetAttribute( SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1 );
#endif
	
#if AE_GL_DEBUG_MODE
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );
#endif
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );

  m_context = SDL_GL_CreateContext( (SDL_Window*)render->GetWindow()->window );
  AE_ASSERT( m_context );
  SDL_GL_MakeCurrent( (SDL_Window*)render->GetWindow()->window, m_context );

  SDL_GL_SetSwapInterval( 1 );

#if _AE_WINDOWS_
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  glGetError(); // Glew currently has an issue which causes a GL_INVALID_ENUM on init
  AE_ASSERT_MSG( err == GLEW_OK, "Could not initialize glew" );
#endif

#if AE_GL_DEBUG_MODE
  glDebugMessageCallback( aeOpenGLDebugCallback, nullptr );
#endif

  glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFbo );

  AE_CHECK_GL_ERROR();
}

void aeOpenGLRender::Terminate( aeRender* render )
{
  SDL_GL_DeleteContext( m_context );
}

void aeOpenGLRender::StartFrame( aeRender* render )
{}

void aeOpenGLRender::EndFrame( aeRender* render )
{
  AE_CHECK_GL_ERROR();

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_defaultFbo );
  glViewport( 0, 0, render->GetWindow()->GetWidth(), render->GetWindow()->GetHeight() );

  // Clear window target in case canvas doesn't fit exactly
  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glClearDepth( 1.0f );

  glDepthMask( GL_TRUE );


  glDisable( GL_DEPTH_TEST );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  AE_CHECK_GL_ERROR();

#if WRITE_TO_SRGB
  glEnable(	GL_FRAMEBUFFER_SRGB );
#endif

  render->GetCanvas()->Render2D( 0, render->GetNDCRect(), 0.5f );

#if WRITE_TO_SRGB
  glDisable( GL_FRAMEBUFFER_SRGB );
#endif
	
#if !_AE_EMSCRIPTEN_
  SDL_GL_SwapWindow( (SDL_Window*)render->GetWindow()->window );
#endif

  AE_CHECK_GL_ERROR();
}
