//------------------------------------------------------------------------------
// aeVulkanRender.cpp
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

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
bool memory_type_from_properties( const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex )
{
    // Search memtypes to find first index with those properties
    for ( uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++ )
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if (( memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
            {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

/*
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
  
  // glGenVertexArrays( 1, &m_array );
  // glBindVertexArray( m_array );
}

void aeVertexData::Destroy()
{
  if ( m_vertexReadable ) { aeAlloc::Release( (uint8_t*)m_vertexReadable ); }
  if ( m_indexReadable ) { aeAlloc::Release( (uint8_t*)m_indexReadable ); }
  
  // glDeleteVertexArrays( 1, &m_array );
  // if ( m_vertices != ~0 ) { glDeleteBuffers( 1, &m_vertices ); }
  // if ( m_indices != ~0 ) { glDeleteBuffers( 1, &m_indices ); }
  
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
  // attribute->type = aeVertexDataTypeToGL( type );
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

    // glGenBuffers( 1, &m_vertices );
    // glBindVertexArray( m_array );
    // glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
    // glBufferData( GL_ARRAY_BUFFER, count * m_vertexSize, vertices, GL_STATIC_DRAW );
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
      // glGenBuffers( 1, &m_vertices );
      // glBindVertexArray( m_array );
      // glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
      // glBufferData( GL_ARRAY_BUFFER, m_vertexSize * m_maxVertexCount, nullptr, GL_DYNAMIC_DRAW );
    }
    
    // glBindVertexArray( m_array );
    // glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
    // glBufferSubData( GL_ARRAY_BUFFER, 0, count * m_vertexSize, vertices );
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

    // glGenBuffers( 1, &m_indices );
    // glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    // glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indexCount * m_indexSize, indices, GL_STATIC_DRAW );
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
      // glGenBuffers( 1, &m_indices );
      // glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
      // glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indexSize * m_maxIndexCount, nullptr, GL_DYNAMIC_DRAW );
    }
    
    // glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
    // glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, m_indexCount * m_indexSize, indices );
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
  // if ( m_vertices == ~0 || !m_vertexCount || ( m_indices != ~0 && !m_indexCount ) )
  // {
  //   return;
  // }

  // shader->Activate( uniforms );

  // glBindVertexArray( m_array );
  // AE_ASSERT( glGetError() == GL_NO_ERROR );

  // glBindBuffer( GL_ARRAY_BUFFER, m_vertices );
  // AE_ASSERT( glGetError() == GL_NO_ERROR );

  // if ( m_indexCount && m_primitive != aeVertexPrimitive::Point )
  // {
  //   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
  //   AE_ASSERT( glGetError() == GL_NO_ERROR );
  // }

  // for ( uint32_t i = 0; i < shader->GetAttributeCount(); i++ )
  // {
  //   const aeShaderAttribute* shaderAttribute = shader->GetAttributeByIndex( i );
  //   const aeVertexAttribute* vertexAttribute = m_GetAttributeByName( shaderAttribute->name );

  //   AE_ASSERT_MSG( vertexAttribute, "No vertex attribute named '#'", shaderAttribute->name );
  //   // @TODO: Verify attribute type and size match

  //   GLint location = shaderAttribute->location;
  //   AE_ASSERT( location != -1 );
  //   glEnableVertexAttribArray( location );
  //   AE_ASSERT( glGetError() == GL_NO_ERROR );

  //   uint32_t componentCount = vertexAttribute->componentCount;
  //   uint64_t attribOffset = vertexAttribute->offset;
  //   glVertexAttribPointer( location, componentCount, vertexAttribute->type, GL_FALSE, m_vertexSize, (void*)attribOffset );
  //   AE_ASSERT( glGetError() == GL_NO_ERROR );
  // }

  // int64_t start = 0; // TODO: Add support to start drawing at non-zero index
  // int32_t count = 0;

  // // Draw
  // GLenum mode;
  // if( m_primitive == aeVertexPrimitive::Triangle )
  // {
  //   count = primitiveCount ? primitiveCount * 3 : m_indexCount;
  //   mode = GL_TRIANGLES;
  // }
  // else if( m_primitive == aeVertexPrimitive::Line )
  // {
  //   count = primitiveCount ? primitiveCount * 2 : m_indexCount;
  //   mode = GL_LINES;
  // }
  // else if( m_primitive == aeVertexPrimitive::Point )
  // {
  //   count = primitiveCount ? primitiveCount : m_indexCount;
  //   mode = GL_POINTS;
  // }
  // else
  // {
  //   AE_FAIL();
  //   return;
  // }
  
  // if ( m_indexCount && mode != GL_POINTS )
  // {
  //   if ( count == 0 ) { count = m_indexCount; }
  //   AE_ASSERT( start + count <= m_indexCount );
  //   if ( mode == GL_TRIANGLES ) { AE_ASSERT( count % 3 == 0 && start % 3 == 0 ); }
    
  //   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices );
  //   AE_ASSERT( glGetError() == GL_NO_ERROR );
  //   GLenum type = 0;
  //   if ( m_indexSize == sizeof(uint8_t) ) { type = GL_UNSIGNED_BYTE; }
  //   else if ( m_indexSize == sizeof(uint16_t) ) { type = GL_UNSIGNED_SHORT; }
  //   else if ( m_indexSize == sizeof(uint32_t) ) { type = GL_UNSIGNED_INT; }
  //   glDrawElements( mode, count, type, (void*)start );
  //   AE_ASSERT( glGetError() == GL_NO_ERROR );
  // }
  // else
  // {
  //   if ( count == 0 ) { count = m_vertexCount; }
  //   AE_ASSERT( start + count <= m_vertexCount );
  //   if ( mode == GL_TRIANGLES ) { AE_ASSERT( count % 3 == 0 && start % 3 == 0 ); }
    
  //   glDrawArrays( mode, start, count );
  //   AE_ASSERT( glGetError() == GL_NO_ERROR );
  // }
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

  m_attributeCount = 0;
}

aeShader::~aeShader()
{
  Destroy();
}

void aeShader::Initialize( const char* vertexStr, const char* fragStr, const char* const* defines, int32_t defineCount )
{
  AE_ASSERT( !m_program );

  // m_program = glCreateProgram();
  
  m_vertexShader = m_LoadShader( vertexStr, aeShaderType::Vertex, defines, defineCount );
  m_fragmentShader = m_LoadShader( fragStr, aeShaderType::Fragment, defines, defineCount );
  
  if ( !m_vertexShader || !m_fragmentShader )
  {
    AE_LOG( "Failed to load shader!" );
    AE_FAIL();
  }
  
  // glAttachShader( m_program, m_vertexShader );
  // glAttachShader( m_program, m_fragmentShader );
  
  // glLinkProgram( m_program );
  
  // GLint status;
  // glGetProgramiv( m_program, GL_LINK_STATUS, &status );
  // if( status == GL_FALSE )
  // {
  //   GLint logLength;
  //   glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &logLength );
    
  //   if( logLength > 0 )
  //   {
  //     unsigned char *log = new unsigned char[ logLength ];
  //     glGetProgramInfoLog( m_program, logLength, NULL, (GLchar*)log );
  //     AE_LOG( "#", log );
  //     delete [] log;
  //   }
    
  //   Destroy();
  //   AE_FAIL();
  // }
  
  // GLint attribCount = 0;
  // glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTES, &attribCount );
  // AE_ASSERT( 0 < attribCount && attribCount <= kMaxShaderAttributeCount );
  // GLint maxLen = 0;
  // glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen );
  // AE_ASSERT( 0 < maxLen && maxLen <= kMaxShaderAttributeNameLength );
  // for ( int32_t i = 0; i < attribCount; i++ )
  // {
  //   AE_ASSERT( m_attributeCount < countof(m_attributes) );
  //   aeShaderAttribute* attribute = &m_attributes[ m_attributeCount ];
  //   m_attributeCount++;

  //   GLsizei length;
  //   GLint size;
  //   glGetActiveAttrib( m_program, i, kMaxShaderAttributeNameLength, &length, &size, (GLenum*)&attribute->type, (GLchar*)attribute->name );
    
  //   attribute->location = glGetAttribLocation( m_program, attribute->name );
  //   AE_ASSERT( attribute->location != -1 );
  // }
  
  // GLint uniformCount = 0;
  // maxLen = 0;
  // glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &uniformCount );
  // glGetProgramiv( m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );
  // AE_ASSERT( maxLen <= aeStr32::kMaxLength ); // @TODO: Read from aeShaderUniform

  // for( int32_t i = 0; i < uniformCount; i++ )
  // {
  //   aeShaderUniform uniform;

  //   GLint size = 0;
  //   char name[ aeStr32::kMaxLength ]; // @TODO: Read from aeShaderUniform
  //   glGetActiveUniform( m_program, i, sizeof(name), nullptr, &size, (GLenum*)&uniform.type, (GLchar*)name );
  //   AE_ASSERT( size == 1 );
    
  //   switch ( uniform.type )
  //   {
  //     case GL_SAMPLER_2D:
  //     case GL_SAMPLER_3D:
  //     case GL_FLOAT:
  //     case GL_FLOAT_VEC2:
  //     case GL_FLOAT_VEC3:
  //     case GL_FLOAT_VEC4:
  //     case GL_FLOAT_MAT4:
  //       break;
  //     default:
  //       AE_FAIL_MSG( "Unsupported uniform '#' type #", name, uniform.type );
  //       break;
  //   }

  //   uniform.name = name;
  //   uniform.location = glGetUniformLocation( m_program, name );
  //   AE_ASSERT( uniform.location != -1 );

  //   m_uniforms.Set( name, uniform );
  // }
}

void aeShader::Destroy()
{
  // if( m_fragmentShader != 0 )
  // {
  //   glDeleteShader( m_fragmentShader );
  //   m_fragmentShader = 0;
  // }
  
  // if( m_vertexShader != 0 )
  // {
  //   glDeleteShader( m_vertexShader );
  //   m_vertexShader = 0;
  // }
  
  // if( m_program != 0 )
  // {
  //   glDeleteProgram( m_program );
  //   m_program = 0;
  // }
}

void aeShader::Activate( const aeUniformList& uniforms ) const
{
//   glUseProgram( m_program );

//   // Set rendering params
//   if ( m_blending )
//   {
//     glEnable( GL_BLEND );
//     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//   }
//   else
//   {
//     glDisable( GL_BLEND );
//   }

//   if ( m_depthTest )
//   {
//     glEnable( GL_DEPTH_TEST );
//     glDepthFunc( GL_LEQUAL );
//   }
//   else
//   {
//     glDisable( GL_DEPTH_TEST );
//   }
  
//   glDepthMask( m_depthWrite ? GL_TRUE : GL_FALSE );

//   // Set shader uniforms
//   uint32_t textureIndex = 0;
//   for ( uint32_t i = 0; i < m_uniforms.Length(); i++ )
//   {
//     const char* uniformVarName = m_uniforms.GetKey( i ).c_str();
//     const aeShaderUniform* uniformVar = &m_uniforms.GetValue( i );
//     const aeUniformList::Value* uniformValue = uniforms.Get( uniformVarName );
    
//     // Start validation
//     AE_ASSERT_MSG( uniformValue, "Shader uniform '#' value is not set", uniformVarName );
//     uint32_t typeSize = 0;
//     switch ( uniformVar->type )
//     {
//       case GL_SAMPLER_2D:
//         typeSize = 0;
//         break;
//       case GL_SAMPLER_3D:
//         typeSize = 0;
//         break;
//       case GL_FLOAT:
//         typeSize = 1;
//         break;
//       case GL_FLOAT_VEC2:
//         typeSize = 2;
//         break;
//       case GL_FLOAT_VEC3:
//         typeSize = 3;
//         break;
//       case GL_FLOAT_VEC4:
//         typeSize = 4;
//         break;
//       case GL_FLOAT_MAT4:
//         typeSize = 16;
//         break;
//       default:
//         AE_FAIL_MSG( "Unsupported uniform '#' type #", uniformVarName, uniformVar->type );
//         break;
//     }
//     AE_ASSERT_MSG( uniformValue->size == typeSize, "Uniform type mismatch '#' type:# size:#", uniformVarName, uniformVar->type, uniformValue->size );
//     // End validation

//     if ( uniformVar->type == GL_SAMPLER_2D )
//     {
//       AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid #", uniformVarName, uniformValue->sampler );
//       glActiveTexture( GL_TEXTURE0 + textureIndex );
//       glBindTexture( GL_TEXTURE_2D, uniformValue->sampler );
//       glUniform1i( uniformVar->location, textureIndex );
//       textureIndex++;
//     }
//     else if ( uniformVar->type == GL_SAMPLER_3D )
//     {
//       AE_ASSERT_MSG( uniformValue->sampler, "Uniform sampler 2d '#' value is invalid #", uniformVarName, uniformValue->sampler );
//       glActiveTexture( GL_TEXTURE0 + textureIndex );
//       glBindTexture( GL_TEXTURE_3D, uniformValue->sampler );
//       glUniform1i( uniformVar->location, textureIndex );
//       textureIndex++;
//     }
//     else if ( uniformVar->type == GL_FLOAT )
//     {
//       glUniform1fv( uniformVar->location, 1, uniformValue->value.data );
//     }
//     else if ( uniformVar->type == GL_FLOAT_VEC2 )
//     {
//       glUniform2fv( uniformVar->location, 1, uniformValue->value.data );
//     }
//     else if ( uniformVar->type == GL_FLOAT_VEC3 )
//     {
//       glUniform3fv( uniformVar->location, 1, uniformValue->value.data );
//     }
//     else if ( uniformVar->type == GL_FLOAT_VEC4 )
//     {
//       glUniform4fv( uniformVar->location, 1, uniformValue->value.data );
//     }
//     else if ( uniformVar->type == GL_FLOAT_MAT4 )
//     {
// #if _AE_EMSCRIPTEN_
//       // WebGL/Emscripten doesn't support glUniformMatrix4fv auto-transpose
//       aeFloat4x4 transposedTransform = uniformValue->value.GetTransposeCopy();
//       glUniformMatrix4fv( uniformVar->location, 1, GL_FALSE, transposedTransform.data );
// #else
//       glUniformMatrix4fv( uniformVar->location, 1, GL_TRUE, uniformValue->value.data );
// #endif
//     }
//     else
//     {
//       AE_ASSERT_MSG( false, "Invalid uniform type '#': #", uniformVarName, uniformVar->type );
//     }
//   }

//   AE_ASSERT( glGetError() == GL_NO_ERROR );
}

const aeShaderAttribute* aeShader::GetAttributeByIndex( uint32_t index ) const
{
  AE_ASSERT( index < m_attributeCount );
  return &m_attributes[ index ];
}

int aeShader::m_LoadShader( const char* shaderStr, aeShaderType::Type type, const char* const* defines, int32_t defineCount )
{
	return 0;

//   GLenum glType = -1;
//   if ( type == aeShaderType::Vertex )
//   {
//     glType = GL_VERTEX_SHADER;
//   }
//   if ( type == aeShaderType::Fragment )
//   {
//     glType = GL_FRAGMENT_SHADER;
//   }
  
//   const uint32_t kPrependMax = 16;
//   uint32_t sourceCount = 0;
//   const char* shaderSource[ kPrependMax + kMaxShaderDefines * 2 + 1 ]; // x2 max defines to make room for newlines. Plus one for actual shader.

//   // Version
// #if _AE_IOS_
//   // shaderSource[ 0 ] = "#version 300 es\n";
// #elif _AE_EMSCRIPTEN_
//   // No version specified
// #else
//   shaderSource[ sourceCount++ ] = "#version 330 core\n";
// #endif

//   // Utility
//   shaderSource[ sourceCount++ ] = "float AE_SRGB_TO_RGB( float _x ) { return pow( _x, 2.2 ); }\n";
//   shaderSource[ sourceCount++ ] = "float AE_RGB_TO_SRGB( float _x ) { return pow( _x, 1.0 / 2.2 ); }\n";
//   shaderSource[ sourceCount++ ] = "vec3 AE_SRGB_TO_RGB( vec3 _x ) { return vec3( AE_SRGB_TO_RGB( _x.r ), AE_SRGB_TO_RGB( _x.g ), AE_SRGB_TO_RGB( _x.b ) ); }\n";
//   shaderSource[ sourceCount++ ] = "vec3 AE_RGB_TO_SRGB( vec3 _x ) { return vec3( AE_RGB_TO_SRGB( _x.r ), AE_RGB_TO_SRGB( _x.g ), AE_RGB_TO_SRGB( _x.b ) ); }\n";
//   shaderSource[ sourceCount++ ] = "vec4 AE_SRGBA_TO_RGBA( vec4 _x ) { return vec4( AE_SRGB_TO_RGB( _x.rgb ), _x.a ); }\n";
//   shaderSource[ sourceCount++ ] = "vec4 AE_RGBA_TO_SRGBA( vec4 _x ) { return vec4( AE_RGB_TO_SRGB( _x.rgb ), _x.a ); }\n";

//   // Input/output
// #if _AE_IOS_
//   // shaderSource[ 1 ] = "precision highp float;\n";
//   // shaderSource[ 2 ] = "precision mediump sampler3D;\n";
// #elif _AE_EMSCRIPTEN_
//   shaderSource[ sourceCount++ ] = "precision highp float;\n";
//   shaderSource[ sourceCount++ ] = "#define AE_COLOR gl_FragColor\n";
//   shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture2d\n";
//   shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
//   // shaderSource[ sourceCount++ ] = "#define AE_FLOAT_HIGHP highp float\n";
//   if ( type == aeShaderType::Vertex )
//   {
//     shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP attribute highp\n";
//     shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP varying highp\n";
//   }
//   else if ( type == aeShaderType::Fragment )
//   {
//     shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP varying highp\n";
//     shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform highp\n";
//   }
// #else
//   shaderSource[ sourceCount++ ] = "#define AE_TEXTURE2D texture\n";
//   shaderSource[ sourceCount++ ] = "#define AE_UNIFORM_HIGHP uniform\n";
//   shaderSource[ sourceCount++ ] = "#define AE_IN_HIGHP in\n";
//   shaderSource[ sourceCount++ ] = "#define AE_OUT_HIGHP out\n";
//   if ( type == aeShaderType::Fragment )
//   {
//     shaderSource[ sourceCount++ ] = "out vec4 AE_COLOR;\n";
//   }
// #endif
//   AE_ASSERT( sourceCount <= kPrependMax );

//   for ( int32_t i = 0; i < defineCount; i++ )
//   {
//     shaderSource[ sourceCount ] = defines[ i ];
//     sourceCount++;
//     shaderSource[ sourceCount ] = "\n";
//     sourceCount++;
//   }

//   shaderSource[ sourceCount ] = shaderStr;
//   sourceCount++;
  
//   GLuint shader = glCreateShader( glType );
//   glShaderSource( shader, sourceCount, shaderSource, nullptr );
//   glCompileShader( shader );
  
//   GLint status;
//   glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
//   if(status == GL_FALSE)
//   {
//     GLint logLength;
//     glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    
//     if (logLength > 0)
//     {
//       unsigned char* log = new unsigned char[ logLength ];
//       glGetShaderInfoLog(shader, logLength, NULL, (GLchar*)log);
//       AE_LOG( "Error compiling shader #", log );
//       delete[] log;
//     }
    
//     return 0;
//   }
  
//   return shader;
}

//------------------------------------------------------------------------------
// aeTexture2D member functions
//------------------------------------------------------------------------------
aeTexture2D::aeTexture2D()
{
  m_width = 0;
  m_height = 0;
  m_hasAlpha = false;
}

void aeTexture2D::Initialize( const uint8_t* data, uint32_t width, uint32_t height, uint32_t depth, aeTextureFilter::Type filter, aeTextureWrap::Type wrap )
{
  // m_width = width;
  // m_height = height;

  // glGenTextures( 1, &m_texture );
  // glBindTexture( GL_TEXTURE_2D, m_texture );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  
  // GLint internalFormat = 0;
  // GLenum format = 0;
  // GLint unpackAlignment;
  // switch ( depth )
  // {
  //   case 1:
  //     internalFormat = GL_RED;
  //     format = GL_RED;
  //     unpackAlignment = 1;
  //     m_hasAlpha = false;
  //     break;
  //   case 3:
  //     internalFormat = GL_RGB;
  //     format = GL_RGB;
  //     unpackAlignment = 1;
  //     m_hasAlpha = false;
  //     break;
  //   case 4:
  //     internalFormat = GL_RGBA;
  //     format = GL_RGBA;
  //     unpackAlignment = 4;
  //     m_hasAlpha = true;
  //     break;
  //   default:
  //     AE_ASSERT_MSG( false, "Invalid texture depth #", depth );
  //     return;
  // }

  // glPixelStorei( GL_UNPACK_ALIGNMENT, unpackAlignment );
  // glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data );
}

void aeTexture2D::Initialize( const char* file, aeTextureFilter::Type filter, aeTextureWrap::Type wrap )
{
  // uint32_t fileSize = aeVfs::GetSize( file );
  // AE_ASSERT_MSG( fileSize, "Could not load #", file );
  
  // uint8_t* fileBuffer = (uint8_t*)malloc( fileSize );
  // aeVfs::Read( file, fileBuffer, fileSize );

  // int32_t width = 0;
  // int32_t height = 0;
  // int32_t channels = 0;
  // stbi_set_flip_vertically_on_load( true );
  // uint8_t* image = stbi_load_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  // AE_ASSERT( image );

  // uint32_t depth = 0;
  // switch ( channels )
  // {
  //   case STBI_grey:
  //     depth = 1;
  //     break;
  //   case STBI_grey_alpha:
  //     AE_FAIL();
  //     break;
  //   case STBI_rgb:
  //     depth = 3;
  //     break;
  //   case STBI_rgb_alpha:
  //     depth = 4;
  //     break;
  // }
  
  // Initialize( image, width, height, depth, filter, wrap );
  
  // stbi_image_free( image );
  // free( fileBuffer );
}

void aeTexture2D::Destroy()
{
  // glDeleteTextures( 1, &m_texture );
  // *this = aeTexture2D();
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
  // AE_ASSERT( m_fbo == 0 );
  // AE_ASSERT( m_texture == 0 );

  // AE_ASSERT( width != 0 );
  // AE_ASSERT( height != 0 );

  // m_width = width;
  // m_height = height;

  // glGenFramebuffers( 1, &m_fbo );
  // AE_ASSERT( m_fbo );
  // glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );

  // glGenTextures( 1, &m_texture );
  // AE_ASSERT( m_texture );
  // glBindTexture( GL_TEXTURE_2D, m_texture );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( filter == aeTextureFilter::Nearest ) ? GL_NEAREST : GL_LINEAR );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ( wrap == aeTextureWrap::Clamp ) ? GL_CLAMP_TO_EDGE : GL_REPEAT );
  // glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr );
  // glBindTexture( GL_TEXTURE_2D, 0 );
  // glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0 );

  // Vertex quadVerts[] =
  // {
  //   { aeQuadVertPos[ 0 ], aeQuadVertUvs[ 0 ] },
  //   { aeQuadVertPos[ 1 ], aeQuadVertUvs[ 1 ] },
  //   { aeQuadVertPos[ 2 ], aeQuadVertUvs[ 2 ] },
  //   { aeQuadVertPos[ 3 ], aeQuadVertUvs[ 3 ] }
  // };
  // AE_STATIC_ASSERT( countof( quadVerts ) == aeQuadVertCount );
  // m_quad.Initialize( sizeof( Vertex ), sizeof( aeQuadIndex ), aeQuadVertCount, aeQuadIndexCount, aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
  // m_quad.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof( Vertex, pos ) );
  // m_quad.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof( Vertex, uv ) );
  // m_quad.SetVertices( quadVerts, aeQuadVertCount );
  // m_quad.SetIndices( aeQuadIndices, aeQuadIndexCount );

  // // @TODO: Figure out if there are any implicit SRGB conversions happening here. Improve interface and visibility to user if there are.
  // const char* vertexStr = "\
  //   AE_UNIFORM_HIGHP mat4 u_localToNdc;\
  //   AE_IN_HIGHP vec3 a_position;\
  //   AE_IN_HIGHP vec2 a_uv;\
  //   AE_OUT_HIGHP vec2 v_uv;\
  //   void main()\
  //   {\
  //     v_uv = a_uv;\
  //     gl_Position = u_localToNdc * vec4( a_position, 1.0 );\
  //   }";
  // const char* fragStr = "\
  //   uniform sampler2D u_tex;\
  //   AE_IN_HIGHP vec2 v_uv;\
  //   void main()\
  //   {\
  //     AE_COLOR = AE_TEXTURE2D( u_tex, v_uv );\
  //   }";
  // m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );

  // AE_ASSERT( glGetError() == GL_NO_ERROR );
}

void aeRenderTexture::Destroy()
{
  // m_shader.Destroy();
  // m_quad.Destroy();

  // if ( m_texture )
  // {
  //   glDeleteTextures( 1, &m_texture );
  //   m_texture = 0;
  // }

  // if ( m_fbo )
  // {
  //   glDeleteFramebuffers( 1, &m_fbo );
  //   m_fbo = 0;
  // }

  // m_width = 0;
  // m_height = 0;
}

void aeRenderTexture::Activate()
{
  // glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  // glViewport( 0, 0, GetWidth(), GetHeight() );
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
*/

#define NUM_DESCRIPTOR_SETS 1
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

//------------------------------------------------------------------------------
// aeVulkanRender member functions
//------------------------------------------------------------------------------
aeVulkanRender::aeVulkanRender()
{}

VkBool32 aeRender_DebugCallback(
  VkDebugReportFlagsEXT flags,
  VkDebugReportObjectTypeEXT objectType,
  uint64_t object,
  size_t location,
  int32_t messageCode,
  const char* pLayerPrefix,
  const char* pMessage,
  void* pUserData )
{
  const char* flagStr = "Unknown";
  bool ignore = false;
  bool criticalIssue = false;
  switch ( flags )
  {
    case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
      flagStr = "Info";
      ignore = true;
      break;
    case VK_DEBUG_REPORT_WARNING_BIT_EXT:
      flagStr = "Warning";
      break;
    case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
      flagStr = "Performance Warning";
      break;
    case VK_DEBUG_REPORT_ERROR_BIT_EXT:
      flagStr = "Error";
      criticalIssue = true;
      break;
    case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
      flagStr = "Debug";
      break;
    default:
      AE_FAIL_MSG( "Vulkan debug callback result not handled." );
      break;
  }
  if ( ignore )
  {
    // Do nothing
  }
  else if ( criticalIssue )
  {
    AE_FAIL_MSG( "# (#) : #", pLayerPrefix, flagStr, pMessage );
  }
  else
  {
    AE_LOG( "# (#) : #", pLayerPrefix, flagStr, pMessage );
  }
  return VK_FALSE;
}

void aeVulkanRender::Initialize( aeRender* render )
{
  VkResult res = VK_SUCCESS;

  // Layers
  uint32_t layerCount = 0;
  res = vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
  AE_ASSERT( res == VK_SUCCESS );
  aeArray< VkLayerProperties > availableLayers( layerCount, VkLayerProperties() );
  if ( layerCount )
  {
    res = vkEnumerateInstanceLayerProperties( &layerCount, &availableLayers[ 0 ] );
    AE_ASSERT( res == VK_SUCCESS );

    for ( uint32_t i = 0; i < availableLayers.Length(); i++ )
    {
      AE_LOG( availableLayers[ i ].layerName );
    }
  }
  aeArray< const char* > validationLayers;
  validationLayers.Append( "VK_LAYER_LUNARG_standard_validation" );
  for ( uint32_t i = 0; i < validationLayers.Length(); i++ )
  {
    const char* instanceLayer = validationLayers[ i ];
    auto findFn = [ instanceLayer ]( const VkLayerProperties& props )
    {
      return strcmp( instanceLayer, props.layerName ) == 0;
    };
    AE_ASSERT_MSG( availableLayers.FindFn( findFn ) >= 0, "Vulkan layer '#' not supported.", instanceLayer );
  }

  // Create instance
  aeArray< const char* > instanceExtensions;
  instanceExtensions.Append( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
  instanceExtensions.Append( VK_KHR_SURFACE_EXTENSION_NAME );
#if _AE_WINDOWS_
  instanceExtensions.Append( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#elif defined( VK_USE_PLATFORM_METAL_EXT ) // @TODO: _AE_APPLE_
  instanceExtensions.Append( VK_EXT_METAL_SURFACE_EXTENSION_NAME );
#else
  instanceExtensions.Append( VK_KHR_XCB_SURFACE_EXTENSION_NAME );
#endif

  VkApplicationInfo appInfo;
  memset( &appInfo, 0, sizeof( appInfo ) );
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "aeLib"; // @HACK
  appInfo.applicationVersion = 1;
  appInfo.pEngineName = "aeLib"; // @HACK
  appInfo.engineVersion = 1;
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instInfo;
  memset( &instInfo, 0, sizeof( instInfo ) );
  instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instInfo.pApplicationInfo = &appInfo;
  instInfo.enabledLayerCount = validationLayers.Length();
  instInfo.ppEnabledLayerNames = validationLayers.Length() ? &validationLayers[ 0 ] : nullptr;
  instInfo.enabledExtensionCount = instanceExtensions.Length();
  instInfo.ppEnabledExtensionNames = instanceExtensions.Length() ? &instanceExtensions[ 0 ] : nullptr;
  res = vkCreateInstance( &instInfo, nullptr, &m_inst );
  if ( res == VK_ERROR_INCOMPATIBLE_DRIVER )
  {
    AE_FAIL_MSG( "Cannot find a compatible Vulkan ICD" );
  }
  else if ( res != VK_SUCCESS )
  {
    AE_FAIL_MSG( "Unknown Vulkan error" );
  }

  // Debug callbacks
  VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
  memset( &debugCallbackCreateInfo, 0, sizeof( debugCallbackCreateInfo ) );
  debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  debugCallbackCreateInfo.flags
    = VK_DEBUG_REPORT_INFORMATION_BIT_EXT
    | VK_DEBUG_REPORT_WARNING_BIT_EXT
    | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
    | VK_DEBUG_REPORT_ERROR_BIT_EXT
    | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
  debugCallbackCreateInfo.pfnCallback = aeRender_DebugCallback;
  PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr( m_inst, "vkCreateDebugReportCallbackEXT" );
  AE_ASSERT( createDebugReportCallback );
  res = createDebugReportCallback( m_inst, &debugCallbackCreateInfo, nullptr, &m_debugCallback );
  AE_ASSERT( res == VK_SUCCESS );

  // Create surface
  SDL_SysWMinfo sdlWindowInfo;
  SDL_VERSION( &sdlWindowInfo.version );
  if ( !SDL_GetWindowWMInfo( (SDL_Window*)render->GetWindow()->window, &sdlWindowInfo ) )
  {
    AE_FAIL_MSG( "Could not get system window properties." );
  }

#if _AE_WINDOWS_
  VkWin32SurfaceCreateInfoKHR surfaceInfo;
  memset( &surfaceInfo, 0, sizeof( surfaceInfo ) );
  surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surfaceInfo.hinstance = sdlWindowInfo.info.win.hinstance;
  surfaceInfo.hwnd = sdlWindowInfo.info.win.window;
  res = vkCreateWin32SurfaceKHR( m_inst, &surfaceInfo, nullptr, &m_surface );
#else
  VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
  surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  surfaceInfo.pNext = nullptr;
  surfaceInfo.connection = info.connection;
  surfaceInfo.window = info.window;
  res = vkCreateXcbSurfaceKHR( info.inst, &surfaceInfo, nullptr, &info.surface );
#endif
  AE_ASSERT( res == VK_SUCCESS );

  // Get physical devices
  uint32_t gpuCount = 0;
  res = vkEnumeratePhysicalDevices( m_inst, &gpuCount, nullptr );
  AE_ASSERT( res == VK_SUCCESS );
  AE_ASSERT( gpuCount );

  aeArray< VkPhysicalDevice > gpus( gpuCount, VkPhysicalDevice() );
  res = vkEnumeratePhysicalDevices( m_inst, &gpuCount, &gpus[ 0 ] );
  AE_ASSERT( res == VK_SUCCESS );
  AE_ASSERT( gpuCount );
  VkPhysicalDevice selectedGpu = gpus[ 0 ];

  // Get queue family info
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties( selectedGpu, &queueFamilyCount, NULL );
  AE_ASSERT( queueFamilyCount >= 1 );
  aeArray< VkQueueFamilyProperties > queueProps( queueFamilyCount, VkQueueFamilyProperties() );
  vkGetPhysicalDeviceQueueFamilyProperties( selectedGpu, &queueFamilyCount, &queueProps[ 0 ] );
  AE_ASSERT( queueFamilyCount >= 1 );

  // Iterate over each queue to learn whether it supports presenting:
  aeArray< VkBool32 > supportsPresent( queueFamilyCount, 0 );
  for ( uint32_t i = 0; i < queueFamilyCount; i++ )
  {
    vkGetPhysicalDeviceSurfaceSupportKHR( selectedGpu, i, m_surface, &supportsPresent[ i ] );
  }

  uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
  uint32_t presentQueueFamilyIndex = UINT32_MAX;
  // Find queue that supports both graphics and present
  for ( uint32_t i = 0; i < queueFamilyCount; i++ )
  {
    if ( ( queueProps[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) != 0 && supportsPresent[ i ] == VK_TRUE )
    {
      graphicsQueueFamilyIndex = i;
      presentQueueFamilyIndex = i;
      break;
    }
  }
  // Find separate queues if none support both
  if ( graphicsQueueFamilyIndex == UINT32_MAX || presentQueueFamilyIndex == UINT32_MAX )
  {
    for ( uint32_t i = 0; i < queueFamilyCount; i++ )
    {
      if ( ( queueProps[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) != 0 )
      {
        graphicsQueueFamilyIndex = i;
        break;
      }
    }

    for ( uint32_t i = 0; i < queueFamilyCount; i++ )
    {
      if ( supportsPresent[ i ] == VK_TRUE )
      {
        presentQueueFamilyIndex = i;
        break;
      }
    }

    AE_ASSERT( graphicsQueueFamilyIndex != UINT32_MAX );
    AE_ASSERT( presentQueueFamilyIndex != UINT32_MAX );
  }

  // Create device
  aeArray< const char* > deviceExtensions;
  deviceExtensions.Append( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

  float queuePriorities[ 1 ] = { 0.0 };
  VkDeviceQueueCreateInfo queueInfo;
  memset( &queueInfo, 0, sizeof( queueInfo ) );
  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = queuePriorities;
  queueInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

  VkDeviceCreateInfo deviceInfo;
  memset( &deviceInfo, 0, sizeof( deviceInfo ) );
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.queueCreateInfoCount = 1;
  deviceInfo.pQueueCreateInfos = &queueInfo;
  deviceInfo.enabledLayerCount = validationLayers.Length();
  deviceInfo.ppEnabledLayerNames = validationLayers.Length() ? &validationLayers[ 0 ] : nullptr;
  deviceInfo.enabledExtensionCount = deviceExtensions.Length();
  deviceInfo.ppEnabledExtensionNames = deviceExtensions.Length() ? &deviceExtensions[ 0 ] : nullptr;
  res = vkCreateDevice( selectedGpu, &deviceInfo, nullptr, &m_device );
  AE_ASSERT( res == VK_SUCCESS );

  // Create command buffer
  VkCommandPoolCreateInfo cmdPoolInfo;
  memset( &cmdPoolInfo, 0, sizeof( cmdPoolInfo ) );
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
  res = vkCreateCommandPool( m_device, &cmdPoolInfo, nullptr, &m_cmdPool );
  AE_ASSERT( res == VK_SUCCESS );

  VkCommandBufferAllocateInfo cmdInfo;
  memset( &cmdInfo, 0, sizeof( cmdInfo ) );
  cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdInfo.commandPool = m_cmdPool;
  cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdInfo.commandBufferCount = 1;
  res = vkAllocateCommandBuffers( m_device, &cmdInfo, &m_cmd );
  AE_ASSERT( res == VK_SUCCESS );

  // Get the list of VkFormats that are supported:
  VkFormat format = VK_FORMAT_UNDEFINED;
  uint32_t formatCount = 0;
  res = vkGetPhysicalDeviceSurfaceFormatsKHR( gpus[ 0 ], m_surface, &formatCount, NULL );
  AE_ASSERT( res == VK_SUCCESS );
  AE_ASSERT( formatCount );
  aeArray< VkSurfaceFormatKHR > surfFormats( formatCount, VkSurfaceFormatKHR() );
  res = vkGetPhysicalDeviceSurfaceFormatsKHR( gpus[ 0 ], m_surface, &formatCount, &surfFormats[ 0 ] );
  AE_ASSERT( res == VK_SUCCESS );
  AE_ASSERT( formatCount );
  if ( formatCount == 1 && surfFormats[ 0 ].format == VK_FORMAT_UNDEFINED )
  {
    // No preferred format
    format = VK_FORMAT_B8G8R8A8_UNORM;
  }
  else
  {
    format = surfFormats[ 0 ].format;
  }

  // Begin command buffer
  VkCommandBufferBeginInfo cmdBufInfo;
  memset( &cmdBufInfo, 0, sizeof( cmdBufInfo ) );
  cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  res = vkBeginCommandBuffer( m_cmd, &cmdBufInfo );
  AE_ASSERT( res == VK_SUCCESS );

  // !!!! init device queue
  vkGetDeviceQueue( m_device, graphicsQueueFamilyIndex, 0, &m_graphicsQueue );
  if ( graphicsQueueFamilyIndex == presentQueueFamilyIndex )
  {
    m_presentQueue = m_graphicsQueue;
  }
  else
  {
    vkGetDeviceQueue( m_device, presentQueueFamilyIndex, 0, &m_presentQueue );
  }

  // !!!! init swap chain
  VkSurfaceCapabilitiesKHR surfCapabilities;
  res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( selectedGpu, m_surface, &surfCapabilities );
  AE_ASSERT( res == VK_SUCCESS );

  uint32_t presentModeCount = 0;
  res = vkGetPhysicalDeviceSurfacePresentModesKHR( selectedGpu, m_surface, &presentModeCount, NULL );
  AE_ASSERT( res == VK_SUCCESS );
  AE_ASSERT( presentModeCount );
  aeArray< VkPresentModeKHR > presentModes( presentModeCount, VkPresentModeKHR() );
  res = vkGetPhysicalDeviceSurfacePresentModesKHR( selectedGpu, m_surface, &presentModeCount, &presentModes[ 0 ] );
  AE_ASSERT( res == VK_SUCCESS );

  VkExtent2D swapchainExtent;
  // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
  if ( surfCapabilities.currentExtent.width == 0xFFFFFFFF )
  {
    // If the surface size is undefined, the size is set to
    // the size of the images requested.
    swapchainExtent.width = render->GetWidth();
    swapchainExtent.height = render->GetHeight();
    if ( swapchainExtent.width < surfCapabilities.minImageExtent.width )
    {
      swapchainExtent.width = surfCapabilities.minImageExtent.width;
    }
    else if ( swapchainExtent.width > surfCapabilities.maxImageExtent.width )
    {
      swapchainExtent.width = surfCapabilities.maxImageExtent.width;
    }

    if ( swapchainExtent.height < surfCapabilities.minImageExtent.height )
    {
      swapchainExtent.height = surfCapabilities.minImageExtent.height;
    }
    else if ( swapchainExtent.height > surfCapabilities.maxImageExtent.height )
    {
      swapchainExtent.height = surfCapabilities.maxImageExtent.height;
    }
  }
  else {
    // If the surface size is defined, the swap chain size must match
    swapchainExtent = surfCapabilities.currentExtent;
  }

  // The FIFO present mode is guaranteed by the spec to be supported
  // Also note that current Android driver only supports FIFO
  VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

  // Determine the number of VkImage's to use in the swap chain.
  // We need to acquire only 1 presentable image at at time.
  // Asking for minImageCount images ensures that we can acquire
  // 1 presentable image as long as we present it before attempting
  // to acquire another.
  uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

  VkSurfaceTransformFlagBitsKHR preTransform;
  if ( surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
  {
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }
  else
  {
    preTransform = surfCapabilities.currentTransform;
  }

  // Find a supported composite alpha mode - one of these is guaranteed to be set
  VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[ 4 ] =
  {
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
  };
  for ( uint32_t i = 0; i < countof( compositeAlphaFlags ); i++ )
  {
    if ( surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[ i ] )
    {
      compositeAlpha = compositeAlphaFlags[ i ];
      break;
    }
  }

  VkSwapchainCreateInfoKHR swapChainCreateInfo;
  memset( &swapChainCreateInfo, 0, sizeof( swapChainCreateInfo ) );
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = m_surface;
  swapChainCreateInfo.minImageCount = desiredNumberOfSwapChainImages;
  swapChainCreateInfo.imageFormat = format;
  swapChainCreateInfo.imageExtent.width = swapchainExtent.width;
  swapChainCreateInfo.imageExtent.height = swapchainExtent.height;
  swapChainCreateInfo.preTransform = preTransform;
  swapChainCreateInfo.compositeAlpha = compositeAlpha;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.presentMode = swapchainPresentMode;
  swapChainCreateInfo.clipped = false;
  swapChainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  uint32_t queueFamilyIndices[ 2 ] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
  if ( graphicsQueueFamilyIndex != presentQueueFamilyIndex )
  {
    // If the graphics and present queues are from different queue families,
    // we either have to explicitly transfer ownership of images between the
    // queues, or we have to create the swapchain with imageSharingMode
    // as VK_SHARING_MODE_CONCURRENT
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  }

  res = vkCreateSwapchainKHR( m_device, &swapChainCreateInfo, nullptr, &m_swapChain );
  AE_ASSERT( res == VK_SUCCESS );

  uint32_t swapChainImageCount = 0;
  res = vkGetSwapchainImagesKHR( m_device, m_swapChain, &swapChainImageCount, nullptr );
  AE_ASSERT( res == VK_SUCCESS );

  aeArray< VkImage > swapChainImages( swapChainImageCount, VkImage() );
  AE_ASSERT( swapChainImageCount );
  res = vkGetSwapchainImagesKHR( m_device, m_swapChain, &swapChainImageCount, &swapChainImages[ 0 ] );
  AE_ASSERT( res == VK_SUCCESS );

  m_swapChainBuffers.Reserve( swapChainImageCount );
  for ( uint32_t i = 0; i < swapChainImageCount; i++ )
  {
    VkImageViewCreateInfo imageViewCreateInfo;
    memset( &imageViewCreateInfo, 0, sizeof( imageViewCreateInfo ) );
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.image = swapChainImages[ i ];

    VkImageView view = 0;
    res = vkCreateImageView( m_device, &imageViewCreateInfo, nullptr, &view );
    AE_ASSERT( res == VK_SUCCESS );

    SwapChainBuffer swapChainBuffer;
    swapChainBuffer.image = swapChainImages[ i ];
    swapChainBuffer.view = view;
    m_swapChainBuffers.Append( swapChainBuffer );
  }
  
  //// !!!! init depth buffer
  VkFormat depthFormat = VK_FORMAT_UNDEFINED;
#ifdef __ANDROID__
    // Depth format needs to be VK_FORMAT_D24_UNORM_S8_UINT on Android (if available).
  vkGetPhysicalDeviceFormatProperties( selectedGpu, VK_FORMAT_D24_UNORM_S8_UINT, &formatProps );
  if ( ( formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ) ||
      ( formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ) )
  {
    depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
  }
  else
  {
    depthFormat = VK_FORMAT_D16_UNORM;
  }
#elif defined(VK_USE_PLATFORM_IOS_MVK)
  if ( depthFormat == VK_FORMAT_UNDEFINED )
  {
    depthFormat = VK_FORMAT_D32_SFLOAT;
  }
#else
  if ( depthFormat == VK_FORMAT_UNDEFINED )
  {
    depthFormat = VK_FORMAT_D16_UNORM;
  }
#endif

  VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
  {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties( selectedGpu, depthFormat, &formatProps );
    if ( formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
    {
      imageTiling = VK_IMAGE_TILING_LINEAR;
    }
    else if ( formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
    {
      imageTiling = VK_IMAGE_TILING_OPTIMAL;
    }
    else
    {
      AE_FAIL_MSG( "Depth format '#' not supported.", (uint32_t)depthFormat );
    }
  }
  
  VkImageCreateInfo imageCreateInfo;
  memset( &imageCreateInfo, 0, sizeof( imageCreateInfo ) );
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = depthFormat;
  imageCreateInfo.extent.width = render->GetWidth();
  imageCreateInfo.extent.height = render->GetHeight();
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = NUM_SAMPLES;
  imageCreateInfo.tiling = imageTiling;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  res = vkCreateImage( m_device, &imageCreateInfo, nullptr, &m_depthImage );
  AE_ASSERT( res == VK_SUCCESS );

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements( m_device, m_depthImage, &memReqs );

  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties( selectedGpu, &memoryProperties );

  VkMemoryAllocateInfo memAllocInfo;
  memset( &memAllocInfo, 0, sizeof( memAllocInfo ) );
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memReqs.size;
  bool pass = memory_type_from_properties( memoryProperties, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAllocInfo.memoryTypeIndex );
  AE_ASSERT( pass );
  

  res = vkAllocateMemory( m_device, &memAllocInfo, nullptr, &m_depthMem );
  AE_ASSERT( res == VK_SUCCESS );
  res = vkBindImageMemory( m_device, m_depthImage, m_depthMem, 0 );
  AE_ASSERT( res == VK_SUCCESS );

  VkImageViewCreateInfo viewCreateInfo;
  memset( &viewCreateInfo, 0, sizeof( viewCreateInfo ) );
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.image = m_depthImage;
  viewCreateInfo.format = depthFormat;
  viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
  viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
  viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
  viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
  if ( depthFormat == VK_FORMAT_D16_UNORM_S8_UINT
    || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT
    || depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT )
  {
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  }
  else
  {
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  viewCreateInfo.subresourceRange.baseMipLevel = 0;
  viewCreateInfo.subresourceRange.levelCount = 1;
  viewCreateInfo.subresourceRange.baseArrayLayer = 0;
  viewCreateInfo.subresourceRange.layerCount = 1;
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  res = vkCreateImageView( m_device, &viewCreateInfo, nullptr, &m_depthView );
  AE_ASSERT( res == VK_SUCCESS );


  // !!!! init uniform buffer
  //VkResult U_ASSERT_ONLY res;
  //bool U_ASSERT_ONLY pass;
  //float fov = glm::radians( 45.0f );
  //if ( info.width > info.height ) {
  //  fov *= static_cast<float>( info.height ) / static_cast<float>( info.width );
  //}
  //info.Projection = glm::perspective( fov, static_cast<float>( info.width ) / static_cast<float>( info.height ), 0.1f, 100.0f );
  //info.View = glm::lookAt( glm::vec3( -5, 3, -10 ),  // Camera is at (-5,3,-10), in World Space
  //                        glm::vec3( 0, 0, 0 ),     // and looks at the origin
  //                        glm::vec3( 0, -1, 0 )     // Head is up (set to 0,-1,0 to look upside-down)
  //                        );
  //info.Model = glm::mat4( 1.0f );
  //// Vulkan clip space has inverted Y and half Z.
  //info.Clip = glm::mat4( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f );

  //info.MVP = info.Clip * info.Projection * info.View * info.Model;

  ///* VULKAN_KEY_START */
  //VkBufferCreateInfo buf_info = {};
  //buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  //buf_info.pNext = NULL;
  //buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  //buf_info.size = sizeof( info.MVP );
  //buf_info.queueFamilyIndexCount = 0;
  //buf_info.pQueueFamilyIndices = NULL;
  //buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  //buf_info.flags = 0;
  //res = vkCreateBuffer( info.device, &buf_info, NULL, &info.uniform_data.buf );
  //assert( res == VK_SUCCESS );

  //VkMemoryRequirements mem_reqs;
  //vkGetBufferMemoryRequirements( info.device, info.uniform_data.buf, &mem_reqs );

  //VkMemoryAllocateInfo alloc_info = {};
  //alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //alloc_info.pNext = NULL;
  //alloc_info.memoryTypeIndex = 0;

  //alloc_info.allocationSize = mem_reqs.size;
  //pass = memory_type_from_properties( info, mem_reqs.memoryTypeBits,
  //                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //                                   &alloc_info.memoryTypeIndex );
  //assert( pass && "No mappable, coherent memory" );

  //res = vkAllocateMemory( info.device, &alloc_info, NULL, &( info.uniform_data.mem ) );
  //assert( res == VK_SUCCESS );

  //uint8_t* pData;
  //res = vkMapMemory( info.device, info.uniform_data.mem, 0, mem_reqs.size, 0, (void**)&pData );
  //assert( res == VK_SUCCESS );

  //memcpy( pData, &info.MVP, sizeof( info.MVP ) );

  //vkUnmapMemory( info.device, info.uniform_data.mem );

  //res = vkBindBufferMemory( info.device, info.uniform_data.buf, info.uniform_data.mem, 0 );
  //assert( res == VK_SUCCESS );

  //info.uniform_data.buffer_info.buffer = info.uniform_data.buf;
  //info.uniform_data.buffer_info.offset = 0;
  //info.uniform_data.buffer_info.range = sizeof( info.MVP );


  // !!!! init_descriptor_and_pipeline_layouts
  //VkDescriptorSetLayoutBinding layout_bindings[ 2 ];
  //layout_bindings[ 0 ].binding = 0;
  //layout_bindings[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //layout_bindings[ 0 ].descriptorCount = 1;
  //layout_bindings[ 0 ].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  //layout_bindings[ 0 ].pImmutableSamplers = NULL;

  //if ( use_texture ) {
  //  layout_bindings[ 1 ].binding = 1;
  //  layout_bindings[ 1 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //  layout_bindings[ 1 ].descriptorCount = 1;
  //  layout_bindings[ 1 ].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  //  layout_bindings[ 1 ].pImmutableSamplers = NULL;
  //}

  ///* Next take layout bindings and use them to create a descriptor set layout
  // */
  //VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
  //descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  //descriptor_layout.pNext = NULL;
  //descriptor_layout.flags = descSetLayoutCreateFlags;
  //descriptor_layout.bindingCount = use_texture ? 2 : 1;
  //descriptor_layout.pBindings = layout_bindings;

  //VkResult U_ASSERT_ONLY res;

  //info.desc_layout.resize( NUM_DESCRIPTOR_SETS );
  //res = vkCreateDescriptorSetLayout( info.device, &descriptor_layout, NULL, info.desc_layout.data() );
  //assert( res == VK_SUCCESS );

  ///* Now use the descriptor layout to create a pipeline layout */
  //VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
  //pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  //pPipelineLayoutCreateInfo.pNext = NULL;
  //pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  //pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
  //pPipelineLayoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
  //pPipelineLayoutCreateInfo.pSetLayouts = info.desc_layout.data();

  //res = vkCreatePipelineLayout( info.device, &pPipelineLayoutCreateInfo, NULL, &info.pipeline_layout );
  //assert( res == VK_SUCCESS );


  // !!!! init renderpass
  //VkResult U_ASSERT_ONLY res;
  ///* Need attachments for render target and depth buffer */
  //VkAttachmentDescription attachments[ 2 ];
  //attachments[ 0 ].format = info.format;
  //attachments[ 0 ].samples = NUM_SAMPLES;
  //attachments[ 0 ].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  //attachments[ 0 ].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  //attachments[ 0 ].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  //attachments[ 0 ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  //attachments[ 0 ].initialLayout = initialLayout;
  //attachments[ 0 ].finalLayout = finalLayout;
  //attachments[ 0 ].flags = 0;

  //if ( include_depth ) {
  //  attachments[ 1 ].format = info.depth.format;
  //  attachments[ 1 ].samples = NUM_SAMPLES;
  //  attachments[ 1 ].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  //  attachments[ 1 ].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  //  attachments[ 1 ].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  //  attachments[ 1 ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  //  attachments[ 1 ].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  //  attachments[ 1 ].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  //  attachments[ 1 ].flags = 0;
  //}

  //VkAttachmentReference color_reference = {};
  //color_reference.attachment = 0;
  //color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  //VkAttachmentReference depth_reference = {};
  //depth_reference.attachment = 1;
  //depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  //VkSubpassDescription subpass = {};
  //subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  //subpass.flags = 0;
  //subpass.inputAttachmentCount = 0;
  //subpass.pInputAttachments = NULL;
  //subpass.colorAttachmentCount = 1;
  //subpass.pColorAttachments = &color_reference;
  //subpass.pResolveAttachments = NULL;
  //subpass.pDepthStencilAttachment = include_depth ? &depth_reference : NULL;
  //subpass.preserveAttachmentCount = 0;
  //subpass.pPreserveAttachments = NULL;

  //// Subpass dependency to wait for wsi image acquired semaphore before starting layout transition
  //VkSubpassDependency subpass_dependency = {};
  //subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  //subpass_dependency.dstSubpass = 0;
  //subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  //subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  //subpass_dependency.srcAccessMask = 0;
  //subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  //subpass_dependency.dependencyFlags = 0;

  //VkRenderPassCreateInfo rp_info = {};
  //rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  //rp_info.pNext = NULL;
  //rp_info.attachmentCount = include_depth ? 2 : 1;
  //rp_info.pAttachments = attachments;
  //rp_info.subpassCount = 1;
  //rp_info.pSubpasses = &subpass;
  //rp_info.dependencyCount = 1;
  //rp_info.pDependencies = &subpass_dependency;

  //res = vkCreateRenderPass( info.device, &rp_info, NULL, &info.render_pass );
  //assert( res == VK_SUCCESS );


  // !!!! init_shaders
  //VkResult U_ASSERT_ONLY res;
  //bool U_ASSERT_ONLY retVal;

  //// If no shaders were submitted, just return
  //if ( !( vertShaderText || fragShaderText ) ) return;

  //init_glslang();
  //VkShaderModuleCreateInfo moduleCreateInfo;

  //if ( vertShaderText ) {
  //  std::vector<unsigned int> vtx_spv;
  //  info.shaderStages[ 0 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  //  info.shaderStages[ 0 ].pNext = NULL;
  //  info.shaderStages[ 0 ].pSpecializationInfo = NULL;
  //  info.shaderStages[ 0 ].flags = 0;
  //  info.shaderStages[ 0 ].stage = VK_SHADER_STAGE_VERTEX_BIT;
  //  info.shaderStages[ 0 ].pName = "main";

  //  retVal = GLSLtoSPV( VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv );
  //  assert( retVal );

  //  moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  //  moduleCreateInfo.pNext = NULL;
  //  moduleCreateInfo.flags = 0;
  //  moduleCreateInfo.codeSize = vtx_spv.size() * sizeof( unsigned int );
  //  moduleCreateInfo.pCode = vtx_spv.data();
  //  res = vkCreateShaderModule( info.device, &moduleCreateInfo, NULL, &info.shaderStages[ 0 ].module );
  //  assert( res == VK_SUCCESS );
  //}

  //if ( fragShaderText ) {
  //  std::vector<unsigned int> frag_spv;
  //  info.shaderStages[ 1 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  //  info.shaderStages[ 1 ].pNext = NULL;
  //  info.shaderStages[ 1 ].pSpecializationInfo = NULL;
  //  info.shaderStages[ 1 ].flags = 0;
  //  info.shaderStages[ 1 ].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  //  info.shaderStages[ 1 ].pName = "main";

  //  retVal = GLSLtoSPV( VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, frag_spv );
  //  assert( retVal );

  //  moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  //  moduleCreateInfo.pNext = NULL;
  //  moduleCreateInfo.flags = 0;
  //  moduleCreateInfo.codeSize = frag_spv.size() * sizeof( unsigned int );
  //  moduleCreateInfo.pCode = frag_spv.data();
  //  res = vkCreateShaderModule( info.device, &moduleCreateInfo, NULL, &info.shaderStages[ 1 ].module );
  //  assert( res == VK_SUCCESS );
  //}

  //finalize_glslang();


  // !!!! init_framebuffers
  //VkResult U_ASSERT_ONLY res;
  //VkImageView attachments[ 2 ];
  //attachments[ 1 ] = info.depth.view;

  //VkFramebufferCreateInfo fb_info = {};
  //fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  //fb_info.pNext = NULL;
  //fb_info.renderPass = info.render_pass;
  //fb_info.attachmentCount = include_depth ? 2 : 1;
  //fb_info.pAttachments = attachments;
  //fb_info.width = info.width;
  //fb_info.height = info.height;
  //fb_info.layers = 1;

  //uint32_t i;

  //info.framebuffers = (VkFramebuffer*)malloc( info.swapchainImageCount * sizeof( VkFramebuffer ) );

  //for ( i = 0; i < info.swapchainImageCount; i++ ) {
  //  attachments[ 0 ] = info.buffers[ i ].view;
  //  res = vkCreateFramebuffer( info.device, &fb_info, NULL, &info.framebuffers[ i ] );
  //  assert( res == VK_SUCCESS );
  //}


  // !!!! init vertex buffer
  //VkResult U_ASSERT_ONLY res;
  //bool U_ASSERT_ONLY pass;

  //VkBufferCreateInfo buf_info = {};
  //buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  //buf_info.pNext = NULL;
  //buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  //buf_info.size = dataSize;
  //buf_info.queueFamilyIndexCount = 0;
  //buf_info.pQueueFamilyIndices = NULL;
  //buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  //buf_info.flags = 0;
  //res = vkCreateBuffer( info.device, &buf_info, NULL, &info.vertex_buffer.buf );
  //assert( res == VK_SUCCESS );

  //VkMemoryRequirements mem_reqs;
  //vkGetBufferMemoryRequirements( info.device, info.vertex_buffer.buf, &mem_reqs );

  //VkMemoryAllocateInfo alloc_info = {};
  //alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //alloc_info.pNext = NULL;
  //alloc_info.memoryTypeIndex = 0;

  //alloc_info.allocationSize = mem_reqs.size;
  //pass = memory_type_from_properties( info, mem_reqs.memoryTypeBits,
  //                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //                                   &alloc_info.memoryTypeIndex );
  //assert( pass && "No mappable, coherent memory" );

  //res = vkAllocateMemory( info.device, &alloc_info, NULL, &( info.vertex_buffer.mem ) );
  //assert( res == VK_SUCCESS );
  //info.vertex_buffer.buffer_info.range = mem_reqs.size;
  //info.vertex_buffer.buffer_info.offset = 0;

  //uint8_t* pData;
  //res = vkMapMemory( info.device, info.vertex_buffer.mem, 0, mem_reqs.size, 0, (void**)&pData );
  //assert( res == VK_SUCCESS );

  //memcpy( pData, vertexData, dataSize );

  //vkUnmapMemory( info.device, info.vertex_buffer.mem );

  //res = vkBindBufferMemory( info.device, info.vertex_buffer.buf, info.vertex_buffer.mem, 0 );
  //assert( res == VK_SUCCESS );

  //info.vi_binding.binding = 0;
  //info.vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  //info.vi_binding.stride = dataStride;

  //info.vi_attribs[ 0 ].binding = 0;
  //info.vi_attribs[ 0 ].location = 0;
  //info.vi_attribs[ 0 ].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  //info.vi_attribs[ 0 ].offset = 0;
  //info.vi_attribs[ 1 ].binding = 0;
  //info.vi_attribs[ 1 ].location = 1;
  //info.vi_attribs[ 1 ].format = use_texture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
  //info.vi_attribs[ 1 ].offset = 16;


  // !!!! init_descriptor_pool
  //VkResult U_ASSERT_ONLY res;
  //VkDescriptorPoolSize type_count[ 2 ];
  //type_count[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //type_count[ 0 ].descriptorCount = 1;
  //if ( use_texture ) {
  //  type_count[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //  type_count[ 1 ].descriptorCount = 1;
  //}

  //VkDescriptorPoolCreateInfo descriptor_pool = {};
  //descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  //descriptor_pool.pNext = NULL;
  //descriptor_pool.maxSets = 1;
  //descriptor_pool.poolSizeCount = use_texture ? 2 : 1;
  //descriptor_pool.pPoolSizes = type_count;

  //res = vkCreateDescriptorPool( info.device, &descriptor_pool, NULL, &info.desc_pool );
  //assert( res == VK_SUCCESS );


  // !!!! init_descriptor_set
  //VkResult U_ASSERT_ONLY res;

  //VkDescriptorSetAllocateInfo alloc_info[ 1 ];
  //alloc_info[ 0 ].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  //alloc_info[ 0 ].pNext = NULL;
  //alloc_info[ 0 ].descriptorPool = info.desc_pool;
  //alloc_info[ 0 ].descriptorSetCount = NUM_DESCRIPTOR_SETS;
  //alloc_info[ 0 ].pSetLayouts = info.desc_layout.data();

  //info.desc_set.resize( NUM_DESCRIPTOR_SETS );
  //res = vkAllocateDescriptorSets( info.device, alloc_info, info.desc_set.data() );
  //assert( res == VK_SUCCESS );

  //VkWriteDescriptorSet writes[ 2 ];

  //writes[ 0 ] = {};
  //writes[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //writes[ 0 ].pNext = NULL;
  //writes[ 0 ].dstSet = info.desc_set[ 0 ];
  //writes[ 0 ].descriptorCount = 1;
  //writes[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //writes[ 0 ].pBufferInfo = &info.uniform_data.buffer_info;
  //writes[ 0 ].dstArrayElement = 0;
  //writes[ 0 ].dstBinding = 0;

  //if ( use_texture ) {
  //  writes[ 1 ] = {};
  //  writes[ 1 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //  writes[ 1 ].dstSet = info.desc_set[ 0 ];
  //  writes[ 1 ].dstBinding = 1;
  //  writes[ 1 ].descriptorCount = 1;
  //  writes[ 1 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //  writes[ 1 ].pImageInfo = &info.texture_data.image_info;
  //  writes[ 1 ].dstArrayElement = 0;
  //}

  //vkUpdateDescriptorSets( info.device, use_texture ? 2 : 1, writes, 0, NULL );


  // !!!! init_pipeline_cache
  //VkResult U_ASSERT_ONLY res;

  //VkPipelineCacheCreateInfo pipelineCache;
  //pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  //pipelineCache.pNext = NULL;
  //pipelineCache.initialDataSize = 0;
  //pipelineCache.pInitialData = NULL;
  //pipelineCache.flags = 0;
  //res = vkCreatePipelineCache( info.device, &pipelineCache, NULL, &info.pipelineCache );
  //assert( res == VK_SUCCESS );


  // !!!! init_pipeline
  //VkResult U_ASSERT_ONLY res;

  //VkDynamicState dynamicStateEnables[ VK_DYNAMIC_STATE_RANGE_SIZE ];
  //VkPipelineDynamicStateCreateInfo dynamicState = {};
  //memset( dynamicStateEnables, 0, sizeof dynamicStateEnables );
  //dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  //dynamicState.pNext = NULL;
  //dynamicState.pDynamicStates = dynamicStateEnables;
  //dynamicState.dynamicStateCount = 0;

  //VkPipelineVertexInputStateCreateInfo vi;
  //memset( &vi, 0, sizeof( vi ) );
  //vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  //if ( include_vi ) {
  //  vi.pNext = NULL;
  //  vi.flags = 0;
  //  vi.vertexBindingDescriptionCount = 1;
  //  vi.pVertexBindingDescriptions = &info.vi_binding;
  //  vi.vertexAttributeDescriptionCount = 2;
  //  vi.pVertexAttributeDescriptions = info.vi_attribs;
  //}
  //VkPipelineInputAssemblyStateCreateInfo ia;
  //ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  //ia.pNext = NULL;
  //ia.flags = 0;
  //ia.primitiveRestartEnable = VK_FALSE;
  //ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  //VkPipelineRasterizationStateCreateInfo rs;
  //rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  //rs.pNext = NULL;
  //rs.flags = 0;
  //rs.polygonMode = VK_POLYGON_MODE_FILL;
  //rs.cullMode = VK_CULL_MODE_BACK_BIT;
  //rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
  //rs.depthClampEnable = VK_FALSE;
  //rs.rasterizerDiscardEnable = VK_FALSE;
  //rs.depthBiasEnable = VK_FALSE;
  //rs.depthBiasConstantFactor = 0;
  //rs.depthBiasClamp = 0;
  //rs.depthBiasSlopeFactor = 0;
  //rs.lineWidth = 1.0f;

  //VkPipelineColorBlendStateCreateInfo cb;
  //cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  //cb.flags = 0;
  //cb.pNext = NULL;
  //VkPipelineColorBlendAttachmentState att_state[ 1 ];
  //att_state[ 0 ].colorWriteMask = 0xf;
  //att_state[ 0 ].blendEnable = VK_FALSE;
  //att_state[ 0 ].alphaBlendOp = VK_BLEND_OP_ADD;
  //att_state[ 0 ].colorBlendOp = VK_BLEND_OP_ADD;
  //att_state[ 0 ].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  //att_state[ 0 ].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  //att_state[ 0 ].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  //att_state[ 0 ].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  //cb.attachmentCount = 1;
  //cb.pAttachments = att_state;
  //cb.logicOpEnable = VK_FALSE;
  //cb.logicOp = VK_LOGIC_OP_NO_OP;
  //cb.blendConstants[ 0 ] = 1.0f;
  //cb.blendConstants[ 1 ] = 1.0f;
  //cb.blendConstants[ 2 ] = 1.0f;
  //cb.blendConstants[ 3 ] = 1.0f;

  //VkPipelineViewportStateCreateInfo vp = {};
  //vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  //vp.pNext = NULL;
  //vp.flags = 0;
  //#ifndef __ANDROID__
  //vp.viewportCount = NUM_VIEWPORTS;
  //dynamicStateEnables[ dynamicState.dynamicStateCount++ ] = VK_DYNAMIC_STATE_VIEWPORT;
  //vp.scissorCount = NUM_SCISSORS;
  //dynamicStateEnables[ dynamicState.dynamicStateCount++ ] = VK_DYNAMIC_STATE_SCISSOR;
  //vp.pScissors = NULL;
  //vp.pViewports = NULL;
  //#else
  //// Temporary disabling dynamic viewport on Android because some of drivers doesn't
  //// support the feature.
  //VkViewport viewports;
  //viewports.minDepth = 0.0f;
  //viewports.maxDepth = 1.0f;
  //viewports.x = 0;
  //viewports.y = 0;
  //viewports.width = info.width;
  //viewports.height = info.height;
  //VkRect2D scissor;
  //scissor.extent.width = info.width;
  //scissor.extent.height = info.height;
  //scissor.offset.x = 0;
  //scissor.offset.y = 0;
  //vp.viewportCount = NUM_VIEWPORTS;
  //vp.scissorCount = NUM_SCISSORS;
  //vp.pScissors = &scissor;
  //vp.pViewports = &viewports;
  //#endif
  //VkPipelineDepthStencilStateCreateInfo ds;
  //ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  //ds.pNext = NULL;
  //ds.flags = 0;
  //ds.depthTestEnable = include_depth;
  //ds.depthWriteEnable = include_depth;
  //ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  //ds.depthBoundsTestEnable = VK_FALSE;
  //ds.stencilTestEnable = VK_FALSE;
  //ds.back.failOp = VK_STENCIL_OP_KEEP;
  //ds.back.passOp = VK_STENCIL_OP_KEEP;
  //ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
  //ds.back.compareMask = 0;
  //ds.back.reference = 0;
  //ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
  //ds.back.writeMask = 0;
  //ds.minDepthBounds = 0;
  //ds.maxDepthBounds = 0;
  //ds.stencilTestEnable = VK_FALSE;
  //ds.front = ds.back;

  //VkPipelineMultisampleStateCreateInfo ms;
  //ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  //ms.pNext = NULL;
  //ms.flags = 0;
  //ms.pSampleMask = NULL;
  //ms.rasterizationSamples = NUM_SAMPLES;
  //ms.sampleShadingEnable = VK_FALSE;
  //ms.alphaToCoverageEnable = VK_FALSE;
  //ms.alphaToOneEnable = VK_FALSE;
  //ms.minSampleShading = 0.0;

  //VkGraphicsPipelineCreateInfo pipeline;
  //pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  //pipeline.pNext = NULL;
  //pipeline.layout = info.pipeline_layout;
  //pipeline.basePipelineHandle = VK_NULL_HANDLE;
  //pipeline.basePipelineIndex = 0;
  //pipeline.flags = 0;
  //pipeline.pVertexInputState = &vi;
  //pipeline.pInputAssemblyState = &ia;
  //pipeline.pRasterizationState = &rs;
  //pipeline.pColorBlendState = &cb;
  //pipeline.pTessellationState = NULL;
  //pipeline.pMultisampleState = &ms;
  //pipeline.pDynamicState = &dynamicState;
  //pipeline.pViewportState = &vp;
  //pipeline.pDepthStencilState = &ds;
  //pipeline.pStages = info.shaderStages;
  //pipeline.stageCount = 2;
  //pipeline.renderPass = info.render_pass;
  //pipeline.subpass = 0;

  //res = vkCreateGraphicsPipelines( info.device, info.pipelineCache, 1, &pipeline, NULL, &info.pipeline );
  //assert( res == VK_SUCCESS );
}

void aeVulkanRender::Terminate( aeRender* render )
{
  vkDestroyImageView( m_device, m_depthView, nullptr );
  vkDestroyImage( m_device, m_depthImage, nullptr );
  vkFreeMemory( m_device, m_depthMem, nullptr );

  for ( uint32_t i = 0; i < m_swapChainBuffers.Length(); i++ )
  {
    vkDestroyImageView( m_device, m_swapChainBuffers[ i ].view, nullptr );
  }
  vkDestroySwapchainKHR( m_device, m_swapChain, nullptr );

  VkCommandBuffer cmdBuffs[ 1 ] = { m_cmd };
  vkFreeCommandBuffers( m_device, m_cmdPool, countof( cmdBuffs ), cmdBuffs );
  vkDestroyCommandPool( m_device, m_cmdPool, nullptr );

  vkDestroyDevice( m_device, nullptr );
  vkDestroySurfaceKHR( m_inst, m_surface, nullptr );
  PFN_vkDestroyDebugReportCallbackEXT destroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( m_inst, "vkDestroyDebugReportCallbackEXT" );
  AE_ASSERT( destroyDebugReportCallback );
  destroyDebugReportCallback( m_inst, m_debugCallback, nullptr );
  vkDestroyInstance( m_inst, nullptr );
}

void aeVulkanRender::StartFrame( aeRender* render )
{
  //VkResult res = VK_SUCCESS;

  //VkClearValue clearValues[2];
  //clearValues[ 0 ].color.float32[ 0 ] = 0.2f;
  //clearValues[ 0 ].color.float32[ 1 ] = 0.2f;
  //clearValues[ 0 ].color.float32[ 2 ] = 0.2f;
  //clearValues[ 0 ].color.float32[ 3 ] = 0.2f;
  //clearValues[ 1 ].depthStencil.depth = 1.0f;
  //clearValues[ 1 ].depthStencil.stencil = 0;

  //VkSemaphore imageAcquiredSemaphore;
  //VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
  //imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  //imageAcquiredSemaphoreCreateInfo.pNext = NULL;
  //imageAcquiredSemaphoreCreateInfo.flags = 0;

  //res = vkCreateSemaphore( m_device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore );
  //AE_ASSERT( res == VK_SUCCESS );

  //// Get the index of the next available swapchain image:
  //res = vkAcquireNextImageKHR( m_device, m_swapChain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
  //  &m_currentBuffer );
  //// TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
  //// return codes
  //AE_ASSERT( res == VK_SUCCESS );

  //VkRenderPassBeginInfo rp_begin;
  //memset( &rp_begin, 0, sizeof( rp_begin ) );
  //rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  //rp_begin.renderPass = m_renderPass;
  //rp_begin.framebuffer = m_frameBuffers[m_currentBuffer];
  //rp_begin.renderArea.offset.x = 0;
  //rp_begin.renderArea.offset.y = 0;
  //rp_begin.renderArea.extent.width = render->GetWidth();
  //rp_begin.renderArea.extent.height = render->GetHeight();
  //rp_begin.clearValueCount = 2;
  //rp_begin.pClearValues = clearValues;

  //vkCmdBeginRenderPass( info.cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE );

  //vkCmdBindPipeline( m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline );
  //vkCmdBindDescriptorSets( m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, NUM_DESCRIPTOR_SETS,
  //  m_descSet.data(), 0, NULL );

  //const VkDeviceSize offsets[1] = { 0 };
  //vkCmdBindVertexBuffers( m_cmd, 0, 1, &m_vertexBuffer.buf, offsets );

  //init_viewports( info );
  //init_scissors( info );

  //vkCmdDraw( info.cmd, 12 * 3, 1, 0, 0 );
}

void aeVulkanRender::EndFrame( aeRender* render )
{
  //VkResult res = VK_SUCCESS;

  //vkCmdEndRenderPass( info.cmd );
  //res = vkEndCommandBuffer( info.cmd );
  //const VkCommandBuffer cmd_bufs[] = { info.cmd };
  //VkFenceCreateInfo fenceInfo;
  //VkFence drawFence;
  //fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  //fenceInfo.pNext = NULL;
  //fenceInfo.flags = 0;
  //vkCreateFence( info.device, &fenceInfo, NULL, &drawFence );

  //VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  //VkSubmitInfo submit_info[1] = {};
  //submit_info[0].pNext = NULL;
  //submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  //submit_info[0].waitSemaphoreCount = 1;
  //submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
  //submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
  //submit_info[0].commandBufferCount = 1;
  //submit_info[0].pCommandBuffers = cmd_bufs;
  //submit_info[0].signalSemaphoreCount = 0;
  //submit_info[0].pSignalSemaphores = NULL;

  ///* Queue the command buffer for execution */
  //res = vkQueueSubmit( info.graphics_queue, 1, submit_info, drawFence );
  //assert( res == VK_SUCCESS );

  ///* Now present the image in the window */

  //VkPresentInfoKHR present;
  //memset( &present, 0, sizeof( present ) );
  //present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  //present.swapchainCount = 1;
  //present.pSwapchains = &info.swap_chain;
  //present.pImageIndices = &info.current_buffer;

  ///* Make sure command buffer is finished before presenting */
  //do {
  //  res = vkWaitForFences( info.device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT );
  //} while (res == VK_TIMEOUT);

  //assert( res == VK_SUCCESS );
  //res = vkQueuePresentKHR( info.present_queue, &present );
  //assert( res == VK_SUCCESS );
}
