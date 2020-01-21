//------------------------------------------------------------------------------
// aeMesh.cpp
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
#include "aeMesh.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//------------------------------------------------------------------------------
// aeMesh member functions
//------------------------------------------------------------------------------
bool aeMesh::LoadFileData( const uint8_t* data, uint32_t length, const char* extension )
{
  m_vertices.Clear();
  m_indices.Clear();

#if _AE_DEBUG_
  aiLogStream stream = aiGetPredefinedLogStream( aiDefaultLogStream_STDOUT, nullptr );
  aiAttachLogStream( &stream );
#endif

  uint32_t importFlags = 0;
  importFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
  importFlags |= aiProcess_JoinIdenticalVertices;
  importFlags |= aiProcess_PreTransformVertices;
  
  const struct aiScene* scene = aiImportFileFromMemory( (const char*)data, length, importFlags, extension );
  if ( !scene )
  {
    return false;
  }
  
  uint32_t meshCount = scene->mNumMeshes;
  for ( uint32_t i = 0; i < meshCount; i++ )
  {
    const struct aiMesh* mesh = scene->mMeshes[ i ];

    uint32_t vertCount = mesh->mNumVertices;
    uint32_t uvCount = aeMath::Min( mesh->GetNumUVChannels(), countof(aeMeshVertex::tex) );
    uint32_t colorCount = aeMath::Min( mesh->GetNumColorChannels(), countof(aeMeshVertex::color) );
    AE_LOGD( "Submesh: vertCount # uvCount # colorCount #", vertCount, uvCount, colorCount );

    for ( uint32_t j = 0; j < vertCount; j++ )
    {
      aeMeshVertex vertex;
      
      // Position
      aiVector3D p = mesh->mVertices[ j ];
      vertex.position = aeFloat4( p.x, p.y, p.z, 1.0f );

      // Normal
      aiVector3D n = mesh->mNormals[ j ];
      vertex.normal = aeFloat4( n.x, n.y, n.z, 1.0f );

      // UVs
      memset( vertex.tex, 0, sizeof(vertex.tex) );
      for ( uint32_t k = 0; k < uvCount; k++ )
      {
        AE_ASSERT( mesh->mTextureCoords[ k ] );
        // @NOTE: An aiMesh may contain 0 to AI_MAX_NUMBER_OF_TEXTURECOORDS per vertex.
        //        NULL if not present. The array is mNumVertices in size.
        aiVector3D t = mesh->mTextureCoords[ k ][ j ];
        vertex.tex[ k ] = aeFloat2( t.x, t.y );
      }

      // Color
      memset( vertex.color, 0, sizeof(vertex.color) );
      for ( uint32_t k = 0; k < colorCount; k++ )
      {
        AE_ASSERT( mesh->mColors[ k ] );
        aiColor4D c = mesh->mColors[ k ][ j ];
        vertex.color[ k ] = aeColor( c.r, c.g, c.b, c.a );
      }

      m_vertices.Append( vertex );
    }
  }
  
  uint32_t currentVertexCount = 0;
  for ( uint32_t i = 0; i < meshCount; i++ )
  {
    const struct aiMesh* mesh = scene->mMeshes[ i ];
    uint32_t faceCount = mesh->mNumFaces;
    
    for ( uint32_t j = 0; j < faceCount; j++ )
    {
      uint32_t faceIndexCount = mesh->mFaces[ j ].mNumIndices;
      if ( faceIndexCount == 3 )
      {
        for ( uint32_t k = 0; k < 3; k++ )
        {
          uint32_t index = currentVertexCount + mesh->mFaces[ j ].mIndices[ k ];
          AE_ASSERT( index < std::numeric_limits< uint16_t >::max() );
          m_indices.Append( (uint16_t)index );
        }
      }
    }
    
    currentVertexCount += mesh->mNumVertices;
  }
  
  aiReleaseImport( scene );
#if _AE_DEBUG_
  aiDetachAllLogStreams();
#endif

  return true;
}

void aeMesh::Transform( aeFloat4x4 transform )
{
  // @TODO: Transform normals
  for ( uint32_t i = 0; i < m_vertices.Length(); i++ )
  {
    m_vertices[ i ].position = transform * m_vertices[ i ].position;
  }
}

const aeMeshVertex* aeMesh::GetVertices() const
{
  return m_vertices.Length() ? &m_vertices[ 0 ] : nullptr;
}

const aeMeshIndex* aeMesh::GetIndices() const
{
  return m_indices.Length() ? &m_indices[ 0 ] : nullptr;
}

uint32_t aeMesh::GetVertexCount() const
{
  return m_vertices.Length();
}

uint32_t aeMesh::GetIndexCount() const
{
  return m_indices.Length();
}